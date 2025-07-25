#pragma once

#include "../Types/Quantity.h"
#include "../Types/Expected.h"
#include "../Time/Date.h"
#include "../Patterns/Observable.h"
#include <memory>
#include <string>
#include <optional>

namespace QuantLib {

// Forward declarations
class PricingEngine;

// Instrument interface using CRTP for zero-overhead polymorphism
template<typename Derived>
class Instrument : public Observable<Derived> {
public:
    using PricingEnginePtr = std::shared_ptr<PricingEngine>;

    virtual ~Instrument() = default;

    // Core interface - implemented by derived classes
    Price npv() const {
        if (!pricingEngine_) {
            throw std::runtime_error("Pricing engine not set");
        }
        return static_cast<const Derived&>(*this).npv_impl();
    }

    Result<Price> tryNpv() const noexcept {
        if (!pricingEngine_) {
            return failure<Price>(QuantLibError::PricingEngineNotSet);
        }
        return static_cast<const Derived&>(*this).tryNpv_impl();
    }

    bool isExpired(const Date& valuationDate) const {
        return static_cast<const Derived&>(*this).isExpired_impl(valuationDate);
    }

    // Pricing engine management
    void setPricingEngine(PricingEnginePtr engine) {
        pricingEngine_ = std::move(engine);
        this->notifyObservers();
    }

    const PricingEnginePtr& pricingEngine() const {
        return pricingEngine_;
    }

    bool hasPricingEngine() const {
        return pricingEngine_ != nullptr;
    }

    // Instrument identification
    const std::string& identifier() const { return identifier_; }
    void setIdentifier(const std::string& id) { 
        identifier_ = id; 
        this->notifyObservers();
    }

    // Valuation date
    std::optional<Date> valuationDate() const { return valuationDate_; }
    void setValuationDate(const Date& date) { 
        valuationDate_ = date; 
        this->notifyObservers();
    }

    // Additional results (can be extended by derived classes)
    template<typename T>
    std::optional<T> additionalResult(const std::string& key) const {
        auto it = additionalResults_.find(key);
        if (it != additionalResults_.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    template<typename T>
    void setAdditionalResult(const std::string& key, const T& value) {
        additionalResults_[key] = value;
    }

    void clearAdditionalResults() {
        additionalResults_.clear();
    }

    // Validation
    virtual bool isValid() const {
        return static_cast<const Derived&>(*this).isValid_impl();
    }

    Result<void> validate() const {
        if (!isValid()) {
            return failure<void>(QuantLibError::InvalidArgument);
        }
        return success<void>({});
    }

protected:
    Instrument() = default;
    Instrument(const std::string& identifier) : identifier_(identifier) {}

    // Force derived classes to implement these
    Price npv_impl() const = delete;
    Result<Price> tryNpv_impl() const = delete;
    bool isExpired_impl(const Date& valuationDate) const = delete;
    bool isValid_impl() const = delete;

private:
    PricingEnginePtr pricingEngine_;
    std::string identifier_;
    std::optional<Date> valuationDate_;
    std::map<std::string, std::any> additionalResults_;
};

// Instrument registry for managing collections of instruments
class InstrumentRegistry {
public:
    template<typename InstrumentType>
    void registerInstrument(const std::string& id, std::shared_ptr<InstrumentType> instrument) {
        instruments_[id] = instrument;
        instrumentsByType_[typeid(InstrumentType)].push_back(instrument);
    }

    template<typename InstrumentType>
    std::shared_ptr<InstrumentType> getInstrument(const std::string& id) const {
        auto it = instruments_.find(id);
        if (it != instruments_.end()) {
            return std::dynamic_pointer_cast<InstrumentType>(it->second);
        }
        return nullptr;
    }

    template<typename InstrumentType>
    std::vector<std::shared_ptr<InstrumentType>> getInstrumentsByType() const {
        std::vector<std::shared_ptr<InstrumentType>> result;
        auto it = instrumentsByType_.find(typeid(InstrumentType));
        if (it != instrumentsByType_.end()) {
            for (const auto& weak : it->second) {
                if (auto strong = weak.lock()) {
                    if (auto typed = std::dynamic_pointer_cast<InstrumentType>(strong)) {
                        result.push_back(typed);
                    }
                }
            }
        }
        return result;
    }

    void removeInstrument(const std::string& id) {
        instruments_.erase(id);
    }

    std::vector<std::string> getAllIds() const {
        std::vector<std::string> ids;
        for (const auto& [id, _] : instruments_) {
            ids.push_back(id);
        }
        return ids;
    }

    size_t size() const { return instruments_.size(); }
    bool empty() const { return instruments_.empty(); }

    void clear() {
        instruments_.clear();
        instrumentsByType_.clear();
    }

private:
    std::map<std::string, std::shared_ptr<void>> instruments_;
    std::map<std::type_index, std::vector<std::weak_ptr<void>>> instrumentsByType_;
};

// Instrument visitor pattern for type-safe operations
template<typename Visitor>
class InstrumentVisitor {
public:
    template<typename InstrumentType>
    auto visit(const InstrumentType& instrument) {
        return static_cast<Visitor&>(*this).visit_impl(instrument);
    }

protected:
    InstrumentVisitor() = default;
};

// Portfolio class for managing collections of instruments
class Portfolio : public Observable<Portfolio> {
public:
    void addInstrument(const std::string& id, std::shared_ptr<void> instrument, Notional weight = Notional(1.0)) {
        positions_[id] = {instrument, weight};
        this->notifyObservers();
    }

    void removeInstrument(const std::string& id) {
        positions_.erase(id);
        this->notifyObservers();
    }

    void setWeight(const std::string& id, Notional weight) {
        auto it = positions_.find(id);
        if (it != positions_.end()) {
            it->second.weight = weight;
            this->notifyObservers();
        }
    }

    template<typename InstrumentType>
    std::shared_ptr<InstrumentType> getInstrument(const std::string& id) const {
        auto it = positions_.find(id);
        if (it != positions_.end()) {
            return std::dynamic_pointer_cast<InstrumentType>(it->second.instrument);
        }
        return nullptr;
    }

    Notional getWeight(const std::string& id) const {
        auto it = positions_.find(id);
        return it != positions_.end() ? it->second.weight : Notional(0.0);
    }

    // Portfolio-level calculations
    template<typename CalculationFunc>
    auto calculate(CalculationFunc&& func) const {
        using ResultType = decltype(func(*positions_.begin()->second.instrument, positions_.begin()->second.weight));
        ResultType total{};
        
        for (const auto& [id, position] : positions_) {
            total += func(*position.instrument, position.weight);
        }
        return total;
    }

    Price npv() const {
        return calculate([](const auto& instrument, Notional weight) {
            // This would need proper typing in real implementation
            // return instrument.npv() * weight;
            return Price(0.0); // Placeholder
        });
    }

    std::vector<std::string> getInstrumentIds() const {
        std::vector<std::string> ids;
        for (const auto& [id, _] : positions_) {
            ids.push_back(id);
        }
        return ids;
    }

    size_t size() const { return positions_.size(); }
    bool empty() const { return positions_.empty(); }

private:
    struct Position {
        std::shared_ptr<void> instrument;
        Notional weight;
    };

    std::map<std::string, Position> positions_;
};

} // namespace QuantLib
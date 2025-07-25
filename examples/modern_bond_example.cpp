#include "../include/QuantLib.h"
#include <iostream>

using namespace QuantLib;
using namespace QuantLib::literals;

// Modern Fixed Rate Bond implementation using CRTP
class FixedRateBond : public Instrument<FixedRateBond> {
public:
    FixedRateBond(const std::string& id, 
                  const Date& issueDate,
                  const Date& maturityDate,
                  Notional faceValue,
                  Rate couponRate,
                  int frequency = 2)
        : Instrument(id)
        , issueDate_(issueDate)
        , maturityDate_(maturityDate) 
        , faceValue_(faceValue)
        , couponRate_(couponRate)
        , frequency_(frequency) {}

    // CRTP implementation methods
    Price npv_impl() const {
        if (!pricingEngine()) {
            throw std::runtime_error("No pricing engine set");
        }
        // Delegate to pricing engine
        return calculateNPV();
    }

    Result<Price> tryNpv_impl() const noexcept {
        try {
            return success(npv_impl());
        } catch (const std::exception&) {
            return failure<Price>(QuantLibError::NumericalError);
        }
    }

    bool isExpired_impl(const Date& valuationDate) const {
        return valuationDate >= maturityDate_;
    }

    bool isValid_impl() const {
        return issueDate_ < maturityDate_ && 
               faceValue_.value() > 0 && 
               couponRate_.value() >= 0;
    }

    // Bond-specific interface
    const Date& issueDate() const { return issueDate_; }
    const Date& maturityDate() const { return maturityDate_; }
    Notional faceValue() const { return faceValue_; }
    Rate couponRate() const { return couponRate_; }
    int frequency() const { return frequency_; }

    // Calculate time to maturity with type safety
    Time timeToMaturity(const Date& valuationDate) const {
        if (isExpired_impl(valuationDate)) {
            return 0.0_years;
        }
        return yearFraction(valuationDate, maturityDate_, DayCountConvention::Actual365);
    }

    // Generate cashflows with modern C++ features
    std::vector<std::pair<Date, Price>> generateCashFlows() const {
        std::vector<std::pair<Date, Price>> cashflows;
        
        // Calculate coupon payment dates
        auto periodicRate = couponRate_ / frequency_;
        auto couponAmount = Price(faceValue_.value() * periodicRate.value());
        
        // Generate coupon payments (simplified)
        Date currentDate = issueDate_;
        while (currentDate < maturityDate_) {
            currentDate += (12 / frequency_) * 30; // Simplified monthly increment
            if (currentDate <= maturityDate_) {
                cashflows.emplace_back(currentDate, couponAmount);
            }
        }
        
        // Add principal repayment
        cashflows.emplace_back(maturityDate_, Price(faceValue_.value()));
        
        return cashflows;
    }

    // Serialization support
    std::string serialize() const {
        return identifier() + "|" + 
               issueDate_.toString() + "|" +
               maturityDate_.toString() + "|" +
               std::to_string(faceValue_.value()) + "|" +
               std::to_string(couponRate_.value()) + "|" +
               std::to_string(frequency_);
    }

    static Result<FixedRateBond> deserialize(const std::string& data) {
        // Implementation would parse the serialized string
        // Returning placeholder for demonstration
        return failure<FixedRateBond>(QuantLibError::InvalidArgument);
    }

private:
    Date issueDate_;
    Date maturityDate_;
    Notional faceValue_;
    Rate couponRate_;
    int frequency_;

    Price calculateNPV() const {
        // Simplified NPV calculation
        auto cashflows = generateCashFlows();
        Price npv(0.0);
        
        if (auto valDate = valuationDate()) {
            Rate discountRate = 0.05_rate; // Simplified: should come from market data
            
            for (const auto& [date, amount] : cashflows) {
                if (date > *valDate) {
                    auto timeToFlow = yearFraction(*valDate, date, DayCountConvention::Actual365);
                    auto discountFactor = DiscountFactor(std::exp(-discountRate.value() * timeToFlow.value()));
                    npv += Price(amount.value() * discountFactor.value());
                }
            }
        }
        
        return npv;
    }
};

// Example pricing engine using strategy pattern
class SimpleBondPricingEngine {
public:
    static Price price(const FixedRateBond& bond, Rate discountRate, const Date& valuationDate) {
        auto cashflows = bond.generateCashFlows();
        Price npv(0.0);
        
        for (const auto& [date, amount] : cashflows) {
            if (date > valuationDate) {
                auto timeToFlow = yearFraction(valuationDate, date, DayCountConvention::Actual365);
                auto discountFactor = DiscountFactor(std::exp(-discountRate.value() * timeToFlow.value()));
                npv += Price(amount.value() * discountFactor.value());
            }
        }
        
        return npv;
    }
};

// Portfolio risk calculator using functional programming concepts
class PortfolioRiskCalculator {
public:
    template<typename InstrumentType>
    static Rate calculatePortfolioDuration(const Portfolio& portfolio) {
        Rate totalDuration(0.0);
        // Implementation would calculate weighted duration
        return totalDuration;
    }
};

// Demonstration of the modern architecture
int main() {
    std::cout << "=== Modern C++ Quantitative Library Demo ===\n\n";

    try {
        // Create bonds with type-safe quantities and literals
        auto bond1 = FixedRateBond(
            "US_TREASURY_10Y",
            Date(2024, 1, 15),
            Date(2034, 1, 15),
            100000.0_notional,
            4.5_rate / 100.0,  // Type-safe rate conversion
            2  // Semi-annual
        );

        auto bond2 = FixedRateBond(
            "CORP_BOND_5Y",
            Date(2024, 3, 1),
            Date(2029, 3, 1),
            50000.0_notional,
            5.25_rate / 100.0,
            2
        );

        // Set valuation date
        Date valuationDate(2024, 6, 15);
        bond1.setValuationDate(valuationDate);
        bond2.setValuationDate(valuationDate);

        // Demonstrate type safety
        std::cout << "1. Type Safety Demonstration:\n";
        auto faceValue = bond1.faceValue();
        auto couponRate = bond1.couponRate();
        auto timeToMaturity = bond1.timeToMaturity(valuationDate);
        
        std::cout << "   Face Value: " << faceValue << "\n";
        std::cout << "   Coupon Rate: " << couponRate << "\n";
        std::cout << "   Time to Maturity: " << timeToMaturity << " years\n\n";

        // Demonstrate monadic error handling
        std::cout << "2. Monadic Error Handling:\n";
        auto npvResult = bond1.tryNpv();
        npvResult
            .ifValue([](Price npv) {
                std::cout << "   Bond NPV calculated successfully: " << npv << "\n";
            })
            .ifError([](const std::error_code& error) {
                std::cout << "   Error calculating NPV: " << error.message() << "\n";
            });

        // Demonstrate observer pattern
        std::cout << "\n3. Observer Pattern:\n";
        auto observer = makeLambdaObserver<FixedRateBond>(
            [](const FixedRateBond& bond) {
                std::cout << "   Bond " << bond.identifier() << " was updated!\n";
            }
        );
        
        bond1.registerObserver(observer);
        bond1.setIdentifier("US_TREASURY_10Y_UPDATED");  // Triggers notification

        // Demonstrate cashflow generation
        std::cout << "\n4. Cashflow Generation:\n";
        auto cashflows = bond1.generateCashFlows();
        std::cout << "   Generated " << cashflows.size() << " cashflows:\n";
        for (size_t i = 0; i < std::min(size_t(3), cashflows.size()); ++i) {
            const auto& [date, amount] = cashflows[i];
            std::cout << "   " << date.toString() << ": $" << amount << "\n";
        }
        if (cashflows.size() > 3) {
            std::cout << "   ... and " << (cashflows.size() - 3) << " more\n";
        }

        // Demonstrate portfolio management
        std::cout << "\n5. Portfolio Management:\n";
        Portfolio portfolio;
        portfolio.addInstrument("BOND_1", std::make_shared<FixedRateBond>(bond1), 0.6_notional);
        portfolio.addInstrument("BOND_2", std::make_shared<FixedRateBond>(bond2), 0.4_notional);
        
        std::cout << "   Portfolio contains " << portfolio.size() << " instruments\n";
        auto bondIds = portfolio.getInstrumentIds();
        for (const auto& id : bondIds) {
            auto weight = portfolio.getWeight(id);
            std::cout << "   " << id << " (weight: " << weight << ")\n";
        }

        // Demonstrate functional pricing
        std::cout << "\n6. Functional Pricing:\n";
        Rate discountRate = 4.0_rate / 100.0;
        auto price1 = SimpleBondPricingEngine::price(bond1, discountRate, valuationDate);
        auto price2 = SimpleBondPricingEngine::price(bond2, discountRate, valuationDate);
        
        std::cout << "   " << bond1.identifier() << " price: $" << price1 << "\n";
        std::cout << "   " << bond2.identifier() << " price: $" << price2 << "\n";

        // Demonstrate serialization
        std::cout << "\n7. Serialization:\n";
        auto serialized = bond1.serialize();
        std::cout << "   Serialized bond: " << serialized << "\n";

        std::cout << "\n8. Validation:\n";
        auto validationResult = bond1.validate();
        if (validationResult) {
            std::cout << "   Bond validation: PASSED\n";
        } else {
            std::cout << "   Bond validation: FAILED - " << validationResult.error().message() << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n=== Demo completed successfully! ===\n";
    return 0;
}
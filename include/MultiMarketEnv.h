#pragma once

#include "Common.h"
#include "Date.h"
#include "InterestRateCurve.h"
#include <map>
#include <string>
#include <memory>

namespace QuantLib {

// Market data types
enum class MarketDataType {
    INTEREST_RATE_CURVE,
    EQUITY_SPOT,
    FX_RATE,
    VOLATILITY_SURFACE,
    CREDIT_CURVE,
    DIVIDEND_YIELD
};

// Base class for market data
class MarketData {
public:
    MarketData(const std::string& name, MarketDataType type, const Date& asOfDate)
        : name_(name), type_(type), asOfDate_(asOfDate) {}
    
    virtual ~MarketData() = default;
    
    const std::string& name() const { return name_; }
    MarketDataType type() const { return type_; }
    const Date& asOfDate() const { return asOfDate_; }
    
    virtual std::string serialize() const = 0;

protected:
    std::string name_;
    MarketDataType type_;
    Date asOfDate_;
};

// Specific market data types
class EquitySpot : public MarketData {
public:
    EquitySpot(const std::string& name, const Date& asOfDate, Real spot)
        : MarketData(name, MarketDataType::EQUITY_SPOT, asOfDate), spot_(spot) {}
    
    Real getSpot() const { return spot_; }
    void setSpot(Real spot) { spot_ = spot; }
    
    std::string serialize() const override;
    static std::shared_ptr<EquitySpot> deserialize(const std::string& data);

private:
    Real spot_;
};

class FXRate : public MarketData {
public:
    FXRate(const std::string& currencyPair, const Date& asOfDate, Real rate)
        : MarketData(currencyPair, MarketDataType::FX_RATE, asOfDate), rate_(rate) {}
    
    Real getRate() const { return rate_; }
    void setRate(Real rate) { rate_ = rate; }
    
    std::string serialize() const override;
    static std::shared_ptr<FXRate> deserialize(const std::string& data);

private:
    Real rate_;
};

class DividendYield : public MarketData {
public:
    DividendYield(const std::string& name, const Date& asOfDate, Real yield)
        : MarketData(name, MarketDataType::DIVIDEND_YIELD, asOfDate), yield_(yield) {}
    
    Real getYield() const { return yield_; }
    void setYield(Real yield) { yield_ = yield; }
    
    std::string serialize() const override;
    static std::shared_ptr<DividendYield> deserialize(const std::string& data);

private:
    Real yield_;
};

// Main market environment class
class MultiMarketEnv {
public:
    // Constructors
    MultiMarketEnv();
    explicit MultiMarketEnv(const Date& valuationDate);
    
    // Copy constructor and assignment
    MultiMarketEnv(const MultiMarketEnv& other);
    MultiMarketEnv& operator=(const MultiMarketEnv& other);
    
    // Destructor
    ~MultiMarketEnv() = default;
    
    // Valuation date
    const Date& valuationDate() const { return valuationDate_; }
    void setValuationDate(const Date& date) { valuationDate_ = date; }
    
    // Interest rate curves
    void addInterestRateCurve(const std::string& name, const InterestRateCurve& curve);
    const InterestRateCurve& getInterestRateCurve(const std::string& name) const;
    InterestRateCurve& getInterestRateCurve(const std::string& name);
    bool hasInterestRateCurve(const std::string& name) const;
    void removeInterestRateCurve(const std::string& name);
    std::vector<std::string> getInterestRateCurveNames() const;
    
    // Equity spots
    void addEquitySpot(const std::string& name, Real spot);
    void addEquitySpot(const std::string& name, const Date& asOfDate, Real spot);
    Real getEquitySpot(const std::string& name) const;
    bool hasEquitySpot(const std::string& name) const;
    void removeEquitySpot(const std::string& name);
    std::vector<std::string> getEquityNames() const;
    
    // FX rates
    void addFXRate(const std::string& currencyPair, Real rate);
    void addFXRate(const std::string& currencyPair, const Date& asOfDate, Real rate);
    Real getFXRate(const std::string& currencyPair) const;
    bool hasFXRate(const std::string& currencyPair) const;
    void removeFXRate(const std::string& currencyPair);
    std::vector<std::string> getFXPairs() const;
    
    // Dividend yields
    void addDividendYield(const std::string& name, Real yield);
    void addDividendYield(const std::string& name, const Date& asOfDate, Real yield);
    Real getDividendYield(const std::string& name) const;
    bool hasDividendYield(const std::string& name) const;
    void removeDividendYield(const std::string& name);
    std::vector<std::string> getDividendYieldNames() const;
    
    // Generic market data access
    void addMarketData(const std::string& name, std::shared_ptr<MarketData> data);
    std::shared_ptr<MarketData> getMarketData(const std::string& name) const;
    bool hasMarketData(const std::string& name) const;
    void removeMarketData(const std::string& name);
    std::vector<std::string> getAllMarketDataNames() const;
    
    // Discount factors and rates
    Real getDiscountFactor(const std::string& curveName, const Date& date) const;
    Real getDiscountFactor(const std::string& curveName, Real timeToMaturity) const;
    Real getRate(const std::string& curveName, const Date& date) const;
    Real getRate(const std::string& curveName, Real timeToMaturity) const;
    Real getForwardRate(const std::string& curveName, const Date& startDate, const Date& endDate) const;
    
    // Market data operations
    void bumpInterestRateCurve(const std::string& curveName, Real parallelShift);
    void bumpEquitySpot(const std::string& name, Real relativeShift);
    void bumpFXRate(const std::string& currencyPair, Real relativeShift);
    
    // Scenario analysis
    MultiMarketEnv getShiftedEnvironment(const std::map<std::string, Real>& curveShifts,
                                        const std::map<std::string, Real>& equityShifts = {},
                                        const std::map<std::string, Real>& fxShifts = {}) const;
    
    // Validation
    bool isValid() const;
    void validate() const;
    
    // Statistics
    Size getNumberOfCurves() const;
    Size getNumberOfEquities() const;
    Size getNumberOfFXRates() const;
    Size getTotalMarketDataItems() const;
    
    // Serialization
    std::string serialize() const;
    static MultiMarketEnv deserialize(const std::string& data);
    
    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const MultiMarketEnv& env);
    
    // Clear all data
    void clear();

private:
    Date valuationDate_;
    CurveCollection interestRateCurves_;
    std::map<std::string, std::shared_ptr<MarketData>> marketData_;
    
    // Helper methods
    template<typename T>
    std::shared_ptr<T> getMarketDataOfType(const std::string& name, MarketDataType expectedType) const;
    
    void validateCurveName(const std::string& name) const;
    void validateMarketDataName(const std::string& name) const;
};

// Market data factory
class MarketDataFactory {
public:
    static std::shared_ptr<MarketData> createMarketData(MarketDataType type, 
                                                       const std::string& name,
                                                       const Date& asOfDate,
                                                       const std::string& data);
    
    static std::shared_ptr<EquitySpot> createEquitySpot(const std::string& name,
                                                       const Date& asOfDate,
                                                       Real spot);
    
    static std::shared_ptr<FXRate> createFXRate(const std::string& currencyPair,
                                               const Date& asOfDate,
                                               Real rate);
    
    static std::shared_ptr<DividendYield> createDividendYield(const std::string& name,
                                                            const Date& asOfDate,
                                                            Real yield);
};

} // namespace QuantLib
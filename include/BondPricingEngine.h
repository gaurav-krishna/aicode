#pragma once

#include "Common.h"
#include "Date.h"
#include "FixedCouponBond.h"
#include "MultiMarketEnv.h"
#include "CashFlow.h"
#include <string>
#include <map>

namespace QuantLib {

// Bond pricing results structure
struct BondPricingResults {
    Real presentValue;
    Real cleanPrice;
    Real dirtyPrice;
    Real accruedInterest;
    Real yieldToMaturity;
    Real modifiedDuration;
    Real macaulayDuration;
    Real convexity;
    Real dv01; // Dollar value of 01 (1 basis point)
    Integer numberOfCashFlows;
    Date valuationDate;
    
    BondPricingResults() : presentValue(0.0), cleanPrice(0.0), dirtyPrice(0.0),
                          accruedInterest(0.0), yieldToMaturity(0.0),
                          modifiedDuration(0.0), macaulayDuration(0.0),
                          convexity(0.0), dv01(0.0), numberOfCashFlows(0) {}
    
    // Stream operator
    friend std::ostream& operator<<(std::ostream& os, const BondPricingResults& results);
};

// Yield calculation methods
enum class YieldCalculationMethod {
    NEWTON_RAPHSON,
    BISECTION,
    SECANT,
    BRENT
};

// Price types
enum class PriceType {
    CLEAN,
    DIRTY,
    PRESENT_VALUE
};

class BondPricingEngine {
public:
    // Constructors
    BondPricingEngine();
    explicit BondPricingEngine(const MultiMarketEnv& marketEnv);
    
    // Copy constructor and assignment
    BondPricingEngine(const BondPricingEngine& other) = default;
    BondPricingEngine& operator=(const BondPricingEngine& other) = default;
    
    // Market environment
    void setMarketEnvironment(const MultiMarketEnv& marketEnv);
    const MultiMarketEnv& getMarketEnvironment() const { return marketEnv_; }
    
    // Main pricing methods (asset-based)
    BondPricingResults priceBond(const FixedCouponBond& bond,
                                const Date& valuationDate) const;
    
    Real calculatePresentValue(const FixedCouponBond& bond,
                              const Date& valuationDate) const;
    
    Real calculateCleanPrice(const FixedCouponBond& bond,
                            const Date& valuationDate) const;
    
    Real calculateDirtyPrice(const FixedCouponBond& bond,
                            const Date& valuationDate) const;
    
    // Alternative pricing methods (curve-based for compatibility)
    BondPricingResults priceBondWithCurve(const FixedCouponBond& bond,
                                         const Date& valuationDate,
                                         const std::string& discountCurveName) const;
    
    Real calculatePresentValueWithCurve(const FixedCouponBond& bond,
                                       const Date& valuationDate,
                                       const std::string& discountCurveName) const;
    
    // Yield calculations
    Real calculateYieldToMaturity(const FixedCouponBond& bond,
                                 Real bondPrice,
                                 const Date& valuationDate,
                                 PriceType priceType = PriceType::CLEAN,
                                 YieldCalculationMethod method = YieldCalculationMethod::NEWTON_RAPHSON) const;
    
    Real calculateYieldFromPV(const FixedCouponBond& bond,
                             Real presentValue,
                             const Date& valuationDate) const;
    
    // Price from yield
    Real calculatePriceFromYield(const FixedCouponBond& bond,
                                Real yield,
                                const Date& valuationDate,
                                PriceType priceType = PriceType::CLEAN) const;
    
    // Duration and convexity calculations
    Real calculateModifiedDuration(const FixedCouponBond& bond,
                                  Real yield,
                                  const Date& valuationDate) const;
    
    Real calculateMacaulayDuration(const FixedCouponBond& bond,
                                  Real yield,
                                  const Date& valuationDate) const;
    
    Real calculateConvexity(const FixedCouponBond& bond,
                           Real yield,
                           const Date& valuationDate) const;
    
    Real calculateDV01(const FixedCouponBond& bond,
                      const Date& valuationDate) const;
    
    // Risk calculations using asset-associated market curves
    Real calculateCurveBasedDuration(const FixedCouponBond& bond,
                                    const Date& valuationDate,
                                    Real bumpSize = 0.0001) const; // 1bp
    
    Real calculateCurveBasedConvexity(const FixedCouponBond& bond,
                                     const Date& valuationDate,
                                     Real bumpSize = 0.0001) const;
    
    // Alternative risk calculations (curve-based)
    Real calculateDV01WithCurve(const FixedCouponBond& bond,
                               const Date& valuationDate,
                               const std::string& discountCurveName) const;
    
    Real calculateCurveBasedDurationWithCurve(const FixedCouponBond& bond,
                                             const Date& valuationDate,
                                             const std::string& discountCurveName,
                                             Real bumpSize = 0.0001) const;
    
    // Spread calculations (asset-based)
    Real calculateZSpread(const FixedCouponBond& bond,
                         Real marketPrice,
                         const Date& valuationDate,
                         PriceType priceType = PriceType::CLEAN) const;
    
    Real calculateOptionAdjustedSpread(const FixedCouponBond& bond,
                                      Real marketPrice,
                                      const Date& valuationDate) const;
    
    // Alternative spread calculations (curve-based)
    Real calculateZSpreadWithCurve(const FixedCouponBond& bond,
                                  Real marketPrice,
                                  const Date& valuationDate,
                                  const std::string& benchmarkCurveName,
                                  PriceType priceType = PriceType::CLEAN) const;
    
    // Cashflow analysis
    CashFlowSchedule generateBondCashFlows(const FixedCouponBond& bond,
                                          const Date& settlementDate) const;
    
    std::vector<Real> calculateCashFlowPresentValues(const FixedCouponBond& bond,
                                                    const Date& valuationDate) const;
    
    // Forward pricing (asset-based)
    Real calculateForwardPrice(const FixedCouponBond& bond,
                              const Date& valuationDate,
                              const Date& forwardDate) const;
    
    // Alternative methods (curve-based)
    std::vector<Real> calculateCashFlowPresentValuesWithCurve(const FixedCouponBond& bond,
                                                             const Date& valuationDate,
                                                             const std::string& discountCurveName) const;
    
    Real calculateForwardPriceWithCurve(const FixedCouponBond& bond,
                                       const Date& valuationDate,
                                       const Date& forwardDate,
                                       const std::string& discountCurveName) const;
    
    // Scenario analysis (asset-based)
    std::map<Real, Real> calculatePriceScenarios(const FixedCouponBond& bond,
                                                 const Date& valuationDate,
                                                 const std::vector<Real>& shiftScenarios) const;
    
    BondPricingResults calculateRiskSensitivities(const FixedCouponBond& bond,
                                                 const Date& valuationDate) const;
    
    // Alternative scenario analysis (curve-based)
    std::map<Real, Real> calculatePriceScenariosWithCurve(const FixedCouponBond& bond,
                                                         const Date& valuationDate,
                                                         const std::string& discountCurveName,
                                                         const std::vector<Real>& shiftScenarios) const;
    
    // Utilities
    Real accruedInterest(const FixedCouponBond& bond, const Date& settlementDate) const;
    Date settlementDate(const FixedCouponBond& bond, const Date& tradeDate) const;
    
    // Validation
    bool validateInputs(const FixedCouponBond& bond, const Date& valuationDate) const;
    
    // Configuration
    void setYieldCalculationMethod(YieldCalculationMethod method) { yieldMethod_ = method; }
    YieldCalculationMethod getYieldCalculationMethod() const { return yieldMethod_; }
    
    void setYieldTolerance(Real tolerance) { yieldTolerance_ = tolerance; }
    Real getYieldTolerance() const { return yieldTolerance_; }
    
    void setMaxIterations(Integer maxIter) { maxIterations_ = maxIter; }
    Integer getMaxIterations() const { return maxIterations_; }

private:
    MultiMarketEnv marketEnv_;
    YieldCalculationMethod yieldMethod_;
    Real yieldTolerance_;
    Integer maxIterations_;
    
    // Internal pricing helper methods
    Real calculatePresentValueWithCurve(const FixedCouponBond& bond,
                                       const Date& valuationDate,
                                       const InterestRateCurve& curve) const;
    
    Real calculatePresentValueWithYield(const FixedCouponBond& bond,
                                       Real yield,
                                       const Date& valuationDate) const;
    
    // Yield calculation implementations
    Real yieldNewtonRaphson(const FixedCouponBond& bond,
                           Real targetPrice,
                           const Date& valuationDate,
                           PriceType priceType) const;
    
    Real yieldBisection(const FixedCouponBond& bond,
                       Real targetPrice,
                       const Date& valuationDate,
                       PriceType priceType) const;
    
    Real yieldSecant(const FixedCouponBond& bond,
                    Real targetPrice,
                    const Date& valuationDate,
                    PriceType priceType) const;
    
    Real yieldBrent(const FixedCouponBond& bond,
                   Real targetPrice,
                   const Date& valuationDate,
                   PriceType priceType) const;
    
    // Helper functions for yield calculations
    Real priceFromYieldFunction(const FixedCouponBond& bond,
                               Real yield,
                               const Date& valuationDate,
                               PriceType priceType) const;
    
    Real priceDerivativeWithRespectToYield(const FixedCouponBond& bond,
                                          Real yield,
                                          const Date& valuationDate,
                                          PriceType priceType) const;
    
    // Validation helpers
    void validateBond(const FixedCouponBond& bond) const;
    void validateMarketData(const std::string& curveName) const;
    void validateDate(const Date& date) const;
};

// Static utility functions for bond calculations
class BondCalculationUtils {
public:
    // Static pricing functions (without market environment)
    static Real calculatePresentValueFromYield(const FixedCouponBond& bond,
                                              Real yield,
                                              const Date& valuationDate);
    
    static Real calculateCleanPriceFromYield(const FixedCouponBond& bond,
                                            Real yield,
                                            const Date& valuationDate);
    
    static Real calculateDirtyPriceFromYield(const FixedCouponBond& bond,
                                            Real yield,
                                            const Date& valuationDate);
    
    static Real calculateModifiedDurationFromYield(const FixedCouponBond& bond,
                                                  Real yield,
                                                  const Date& valuationDate);
    
    static Real calculateMacaulayDurationFromYield(const FixedCouponBond& bond,
                                                  Real yield,
                                                  const Date& valuationDate);
    
    static Real calculateConvexityFromYield(const FixedCouponBond& bond,
                                           Real yield,
                                           const Date& valuationDate);
    
    // Accrued interest calculation
    static Real calculateAccruedInterest(const FixedCouponBond& bond,
                                        const Date& settlementDate);
    
    // Time calculations
    static Real timeToMaturity(const FixedCouponBond& bond,
                              const Date& valuationDate);
    
    static std::vector<Real> timesToCashFlows(const FixedCouponBond& bond,
                                             const Date& valuationDate);
};

} // namespace QuantLib
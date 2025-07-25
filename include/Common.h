#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cmath>

namespace QuantLib {

// Common types
using Real = double;
using Integer = int;
using Size = std::size_t;

// Day count conventions
enum class DayCountConvention {
    ACT_360,
    ACT_365,
    THIRTY_360,
    ACT_ACT_ISDA
};

// Frequency
enum class Frequency {
    ANNUAL = 1,
    SEMI_ANNUAL = 2,
    QUARTERLY = 4,
    MONTHLY = 12
};

// Business day conventions
enum class BusinessDayConvention {
    FOLLOWING,
    MODIFIED_FOLLOWING,
    PRECEDING,
    UNADJUSTED
};

// Forward declarations
class Date;
class CashFlow;
class FixedCouponBond;
class MultiMarketEnv;
class BondPricingEngine;
class InterestRateCurve;

// Utility functions
Real yearFraction(const Date& start, const Date& end, DayCountConvention dayCount);
Real compoundFactor(Real rate, Real time, Frequency compounding = Frequency::ANNUAL);
Real discountFactor(Real rate, Real time, Frequency compounding = Frequency::ANNUAL);

} // namespace QuantLib
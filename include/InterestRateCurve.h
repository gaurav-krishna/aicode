#pragma once

#include "Common.h"
#include "Date.h"
#include <vector>
#include <map>
#include <string>

namespace QuantLib {

struct RatePoint {
    Date date;
    Real rate;
    Real time; // Year fraction from base date
    
    RatePoint(const Date& d, Real r, Real t) : date(d), rate(r), time(t) {}
};

class InterestRateCurve {
public:
    // Constructors
    InterestRateCurve();
    InterestRateCurve(const std::string& curveName, const Date& baseDate);
    InterestRateCurve(const std::string& curveName, const Date& baseDate,
                     const std::vector<Date>& dates, const std::vector<Real>& rates);
    
    // Copy constructor and assignment
    InterestRateCurve(const InterestRateCurve& other) = default;
    InterestRateCurve& operator=(const InterestRateCurve& other) = default;
    
    // Curve building
    void addRate(const Date& date, Real rate);
    void addRate(Real timeToMaturity, Real rate);
    void setRates(const std::vector<Date>& dates, const std::vector<Real>& rates);
    void setRates(const std::vector<Real>& times, const std::vector<Real>& rates);
    
    // Rate and discount factor queries
    Real getRate(const Date& date) const;
    Real getRate(Real timeToMaturity) const;
    Real getDiscountFactor(const Date& date) const;
    Real getDiscountFactor(Real timeToMaturity) const;
    Real getForwardRate(const Date& startDate, const Date& endDate) const;
    Real getForwardRate(Real startTime, Real endTime) const;
    
    // Getters
    const std::string& name() const { return curveName_; }
    const Date& baseDate() const { return baseDate_; }
    const std::vector<RatePoint>& ratePoints() const { return ratePoints_; }
    DayCountConvention dayCountConvention() const { return dayCount_; }
    
    // Setters
    void setDayCountConvention(DayCountConvention dayCount) { dayCount_ = dayCount; }
    void setBaseDate(const Date& baseDate);
    
    // Curve operations
    void shift(Real parallelShift); // Parallel shift in basis points
    void bump(const Date& date, Real bumpSize); // Bump specific point
    InterestRateCurve getShiftedCurve(Real parallelShift) const;
    
    // Validation
    bool isValid() const;
    void validate() const;
    
    // Interpolation methods
    enum class InterpolationMethod {
        LINEAR,
        LOG_LINEAR,
        CUBIC_SPLINE,
        FLAT_FORWARD
    };
    
    void setInterpolationMethod(InterpolationMethod method) { interpolationMethod_ = method; }
    InterpolationMethod getInterpolationMethod() const { return interpolationMethod_; }
    
    // Curve metrics
    Real getMaxMaturity() const;
    Date getMaxMaturityDate() const;
    Size getNumberOfPoints() const { return ratePoints_.size(); }
    
    // Serialization
    std::string serialize() const;
    static InterestRateCurve deserialize(const std::string& data);
    
    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const InterestRateCurve& curve);

private:
    std::string curveName_;
    Date baseDate_;
    std::vector<RatePoint> ratePoints_;
    DayCountConvention dayCount_;
    InterpolationMethod interpolationMethod_;
    
    // Internal interpolation methods
    Real interpolateRate(Real timeToMaturity) const;
    Real linearInterpolation(Real time) const;
    Real logLinearInterpolation(Real time) const;
    Real flatForwardInterpolation(Real time) const;
    
    // Utility functions
    void sortRatePoints();
    Real timeFromBase(const Date& date) const;
    Size findLowerBound(Real time) const;
};

// Curve collection for managing multiple curves
class CurveCollection {
public:
    CurveCollection() = default;
    
    // Add/remove curves
    void addCurve(const std::string& name, const InterestRateCurve& curve);
    void removeCurve(const std::string& name);
    
    // Get curves
    const InterestRateCurve& getCurve(const std::string& name) const;
    InterestRateCurve& getCurve(const std::string& name);
    bool hasCurve(const std::string& name) const;
    
    // List available curves
    std::vector<std::string> getCurveNames() const;
    Size getNumberOfCurves() const { return curves_.size(); }
    
    // Curve operations
    void shiftAllCurves(Real parallelShift);
    CurveCollection getShiftedCollection(Real parallelShift) const;
    
    // Serialization
    std::string serialize() const;
    static CurveCollection deserialize(const std::string& data);

private:
    std::map<std::string, InterestRateCurve> curves_;
};

} // namespace QuantLib
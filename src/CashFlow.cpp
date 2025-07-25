#include "CashFlow.h"
#include "InterestRateCurve.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace QuantLib {

// CashFlow implementation
CashFlow::CashFlow() : amount_(0.0) {}

CashFlow::CashFlow(const Date& date, Real amount, const std::string& type)
    : date_(date), amount_(amount), type_(type) {}

Real CashFlow::presentValue(Real discountRate, const Date& valuationDate, 
                           DayCountConvention dayCount) const {
    if (date_ <= valuationDate) return 0.0;
    
    Real timeToFlow = yearFraction(valuationDate, date_, dayCount);
    Real discountFactor = std::exp(-discountRate * timeToFlow);
    return amount_ * discountFactor;
}

Real CashFlow::timeToFlow(const Date& valuationDate, DayCountConvention dayCount) const {
    return yearFraction(valuationDate, date_, dayCount);
}

bool CashFlow::operator<(const CashFlow& other) const {
    return date_ < other.date_;
}

bool CashFlow::operator==(const CashFlow& other) const {
    return date_ == other.date_ && 
           std::abs(amount_ - other.amount_) < 1e-10 && 
           type_ == other.type_;
}

std::string CashFlow::serialize() const {
    std::ostringstream oss;
    oss << date_.serialize() << "|" << amount_ << "|" << type_;
    return oss.str();
}

CashFlow CashFlow::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string dateStr, amountStr, type;
    
    if (std::getline(iss, dateStr, '|') && 
        std::getline(iss, amountStr, '|') && 
        std::getline(iss, type)) {
        Date date = Date::deserialize(dateStr);
        Real amount = std::stod(amountStr);
        return CashFlow(date, amount, type);
    }
    
    throw std::invalid_argument("Invalid cashflow serialization format");
}

std::ostream& operator<<(std::ostream& os, const CashFlow& cf) {
    os << "CashFlow(" << cf.date_ << ", " << cf.amount_ << ", " << cf.type_ << ")";
    return os;
}

// CashFlowSchedule implementation
CashFlowSchedule::CashFlowSchedule(const std::vector<CashFlow>& flows)
    : cashFlows_(flows) {
    sort();
}

void CashFlowSchedule::addCashFlow(const CashFlow& flow) {
    cashFlows_.push_back(flow);
    sort();
}

void CashFlowSchedule::addCashFlow(const Date& date, Real amount, const std::string& type) {
    addCashFlow(CashFlow(date, amount, type));
}

std::vector<CashFlow> CashFlowSchedule::getCashFlowsByType(const std::string& type) const {
    std::vector<CashFlow> result;
    for (const auto& cf : cashFlows_) {
        if (cf.type() == type) {
            result.push_back(cf);
        }
    }
    return result;
}

Real CashFlowSchedule::presentValue(Real discountRate, const Date& valuationDate,
                                   DayCountConvention dayCount) const {
    Real pv = 0.0;
    for (const auto& cf : cashFlows_) {
        pv += cf.presentValue(discountRate, valuationDate, dayCount);
    }
    return pv;
}

Real CashFlowSchedule::netPresentValue(const InterestRateCurve& curve, const Date& valuationDate) const {
    Real npv = 0.0;
    for (const auto& cf : cashFlows_) {
        if (cf.date() > valuationDate) {
            Real discountFactor = curve.getDiscountFactor(cf.date());
            npv += cf.amount() * discountFactor;
        }
    }
    return npv;
}

Real CashFlowSchedule::modifiedDuration(Real yield, const Date& valuationDate,
                                       DayCountConvention dayCount) const {
    Real pv = presentValue(yield, valuationDate, dayCount);
    if (pv == 0.0) return 0.0;
    
    Real weightedTime = 0.0;
    for (const auto& cf : cashFlows_) {
        if (cf.date() > valuationDate) {
            Real time = cf.timeToFlow(valuationDate, dayCount);
            Real cfPV = cf.presentValue(yield, valuationDate, dayCount);
            weightedTime += time * cfPV;
        }
    }
    
    return weightedTime / pv;
}

Real CashFlowSchedule::convexity(Real yield, const Date& valuationDate,
                                DayCountConvention dayCount) const {
    Real pv = presentValue(yield, valuationDate, dayCount);
    if (pv == 0.0) return 0.0;
    
    Real weightedTimeSquared = 0.0;
    for (const auto& cf : cashFlows_) {
        if (cf.date() > valuationDate) {
            Real time = cf.timeToFlow(valuationDate, dayCount);
            Real cfPV = cf.presentValue(yield, valuationDate, dayCount);
            weightedTimeSquared += time * time * cfPV;
        }
    }
    
    return weightedTimeSquared / pv;
}

void CashFlowSchedule::sort() {
    std::sort(cashFlows_.begin(), cashFlows_.end());
}

CashFlowSchedule CashFlowSchedule::getFutureCashFlows(const Date& cutoffDate) const {
    std::vector<CashFlow> futureCFs;
    for (const auto& cf : cashFlows_) {
        if (cf.date() > cutoffDate) {
            futureCFs.push_back(cf);
        }
    }
    return CashFlowSchedule(futureCFs);
}

std::string CashFlowSchedule::serialize() const {
    std::ostringstream oss;
    oss << cashFlows_.size();
    for (const auto& cf : cashFlows_) {
        oss << "\n" << cf.serialize();
    }
    return oss.str();
}

CashFlowSchedule CashFlowSchedule::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string line;
    
    if (!std::getline(iss, line)) {
        throw std::invalid_argument("Invalid cashflow schedule format");
    }
    
    Size numCashFlows = std::stoul(line);
    std::vector<CashFlow> cashFlows;
    
    for (Size i = 0; i < numCashFlows; ++i) {
        if (std::getline(iss, line)) {
            cashFlows.push_back(CashFlow::deserialize(line));
        }
    }
    
    return CashFlowSchedule(cashFlows);
}

} // namespace QuantLib
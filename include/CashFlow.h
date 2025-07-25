#pragma once

#include "Common.h"
#include "Date.h"
#include <iostream>
#include <vector>

namespace QuantLib {

class CashFlow {
public:
    // Constructors
    CashFlow();
    CashFlow(const Date& date, Real amount, const std::string& type = "");
    
    // Copy constructor and assignment
    CashFlow(const CashFlow& other) = default;
    CashFlow& operator=(const CashFlow& other) = default;
    
    // Getters
    const Date& date() const { return date_; }
    Real amount() const { return amount_; }
    const std::string& type() const { return type_; }
    
    // Setters
    void setDate(const Date& date) { date_ = date; }
    void setAmount(Real amount) { amount_ = amount; }
    void setType(const std::string& type) { type_ = type; }
    
    // Present value calculation
    Real presentValue(Real discountRate, const Date& valuationDate, 
                     DayCountConvention dayCount = DayCountConvention::ACT_365) const;
    
    // Time to cashflow
    Real timeToFlow(const Date& valuationDate, 
                   DayCountConvention dayCount = DayCountConvention::ACT_365) const;
    
    // Comparison operators (by date)
    bool operator<(const CashFlow& other) const;
    bool operator==(const CashFlow& other) const;
    
    // Serialization
    std::string serialize() const;
    static CashFlow deserialize(const std::string& data);
    
    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const CashFlow& cf);

private:
    Date date_;
    Real amount_;
    std::string type_; // "COUPON", "PRINCIPAL", "CALL", etc.
};

// CashFlow collection utilities
class CashFlowSchedule {
public:
    CashFlowSchedule() = default;
    explicit CashFlowSchedule(const std::vector<CashFlow>& flows);
    
    // Add cashflows
    void addCashFlow(const CashFlow& flow);
    void addCashFlow(const Date& date, Real amount, const std::string& type = "");
    
    // Access cashflows
    const std::vector<CashFlow>& cashFlows() const { return cashFlows_; }
    Size size() const { return cashFlows_.size(); }
    bool empty() const { return cashFlows_.empty(); }
    
    // Get cashflows by type
    std::vector<CashFlow> getCashFlowsByType(const std::string& type) const;
    
    // Present value calculations
    Real presentValue(Real discountRate, const Date& valuationDate,
                     DayCountConvention dayCount = DayCountConvention::ACT_365) const;
    
    Real netPresentValue(const InterestRateCurve& curve, const Date& valuationDate) const;
    
    // Duration and convexity
    Real modifiedDuration(Real yield, const Date& valuationDate,
                         DayCountConvention dayCount = DayCountConvention::ACT_365) const;
    
    Real convexity(Real yield, const Date& valuationDate,
                  DayCountConvention dayCount = DayCountConvention::ACT_365) const;
    
    // Sort cashflows by date
    void sort();
    
    // Filter future cashflows
    CashFlowSchedule getFutureCashFlows(const Date& cutoffDate) const;
    
    // Serialization
    std::string serialize() const;
    static CashFlowSchedule deserialize(const std::string& data);
    
    // Iterator support
    std::vector<CashFlow>::const_iterator begin() const { return cashFlows_.begin(); }
    std::vector<CashFlow>::const_iterator end() const { return cashFlows_.end(); }

private:
    std::vector<CashFlow> cashFlows_;
};

} // namespace QuantLib
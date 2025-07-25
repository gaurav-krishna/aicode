#pragma once

#include "Common.h"
#include "Date.h"
#include "CashFlow.h"
#include <string>
#include <iostream>

namespace QuantLib {

class FixedCouponBond {
public:
    // Constructors
    FixedCouponBond();
    FixedCouponBond(const std::string& identifier,
                   const Date& issueDate,
                   const Date& maturityDate,
                   Real faceValue,
                   Real couponRate,
                   Frequency paymentFrequency = Frequency::SEMI_ANNUAL,
                   DayCountConvention dayCount = DayCountConvention::ACT_365,
                   BusinessDayConvention businessDayConvention = BusinessDayConvention::MODIFIED_FOLLOWING);
    
    // Copy constructor and assignment
    FixedCouponBond(const FixedCouponBond& other) = default;
    FixedCouponBond& operator=(const FixedCouponBond& other) = default;
    
    // Destructor
    ~FixedCouponBond() = default;
    
    // Basic getters
    const std::string& identifier() const { return identifier_; }
    const Date& issueDate() const { return issueDate_; }
    const Date& maturityDate() const { return maturityDate_; }
    Real faceValue() const { return faceValue_; }
    Real couponRate() const { return couponRate_; }
    Frequency paymentFrequency() const { return paymentFrequency_; }
    DayCountConvention dayCountConvention() const { return dayCount_; }
    BusinessDayConvention businessDayConvention() const { return businessDayConvention_; }
    
    // Basic setters
    void setIdentifier(const std::string& identifier) { identifier_ = identifier; }
    void setIssueDate(const Date& date) { issueDate_ = date; }
    void setMaturityDate(const Date& date) { maturityDate_ = date; }
    void setFaceValue(Real faceValue) { faceValue_ = faceValue; }
    void setCouponRate(Real couponRate) { couponRate_ = couponRate; }
    void setPaymentFrequency(Frequency frequency) { paymentFrequency_ = frequency; }
    void setDayCountConvention(DayCountConvention dayCount) { dayCount_ = dayCount; }
    void setBusinessDayConvention(BusinessDayConvention convention) { businessDayConvention_ = convention; }
    
    // Bond characteristics
    Real periodicCouponRate() const;
    Real periodicCouponAmount() const;
    Integer numberOfPayments() const;
    Integer numberOfPayments(const Date& settlementDate) const;
    Real timeToMaturity(const Date& valuationDate) const;
    
    // Accrued interest
    Real accruedInterest(const Date& settlementDate) const;
    Real daysSinceLastCoupon(const Date& settlementDate) const;
    Real daysToNextCoupon(const Date& settlementDate) const;
    Date lastCouponDate(const Date& settlementDate) const;
    Date nextCouponDate(const Date& settlementDate) const;
    
    // Cashflow generation
    CashFlowSchedule generateCashFlows() const;
    CashFlowSchedule generateCashFlows(const Date& settlementDate) const;
    std::vector<Date> getCouponDates() const;
    std::vector<Date> getCouponDates(const Date& settlementDate) const;
    
    // Bond validation
    bool isValid() const;
    void validate() const;
    bool isActive(const Date& date) const;
    bool hasMatured(const Date& date) const;
    
    // Additional bond properties
    const std::string& currency() const { return currency_; }
    void setCurrency(const std::string& currency) { currency_ = currency; }
    
    const std::string& issuer() const { return issuer_; }
    void setIssuer(const std::string& issuer) { issuer_ = issuer; }
    
    const std::string& sector() const { return sector_; }
    void setSector(const std::string& sector) { sector_ = sector; }
    
    const std::string& rating() const { return rating_; }
    void setRating(const std::string& rating) { rating_ = rating; }
    
    bool isCallable() const { return isCallable_; }
    void setCallable(bool callable) { isCallable_ = callable; }
    
    const Date& firstCallDate() const { return firstCallDate_; }
    void setFirstCallDate(const Date& date) { firstCallDate_ = date; }
    
    Real callPrice() const { return callPrice_; }
    void setCallPrice(Real price) { callPrice_ = price; }
    
    // Settlement and calendar
    Integer settlementDays() const { return settlementDays_; }
    void setSettlementDays(Integer days) { settlementDays_ = days; }
    Date settlementDate(const Date& tradeDate) const;
    
    // Comparison operators
    bool operator==(const FixedCouponBond& other) const;
    bool operator!=(const FixedCouponBond& other) const;
    
    // Serialization to/from text stream
    std::string serialize() const;
    static FixedCouponBond deserialize(const std::string& data);
    
    // JSON serialization
    std::string toJSON() const;
    static FixedCouponBond fromJSON(const std::string& json);
    
    // XML serialization
    std::string toXML() const;
    static FixedCouponBond fromXML(const std::string& xml);
    
    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const FixedCouponBond& bond);
    friend std::istream& operator>>(std::istream& is, FixedCouponBond& bond);
    
    // Hash function support
    std::size_t hash() const;

private:
    // Core bond properties
    std::string identifier_;
    Date issueDate_;
    Date maturityDate_;
    Real faceValue_;
    Real couponRate_;
    Frequency paymentFrequency_;
    DayCountConvention dayCount_;
    BusinessDayConvention businessDayConvention_;
    
    // Additional properties
    std::string currency_;
    std::string issuer_;
    std::string sector_;
    std::string rating_;
    
    // Call features
    bool isCallable_;
    Date firstCallDate_;
    Real callPrice_;
    
    // Settlement
    Integer settlementDays_;
    
    // Helper methods
    Date adjustBusinessDay(const Date& date) const;
    std::vector<Date> generateCouponSchedule() const;
    std::vector<Date> generateCouponSchedule(const Date& startDate) const;
    Date getNextCouponDate(const Date& fromDate) const;
    Date getPreviousCouponDate(const Date& fromDate) const;
    Real calculateAccruedInterest(const Date& lastCouponDate, const Date& settlementDate) const;
};

// Hash specialization for std::unordered_map support
struct FixedCouponBondHash {
    std::size_t operator()(const FixedCouponBond& bond) const {
        return bond.hash();
    }
};

// Bond factory for creating common bond types
class BondFactory {
public:
    // US Treasury bonds
    static FixedCouponBond createUST(const std::string& identifier,
                                   const Date& issueDate,
                                   const Date& maturityDate,
                                   Real couponRate);
    
    // Corporate bonds
    static FixedCouponBond createCorporateBond(const std::string& identifier,
                                             const std::string& issuer,
                                             const Date& issueDate,
                                             const Date& maturityDate,
                                             Real couponRate,
                                             const std::string& rating = "");
    
    // Municipal bonds
    static FixedCouponBond createMunicipalBond(const std::string& identifier,
                                             const std::string& issuer,
                                             const Date& issueDate,
                                             const Date& maturityDate,
                                             Real couponRate);
    
    // International bonds
    static FixedCouponBond createInternationalBond(const std::string& identifier,
                                                  const std::string& issuer,
                                                  const Date& issueDate,
                                                  const Date& maturityDate,
                                                  Real couponRate,
                                                  const std::string& currency,
                                                  Frequency frequency = Frequency::ANNUAL);
};

} // namespace QuantLib
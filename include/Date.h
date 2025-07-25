#pragma once

#include "Common.h"
#include <iostream>
#include <string>

namespace QuantLib {

class Date {
public:
    // Constructors
    Date();
    Date(Integer day, Integer month, Integer year);
    Date(const std::string& dateString); // Format: "YYYY-MM-DD"
    
    // Copy constructor and assignment
    Date(const Date& other) = default;
    Date& operator=(const Date& other) = default;
    
    // Getters
    Integer day() const { return day_; }
    Integer month() const { return month_; }
    Integer year() const { return year_; }
    Integer serialNumber() const;
    
    // Date arithmetic
    Date operator+(Integer days) const;
    Date operator-(Integer days) const;
    Integer operator-(const Date& other) const; // Days between dates
    
    // Comparison operators
    bool operator==(const Date& other) const;
    bool operator!=(const Date& other) const;
    bool operator<(const Date& other) const;
    bool operator<=(const Date& other) const;
    bool operator>(const Date& other) const;
    bool operator>=(const Date& other) const;
    
    // Utility functions
    bool isLeapYear() const;
    Integer dayOfYear() const;
    std::string toString() const;
    
    // Static functions
    static Date today();
    static bool isValid(Integer day, Integer month, Integer year);
    
    // Business day adjustment
    Date adjust(BusinessDayConvention convention) const;
    
    // Serialization
    std::string serialize() const;
    static Date deserialize(const std::string& data);
    
    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const Date& date);
    friend std::istream& operator>>(std::istream& is, Date& date);

private:
    Integer day_;
    Integer month_;
    Integer year_;
    
    void validateDate() const;
    Integer calculateSerialNumber() const;
    static Integer daysInMonth(Integer month, Integer year);
};

} // namespace QuantLib
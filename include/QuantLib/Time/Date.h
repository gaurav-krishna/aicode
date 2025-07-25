#pragma once

#include "../Types/Quantity.h"
#include <chrono>
#include <string>
#include <iostream>
#include <optional>

namespace QuantLib {

// Date class using std::chrono for better performance and standard compliance
class Date {
public:
    using days = std::chrono::days;
    using sys_days = std::chrono::sys_days;
    using year_month_day = std::chrono::year_month_day;

    // Constructors
    Date() : date_(std::chrono::floor<days>(std::chrono::system_clock::now())) {}
    
    Date(int year, int month, int day) 
        : date_(sys_days{std::chrono::year{year}/std::chrono::month{month}/std::chrono::day{day}}) {}
    
    Date(const std::chrono::year_month_day& ymd) 
        : date_(sys_days{ymd}) {}
    
    explicit Date(sys_days sd) : date_(sd) {}
    
    explicit Date(const std::string& dateStr); // "YYYY-MM-DD" format

    // Factory methods
    static Date today() {
        return Date();
    }

    static Date fromSerial(int serial) {
        // Convert from Excel-style serial number (days since 1900-01-01)
        constexpr auto epoch = sys_days{std::chrono::year{1900}/std::chrono::January/1};
        return Date(epoch + days{serial});
    }

    static std::optional<Date> tryParse(const std::string& dateStr) noexcept;

    // Accessors
    int year() const {
        return static_cast<int>(std::chrono::year_month_day{date_}.year());
    }

    int month() const {
        return static_cast<unsigned>(std::chrono::year_month_day{date_}.month());
    }

    int day() const {
        return static_cast<unsigned>(std::chrono::year_month_day{date_}.day());
    }

    sys_days sys_days_value() const { return date_; }

    int serialNumber() const {
        constexpr auto epoch = sys_days{std::chrono::year{1900}/std::chrono::January/1};
        return (date_ - epoch).count();
    }

    std::chrono::weekday weekday() const {
        return std::chrono::weekday{date_};
    }

    // Arithmetic operations
    Date operator+(int days) const {
        return Date(date_ + std::chrono::days{days});
    }

    Date operator-(int days) const {
        return Date(date_ - std::chrono::days{days});
    }

    Date& operator+=(int days) {
        date_ += std::chrono::days{days};
        return *this;
    }

    Date& operator-=(int days) {
        date_ -= std::chrono::days{days};
        return *this;
    }

    int operator-(const Date& other) const {
        return (date_ - other.date_).count();
    }

    // Comparison operators
    bool operator==(const Date& other) const { return date_ == other.date_; }
    bool operator!=(const Date& other) const { return date_ != other.date_; }
    bool operator<(const Date& other) const { return date_ < other.date_; }
    bool operator<=(const Date& other) const { return date_ <= other.date_; }
    bool operator>(const Date& other) const { return date_ > other.date_; }
    bool operator>=(const Date& other) const { return date_ >= other.date_; }

    // Utilities
    bool isWeekend() const {
        auto wd = weekday();
        return wd == std::chrono::Saturday || wd == std::chrono::Sunday;
    }

    bool isLeapYear() const {
        auto y = std::chrono::year_month_day{date_}.year();
        return y.is_leap();
    }

    Date endOfMonth() const {
        auto ymd = std::chrono::year_month_day{date_};
        auto last_day = ymd.year()/ymd.month()/std::chrono::last;
        return Date(sys_days{last_day});
    }

    Date beginningOfMonth() const {
        auto ymd = std::chrono::year_month_day{date_};
        return Date(static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()), 1);
    }

    // String representation
    std::string toString() const;
    std::string toString(const std::string& format) const;

    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const Date& date);
    friend std::istream& operator>>(std::istream& is, Date& date);

private:
    sys_days date_;
};

// Date range class
class DateRange {
public:
    DateRange(const Date& start, const Date& end) : start_(start), end_(end) {
        if (start_ > end_) {
            throw std::invalid_argument("Start date must be <= end date");
        }
    }

    const Date& start() const { return start_; }
    const Date& end() const { return end_; }

    bool contains(const Date& date) const {
        return date >= start_ && date <= end_;
    }

    int length() const {
        return end_ - start_ + 1;
    }

    class iterator {
    public:
        iterator(const Date& date) : current_(date) {}
        
        Date operator*() const { return current_; }
        iterator& operator++() { current_ += 1; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
        bool operator==(const iterator& other) const { return current_ == other.current_; }
        bool operator!=(const iterator& other) const { return !(*this == other); }

    private:
        Date current_;
    };

    iterator begin() const { return iterator(start_); }
    iterator end() const { return iterator(end_ + 1); }

private:
    Date start_;
    Date end_;
};

// Time period representation
class Period {
public:
    enum Unit { Days, Weeks, Months, Years };

    Period(int n, Unit unit) : length_(n), unit_(unit) {}

    int length() const { return length_; }
    Unit unit() const { return unit_; }

    Date addTo(const Date& date) const;
    Date subtractFrom(const Date& date) const;

    // Arithmetic with dates
    friend Date operator+(const Date& date, const Period& period) {
        return period.addTo(date);
    }

    friend Date operator-(const Date& date, const Period& period) {
        return period.subtractFrom(date);
    }

    // Comparison
    bool operator==(const Period& other) const;
    bool operator!=(const Period& other) const { return !(*this == other); }

    // String representation
    std::string toString() const;

private:
    int length_;
    Unit unit_;
};

// Period literals
namespace literals {
    Period operator""_days(unsigned long long n) {
        return Period(static_cast<int>(n), Period::Days);
    }

    Period operator""_weeks(unsigned long long n) {
        return Period(static_cast<int>(n), Period::Weeks);
    }

    Period operator""_months(unsigned long long n) {
        return Period(static_cast<int>(n), Period::Months);
    }

    Period operator""_years(unsigned long long n) {
        return Period(static_cast<int>(n), Period::Years);
    }
}

// Day count conventions
enum class DayCountConvention {
    Actual360,
    Actual365,
    Thirty360,
    ActualActual
};

// Calculate year fraction between two dates
Time yearFraction(const Date& start, const Date& end, DayCountConvention dayCount);

} // namespace QuantLib
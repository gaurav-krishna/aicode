#include "Date.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace QuantLib {

// Static data for day calculations
static const Integer daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const Integer daysInMonthLeap[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

Date::Date() {
    // Initialize to current date
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    day_ = tm->tm_mday;
    month_ = tm->tm_mon + 1;
    year_ = tm->tm_year + 1900;
}

Date::Date(Integer day, Integer month, Integer year)
    : day_(day), month_(month), year_(year) {
    validateDate();
}

Date::Date(const std::string& dateString) {
    // Parse "YYYY-MM-DD" format
    if (dateString.length() != 10 || dateString[4] != '-' || dateString[7] != '-') {
        throw std::invalid_argument("Invalid date format. Expected YYYY-MM-DD");
    }
    
    try {
        year_ = std::stoi(dateString.substr(0, 4));
        month_ = std::stoi(dateString.substr(5, 2));
        day_ = std::stoi(dateString.substr(8, 2));
        validateDate();
    } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid date string: " + dateString);
    }
}

Integer Date::serialNumber() const {
    return calculateSerialNumber();
}

Date Date::operator+(Integer days) const {
    Integer serial = calculateSerialNumber() + days;
    
    // Convert back to date
    // Using a simple algorithm for demonstration
    // In practice, you'd want a more efficient implementation
    Date result(1, 1, 1900);
    while (serial > 365) {
        if (result.isLeapYear()) {
            if (serial > 366) {
                serial -= 366;
                result.year_++;
            } else {
                break;
            }
        } else {
            serial -= 365;
            result.year_++;
        }
    }
    
    const Integer* monthDays = result.isLeapYear() ? daysInMonthLeap : daysInMonth;
    result.month_ = 1;
    while (serial > monthDays[result.month_ - 1]) {
        serial -= monthDays[result.month_ - 1];
        result.month_++;
    }
    result.day_ = serial;
    
    return result;
}

Date Date::operator-(Integer days) const {
    return *this + (-days);
}

Integer Date::operator-(const Date& other) const {
    return calculateSerialNumber() - other.calculateSerialNumber();
}

bool Date::operator==(const Date& other) const {
    return day_ == other.day_ && month_ == other.month_ && year_ == other.year_;
}

bool Date::operator!=(const Date& other) const {
    return !(*this == other);
}

bool Date::operator<(const Date& other) const {
    if (year_ != other.year_) return year_ < other.year_;
    if (month_ != other.month_) return month_ < other.month_;
    return day_ < other.day_;
}

bool Date::operator<=(const Date& other) const {
    return *this < other || *this == other;
}

bool Date::operator>(const Date& other) const {
    return !(*this <= other);
}

bool Date::operator>=(const Date& other) const {
    return !(*this < other);
}

bool Date::isLeapYear() const {
    return (year_ % 4 == 0 && year_ % 100 != 0) || (year_ % 400 == 0);
}

Integer Date::dayOfYear() const {
    Integer dayCount = day_;
    const Integer* monthDays = isLeapYear() ? daysInMonthLeap : daysInMonth;
    
    for (Integer i = 0; i < month_ - 1; ++i) {
        dayCount += monthDays[i];
    }
    
    return dayCount;
}

std::string Date::toString() const {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << year_ << "-"
        << std::setw(2) << month_ << "-" << std::setw(2) << day_;
    return oss.str();
}

Date Date::today() {
    return Date(); // Default constructor gives current date
}

bool Date::isValid(Integer day, Integer month, Integer year) {
    if (year < 1900 || year > 2200) return false;
    if (month < 1 || month > 12) return false;
    
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    Integer maxDays = isLeap ? daysInMonthLeap[month - 1] : daysInMonth[month - 1];
    
    return day >= 1 && day <= maxDays;
}

Date Date::adjust(BusinessDayConvention convention) const {
    // Simplified business day adjustment
    // In practice, you'd need a proper calendar
    Date adjusted = *this;
    
    switch (convention) {
        case BusinessDayConvention::FOLLOWING:
            // Move to next business day if weekend
            while (adjusted.dayOfWeek() == 0 || adjusted.dayOfWeek() == 6) {
                adjusted = adjusted + 1;
            }
            break;
        case BusinessDayConvention::PRECEDING:
            // Move to previous business day if weekend
            while (adjusted.dayOfWeek() == 0 || adjusted.dayOfWeek() == 6) {
                adjusted = adjusted - 1;
            }
            break;
        case BusinessDayConvention::MODIFIED_FOLLOWING:
            adjusted = adjust(BusinessDayConvention::FOLLOWING);
            if (adjusted.month() != month_) {
                adjusted = adjust(BusinessDayConvention::PRECEDING);
            }
            break;
        case BusinessDayConvention::UNADJUSTED:
        default:
            break;
    }
    
    return adjusted;
}

std::string Date::serialize() const {
    return toString();
}

Date Date::deserialize(const std::string& data) {
    return Date(data);
}

void Date::validateDate() const {
    if (!isValid(day_, month_, year_)) {
        throw std::invalid_argument("Invalid date: " + toString());
    }
}

Integer Date::calculateSerialNumber() const {
    // Calculate days since a reference date (e.g., Jan 1, 1900)
    Integer serial = 0;
    
    // Add days for complete years
    for (Integer y = 1900; y < year_; ++y) {
        bool isLeap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        serial += isLeap ? 366 : 365;
    }
    
    // Add days for complete months in current year
    const Integer* monthDays = isLeapYear() ? daysInMonthLeap : daysInMonth;
    for (Integer m = 1; m < month_; ++m) {
        serial += monthDays[m - 1];
    }
    
    // Add days in current month
    serial += day_;
    
    return serial;
}

Integer Date::daysInMonth(Integer month, Integer year) {
    if (month < 1 || month > 12) return 0;
    
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    return isLeap ? daysInMonthLeap[month - 1] : ::daysInMonth[month - 1];
}

Integer Date::dayOfWeek() const {
    // Simplified day of week calculation (0 = Sunday, 6 = Saturday)
    // Using Zeller's congruence
    Integer q = day_;
    Integer m = month_;
    Integer K = year_ % 100;
    Integer J = year_ / 100;
    
    if (m < 3) {
        m += 12;
        K = (year_ - 1) % 100;
        J = (year_ - 1) / 100;
    }
    
    Integer h = (q + ((13 * (m + 1)) / 5) + K + (K / 4) + (J / 4) - 2 * J) % 7;
    return (h + 5) % 7; // Convert to 0=Sunday format
}

std::ostream& operator<<(std::ostream& os, const Date& date) {
    os << date.toString();
    return os;
}

std::istream& operator>>(std::istream& is, Date& date) {
    std::string dateStr;
    is >> dateStr;
    date = Date(dateStr);
    return is;
}

// Utility functions
Real yearFraction(const Date& start, const Date& end, DayCountConvention dayCount) {
    Integer days = end - start;
    
    switch (dayCount) {
        case DayCountConvention::ACT_360:
            return static_cast<Real>(days) / 360.0;
        case DayCountConvention::ACT_365:
            return static_cast<Real>(days) / 365.0;
        case DayCountConvention::THIRTY_360:
            // Simplified 30/360 calculation
            return static_cast<Real>(days) / 360.0;
        case DayCountConvention::ACT_ACT_ISDA:
            // Simplified actual/actual calculation
            Real totalDays = 0.0;
            Date current = start;
            while (current.year() <= end.year()) {
                Date yearEnd(31, 12, current.year());
                if (yearEnd > end) yearEnd = end;
                
                Integer daysInYear = current.isLeapYear() ? 366 : 365;
                totalDays += static_cast<Real>(yearEnd - current + 1) / daysInYear;
                
                if (yearEnd == end) break;
                current = Date(1, 1, current.year() + 1);
            }
            return totalDays;
        default:
            return static_cast<Real>(days) / 365.0;
    }
}

Real compoundFactor(Real rate, Real time, Frequency compounding) {
    Integer freq = static_cast<Integer>(compounding);
    return std::pow(1.0 + rate / freq, freq * time);
}

Real discountFactor(Real rate, Real time, Frequency compounding) {
    return 1.0 / compoundFactor(rate, time, compounding);
}

} // namespace QuantLib
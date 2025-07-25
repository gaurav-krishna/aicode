#pragma once

#include <iostream>
#include <type_traits>
#include <cmath>

namespace QuantLib {

// Strong typing for financial quantities to prevent unit mixing errors
template<typename Tag, typename ValueType = double>
class Quantity {
public:
    using value_type = ValueType;
    using tag_type = Tag;

    // Constructors
    constexpr Quantity() noexcept : value_(ValueType{}) {}
    explicit constexpr Quantity(ValueType value) noexcept : value_(value) {}
    
    // Copy and move
    constexpr Quantity(const Quantity&) = default;
    constexpr Quantity(Quantity&&) = default;
    constexpr Quantity& operator=(const Quantity&) = default;
    constexpr Quantity& operator=(Quantity&&) = default;

    // Access
    constexpr ValueType value() const noexcept { return value_; }
    constexpr operator ValueType() const noexcept { return value_; }

    // Arithmetic operators
    constexpr Quantity& operator+=(const Quantity& other) noexcept {
        value_ += other.value_;
        return *this;
    }

    constexpr Quantity& operator-=(const Quantity& other) noexcept {
        value_ -= other.value_;
        return *this;
    }

    constexpr Quantity& operator*=(ValueType scalar) noexcept {
        value_ *= scalar;
        return *this;
    }

    constexpr Quantity& operator/=(ValueType scalar) noexcept {
        value_ /= scalar;
        return *this;
    }

    // Unary operators
    constexpr Quantity operator+() const noexcept { return *this; }
    constexpr Quantity operator-() const noexcept { return Quantity(-value_); }

    // Comparison operators
    constexpr bool operator==(const Quantity& other) const noexcept {
        return std::abs(value_ - other.value_) < std::numeric_limits<ValueType>::epsilon();
    }

    constexpr bool operator!=(const Quantity& other) const noexcept {
        return !(*this == other);
    }

    constexpr bool operator<(const Quantity& other) const noexcept {
        return value_ < other.value_;
    }

    constexpr bool operator<=(const Quantity& other) const noexcept {
        return value_ <= other.value_;
    }

    constexpr bool operator>(const Quantity& other) const noexcept {
        return value_ > other.value_;
    }

    constexpr bool operator>=(const Quantity& other) const noexcept {
        return value_ >= other.value_;
    }

private:
    ValueType value_;
};

// Binary arithmetic operators
template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> operator+(const Quantity<Tag, ValueType>& lhs,
                                            const Quantity<Tag, ValueType>& rhs) noexcept {
    return Quantity<Tag, ValueType>(lhs.value() + rhs.value());
}

template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> operator-(const Quantity<Tag, ValueType>& lhs,
                                            const Quantity<Tag, ValueType>& rhs) noexcept {
    return Quantity<Tag, ValueType>(lhs.value() - rhs.value());
}

template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> operator*(const Quantity<Tag, ValueType>& lhs,
                                            ValueType rhs) noexcept {
    return Quantity<Tag, ValueType>(lhs.value() * rhs);
}

template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> operator*(ValueType lhs,
                                            const Quantity<Tag, ValueType>& rhs) noexcept {
    return Quantity<Tag, ValueType>(lhs * rhs.value());
}

template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> operator/(const Quantity<Tag, ValueType>& lhs,
                                            ValueType rhs) noexcept {
    return Quantity<Tag, ValueType>(lhs.value() / rhs);
}

// Division of same quantities yields ratio
template<typename Tag, typename ValueType>
constexpr ValueType operator/(const Quantity<Tag, ValueType>& lhs,
                             const Quantity<Tag, ValueType>& rhs) noexcept {
    return lhs.value() / rhs.value();
}

// Stream operators
template<typename Tag, typename ValueType>
std::ostream& operator<<(std::ostream& os, const Quantity<Tag, ValueType>& q) {
    return os << q.value();
}

template<typename Tag, typename ValueType>
std::istream& operator>>(std::istream& is, Quantity<Tag, ValueType>& q) {
    ValueType value;
    is >> value;
    q = Quantity<Tag, ValueType>(value);
    return is;
}

// Financial quantity types
struct RateTag {};
struct PriceTag {};
struct NotionalTag {};
struct DiscountFactorTag {};
struct TimeTag {};
struct VolatilityTag {};
struct SpreadTag {};

using Rate = Quantity<RateTag>;
using Price = Quantity<PriceTag>;
using Notional = Quantity<NotionalTag>;
using DiscountFactor = Quantity<DiscountFactorTag>;
using Time = Quantity<TimeTag>;
using Volatility = Quantity<VolatilityTag>;
using Spread = Quantity<SpreadTag>;

// Literals for convenience
namespace literals {
    constexpr Rate operator""_rate(long double value) {
        return Rate(static_cast<double>(value));
    }

    constexpr Price operator""_price(long double value) {
        return Price(static_cast<double>(value));
    }

    constexpr Notional operator""_notional(long double value) {
        return Notional(static_cast<double>(value));
    }

    constexpr Time operator""_years(long double value) {
        return Time(static_cast<double>(value));
    }

    constexpr Volatility operator""_vol(long double value) {
        return Volatility(static_cast<double>(value));
    }

    constexpr Spread operator""_bp(long double value) {
        return Spread(static_cast<double>(value) / 10000.0); // basis points to decimal
    }
}

// Mathematical functions for quantities
template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> abs(const Quantity<Tag, ValueType>& q) noexcept {
    return Quantity<Tag, ValueType>(std::abs(q.value()));
}

template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> max(const Quantity<Tag, ValueType>& a,
                                       const Quantity<Tag, ValueType>& b) noexcept {
    return Quantity<Tag, ValueType>(std::max(a.value(), b.value()));
}

template<typename Tag, typename ValueType>
constexpr Quantity<Tag, ValueType> min(const Quantity<Tag, ValueType>& a,
                                       const Quantity<Tag, ValueType>& b) noexcept {
    return Quantity<Tag, ValueType>(std::min(a.value(), b.value()));
}

} // namespace QuantLib
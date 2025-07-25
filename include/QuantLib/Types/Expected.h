#pragma once

#include <variant>
#include <functional>
#include <type_traits>
#include <system_error>

namespace QuantLib {

// Error codes for financial calculations
enum class QuantLibError {
    None = 0,
    InvalidArgument,
    NumericalError,
    MarketDataUnavailable,
    InstrumentExpired,
    PricingEngineNotSet,
    CurveNotFound,
    IterationLimitReached
};

// Error category for QuantLib errors
class QuantLibErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "QuantLib";
    }

    std::string message(int ev) const override {
        switch (static_cast<QuantLibError>(ev)) {
            case QuantLibError::None:
                return "No error";
            case QuantLibError::InvalidArgument:
                return "Invalid argument";
            case QuantLibError::NumericalError:
                return "Numerical error";
            case QuantLibError::MarketDataUnavailable:
                return "Market data unavailable";
            case QuantLibError::InstrumentExpired:
                return "Instrument has expired";
            case QuantLibError::PricingEngineNotSet:
                return "Pricing engine not set";
            case QuantLibError::CurveNotFound:
                return "Yield curve not found";
            case QuantLibError::IterationLimitReached:
                return "Maximum iterations reached";
            default:
                return "Unknown error";
        }
    }
};

inline const QuantLibErrorCategory& quantlib_category() {
    static QuantLibErrorCategory instance;
    return instance;
}

inline std::error_code make_error_code(QuantLibError e) {
    return {static_cast<int>(e), quantlib_category()};
}

// Expected type for monadic error handling
template<typename T, typename E = std::error_code>
class Expected {
public:
    using value_type = T;
    using error_type = E;

    // Constructors
    Expected(const T& value) : data_(value) {}
    Expected(T&& value) : data_(std::move(value)) {}
    Expected(const E& error) : data_(error) {}
    Expected(E&& error) : data_(std::move(error)) {}

    // Factory methods
    static Expected success(const T& value) {
        return Expected(value);
    }

    static Expected success(T&& value) {
        return Expected(std::move(value));
    }

    static Expected failure(const E& error) {
        return Expected(error);
    }

    static Expected failure(E&& error) {
        return Expected(std::move(error));
    }

    // State queries
    bool hasValue() const noexcept {
        return std::holds_alternative<T>(data_);
    }

    bool hasError() const noexcept {
        return std::holds_alternative<E>(data_);
    }

    explicit operator bool() const noexcept {
        return hasValue();
    }

    // Value access
    const T& value() const & {
        if (!hasValue()) {
            throw std::runtime_error("Expected has no value");
        }
        return std::get<T>(data_);
    }

    T& value() & {
        if (!hasValue()) {
            throw std::runtime_error("Expected has no value");
        }
        return std::get<T>(data_);
    }

    T&& value() && {
        if (!hasValue()) {
            throw std::runtime_error("Expected has no value");
        }
        return std::get<T>(std::move(data_));
    }

    const T& operator*() const & {
        return value();
    }

    T& operator*() & {
        return value();
    }

    T&& operator*() && {
        return std::move(*this).value();
    }

    const T* operator->() const {
        return hasValue() ? &std::get<T>(data_) : nullptr;
    }

    T* operator->() {
        return hasValue() ? &std::get<T>(data_) : nullptr;
    }

    // Error access
    const E& error() const {
        if (!hasError()) {
            throw std::runtime_error("Expected has no error");
        }
        return std::get<E>(data_);
    }

    // Value with default
    template<typename U>
    T valueOr(U&& defaultValue) const & {
        return hasValue() ? value() : static_cast<T>(std::forward<U>(defaultValue));
    }

    template<typename U>
    T valueOr(U&& defaultValue) && {
        return hasValue() ? std::move(*this).value() : static_cast<T>(std::forward<U>(defaultValue));
    }

    // Monadic operations
    template<typename F>
    auto map(F&& f) const & -> Expected<decltype(f(value())), E> {
        using ReturnType = decltype(f(value()));
        if (hasValue()) {
            return Expected<ReturnType, E>::success(f(value()));
        } else {
            return Expected<ReturnType, E>::failure(error());
        }
    }

    template<typename F>
    auto map(F&& f) && -> Expected<decltype(f(std::move(*this).value())), E> {
        using ReturnType = decltype(f(std::move(*this).value()));
        if (hasValue()) {
            return Expected<ReturnType, E>::success(f(std::move(*this).value()));
        } else {
            return Expected<ReturnType, E>::failure(std::move(*this).error());
        }
    }

    template<typename F>
    auto flatMap(F&& f) const & -> decltype(f(value())) {
        if (hasValue()) {
            return f(value());
        } else {
            using ReturnType = decltype(f(value()));
            return ReturnType::failure(error());
        }
    }

    template<typename F>
    auto flatMap(F&& f) && -> decltype(f(std::move(*this).value())) {
        if (hasValue()) {
            return f(std::move(*this).value());
        } else {
            using ReturnType = decltype(f(std::move(*this).value()));
            return ReturnType::failure(std::move(*this).error());
        }
    }

    template<typename F>
    auto mapError(F&& f) const & -> Expected<T, decltype(f(error()))> {
        using NewErrorType = decltype(f(error()));
        if (hasError()) {
            return Expected<T, NewErrorType>::failure(f(error()));
        } else {
            return Expected<T, NewErrorType>::success(value());
        }
    }

    template<typename F>
    auto mapError(F&& f) && -> Expected<T, decltype(f(std::move(*this).error()))> {
        using NewErrorType = decltype(f(std::move(*this).error()));
        if (hasError()) {
            return Expected<T, NewErrorType>::failure(f(std::move(*this).error()));
        } else {
            return Expected<T, NewErrorType>::success(std::move(*this).value());
        }
    }

    // Execute function on value if present
    template<typename F>
    const Expected& ifValue(F&& f) const & {
        if (hasValue()) {
            f(value());
        }
        return *this;
    }

    template<typename F>
    Expected& ifValue(F&& f) & {
        if (hasValue()) {
            f(value());
        }
        return *this;
    }

    // Execute function on error if present
    template<typename F>
    const Expected& ifError(F&& f) const & {
        if (hasError()) {
            f(error());
        }
        return *this;
    }

    template<typename F>
    Expected& ifError(F&& f) & {
        if (hasError()) {
            f(error());
        }
        return *this;
    }

private:
    std::variant<T, E> data_;
};

// Convenience type aliases
template<typename T>
using Result = Expected<T, std::error_code>;

// Factory functions
template<typename T>
Result<T> success(T&& value) {
    return Result<T>::success(std::forward<T>(value));
}

template<typename T>
Result<T> failure(QuantLibError error) {
    return Result<T>::failure(make_error_code(error));
}

template<typename T>
Result<T> failure(std::error_code error) {
    return Result<T>::failure(error);
}

} // namespace QuantLib

// Enable error_code conversion for QuantLibError
namespace std {
    template<>
    struct is_error_code_enum<QuantLib::QuantLibError> : true_type {};
}
#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <algorithm>
#include <mutex>

namespace QuantLib {

// Forward declaration
template<typename T> class Observer;

// Observable base class using CRTP for type safety
template<typename Derived>
class Observable {
public:
    using ObserverType = Observer<Derived>;
    using WeakObserverPtr = std::weak_ptr<ObserverType>;
    using Callback = std::function<void(const Derived&)>;

    virtual ~Observable() = default;

    // Register an observer
    void registerObserver(std::shared_ptr<ObserverType> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.push_back(observer);
    }

    // Register a callback function
    void registerObserver(Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.push_back(std::move(callback));
    }

    // Unregister an observer
    void unregisterObserver(std::shared_ptr<ObserverType> observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [&observer](const WeakObserverPtr& weak) {
                    return weak.expired() || weak.lock() == observer;
                }),
            observers_.end()
        );
    }

    // Notify all observers
    void notifyObservers() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Clean up expired observers
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const WeakObserverPtr& weak) { return weak.expired(); }),
            observers_.end()
        );

        // Notify remaining observers
        const auto& derived = static_cast<const Derived&>(*this);
        
        for (const auto& weakObserver : observers_) {
            if (auto observer = weakObserver.lock()) {
                observer->update(derived);
            }
        }

        // Notify callback functions
        for (const auto& callback : callbacks_) {
            callback(derived);
        }
    }

    // Get number of active observers
    size_t observerCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::count_if(observers_.begin(), observers_.end(),
            [](const WeakObserverPtr& weak) { return !weak.expired(); }) + callbacks_.size();
    }

    // Clear all observers
    void clearObservers() {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.clear();
        callbacks_.clear();
    }

protected:
    // Convenience method for derived classes
    void notifyObserversImpl() {
        notifyObservers();
    }

private:
    mutable std::mutex mutex_;
    std::vector<WeakObserverPtr> observers_;
    std::vector<Callback> callbacks_;
};

// RAII helper for automatic observer registration/unregistration
template<typename ObservableType, typename ObserverType>
class ObserverRegistration {
public:
    ObserverRegistration(ObservableType& observable, std::shared_ptr<ObserverType> observer)
        : observable_(observable), observer_(observer) {
        observable_.registerObserver(observer_);
    }

    ~ObserverRegistration() {
        if (auto obs = observer_.lock()) {
            observable_.unregisterObserver(obs);
        }
    }

    // Non-copyable, movable
    ObserverRegistration(const ObserverRegistration&) = delete;
    ObserverRegistration& operator=(const ObserverRegistration&) = delete;

    ObserverRegistration(ObserverRegistration&&) = default;
    ObserverRegistration& operator=(ObserverRegistration&&) = default;

private:
    ObservableType& observable_;
    std::weak_ptr<ObserverType> observer_;
};

// Helper function to create observer registration
template<typename ObservableType, typename ObserverType>
auto makeObserverRegistration(ObservableType& observable, std::shared_ptr<ObserverType> observer) {
    return ObserverRegistration<ObservableType, ObserverType>(observable, observer);
}

// Scoped observer for automatic cleanup
template<typename ObservableType>
class ScopedObserver {
public:
    using Callback = typename ObservableType::Callback;

    ScopedObserver(ObservableType& observable, Callback callback)
        : observable_(observable), callback_(std::move(callback)) {
        observable_.registerObserver(callback_);
    }

    ~ScopedObserver() {
        // Note: In a real implementation, you'd need a way to unregister callbacks
        // This is simplified for demonstration
    }

    // Non-copyable, movable
    ScopedObserver(const ScopedObserver&) = delete;
    ScopedObserver& operator=(const ScopedObserver&) = delete;

    ScopedObserver(ScopedObserver&&) = default;
    ScopedObserver& operator=(ScopedObserver&&) = default;

private:
    ObservableType& observable_;
    Callback callback_;
};

// Helper function to create scoped observer
template<typename ObservableType>
auto makeScopedObserver(ObservableType& observable, 
                       typename ObservableType::Callback callback) {
    return ScopedObserver<ObservableType>(observable, std::move(callback));
}

} // namespace QuantLib
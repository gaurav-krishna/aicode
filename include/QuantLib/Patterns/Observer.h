#pragma once

namespace QuantLib {

// Observer interface
template<typename Observable>
class Observer {
public:
    virtual ~Observer() = default;
    
    // Called when the observable notifies its observers
    virtual void update(const Observable& observable) = 0;
    
protected:
    Observer() = default;
    Observer(const Observer&) = default;
    Observer(Observer&&) = default;
    Observer& operator=(const Observer&) = default;
    Observer& operator=(Observer&&) = default;
};

// Concrete observer that delegates to a function
template<typename Observable>
class FunctionObserver : public Observer<Observable> {
public:
    using UpdateFunction = std::function<void(const Observable&)>;

    explicit FunctionObserver(UpdateFunction updateFunc)
        : updateFunc_(std::move(updateFunc)) {}

    void update(const Observable& observable) override {
        if (updateFunc_) {
            updateFunc_(observable);
        }
    }

private:
    UpdateFunction updateFunc_;
};

// Helper function to create function observer
template<typename Observable>
auto makeFunctionObserver(std::function<void(const Observable&)> updateFunc) {
    return std::make_shared<FunctionObserver<Observable>>(std::move(updateFunc));
}

// Lambda observer - more convenient for lambdas
template<typename Observable, typename F>
auto makeLambdaObserver(F&& lambda) {
    return std::make_shared<FunctionObserver<Observable>>(std::forward<F>(lambda));
}

} // namespace QuantLib
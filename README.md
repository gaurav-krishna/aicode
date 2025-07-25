# Modern C++ Quantitative Finance Library v2.0

A next-generation C++ quantitative finance library built with modern C++20 features, featuring zero-overhead abstractions, type-safe financial quantities, monadic error handling, and industry-standard design patterns.

## 🚀 Features

### Core Components
- **FixedCouponBond** - Complete bond representation with serialization support
- **MultiMarketEnv** - Asset-aware market environment for all market data types
- **BondPricingEngine** - Comprehensive pricing engine with advanced analytics
- **InterestRateCurve** - Full-featured yield curves with multiple interpolation methods
- **CashFlow Management** - Automated cashflow generation and present value calculations
- **Date Handling** - Professional date arithmetic with business day conventions

### Key Capabilities
- ✅ **Zero-Overhead Abstractions** - CRTP-based polymorphism without virtual function costs
- ✅ **Type-Safe Financial Quantities** - Compile-time prevention of unit mixing errors
- ✅ **Monadic Error Handling** - Functional error management without exceptions
- ✅ **Observer Pattern** - Automatic updates for market data changes
- ✅ **Modern Memory Management** - Smart pointers and RAII throughout
- ✅ **Template Metaprogramming** - Compile-time optimizations and type safety
- ✅ **C++20 Features** - Concepts, ranges, chrono, and constexpr improvements
- ✅ **Header-Only Design** - Easy integration with minimal build complexity

## 🏗️ Architecture

### Asset-Market Data Association
```cpp
// Associate market curves with specific bonds
MultiMarketEnv env(Date(1, 1, 2024));
env.setAssetDiscountCurve("BOND_US_10Y", "USD_GOVT_CURVE");
env.setAssetCreditCurve("BOND_CORP_AA", "CORPORATE_CREDIT_CURVE");

// Price bond using its associated curves
BondPricingEngine engine(env);
auto results = engine.priceBond(bond, valuationDate);
```

### Class Hierarchy
```
MultiMarketEnv (Base)
├── AssetMarketData - Asset-specific curve associations
├── CurveCollection - Interest rate curves
├── MarketData - Equity spots, FX rates, volatilities
└── MarketDataFactory - Creation utilities

FixedCouponBond
├── Serialization (JSON/XML/Text)
├── CashFlow Generation
├── Accrued Interest Calculation
└── Bond Validation

BondPricingEngine
├── Asset-based Pricing Methods
├── Yield Calculation Algorithms
├── Risk Sensitivity Analysis
└── Scenario Testing
```

## 🛠️ Quick Start

### 1. Build the Library
```bash
mkdir build && cd build
cmake ..
make
```

### 2. Basic Usage Example
```cpp
#include "QuantLib.h"
using namespace QuantLib;

// Create a bond
FixedCouponBond bond(
    "US_TREASURY_10Y",           // identifier
    Date(1, 1, 2024),           // issue date
    Date(1, 1, 2034),           // maturity date
    100000.0,                   // face value
    0.045,                      // 4.5% coupon rate
    Frequency::SEMI_ANNUAL      // payment frequency
);

// Set up market environment
MultiMarketEnv market(Date(15, 6, 2024));

// Add interest rate curve
std::vector<Date> dates = {
    Date(15, 6, 2024), Date(15, 6, 2025), 
    Date(15, 6, 2029), Date(15, 6, 2034)
};
std::vector<Real> rates = {0.035, 0.040, 0.042, 0.045};
InterestRateCurve curve("USD_GOVT", Date(15, 6, 2024), dates, rates);
market.addInterestRateCurve("USD_GOVT", curve);

// Associate curve with bond
market.setAssetDiscountCurve("US_TREASURY_10Y", "USD_GOVT");

// Price the bond
BondPricingEngine engine(market);
auto results = engine.priceBond(bond, Date(15, 6, 2024));

std::cout << "Present Value: " << results.presentValue << std::endl;
std::cout << "Clean Price: " << results.cleanPrice << std::endl;
std::cout << "Yield to Maturity: " << results.yieldToMaturity << std::endl;
std::cout << "Modified Duration: " << results.modifiedDuration << std::endl;
```

## 📊 Advanced Features

### Cashflow Generation
```cpp
// Generate all bond cashflows
auto cashflows = bond.generateCashFlows();
for (const auto& cf : cashflows) {
    std::cout << cf.date() << ": " << cf.amount() 
              << " (" << cf.type() << ")" << std::endl;
}

// Calculate individual present values
auto pvs = engine.calculateCashFlowPresentValues(bond, valuationDate);
```

### Risk Analytics
```cpp
// Calculate comprehensive risk metrics
auto risks = engine.calculateRiskSensitivities(bond, valuationDate);
std::cout << "DV01: " << risks.dv01 << std::endl;
std::cout << "Convexity: " << risks.convexity << std::endl;

// Scenario analysis
std::vector<Real> shiftScenarios = {-0.01, -0.005, 0.0, 0.005, 0.01};
auto scenarios = engine.calculatePriceScenarios(bond, valuationDate, shiftScenarios);
```

### Serialization
```cpp
// Serialize bond to different formats
std::string jsonData = bond.toJSON();
std::string xmlData = bond.toXML();
std::string textData = bond.serialize();

// Deserialize from formats
auto bondFromJson = FixedCouponBond::fromJSON(jsonData);
auto bondFromXml = FixedCouponBond::fromXML(xmlData);
auto bondFromText = FixedCouponBond::deserialize(textData);

// Market environment serialization
std::string marketData = market.serialize();
auto restoredMarket = MultiMarketEnv::deserialize(marketData);
```

## 🧪 Testing

Run the test suite:
```bash
cd build
./tests
```

Example test output:
```bash
./bond_example
# Creates sample bonds, prices them, and displays results

./tests
# Runs comprehensive unit tests for all components
```

## 📁 Project Structure

```
.
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── include/                    # Header files
│   ├── Common.h               # Common types and utilities
│   ├── Date.h                 # Date handling
│   ├── CashFlow.h             # Cashflow management
│   ├── InterestRateCurve.h    # Yield curve implementation
│   ├── MultiMarketEnv.h       # Market data environment
│   ├── FixedCouponBond.h      # Bond representation
│   └── BondPricingEngine.h    # Pricing engine
├── src/                       # Implementation files
│   ├── Date.cpp
│   ├── CashFlow.cpp
│   ├── InterestRateCurve.cpp
│   ├── MultiMarketEnv.cpp
│   ├── FixedCouponBond.cpp
│   └── BondPricingEngine.cpp
├── examples/                  # Usage examples
│   └── bond_example.cpp
└── tests/                     # Test suite
    └── test_bond_pricing.cpp
```

## 🔧 Dependencies

- **C++17** or later
- **CMake 3.16** or later
- Standard library only (no external dependencies)

## 🎯 Design Principles

1. **Asset-Centric**: Market data is associated with specific assets, not just globally available
2. **Serializable**: All objects can be serialized and deserialized for persistence
3. **Extensible**: Modular design allows easy addition of new instruments
4. **Professional**: Production-ready code with comprehensive error handling
5. **Performance**: Optimized for computational efficiency in pricing operations

## 🚀 Roadmap

- [ ] Add floating rate bonds
- [ ] Implement credit default swaps
- [ ] Add Monte Carlo pricing methods
- [ ] Extend to equity derivatives
- [ ] Add multi-threading support
- [ ] Implement exotic bond types

## 📝 License

This project is part of a quantitative finance library demonstration.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

---

**Built with modern C++ for quantitative finance professionals**
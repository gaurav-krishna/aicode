# Option Adjusted Spread (OAS) Calculator

A comprehensive Python implementation for computing the Option Adjusted Spread of bonds with embedded options using Monte Carlo simulation and advanced interest rate modeling.

## Overview

Option Adjusted Spread (OAS) is a crucial metric in fixed-income analysis that measures the yield spread of a bond with embedded options after adjusting for the value of those options. The fundamental relationship is:

```
OAS = Z-Spread - Option Cost
```

Where:
- **Z-Spread**: The constant spread over the risk-free yield curve for an option-free bond
- **Option Cost**: The value of embedded options (positive for callable bonds, negative for putable bonds)

## Features

### Core Functionality
- **Monte Carlo Simulation**: Advanced stochastic interest rate path generation
- **Vasicek Interest Rate Model**: dr = a(b - r)dt + σ dW
- **Embedded Options Support**: Callable and putable bonds
- **Yield Curve Integration**: Flexible yield curve interpolation
- **Sensitivity Analysis**: Comprehensive volatility and parameter sensitivity

### Bond Types Supported
- Option-free corporate bonds
- Callable bonds (single or multiple call dates)
- Putable bonds (single or multiple put dates)
- Mortgage-backed securities (simplified prepayment modeling)
- Complex securities with both call and put options

## Installation

```bash
# Install required dependencies
pip install -r requirements.txt

# Or install individually
pip install numpy scipy pandas matplotlib quantlib statsmodels jupyter seaborn
```

## Quick Start

```python
from oas_calculator import OASCalculator, Bond, YieldCurve, InterestRateModel

# Create yield curve
tenors = [0.5, 1, 2, 5, 10]
rates = [0.02, 0.025, 0.03, 0.035, 0.04]
yield_curve = YieldCurve(tenors, rates)

# Initialize interest rate model
rate_model = InterestRateModel(
    mean_reversion=0.1,
    long_term_mean=0.04,
    volatility=0.02,
    initial_rate=0.03
)

# Create OAS calculator
calculator = OASCalculator(yield_curve, rate_model)

# Define a callable bond
call_schedule = {3.0: 100, 4.0: 100, 5.0: 100}  # Callable at par from year 3
bond = Bond(
    face_value=100,
    coupon_rate=0.05,
    maturity=5.0,
    frequency=2,  # Semi-annual
    call_schedule=call_schedule
)

# Calculate OAS
market_price = 102.5
results = calculator.calculate_oas(bond, market_price)

print(f"OAS: {results['oas_bps']:.0f} basis points")
print(f"Z-Spread: {results['z_spread_bps']:.0f} basis points")
print(f"Option Cost: {results['option_cost_bps']:.0f} basis points")
```

## Mathematical Foundation

### Interest Rate Model (Vasicek)

The Vasicek model generates stochastic interest rate paths:

```
dr(t) = a(b - r(t))dt + σ dW(t)
```

Where:
- `a`: Mean reversion speed
- `b`: Long-term mean rate
- `σ`: Volatility parameter
- `dW(t)`: Wiener process

### Bond Pricing with Options

For each Monte Carlo path, bond pricing uses backward induction:

1. **Start at maturity**: Bond value = Final payment
2. **Work backwards**: For each payment date:
   - Discount bond value using path-dependent rates
   - Add current coupon payment
   - Check option exercise conditions:
     - **Callable**: `Bond_value = min(Bond_value, Call_price)`
     - **Putable**: `Bond_value = max(Bond_value, Put_price)`

### OAS Calculation

The OAS is found iteratively using root-finding methods:

1. Calculate Z-spread for equivalent option-free bond
2. Use Monte Carlo to price bond with embedded options
3. Iterate to find spread that matches market price
4. Option cost = Z-spread - OAS

## Usage Examples

### Example 1: Callable Corporate Bond

```python
# 6% coupon, 10-year callable bond
call_schedule = {year: 102 for year in range(3, 11)}
callable_bond = Bond(100, 0.06, 10.0, 2, call_schedule=call_schedule)

results = calculator.calculate_oas(callable_bond, 108.5)
# Results show call option reduces OAS compared to Z-spread
```

### Example 2: Putable Bond

```python
# 4% coupon, 7-year putable bond
put_schedule = {year: 100 for year in range(3, 8)}
putable_bond = Bond(100, 0.04, 7.0, 1, put_schedule=put_schedule)

results = calculator.calculate_oas(putable_bond, 98.2)
# Results show put option increases OAS (negative option cost)
```

### Example 3: Sensitivity Analysis

```python
# Analyze sensitivity to interest rate volatility
sensitivity = calculator.sensitivity_analysis(
    bond, market_price,
    volatility_range=(0.01, 0.05),
    num_scenarios=10
)

# Higher volatility increases option values
print(sensitivity)
```

## Advanced Features

### Custom Yield Curves

```python
# Create custom yield curve with term structure
tenors = [0.25, 0.5, 1, 2, 3, 5, 7, 10, 20, 30]
rates = [0.02, 0.025, 0.03, 0.035, 0.04, 0.042, 0.043, 0.044, 0.045, 0.045]
yield_curve = YieldCurve(tenors, rates)
```

### Interest Rate Model Calibration

```python
# Calibrate model parameters to market conditions
rate_model = InterestRateModel(
    mean_reversion=0.15,      # Faster mean reversion
    long_term_mean=0.045,     # Higher long-term rate
    volatility=0.025,         # Market-implied volatility
    initial_rate=0.035        # Current short rate
)
```

### Complex Option Schedules

```python
# Declining call prices (typical for corporate bonds)
call_schedule = {}
for year in range(2, 11):
    call_price = 105 - (year - 2) * 0.5  # Starts at 105, declines to 100
    call_schedule[float(year)] = max(call_price, 100)

# Multiple put dates
put_schedule = {3.0: 98, 5.0: 99, 7.0: 100}  # Increasing put prices
```

## Model Validation

### Convergence Testing

The implementation includes convergence diagnostics:

```python
# Test Monte Carlo convergence
num_paths_test = [1000, 5000, 10000, 20000]
for paths in num_paths_test:
    result = calculator.calculate_oas(bond, market_price, num_paths=paths)
    print(f"Paths: {paths}, OAS: {result['oas_bps']:.1f} bps")
```

### Benchmark Comparisons

Compare results with market standards:

```python
# For option-free bonds, OAS should equal Z-spread
option_free_bond = Bond(100, 0.05, 5.0, 2)
z_spread = calculator.calculate_z_spread(option_free_bond, market_price)
oas_result = calculator.calculate_oas(option_free_bond, market_price)

print(f"Z-Spread: {z_spread * 10000:.1f} bps")
print(f"OAS: {oas_result['oas_bps']:.1f} bps")
print(f"Difference: {abs(z_spread * 10000 - oas_result['oas_bps']):.1f} bps")
```

## Performance Considerations

### Optimization Tips

1. **Path Count**: Start with 5,000 paths for testing, use 10,000+ for production
2. **Convergence**: Monitor price convergence with increasing path counts
3. **Time Step**: Smaller time steps improve accuracy but increase computation time
4. **Caching**: Cache yield curve calculations for repeated use

### Computational Complexity

- **Time Complexity**: O(N × M × T) where N = paths, M = time periods, T = payment dates
- **Memory Usage**: Linear in number of paths and time periods
- **Parallelization**: Monte Carlo paths can be computed in parallel

## Limitations and Considerations

### Model Limitations

1. **Interest Rate Model**: Vasicek model assumes normal rates (can go negative)
2. **Simplified Options**: Real-world exercise decisions more complex
3. **Credit Risk**: Does not explicitly model default risk
4. **Liquidity**: Does not account for liquidity premiums

### Practical Considerations

1. **Market Data**: Requires accurate yield curve and volatility estimates
2. **Model Risk**: Results sensitive to parameter choices
3. **Computation Time**: Large Monte Carlo simulations can be slow
4. **Model Validation**: Regular backtesting recommended

## Comparison with Market Practice

### Industry Standards

This implementation follows market conventions:

- Uses risk-neutral pricing framework
- Employs Monte Carlo for path-dependent options
- Follows OAS calculation methodology used by major vendors
- Includes sensitivity analysis capabilities

### Differences from Commercial Systems

- **Simplified**: Focuses on core OAS calculation rather than full trading system
- **Educational**: Emphasizes transparency and understanding
- **Extensible**: Designed for customization and research

## Future Enhancements

### Potential Additions

1. **Additional Rate Models**: Cox-Ingersoll-Ross, Hull-White, Black-Karasinski
2. **Credit Risk**: Integrate default probability modeling
3. **American Options**: More sophisticated exercise boundary estimation
4. **Performance**: GPU acceleration for Monte Carlo simulations
5. **Calibration**: Automatic model parameter estimation from market data

### Research Applications

- Academic research in fixed-income pricing
- Risk management model development
- Trading strategy backtesting
- Educational demonstrations

## Support and Contributing

### Documentation

- **Code Documentation**: Comprehensive docstrings throughout
- **Examples**: Multiple practical examples provided
- **Theory**: Mathematical foundation explained
- **Validation**: Test cases and benchmarks included

### Development

This implementation is designed for:
- Financial researchers and practitioners
- Academic coursework and research
- Risk management applications
- Trading system development

## License

This project is provided for educational and research purposes. Please ensure compliance with any applicable regulations when using for commercial purposes.

---

## References

1. Fabozzi, F. J. (2012). *Bond Portfolio Management*. Wiley.
2. Tuckman, B., & Serrat, A. (2011). *Fixed Income Securities*. Wiley.
3. Martellini, L., Priaulet, P., & Priaulet, S. (2003). *Fixed-Income Securities*. Wiley.
4. Hull, J. C. (2017). *Options, Futures, and Other Derivatives*. Pearson.

For detailed mathematical derivations and implementation notes, see the inline documentation in the source code.
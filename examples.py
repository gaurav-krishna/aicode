"""
Example calculations and demonstrations of Option Adjusted Spread (OAS)

This module provides practical examples of OAS calculations for various bond types:
- Callable bonds
- Putable bonds
- Mortgage-backed securities simulation
- Comparative analysis
"""

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from oas_calculator import (OASCalculator, Bond, YieldCurve, InterestRateModel, 
                           create_sample_yield_curve)


def example_1_callable_bond():
    """
    Example 1: OAS calculation for a callable corporate bond
    """
    print("=" * 60)
    print("EXAMPLE 1: CALLABLE CORPORATE BOND OAS")
    print("=" * 60)
    
    # Market conditions
    yield_curve = create_sample_yield_curve()
    rate_model = InterestRateModel(mean_reversion=0.15, long_term_mean=0.04, 
                                  volatility=0.025, initial_rate=0.035)
    calculator = OASCalculator(yield_curve, rate_model)
    
    # Callable bond: 6% coupon, 10-year maturity, callable at 102 after year 3
    call_schedule = {}
    for year in range(3, 11):
        call_price = 102 - (year - 3) * 0.5  # Declining call price
        call_schedule[float(year)] = max(call_price, 100)
    
    callable_bond = Bond(
        face_value=100,
        coupon_rate=0.06,
        maturity=10.0,
        frequency=2,  # Semi-annual
        call_schedule=call_schedule
    )
    
    market_price = 108.5  # Trading at premium
    
    print(f"Bond Details:")
    print(f"- Face Value: ${callable_bond.face_value}")
    print(f"- Coupon Rate: {callable_bond.coupon_rate:.1%}")
    print(f"- Maturity: {callable_bond.maturity} years")
    print(f"- Market Price: ${market_price}")
    print(f"- Call Protection: 3 years")
    print(f"- Call Schedule: {call_schedule}")
    
    # Calculate OAS
    results = calculator.calculate_oas(callable_bond, market_price, num_paths=8000)
    
    print(f"\nOAS Analysis:")
    print(f"- Option Adjusted Spread: {results['oas_bps']:.0f} basis points")
    print(f"- Z-Spread (option-free): {results['z_spread_bps']:.0f} basis points")
    print(f"- Option Cost: {results['option_cost_bps']:.0f} basis points")
    
    # Compare with option-free bond
    option_free_bond = Bond(100, 0.06, 10.0, 2)
    option_free_price = calculator.price_option_free_bond(option_free_bond, results['z_spread'])
    
    print(f"\nComparison:")
    print(f"- Option-free theoretical price: ${option_free_price:.2f}")
    print(f"- Callable bond market price: ${market_price:.2f}")
    print(f"- Option value: ${option_free_price - market_price:.2f}")
    
    return results


def example_2_putable_bond():
    """
    Example 2: OAS calculation for a putable bond
    """
    print("\n" + "=" * 60)
    print("EXAMPLE 2: PUTABLE BOND OAS")
    print("=" * 60)
    
    yield_curve = create_sample_yield_curve()
    rate_model = InterestRateModel(mean_reversion=0.12, long_term_mean=0.045, 
                                  volatility=0.03, initial_rate=0.04)
    calculator = OASCalculator(yield_curve, rate_model)
    
    # Putable bond: 4% coupon, 7-year maturity, putable at par after year 2
    put_schedule = {3.0: 100, 4.0: 100, 5.0: 100, 6.0: 100, 7.0: 100}
    
    putable_bond = Bond(
        face_value=100,
        coupon_rate=0.04,
        maturity=7.0,
        frequency=1,  # Annual
        put_schedule=put_schedule
    )
    
    market_price = 98.2  # Trading at discount
    
    print(f"Bond Details:")
    print(f"- Face Value: ${putable_bond.face_value}")
    print(f"- Coupon Rate: {putable_bond.coupon_rate:.1%}")
    print(f"- Maturity: {putable_bond.maturity} years")
    print(f"- Market Price: ${market_price}")
    print(f"- Put Protection: 2 years")
    print(f"- Put Schedule: {put_schedule}")
    
    results = calculator.calculate_oas(putable_bond, market_price, num_paths=6000)
    
    print(f"\nOAS Analysis:")
    print(f"- Option Adjusted Spread: {results['oas_bps']:.0f} basis points")
    print(f"- Z-Spread (option-free): {results['z_spread_bps']:.0f} basis points")
    print(f"- Option Cost: {results['option_cost_bps']:.0f} basis points")
    
    return results


def example_3_comparative_analysis():
    """
    Example 3: Comparative analysis of different bond structures
    """
    print("\n" + "=" * 60)
    print("EXAMPLE 3: COMPARATIVE BOND ANALYSIS")
    print("=" * 60)
    
    yield_curve = create_sample_yield_curve()
    rate_model = InterestRateModel(mean_reversion=0.1, long_term_mean=0.04, 
                                  volatility=0.025, initial_rate=0.035)
    calculator = OASCalculator(yield_curve, rate_model)
    
    # Define three similar bonds with different option features
    bonds = {
        'Option-Free': Bond(100, 0.05, 5.0, 2),
        'Callable': Bond(100, 0.05, 5.0, 2, call_schedule={3.0: 100, 4.0: 100, 5.0: 100}),
        'Putable': Bond(100, 0.05, 5.0, 2, put_schedule={3.0: 100, 4.0: 100, 5.0: 100})
    }
    
    # Assume market prices
    market_prices = {
        'Option-Free': 101.5,
        'Callable': 100.8,    # Lower due to call risk
        'Putable': 102.2      # Higher due to put benefit
    }
    
    results_df = []
    
    for bond_type, bond in bonds.items():
        price = market_prices[bond_type]
        
        if bond_type == 'Option-Free':
            # For option-free bond, OAS = Z-spread
            z_spread = calculator.calculate_z_spread(bond, price)
            results = {
                'oas_bps': z_spread * 10000,
                'z_spread_bps': z_spread * 10000,
                'option_cost_bps': 0
            }
        else:
            results = calculator.calculate_oas(bond, price, num_paths=5000)
        
        results_df.append({
            'Bond Type': bond_type,
            'Market Price': price,
            'OAS (bps)': results['oas_bps'],
            'Z-Spread (bps)': results['z_spread_bps'],
            'Option Cost (bps)': results['option_cost_bps']
        })
    
    comparison_df = pd.DataFrame(results_df)
    print("\nComparative Analysis:")
    print(comparison_df.round(1))
    
    print(f"\nKey Insights:")
    print(f"- Callable bond has lowest OAS due to call risk")
    print(f"- Putable bond has highest OAS due to put benefit")
    print(f"- Option costs reflect embedded option values")
    
    return comparison_df


def example_4_sensitivity_analysis():
    """
    Example 4: Comprehensive sensitivity analysis
    """
    print("\n" + "=" * 60)
    print("EXAMPLE 4: SENSITIVITY ANALYSIS")
    print("=" * 60)
    
    yield_curve = create_sample_yield_curve()
    rate_model = InterestRateModel(mean_reversion=0.1, long_term_mean=0.04, 
                                  volatility=0.02, initial_rate=0.035)
    calculator = OASCalculator(yield_curve, rate_model)
    
    # Callable bond for analysis
    call_schedule = {2.0: 101, 3.0: 100.5, 4.0: 100, 5.0: 100}
    callable_bond = Bond(100, 0.055, 5.0, 2, call_schedule=call_schedule)
    market_price = 103.0
    
    print(f"Analyzing callable bond sensitivity...")
    print(f"Base case: 5.5% coupon, 5-year callable bond at ${market_price}")
    
    # Volatility sensitivity
    volatility_sensitivity = calculator.sensitivity_analysis(
        callable_bond, market_price, 
        volatility_range=(0.005, 0.05), 
        num_scenarios=8
    )
    
    print(f"\nVolatility Sensitivity:")
    print(volatility_sensitivity.round(1))
    
    # Create visualization
    plt.figure(figsize=(12, 8))
    
    # Plot 1: OAS vs Volatility
    plt.subplot(2, 2, 1)
    plt.plot(volatility_sensitivity['volatility'] * 100, 
             volatility_sensitivity['oas_bps'], 'b-o', linewidth=2)
    plt.xlabel('Interest Rate Volatility (%)')
    plt.ylabel('OAS (basis points)')
    plt.title('OAS vs Interest Rate Volatility')
    plt.grid(True, alpha=0.3)
    
    # Plot 2: Option Cost vs Volatility
    plt.subplot(2, 2, 2)
    plt.plot(volatility_sensitivity['volatility'] * 100, 
             volatility_sensitivity['option_cost_bps'], 'r-s', linewidth=2)
    plt.xlabel('Interest Rate Volatility (%)')
    plt.ylabel('Option Cost (basis points)')
    plt.title('Option Cost vs Interest Rate Volatility')
    plt.grid(True, alpha=0.3)
    
    # Plot 3: Price sensitivity to spread changes
    plt.subplot(2, 2, 3)
    spreads = np.linspace(-0.01, 0.05, 20)
    prices = [calculator.price_option_free_bond(callable_bond, s) for s in spreads]
    plt.plot(spreads * 10000, prices, 'g-', linewidth=2)
    plt.axhline(y=market_price, color='r', linestyle='--', label=f'Market Price: ${market_price}')
    plt.xlabel('Spread (basis points)')
    plt.ylabel('Bond Price ($)')
    plt.title('Price Sensitivity to Spread')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    # Plot 4: Interest rate paths sample
    plt.subplot(2, 2, 4)
    sample_paths = rate_model.generate_paths(5, 20, 0.25)
    time_points = np.arange(0, 5.25, 0.25)
    for i in range(5):
        plt.plot(time_points, sample_paths[i] * 100, alpha=0.7)
    plt.xlabel('Time (years)')
    plt.ylabel('Interest Rate (%)')
    plt.title('Sample Interest Rate Paths')
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('/workspace/oas_sensitivity_analysis.png', dpi=300, bbox_inches='tight')
    print(f"\nSensitivity charts saved to 'oas_sensitivity_analysis.png'")
    
    return volatility_sensitivity


def example_5_mortgage_backed_security():
    """
    Example 5: Simplified MBS-style prepayment modeling
    """
    print("\n" + "=" * 60)
    print("EXAMPLE 5: MORTGAGE-BACKED SECURITY SIMULATION")
    print("=" * 60)
    
    yield_curve = create_sample_yield_curve()
    rate_model = InterestRateModel(mean_reversion=0.08, long_term_mean=0.04, 
                                  volatility=0.03, initial_rate=0.04)
    calculator = OASCalculator(yield_curve, rate_model)
    
    # Simulate MBS with prepayment options at multiple points
    # Assume prepayment possible every 6 months after year 1
    call_schedule = {}
    for period in np.arange(1.0, 10.5, 0.5):
        # Prepayment at par with some cost
        call_schedule[period] = 100
    
    mbs_bond = Bond(
        face_value=100,
        coupon_rate=0.045,  # Lower coupon rate typical of MBS
        maturity=10.0,
        frequency=12,  # Monthly payments
        call_schedule=call_schedule
    )
    
    market_price = 99.5
    
    print(f"MBS Details:")
    print(f"- Face Value: ${mbs_bond.face_value}")
    print(f"- Coupon Rate: {mbs_bond.coupon_rate:.2%}")
    print(f"- Maturity: {mbs_bond.maturity} years")
    print(f"- Payment Frequency: Monthly")
    print(f"- Market Price: ${market_price}")
    print(f"- Prepayment: Allowed every 6 months after year 1")
    
    # Calculate OAS with more paths due to complexity
    results = calculator.calculate_oas(mbs_bond, market_price, num_paths=10000)
    
    print(f"\nMBS OAS Analysis:")
    print(f"- Option Adjusted Spread: {results['oas_bps']:.0f} basis points")
    print(f"- Z-Spread (option-free): {results['z_spread_bps']:.0f} basis points")
    print(f"- Prepayment Cost: {results['option_cost_bps']:.0f} basis points")
    
    print(f"\nNote: This is a simplified MBS model. Real MBS pricing requires:")
    print(f"- Prepayment speed modeling (PSA, CPR)")
    print(f"- Credit risk modeling")
    print(f"- Pool composition analysis")
    print(f"- Servicing fee considerations")
    
    return results


def run_all_examples():
    """
    Run all OAS calculation examples
    """
    print("OPTION ADJUSTED SPREAD (OAS) CALCULATION EXAMPLES")
    print("=" * 60)
    print("This demonstration shows various applications of OAS calculations")
    print("for different types of bonds with embedded options.\n")
    
    # Set random seed for reproducible results
    np.random.seed(42)
    
    results = {}
    
    try:
        results['callable'] = example_1_callable_bond()
        results['putable'] = example_2_putable_bond()
        results['comparative'] = example_3_comparative_analysis()
        results['sensitivity'] = example_4_sensitivity_analysis()
        results['mbs'] = example_5_mortgage_backed_security()
        
        print("\n" + "=" * 60)
        print("SUMMARY OF ALL EXAMPLES")
        print("=" * 60)
        print("All examples completed successfully!")
        print("Key takeaways:")
        print("1. OAS isolates credit spread from option effects")
        print("2. Callable bonds have lower OAS (higher option cost)")
        print("3. Putable bonds have higher OAS (negative option cost)")
        print("4. Volatility significantly affects option values")
        print("5. MBS require sophisticated prepayment modeling")
        
    except Exception as e:
        print(f"Error in examples: {e}")
        import traceback
        traceback.print_exc()
    
    return results


if __name__ == "__main__":
    run_all_examples()
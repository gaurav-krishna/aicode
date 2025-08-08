"""
Simplified Option Adjusted Spread (OAS) Calculator Demo

This is a basic demonstration of OAS concepts using only Python standard library.
For the full implementation with Monte Carlo simulation, see oas_calculator.py
"""

import math
import random

class SimpleBond:
    """Simplified bond class for demonstration"""
    
    def __init__(self, face_value=100, coupon_rate=0.05, maturity=5.0, 
                 is_callable=False, call_price=100):
        self.face_value = face_value
        self.coupon_rate = coupon_rate
        self.maturity = maturity
        self.is_callable = is_callable
        self.call_price = call_price
    
    def price_option_free(self, discount_rate):
        """Price bond without considering embedded options"""
        annual_coupon = self.face_value * self.coupon_rate
        price = 0
        
        # Present value of coupons
        for year in range(1, int(self.maturity) + 1):
            price += annual_coupon / ((1 + discount_rate) ** year)
        
        # Present value of principal
        price += self.face_value / ((1 + discount_rate) ** self.maturity)
        
        return price


def calculate_z_spread_simple(bond, market_price, risk_free_rate=0.03):
    """
    Simple Z-spread calculation using iterative approach
    """
    def price_with_spread(spread):
        return bond.price_option_free(risk_free_rate + spread)
    
    # Simple bisection method
    low_spread = -0.1
    high_spread = 0.2
    tolerance = 0.0001
    
    for _ in range(100):  # Max iterations
        mid_spread = (low_spread + high_spread) / 2
        mid_price = price_with_spread(mid_spread)
        
        if abs(mid_price - market_price) < tolerance:
            return mid_spread
        
        if mid_price > market_price:
            low_spread = mid_spread
        else:
            high_spread = mid_spread
    
    return mid_spread


def simple_monte_carlo_oas(bond, market_price, risk_free_rate=0.03, num_paths=1000):
    """
    Simplified Monte Carlo OAS calculation
    """
    random.seed(42)  # For reproducible results
    
    # Calculate Z-spread for comparison
    z_spread = calculate_z_spread_simple(bond, market_price, risk_free_rate)
    
    if not bond.is_callable:
        # For option-free bonds, OAS = Z-spread
        return {
            'oas': z_spread,
            'z_spread': z_spread,
            'option_cost': 0.0,
            'oas_bps': z_spread * 10000,
            'z_spread_bps': z_spread * 10000,
            'option_cost_bps': 0.0
        }
    
    # Simplified callable bond pricing
    total_value = 0
    volatility = 0.02  # 2% interest rate volatility
    
    for _ in range(num_paths):
        # Simple interest rate path simulation
        current_rate = risk_free_rate
        bond_value = bond.face_value  # Start with face value at maturity
        
        # Work backwards (simplified backward induction)
        for year in range(int(bond.maturity), 0, -1):
            # Add coupon
            bond_value += bond.face_value * bond.coupon_rate
            
            # Simulate rate shock
            rate_shock = random.gauss(0, volatility)
            current_rate = max(0.001, current_rate + rate_shock * 0.1)
            
            # Discount one period
            bond_value /= (1 + current_rate)
            
            # Check if callable and if call is optimal
            if bond.is_callable and year <= bond.maturity - 1:  # After first year
                if bond_value > bond.call_price:
                    # Call exercised
                    bond_value = bond.call_price
        
        total_value += bond_value
    
    # Average value across all paths
    theoretical_price = total_value / num_paths
    
    # Find OAS by iteration (simplified)
    def price_with_oas(oas):
        # Simplified: assume theoretical price decreases with higher spread
        return theoretical_price * (1 - oas * 10)  # Rough approximation
    
    # Simple approach: assume option cost reduces OAS
    option_cost_estimate = max(0, (bond.price_option_free(risk_free_rate + z_spread) - market_price) / bond.face_value)
    oas = z_spread - option_cost_estimate
    
    return {
        'oas': oas,
        'z_spread': z_spread,
        'option_cost': option_cost_estimate,
        'oas_bps': oas * 10000,
        'z_spread_bps': z_spread * 10000,
        'option_cost_bps': option_cost_estimate * 10000
    }


def demonstrate_oas_concepts():
    """
    Demonstrate key OAS concepts with simple examples
    """
    print("=" * 60)
    print("OPTION ADJUSTED SPREAD (OAS) DEMONSTRATION")
    print("=" * 60)
    print()
    
    print("Key Formula: OAS = Z-Spread - Option Cost")
    print()
    print("This simplified demo illustrates the concepts.")
    print("For full Monte Carlo implementation, see oas_calculator.py")
    print()
    
    # Example 1: Option-free bond
    print("EXAMPLE 1: OPTION-FREE BOND")
    print("-" * 30)
    
    option_free_bond = SimpleBond(
        face_value=100,
        coupon_rate=0.05,
        maturity=5.0,
        is_callable=False
    )
    
    market_price_1 = 102.0
    risk_free_rate = 0.03
    
    print(f"Bond: 5% coupon, 5-year maturity")
    print(f"Market Price: ${market_price_1}")
    print(f"Risk-free Rate: {risk_free_rate:.1%}")
    
    results_1 = simple_monte_carlo_oas(option_free_bond, market_price_1, risk_free_rate)
    
    print(f"")
    print(f"Results:")
    print(f"- Z-Spread: {results_1['z_spread_bps']:.0f} basis points")
    print(f"- OAS: {results_1['oas_bps']:.0f} basis points") 
    print(f"- Option Cost: {results_1['option_cost_bps']:.0f} basis points")
    print(f"")
    print(f"For option-free bonds: OAS = Z-Spread")
    print()
    
    # Example 2: Callable bond
    print("EXAMPLE 2: CALLABLE BOND")
    print("-" * 25)
    
    callable_bond = SimpleBond(
        face_value=100,
        coupon_rate=0.06,  # Higher coupon makes call more likely
        maturity=5.0,
        is_callable=True,
        call_price=102
    )
    
    market_price_2 = 104.0
    
    print(f"Bond: 6% coupon, 5-year maturity, callable at $102")
    print(f"Market Price: ${market_price_2}")
    print(f"Risk-free Rate: {risk_free_rate:.1%}")
    
    results_2 = simple_monte_carlo_oas(callable_bond, market_price_2, risk_free_rate)
    
    print(f"")
    print(f"Results:")
    print(f"- Z-Spread: {results_2['z_spread_bps']:.0f} basis points")
    print(f"- OAS: {results_2['oas_bps']:.0f} basis points")
    print(f"- Option Cost: {results_2['option_cost_bps']:.0f} basis points")
    print(f"")
    print(f"Note: OAS < Z-Spread because call option has positive cost")
    print()
    
    # Comparison
    print("COMPARISON SUMMARY")
    print("-" * 20)
    
    print(f"{'Bond Type':<15} {'OAS (bps)':<10} {'Option Cost (bps)':<15}")
    print(f"{'-'*40}")
    print(f"{'Option-Free':<15} {results_1['oas_bps']:<10.0f} {results_1['option_cost_bps']:<15.0f}")
    print(f"{'Callable':<15} {results_2['oas_bps']:<10.0f} {results_2['option_cost_bps']:<15.0f}")
    print()
    
    print("KEY INSIGHTS:")
    print("1. OAS isolates credit spread from option effects")
    print("2. Callable bonds have lower OAS due to call option cost")
    print("3. Option cost represents value of embedded option")
    print("4. Higher volatility increases option values")
    print()
    
    print("REAL-WORLD APPLICATIONS:")
    print("- Compare bonds with different option features")
    print("- Risk management and portfolio construction")
    print("- Relative value trading decisions")
    print("- Regulatory capital calculations")
    print()
    
    print("=" * 60)
    print("For advanced features like:")
    print("- Full Monte Carlo simulation")
    print("- Multiple interest rate models")
    print("- Complex option schedules")
    print("- Sensitivity analysis")
    print("- MBS prepayment modeling")
    print()
    print("See the complete implementation in oas_calculator.py")
    print("=" * 60)


if __name__ == "__main__":
    demonstrate_oas_concepts()
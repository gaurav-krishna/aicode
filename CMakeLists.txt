cmake_minimum_required(VERSION 3.16)
project(ModernQuantLib VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Modern modular source files
set(SOURCES
    # Legacy sources (for backward compatibility)
    src/Date.cpp
    src/CashFlow.cpp
    
    # Modern architecture would have implementations here
    # (Header-only template library for now)
)

# Modern header files - organized by module
set(HEADERS
    # Main library header
    include/QuantLib.h
    
    # Type system
    include/QuantLib/Types/Quantity.h
    include/QuantLib/Types/Expected.h
    include/QuantLib/Types/Handle.h
    
    # Patterns
    include/QuantLib/Patterns/Observable.h
    include/QuantLib/Patterns/Observer.h
    include/QuantLib/Patterns/Singleton.h
    
    # Time module
    include/QuantLib/Time/Date.h
    include/QuantLib/Time/Calendar.h
    include/QuantLib/Time/DayCounter.h
    include/QuantLib/Time/Schedule.h
    
    # Math module
    include/QuantLib/Math/Array.h
    include/QuantLib/Math/Matrix.h
    include/QuantLib/Math/Interpolation.h
    include/QuantLib/Math/Statistics.h
    
    # Market data
    include/QuantLib/MarketData/Quote.h
    include/QuantLib/MarketData/YieldTermStructure.h
    include/QuantLib/MarketData/MarketDataService.h
    
    # Instruments
    include/QuantLib/Instruments/Instrument.h
    include/QuantLib/Instruments/Bond.h
    include/QuantLib/Instruments/FixedRateBond.h
    
    # Pricing engines
    include/QuantLib/PricingEngines/PricingEngine.h
    include/QuantLib/PricingEngines/DiscountingBondEngine.h
    
    # Cash flows
    include/QuantLib/CashFlows/CashFlow.h
    include/QuantLib/CashFlows/FixedRateCoupon.h
)

# Create header-only library target
add_library(ModernQuantLib INTERFACE)
target_include_directories(ModernQuantLib INTERFACE include)

# Create legacy library for backward compatibility
add_library(QuantLib_Legacy ${SOURCES})
target_include_directories(QuantLib_Legacy PUBLIC include)

# Modern bond example
add_executable(modern_bond_example examples/modern_bond_example.cpp)
target_link_libraries(modern_bond_example ModernQuantLib)
target_compile_features(modern_bond_example PRIVATE cxx_std_20)

# Legacy example (if needed)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/bond_example.cpp")
    add_executable(legacy_bond_example examples/bond_example.cpp)
    target_link_libraries(legacy_bond_example QuantLib_Legacy)
endif()

# Tests
add_executable(modern_tests tests/test_modern_features.cpp)
target_link_libraries(modern_tests ModernQuantLib)
target_compile_features(modern_tests PRIVATE cxx_std_20)

# Compiler flags for all targets
target_compile_options(ModernQuantLib INTERFACE -Wall -Wextra -O3)
if(TARGET QuantLib_Legacy)
    target_compile_options(QuantLib_Legacy PRIVATE -Wall -Wextra -O3)
endif()

# Enable concepts and coroutines for C++20
target_compile_features(ModernQuantLib INTERFACE cxx_std_20)

# Installation
install(TARGETS ModernQuantLib
        EXPORT ModernQuantLibTargets
        INCLUDES DESTINATION include)

install(DIRECTORY include/QuantLib
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")
#pragma once

// Main QuantLib header - includes all public interfaces
#include "QuantLib/Math/Array.h"
#include "QuantLib/Math/Matrix.h"
#include "QuantLib/Math/Interpolation.h"
#include "QuantLib/Math/Statistics.h"

#include "QuantLib/Time/Date.h"
#include "QuantLib/Time/Calendar.h"
#include "QuantLib/Time/DayCounter.h"
#include "QuantLib/Time/Schedule.h"

#include "QuantLib/Patterns/Observable.h"
#include "QuantLib/Patterns/Observer.h"
#include "QuantLib/Patterns/Singleton.h"

#include "QuantLib/Types/Quantity.h"
#include "QuantLib/Types/Expected.h"
#include "QuantLib/Types/Handle.h"

#include "QuantLib/MarketData/Quote.h"
#include "QuantLib/MarketData/YieldTermStructure.h"
#include "QuantLib/MarketData/MarketDataService.h"

#include "QuantLib/Instruments/Instrument.h"
#include "QuantLib/Instruments/Bond.h"
#include "QuantLib/Instruments/FixedRateBond.h"

#include "QuantLib/PricingEngines/PricingEngine.h"
#include "QuantLib/PricingEngines/DiscountingBondEngine.h"

#include "QuantLib/CashFlows/CashFlow.h"
#include "QuantLib/CashFlows/FixedRateCoupon.h"

// Convenience namespace alias
namespace ql = QuantLib;
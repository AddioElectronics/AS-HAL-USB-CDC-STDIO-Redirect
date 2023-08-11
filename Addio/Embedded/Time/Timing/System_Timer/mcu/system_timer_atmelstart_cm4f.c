#include "Addio/Embedded/Time/Timing/System_Timer/system_timer.h"
#include "check_mcu_core.h"

/*
*	Note:	If you are using the system timer for something else, you are still able to use the timing functions.
*			Just make sure you increment "system_timer_overflows" in your event handler,
*			and adjust "SYSTEM_TIMER_COUNTER_TOP," to your "SysTick->LOAD" value.
*			That is all that is required to get "millis" and "micros" function working.
*/

#if __has_include("RTE_Components.h") && __has_include("core_cm4.h") && IS_MCU_CORE_CM4F
#include <RTE_Components.h> //Contains ATMEL_START definition

#if defined(ATMEL_START)

/*
*	Include IC header
*/
//SAMD51
#if   defined(__SAMD51G18A__) || defined(__ATSAMD51G18A__)
#include "samd51g18a.h"
#elif defined(__SAMD51G19A__) || defined(__ATSAMD51G19A__)
#include "samd51g19a.h"
#elif defined(__SAMD51J18A__) || defined(__ATSAMD51J18A__)
#include "samd51j18a.h"
#elif defined(__SAMD51J19A__) || defined(__ATSAMD51J19A__)
#include "samd51j19a.h"
#elif defined(__SAMD51J19B__) || defined(__ATSAMD51J19B__)
#include "samd51j19b.h"
#elif defined(__SAMD51J20A__) || defined(__ATSAMD51J20A__)
#include "samd51j20a.h"
#elif defined(__SAMD51J20C__) || defined(__ATSAMD51J20C__)
#include "samd51j20c.h"
#elif defined(__SAMD51N19A__) || defined(__ATSAMD51N19A__)
#include "samd51n19a.h"
#elif defined(__SAMD51N20A__) || defined(__ATSAMD51N20A__)
#include "samd51n20a.h"
#elif defined(__SAMD51P19A__) || defined(__ATSAMD51P19A__)
#include "samd51p19a.h"
#elif defined(__SAMD51P20A__) || defined(__ATSAMD51P20A__)
#include "samd51p20a.h"

//SAME51
#elif   defined(__SAME51G18A__) || defined(__ATSAME51G18A__)
#include "same51g18a.h"
#elif defined(__SAME51G19A__) || defined(__ATSAME51G19A__)
#include "same51g19a.h"
#elif defined(__SAME51J18A__) || defined(__ATSAME51J18A__)
#include "same51j18a.h"
#elif defined(__SAME51J19A__) || defined(__ATSAME51J19A__)
#include "same51j19a.h"
#elif defined(__SAME51J20A__) || defined(__ATSAME51J20A__)
#include "same51j20a.h"
#elif defined(__SAME51N19A__) || defined(__ATSAME51N19A__)
#include "same51n19a.h"
#elif defined(__SAME51N20A__) || defined(__ATSAME51N20A__)
#include "same51n20a.h"

//SAME53
#elif   defined(__SAME53J18A__) || defined(__ATSAME53J18A__)
#include "same53j18a.h"
#elif defined(__SAME53J19A__) || defined(__ATSAME53J19A__)
#include "same53j19a.h"
#elif defined(__SAME53J20A__) || defined(__ATSAME53J20A__)
#include "same53j20a.h"
#elif defined(__SAME53N19A__) || defined(__ATSAME53N19A__)
#include "same53n19a.h"
#elif defined(__SAME53N20A__) || defined(__ATSAME53N20A__)
#include "same53n20a.h"

//SAMD54
#elif   defined(__SAME54N19A__) || defined(__ATSAME54N19A__)
#include "same54n19a.h"
#elif defined(__SAME54N20A__) || defined(__ATSAME54N20A__)
#include "same54n20a.h"
#elif defined(__SAME54P19A__) || defined(__ATSAME54P19A__)
#include "same54p19a.h"
#elif defined(__SAME54P20A__) || defined(__ATSAME54P20A__)
#include "same54p20a.h"

//#elif Add your IC here.

#else
#error unsupported
#endif

#include <core_cm4.h> //Requires IC header.
#include <peripheral_clk_config.h>

/*
*	The top value of the counter.
*/
#define SYSTEM_TIMER_COUNTER_TOP	SysTick_LOAD_RELOAD_Msk

/*
*	How many ticks equal a unit of time.
*/
#define US_TICKS (CONF_CPU_FREQUENCY/1000000)
#define MS_TICKS (CONF_CPU_FREQUENCY/1000)
//#define SEC_TICKS (CONF_CPU_FREQUENCY)
//#define MIN_TICKS (CONF_CPU_FREQUENCY * 60)


/*
*	How many times the system timer has overflowed.
*/
volatile uint64_t system_timer_overflows = 0;


void system_timer_init()
{
	SysTick->LOAD  = (uint32_t)(SYSTEM_TIMER_COUNTER_TOP);             /* set reload register */
	NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);  /* set Priority for Systick Interrupt */
	SysTick->VAL   = (uint32_t)(SYSTEM_TIMER_COUNTER_TOP);             /* Load the SysTick Counter Value */
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
	SysTick_CTRL_TICKINT_Msk   |
	SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick Int and SysTick Timer */
}


//void __attribute__((weak)) SysTick_Handler();	//Remove GCC compiler warning
void /*__attribute__((weak))*/ SysTick_Handler()
{
	system_timer_overflows++;
}


#pragma region Timing Functions


uint64_t millis()
{
	//Timer counts down, we must subtract
	return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / MS_TICKS;
}

uint64_t micros()
{
	return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / US_TICKS;
}

//uint64_t seconds()
//{
	////Timer counts down, we must subtract
	//return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / SEC_TICKS;
//}
//
//uint64_t minutes()
//{
	////Timer counts down, we must subtract
	//return ((SYSTEM_TIMER_COUNTER_TOP - SysTick->VAL) + (SYSTEM_TIMER_COUNTER_TOP * system_timer_overflows) ) / MIN_TICKS;
//}


#pragma endregion Timing Functions

#endif //defined(ATMEL_START)
#endif //__has_include("RTE_Components.h") && __has_include("core_cm0plus.h")
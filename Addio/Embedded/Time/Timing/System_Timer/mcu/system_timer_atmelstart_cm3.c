#include "Addio/Embedded/Time/Timing/System_Timer/system_timer.h"
#include "check_mcu_core.h"

/*
*	Note:	If you are using the system timer for something else, you are still able to use the timing functions.
*			Just make sure you increment "system_timer_overflows" in your event handler,
*			and adjust "SYSTEM_TIMER_COUNTER_TOP," to your "SysTick->LOAD" value.
*			That is all that is required to get "millis" and "micros" function working.
*/

#if __has_include("RTE_Components.h") && __has_include("core_cm3.h") && IS_MCU_CORE_CM3
#include <RTE_Components.h> //Contains ATMEL_START definition

#warning Cortex M3 Untested

#if defined(ATMEL_START)

/*
*	Include IC header
*/
//SAM3A
#if defined(__SAM3A4C__)
#include "sam3a4c.h"
#elif defined(__SAM3A8C__)
#include "sam3a8c.h"

//SAM3X
#elif defined(__SAM3X4C__)
#include "sam3x4c.h"
#elif defined(__SAM3X4E__)
#include "sam3x4e.h"
#elif defined(__SAM3X8C__)
#include "sam3x8c.h"
#elif defined(__SAM3X8E__)
#include "sam3x8e.h"
#elif defined(__SAM3X8H__)
#include "sam3x8h.h"

//SAM3N
#elif defined(__SAM3N00A__)
#include "sam3n00a.h"
#elif defined(__SAM3N0A__)
#include "sam3n0a.h"
#elif defined(__SAM3N00B__)
#include "sam3n00b.h"
#elif defined(__SAM3N0B__)
#include "sam3n0b.h"
#elif defined(__SAM3N0C__)
#include "sam3n0c.h"
#elif defined(__SAM3N1A__)
#include "sam3n1a.h"
#elif defined(__SAM3N1B__)
#include "sam3n1b.h"
#elif defined(__SAM3N1C__)
#include "sam3n1c.h"
#elif defined(__SAM3N2A__)
#include "sam3n2a.h"
#elif defined(__SAM3N2B__)
#include "sam3n2b.h"
#elif defined(__SAM3N2C__)
#include "sam3n2c.h"
#elif defined(__SAM3N4A__)
#include "sam3n4a.h"
#elif defined(__SAM3N4B__)
#include "sam3n4b.h"
#elif defined(__SAM3N4C__)
#include "sam3n4c.h"

//SAM3S
#elif defined(__SAM3S1A__)
#include "sam3s1a.h"
#elif defined(__SAM3S1B__)
#include "sam3s1b.h"
#elif defined(__SAM3S1C__)
#include "sam3s1c.h"
#elif defined(__SAM3S2A__)
#include "sam3s2a.h"
#elif defined(__SAM3S2B__)
#include "sam3s2b.h"
#elif defined(__SAM3S2C__)
#include "sam3s2c.h"
#elif defined(__SAM3S4A__)
#include "sam3s4a.h"
#elif defined(__SAM3S4B__)
#include "sam3s4b.h"
#elif defined(__SAM3S4C__)
#include "sam3s4c.h"


//SAM3SD
#elif defined(__SAM3SD8B__)
#include "sam3sd8b.h"
#elif defined(__SAM3SD8C__)
#include "sam3sd8c.h"
#elif defined(__SAM3S8B__)
#include "sam3s8b.h"
#elif defined(__SAM3S8C__)
#include "sam3s8c.h"

//SAM3U
#elif defined(__SAM3U1C__)
#include "sam3u1c.h"
#elif defined(__SAM3U1E__)
#include "sam3u1e.h"
#elif defined(__SAM3U2C__)
#include "sam3u2c.h"
#elif defined(__SAM3U2E__)
#include "sam3u2e.h"
#elif defined(__SAM3U4C__)
#include "sam3u4c.h"
#elif defined(__SAM3U4E__)
#include "sam3u4e.h"


//#elif Add your IC here.

#else
#error unsupported
#endif

#include <core_cm3.h> //Requires IC header.
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
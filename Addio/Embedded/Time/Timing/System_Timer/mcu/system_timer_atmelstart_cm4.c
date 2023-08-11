#include "Addio/Embedded/Time/Timing/System_Timer/system_timer.h"
#include "Addio/Embedded/Time/Timing/timing.h"
#include "check_mcu_core.h"

/*
*	Note:	If you are using the system timer for something else, you are still able to use the timing functions.
*			Just make sure you increment "system_timer_overflows" in your event handler,
*			and adjust "SYSTEM_TIMER_COUNTER_TOP," to your "SysTick->LOAD" value.
*			That is all that is required to get "millis" and "micros" function working.
*/

#if __has_include("RTE_Components.h") && __has_include("core_cm4.h") && IS_MCU_CORE_CM4
#include <RTE_Components.h> //Contains ATMEL_START definition

#warning Cortex M4 Untested

#if defined(ATMEL_START)

/*
*	Include IC header
*/
//SAM4C
#if ((defined __SAM4CP16B_0__) || (defined __SAM4CP16C_0__))
#include "sam4cp_0.h"
#elif ((defined __SAM4CP16B_1__) || (defined __SAM4CP16C_1__))
#include "sam4cp_1.h"

//SAM4E
#elif defined(__SAM4E8C__)
#include "sam4e8c.h"
#elif defined(__SAM4E8E__)
#include "sam4e8e.h"
#elif defined(__SAM4E16C__)
#include "sam4e16c.h"
#elif defined(__SAM4E16E__)
#include "sam4e16e.h"

//SAM4L
#elif   defined(__SAM4LC8A__) || defined(__ATSAM4LC8A__)
#include "sam4lc8a.h"
#elif defined(__SAM4LC8B__) || defined(__ATSAM4LC8B__)
#include "sam4lc8b.h"
#elif defined(__SAM4LC8C__) || defined(__ATSAM4LC8C__)
#include "sam4lc8c.h"
#elif defined(__SAM4LS8A__) || defined(__ATSAM4LS8A__)
#include "sam4ls8a.h"
#elif defined(__SAM4LS8B__) || defined(__ATSAM4LS8B__)
#include "sam4ls8b.h"
#elif defined(__SAM4LS8C__) || defined(__ATSAM4LS8C__)
#include "sam4ls8c.h"

//SAM4N

//SAM4S

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
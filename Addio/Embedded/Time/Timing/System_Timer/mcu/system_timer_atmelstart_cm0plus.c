#include "Addio/Embedded/Time/Timing/System_Timer/system_timer.h"
#include "Addio/Embedded/Time/Timing/timing.h"
#include "check_mcu_core.h"
/*
*	Note:	If you are using the system timer for something else, you are still able to use the timing functions.
*			Just make sure you increment "system_timer_overflows" in your event handler,
*			and adjust "SYSTEM_TIMER_COUNTER_TOP," to your "SysTick->LOAD" value.
*			That is all that is required to get "millis" and "micros" function working.
*/


#if __has_include("RTE_Components.h") && __has_include("core_cm0plus.h")  && IS_MCU_CORE_CM0PLUS
#include <RTE_Components.h> //Contains ATMEL_START definition
#if defined(ATMEL_START)

/*
*	Include IC header
*/
//SAMD21
#if   defined(__SAMD21E15A__) || defined(__ATSAMD21E15A__)
#include "samd21e15a.h"
#elif defined(__SAMD21E16A__) || defined(__ATSAMD21E16A__)
#include "samd21e16a.h"
#elif defined(__SAMD21E17A__) || defined(__ATSAMD21E17A__)
#include "samd21e17a.h"
#elif defined(__SAMD21E18A__) || defined(__ATSAMD21E18A__)
#include "samd21e18a.h"
#elif defined(__SAMD21G15A__) || defined(__ATSAMD21G15A__)
#include "samd21g15a.h"
#elif defined(__SAMD21G16A__) || defined(__ATSAMD21G16A__)
#include "samd21g16a.h"
#elif defined(__SAMD21G17A__) || defined(__ATSAMD21G17A__)
#include "samd21g17a.h"
#elif defined(__SAMD21G17AU__) || defined(__ATSAMD21G17AU__)
#include "samd21g17au.h"
#elif defined(__SAMD21G18A__) || defined(__ATSAMD21G18A__)
#include "samd21g18a.h"
#elif defined(__SAMD21G18AU__) || defined(__ATSAMD21G18AU__)
#include "samd21g18au.h"
#elif defined(__SAMD21J15A__) || defined(__ATSAMD21J15A__)
#include "samd21j15a.h"
#elif defined(__SAMD21J16A__) || defined(__ATSAMD21J16A__)
#include "samd21j16a.h"
#elif defined(__SAMD21J17A__) || defined(__ATSAMD21J17A__)
#include "samd21j17a.h"
#elif defined(__SAMD21J18A__) || defined(__ATSAMD21J18A__)
#include "samd21j18a.h"

//SAMDA1
#elif   defined(__SAMDA1E14A__) || defined(__ATSAMDA1E14A__)
#include "samda1e14a.h"
#elif defined(__SAMDA1E15A__) || defined(__ATSAMDA1E15A__)
#include "samda1e15a.h"
#elif defined(__SAMDA1E16A__) || defined(__ATSAMDA1E16A__)
#include "samda1e16a.h"
#elif defined(__SAMDA1G14A__) || defined(__ATSAMDA1G14A__)
#include "samda1g14a.h"
#elif defined(__SAMDA1G15A__) || defined(__ATSAMDA1G15A__)
#include "samda1g15a.h"
#elif defined(__SAMDA1G16A__) || defined(__ATSAMDA1G16A__)
#include "samda1g16a.h"
#elif defined(__SAMDA1J14A__) || defined(__ATSAMDA1J14A__)
#include "samda1j14a.h"
#elif defined(__SAMDA1J15A__) || defined(__ATSAMDA1J15A__)
#include "samda1j15a.h"
#elif defined(__SAMDA1J16A__) || defined(__ATSAMDA1J16A__)
#include "samda1j16a.h"

//SAML21
#elif   defined(__SAML21E18A__) || defined(__ATSAML21E18A__)
#include "saml21e18a.h"
#elif defined(__SAML21G18A__) || defined(__ATSAML21G18A__)
#include "saml21g18a.h"
#elif defined(__SAML21J18A__) || defined(__ATSAML21J18A__)
#include "saml21j18a.h"

//SAML22
#elif   defined(__SAML22G16A__) || defined(__ATSAML22G16A__)
#include "saml22g16a.h"
#elif defined(__SAML22G17A__) || defined(__ATSAML22G17A__)
#include "saml22g17a.h"
#elif defined(__SAML22G18A__) || defined(__ATSAML22G18A__)
#include "saml22g18a.h"
#elif defined(__SAML22J16A__) || defined(__ATSAML22J16A__)
#include "saml22j16a.h"
#elif defined(__SAML22J17A__) || defined(__ATSAML22J17A__)
#include "saml22j17a.h"
#elif defined(__SAML22J18A__) || defined(__ATSAML22J18A__)
#include "saml22j18a.h"
#elif defined(__SAML22N16A__) || defined(__ATSAML22N16A__)
#include "saml22n16a.h"
#elif defined(__SAML22N17A__) || defined(__ATSAML22N17A__)
#include "saml22n17a.h"
#elif defined(__SAML22N18A__) || defined(__ATSAML22N18A__)
#include "saml22n18a.h"

//#elif Add your IC here.

#else
#error unsupported
#endif

#include <core_cm0plus.h> //Requires IC header.
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
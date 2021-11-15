#ifndef SYSTEM_TIMER_H_
#define SYSTEM_TIMER_H_

#include <stdint.h>
#include <peripheral_clk_config.h>

#define US_TICKS (CONF_CPU_FREQUENCY/1000000)
#define MS_TICKS (CONF_CPU_FREQUENCY/1000)
//#define SEC_TICKS (CONF_CPU_FREQUENCY)
//#define MIN_TICKS (CONF_CPU_FREQUENCY * 60)

void system_timer_init();

//void system_timer_enable();
//void system_timer_disable();





#endif /* SYSTEM_TIMER_H_ */
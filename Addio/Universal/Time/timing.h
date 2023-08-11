#ifndef TIMING_H_
#define TIMING_H_

#include <stdint.h>
#include <stdbool.h>

bool has_time_elapsed_ms(unsigned long ms, unsigned long start_ms);
bool has_time_elapsed_us(unsigned long us, unsigned long start_us);
bool has_time_elapsed_sec(unsigned long sec, unsigned long start_ms);
bool has_time_elapsed_min(unsigned long min, unsigned long start_ms);

void convert_ms_to_time(unsigned long elapsed_ms, uint32_t* hr, uint32_t* min, uint32_t* sec, uint32_t* ms);

#endif /* TIMING_H_ */
#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include "utils.h"

static struct timeval _timers_dram[2 * NUM_BANKS];

#define GET_TIME_INIT(num) struct timeval _timers[num]

#define GET_TIME_VAL(num) gettimeofday(&_timers[num], NULL)

#define TIME_VAL_TO_MS(num) (((double)_timers[num].tv_sec*1000.0) + ((double)_timers[num].tv_usec/1000.0))

#define GET_TIME_VAL_DRAM(num) gettimeofday(&_timers_dram[num], NULL)

#define TIME_VAL_TO_MS_DRAM(num) (((double)_timers_dram[num].tv_sec*1000.0) + ((double)_timers_dram[num].tv_usec/1000.0))

#endif

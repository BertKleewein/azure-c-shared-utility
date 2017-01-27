// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/tickcounter_msp430.h"

#ifdef __cplusplus
  #include <cstdlib>
#else
  #include <stdlib.h>
#endif

#ifdef _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
#endif

#include <driverlib.h>

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"

#ifdef _MSC_VER
  #pragma warning(disable:4068)
#endif

typedef struct tick_t {
    tickcounter_ms_t tick_count;
    uint16_t tick_overflows;
} tick_t;

typedef struct timerA_t {
    uint16_t counter_value;
    tickcounter_ms_t counter_overflows;
} timerA_t;

typedef union tickcount_t {
    tick_t tick;
    timerA_t timer_a;
} tickcount_t;

static tickcount_t system_ticks;

/*
* The value of `CS_getSMCLK()` divided by the value
* of the `clockSourceDivider` register of Timer A3
*/
static tickcounter_ms_t timer_a3_ticks_per_second;

/******************************************************************************
* Interrupt to signal overflow of timer counter
******************************************************************************/
#pragma vector = TIMER3_A1_VECTOR
__interrupt void TIMER3_A1_ISR(void)
{
    switch (__even_in_range(TA3IV, 14))
    {
    case TA3IV_NONE: break;
    case TA3IV_TACCR1: break;
    case TA3IV_3: break;
    case TA3IV_4: break;
    case TA3IV_5: break;
    case TA3IV_6: break;
    case TA3IV_TAIFG:
        ++system_ticks.timer_a.counter_overflows;
        break;
    default: __never_executed();
    }
}


static inline
tickcounter_ms_t
now_ms (
    void
) {
    return ((tickcounter_ms_t)(((system_ticks.timer_a.counter_overflows / (float)timer_a3_ticks_per_second) * 65536000.0) + ((system_ticks.timer_a.counter_value / (float)timer_a3_ticks_per_second) * 1000.0)));
}


TICK_COUNTER_HANDLE tickcounter_create(void)
{
    tickcounter_ms_t * creation_offset_ms;

    if (NULL == (creation_offset_ms = (tickcounter_ms_t *)malloc(sizeof(tickcounter_ms_t)))) {
        // Insufficient memory
    } else {
        system_ticks.timer_a.counter_value = Timer_A_getCounterValue(TIMER_A3_BASE);
        *creation_offset_ms = now_ms();
    }

    return (TICK_COUNTER_HANDLE)creation_offset_ms;
}


void timer_a3_deinit(void)
{
    Timer_A_disableInterrupt(TIMER_A3_BASE);
    Timer_A_stop(TIMER_A3_BASE);
}


void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (NULL == tick_counter) {
        LogError("NULL handle passed to `tickcounter_destroy`");
    } else {
        free(tick_counter);
    }
}


int timer_a3_init(void)
{
    const size_t minimum_Hz = 16000;
    size_t aclk_Hz = 0;
    int error;

    // Ensure the ACLK is available to the Timer A3 module
    CS_enableClockRequest(CS_ACLK);

    // Update `timer_a3_ticks_per_second` with actual ticks per second
    // divided by the `.clockSourceDivider` value provided below
    if (minimum_Hz > (aclk_Hz = CS_getACLK())) {
        error = __LINE__;
    } else {
        timer_a3_ticks_per_second = (aclk_Hz >> 4);
        Timer_A_initContinuousModeParam param = {
            .clockSource = TIMER_A_CLOCKSOURCE_ACLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_16,
            .timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE,
            .timerClear = TIMER_A_SKIP_CLEAR,
            .startTimer = true,
        };
        Timer_A_initContinuousMode(TIMER_A3_BASE, &param);
        error = 0;
    }

    return error;
}


int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    int error;

    if (NULL == tick_counter) {
        error = __LINE__;
    } else if (NULL == current_ms) {
        error = __LINE__;
    } else {
        system_ticks.timer_a.counter_value = Timer_A_getCounterValue(TIMER_A3_BASE);
        *current_ms = (now_ms() - *(tickcounter_ms_t *)tick_counter);
        error = 0;
    }
    return error;
}


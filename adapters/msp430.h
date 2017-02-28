// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MSP430_H
#define MSP430_H

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "stdint.h"

// BKTODO: we can do a much better job of pulling out SHA code!

#ifdef __cplusplus
  #include <cstdbool>
  extern "C" {
#else
  #include <stdbool.h>
#endif /* __cplusplus */

MOCKABLE_FUNCTION(, int, msp430_sleep, tickcounter_ms_t, sleepTimeMs);
MOCKABLE_FUNCTION(, int, msp430_turn_on_sim808);
MOCKABLE_FUNCTION(, int, msp430_turn_off_sim808);
MOCKABLE_FUNCTION(, int, msp430_power_cycle_sim808);
MOCKABLE_FUNCTION(, int, msp430_exit_sim808_data_mode);
MOCKABLE_FUNCTION(, bool, msp_430_is_sim808_powered_up);

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* MSP430_H */


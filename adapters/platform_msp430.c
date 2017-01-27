// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"

#ifdef __cplusplus
  #include <cstddef>
#else
  #include <stddef.h>
#endif // __cplusplus

#include <driverlib.h>

#include "azure_c_shared_utility/tickcounter_msp430.h"

#ifdef _MSC_VER
#pragma warning(disable:4068)
#endif

int platform_init (void) {
    int error;
    tickcounter_ms_t mark_ms, current_ms;
    TICK_COUNTER_HANDLE tick_counter;

    if (0 != timer_a3_init()) {
        error = __LINE__;
    } else if ( NULL == (tick_counter = tickcounter_create()) ) {
        error = __LINE__;
    } else {
        /*
         * Port 3 pin 5 is connected to the Sim808 Status pin.
         * HIGH on the pin indicates modem is ON, otherwise OFF.
         */
        GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5);

        /*
         * Port 4 pin 6 is connected to GSM POWER KEY pin.
         * A 1 second HIGH pulse will turn ON/OFF the modem.
         */
        GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);

        /*
         * The corresponding pin on the Sim808 has a built-in pulldown
         * resistor, so we set the pin to the resting position or LOW.
         */
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

        /*
         * The Sim808 status pin reflects the status of the SIM808
         * If the device is powered down, the value is GPIO_INPUT_PIN_LOW.
         * Otherwise if the device is powered up, the value is GPIO_INPUT_PIN_HIGH
         */
        if (GPIO_INPUT_PIN_HIGH == GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5)) {
            // Sim808 is already enabled
            error = 0;
        } else {
            /*
             * The Sim808 must be powered on for 550ms before
             * it is ready to receive any interaction.
             *
             * NOTE: During the creation of a tickcounter, a millisecond
             *       snapshot is captured and used as an offset.
             */
            for (error = current_ms = 0 ; (0 == error) && (550 >= current_ms) ; error = tickcounter_get_current_ms(tick_counter, &current_ms));
            if (0 != error) {
                error = __LINE__;
            } else {
                // Send a HIGH pulse to the PWRKEY pin to signal the Sim808 to wake.
                GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);

                // The pulse must be at least 1 second long
                for (mark_ms = current_ms; (0 == error) && ((1100 + mark_ms) >= current_ms); error = tickcounter_get_current_ms(tick_counter, &current_ms));
                if (0 != error) {
                    error = __LINE__;
                } else {
                    /*
                    * The corresponding pin on the Sim808 has a built-in pulldown
                    * resistor, so we set the pin to the resting position or LOW.
                    */
                    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

                    // Ensure Sim808 is ready before exit
                    for (; GPIO_INPUT_PIN_HIGH != GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5) ;);
                }
            }
        }

        // Configure the TX/RX lines (port 2, pins 5 and 6) for communication with the SIM808
        GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, (GPIO_PIN5 | GPIO_PIN6), GPIO_SECONDARY_MODULE_FUNCTION);

        tickcounter_destroy(tick_counter);
    }

    return error;
}


void platform_deinit (void) {
    TICK_COUNTER_HANDLE tick_counter;
    tickcounter_ms_t current_ms;

    if (NULL != (tick_counter = tickcounter_create())) {
        /*
         * Port 3 pin 5 is connected to the Sim808 Status pin.
         * HIGH on the pin indicates modem is ON, otherwise OFF.
         */
        GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5);

        /*
         * Port 4 pin 6 is connected to GSM POWER KEY pin.
         * A 1 second HIGH pulse will turn ON/OFF the modem.
         */
        GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);

        /*
         * The corresponding pin on the Sim808 has a built-in pulldown
         * resistor, so we set the pin to the resting position or LOW.
         */
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

        /*
         * The Sim808 status pin reflects the status of the SIM808
         * If the device is powered down, the value is GPIO_INPUT_PIN_LOW.
         * Otherwise if the device is powered up, the value is GPIO_INPUT_PIN_HIGH
         */
        if (GPIO_INPUT_PIN_HIGH == GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5)) {
            int error;

            // Send a HIGH pulse to the PWRKEY pin to signal the Sim808 to wake.
            GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);

            /*
             * The Sim808 must be powered on for 550ms before
             * it is ready to receive any interaction.
             *
             * NOTE: During the creation of a tickcounter, a millisecond
             *       snapshot is captured and used as an offset.
             */
            for (error = current_ms = 0; (0 == error) && (1100 >= current_ms); error = tickcounter_get_current_ms(tick_counter, &current_ms));
            if (0 != error) {
                error = __LINE__;
            } else {
                /*
                 * The corresponding pin on the Sim808 has a built-in pulldown
                 * resistor, so we set the pin to the resting position or LOW.
                 */
                GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

                // Ensure Sim808 is ready before exit
                for (; GPIO_INPUT_PIN_LOW != GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5) ;);
            }
        }

        tickcounter_destroy(tick_counter);
    }
    timer_a3_deinit();
}


const IO_INTERFACE_DESCRIPTION * platform_get_default_tlsio (void) {
	return (const IO_INTERFACE_DESCRIPTION *)NULL;
}


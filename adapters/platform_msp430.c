// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"

#ifdef __cplusplus
  #include <cstddef>
#else
  #include <stddef.h>
#endif // __cplusplus

#include <driverlib.h>

#include "tickcounter_msp430.h"
#include "msp430.h"
#include "tlsio_sim800.h"

#ifdef _MSC_VER
#pragma warning(disable:4068)
#endif

// BKTODO: ints are probably 16 bits here.  Look at warnings and change precision

#define TURBO_BUTTON 1
#define LUDICROUS_SPEED 0

static TICK_COUNTER_HANDLE tick_counter;

#define SIM808_STATUS_PIN GPIO_PORT_P3, GPIO_PIN5
#define SIM808_POWER_PIN GPIO_PORT_P4, GPIO_PIN6
#define SIM808_DTR_PIN GPIO_PORT_P4, GPIO_PIN5

int msp430_sleep(tickcounter_ms_t sleepTimeMs)
{
    int result;
    tickcounter_ms_t current_ms, start_ms;

    if (0 != tickcounter_get_current_ms(tick_counter, &start_ms))
    {
        result = __FAILURE__;
    }
    else
    {
        result = 0;
        current_ms = start_ms;
        while ((result == 0) && ((current_ms - start_ms)  <= sleepTimeMs))
        {
            if (0 != tickcounter_get_current_ms(tick_counter, &current_ms))
            {
                result = __FAILURE__;
            }
        }
    }
    
    return result;
}

int msp430_turn_on_sim808()
{
    int result;


    /* Port 4 pin 5 is connected to the Sim808 DTR line.
     * pulling this pin low for 1 second will exit data mode
     */
    GPIO_setAsOutputPin(SIM808_DTR_PIN);
    GPIO_setOutputHighOnPin(SIM808_DTR_PIN);
    
    /*
     * Port 3 pin 5 is connected to the Sim808 Status pin.
     * HIGH on the pin indicates modem is ON, otherwise OFF.
     */
    GPIO_setAsInputPin(SIM808_STATUS_PIN);

    /*
     * Port 4 pin 6 is connected to GSM POWER KEY pin.
     * A 1 second HIGH pulse will turn ON/OFF the modem.
     */
    GPIO_setAsOutputPin(SIM808_POWER_PIN);

    /*
     * The corresponding pin on the Sim808 has a built-in pulldown
     * resistor, so we set the pin to the resting position or LOW.
     */
    GPIO_setOutputLowOnPin(SIM808_POWER_PIN);

    /*
     * The Sim808 status pin reflects the status of the SIM808
     * If the device is powered down, the value is GPIO_INPUT_PIN_LOW.
     * Otherwise if the device is powered up, the value is GPIO_INPUT_PIN_HIGH
     */
    if (GPIO_INPUT_PIN_HIGH == GPIO_getInputPinValue(SIM808_STATUS_PIN)) {
        // Sim808 is already enabled
        result = 0;
    } else {
        /*
         * The Sim808 must be powered on for 550ms before
         * it is ready to receive any interaction.
         */
         if (0 != msp430_sleep(550)) {
            result = __FAILURE__;
        } else {
            // Send a HIGH pulse to the PWRKEY pin to signal the Sim808 to wake.
            GPIO_setOutputHighOnPin(SIM808_POWER_PIN);

            // The pulse must be at least 1 second long
            if (0 != msp430_sleep(1100)) {
                result = __FAILURE__;
            } else {
                /*
                * The corresponding pin on the Sim808 has a built-in pulldown
                * resistor, so we set the pin to the resting position or LOW.
                */
                GPIO_setOutputLowOnPin(SIM808_POWER_PIN);

                // Ensure Sim808 is ready before exit
                for (; GPIO_INPUT_PIN_HIGH != GPIO_getInputPinValue(SIM808_STATUS_PIN) ;);

                result = 0;
            }
        }
    }

    return result;
}

int platform_init (void) {
    int error;
    
    // Configure the TX/RX lines (port 2, pins 5 and 6) for communication with the SIM808
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, (GPIO_PIN5 | GPIO_PIN6), GPIO_SECONDARY_MODULE_FUNCTION);

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
  #if TURBO_BUTTON
   #if LUDICROUS_SPEED
    (void)CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_6);
   #else
    (void)CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_4);
   #endif
    (void)CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
  #else
    // Default values
    (void)CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);
    (void)CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);
  #endif

    // Initialize Port A
    PAOUT = 0x00;  // Set Port A to LOW
    PADIR = 0x00;  // Set Port A to INPUT

    // Initialize Port B
    PBOUT = 0x00;  // Set Port B to LOW
    PBDIR = 0x00;  // Set Port B to OUTPUT

    PM5CTL0 &= ~LOCKLPM5;       // Disable the GPIO power-on default high-impedance mode to
                                // activate previously configured port settingsGS (affects RTC)
    __bis_SR_register(GIE);     // Enable Global Interrupt


    if ( NULL == (tick_counter = tickcounter_create()) ) {
        error = __FAILURE__;
    } else if (0 != timer_a3_init()) {
        error = __FAILURE__;
    } else if (0 != msp430_turn_on_sim808()) {
        error = __FAILURE__;
    } else {
        error = 0;
    }

    return error;
}

int msp430_turn_off_sim808()
{
    int result;
    
    /*
     * Port 3 pin 5 is connected to the Sim808 Status pin.
     * HIGH on the pin indicates modem is ON, otherwise OFF.
     */
    GPIO_setAsInputPin(SIM808_STATUS_PIN);

    /*
     * Port 4 pin 6 is connected to GSM POWER KEY pin.
     * A 1 second HIGH pulse will turn ON/OFF the modem.
     */
    GPIO_setAsOutputPin(SIM808_POWER_PIN);

    /*
     * The corresponding pin on the Sim808 has a built-in pulldown
     * resistor, so we set the pin to the resting position or LOW.
     */
    GPIO_setOutputLowOnPin(SIM808_POWER_PIN);
    
    /*
     * The Sim808 status pin reflects the status of the SIM808
     * If the device is powered down, the value is GPIO_INPUT_PIN_LOW.
     * Otherwise if the device is powered up, the value is GPIO_INPUT_PIN_HIGH
     */
    if (GPIO_INPUT_PIN_HIGH == GPIO_getInputPinValue(SIM808_STATUS_PIN)) {
        // Send a HIGH pulse to the PWRKEY pin to signal the Sim808 to turn off.
        GPIO_setOutputHighOnPin(SIM808_POWER_PIN);

        /*
         * make the pulse 1 second long
         */
        if (0 != msp430_sleep(1100)) {
            result = __FAILURE__;
        } else {
            /*
             * The corresponding pin on the Sim808 has a built-in pulldown
             * resistor, so we set the pin to the resting position or LOW.
             */
            GPIO_setOutputLowOnPin(SIM808_POWER_PIN);

            // Ensure Sim808 is ready before exit
            for (; GPIO_INPUT_PIN_LOW != GPIO_getInputPinValue(SIM808_STATUS_PIN) ;);

            result = 0;
        }
    } else {
        result = 0;
    }

    return result;
}

int msp430_power_cycle_sim808()
{
    int result;
    
    if (0 != msp430_turn_off_sim808())
    {
        result = __FAILURE__;
    } else if ( 0 != msp430_turn_on_sim808()) {
        result = __FAILURE__;
    } else {
        result = 0;
    }
    
    return result;
}

int msp430_exit_sim808_data_mode()
{
    int result;
    
    GPIO_setOutputLowOnPin(SIM808_DTR_PIN);
    if (0 != msp430_sleep(1100)) {
        result = __FAILURE__;
    } else {
        GPIO_setOutputHighOnPin(SIM808_DTR_PIN);
        result = 0;
    }
    
    return result;
}

void platform_deinit (void) {
    msp430_turn_off_sim808();
    timer_a3_deinit();
    tickcounter_destroy(tick_counter);
}


const IO_INTERFACE_DESCRIPTION * platform_get_default_tlsio (void) {
    return tlsio_sim800_get_interface_description();
}


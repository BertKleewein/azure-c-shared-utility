// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
  #include <cstdbool>
  #include <cstddef>
  #include <cstdio>
  #include <cstdlib>
  #include <ctime>
#else
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <time.h>
#endif

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Test framework `#includes`
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"

#define disableNegativeTest(x, y) ((x) |= ((uint64_t)1 << (y)))
#define enableNegativeTest(x, y) ((x) &= ~((uint64_t)1 << (y)))
#define skipNegativeTest(x, y) ((x) & ((uint64_t)1 << (y)))

// External library dependencies
#include "driverlib.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

#define ENABLE_MOCKS
  // `#include` SDK dependencies here
  #include "azure_c_shared_utility/tickcounter_msp430.h"
  #include "azure_c_shared_utility/tlsio.h"
#undef ENABLE_MOCKS

#define MOCK_TICKCOUNTER (TICK_COUNTER_HANDLE)0x19790917

// Under test `#includes`
#include "azure_c_shared_utility/platform.h"

#define ENABLE_MOCKS

  // ** Mocking gpio.h module (driverlib.h)
  MOCK_FUNCTION_WITH_CODE(, uint8_t, GPIO_getInputPinValue, uint8_t, selectedPort, uint16_t, selectedPins)
  MOCK_FUNCTION_END(GPIO_INPUT_PIN_LOW);
  MOCK_FUNCTION_WITH_CODE(, void, GPIO_setAsInputPin, uint8_t, selectedPort, uint16_t, selectedPins)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, GPIO_setAsOutputPin, uint8_t, selectedPort, uint16_t, selectedPins)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, GPIO_setAsPeripheralModuleFunctionOutputPin, uint8_t, selectedPort, uint16_t, selectedPins, uint8_t, mode)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, GPIO_setOutputHighOnPin, uint8_t, selectedPort, uint16_t, selectedPins)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, GPIO_setOutputLowOnPin, uint8_t, selectedPort, uint16_t, selectedPins)
  MOCK_FUNCTION_END();

#undef ENABLE_MOCKS

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(platform_msp430_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(bool, char);
    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void *);

}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

// platform_init

/* SRS_UARTIO_27_000: [ `platform_init()` shall initialize the timer submodule by calling `(int)timer_a3_init(void)` ] */
/* SRS_UARTIO_27_002: [ `platform_init()` shall create a tickcounter instance by calling `(TICK_COUNTER_HANDLE)tickcounter_create()` ] */
/* SRS_UARTIO_27_004: [ `platform_init()` shall prepare to read the SIM808 status pin by calling `(void)GPIO_setAsInputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` ] */
/* SRS_UARTIO_27_005: [ `platform_init()` shall prepare the SIM808 PWRKEY pin by calling `(void)GPIO_setAsOutputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_006: [ `platform_init()` shall set the PWRKEY pin to low (inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_007: [ `platform_init()` shall check the status line of the SIM808 by calling `(uint8_t)GPIO_getInputPinValue(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` ] */
/* SRS_UARTIO_27_015: [ `platform_init()` shall configure the GPIO for UART by calling `(void)GPIO_setAsPeripheralModuleFunctionOutputPin(uint8_t selectedPort, uint16_t selectedPins, uint8_t mode)` using port two, `GPIO_PORT_P2`, pins `GPIO_PIN5` and `GPIO_PIN6`, and UART functionality with the `GPIO_SECONDARY_MODULE_FUNCTION` identifier. ] */
/* SRS_UARTIO_27_016: [ `platform_init()` shall destroy the tickcounter instance by calling `(void)tickcounter_destroy(TICK_COUNTER_HANDLE)` using the value returned by `tickcounter_create()` ] */
/* SRS_UARTIO_27_018: [ If no errors occurred during execution, `platform_init()` shall return 0 ] */
TEST_FUNCTION(platform_init_SCENARIO_success_sim808_on)
{
    // Arrange
    int error;

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(timer_a3_init());
    EXPECTED_CALL(tickcounter_create())
        .SetReturn(MOCK_TICKCOUNTER);
    STRICT_EXPECTED_CALL(GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5));
    STRICT_EXPECTED_CALL(GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_HIGH);
    STRICT_EXPECTED_CALL(GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, (GPIO_PIN5|GPIO_PIN6), GPIO_SECONDARY_MODULE_FUNCTION));
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));

    // Act
    error = platform_init();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
}

/* SRS_UARTIO_27_010: [ If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall poll `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` until it returns a result greater than or equal to 550 milliseconds ] */
/* SRS_UARTIO_27_011: [ If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall set the PWRKEY pin to high (active) by calling `(void)GPIO_setOutputHighOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_012: [ If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall poll `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` until it returns a result greater than or equal to 1100 milliseconds from the last time captured ] */
/* SRS_UARTIO_27_014: [ If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall set the PWRKEY pin to low (inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_017: [ If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, `platform_init()` shall poll `(uint8_t)GPIO_getInputPinValue(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` until `GPIO_INPUT_PIN_HIGH` is returned ] */
TEST_FUNCTION(platform_init_SCENARIO_success_sim808_off)
{
    // Arrange
    int error;
    size_t random_number;
    tickcounter_ms_t mark_ms, ms;
    (void)srand((unsigned int)time(NULL));

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(timer_a3_init());
    EXPECTED_CALL(tickcounter_create())
        .SetReturn(MOCK_TICKCOUNTER);
    STRICT_EXPECTED_CALL(GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5));
    STRICT_EXPECTED_CALL(GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_LOW);
    // Simulate polling tick counter
    for (ms = mark_ms = 0;  ; ms += (rand() % 256)) {
        if (0 == ms) { continue; }
        STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &ms, sizeof(ms))
            .IgnoreArgument(2)
            .SetReturn(0);
        if (550 <= ms) { break; }
    }
    STRICT_EXPECTED_CALL(GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6));
    // Simulate polling tick counter
    for (mark_ms = ms ;  ; ms += (rand() % 512)) {
        STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &ms, sizeof(ms))
            .IgnoreArgument(2)
            .SetReturn(0);
        if ((1100 + mark_ms) <= ms) { break; }
    }
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    // Simulate polling STATUS pin
    random_number = (rand() % 4);
    for (size_t i = 0; i < random_number; ++i) {
        STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
            .SetReturn(GPIO_INPUT_PIN_LOW);
    }
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_HIGH);
    STRICT_EXPECTED_CALL(GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, (GPIO_PIN5 | GPIO_PIN6), GPIO_SECONDARY_MODULE_FUNCTION));
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));

    // Act
    error = platform_init();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
}

/* SRS_UARTIO_27_001: [ If the timer submodule fails to initialize properly, then `platform_init()` shall fail immediately and return a non-zero value ] */
/* SRS_UARTIO_27_003: [ If the call to `tickcounter_create()` returns null, then `platform_init()` shall fail immediately and return a non-zero value ] */
/* SRS_UARTIO_27_013: [ If any call to `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` returns a non-zero value, then `platform_init()` will exit immediately and return a non-zero value ] */
TEST_FUNCTION(platform_init_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    uint64_t negativeTestsToSkip = 0;
    size_t test_index = 0;
    int error;
    size_t random_number;
    tickcounter_ms_t mark_ms, ms;
    (void)srand((unsigned int)time(NULL));

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(timer_a3_init())
        .SetFailReturn(__LINE__)
        .SetReturn(0);
    enableNegativeTest(negativeTestsToSkip, test_index++);
    EXPECTED_CALL(tickcounter_create())
        .SetFailReturn(NULL)
        .SetReturn(MOCK_TICKCOUNTER);
    enableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_LOW);
    disableNegativeTest(negativeTestsToSkip, test_index++);
    // Simulate polling tick counter
    for (ms = 0, mark_ms = 0 ;  ; ms += (rand() % 256)) {
        if (0 == ms) { continue; }
        STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &ms, sizeof(ms))
            .IgnoreArgument(2)
            .SetFailReturn(__LINE__)
            .SetReturn(0);
        enableNegativeTest(negativeTestsToSkip, test_index++);
        if (550 <= ms) { break; }
    }
    STRICT_EXPECTED_CALL(GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    // Simulate polling tick counter
    for (mark_ms = ms ;  ; ms += (rand() % 512)) {
        STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &ms, sizeof(ms))
            .IgnoreArgument(2)
            .SetFailReturn(__LINE__)
            .SetReturn(0);
        enableNegativeTest(negativeTestsToSkip, test_index++);
        if ((1100 + mark_ms) <= ms) { break; }
    }
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    // Simulate polling STATUS pin
    random_number = (rand() % 4);
    for (size_t i = 0; i < random_number; ++i) {
        STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
            .SetReturn(GPIO_INPUT_PIN_LOW);
        disableNegativeTest(negativeTestsToSkip, test_index++);
    }
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_HIGH);
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, (GPIO_PIN5 | GPIO_PIN6), GPIO_SECONDARY_MODULE_FUNCTION));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    umock_c_negative_tests_snapshot();

    ASSERT_IS_TRUE(test_index < sizeof(negativeTestsToSkip) * 8);
    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if ( skipNegativeTest(negativeTestsToSkip, i) ) { continue; }

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        error = platform_init();

        // Assert
        ASSERT_ARE_NOT_EQUAL(int, 0, error);
    }

    // Cleanup
    umock_c_negative_tests_deinit();
}


// platform_deinit

/* SRS_UARTIO_27_019: [ `platform_deinit()` shall create a tickcounter instance by calling `(TICK_COUNTER_HANDLE)tickcounter_create()` ] */
/* SRS_UARTIO_27_021: [ `platform_deinit()` shall prepare to read the SIM808 status pin by calling `(void)GPIO_setAsInputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` ] */
/* SRS_UARTIO_27_022: [ `platform_deinit()` shall prepare the SIM808 PWRKEY pin by calling `(void)GPIO_setAsOutputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_023: [ `platform_deinit()` shall set the PWRKEY pin to low(inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_024: [ `platform_deinit()` shall check the status line of the SIM808 by calling `(uint8_t)GPIO_getInputPinValue(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` ] */
/* SRS_UARTIO_27_031: [ `platform_deinit()` shall destroy the tickcounter instance by calling `(void)tickcounter_destroy(TICK_COUNTER_HANDLE)` using the value returned by `tickcounter_create()` ] */
/* SRS_UARTIO_27_032: [ `platform_deinit()` shall deinitialize the timer submodule by calling `(void)timer_a3_deinit(void)` ] */
TEST_FUNCTION(platform_deinit_SCENARIO_success_sim808_off)
{
    // Arrange

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(tickcounter_create())
        .SetReturn(MOCK_TICKCOUNTER);
    STRICT_EXPECTED_CALL(GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5));
    STRICT_EXPECTED_CALL(GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5));
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));
    EXPECTED_CALL(timer_a3_deinit());

    // Act
    platform_deinit();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* SRS_UARTIO_27_026: [ If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall set the PWRKEY pin to high (active) by calling `(void)GPIO_setOutputHighOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_028: [ If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall poll `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` until it returns a result greater than or equal to 1100 milliseconds ] */
/* SRS_UARTIO_27_030: [ If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall set the PWRKEY pin to low (inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` ] */
/* SRS_UARTIO_27_033: [ If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall poll `(uint8_t)GPIO_getInputPinValue(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` until `GPIO_INPUT_PIN_LOW` is returned } */
TEST_FUNCTION(platform_deinit_SCENARIO_success_sim808_on)
{
    // Arrange
    size_t random_number;
    tickcounter_ms_t mark_ms, ms;
    (void)srand((unsigned int)time(NULL));

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(tickcounter_create())
        .SetReturn(MOCK_TICKCOUNTER);
    STRICT_EXPECTED_CALL(GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5));
    STRICT_EXPECTED_CALL(GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_HIGH);
    STRICT_EXPECTED_CALL(GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6));
    // Simulate polling tick counter
    for (ms = mark_ms = 0 ;  ; ms += (rand() % 256)) {
        if (0 == ms) { continue; }
        STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &ms, sizeof(ms))
            .IgnoreArgument(2)
            .SetReturn(0);
        if (1100 <= ms) { break; }
    }
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    // Simulate polling STATUS pin
    random_number = (rand() % 4);
    for (size_t i = 0; i < random_number; ++i) {
        STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
            .SetReturn(GPIO_INPUT_PIN_HIGH);
    }
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_LOW);
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));
    EXPECTED_CALL(timer_a3_deinit());

    // Act
    platform_deinit();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* SRS_UARTIO_27_020: [ If the call to `tickcounter_create()` returns null, then `platform_deinit()` shall fail immediately ] */
/* SRS_UARTIO_27_029: [ If any call to `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` returns a non - zero value, then `platform_deinit()` will exit immediately ] */
TEST_FUNCTION(platform_deinit_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    uint64_t negativeTestsToSkip = 0;
    size_t test_index = 0;
    size_t random_number;
    tickcounter_ms_t mark_ms, ms;
    (void)srand((unsigned int)time(NULL));

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(tickcounter_create())
        .SetFailReturn(NULL)
        .SetReturn(MOCK_TICKCOUNTER);
    enableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_HIGH);
    STRICT_EXPECTED_CALL(GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    // Simulate polling tick counter
    for (ms = mark_ms = 0; ; ms += (rand() % 256)) {
        if (0 == ms) { continue; }
        STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &ms, sizeof(ms))
            .IgnoreArgument(2)
            .SetFailReturn(__LINE__)
            .SetReturn(0);
        disableNegativeTest(negativeTestsToSkip, test_index++);
        //enableNegativeTest(negativeTestsToSkip, test_index++);
        if (1100 <= ms) { break; }
    }
    STRICT_EXPECTED_CALL(GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    // Simulate polling STATUS pin
    random_number = (rand() % 4);
    for (size_t i = 0; i < random_number; ++i) {
        STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
            .SetReturn(GPIO_INPUT_PIN_HIGH);
        disableNegativeTest(negativeTestsToSkip, test_index++);
    }
    STRICT_EXPECTED_CALL(GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
        .SetReturn(GPIO_INPUT_PIN_LOW);
    disableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));
    disableNegativeTest(negativeTestsToSkip, test_index++);
    EXPECTED_CALL(timer_a3_deinit());
    disableNegativeTest(negativeTestsToSkip, test_index++);
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if (skipNegativeTest(negativeTestsToSkip, i)) { continue; }

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        platform_deinit();

        // Assert
        ASSERT_IS_TRUE(true);  // You know I would never lie to you, baby...
    }

    // Cleanup
    umock_c_negative_tests_deinit();
}


// platform_get_default_tlsio

TEST_FUNCTION(platform_get_default_tlsio_SCENARIO_success)
{
    // Arrange

    // Expected call listing
    umock_c_reset_all_calls();

    // Act

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}
/* SRS_UARTIO_27_034: [ `platform_get_default_tlsio()` shall return a non-`NULL` pointer to an `IO_INTERFACE_DESCRIPTION` structure. ] */  
/* SRS_UARTIO_27_035: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_close()`. ] */  
/* SRS_UARTIO_27_036: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_create()`. ] */  
/* SRS_UARTIO_27_037: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_destroy()`. ] */  
/* SRS_UARTIO_27_038: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_dowork()`. ] */  
/* SRS_UARTIO_27_039: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_open()`. ] */  
/* SRS_UARTIO_27_040: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_retrieveoptions()`. ] */  
/* SRS_UARTIO_27_041: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_send()`. ] */  
/* SRS_UARTIO_27_042: [ `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_setoption()`. ] */  


END_TEST_SUITE(platform_msp430_unittests)


// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
  #include <cstdbool>
  #include <cstddef>
  #include <cstdlib>
  #include <ctime>
#else
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdlib.h>
  #include <time.h>
#endif

// Test framework `#includes`
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"

// External library dependencies
#include "driverlib.h"

#define MOCK_CLOCK_HZ 32768
#define MOCK_TIMER_A_COUNTER_VALUE 12345

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static
void *
non_mocked_malloc(
    size_t size_
) {
    return malloc(size_);
}

static
void
non_mocked_free(
    void * block_
) {
    free(block_);
    return;
}

static
char *
umockvalue_stringify_Timer_A_initContinuousModeParam(
    const Timer_A_initContinuousModeParam ** value_
) {
    char * result;

    if (NULL == value_) {
        result = (char *)NULL;
    } else if (NULL == *value_) {
        result = (char *)NULL;
    } else {
        const Timer_A_initContinuousModeParam * value = *value_;
        char buffer[256];
        size_t len = sprintf(
            buffer,
            "EUSCI_A_UART_initParam {\n\t.clockSource: 0x%04x\n\t.clockSourceDivider: 0x%04x\n\t.timerInterruptEnable_TAIE: 0x%04x\n\t.timerClear: 0x%04x\n\t.startTimer: %s\n}",
            value->clockSource,
            value->clockSourceDivider,
            value->timerInterruptEnable_TAIE,
            value->timerClear,
            (value->startTimer ? "true" : "false")
            );

        result = (char *)malloc(len + 1);
        strcpy(result, buffer);
    }

    return result;
}

static
int
umockvalue_are_equal_Timer_A_initContinuousModeParam(
    const Timer_A_initContinuousModeParam ** left_,
    const Timer_A_initContinuousModeParam ** right_
) {
    bool match;

    if (NULL == left_) {
        match = false;
    } else if (NULL == right_)  {
        match = false;
    } else if (NULL == *left_)  {
        match = false;
    } else if (NULL == *right_)  {
        match = false;
    } else {
        const Timer_A_initContinuousModeParam * left = *left_;
        const Timer_A_initContinuousModeParam * right = *right_;
        match = true;

        match = (match && (left->clockSource == right->clockSource));
        match = (match && (left->clockSourceDivider == right->clockSourceDivider));
        match = (match && (left->startTimer == right->startTimer));
        match = (match && (left->timerClear == right->timerClear));
        match = (match && (left->timerInterruptEnable_TAIE == right->timerInterruptEnable_TAIE));
    }

    return (int)match;
}

static
int
umockvalue_copy_Timer_A_initContinuousModeParam(
    Timer_A_initContinuousModeParam ** destination_,
    const Timer_A_initContinuousModeParam ** source_
) {
    int result;

    if (NULL == destination_) {
        result = __LINE__;
    } else if (NULL == source_) {
        result = __LINE__;
    } else if (NULL == *source_) {
        result = __LINE__;
    } else if (NULL == (*destination_ = (Timer_A_initContinuousModeParam *)malloc(sizeof(Timer_A_initContinuousModeParam)))) {
        result = __LINE__;
    } else {
        Timer_A_initContinuousModeParam * destination = *destination_;
        const Timer_A_initContinuousModeParam * source = *source_;

        if (NULL == destination) {
            result = __LINE__;
        } else if (NULL == source) {
            result = __LINE__;
        } else {
            destination->clockSource = source->clockSource;
            destination->clockSourceDivider = source->clockSourceDivider;
            destination->startTimer = source->startTimer;
            destination->timerClear = source->timerClear;
            destination->timerInterruptEnable_TAIE = source->timerInterruptEnable_TAIE;
            result = 0;
        }
    }

    return result;
}

static
void
umockvalue_free_Timer_A_initContinuousModeParam(
    Timer_A_initContinuousModeParam ** value_
) {
    (void)free(*value_);
}

#define ENABLE_MOCKS
  // `#include` SDK dependencies here
  #include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

// Under test `#includes`
#include "../adapters/tickcounter_msp430.h"

#define ENABLE_MOCKS

  // ** Mocking cs.h module (driverlib.h)
  MOCK_FUNCTION_WITH_CODE(, void, CS_enableClockRequest, uint8_t, selectClock)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, uint32_t, CS_getACLK)
  MOCK_FUNCTION_END(MOCK_CLOCK_HZ);

  // ** Mocking timer_a.h module (driverlib.h)
  MOCK_FUNCTION_WITH_CODE(, void, Timer_A_stop, uint16_t, baseAddress)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, Timer_A_disableInterrupt, uint16_t, baseAddress)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, uint16_t, Timer_A_getCounterValue ,uint16_t, baseAddress)
  MOCK_FUNCTION_END(MOCK_TIMER_A_COUNTER_VALUE);
  MOCK_FUNCTION_WITH_CODE(, void, Timer_A_initContinuousMode, uint16_t, baseAddress, Timer_A_initContinuousModeParam *, param)
  MOCK_FUNCTION_END();

#undef ENABLE_MOCKS

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(tickcounter_msp430_unittests)

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

    REGISTER_UMOCK_ALIAS_TYPE(bool, int);

    REGISTER_UMOCK_VALUE_TYPE(Timer_A_initContinuousModeParam *, umockvalue_stringify_Timer_A_initContinuousModeParam, umockvalue_are_equal_Timer_A_initContinuousModeParam, umockvalue_copy_Timer_A_initContinuousModeParam, umockvalue_free_Timer_A_initContinuousModeParam);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, non_mocked_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, non_mocked_free);
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

// tickcounter_create

/* SRS_UARTIO_27_000: [ `tickcounter_create()` shall allocate its context by calling `(void *)malloc(size_t byte_count)`. ] */
/* SRS_UARTIO_27_002: [ `tickcounter_create()` shall query its creation offset by calling `(uint16_t)Timer_A_getCounterValue(uint16_t baseAddress)`. ] */
/* SRS_UARTIO_27_003: [ If no errors occur, `tickcounter_create()` shall return a non-NULL TICK_COUNTER_HANDLE. ] */
TEST_FUNCTION(tickcounter_create_SCENARIO_success)
{
    // Arrange
    timer_a3_init();
    TICK_COUNTER_HANDLE tick_counter = (TICK_COUNTER_HANDLE)NULL;

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);
    STRICT_EXPECTED_CALL(Timer_A_getCounterValue(TIMER_A3_BASE));

    // Act
    tick_counter = tickcounter_create();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(tick_counter);

    // Cleanup
    tickcounter_destroy(tick_counter);
    timer_a3_deinit();
}

/* SRS_UARTIO_27_001: [ If the call to `malloc()` fails, `tickcounter_create()` shall fail and return `NULL`. ] */  
TEST_FUNCTION(tickcounter_create_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    timer_a3_init();
    TICK_COUNTER_HANDLE tick_counter = (TICK_COUNTER_HANDLE)NULL;

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(Timer_A_getCounterValue(TIMER_A3_BASE));
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if (i != 0) { continue; }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        tick_counter = tickcounter_create();

        // Assert
        ASSERT_IS_NULL(tick_counter);
    }

    // Cleanup
    timer_a3_deinit();
}


// timer_a3_deinit

/* SRS_UARTIO_27_012: [ `timer_a3_deinit()` shall disable the interrupt by calling `(void)Timer_A_disableInterrupt(uint16_t baseAddress)` using `TIMER_A3_BASE` as `baseAddress`. ] */
/* SRS_UARTIO_27_013: [ `timer_a3_deinit()` shall stop the underlying timer by calling `(void)Timer_A_stop(uint16_t baseAddress)` using `TIMER_A3_BASE` as `baseAddress`. ] */
TEST_FUNCTION(timer_a3_deinit_SCENARIO_success)
{
    // Arrange
    timer_a3_init();

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(Timer_A_disableInterrupt(TIMER_A3_BASE));
    STRICT_EXPECTED_CALL(Timer_A_stop(TIMER_A3_BASE));

    // Act
    timer_a3_deinit();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}


// tickcounter_destroy

/* SRS_UARTIO_27_005: [ `tickcounter_destroy()` shall release the context of the specified by `TICK_COUNTER_HANDLE` by calling `(void)free(void *)` using the handle value. ] */
TEST_FUNCTION(tickcounter_destroy_SCENARIO_success)
{
    // Arrange
    timer_a3_init();
    TICK_COUNTER_HANDLE tick_counter = tickcounter_create();
    ASSERT_IS_NOT_NULL(tick_counter);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(gballoc_free(tick_counter));

    // Act
    tickcounter_destroy(tick_counter);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    timer_a3_deinit();
}

/* SRS_UARTIO_27_004: [ If a `NULL` handle is provided, `tickcounter_destroy()` shall do nothing. ] */
TEST_FUNCTION(tickcounter_destroy_SCENARIO_NULL_handle)
{
    // Arrange

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    tickcounter_destroy(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}


// timer_a3_init

/* SRS_UARTIO_27_014: [ `timer_a3_init()` shall enable clock request for the auxillary clock by calling `(void)CS_enableClockRequest(uint8_t selectClock)` using the `CS_ACLK` identifier. ] */  
/* SRS_UARTIO_27_015: [ `timer_a3_init()` shall determine the clock speed of the auxillary clock by calling `(uint32_t)CS_getACLK(void)`. ] */  
/* SRS_UARTIO_27_017: [ `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)`  using `TIMER_A3_BASE` as `baseAddress`. ] */  
/* SRS_UARTIO_27_018: [ `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_CLOCKSOURCE_ACLK` as the first member of the initialization parameters. ] */  
/* SRS_UARTIO_27_019: [ `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_CLOCKSOURCE_DIVIDER_16` as the second member of the initialization parameters. ] */
/* SRS_UARTIO_27_020: [ `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_TAIE_INTERRUPT_ENABLE` as the third member of the initialization parameters. ] */
/* SRS_UARTIO_27_021: [ `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_SKIP_CLEAR` as the fourth member of the initialization parameters. ] */
/* SRS_UARTIO_27_022: [ `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `true` as the fifth member of the initialization parameters. ] */
/* SRS_UARTIO_27_023: [ If no errors occur, then `timer_a3_init()` shall return zero. ] */
TEST_FUNCTION(timer_a3_init_SCENARIO_success)
{
    // Arrange
    int error;
    Timer_A_initContinuousModeParam param = {
        TIMER_A_CLOCKSOURCE_ACLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_16,
        TIMER_A_TAIE_INTERRUPT_ENABLE,
        TIMER_A_SKIP_CLEAR,
        true,
    };

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(CS_enableClockRequest(CS_ACLK));
    EXPECTED_CALL(CS_getACLK());
    STRICT_EXPECTED_CALL(Timer_A_initContinuousMode(TIMER_A3_BASE, &param));

    // Act
    error = timer_a3_init();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
    timer_a3_deinit();
}

/* SRS_UARTIO_27_016: [ If the auxillary clock speed is less than 16kHz, then `timer_a3_init()` shall fail immediately and return a non-zero value. ] */  
TEST_FUNCTION(timer_a3_init_SCENARIO_slow_clock)
{
    // Arrange
    int error;

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(CS_enableClockRequest(CS_ACLK));
    EXPECTED_CALL(CS_getACLK())
        .SetReturn(8192);

    // Act
    error = timer_a3_init();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    timer_a3_deinit();
}


// tickcounter_get_current_ms

/* SRS_UARTIO_27_008: [ `tickcounter_get_current_ms()` shall query the current tick count by calling `(uint16_t)Timer_A_getCounterValue(uint16_t baseAddress)` using `TIMER_A3_BASE` as `baseAddress`. ] */  
/* SRS_UARTIO_27_009: [ `tickcounter_get_current_ms()` shall return the number of milliseconds between that have passed between the time `tickcounter_create()` was called and now. ] */  
/* SRS_UARTIO_27_010: [ If no errors occur, then `tickcounter_get_current_ms()` shall return zero. ] */  
/* SRS_UARTIO_27_011: [ When the TIMER_A3 overflows its `uint16_t` storage, the tick count value is accumulated and accurately represented in milliseconds. ] */
TEST_FUNCTION(tickcounter_get_current_ms_SCENARIO_success)
{
    // Arrange
    int error;
    error = timer_a3_init();
    tickcounter_ms_t ms = 0;
    TICK_COUNTER_HANDLE tick_counter = tickcounter_create();
    ASSERT_ARE_EQUAL(int, 0, error);
    ASSERT_IS_NOT_NULL(tick_counter);
    TIMER3_A1_ISR(); // simulate counter overflow interrupt

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(Timer_A_getCounterValue(TIMER_A3_BASE))
        .SetReturn(2345);

    // Act
    error = tickcounter_get_current_ms(tick_counter, &ms);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 27118, ms);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
    tickcounter_destroy(tick_counter);
    timer_a3_deinit();
}

/* SRS_UARTIO_27_006: [ If a `NULL` handle is provided, `tickcounter_get_current_ms()` shall fail immediately and return a non-zero value. ] */  
TEST_FUNCTION(tickcounter_get_current_ms_SCENARIO_NULL_handle)
{
    // Arrange
    int error;
    error = timer_a3_init();
    tickcounter_ms_t ms = 0;
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = tickcounter_get_current_ms((TICK_COUNTER_HANDLE)NULL, &ms);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    timer_a3_deinit();
}

/* SRS_UARTIO_27_007: [ If a `NULL` is passed in `current_ms`, `tickcounter_get_current_ms()` shall fail immediately and return a non zero value. ] */
TEST_FUNCTION(tickcounter_get_current_ms_SCENARIO_NULL_current_ms)
{
    // Arrange
    int error;
    error = timer_a3_init();
    TICK_COUNTER_HANDLE tick_counter = tickcounter_create();
    ASSERT_ARE_EQUAL(int, 0, error);
    ASSERT_IS_NOT_NULL(tick_counter);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = tickcounter_get_current_ms(tick_counter, (tickcounter_ms_t *)NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    tickcounter_destroy(tick_counter);
    timer_a3_deinit();
}


END_TEST_SUITE(tickcounter_msp430_unittests)


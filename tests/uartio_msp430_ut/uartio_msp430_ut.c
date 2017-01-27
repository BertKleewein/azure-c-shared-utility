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

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Test framework `#includes`
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"

// External library dependencies
#include "driverlib.h"

#define MOCK_CLOCK_HZ 1000000
#define MOCK_NON_NULL_XIO_HANDLE (CONCRETE_IO_HANDLE)0x19790917
#define MOCK_OPTION_HANDLER (OPTIONHANDLER_HANDLE)0x19790917

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static char * uartio_rx_data;

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
umockvalue_stringify_EUSCI_A_UART_initParam(
    const EUSCI_A_UART_initParam ** value_
) {
    char * result;

    if (NULL == value_) {
        result = (char *)NULL;
    } else if (NULL == *value_) {
        result = (char *)NULL;
    } else {
        const EUSCI_A_UART_initParam * value = *value_;
        char buffer[256];
        size_t len = sprintf(
            buffer,
            "EUSCI_A_UART_initParam {\n\t.selectClockSource: 0x%02x\n\t.clockPrescalar: %u\n\t.firstModReg: %u\n\t.secondModReg: %u\n\t.parity: 0x%02x\n\t.msborLsbFirst: 0x%04x\n\t.numberofStopBits: 0x%04x\n\t.uartMode: 0x%04x\n\t.overSampling: 0x%02x\n}\n",
            value->selectClockSource,
            value->clockPrescalar,
            value->firstModReg,
            value->secondModReg,
            value->parity,
            value->msborLsbFirst,
            value->numberofStopBits,
            value->uartMode,
            value->overSampling
        );

        result = (char *)malloc(len + 1);
        strcpy(result, buffer);
    }

    return result;
}

static
int
umockvalue_are_equal_EUSCI_A_UART_initParam(
    const EUSCI_A_UART_initParam ** left_,
    const EUSCI_A_UART_initParam ** right_
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
        const EUSCI_A_UART_initParam * left = *left_;
        const EUSCI_A_UART_initParam * right = *right_;
        match = true;

        match = (match && (left->selectClockSource == right->selectClockSource));
        match = (match && (left->clockPrescalar == right->clockPrescalar));
        match = (match && (left->firstModReg == right->firstModReg));
        match = (match && (left->secondModReg == right->secondModReg));
        match = (match && (left->parity == right->parity));
        match = (match && (left->msborLsbFirst == right->msborLsbFirst));
        match = (match && (left->numberofStopBits == right->numberofStopBits));
        match = (match && (left->uartMode == right->uartMode));
        match = (match && (left->overSampling == right->overSampling));
    }

    return (int)match;
}

static
int
umockvalue_copy_EUSCI_A_UART_initParam(
    EUSCI_A_UART_initParam ** destination_,
    const EUSCI_A_UART_initParam ** source_
) {
    int result;

    if (NULL == destination_) {
        result = __LINE__;
    } else if (NULL == source_) {
        result = __LINE__;
    } else if (NULL == *source_) {
        result = __LINE__;
    } else if (NULL == (*destination_ = (EUSCI_A_UART_initParam *)malloc(sizeof(EUSCI_A_UART_initParam)))) {
        result = __LINE__;
    } else {
        EUSCI_A_UART_initParam * destination = *destination_;
        const EUSCI_A_UART_initParam * source = *source_;

        if (NULL == destination) {
            result = __LINE__;
        } else if (NULL == source) {
            result = __LINE__;
        } else {
            destination->selectClockSource = source->selectClockSource;
            destination->clockPrescalar = source->clockPrescalar;
            destination->firstModReg = source->firstModReg;
            destination->secondModReg = source->secondModReg;
            destination->parity = source->parity;
            destination->msborLsbFirst = source->msborLsbFirst;
            destination->numberofStopBits = source->numberofStopBits;
            destination->uartMode = source->uartMode;
            destination->overSampling = source->overSampling;
            result = 0;
        }
    }

    return result;
}

static
void
umockvalue_free_EUSCI_A_UART_initParam(
    EUSCI_A_UART_initParam ** value_
) {
    (void)free(*value_);
}

#define ENABLE_MOCKS
  // `#include` SDK dependencies here
  #include "azure_c_shared_utility/gballoc.h"
  #include "azure_c_shared_utility/optionhandler.h"
#undef ENABLE_MOCKS

// Under test `#includes`
#include "azure_c_shared_utility/uartio.h"

pfCloneOption options_cloneoption;
pfDestroyOption options_destroyoption;

static
OPTIONHANDLER_HANDLE
mock_OptionHandler_Create(
    pfCloneOption clone_option,
    pfDestroyOption destroy_option,
    pfSetOption set_option
) {
    (void)set_option;

    options_cloneoption = clone_option;
    options_destroyoption = destroy_option;
    return MOCK_OPTION_HANDLER;
}

#define ENABLE_MOCKS
  TEST_DEFINE_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
  IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT_VALUES);
  TEST_DEFINE_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);
  IMPLEMENT_UMOCK_C_ENUM_TYPE(IO_SEND_RESULT, IO_SEND_RESULT_VALUES);

  // ** Mocking cs.h module (driverlib.h)
  MOCK_FUNCTION_WITH_CODE(, void, CS_enableClockRequest, uint8_t, selectClock)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, uint32_t, CS_getSMCLK)
  MOCK_FUNCTION_END(MOCK_CLOCK_HZ);

  // ** Mocking eusci_a_uart.h module (driverlib.h)
  MOCK_FUNCTION_WITH_CODE(, void, EUSCI_A_UART_disable, uint16_t, baseAddress)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, EUSCI_A_UART_disableInterrupt, uint16_t, baseAddress, uint8_t, mask)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, EUSCI_A_UART_enable, uint16_t, baseAddress)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, EUSCI_A_UART_enableInterrupt, uint16_t, baseAddress, uint8_t, mask)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, bool, EUSCI_A_UART_init, uint16_t, baseAddress, EUSCI_A_UART_initParam *, param)
  MOCK_FUNCTION_END(true);
  MOCK_FUNCTION_WITH_CODE(, uint8_t, EUSCI_A_UART_queryStatusFlags, uint16_t, baseAddress, uint8_t, mask)
  MOCK_FUNCTION_END(0x00);
  MOCK_FUNCTION_WITH_CODE(, uint8_t, EUSCI_A_UART_receiveData, uint16_t, baseAddress)
  MOCK_FUNCTION_END(0x00);
  MOCK_FUNCTION_WITH_CODE(, void, EUSCI_A_UART_transmitData, uint16_t, baseAddress, uint8_t, transmitData)
  MOCK_FUNCTION_END();

  // ** Mocking out the user callbacks
  MOCK_FUNCTION_WITH_CODE(, void, mock_on_io_open_complete, void *, context, IO_OPEN_RESULT, open_result)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, mock_on_bytes_received, void *, context, const unsigned char *, buffer, size_t, size)
      uartio_rx_data = (char *)buffer;
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, mock_on_io_error, void *, context)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, mock_on_io_close_complete, void *, context)
  MOCK_FUNCTION_END();
  MOCK_FUNCTION_WITH_CODE(, void, mock_on_send_complete, void *, context, IO_SEND_RESULT, send_result)
  MOCK_FUNCTION_END();

#undef ENABLE_MOCKS

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(uartio_msp430_unittests)

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

    REGISTER_TYPE(IO_OPEN_RESULT, IO_OPEN_RESULT);
    REGISTER_TYPE(IO_SEND_RESULT, IO_SEND_RESULT);

    REGISTER_UMOCK_ALIAS_TYPE(bool, int);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfCloneOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfDestroyOption, void*);
    REGISTER_UMOCK_ALIAS_TYPE(pfSetOption, void*);

    REGISTER_UMOCK_VALUE_TYPE(EUSCI_A_UART_initParam *, umockvalue_stringify_EUSCI_A_UART_initParam, umockvalue_are_equal_EUSCI_A_UART_initParam, umockvalue_copy_EUSCI_A_UART_initParam, umockvalue_free_EUSCI_A_UART_initParam);

    REGISTER_GLOBAL_MOCK_HOOK(OptionHandler_Create, mock_OptionHandler_Create);
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

/* uartio_get_interface_description */

/* SRS_UARTIO_27_000: [ `uartio_get_interface_description()` shall return a non-`NULL` pointer to an `IO_INTERFACE_DESCRIPTION` structure. ] */
/* SRS_UARTIO_27_001: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_close()`. ] */
/* SRS_UARTIO_27_002: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_create()`. ] */
/* SRS_UARTIO_27_003: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_destroy()`. ] */
/* SRS_UARTIO_27_004: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_dowork()`. ] */
/* SRS_UARTIO_27_005: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_open()`. ] */
/* SRS_UARTIO_27_006: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_retrieveoptions()`. ] */
/* SRS_UARTIO_27_007: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_send()`. ] */
/* SRS_UARTIO_27_008: [ `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_setoption()`. ] */
TEST_FUNCTION(uartio_get_interface_description_SCENARIO_success)
{
    // Arrange

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(io_interface);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_close);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_create);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_destroy);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_dowork);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_open);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_retrieveoptions);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_send);
    ASSERT_IS_NOT_NULL(io_interface->concrete_io_setoption);

    // Cleanup
}


/* uartio_cloneoption */

/* SRS_UARTIO_27_009: [ If the `option_name` argument is NULL, `uartio_cloneoption()` shall return `NULL`. ] */
TEST_FUNCTION(uartio_cloneoption_SCENARIO_NULL_option_name)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    void * result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options = io_interface->concrete_io_retrieveoptions(xio);
    ASSERT_IS_NOT_NULL(options);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = options_cloneoption(NULL, "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_010: [ If the `option_value` argument is NULL, `uartio_cloneoption()` shall return `NULL`. ] */
TEST_FUNCTION(uartio_cloneoption_SCENARIO_NULL_option_value)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    void * result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options = io_interface->concrete_io_retrieveoptions(xio);
    ASSERT_IS_NOT_NULL(options);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = options_cloneoption("Options", NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_011: [ If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_cloneoption()` shall return `NULL`. ] */
TEST_FUNCTION(uartio_cloneoption_SCENARIO_invalid_option_name)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    void * result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options = io_interface->concrete_io_retrieveoptions(xio);
    ASSERT_IS_NOT_NULL(options);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = options_cloneoption("Options", "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}


/* uartio_close */

/* SRS_UARTIO_27_012: [ If the argument `io_handle` is NULL, `uartio_close()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_close_SCENARIO_NULL_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_close(NULL, mock_on_io_close_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_100: [ If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_close()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_close_SCENARIO_invalid_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_close(MOCK_NON_NULL_XIO_HANDLE, mock_on_io_close_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_013: [ `uartio_close()` shall initiate closing the UART IO and on success it shall return 0. ] */
/* SRS_UARTIO_27_014: [ On success, `on_io_close_complete()` shall be called while passing as argument `on_io_close_complete_context`. ] */
/* SRS_UARTIO_27_101: [ `uartio_close()` shall await any outstanding bytes by polling `(uint8_t)EUSCI_A_UART_queryStatusFlags(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask , `EUSCI_A_UART_BUSY`, until it returns an empty mask. ] */
/* SRS_UARTIO_27_102: [ `uartio_close()` shall disable the UART interrupt (while caching the ring buffer) by calling `(void)EUSCI_A_UART_disableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. ] */
/* SRS_UARTIO_27_103: [ `uartio_close()` shall disable the UART by calling `(void)EUSCI_A_UART_disable(uint16_t baseAddress)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`. ] */
TEST_FUNCTION(uartio_close_SCENARIO_success)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int random_number, result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    (void)srand((unsigned int)time(NULL));
    random_number = (rand() % 4);

    // Expected call listing
    umock_c_reset_all_calls();
    // Simulate busy UART
    for (int i = 0; i <= random_number; ++i) {
        STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, EUSCI_A_UART_BUSY))
            .SetReturn(EUSCI_A_UART_BUSY);
    }
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, EUSCI_A_UART_BUSY));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disable(EUSCI_A1_BASE));
    STRICT_EXPECTED_CALL(mock_on_io_close_complete(&result));

    // Act
    result = io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_015: [ If the argument `on_io_close_complete` is NULL, `uartio_close()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_close_SCENARIO_NULL_on_io_close_complete)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_close(xio, NULL, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_016: [ If called when `uartio` is closed, `uartio_close()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_close_SCENARIO_close_called_twice)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}


/* uartio_create */

/* SRS_UARTIO_27_104: [ If no errors are encountered, `uartio_create()` shall return a pointer to the single static instance. ] */
TEST_FUNCTION(uartio_create_SCENARIO_success)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(gballoc_malloc(sizeof(uint8_t) * config.ring_buffer_size));
    STRICT_EXPECTED_CALL(gballoc_malloc(sizeof(uint8_t) * config.ring_buffer_size));

    // Act
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(xio);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_017: [ If the argument `io_create_parameters` is `NULL`, `uartio_create()` shall fail and return `NULL`. ] */
TEST_FUNCTION(uartio_create_SCENARIO_NULL_io_create_parameters)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(xio);

    // Cleanup
}

/* SRS_UARTIO_27_105: [ If the `baud_rate` member of the `UARTIO_CONFIG *` parameter is 0, then `uartio_create()` shall fail and return `NULL`. ] */
TEST_FUNCTION(uartio_create_SCENARIO_bad_parameters_baud_rate_equals_zero)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 0, 4 };

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(xio);

    // Cleanup
}

/* SRS_UARTIO_27_106: [ If the `ring_buffer_size` member of the `UARTIO_CONFIG *` parameter is 0, then `uartio_create()` shall fail and return `NULL`. ] */
TEST_FUNCTION(uartio_create_SCENARIO_bad_parameters_ring_buffer_size_equals_zero)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 0 };

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(xio);

    // Cleanup
}

/* SRS_UARTIO_27_107: [ If the `ring_buffer_size` member of the `UARTIO_CONFIG *` parameter is not a power of 2, then `uartio_create()` shall fail and return `NULL`. ] */
TEST_FUNCTION(uartio_create_SCENARIO_bad_parameters_ring_buffer_size_non_power_of_two)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 6 };

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Assert
    ASSERT_ARE_NOT_EQUAL(size_t, 0, (config.ring_buffer_size & (config.ring_buffer_size - 1)));
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(xio);

    // Cleanup
}

/* SRS_UARTIO_27_108: [ If `uartio_create()` has previously been called without subsequently calling `uartio_destroy()`, then `uartio_create()` shall fail immediately and return `NULL`. ] */
TEST_FUNCTION(uartio_create_SCENARIO_create_called_twice)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    CONCRETE_IO_HANDLE xio2 = io_interface->concrete_io_create(&config);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(xio2);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_109: [ If allocating memory for the ring buffer fails, then the `uartio_create()` shall fail and return `NULL`. ] */
/* SRS_UARTIO_27_110: [ If allocating memory for the cache buffer fails, then the `uartio_create()` shall fail and return `NULL`. ] */
/* SRS_UARTIO_27_018: [ VALGRIND - When `uartio_create()` fails, all allocated resources up to that point shall be freed. ] */
TEST_FUNCTION(uartio_create_SCENARIO_negative_tests)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    UARTIO_CONFIG config = { 9600, 4 };

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(gballoc_malloc(sizeof(uint8_t) * config.ring_buffer_size))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(gballoc_malloc(sizeof(uint8_t) * config.ring_buffer_size))
        .SetFailReturn(NULL);
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

        // Assert
        ASSERT_IS_NULL(xio);
    }

    // Cleanup
    umock_c_negative_tests_deinit();
}


/* uartio_destroy */

/* SRS_UARTIO_27_019: [ If the argument `io_handle` is `NULL`, `uartio_destroy()` shall do nothing. ] */
TEST_FUNCTION(uartio_destroy_SCENARIO_NULL_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    (void)io_interface->concrete_io_destroy(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* SRS_UARTIO_27_111: [ If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_destroy()` shall do nothing. ] */
TEST_FUNCTION(uartio_destroy_SCENARIO_invalid_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    (void)io_interface->concrete_io_destroy(MOCK_NON_NULL_XIO_HANDLE);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* SRS_UARTIO_27_112: [ `uartio_destroy()` shall free the ring buffer. ] */
/* SRS_UARTIO_27_113: [ `uartio_destroy()` shall free the cache buffer. ] */
/* SRS_UARTIO_27_020: [ `uartio_destroy()` shall close the IO if it was open before freeing all the resources. ] */
/* SRS_UARTIO_27_021: [ NO OPTIONS - All options cached via `uartio_set_option` shall also be freed. ] */
TEST_FUNCTION(uartio_destroy_SCENARIO_success)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, EUSCI_A_UART_BUSY));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disable(EUSCI_A1_BASE));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));

    // Act
    (void)io_interface->concrete_io_destroy(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* SRS_UARTIO_27_114: [ If `uartio_destroy()` has previously been called on a handle, then `uartio_destroy()` shall do nothing. ] */
TEST_FUNCTION(uartio_destroy_SCENARIO_destory_called_twice_on_same_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    (void)io_interface->concrete_io_destroy(xio);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    (void)io_interface->concrete_io_destroy(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}


/* uartio_destroyoption */

/* SRS_UARTIO_27_022: [ If the `option_name` argument is `NULL`, `uartio_destroyoption()` shall do nothing. ] */
TEST_FUNCTION(uartio_destroyoption_SCENARIO_NULL_option_name)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options = io_interface->concrete_io_retrieveoptions(xio);
    ASSERT_IS_NOT_NULL(options);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    options_destroyoption(NULL, "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_023: [ If the `option_value` argument is `NULL`, `uartio_destroyoption()` shall do nothing. ] */
TEST_FUNCTION(uartio_destroyoption_SCENARIO_NULL_option_value)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options = io_interface->concrete_io_retrieveoptions(xio);
    ASSERT_IS_NOT_NULL(options);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    options_destroyoption("Options", NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_024: [ If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_destroyoption()` shall do nothing. ] */
TEST_FUNCTION(uartio_destroyoption_SCENARIO_invalid_option_name)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options = io_interface->concrete_io_retrieveoptions(xio);
    ASSERT_IS_NOT_NULL(options);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    options_destroyoption("Options", "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}


/* uartio_dowork */

/* SRS_UARTIO_27_025: [ If the `io_handle` argument is `NULL`, `uartio_dowork()` shall do nothing. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_NULL_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    (void)io_interface->concrete_io_dowork(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* SRS_UARTIO_27_115: [ If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_dowork()` shall do nothing. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_invalid_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    (void)io_interface->concrete_io_dowork(MOCK_NON_NULL_XIO_HANDLE);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* SRS_UARTIO_27_026: [ If the IO is closed (and open has not been initiated), then `uartio_dowork` shall do nothing. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_xio_closed)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    (void)io_interface->concrete_io_dowork(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_116: [ `uartio_dowork()` shall disable the UART interrupt while caching the ring buffer by calling `(void)EUSCI_A_UART_disableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. ] */
/* SRS_UARTIO_27_117: [ `uartio_dowork()` shall enable the UART interrupt after caching the ring buffer by calling `(void)EUSCI_A_UART_enableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. ] */
/* SRS_UARTIO_27_118: [ If the IO is open, `uartio_dowork()` shall indicate all bytes presented by the UART interrupt via the `on_bytes_received()` callback passed to `uartio_open()`. ] */
/* SRS_UARTIO_27_119: [ `uartio_dowork()` shall discard any bytes presented byte by the UART interrupt when the IO is closed. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_success)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    EUSCI_A_UART_initParam eusci_a_parameters = { EUSCI_A_UART_CLOCKSOURCE_SMCLK, 6, 8, 0x20, EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT, EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();

    // Ignored UART activity
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\r');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\n');
    USCI_A1_ISR();

    STRICT_EXPECTED_CALL(CS_enableClockRequest(CS_SMCLK));
    EXPECTED_CALL(CS_getSMCLK());
    STRICT_EXPECTED_CALL(EUSCI_A_UART_init(EUSCI_A1_BASE, &eusci_a_parameters));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enable(EUSCI_A1_BASE));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_OK));
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, &result, mock_on_io_error, &result);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Mock asynchronous UART activity
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('0');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\r');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\n');
    USCI_A1_ISR();

    // Called during `uartio_dowork()`
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_bytes_received(&result, IGNORED_PTR_ARG, 3))
        .IgnoreArgument(2);

    // Act
    (void)io_interface->concrete_io_dowork(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, strncmp("0\r\n", uartio_rx_data, 3));

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_120: [ If a UART framing error is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_UART_framing_error)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, &result, mock_on_io_error, &result);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();

    // Mock asynchronous UART activity
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('0');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)))
        .SetReturn(EUSCI_A_UART_FRAMING_ERROR);
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\r');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\n');
    USCI_A1_ISR();

    // Called during `uartio_dowork()`
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_bytes_received(&result, IGNORED_PTR_ARG, 3))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(mock_on_io_error(&result));

    // Act
    (void)io_interface->concrete_io_dowork(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_121: [ If a UART overrun error is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_UART_overrun_error)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, &result, mock_on_io_error, &result);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();

    // Mock asynchronous UART activity
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('0');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)))
        .SetReturn(EUSCI_A_UART_OVERRUN_ERROR);
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\r');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\n');
    USCI_A1_ISR();

    // Called during `uartio_dowork()`
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_bytes_received(&result, IGNORED_PTR_ARG, 3))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(mock_on_io_error(&result));

    // Act
    (void)io_interface->concrete_io_dowork(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_122: [ If a UART parity error is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_UART_parity_error)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, &result, mock_on_io_error, &result);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();

    // Mock asynchronous UART activity
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('0');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)))
        .SetReturn(EUSCI_A_UART_PARITY_ERROR);
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\r');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\n');
    USCI_A1_ISR();

    // Called during `uartio_dowork()`
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_bytes_received(&result, IGNORED_PTR_ARG, 3))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(mock_on_io_error(&result));

    // Act
    (void)io_interface->concrete_io_dowork(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_123: [ If a ring buffer overflow is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. ] */
TEST_FUNCTION(uartio_dowork_SCENARIO_UART_ring_buffer_overflow)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, &result, mock_on_io_error, &result);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();

    // Mock asynchronous UART activity
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\r');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\n');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('0');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\r');
    USCI_A1_ISR();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR)));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_receiveData(EUSCI_A1_BASE))
        .SetReturn('\n');
    USCI_A1_ISR();

    // Called during `uartio_dowork()`
    STRICT_EXPECTED_CALL(EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_bytes_received(&result, IGNORED_PTR_ARG, 4))
        .IgnoreArgument(2);
    STRICT_EXPECTED_CALL(mock_on_io_error(&result));

    // Act
    (void)io_interface->concrete_io_dowork(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}


/* uartio_open */

/* SRS_UARTIO_27_027: [ If the argument `io_handle` is `NULL` then `uartio_open()` shall return a non-zero value. ] */
/* SRS_UARTIO_27_028: [ If `uartio_open()` fails, the callback `on_io_open_complete()` shall be called, while passing `on_io_open_complete_context` and `IO_OPEN_ERROR` as arguments. ] */
TEST_FUNCTION(uartio_open_SCENARIO_NULL_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_ERROR));

    // Act
    result = io_interface->concrete_io_open(NULL, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_124: [ If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_open()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_open_SCENARIO_invalid_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_ERROR));

    // Act
    result = io_interface->concrete_io_open(MOCK_NON_NULL_XIO_HANDLE, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_029: [ If no errors are encountered, `uartio_open()` shall return 0. ] */
/* SRS_UARTIO_27_030: [ If `uartio_open()` succeeds, the callback `on_io_open_complete()` shall be called, while passing `on_io_open_complete_context` and `IO_OPEN_OK` as arguments. ] */
/* SRS_UARTIO_27_125: [ `uartio_open()` shall enable clock request for the submodule clock by calling `(void)CS_enableClockRequest(uint8_t selectClock)` using the `CS_SMCLK` identifier. ] */  
/* SRS_UARTIO_27_126: [ `uartio_open()` shall determine the clock speed of the submodule clock by calling `(uint32_t)CS_getSMCLK(void)`. ] */
/* SRS_UARTIO_27_127: [ `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_CLOCKSOURCE_SMCLK as the first member of the initialization parameters. ] */
/* SRS_UARTIO_27_128: [ `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_NO_PARITY as the fifth member of the initialization parameters. ] */
/* SRS_UARTIO_27_129: [ `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_LSB_FIRST as the sixth member of the initialization parameters. ] */
/* SRS_UARTIO_27_130: [ `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_ONE_STOP_BIT as the seventh member of the initialization parameters. ] */
/* SRS_UARTIO_27_131: [ `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_MODE as the eighth member of the initialization parameters. ] */
/* SRS_UARTIO_27_132: [ `uartio_open()` shall enable the UART by calling `(void)EUSCI_A_UART_enable(uint16_t baseAddress)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`. ] */
/* SRS_UARTIO_27_133: [ `uartio_open()` shall enable the UART interrupt by calling `(void)EUSCI_A_UART_enableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. ] */
TEST_FUNCTION(uartio_open_SCENARIO_success)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    // Parameter values based on 1000000Hz @ 9600baud
    // To understand parameter values, refer to the MSP430 User's Guide - 24.3.10 - Setting a Baud Rate
    EUSCI_A_UART_initParam eusci_a_parameters = { EUSCI_A_UART_CLOCKSOURCE_SMCLK, 6, 8, 0x20, EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT, EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(CS_enableClockRequest(CS_SMCLK));
    EXPECTED_CALL(CS_getSMCLK());
    STRICT_EXPECTED_CALL(EUSCI_A_UART_init(EUSCI_A1_BASE, &eusci_a_parameters));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enable(EUSCI_A1_BASE));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_OK));

    // Act
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_031: [ If the argument `on_io_open_complete` is `NULL` then `uartio_open()` shall return a non-zero value. ] */
TEST_FUNCTION(uartio_open_SCENARIO_NULL_on_io_open_complete)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_open(xio, NULL, NULL, mock_on_bytes_received, NULL, mock_on_io_error, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_032: [ If the argument `on_bytes_received` is `NULL` then `uartio_open()` shall return a non-zero value. ] */
TEST_FUNCTION(uartio_open_SCENARIO_NULL_on_bytes_received)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_ERROR));

    // Act
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, NULL, NULL, mock_on_io_error, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_033: [ If the argument `on_io_error` is `NULL` then `uartio_open()` shall return a non-zero value. ] */
TEST_FUNCTION(uartio_open_SCENARIO_NULL_on_io_error)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_ERROR));

    // Act
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_034: [ If `uartio_open()` is called while the IO is open, `uartio_open()` shall return a non-zero value without performing any work to open the IO. ] */
TEST_FUNCTION(uartio_open_SCENARIO_call_open_twice)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_ERROR));

    // Act
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_134: [ `uartio_open()` shall fail if `false` is returned from calling `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the calculated parameters. ] */
TEST_FUNCTION(uartio_open_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    EUSCI_A_UART_initParam eusci_a_parameters = { EUSCI_A_UART_CLOCKSOURCE_SMCLK, 6, 8, 0x20, EUSCI_A_UART_NO_PARITY, EUSCI_A_UART_LSB_FIRST, EUSCI_A_UART_ONE_STOP_BIT, EUSCI_A_UART_MODE, EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(CS_enableClockRequest(CS_SMCLK));
    EXPECTED_CALL(CS_getSMCLK());
    STRICT_EXPECTED_CALL(EUSCI_A_UART_init(EUSCI_A1_BASE, &eusci_a_parameters))
        .SetFailReturn(false);
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enable(EUSCI_A1_BASE));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT));
    STRICT_EXPECTED_CALL(mock_on_io_open_complete(&result, IO_OPEN_ERROR));
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if (i != 2) { continue; }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);

        // Assert
        ASSERT_ARE_NOT_EQUAL(int, 0, result);
    }

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
    umock_c_negative_tests_deinit();
}


/* uartio_retrieveoptions */

/* SRS_UARTIO_27_035: [ If parameter `io_handle` is `NULL` then `uartio_retrieveoptions()` shall fail and return `NULL`. ] */
TEST_FUNCTION(uartio_retrieveoptions_SCENARIO_NULL_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    OPTIONHANDLER_HANDLE options;

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    options = io_interface->concrete_io_retrieveoptions(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(options);

    // Cleanup
}

/* SRS_UARTIO_27_135: [ If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_retrieveoptions()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_retrieveoptions_SCENARIO_invalid_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    OPTIONHANDLER_HANDLE options;

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    options = io_interface->concrete_io_retrieveoptions(MOCK_NON_NULL_XIO_HANDLE);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(options);

    // Cleanup
}

/* SRS_UARTIO_27_036: [ `uartio_retrieveoptions()` shall produce an instance of `OPTIONHANDLER_HANDLE` by calling `OptionHandler_Create()`. ] */
TEST_FUNCTION(uartio_retrieveoptions_SCENARIO_success)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options;

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));

    // Act
    options = io_interface->concrete_io_retrieveoptions(xio);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(options);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_037: [ If producing the `OPTIONHANDLER_HANDLE` fails, then `uartio_retrieveoptions()` shall fail and return `NULL`. ] */
TEST_FUNCTION(uartio_retrieveoptions_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    OPTIONHANDLER_HANDLE options;

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(OptionHandler_Create(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetFailReturn(NULL);
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        options = io_interface->concrete_io_retrieveoptions(xio);

        // Assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(options);
    }

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
    umock_c_negative_tests_deinit();
}


/* uartio_send */

/* SRS_UARTIO_27_038: [ If the argument `io_handle` is `NULL`, `uartio_send()` shall fail and return a non-zero value. ] */
/* SRS_UARTIO_27_039: [ If `uartio_send()` fails, the callback `on_send_complete()` shall be called, while passing `on_send_complete_context` and `IO_SEND_ERROR` as arguments. ] */
TEST_FUNCTION(uartio_send_SCENARIO_NULL_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_send_complete(&result, IO_SEND_ERROR));

    // Act
    result = io_interface->concrete_io_send(NULL, "AT", 2, mock_on_send_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_136: [ If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_send()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_send_SCENARIO_invalid_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_send_complete(&result, IO_SEND_ERROR));

    // Act
    result = io_interface->concrete_io_send(MOCK_NON_NULL_XIO_HANDLE, "AT", 2, mock_on_send_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_137: [ `uartio_send()` shall send each byte of `buffer` by calling `(void)EUSCI_A_UART_transmitData(uint16_t baseAddress, uint8_t transmitData)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the byte to be sent, `buffer_size` times. ] */
/* SRS_UARTIO_27_040: [ If `uartio_send()` completes without errors, `on_send_complete()` shall be called while passing to it the `on_send_complete_context` value and `IO_SEND_OK` as arguments. ] */
/* SRS_UARTIO_27_041: [ If `uartio_send()` completes without errors, it shall return 0. ] */
/* SRS_UARTIO_27_049: [ If the buffer passed as argument `buffer` has NULL bytes(0x00), `uartio_send()` shall send those bytes to the UART controller. ] */  
TEST_FUNCTION(uartio_send_SCENARIO_success)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);
    result = io_interface->concrete_io_open(xio, mock_on_io_open_complete, &result, mock_on_bytes_received, NULL, mock_on_io_error, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(EUSCI_A_UART_transmitData(EUSCI_A1_BASE, 'A'));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_transmitData(EUSCI_A1_BASE, 'T'));
    STRICT_EXPECTED_CALL(EUSCI_A_UART_transmitData(EUSCI_A1_BASE, '\0'));
    STRICT_EXPECTED_CALL(mock_on_send_complete(&result, IO_SEND_OK));

    // Act
    result = io_interface->concrete_io_send(xio, "AT", sizeof("AT"), mock_on_send_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_close(xio, mock_on_io_close_complete, &result);
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_042: [ If the argument `buffer` is `NULL`, `uartio_send()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_send_SCENARIO_NULL_buffer)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_send_complete(&result, IO_SEND_ERROR));

    // Act
    result = io_interface->concrete_io_send(xio, NULL, 2, mock_on_send_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_043: [ If `buffer_size` is 0, `uartio_send()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_send_SCENARIO_buffer_size_0)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_send_complete(&result, IO_SEND_ERROR));

    // Act
    result = io_interface->concrete_io_send(xio, "AT", 0, mock_on_send_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_044: [ If parameter `on_send_complete` is NULL, `uartio_send()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_send_SCENARIO_NULL_on_send_complete)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_send(xio, "AT", 2, NULL, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_045: [ If `uartio_send()` is called when the IO is not open, `uartio_send()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_send_SCENARIO_xio_not_open)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_send_complete(&result, IO_SEND_ERROR));

    // Act
    result = io_interface->concrete_io_send(xio, "AT", 2, mock_on_send_complete, &result);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}


/* uartio_setoption */

/* SRS_UARTIO_27_046: [ If the argument `io_handle` is `NULL` `uartio_setoption()` shall return a non-zero value. ] */
TEST_FUNCTION(uartio_setoption_SCENARIO_NULL_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_setoption(NULL, "Options", "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_138: [ If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_setoption()` shall fail and return a non-zero value. ] */
TEST_FUNCTION(uartio_setoption_SCENARIO_invalid_handle)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_setoption(MOCK_NON_NULL_XIO_HANDLE, "Options", "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
}

/* SRS_UARTIO_27_047: [ If the argument `option_name` is `NULL` `uartio_setoption()` shall return a non-zero value. ] */
TEST_FUNCTION(uartio_setoption_SCENARIO_NULL_option_name)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_setoption(xio, NULL, "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}

/* SRS_UARTIO_27_048: [ If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_setoption()` shall return a non-zero value. ] */
TEST_FUNCTION(uartio_setoption_SCENARIO_invalid_option_name)
{
    // Arrange
    const IO_INTERFACE_DESCRIPTION * io_interface = uartio_get_interface_description();
    int result;
    UARTIO_CONFIG config = { 9600, 4 };
    CONCRETE_IO_HANDLE xio = io_interface->concrete_io_create(&config);
    ASSERT_IS_NOT_NULL(xio);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    result = io_interface->concrete_io_setoption(xio, "Options", "NotSupported");

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);

    // Cleanup
    (void)io_interface->concrete_io_destroy(xio);
}


END_TEST_SUITE(uartio_msp430_unittests)


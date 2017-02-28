// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
  #include <cstdbool>
  #include <cstdlib>
  #include <ctime>
#else
  #include <stdbool.h>
  #include <stdlib.h>
  #include <time.h>
#endif

// Test framework #includes
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"

#define disableNegativeTest(x, y) ((x) |= ((uint64_t)1 << (y)))
#define enableNegativeTest(x, y) ((x) &= ~((uint64_t)1 << (y)))
#define skipNegativeTest(x, y) ((x) & ((uint64_t)1 << (y)))

// External library dependencies

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static
void *
non_mocked_calloc(
    size_t nmemb_,
    size_t size_
) {
    return calloc(nmemb_, size_);
}

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

// Under test #includes
#include "azure_c_shared_utility/atrpc.h"

#define ENABLE_MOCKS
  // #include SDK dependencies here
  #include "azure_c_shared_utility/gballoc.h"
  #include "../adapters/tickcounter_msp430.h"    // BKTODO: shouldn't be here.
  #include "azure_c_shared_utility/uartio.h"
  #include "azure_c_shared_utility/vector.h"
#undef ENABLE_MOCKS

#define MOCK_IO_INTERFACE_DESCRIPTION_PTR (const IO_INTERFACE_DESCRIPTION *)0x09171979
#define MOCK_TICKCOUNTER (TICK_COUNTER_HANDLE)0x19790917
#define MOCK_UARTIO (XIO_HANDLE)0x17091979

#ifdef __cplusplus
  extern "C" {
#endif

static void * mock_tickcounter_memory;
static void * mock_xio_memory;
static ON_BYTES_RECEIVED intercepted_xio_on_bytes_received;
static void * intercepted_xio_on_bytes_received_context;
static ON_IO_CLOSE_COMPLETE intercepted_xio_on_io_close_complete;
static void * intercepted_xio_on_io_close_context;
static ON_IO_ERROR intercepted_xio_on_io_error;
static void * intercepted_xio_on_io_error_context;
static ON_IO_OPEN_COMPLETE intercepted_xio_on_io_open_complete;
static void * intercepted_xio_on_io_open_context;
static ON_SEND_COMPLETE intercepted_xio_on_send_complete;
static void * intercepted_xio_on_send_context;

void dump_recent_bytes()
{
}

static
TICK_COUNTER_HANDLE
mock_tickcounter_create(
    void
) {
    mock_tickcounter_memory = non_mocked_malloc(1);
    return MOCK_TICKCOUNTER;
}

static
void
mock_tickcounter_destroy(
    TICK_COUNTER_HANDLE handle_
) {
    (void)handle_;
    non_mocked_free(mock_tickcounter_memory);
    mock_tickcounter_memory = NULL;
    return;
}

static
const IO_INTERFACE_DESCRIPTION *
mock_uartio_get_interface_description (
    void
) {
    return MOCK_IO_INTERFACE_DESCRIPTION_PTR;
}

static
int
mock_xio_close(
    XIO_HANDLE handle_,
    ON_IO_CLOSE_COMPLETE on_io_close_complete_,
    void * on_io_close_context_
) {
    (void)handle_;
    intercepted_xio_on_io_close_complete = on_io_close_complete_;
    intercepted_xio_on_io_close_context = on_io_close_context_;
    on_io_close_complete_(on_io_close_context_);
    return 0;
}

static
XIO_HANDLE
mock_xio_create(
    const IO_INTERFACE_DESCRIPTION * io_interface_description_,
    const void * io_parameters_
) {
    (void)io_interface_description_, io_parameters_;
    mock_xio_memory = non_mocked_malloc(1);
    return MOCK_UARTIO;
}

static
void
mock_xio_destroy (
    XIO_HANDLE handle_
) {
    (void)handle_;
    non_mocked_free(mock_xio_memory);
    mock_xio_memory = NULL;
    return;
}

static
int
mock_xio_open(
    XIO_HANDLE handle_,
    ON_IO_OPEN_COMPLETE on_io_open_complete_,
    void * on_io_open_context_,
    ON_BYTES_RECEIVED on_bytes_received_,
    void * on_bytes_received_context_,
    ON_IO_ERROR on_io_error_,
    void * on_io_error_context_
) {
    (void)handle_;
    intercepted_xio_on_bytes_received = on_bytes_received_;
    intercepted_xio_on_bytes_received_context = on_bytes_received_context_;
    intercepted_xio_on_io_error = on_io_error_;
    intercepted_xio_on_io_error_context = on_io_error_context_;
    intercepted_xio_on_io_open_complete = on_io_open_complete_;
    intercepted_xio_on_io_open_context = on_io_open_context_;
    return 0;
}

static
int
mock_xio_send(
    XIO_HANDLE handle_,
    const void * buffer_,
    size_t size_,
    ON_SEND_COMPLETE on_send_complete_,
    void * on_send_context_
) {
    (void)handle_, buffer_, size_;
    intercepted_xio_on_send_complete = on_send_complete_;
    intercepted_xio_on_send_context = on_send_context_;
    on_send_complete_(on_send_context_, IO_SEND_OK);
    return 0;
}

#ifdef __cplusplus 
  }
#endif

#define ENABLE_MOCKS
  // define free mocked function(s) and enum type(s) (platform, external libraries, etc.)
  MOCKABLE_FUNCTION(, void, mock_on_open_response, void *, context, TA_RESULT_CODE, result_code);
  MOCKABLE_FUNCTION(, void, mock_on_ta_response, void *, context, TA_RESULT_CODE, result_code, const unsigned char *, response, size_t, response_size);

  MOCK_FUNCTION_WITH_CODE(, int, mock_custom_ta_result_code_parser, void *, context_, unsigned char, input_, TA_RESULT_CODE *, ta_result_code_)
      static size_t machine_state = 0;
      int response_complete = false;
      *ta_result_code_ = ERROR_ATRPC;

      switch (machine_state) {
        case 0:
          machine_state += ( 'S' == input_ );
          break;
        case 1:
          machine_state += ( 'H' == input_ );
          break;
        case 2:
          machine_state += ( 'U' == input_ );
          break;
        case 3:
          machine_state += ( 'T' == input_ );
          break;
        case 4:
          machine_state += ( ' ' == input_ );
          break;
        case 5:
          machine_state += ( 'O' == input_ );
          break;
        case 6:
          machine_state += ( 'K' == input_ );
          break;
        case 7:
          machine_state += ( '\r' == input_ );
          break;
        case 8:
          response_complete = ('\n' == input_);
          *ta_result_code_ = OK_3GPP;
          machine_state = 0;
          break;
      }
  MOCK_FUNCTION_END(response_complete);
#undef ENABLE_MOCKS

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(atrpc_unittests)

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
    REGISTER_UMOCK_ALIAS_TYPE(CUSTOM_TA_RESULT_CODE_PARSER, void *);
    REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void *);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_CLOSE_COMPLETE, void *);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_ERROR, void *);
    REGISTER_UMOCK_ALIAS_TYPE(ON_IO_OPEN_COMPLETE, void *);
    REGISTER_UMOCK_ALIAS_TYPE(ON_SEND_COMPLETE, void *);
    REGISTER_UMOCK_ALIAS_TYPE(TA_RESULT_CODE, int);
    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void *);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void *);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, non_mocked_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, non_mocked_free);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, non_mocked_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(tickcounter_create, mock_tickcounter_create);
    REGISTER_GLOBAL_MOCK_HOOK(tickcounter_destroy, mock_tickcounter_destroy);
    REGISTER_GLOBAL_MOCK_HOOK(uartio_get_interface_description, mock_uartio_get_interface_description);
    REGISTER_GLOBAL_MOCK_HOOK(xio_close, mock_xio_close);
    REGISTER_GLOBAL_MOCK_HOOK(xio_create, mock_xio_create);
    REGISTER_GLOBAL_MOCK_HOOK(xio_destroy, mock_xio_destroy);
    REGISTER_GLOBAL_MOCK_HOOK(xio_open, mock_xio_open);
    REGISTER_GLOBAL_MOCK_HOOK(xio_send, mock_xio_send);
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

/* Tests_SRS_ATRPC_27_004: [ atrpc_attention() shall mark the call time, by calling (int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms) using the handle returned from atrpc_create() as the handle parameter. ] */
/* Tests_SRS_ATRPC_27_006: [ atrpc_attention() store the command string, by calling (void *)malloc(size_t size) using (command_string_length + 3) for the size parameter. ] */
/* Tests_SRS_ATRPC_27_008: [ atrpc_attention() shall call (int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context) using the xio handle returned from xio_create() for the handle parameter, and AT<command_string>\r for the buffer parameter, and (command_string_length + 3) for the size parameter. ] */
/* Tests_SRS_ATRPC_27_010: [ atrpc_attention() shall block until the on_send_complete callback passed to xio_send() returns. ] */
/* Tests_SRS_ATRPC_27_011: [ If no errors are encountered during execution, then atrpc_attention() shall return 0. ] */
TEST_FUNCTION(attention_SCENARIO_success)
{
    // Arrange
    int error;
    char at_command[] = "AT&W\r";
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(0);
    STRICT_EXPECTED_CALL(gballoc_malloc(2 + 3));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, (2 + 3), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, at_command, (sizeof(at_command) - 1));

    // Act
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 100, NULL, 0, mock_on_ta_response, &atrpc, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_005: [ If the call to tickcounter_get_current_ms() returns a non-zero value, then atrpc_attention() shall fail and return a non-zero value. ] */
/* Tests_SRS_ATRPC_27_007: [ If the call to malloc() returns NULL, then atrpc_attention() shall fail and return a non-zero value. ] */
/* Tests_SRS_ATRPC_27_009: [ If the call to xio_send() returns a non-zero value, then atrpc_attention() shall fail and return a non-zero value. ] */
/* Tests_SRS_ATRPC_27_012: [ VALGRIND - If the call to xio_send() returns a non-zero value, then the stored command string shall be freed. ] */
TEST_FUNCTION(attention_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    uint64_t negativeTestsToSkip = 0;
    size_t test_index = 0;
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    srand((unsigned int)time(NULL));
    int error;
    char at_command[] = "AT&W\r";
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));

    // Expected call listing
    umock_c_reset_all_calls();
    enableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetFailReturn(__LINE__)
        .SetReturn(0);
    enableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(gballoc_malloc(2 + 3))
        .SetFailReturn(NULL);
    enableNegativeTest(negativeTestsToSkip, test_index++);
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, (2 + 3), IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .SetFailReturn(__LINE__)
        .ValidateArgumentBuffer(2, at_command, (sizeof(at_command) - 1));
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if (skipNegativeTest(negativeTestsToSkip, i)) { continue; }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 100, NULL, 0, mock_on_ta_response, &atrpc, NULL, NULL);

        // Assert
        ASSERT_ARE_NOT_EQUAL(int, 0, error);
    }

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);

    umock_c_negative_tests_deinit();
}

/* Tests_SRS_ATRPC_27_000: [ If the handle argument is NULL, then atrpc_attention() shall fail and return a non-zero value. ] */
TEST_FUNCTION(attention_SCENARIO_NULL_handle)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_attention(NULL, (const unsigned char *)"&W", (sizeof("&W") - 1), 100, NULL, 0, mock_on_ta_response, &atrpc, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_001: [ If the on_ta_response argument is NULL, then atrpc_attention() shall fail and return a non-zero value. ] */
TEST_FUNCTION(attention_SCENARIO_NULL_on_ta_response)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 100, NULL, 0, NULL, &atrpc, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_085: [ If command_string_length is not zero and command_string is NULL, then atrpc_attention() shall fail and return a non-zero value. ] */
TEST_FUNCTION(attention_SCENARIO_command_string_mismatch)
{
    // Arrange
    int error;
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_attention(atrpc, NULL, 2, 100, NULL, 0, mock_on_ta_response, &atrpc, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_086: [ If ta_response_buffer_size is not zero and response_buffer is not NULL, then atrpc_attention() shall fail and return a non-zero value. ] */
TEST_FUNCTION(attention_SCENARIO_response_string_mismatch)
{
    // Arrange
    int error;
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_attention(atrpc, NULL, 0, 100, NULL, 4, mock_on_ta_response, &atrpc, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_002: [ If the on_io_open_complete() callback from the underlying xio has not been called, then atrpc_attention() shall fail and return a non-zero value. ] */
TEST_FUNCTION(attention_SCENARIO_handle_not_open)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 100, NULL, 0, mock_on_ta_response, &atrpc, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_003: [ If a command is currently outstanding, then atrpc_attention() shall fail and return a non-zero value. ] */
TEST_FUNCTION(attention_SCENARIO_request_outstanding)
{
    // Arrange
    int error;
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 0, NULL, 0, mock_on_ta_response, &atrpc, NULL, NULL);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 0, NULL, 0, mock_on_ta_response, &atrpc, NULL, NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_016: [ atrpc_close() shall call (int)xio_close(XIO_HANDLE handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void * on_io_close_complete_context). ] */
/* Tests_SRS_ATRPC_27_018: [ atrpc_close() shall block until the on_io_close_complete callback passed to xio_close() completes. ] */
/* Tests_SRS_ATRPC_27_019: [ If no errors are encountered during execution, then atrpc_close() shall return 0. ] */
TEST_FUNCTION(close_SCENARIO_success)
{
    // Arrange
    int error;
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_close(MOCK_UARTIO, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(3);

    // Act
    error = atrpc_close(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_017: [ If the call to xio_close() returns a non-zero value, then atrpc_close() shall fail and return a non-zero value. ] */
TEST_FUNCTION(close_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    uint64_t negativeTestsToSkip = 0;
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_close(MOCK_UARTIO, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(3)
        .SetFailReturn(__LINE__)
        .SetReturn(0);
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if (skipNegativeTest(negativeTestsToSkip, i)) { continue; }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        error = atrpc_close(atrpc);

        // Assert
        ASSERT_ARE_NOT_EQUAL(int, 0, error);
    }

    // Cleanup
    atrpc_destroy(atrpc);

    umock_c_negative_tests_deinit();
}

/* Tests_SRS_ATRPC_27_013: [ If the handle argument is NULL, then atrpc_close() shall fail and return a non-zero value. ] */
TEST_FUNCTION(close_SCENARIO_NULL_handle)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_close(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_014: [ If atrpc_open() has not been called on the handle, atrpc_close() shall do nothing and return 0. ] */
TEST_FUNCTION(close_SCENARIO_already_closed)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_close(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_015: [ If atrpc_open() has been called on the handle and the on_open_complete callback has not been called, atrpc_close() shall call the (void)on_open_complete(void * context, ta_result_code result_code, char * response) callback provided to atrpc_open(), using the on_open_complete_context argument provided to atrpc_open() as the context parameter, and ERROR_ATRPC as the result_code parameter. ] */
TEST_FUNCTION(close_SCENARIO_cancel_open)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_close(MOCK_UARTIO, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    error = atrpc_close(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_020: [ atrpc_create() shall call malloc() to allocate the memory required for the internal data structure. ] */
/* Tests_SRS_ATRPC_27_022: [ atrpc_create() shall create a tickcounter to support timeout functionality by calling (TICKCOUNTER_HANDLE)tickcounter_create(void). ] */
/* Tests_SRS_ATRPC_27_024: [ atrpc_create() shall acquire an xio interface to a modem chipset by calling (IO_INTERFACE_DESCRIPTION *)uartio_get_interface_description(). ] */
/* Tests_SRS_ATRPC_27_026: [ atrpc_create() shall create an xio connection to a modem chipset by calling (XIO_HANDLE)xio_create(const IO_INTERFACE_DESCRIPTION * io_interface_description, const void * io_create_parameters) using the interface description returned from uartio_get_interface_description() for io_interface_description. ] */
/* Tests_SRS_ATRPC_27_028: [ atrpc_create() shall create a vector to buffer responses from the terminal adapter. ] */
/* Tests_SRS_ATRPC_27_031: [ If no errors are encountered during execution, atrpc_create() shall return a handle to an AT RPC instance. ] */
TEST_FUNCTION(create_SCENARIO_success)
{
    // Arrange
    ATRPC_HANDLE atrpc = NULL;

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(tickcounter_create())
        .SetReturn(MOCK_TICKCOUNTER);
    STRICT_EXPECTED_CALL(uartio_get_interface_description())
        .SetReturn(MOCK_IO_INTERFACE_DESCRIPTION_PTR);
    STRICT_EXPECTED_CALL(xio_create(MOCK_IO_INTERFACE_DESCRIPTION_PTR, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetReturn(MOCK_UARTIO);

    // Act
    atrpc = atrpc_create();

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(atrpc);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_021: [ If malloc() fails to allocate the memory required for the internal data structure, then atrpc_create() shall fail and return a NULL handle. ] */
/* Tests_SRS_ATRPC_27_023: [ If the call to tickcounter_create() returns NULL, then atrpc_create() shall fail and return NULL. ] */
/* Tests_SRS_ATRPC_27_025: [ If the call to uartio_get_interface_description() returns NULL, then atrpc_create() shall fail and return NULL. ] */
/* Tests_SRS_ATRPC_27_027: [ If the call to xio_create() returns NULL, then atrpc_create() shall fail and return NULL. ] */
/* Tests_SRS_ATRPC_27_030: [ VALGRIND - When atrpc_create() returns a non-zero value, all allocated resources up to that point shall be freed. ] */
TEST_FUNCTION(create_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    uint64_t negativeTestsToSkip = 0;
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    ATRPC_HANDLE atrpc = NULL;

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_calloc(IGNORED_NUM_ARG, IGNORED_NUM_ARG))
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(tickcounter_create())
        .SetReturn(MOCK_TICKCOUNTER)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(uartio_get_interface_description())
        .SetReturn(MOCK_IO_INTERFACE_DESCRIPTION_PTR)
        .SetFailReturn(NULL);
    STRICT_EXPECTED_CALL(xio_create(MOCK_IO_INTERFACE_DESCRIPTION_PTR, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .SetFailReturn(NULL)
        .SetReturn(MOCK_UARTIO);
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if (skipNegativeTest(negativeTestsToSkip, i)) { continue; }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        atrpc = atrpc_create();

        // Assert
        ASSERT_IS_NULL(atrpc);
    }

    // Cleanup
    umock_c_negative_tests_deinit();
}

/* Tests_SRS_ATRPC_27_034: [ atrpc_destroy() shall call (void)xio_destroy(XIO_HANDLE handle) using the handle returned from the call to xio_create() for the handle parameter. ] */
/* Tests_SRS_ATRPC_27_036: [ atrpc_destroy() shall call (void)tickcounter_destroy(TICKCOUNTER_HANDLE handle) using the handle returned from the call to tickcounter_create() for the handle parameter. ] */
/* Tests_SRS_ATRPC_27_037: [ atrpc_destroy() shall free the memory required for current request. ] */
/* Tests_SRS_ATRPC_27_038: [ atrpc_destroy() shall free the memory required for the internal data structure. ] */
TEST_FUNCTION(destroy_SCENARIO_success)
{
    // Arrange
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_destroy(MOCK_UARTIO));
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(atrpc));

    // Act
    atrpc_destroy(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* Tests_SRS_ATRPC_27_032: [ If the handle argument is NULL, then atrpc_destroy() shall do nothing. ] */
TEST_FUNCTION(destroy_SCENARIO_NULL_handle)
{
    // Arrange

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    atrpc_destroy(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* Tests_SRS_ATRPC_27_033: [ If atrpc_open() has previously been called and atrpc_close() has not been called on the handle, atrpc_destroy() shall call (int)atrpc_close(ATRPC_HANDLE handle) using the handle argument passed to atrpc_destroy() as the handle parameter. ] */
TEST_FUNCTION(destroy_SCENARIO_close_not_called_before_destroy)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_close(MOCK_UARTIO, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(3);
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));
    STRICT_EXPECTED_CALL(xio_destroy(MOCK_UARTIO));
    STRICT_EXPECTED_CALL(tickcounter_destroy(MOCK_TICKCOUNTER));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(gballoc_free(atrpc));

    // Act
    atrpc_destroy(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
}

/* Tests_SRS_ATRPC_27_041: [ If atrpc_open() has been called on the handle, atrpc_dowork() shall mark the call time, by calling (int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms) using the handle returned from atrpc_create() as the handle parameter. ] */
/* Tests_SRS_ATRPC_27_044: [ If atrpc_open() has been called on the handle, and the timeout value sent as timeout_ms to the originating attention() call is non-zero and has expired, then atrpc_dowork() shall free the stored command string. ] */
/* Tests_SRS_ATRPC_27_045: [ If atrpc_open() has been called on the handle, and the timeout value sent as timeout_ms to the originating attention() call is non-zero and has expired, then atrpc_dowork() shall call the terminal adapter response callback passed as on_ta_response to attention() using the ta_response_context parameter passed to attention() as the context parameter, ERROR_ATRPC as the result_code parameter, and NULL as the message parameter. ] */
TEST_FUNCTION(dowork_SCENARIO_handle_open_response_timeout) {
    // Arrange
    int error;
    tickcounter_ms_t send_ms = 150, work_ms = 475;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    // on_io_open_complete() calls
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &send_ms, sizeof(send_ms))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT\r", 3);
    // atrpc_dowork() calls
    STRICT_EXPECTED_CALL(xio_dowork(MOCK_UARTIO));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &work_ms, sizeof(work_ms))
        .IgnoreArgument(2)
        .SetReturn(0);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, atrpc))
        .IgnoreArgument(4)
        .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));  // Confirms ERROR_ATRPC was passed to on_ta_response

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // Initiate the handshake procedure
    atrpc_dowork(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_042: [ If tickcounter_get_current_ms() returns a non-zero value, then dowork() shall not attempt to calculate a timeout. ] */
TEST_FUNCTION(dowork_SCENARIO_open_timeout_fails) {
    // Arrange
    int error;
    tickcounter_ms_t send_ms = 150, work_ms = 475;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    // on_io_open_complete() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &send_ms, sizeof(send_ms))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));
    // atrpc_dowork() calls
    STRICT_EXPECTED_CALL(xio_dowork(MOCK_UARTIO));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &work_ms, sizeof(work_ms))
        .IgnoreArgument(2)
        .SetReturn(__LINE__);

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK); // Allows capture of on_send_complete and begins handshake
    atrpc_dowork(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_043: [ If attention() was called with a timeout of 0, then dowork() shall not attempt to calculate a timeout. ] */
TEST_FUNCTION(dowork_SCENARIO_open_no_timeout) 
{
    // Arrange
    int error;
    tickcounter_ms_t send_ms = 150;
    const char bytes_from_modem[] = "AT&W\r0\r\n";
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Expected call listing
    umock_c_reset_all_calls();
    // atrpc_attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &send_ms, sizeof(send_ms))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(xio_send(IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_NUM_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG));
    // atrpc_dowork() calls
    STRICT_EXPECTED_CALL(xio_dowork(MOCK_UARTIO));

    // Act
    atrpc_attention(atrpc, NULL, 0, 0, NULL, 0, mock_on_ta_response, atrpc, NULL, NULL);
    atrpc_dowork(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_039: [ If the handle argument is NULL, then atrpc_dowork() shall do nothing. ] */
TEST_FUNCTION(dowork_SCENARIO_NULL_handle) {
    // Arrange
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    atrpc_dowork(NULL);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_040: [ atrpc_dowork() shall call (void)xio_dowork(XIO_HANDLE handle) using the handle returned from the call to xio_create() for the handle parameter. ] */
TEST_FUNCTION(dowork_SCENARIO_work_underlying_io) {
    // Arrange
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_dowork(MOCK_UARTIO));

    // Act
    atrpc_dowork(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_049: [ atrpc_open() shall call (int)xio_open(XIO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void * on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void * on_bytes_received_context, ON_IO_ERROR on_io_error, void * on_io_error_context) using the handle returned from xio_create() as the handle parameter, the incoming handle parameter as the on_bytes_received_context parameter, and the incoming handle parameter as the on_io_open_complete_context parameter. ] */
/* Tests_SRS_ATRPC_27_051: [ If no errors are encountered, atrpc_open() shall return 0. ] */
TEST_FUNCTION(open_SCENARIO_success)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_open(MOCK_UARTIO, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(3)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .IgnoreArgument(6)
        .IgnoreArgument(7)
        .SetReturn(0);
        
    // Act
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, error);

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_050: [ If xio_open() returns a non-zero value, then atrpc_open() shall do nothing and return a non-zero value. ] */
TEST_FUNCTION(open_SCENARIO_negative_tests)
{
    // Arrange
    int negativeTestsInitResult = umock_c_negative_tests_init();
    uint64_t negativeTestsToSkip = 0;
    ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);

    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(xio_open(MOCK_UARTIO, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(3)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .IgnoreArgument(6)
        .IgnoreArgument(7)
        .SetFailReturn(__LINE__)
        .SetReturn(0);
    umock_c_negative_tests_snapshot();

    for (size_t i = 0; i < umock_c_negative_tests_call_count(); ++i)
    {
        if (skipNegativeTest(negativeTestsToSkip, i)) { continue; }
        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        // Act
        error = atrpc_open(atrpc, mock_on_open_response, atrpc);

        // Assert
        ASSERT_ARE_NOT_EQUAL(int, 0, error);
    }

    // Cleanup
    atrpc_destroy(atrpc);

    umock_c_negative_tests_deinit();
}

/* Tests_SRS_ATRPC_27_046: [ If the handle argument is NULL, then atrpc_open() shall fail and return a non-zero value. ] */
/* Tests_SRS_ATRPC_27_077: [ If any errors are encountered, atrpc_open() shall call on_open_complete using the on_open_complete_context, and a TA_RESULT_CODE of ERROR_ATRPC. ] */
TEST_FUNCTION(open_SCENARIO_NULL_handle)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    error = atrpc_open(NULL, mock_on_open_response, atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_047: [ If the on_open_complete argument is NULL, then atrpc_open() shall fail and return a non-zero value. ] */
TEST_FUNCTION(open_SCENARIO_NULL_on_open_complete)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    error = atrpc_open(atrpc, NULL, atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_048: [ If atrpc_open() has been called previously and atrpc_close() has not been called on the handle, atrpc_open() shall fail and return a non-zero value. ] */
TEST_FUNCTION(open_SCENARIO_attempt_to_open_again_before_closing)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, error);

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_056: [ If the ping times-out when negotiating auto-baud, then modem_on_bytes_received() shall reissue the ping by calling (int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context) using the xio handle returned from xio_create() for the handle parameter, and AT\r for the buffer parameter, and 3 for the size parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_auto_baud_timeout)
{
    // Arrange
    int error;
    tickcounter_ms_t send_ms = 150, work_ms = 475;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    // on_io_open_complete() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &send_ms, sizeof(send_ms))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));
    // atrpc_dowork() calls
    STRICT_EXPECTED_CALL(xio_dowork(MOCK_UARTIO));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &work_ms, sizeof(work_ms))
        .IgnoreArgument(2)
        .SetReturn(0);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK); // Begins handshake
    atrpc_dowork(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_057: [ If atrpc_attention returns a non-zero value, then on_io_open_complete() shall call the on_open_complete callback passed to atrpc_open() using the on_open_complete_context parameter passed to atrpc_open() as the context parameter, ERROR_3GPP as the result_code parameter, and NULL as the response parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_auto_baud_resend_failure)
{
    // Arrange
    int error;
    tickcounter_ms_t send_ms = 150, work_ms = 475;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    // on_io_open_complete() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &send_ms, sizeof(send_ms))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));
    // atrpc_dowork() calls
    STRICT_EXPECTED_CALL(xio_dowork(MOCK_UARTIO));
    STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &work_ms, sizeof(work_ms))
        .IgnoreArgument(2)
        .SetReturn(0);
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .SetReturn(__LINE__)
        .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK); // Allows capture of on_send_complete and begins handshake
    atrpc_dowork(atrpc);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_065: [ During auto-baud negotiation, if 50 or more time-outs occur, then modem_handshake() shall call the (void)on_open_complete(void * context, ta_result_code result_code, char * response) callback provided to atrpc_open(), using the on_open_complete_context argument provided to atrpc_open() as the context parameter, ERROR_ATRPC as the result_code parameter, and NULL as the response parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_max_timeouts)
{
    // Arrange
    const int MAX_ATTEMPTS = 50;
    int error;
    tickcounter_ms_t send_ms = 150, work_ms = 475;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    // on_io_open_complete() calls -> handshake() -> attention()
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .CopyOutArgumentBuffer(2, &send_ms, sizeof(send_ms))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT\r", 3);
    for (int i = 0; i < MAX_ATTEMPTS; ++i) {
        // atrpc_dowork() calls
        STRICT_EXPECTED_CALL(xio_dowork(MOCK_UARTIO));
        STRICT_EXPECTED_CALL(tickcounter_get_current_ms(MOCK_TICKCOUNTER, IGNORED_PTR_ARG))
            .CopyOutArgumentBuffer(2, &work_ms, sizeof(work_ms))
            .IgnoreArgument(2)
            .SetReturn(0);
        EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        if (i < (MAX_ATTEMPTS - 1)) {
            // attention() calls
            EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
                .CopyOutArgumentBuffer(2, &send_ms, sizeof(send_ms))
                .SetReturn(0);
            EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
            STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
                .IgnoreArgument(2)
                .IgnoreArgument(4)
                .IgnoreArgument(5)
                .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));
        }
    }
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK); // Begins handshake
    for (int i = 0; i < MAX_ATTEMPTS; ++i) {
        atrpc_dowork(atrpc);
    }

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_058: [ During auto-baud negotiation, modem_on_bytes_received() shall accept "0\r" as a valid response. ] */
/* Tests_SRS_ATRPC_27_060: [ Once a complete response has been received, then modem_on_bytes_received() shall free the stored command string. ] */
/* Tests_SRS_ATRPC_27_061: [ When auto-baud negotiation has completed, then modem_on_bytes_received() shall normalize the ta responses by calling (int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context) using the xio handle returned from xio_create() during atrpc_create() for the handle parameter, and ATE1V0\r for the buffer parameter, and 7 for the size parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_auto_baud_success)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "AT\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks

    // Expected call listing
    umock_c_reset_all_calls();
    // modem_on_bytes_received() calls
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "ATE1V0\r", (sizeof("ATE1V0\r") - 1));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_059: [ During auto-baud negotiation, modem_on_bytes_received() shall accept "\r\nOK\r\n" as a valid response. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_verbose_auto_baud_success)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "AT\r\r\nOK\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks

    // Expected call listing
    umock_c_reset_all_calls();
    // modem_on_bytes_received() calls
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "ATE1V0\r", (sizeof("ATE1V0\r") - 1));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_062: [ If the call to attention() returns a non-zero value, then modem_handshake() shall call the (void)on_open_complete(void * context, ta_result_code result_code, char * response) callback provided to atrpc_open(), using the on_open_complete_context argument provided to atrpc_open() as the context parameter, 3GPP_ERROR as the result_code parameter, and NULL as the response parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_normalization_failure)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "AT\r\r\nOK\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks

    // Expected call listing
    umock_c_reset_all_calls();
    // modem_on_bytes_received() calls
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    // attention() calls
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 7, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .SetReturn(__LINE__)
        .ValidateArgumentBuffer(2, "ATE1V0\r", (sizeof("ATE1V0\r") - 1));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_063: [ Once the communication with the modem has been normalized, modem_on_bytes_received() shall write the active profile by calling (int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context), using the context argument for the handle parameter, &W as the command_string parameter, 2 as the command_string_length parameter, 0 as the timeout_ms parameter, modem_handshake as the ta_response parameter, and the context argument as the ta_response_context parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_normalization_success)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"

    // Expected call listing
    umock_c_reset_all_calls();
    // modem_on_bytes_received() calls
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 5, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT&W\r", (sizeof("AT&W\r") - 1));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_064: [ If the call to attention() returns a non-zero value, then modem_handshake() shall call the (void)on_open_complete(void * context, ta_result_code result_code, char * response) callback provided to atrpc_open(), using the on_open_complete_context argument provided to atrpc_open() as the context parameter, 3GPP_ERROR as the result_code parameter, and "XIO ERROR: Unable to write the active profile!" as the response parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_write_profile_failure)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"

    // Expected call listing
    umock_c_reset_all_calls();
    // modem_on_bytes_received() calls
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    // attention() calls
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 5, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .SetReturn(__LINE__)
        .ValidateArgumentBuffer(2, "AT&W\r", (sizeof("AT&W\r") - 1));
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_066: [ Once the profile has been successfully stored, then modem_on_bytes_received() shall call the (void)on_open_complete(void * context, ta_result_code result_code, char * response) callback provided to atrpc_open(), using the on_open_complete_context argument provided to atrpc_open() as the context parameter, 3GPP_OK as the result_code parameter, and NULL as the response parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_handshake_complete)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "AT&W\r0\r\n";
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, OK_3GPP));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_052: [ If atrpc_open() has not been called on the handle (passed in the callback context), then modem_on_bytes_received() shall discard all bytes. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_atrpc_not_open)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "AT\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_053: [ If the on_open_complete() callback has been called, then modem_on_bytes_received() shall capture any bytes following the prefix of the command_string parameter passed to attention() along with the postfixed <result code>"\r". ] */
/* Tests_SRS_ATRPC_27_054: [ If any bytes where captured, modem_on_bytes_received() shall call the ta_response callback passed to attention() using the ta_response_context as the context parameter, the captured result code as the result_code parameter, and the captured message as the message parameter. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_parse_response)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "RDY\r\n+CFUN: 1\r\nAT+GMM\rSIMCOM_SIM808\r\n0\r\n";
    unsigned char bytes_to_capture[(sizeof("SIMCOM_SIM808\r\n0\r") - 1)];
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));
    error = atrpc_attention(atrpc, (const unsigned char *)"+GMM", (sizeof("+GMM") - 1), 0, bytes_to_capture, sizeof(bytes_to_capture), mock_on_ta_response, atrpc, NULL, NULL);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_ta_response(atrpc, OK_3GPP, bytes_to_capture, sizeof(bytes_to_capture)))
        .ValidateArgumentBuffer(3, "SIMCOM_SIM808\r\n0\r", (sizeof("SIMCOM_SIM808\r\n0\r") - 1));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_080: [ If more bytes where captured than can fit in the supplied, modem_on_bytes_received() shall fill the buffer(discarding the remaining bytes) and call the ta_response callback passed to attention() using the ta_response_context as the context parameter, the captured result code as the result_code parameter, a pointer to the buffer as the message parameter, and the size of the received message as size. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_parse_partial_response)
{
    // Arrange
    int error;
    const char bytes_from_modem[] = "RDY\r\n+CFUN: 1\r\nAT+GMM\rSIMCOM_SIM808\r\n0\r\n";
    unsigned char bytes_to_capture[(sizeof("SIMCOM_S") - 1)];
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));
    error = atrpc_attention(atrpc, (const unsigned char *)"+GMM", (sizeof("+GMM") - 1), 0, bytes_to_capture, sizeof(bytes_to_capture), mock_on_ta_response, atrpc, NULL, NULL);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_ta_response(atrpc, OK_3GPP, bytes_to_capture, sizeof(bytes_to_capture)))
        .ValidateArgumentBuffer(3, "SIMCOM_S", (sizeof("SIMCOM_S") - 1));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_081: [ When a CUSTOM_RESULT_CODE_PARSER callback is supplied to attention(), modem_on_bytes_received() shall call the callback with each byte to determine the end of a response instead of searching for a standard result code. ] */
/* Tests_SRS_ATRPC_27_082: [ When the CUSTOM_RESULT_CODE_PARSER callback indicates completion by returning a non-zero value, modem_on_bytes_received() shall return the result code value supplied by the callback as the result code sent to the ON_TA_RESULT callback. ] */
TEST_FUNCTION(modem_on_bytes_received_SCENARIO_custom_response_parser)
{
    // Arrange
    int error;
    unsigned char bytes_for_custom_processing[] = "0\r\nSHUT OK\r\n";
    const char bytes_from_modem[] = "RDY\r\n+CFUN: 1\r\nAT+CIPSHUT\r0\r\nSHUT OK\r\n0\r\n";
    unsigned char bytes_to_capture[(sizeof("0\r\nSHUT OK\r\n") - 1)];
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));
    error = atrpc_attention(atrpc, (const unsigned char *)"+CIPSHUT", (sizeof("+CIPSHUT") - 1), 0, bytes_to_capture, sizeof(bytes_to_capture), mock_on_ta_response, atrpc, mock_custom_ta_result_code_parser, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    for (size_t i = 0; i < (sizeof(bytes_for_custom_processing) - 1); ++i) {
        STRICT_EXPECTED_CALL(mock_custom_ta_result_code_parser(atrpc, bytes_for_custom_processing[i], IGNORED_PTR_ARG))
            .IgnoreArgument(3);
    }
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_ta_response(atrpc, OK_3GPP, bytes_to_capture, sizeof(bytes_to_capture)))
        .ValidateArgumentBuffer(3, "0\r\nSHUT OK\r\n", (sizeof("0\r\nSHUT OK\r\n") - 1));

    // Act
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)bytes_from_modem, (sizeof(bytes_from_modem) - 1));

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_067: [ on_io_close_complete() shall call nothing. ] */
TEST_FUNCTION(on_io_close_complete_SCENARIO_success)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    intercepted_xio_on_io_close_complete(intercepted_xio_on_io_close_context);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    atrpc_destroy(atrpc);
}

#if 0
// BKTODO: remove these requirements
/* Tests_SRS_ATRPC_27_068: [ on_io_error() shall free the stored command string. ] */
/* Tests_SRS_ATRPC_27_069: [ on_io_error() shall call the terminal adapter response callback passed as ta_response to attention() using the ta_response_context parameter passed to attention() as the context parameter, ERROR_ATRPC as the result_code parameter, and NULL as the message parameter. ] */
TEST_FUNCTION(on_io_error_SCENARIO_success)
{
    // Arrange
    int error;
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 0, NULL, 0, mock_on_ta_response, atrpc, NULL, NULL);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_ta_response(atrpc, ERROR_ATRPC, NULL, 0));

    // Act
    intercepted_xio_on_io_error(intercepted_xio_on_io_error_context);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}
#endif

/* Tests_SRS_ATRPC_27_071: [ If the open_result parameter is OPEN_OK, then on_io_open_complete() shall initiate the auto-bauding procedure by calling (void)atrpc_attention(void * context, const char * const command_string, const size_t command_string_length const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context) using the incoming context parameter as the handle parameter, NULL as the command_string parameter, 0 as the command_string_length parameter, 250 as the timeout_ms parameter, modem_handshake as the ta_response parameter, and ta_response_context as the context parameter. ] */
TEST_FUNCTION(on_io_open_complete_SCENARIO_success)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(0);
    EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(xio_send(MOCK_UARTIO, IGNORED_PTR_ARG, 3, IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .IgnoreArgument(2)
        .IgnoreArgument(4)
        .IgnoreArgument(5)
        .ValidateArgumentBuffer(2, "AT\r", (sizeof("AT\r") - 1));

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_070: [ If the open_result parameter is not IO_OPEN_OK, then on_io_open_complete() shall call the on_open_complete callback passed to atrpc_open() using the on_open_complete_context parameter passed to atrpc_open() as the context parameter, ERROR_ATRPC as the result_code parameter, and NULL as the response parameter. ] */
TEST_FUNCTION(on_io_open_complete_SCENARIO_xio_error)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_ERROR);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_072: [ If atrpc_attention returns a non-zero value, then on_io_open_complete() shall call the on_open_complete callback passed to atrpc_open() using the on_open_complete_context parameter passed to atrpc_open() as the context parameter, ERROR_ATRPC as the result_code parameter, and NULL as the response parameter. ] */
TEST_FUNCTION(on_io_open_complete_SCENARIO_attention_fails)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(tickcounter_get_current_ms(IGNORED_PTR_ARG, IGNORED_PTR_ARG))
        .SetReturn(__LINE__);
    STRICT_EXPECTED_CALL(mock_on_open_response(atrpc, ERROR_ATRPC));

    // Act
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_073: [ If the result of underlying xio on_io_send_complete() is IO_SEND_OK, then on_send_complete() shall call nothing. ] */
TEST_FUNCTION(on_send_complete_SCENARIO_io_send_ok)
{
    // Arrange
    int error;
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK); // Allows capture of on_send_complete

    // Expected call listing
    umock_c_reset_all_calls();

    // Act
    intercepted_xio_on_send_complete(intercepted_xio_on_send_context, IO_SEND_OK);
    
    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_074: [ If the result of underlying xio on_io_send_complete() is IO_SEND_CANCELLED, then on_send_complete() shall call the terminal adapter response callback passed as ta_response to attention() using the ta_response_context parameter passed to attention() as the context parameter, ERROR_ATRPC as the result_code parameter, and NULL as the message parameter. ] */
/* Tests_SRS_ATRPC_27_075: [ If the result of underlying xio on_io_send_complete() is not IO_SEND_OK, then on_send_complete() shall free the command string passed to attention(). ] */
TEST_FUNCTION(on_send_complete_SCENARIO_io_send_cancelled)
{
    // Arrange
    int error;
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 0, NULL, 0, mock_on_ta_response, atrpc, NULL, NULL);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_ta_response(atrpc, ERROR_ATRPC, NULL, 0));

    // Act
    intercepted_xio_on_send_complete(intercepted_xio_on_send_context, IO_SEND_CANCELLED);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

/* Tests_SRS_ATRPC_27_076: [ If the result of underlying xio on_io_send_complete() is IO_SEND_ERROR, then on_send_complete() shall call the terminal adapter response callback passed as ta_response to attention() using the ta_response_context parameter passed to attention() as the context parameter, ERROR_ATRPC as the result_code parameter, and NULL as the message parameter. ] */
TEST_FUNCTION(on_send_complete_SCENARIO_io_send_error)
{
    // Arrange
    int error;
    const char normalization_response_from_modem[] = "ATE1V0\r0\r\n";
    const char ping_response_from_modem[] = "AT\r\r\nOK\r\n";
    const char write_response_from_modem[] = "AT&W\r0\r\n";
    ATRPC_HANDLE atrpc = atrpc_create();
    ASSERT_IS_NOT_NULL(atrpc);
    error = atrpc_open(atrpc, mock_on_open_response, atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    intercepted_xio_on_io_open_complete(intercepted_xio_on_io_open_context, IO_OPEN_OK);  // call attention and store response callbacks
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)ping_response_from_modem, (sizeof(ping_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "ATE1V0\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)normalization_response_from_modem, (sizeof(normalization_response_from_modem) - 1));  // Advance the handshake to prime the matching string with "AT&W\r"
    intercepted_xio_on_bytes_received(intercepted_xio_on_bytes_received_context, (const unsigned char *)write_response_from_modem, (sizeof(write_response_from_modem) - 1));
    error = atrpc_attention(atrpc, (const unsigned char *)"&W", (sizeof("&W") - 1), 0, NULL, 0, mock_on_ta_response, atrpc, NULL, NULL);
    ASSERT_ARE_EQUAL(int, 0, error);

    // Expected call listing
    umock_c_reset_all_calls();
    EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(mock_on_ta_response(atrpc, ERROR_ATRPC, NULL, 0));

    // Act
    intercepted_xio_on_send_complete(intercepted_xio_on_send_context, IO_SEND_ERROR);

    // Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    // Cleanup
    error = atrpc_close(atrpc);
    ASSERT_ARE_EQUAL(int, 0, error);
    atrpc_destroy(atrpc);
}

END_TEST_SUITE(atrpc_unittests)


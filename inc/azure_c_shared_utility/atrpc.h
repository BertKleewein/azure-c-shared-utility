// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AT_RPC_H
#define AT_RPC_H

#include "azure_c_shared_utility/macro_utils.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct ATRPC_INSTANCE_TAG * ATRPC_HANDLE;

#define TA_RESULT_CODE_VALUES \
    OK_3GPP, \
    CONNECT_3GPP, \
    RING_3GPP, \
    NO_CARRIER_3GPP, \
    ERROR_3GPP, \
    ERROR_ATRPC, \
    NO_DIALTONE_3GPP, \
    BUSY_3GPP, \
    NO_ANSWER_3GPP, \
    PROCEEDING_SIMCOM, \
    TIMEOUT_ATRPC, \
    ERROR_AUTOBAUD

DEFINE_ENUM(TA_RESULT_CODE, TA_RESULT_CODE_VALUES);

typedef int(*CUSTOM_TA_RESULT_CODE_PARSER)(void * context, unsigned char input, TA_RESULT_CODE * result_code);
typedef void(*ON_ATRPC_OPEN_COMPLETE)(void * context, TA_RESULT_CODE result_code);
typedef void(*ON_ATRPC_TA_RESPONSE)(void * context, TA_RESULT_CODE result_code, const unsigned char * ta_response, size_t ta_response_size);
typedef void(*ON_ATRPC_RAW_DATA_RECEIVED)(void *context, const unsigned char *buffer, size_t size);
typedef void(*ON_ATRPC_SEND_RAW_DATA_COMPLETE)(void *context);

#include "azure_c_shared_utility/umock_c_prod.h"

MOCKABLE_FUNCTION(, int, atrpc_attention, ATRPC_HANDLE, handle, const unsigned char *, command_string, size_t, command_string_length, size_t, timeout_ms, unsigned char *, ta_response_buffer, size_t, ta_response_buffer_size, ON_ATRPC_TA_RESPONSE, on_ta_response, void *, ta_response_context, CUSTOM_TA_RESULT_CODE_PARSER, result_code_parser, void *, result_code_parser_context);
MOCKABLE_FUNCTION(, int, atrpc_close, ATRPC_HANDLE, handle);
MOCKABLE_FUNCTION(, ATRPC_HANDLE, atrpc_create);
MOCKABLE_FUNCTION(, void, atrpc_destroy, ATRPC_HANDLE, handle);
MOCKABLE_FUNCTION(, void, atrpc_dowork, ATRPC_HANDLE, handle);
MOCKABLE_FUNCTION(, int, atrpc_open, ATRPC_HANDLE, handle, ON_ATRPC_OPEN_COMPLETE, on_open_complete, void *, on_open_complete_context);
MOCKABLE_FUNCTION(, int, atrpc_send_raw_data, ATRPC_HANDLE, handle, const unsigned char *, data_buffer, size_t, data_buffer_length, ON_ATRPC_SEND_RAW_DATA_COMPLETE, on_send_raw_data_complete, void *, on_send_raw_data_complete_context);
MOCKABLE_FUNCTION(, int, atrpc_set_raw_data_callback, ATRPC_HANDLE, handle, ON_ATRPC_RAW_DATA_RECEIVED, on_raw_data_received, void *, on_raw_data_received_context);

#ifdef __cplusplus
  }
#endif

#endif /* AT_RPC_H */

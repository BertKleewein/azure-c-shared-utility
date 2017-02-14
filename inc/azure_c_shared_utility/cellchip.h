// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CELLCHIP_H
#define CELLCHIP_H

#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/atrpc.h"   // BKTODO: remove when no longer necessary

#ifdef __cplusplus
extern "C" {
#endif

#define CELLCHIP_RESULT_CODE_VALUES \
    CELLCHIP_OK, \
    CELLCHIP_ERROR, \
    CELLCHIP_CONNECT_ERROR

DEFINE_ENUM(CELLCHIP_RESULT_CODE, CELLCHIP_RESULT_CODE_VALUES);

#define CELLCHIP_CONNECTION_STATE_VALUES \
    CELLCHIP_MODE_UNKNOWN, \
    CELLCHIP_DISCONNECTED, \
    CELLCHIP_COMMAND_MODE, \
    CELLCHIP_DATA_MODE
    
DEFINE_ENUM(CELLCHIP_CONNECTION_STATE, CELLCHIP_CONNECTION_STATE_VALUES);

typedef void * CELLCHIP_HANDLE;

typedef void(*ON_CELLCHIP_OPEN_COMPLETE)(void *context, CELLCHIP_RESULT_CODE cellchip_result);
typedef void(*ON_CELLCHIP_ACTION_COMPLETE)(void *context, CELLCHIP_RESULT_CODE cellchip_result);
typedef void(*ON_CELLCHIP_SEND_COMPLETE)(void* context, CELLCHIP_RESULT_CODE cellchip_result);
typedef void(*ON_CELLCHIP_STATE_CHANGE)(void *context, CELLCHIP_CONNECTION_STATE old_state, CELLCHIP_CONNECTION_STATE new_state);
typedef void(*ON_CELLCHIP_DATA_RECEIVED)(void* context, const uint8_t *data, size_t size);


#include "azure_c_shared_utility/umock_c_prod.h"

MOCKABLE_FUNCTION(, CELLCHIP_HANDLE, cellchip_create);
MOCKABLE_FUNCTION(, void, cellchip_destroy, CELLCHIP_HANDLE, handle);
MOCKABLE_FUNCTION(, int, cellchip_open, CELLCHIP_HANDLE, handle, ON_CELLCHIP_OPEN_COMPLETE, on_open_complete, void *, on_open_complete_context, ON_CELLCHIP_STATE_CHANGE, on_state_change, void *, on_state_change_context, ON_CELLCHIP_DATA_RECEIVED, on_data_received, void *, on_data_received_context);
MOCKABLE_FUNCTION(, int, cellchip_close, CELLCHIP_HANDLE, handle);
MOCKABLE_FUNCTION(, void, cellchip_dowork, CELLCHIP_HANDLE, handle);
// BKTODO: sim808_attention will eventaully go away
MOCKABLE_FUNCTION(, int, cellchip_attention, CELLCHIP_HANDLE, handle, const unsigned char *, command_string, size_t, command_string_length, size_t, timeout_ms, unsigned char *, ta_response_buffer, size_t, ta_response_buffer_size, ON_ATRPC_TA_RESPONSE, on_ta_response, void *, ta_response_context, CUSTOM_TA_RESULT_CODE_PARSER, result_code_parser, void *, result_code_parser_context);
MOCKABLE_FUNCTION(, int, cellchip_attach_to_network, CELLCHIP_HANDLE, handle, ON_CELLCHIP_ACTION_COMPLETE, on_action_complete, void*, on_action_complete_context);
MOCKABLE_FUNCTION(, int, cellchip_tcp_connect, CELLCHIP_HANDLE, handle, ON_CELLCHIP_ACTION_COMPLETE, on_action_complete, void*, on_action_complete_context);
MOCKABLE_FUNCTION(, int, cellchip_tls_connect, CELLCHIP_HANDLE, handle, const char *, host, uint16_t, port, ON_CELLCHIP_ACTION_COMPLETE, on_action_complete, void*, on_action_complete_context);
MOCKABLE_FUNCTION( ,int, cellchip_send, CELLCHIP_HANDLE, handle, const uint8_t*, buffer, size_t, size, ON_CELLCHIP_SEND_COMPLETE, on_send_complete, void*, on_send_complete_context);

// BKTODO: enable keepalive.
// BKTODO: how do we re-connect when a connection is dropped.  I have a note in my onenote for dealing with this. ("Sim808 error recovery")

void *hack_to_return_atrpc_instance(CELLCHIP_HANDLE h);


#ifdef __cplusplus
}
#endif

#endif // CELLCHIP_H


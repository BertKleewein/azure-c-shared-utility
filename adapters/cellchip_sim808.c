// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

#ifdef __cplusplus
  #include <cassert>
  #include <cstdbool>
  #include <cstddef>
  #include <cstdlib>
#else
  #include <assert.h>
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdlib.h>
  #include <stdint.h>
#endif

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/cellchip.h"
#include "azure_c_shared_utility/atrpc.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "msp430.h"

#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

#if (defined DEBUG) && (defined __MSP430__)
// VERBOSE_MODEM_DEBUGGING shows you individual AT commands
// #define VERBOSE_MODEM_DEBUGGING 1
// TIGHT_MODEM_DEBUGGING shows you summaries of AT commands, with 1 character per command
#define TIGHT_MODEM_DEBUGGING 1
// VERBOSE_INCOMING shows you incoming buffers
#define VERBOSE_INCOMING 1
// VERBOSE_OUTGOING shows you outgoing buffers
//#define VERBOSE_OUTGOING 1
#endif  

// BKTODO: this is a singleton.  Could we support multiple connections?
// BKTODO: can we use low power mode (DTR pin)?
// BKTODO: we could set context and use that to make future connections faster



// Most AT commands respond with standard success/failure codes.  0 maps to "OK", 4 maps to "CME ERROR", etc.  
// Some AT commands don't follow this pattern.  For these commands, we have a string TA parsers which map 
// fixed strings to success or failure.  We can also make custom parsers

// For string TA parsers, we have structure to hold the fixed strings we compare to.
typedef struct TAG_TA_PARSER_STRINGS {
    const char *success_message;
    const uint8_t success_message_length;
    const char *failure_message;
    const uint8_t failure_message_length;
} TA_PARSER_STRINGS;

// Macros to make the code easier to write
#define SUCCESS_MESSAGE(x) x, sizeof(x)-1
#define FAILURE_MESSAGE(x) x, sizeof(x)-1

// Array of possible messages for string TA parsers
static const TA_PARSER_STRINGS ta_parser_strings[] =
{
    { 
        SUCCESS_MESSAGE("\r\nSHUT OK\r\n"), 
        FAILURE_MESSAGE("\r\nERROR\r\n")
    }, // TA_PARSER_CIPSHUT
};

// The parser list starts with string parsers (using the ta_parser_strings array), followed by custom parsers (using their own functions)
// For string parsers, the value needs to match the index of the appropriate strings inside ta_parser_strings
// This means that adding a parser means in incrementing all the indexes below it.  Sorry. :(
#define TA_PARSER_CIPSHUT 0
#define TA_PARSER_LAST_STRING_PARSER 0  // last parser to use ta_parser_strings
#define TA_PARSER_IP_ADDRESS 1  
#define TA_PARSER_COUNT 2   // total count of parsers

// We also have a function to verify a response from the chip.  This lets us verify things like the response to "AT+CREG?"
// which checks network connection status.  This command can return TA = 0, which means success, but we want to treat
// it as a failure (and retry) if the network isn't connected yet.  This is the function prototype which verifies these
// special responses.
typedef uint8_t(*_RESPONSE_VERIFIER)(const uint8_t * ta_response, size_t ta_response_size);

// special step index values that can be returned by the verifier functions.  The verifiers can also return hardcoded indices.  
#define STEP_INDEX_MOVE_NEXT 0xfe
#define STEP_INDEX_RETRY 0xfd

// In this file, we define a state machine which represents a sequence of commands that go to the modem.  
// We call this a "sequence".  Each step in teh sequence is defined as follows:
typedef struct SEQUENCE_STEP_TAG {
    const char *command_string;
    const size_t command_string_length;
    uint16_t retry_delay;
    _RESPONSE_VERIFIER response_verifier;
    const uint8_t ta_parser;
} SEQUENCE_STEP;

// After a step executes, we have to decide what to do next.  These are the things we can do after each step:
typedef enum {
    SEQUENCE_ACTION_FAIL,
    SEQUENCE_ACTION_TEST_RETRY,
    SEQUENCE_ACTION_WAIT_FOR_RETRY,
    SEQUENCE_ACTION_MOVE_TO_NEXT,
    SEQUENCE_ACTION_SUBMIT_CURRENT
} SEQUENCE_ACTION;

// To build our sequence lists, we build up layers of simple macros that define each step.
// The macros that start with _ are meant to be used from other macros.
#define _COMMAND(x) x, sizeof(x)-1  // hardcoded string command
#define _CUSTOM_COMMAND(x) NULL, x  // custom command
#define _RETRY_DELAY(x) x   // delay between command failures.
#define _RESPONSE_VERIFIER(x) x // function to verify the return string
#define _TA_PARSER(x) x // parser function to handle custom TA codes
#define _NO_TA_PARSER() _TA_PARSER(TA_PARSER_COUNT)

// Default timeout on all actions
#define DEFAULT_TIMEOUT 1000

// In addition to fixed string steps, we also have custom comands.  These are strings which are built manually
#define CC_SET_APN 1
#define CC_CIP_START 2

// to build our structure, we have a few top-layer macros which define the most common cases.

// AT command with default values
#define CMD(x) \
    { \
        _COMMAND(x), \
        _RETRY_DELAY(0), \
        _RESPONSE_VERIFIER(NULL), \
        _NO_TA_PARSER() \
    }
// AT command with custom retry delay
#define CMD_RETRYDELAY(x,r) \
    { \
        _COMMAND(x), \
        _RETRY_DELAY(r), \
        _RESPONSE_VERIFIER(NULL), \
        _NO_TA_PARSER() \
    }
// AT command with custom retry delay and custom response verifier
#define CMD_RETRYDELAY_VERIFIER(x,r,v) \
    { \
        _COMMAND(x), \
        _RETRY_DELAY(r), \
        _RESPONSE_VERIFIER(v), \
        _NO_TA_PARSER() \
    }
// AT command with custom TA parser
#define CMD_PARSER(x, p) \
    { \
        _COMMAND(x), \
        _RETRY_DELAY(0), \
        _RESPONSE_VERIFIER(NULL), \
        _TA_PARSER(p) \
    }
// custom command with manually generated strings
#define CUSTOM_CMD(x) \
    { \
        _CUSTOM_COMMAND(x), \
        _RETRY_DELAY(0), \
        _RESPONSE_VERIFIER(NULL), \
        _NO_TA_PARSER() \
    }

// First step.
#define STEP_INDEX_START 0

// Prototype for function which evaluates response of AT+CREG? and AT+CGREG? commands
static uint8_t network_registration_verifier(const uint8_t * ta_response, size_t ta_response_size);

// Sequence of commands to attach to the network
static const SEQUENCE_STEP attach_sequence[] =
{
    // Disable all incoming calls
    CMD("+GSMBUSY=1"),
    // Make sure we're connected to the cell network
    CMD_RETRYDELAY_VERIFIER("+CREG?",2000,network_registration_verifier),
    // Disconnect from GPRS network
    CMD_RETRYDELAY("+CGATT=0",5000),
    // Connect to GPRS network
    CMD_RETRYDELAY("+CGATT=1",2000),
    // Make sure we're connected to the GPRS network
    CMD_RETRYDELAY_VERIFIER("+CGREG?",2000,network_registration_verifier),
    // Disable unsolicited status events
    CMD("+CGEREP=0"),
    // Close the previous GPRS connection
    CMD_PARSER("+CIPSHUT", TA_PARSER_CIPSHUT),
    // Use a single IP connection
    CMD("+CIPMUX=0"),
    // Set wireless mode to use GPRS.
    CMD("+CIPCSGP=1"),
    // Set network timing check
    CMD("+CIPDPDP=1,10,3"),
    // set remote delay timers
    CMD("+CIPRDTIMER=2000,3500"),
    // Set data transmit mode to "normal".  Server will respond SEND_OK after sending.  BKTODO: maybe not needed
    CMD("+CIPQSEND=0"),
    // do not set timer when sending data
    CMD("+CIPATS=0"),
    // Set TCP application mode to transparent
    CMD("+CIPMODE=1"),
    // Configure transparent transfer mode
    // Parameters are all default except for the 4th which turns off the +++ escape
    CMD("+CIPCCFG=5,2,1024,0,0,1460,50"),
    // Configure DTR to bring unit out of raw mode
    CMD("&D1"),
    // Get data manually.  unknown.  parameter is undefined
    CMD("+CIPRXGET=0"),
    // Disable TCP keepalive.  BKTODO: we need keeplive
    CMD("+CIPTKA=0"),
    // Don't print the IP header
    CMD("+CIPHEAD=0"),
    // Don't show the transfer protocol in the header
    CMD("+CIPSHOWTP=0"),
    // Don't show remote IP and port in header
    CMD("+CIPSRIP=0"),
    // Set the send prompt to >
    CMD("+CIPSPRT=1"),
    // Set the APN
    // was CMD("+CSTT=\"wholesale\""),
    CUSTOM_CMD(CC_SET_APN),
    // Save the TCP/IP context
    CMD("+CIPSCONT"),
    // Bring up the wireless connection
    CMD_RETRYDELAY("+CIICR",1000),
};

// Sequence of commands to connect to a given host and port
static const SEQUENCE_STEP tcp_connect_sequence[] =
{
    // Get IP address
    CMD_PARSER("+CIFSR",TA_PARSER_IP_ADDRESS),
    // Turn on SSL and set some undocumented options
    CMD("+CIPSSL=1"),
    //CMD("+SSLOPT=0,1"),
    //CMD("+SSLOPT=1,1"),
    // Connect 
    //Was CMD("+CIPSTART=\"TCP\",\"40.118.160.105\",7"),
    CUSTOM_CMD(CC_CIP_START),
    // Verify that we're ready to send 
    CMD("+CIPSEND?"),
};

// Maximum number of retries before a command is considered to be failed.
// BKTODO: make constants into #defines?
static const uint8_t max_retry_count = 10; 

// Size of the buffer we use to hold on to responses.  
#define DEFAULT_RESPONSE_BUFFER_SIZE 128  // response buffer size 

// maximum times to cycle power to the sim808 before failing.  Since this is recursive, too many retries will blow the stack!
#define MAX_POWER_CYCLE_COUNT 3

// Largest number of characters in a custom command
#define MAX_CUSTOM_COMMAND_BUFFER_SIZE 128

typedef struct CELLCHIP_SIM808_INSTANCE_TAG
{
    ATRPC_HANDLE atrpc;
    TICK_COUNTER_HANDLE tickcounter;
    ON_CELLCHIP_OPEN_COMPLETE on_open_complete;
    void * on_open_complete_context;
    ON_CELLCHIP_ACTION_COMPLETE on_action_complete;
    void* on_action_complete_context;
    ON_CELLCHIP_STATE_CHANGE on_state_change;
    void *on_state_change_context;
    ON_CELLCHIP_DATA_RECEIVED on_data_received;
    void *on_data_received_context;
    ON_CELLCHIP_ACTION_COMPLETE on_connect_complete;
    void *on_connect_complete_context;
    ON_CELLCHIP_ACTION_COMPLETE on_attach_complete;
    void* on_attach_complete_context;
    CELLCHIP_CONNECTION_STATE connection_state;
    ON_CELLCHIP_SEND_COMPLETE on_send_complete;
    void* on_send_complete_context;
    
    union 
    {
        // Command and response are never used at the same time, so we use teh same memory for both.  Pretty tricky, eh?
        uint8_t default_response_buffer[DEFAULT_RESPONSE_BUFFER_SIZE];
        char  custom_command_string[MAX_CUSTOM_COMMAND_BUFFER_SIZE];
    };
    uint8_t retry_count;
    tickcounter_ms_t retry_time;
    const SEQUENCE_STEP *current_sequence;
    uint8_t current_sequence_step_count;
    uint8_t current_sequence_step_index;
    uint8_t ta_parser;
    uint8_t ta_parser_success_state_index;
    uint8_t ta_parser_failure_state_index;
    uint8_t power_cycle_count;
    const char *host;
    uint16_t port;
} CELLCHIP_SIM808_INSTANCE;

static void set_cellchip_connection_state(CELLCHIP_SIM808_INSTANCE *cellchip, CELLCHIP_CONNECTION_STATE newState)
{
    CELLCHIP_CONNECTION_STATE oldState = cellchip->connection_state;
    if (oldState != newState)
    {
        cellchip->connection_state = newState;
        if (cellchip->on_state_change != NULL)
        {
            cellchip->on_state_change(cellchip->on_state_change_context, oldState, newState);
        }
    }
}

CELLCHIP_HANDLE cellchip_create()
{
    CELLCHIP_SIM808_INSTANCE *cellchip;

    if (NULL == (cellchip = (CELLCHIP_SIM808_INSTANCE *)calloc(sizeof(CELLCHIP_SIM808_INSTANCE), sizeof(uint8_t))))
    {
        LogError("cellchip_create() unable to allocate memory for internal data structure!");
    }
    else
    {
        if (NULL == (cellchip->atrpc = atrpc_create()))
        {
            LogError("atrpc_create failed");
            free(cellchip);
            cellchip = NULL;
        }
        else if (NULL == (cellchip->tickcounter = tickcounter_create()))
        {
            LogError("tickcounter_create failed");
            atrpc_destroy(cellchip->atrpc);
            free(cellchip);
            cellchip = NULL;
        }
    }
    
    cellchip->on_open_complete = NULL;
    cellchip->on_action_complete = NULL;
    cellchip->on_state_change = NULL;
    cellchip->on_data_received = NULL;
    cellchip->on_connect_complete = NULL;
    cellchip->on_attach_complete = NULL;
    cellchip->connection_state = CELLCHIP_MODE_UNKNOWN;

    return (CELLCHIP_HANDLE)cellchip;
}

// BKTODO: remove
uint8_t g_buffer[128];
size_t g_buffer_index = 0;
void save_data_dump()
{
    printf ("<in %d>\n", g_buffer_index);
    for (int j = 0; j < g_buffer_index; j++)
    {
        _putchar(g_buffer[j]);
    }
    printf ("</in>\n");
}
static void save_data_hack(const uint8_t *buffer, size_t size)
{
    for (size_t i=0; i<size; i++)
    {
        g_buffer[g_buffer_index] = buffer[i];
        g_buffer_index++;
        if (g_buffer_index == sizeof(g_buffer))
        {
#ifdef VERBOSE_INCOMING
            save_data_dump();
#endif
            g_buffer_index = 0;
        }
    }
}


// BKTODO: there's no extrenal interface for getting out of data mode

static void on_atrpc_raw_data_received(void *context, const uint8_t *buffer, size_t size)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)context;
    save_data_hack(buffer, size);

    if (cellchip->on_data_received && (cellchip->connection_state == CELLCHIP_DATA_MODE))
    {
        cellchip->on_data_received(cellchip->on_data_received_context, buffer, size);
    }
}

static void on_atrpc_open_complete(void *handle, TA_RESULT_CODE result_code)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;
    if (NULL != cellchip)
    {
        if ((result_code == ERROR_AUTOBAUD) && (cellchip->power_cycle_count < MAX_POWER_CYCLE_COUNT))
        {
#ifdef VERBOSE_MODEM_DEBUGGING
            printf("cycling power to sim808\n");
#endif
            atrpc_close(cellchip->atrpc);
            msp430_power_cycle_sim808();
            cellchip->power_cycle_count++;
            if (0 != atrpc_open(cellchip->atrpc, on_atrpc_open_complete, cellchip)) // Recursive!
            {
                LogError("atrpc_open failed");
                set_cellchip_connection_state(cellchip, CELLCHIP_DISCONNECTED);
                if (NULL != cellchip->on_open_complete)
                {
                    cellchip->on_open_complete(cellchip->on_open_complete_context, CELLCHIP_CONNECT_ERROR);
                }
            }
        }
        else
        {
            if (result_code == OK_3GPP)
            {
                set_cellchip_connection_state(cellchip, CELLCHIP_COMMAND_MODE);
                if (NULL != cellchip->on_open_complete)
                {
                    cellchip->on_open_complete(cellchip->on_open_complete_context, CELLCHIP_OK);
                }
            }
            else
            {
                set_cellchip_connection_state(cellchip, CELLCHIP_MODE_UNKNOWN);
                if (NULL != cellchip->on_open_complete)
                {
                    cellchip->on_open_complete(cellchip->on_open_complete_context, CELLCHIP_CONNECT_ERROR);
                }
            }
        }
    }
}

int cellchip_open (CELLCHIP_HANDLE handle, ON_CELLCHIP_OPEN_COMPLETE on_open_complete, void *on_open_complete_context, ON_CELLCHIP_STATE_CHANGE on_state_change, void *on_state_change_context, ON_CELLCHIP_DATA_RECEIVED on_data_received, void *on_data_received_context)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;
    int result;
    
#if SAFETY_NET
    if ((cellchip == NULL) ||
        (cellchip->atrpc == NULL))
    {
        LogError("invalid args to cellchip_open");
        result = __FAILURE__;
    }
    else
#endif
    {
        cellchip->on_open_complete = on_open_complete;
        cellchip->on_open_complete_context = on_open_complete_context;
        cellchip->on_state_change = on_state_change;
        cellchip->on_state_change_context = on_state_change_context;
        cellchip->on_data_received = on_data_received;
        cellchip->on_data_received_context = on_data_received_context;
        cellchip->power_cycle_count = 0;

        set_cellchip_connection_state(cellchip, CELLCHIP_DISCONNECTED);
        
        if (0 != atrpc_set_raw_data_callback(cellchip->atrpc, on_atrpc_raw_data_received, cellchip))
        {
            LogError("atrpc_set_raw_data_callback failed");
            result = __FAILURE__;
        }
        else if (0 != atrpc_open(cellchip->atrpc, on_atrpc_open_complete, cellchip))
        {
            LogError("atrpc_open failed");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

static int ta_string_parser(void * context, uint8_t input, TA_RESULT_CODE * result_code)
{
    bool result = false;
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)context;
    const TA_PARSER_STRINGS *strings = &ta_parser_strings[cellchip->ta_parser];

    if (input == strings->success_message[cellchip->ta_parser_success_state_index])
    {
        cellchip->ta_parser_success_state_index++;
        if (cellchip->ta_parser_success_state_index == strings->success_message_length)
        {
            *result_code = OK_3GPP;
            result = true;
        }
    }
    else if (input == strings->success_message[0])
    {
        cellchip->ta_parser_success_state_index = 1;
    }
    else
    {
        cellchip->ta_parser_success_state_index = 0;
    }
    
    if (input == strings->failure_message[cellchip->ta_parser_failure_state_index])
    {
        cellchip->ta_parser_failure_state_index++;
        if (cellchip->ta_parser_failure_state_index == strings->success_message_length)
        {
            *result_code = ERROR_3GPP;
            result = true;
        }
    }
    else if (input == strings->failure_message[0])
    {
        cellchip->ta_parser_failure_state_index = 1;
    }
    else
    {
        cellchip->ta_parser_failure_state_index = 0;
    }

    return result;
}

static int ta_ip_parser(void * context, uint8_t input, TA_RESULT_CODE * result_code)
{
    bool result = false;
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)context;

    switch (cellchip->ta_parser_success_state_index)
    {
        case 0:
        {
            if (input == '\r') 
            {
                cellchip->ta_parser_success_state_index++;
            }
            break;
        }
        case 1:
        {
            if (input == '\n') 
            {
                cellchip->ta_parser_success_state_index++;
            }
            else if (input == '\r')
            {
                // stay in this state
            }
            else
            {
                cellchip->ta_parser_success_state_index = 0;
            }
            break;
        }
        case 2:
        {
            if (input == '\r')
            {
                cellchip->ta_parser_success_state_index++;
            }
            else if (input == '.' || (input >= '0' && input <= '9'))
            {
                // stay right here
            }
            else
            {
                cellchip->ta_parser_success_state_index = 0;
            }
            break;
        }
        case 3:
        {
            if (input == '\n')
            {
                result = true;
                *result_code = OK_3GPP;
            }
            else if (input == '\r')
            {
                cellchip->ta_parser_success_state_index = 1;
            }
            else
            {
                cellchip->ta_parser_success_state_index = 0;
            }
        }
    }

    // Instead of parsing failure, we'll just let it time out to save code space.
 
    return result;
}

// BKTODO: move string functions into sillystring.c
// guarantees null termination
static bool safe_strcat(char *dest, const char *src, size_t destSize)
{
    const char *ps = src;
    char *pd = dest;
    size_t spaceLeft = destSize-1;  // save a character for the null

    // skip over the first part of the dest
    while (*pd && spaceLeft)
    {
        pd++;
        spaceLeft--;
    } 

    // copy src to dest
    while (*ps && spaceLeft)
    {
        *(pd++) = *(ps++);
        spaceLeft--;
    } 

    // terminate it
    *pd = 0;

    // return true if we have space left or if we handled both strings fully
    return (spaceLeft != 0) || (*ps == 0);
}

// BKTODO: APN to options
#define APN_NAME "wholesale"
// BKTODO: make TLS a parameter
#define PROTOCOL "TCP"

static int create_custom_command_string(CELLCHIP_SIM808_INSTANCE *cellchip, const SEQUENCE_STEP *step)
{
    int result;
    char *dest = cellchip->custom_command_string;
    
    if (step->command_string_length == CC_SET_APN)
    {
        // "+CSTT=\"wholesale\""
        dest[0] = 0;
        if (safe_strcat(dest, "+CSTT=\"", COUNTOF(cellchip->custom_command_string)) &&
            safe_strcat(dest, APN_NAME, COUNTOF(cellchip->custom_command_string)) &&
           safe_strcat(dest, "\"", COUNTOF(cellchip->custom_command_string)))
        {
            result = 0;
        }
        else
        {
            LogError("string too short");
            result = __FAILURE__;
        }
    }
    else if (step->command_string_length == CC_CIP_START)
    {
        //"+CIPSTART=\"TCP\",\"40.118.160.XXX\",7"
        dest[0] = 0;
        char portstring[10];
        if ((0 == unsignedIntToString(portstring, COUNTOF(portstring), cellchip->port)) &&
            safe_strcat(dest, "+CIPSTART=\"", COUNTOF(cellchip->custom_command_string)) &&
            safe_strcat(dest, PROTOCOL, COUNTOF(cellchip->custom_command_string)) &&
            safe_strcat(dest, "\",\"", COUNTOF(cellchip->custom_command_string)) &&
            safe_strcat(dest, cellchip->host, COUNTOF(cellchip->custom_command_string)) &&
            safe_strcat(dest, "\",", COUNTOF(cellchip->custom_command_string)) &&
            safe_strcat(dest, portstring, COUNTOF(cellchip->custom_command_string)))
        {
            result = 0;
        }
        else
        {
            LogError("string too short");
            result = __FAILURE__;
        }
    }
    else
    {
        LogError("invalid arg");
        result = __FAILURE__;
    }
    
    return result;
}

static bool is_custom_command_step(const SEQUENCE_STEP *step)
{
    return (step->command_string == NULL);
}

static void on_sequence_at_command_complete(void * context, TA_RESULT_CODE result_code, const uint8_t * ta_response, size_t ta_response_size);
static int enqueue_current_step(CELLCHIP_SIM808_INSTANCE *cellchip)
{
    int result;
    const SEQUENCE_STEP *currentState = &cellchip->current_sequence[cellchip->current_sequence_step_index];

#ifdef TIGHT_MODEM_DEBUGGING
    if (cellchip->current_sequence == attach_sequence)
    {
        printf("a");
    }
    else
    {
        printf ("c");
    }
#endif

    CUSTOM_TA_RESULT_CODE_PARSER ta_parser_function;
    cellchip->ta_parser_success_state_index = 0;
    cellchip->ta_parser_failure_state_index = 0;
    
    if (currentState->ta_parser <= TA_PARSER_LAST_STRING_PARSER)
    {
        ta_parser_function = ta_string_parser;
        cellchip->ta_parser = currentState->ta_parser;
    }
    else if (currentState->ta_parser == TA_PARSER_IP_ADDRESS)
    {
        ta_parser_function = ta_ip_parser;
        cellchip->ta_parser = TA_PARSER_IP_ADDRESS;
    }
    else
    {
        ta_parser_function = NULL;
        cellchip->ta_parser = TA_PARSER_COUNT;
    }

    const uint8_t *commandString = (const uint8_t *)currentState->command_string;
    size_t commandStringLength = currentState->command_string_length;
    if (is_custom_command_step(currentState))
    {
        if (0 == create_custom_command_string(cellchip, currentState))
        {
            commandString = (const uint8_t *)cellchip->custom_command_string;
            commandStringLength = strlen(cellchip->custom_command_string);
        }
        else
        {
            LogError("create_custom_command_string returned failure");
            commandString = NULL;
        }
    }

    cellchip->retry_time = 0;
    if (NULL == commandString)
    {
        LogError("Invalid command string");
        result = __FAILURE__;
    }
    else if (0 == atrpc_attention(cellchip->atrpc, 
            commandString, 
            commandStringLength, 
            DEFAULT_TIMEOUT,
            cellchip->default_response_buffer, 
            sizeof(cellchip->default_response_buffer), 
            on_sequence_at_command_complete, 
            cellchip, 
            ta_parser_function, 
            cellchip))
    {
        result = 0;
    }
    else
    {
        LogError("AT comand %.*s returned failure", commandStringLength, commandString);
        result = __FAILURE__;
    }
    
    return result;
}

static uint8_t network_registration_verifier(const uint8_t * ta_response, size_t ta_response_size)
{
    const uint8_t *p = ta_response;
    for (; p < ta_response + ta_response_size - 2; p++)
    {
        if ((*p == '1' || *p == '0') &&
            (*(p+1) == ',') &&
            ((*(p+2) == '1' || *(p+2) == '5')))
        {
            return STEP_INDEX_MOVE_NEXT;
        }
    }
    return STEP_INDEX_RETRY;
}

static void on_sequence_at_command_complete(void * context, TA_RESULT_CODE result_code, const uint8_t * ta_response, size_t ta_response_size)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)context;
    SEQUENCE_ACTION action;
    const SEQUENCE_STEP *currentState = &cellchip->current_sequence[cellchip->current_sequence_step_index];
    
    // First, look at the result code to determine our next action
    if (result_code != OK_3GPP)
    {
        action = SEQUENCE_ACTION_TEST_RETRY;
#ifdef TIGHT_MODEM_DEBUGGING
        printf("-");
#endif
    }
    else if (currentState->response_verifier != NULL)
    {
        uint8_t nextState = currentState->response_verifier(ta_response, ta_response_size);
        if (nextState == STEP_INDEX_MOVE_NEXT)
        {
            action = SEQUENCE_ACTION_MOVE_TO_NEXT;
        }
        else if (nextState == STEP_INDEX_RETRY)
        {
            action = SEQUENCE_ACTION_TEST_RETRY;
        }
        else
        {
            cellchip->current_sequence_step_index = nextState;
            currentState = &cellchip->current_sequence[cellchip->current_sequence_step_index];
            action = SEQUENCE_ACTION_SUBMIT_CURRENT;
        }
    }
    else
    {
        action = SEQUENCE_ACTION_MOVE_TO_NEXT;
    }

    // If we need to retry, look at the retry count
    if (action == SEQUENCE_ACTION_TEST_RETRY)
    {
        cellchip->retry_count++;
        if (cellchip->retry_count < max_retry_count)
        {
            if (currentState->retry_delay == 0)
            {
                action = SEQUENCE_ACTION_SUBMIT_CURRENT;
            }
            else
            {
#ifdef TIGHT_MODEM_DEBUGGING
                printf(".");
#endif
#ifdef VERBOSE_MODEM_DEBUGGING
                printf("pausing before retry\n");
#endif
                tickcounter_ms_t now;
                if (0 == tickcounter_get_current_ms(cellchip->tickcounter, &now))
                {
                    cellchip->retry_time = now + cellchip->current_sequence[cellchip->current_sequence_step_index].retry_delay;
                    action = SEQUENCE_ACTION_WAIT_FOR_RETRY;
                }
                else
                {
                    LogError("tickcounter_get_current_ms failed");
                    action = SEQUENCE_ACTION_FAIL;
                }
            }
        }
        else
        {
            LogError("max_retry_count reached.  Timing out");
            action = SEQUENCE_ACTION_FAIL;
        }
    }

    // Increment our state if we're moving on
    if (action == SEQUENCE_ACTION_MOVE_TO_NEXT)
    {
        cellchip->current_sequence_step_index++;
        currentState = &cellchip->current_sequence[cellchip->current_sequence_step_index];
        cellchip->retry_count = 0;
        action = SEQUENCE_ACTION_SUBMIT_CURRENT;
    }

    // Submit new state if necessary
    if (action == SEQUENCE_ACTION_SUBMIT_CURRENT)
    {
        if (cellchip->current_sequence_step_index == cellchip->current_sequence_step_count)
        {
            cellchip->on_action_complete(cellchip->on_action_complete_context, CELLCHIP_OK);
        }
        else if (cellchip->current_sequence_step_index < cellchip->current_sequence_step_count)
        {
            if (0 != enqueue_current_step(cellchip))
            {
                LogError("enqueue_current_step failed");
                action = SEQUENCE_ACTION_FAIL;
            }
        }
        else
        {
            LogError("unexpected");
            action = SEQUENCE_ACTION_FAIL;
        }
    }

    // callback with failure if necessary.
    if (action == SEQUENCE_ACTION_FAIL)
    {
        cellchip->on_action_complete(cellchip->on_action_complete_context, CELLCHIP_ERROR);
    }

}

static int start_sequence(CELLCHIP_HANDLE handle, const SEQUENCE_STEP *sequence, uint8_t sequence_step_count, ON_CELLCHIP_ACTION_COMPLETE on_action_complete, void* on_action_complete_context)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;
    int result;

#if SAFETY_NET
    if ((cellchip == NULL) ||
        (cellchip->atrpc == NULL))
    {
        LogError("invalid args to start_sequence");
        result = __FAILURE__;
    }
    else
#endif
    {
        cellchip->on_action_complete = on_action_complete;
        cellchip->on_action_complete_context = on_action_complete_context;
        cellchip->current_sequence = sequence;
        cellchip->current_sequence_step_count = sequence_step_count;
        cellchip->current_sequence_step_index = STEP_INDEX_START;
        cellchip->retry_count = 0;
        if (0 != enqueue_current_step(cellchip))
        {
            LogInfo("start_sequence: atrpc_attention returned failure");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

static void internal_on_attach_complete(CELLCHIP_HANDLE handle, CELLCHIP_RESULT_CODE cellchip_result)

{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;

    set_cellchip_connection_state(cellchip, cellchip_result == CELLCHIP_OK ? CELLCHIP_COMMAND_MODE : CELLCHIP_MODE_UNKNOWN);

    if (cellchip->on_attach_complete)
    {
        cellchip->on_attach_complete(cellchip->on_attach_complete_context, cellchip_result);
    }
}
    
int cellchip_attach_to_network(CELLCHIP_HANDLE handle, ON_CELLCHIP_ACTION_COMPLETE on_attach_complete, void* on_attach_complete_context)
{
    int result;
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;

#if SAFETY_NET
    if (cellchip == NULL)
    {
        LogError("invalid args to cellchip_attach_to_network");
        result = __FAILURE__;
    }
    else
#endif
    {
        cellchip->on_attach_complete = on_attach_complete;
        cellchip->on_attach_complete_context = on_attach_complete_context;
            
        if (0 != start_sequence(cellchip, attach_sequence, COUNTOF(attach_sequence), internal_on_attach_complete, cellchip))
        {
            LogError("start_sequence failed");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }
        
    return result;
}

static void internal_on_tcp_connect_complete(CELLCHIP_HANDLE handle, CELLCHIP_RESULT_CODE cellchip_result)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;

#ifdef TIGHT_MODEM_DEBUGGING
    printf("*\n");
#endif

    set_cellchip_connection_state(cellchip, cellchip_result == CELLCHIP_OK ? CELLCHIP_DATA_MODE : CELLCHIP_MODE_UNKNOWN);

    if (cellchip->on_connect_complete)
    {
        cellchip->on_connect_complete(cellchip->on_connect_complete_context, cellchip_result);
    }
    
}

int cellchip_tls_connect(CELLCHIP_HANDLE handle, const char * host, uint16_t port, ON_CELLCHIP_ACTION_COMPLETE on_connect_complete, void* on_connect_complete_context)
{
    int result;
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;

#if SAFETY_NET
    if (cellchip == NULL)
    {
        LogError("invalid args to cellchip_tls_connect");
        result = __FAILURE__;
    }
    else
#endif
    {
        cellchip->on_connect_complete = on_connect_complete;
        cellchip->on_connect_complete_context = on_connect_complete_context;
        cellchip->host = host;
        cellchip->port = port;
        
        if (0 != start_sequence(handle, tcp_connect_sequence, COUNTOF(tcp_connect_sequence), internal_on_tcp_connect_complete, cellchip))
        {
            LogError("start_sequence failed");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }
    
    return result;
}

// BKTODO: no way to get error here.
void on_atrpc_send_raw_data_complete(void *context)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)context;
    if (cellchip->on_send_complete != NULL)
    {
        cellchip->on_send_complete(cellchip->on_send_complete_context, CELLCHIP_OK);
    }
}

int cellchip_send(CELLCHIP_HANDLE handle, const uint8_t* buffer, size_t size, ON_CELLCHIP_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
{
    int result;
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;

#if SAFETY_NET
    if (cellchip == NULL)
    {
        LogError("invalid args to cellchip_send");
        result = __FAILURE__;
    }
    else
#endif
    {
        cellchip->on_send_complete = on_send_complete;
        cellchip->on_send_complete_context = on_send_complete_context;

#ifdef VERBOSE_OUTGOING
        printf("<out %d>\n",size);
        for (int i = 0; i < size; i++)
        {
            _putchar(buffer[i]);
        }
        printf("</out>\n");
#endif

        if (0 != atrpc_send_raw_data(cellchip->atrpc, buffer, size, on_atrpc_send_raw_data_complete, cellchip))
        {
            // BKTODO: callback with error?
            LogError("atrpc_send_raw_data failed");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}


int cellchip_close (CELLCHIP_HANDLE handle)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;
    int result;
    
#if SAFETY_NET
    if ((cellchip == NULL) ||
        (cellchip->atrpc == NULL))
    {
        LogError("invalid args to cellchip_close");
        result = __FAILURE__;
    }
    else
#endif
    {
        result = atrpc_close(cellchip->atrpc);
    }
    return result;
}

void cellchip_destroy (CELLCHIP_HANDLE handle)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;
    
#if SAFETY_NET
    if ((cellchip == NULL) ||
        (cellchip->atrpc == NULL))
    {
        LogError("invalid args to cellchip_destroy");
    }
    else
#endif
    {
        atrpc_destroy(cellchip->atrpc);
        tickcounter_destroy(cellchip->tickcounter);
        free(cellchip);
    }
}

void cellchip_dowork(CELLCHIP_HANDLE handle)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;

#if SAFETY_NET
    if ((cellchip == NULL) ||
        (cellchip->atrpc == NULL))
    {
        LogError("invalid args to cellchip_dowork");
    }
    else
#endif
    {
        if (cellchip->retry_time > 0)
        {
            tickcounter_ms_t now;
            if (0 == tickcounter_get_current_ms(cellchip->tickcounter, &now))
            {
                if (now >= cellchip->retry_time)
                {
                    if (0 != enqueue_current_step(cellchip))
                    {
                        // If enqueue_current_step fails, it will callback to the user.
                        // BKTODO: make sure there's an SRS for this failure calling the callback.
                        LogError("enqueue_current_step failed");
                    }
                }
            }
            else
            {
                LogError("tickcounter_get_current_ms failed");
                // BKTODO: do we fail this out so we don't keep retrying?  I think so.
            }
        }
       
        atrpc_dowork(cellchip->atrpc);
    }
}

void *hack_to_return_atrpc_instance(CELLCHIP_HANDLE handle)
{
    CELLCHIP_SIM808_INSTANCE *cellchip = (CELLCHIP_SIM808_INSTANCE *)handle;
    return cellchip->atrpc;
}
 

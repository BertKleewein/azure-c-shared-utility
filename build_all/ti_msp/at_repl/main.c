// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <msp430.h>
#include <driverlib.h>

#include "azure_c_shared_utility/cellchip.h"
#include "azure_c_shared_utility/platform.h"
#include "../adapters/msp430.h"

#define INPUT_BUFFER_SIZE 128
#define OUTPUT_BUFFER_SIZE 128

/*
 * (ULP 5.3) Detected printf() operation(s).
 *
 * Recommend moving them to RAM during run time or
 * not using as these are processing/power intensive
 */
//TODO: Confirm decision or address suppression diagnostic message
#pragma diag_suppress 1532

/*
 * controlling expression is constant
 */
//TODO: Confirm decision or address suppression diagnostic message
//#pragma diag_suppress 238

/*
 * SIM800 Series_AT Command Manual_V1.09 - Section 1.4.4 - pg. 24
 *
 * The Command line buffer can accept a maximum of 556 characters
 * (counted from the first command without “AT” or “at” prefix).
 * If the characters entered exceeded this number then none of the
 * Command will executed and TA will return "ERROR".
 *
 * strlen("AT") + 556 + strlen("\r");
 */
//#define MAX_AT_COMMAND_SIZE 559

typedef struct RxContext {
    CELLCHIP_HANDLE cellchip;
    bool atrpc_open;
    bool atrpc_error;
    bool awaiting_response;
    unsigned char response_buffer[OUTPUT_BUFFER_SIZE];
    bool show_data_prompt;
    size_t system_check_machine_state;
    size_t ta_result_code_machine_state;
} RxContext;

void
atRepl (
    void * const context_
);

void
onTaResponse (
    void * context_,
    TA_RESULT_CODE result_code_,
    const unsigned char * response_,
    size_t response_size_
);

int
parserForCipshut (
    void * context_,
    unsigned char input_,
    TA_RESULT_CODE * ta_result_code_
);

static 
void 
on_connect_complete(
    void *context, 
    CELLCHIP_RESULT_CODE result
) {
    printf("on_connect_complete returned %d\n",result);
    RxContext * sim808 = (RxContext *)context;
    switch(result)
    {
        case CELLCHIP_OK:
            sim808->atrpc_open = true;
            sim808->atrpc_error = false;
            sim808->awaiting_response = false;
            break;
        default:
            sim808->atrpc_open = false;
            sim808->atrpc_error = true;
            break;
    }

}

static 
void 
on_attach_complete(
    void *context, 
    CELLCHIP_RESULT_CODE result
) {
    RxContext * sim808 = (RxContext *)context;
    switch(result)
    {
        case CELLCHIP_OK:
            if (0 != cellchip_tls_connect(sim808->cellchip, "40.118.160.105", 7, on_connect_complete, sim808))
            {
                LogError("");
                sim808->atrpc_open = false;
                sim808->atrpc_error = true;
            }
            else
            {
                sim808->atrpc_open = true;
                sim808->atrpc_error = false;
                sim808->awaiting_response = true;
            }
            break;
        default:
            sim808->atrpc_open = false;
            sim808->atrpc_error = true;
            break;
    }
}

static
void
on_cellchip_open_complete (
    void * context_,
    CELLCHIP_RESULT_CODE open_result_
) {
    RxContext * sim808 = (RxContext *)context_;

    if ( !context_ ) {
        (void)printf("AT RPC called callback without providing a context\n");
    } else {
        switch (open_result_) {
          case CELLCHIP_OK:
#define ATTACH_TO_NETWORK 1
#if ATTACH_TO_NETWORK
            if ( 0!= cellchip_attach_to_network(sim808->cellchip, on_attach_complete, sim808))
            {
                LogError("cellchip_attach_to_network failed");
            }
#else
            sim808->atrpc_open = true;
            sim808->atrpc_error = false;
            sim808->awaiting_response = false;
#endif
            break;
          default:
            sim808->atrpc_open = false;
            sim808->atrpc_error = true;
            printf("AT RPC failed to open with error: %d!\n", open_result_);
            break;
        }
    }
}


int
main (
    int argc,
    char * argv[]
) {
    RxContext *sim808 = malloc(sizeof(RxContext));
    if (NULL == sim808)
    {
        printf("Allocation error\n");
        return -1;
    }

    sim808->cellchip = NULL;
    sim808->atrpc_open = false;
    sim808->atrpc_error = false;
    sim808->awaiting_response = false;
    sim808->show_data_prompt = false;
    sim808->system_check_machine_state = 0;
    sim808->ta_result_code_machine_state = 0;

    static char answer = 'y';

    for(; tolower(answer) == 'y' ;) {
        // Initialize MSP430FR5969
        if ( 0 != platform_init() ) {
            (void)printf("Failed to initialize the platform!\n");
        // Initialize SIMCOM_SIM808
        } else if ( NULL == (sim808->cellchip = cellchip_create()) ) {
            (void)printf("Failed to create AT RPC layer!\n");
        } else {
            for(; tolower(answer) == 'y' ;) {
                sim808->atrpc_open = false;
                sim808->atrpc_error = false;
                if ( cellchip_open(sim808->cellchip, on_cellchip_open_complete, sim808, NULL, NULL, NULL, NULL) ) {
                    (void)printf("Failed to open the AT RPC layer!\n");
                } else {
                    // (R)ead (E)val (P)rint (L)oop for AT commands
                    (void)atRepl(sim808);
                    if ( cellchip_close(sim808->cellchip) ) {
                        (void)printf("Failed to close the AT RPC layer!\n");
                    } else {
                        (void)printf("The connection to the Sim808 has been closed.\n");
                    }
                }

                // Prompt user to reopen
                (void)fflush(stdin);
                (void)printf("Would you like to reopen (y/N)? ");
                answer = getc(stdin);
            }
            (void)cellchip_destroy(sim808->cellchip);
            sim808->system_check_machine_state = 0;
            (void)printf("Resources have been released.\n");
        }
        platform_deinit();

        // Prompt user for restart
        (void)fflush(stdin);
        (void)printf("Would you like to restart (y/N)? ");
        answer = getc(stdin);
    }
    (void)printf("OK to halt debugger.");

    return 0;
}


/******************************************************************************
 * (R)ead (E)val (P)rint (L)oop for AT commands
 *
 * This REPL is used to manually test AT commands
 *
 ******************************************************************************/
void
atRepl (
    void * const context_
) {
    RxContext * sim808 = (RxContext *)context_;
    if ( !context_ ) { return; }

    static char buffer[INPUT_BUFFER_SIZE];
    static char * upper_case = buffer;
    static CUSTOM_TA_RESULT_CODE_PARSER parser = NULL;

    (void)printf("|>>>> Begin REPL <<<<|\n");
    (void)printf("MCLK (MCU) Hz: %d\n", (unsigned int)(CS_getMCLK()));
    (void)printf("SMCLK (UART) Hz: %d\n", (unsigned int)(CS_getSMCLK()));
    (void)printf("ACLK/%d (Timer A3) Hz: %d\n", (1 << 4), (unsigned int)(CS_getACLK() >> 4));
    sim808->awaiting_response = true;

    for(;!sim808->atrpc_error;) {
        if ( !sim808->awaiting_response ) {
            int send = true;
            // Prompt user for AT command
            if ( sim808->show_data_prompt ) {
                (void)printf("data> ");
            } else {
                (void)printf("Please enter an AT command (or \"quit\"): ");
            }

            (void)fflush(stdin);\
            (void)scanf("%s", buffer);

            if ( sim808->show_data_prompt ) {
                sim808->show_data_prompt = false;
            } else {
              #pragma diag_push
              /*
               * (ULP 2.1) Detected SW delay loop using empty loop.
               *
               * Recommend using a timer module instead
               */
              //TODO: Confirm decision or address suppression diagnostic message
              #pragma diag_suppress 1527
                for(upper_case = buffer ; *upper_case ; *upper_case++ = toupper(*upper_case));  // Uppercase user input
              #pragma diag_pop
                // Check for special condition(s)
                if ( !strcmp("+++", buffer) ) { msp430_exit_sim808_data_mode(); send=false;}
                if ( !strcmp("QUIT", buffer) ) { break; }
                if ( !strcmp("+CIPSHUT", &buffer[2]) ) {
                    parser = parserForCipshut;
                } else {
                    parser = NULL;
                }
            }

            // Send AT command
            if (send)
            {
                (void)printf("Sending \"AT%s\\r\" to SIM808...\n", &buffer[2]);
                (void)strcat(buffer, "\r");
                ATRPC_HANDLE atrpc = (ATRPC_HANDLE)hack_to_return_atrpc_instance(sim808->cellchip);
                if ( atrpc_attention(atrpc, (const unsigned char *)&buffer[2], (strlen(buffer) - 3), 10000, sim808->response_buffer, OUTPUT_BUFFER_SIZE, onTaResponse, sim808, parser, sim808) ) {
                    (void)printf("Failed to send buffer to the XIO layer\n");
                } else {
                    sim808->awaiting_response = true;
                }
            }
        }
        cellchip_dowork(sim808->cellchip);
    }
}


void
onTaResponse (
    void * context_,
    TA_RESULT_CODE result_code_,
    const unsigned char * response_,
    size_t response_size_
) {
    RxContext * sim808 = (RxContext *)context_;
    char * response = (char *)response_;

    if ( !context_ ) { return; }

    if ( !response_ ) {
        printf("Result Code: %d\n", result_code_);
    } else {
        // NULL terminate string
        if ( response_size_ < OUTPUT_BUFFER_SIZE ) {
            response[response_size_] = '\0';
        } else {
            response[OUTPUT_BUFFER_SIZE - 1] = '\0';
        }

        printf("Response:\n%s\nResult Code: %d\n", response_, result_code_);
    }
    sim808->awaiting_response = false;
}


int
parserForCipshut (
    void * context_,
    unsigned char input_,
    TA_RESULT_CODE * ta_result_code_
) {
    RxContext * sim808 = (RxContext *)context_;
    bool response_complete = false;
    *ta_result_code_ = ERROR_ATRPC;

    switch (sim808->ta_result_code_machine_state) {
      case 0:
        if ('\r' == input_) {
            sim808->ta_result_code_machine_state = 1;
        }
        break;
      case 1:
        if ('S' == input_) {
            sim808->ta_result_code_machine_state = 2;
        } else if ('\r' == input_) {
            /* defer */
        } else if ('\n' == input_) {
            sim808->ta_result_code_machine_state = 3;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 2:
        if ('H' == input_) {
            sim808->ta_result_code_machine_state = 4;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 3:
        if ('S' == input_) {
            sim808->ta_result_code_machine_state = 2;
        } else if ('\r' == input_) {
            sim808->ta_result_code_machine_state = 1;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 4:
        if ('U' == input_) {
            sim808->ta_result_code_machine_state = 5;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 5:
        if ('T' == input_) {
            sim808->ta_result_code_machine_state = 6;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 6:
        if (' ' == input_) {
            sim808->ta_result_code_machine_state = 7;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 7:
        if ('O' == input_) {
            sim808->ta_result_code_machine_state = 8;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 8:
        if ('K' == input_) {
            sim808->ta_result_code_machine_state = 9;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 9:
        if ('\r' == input_) {
            sim808->ta_result_code_machine_state = 10;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      case 10:
        if ('\n' == input_) {
            response_complete = true;
            *ta_result_code_ = OK_3GPP;
            sim808->ta_result_code_machine_state = 0;
        } else if ('\r' == input_) {
            sim808->ta_result_code_machine_state = 1;
        } else {
            sim808->ta_result_code_machine_state = 0;
        }
        break;
      default: __never_executed();
    }

    return response_complete;
}



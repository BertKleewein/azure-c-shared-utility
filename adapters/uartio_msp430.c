// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
  #include <cstdbool>
  #include <cstddef>
  #include <cstdint>
#else
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdint.h>
#endif

#include "driverlib.h"

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/uartio.h"
#include "azure_c_shared_utility/dmapingpong.h"

#ifdef _MSC_VER
#pragma warning(disable:4068)
#endif

#define COUNTOF(x) (sizeof(x) / sizeof(x[0]))

typedef struct UartIoState {
    PingPongBuffer uart_rx_buffer;
    PingPongBuffer uart_rxstatus_buffer;
    UARTIO_CONFIG config;
    ON_BYTES_RECEIVED on_bytes_received;
    void * on_bytes_received_context;
    ON_IO_ERROR on_io_error;
    void * on_io_error_context;
    bool open;
} UartIoState;

void *
uartio_cloneoption(
    const char * option_name_,
    const void * option_value_
);

int
uartio_close(
    CONCRETE_IO_HANDLE uartio_,
    ON_IO_CLOSE_COMPLETE on_io_close_complete_,
    void * callback_context_
);

CONCRETE_IO_HANDLE
uartio_create(
    void * io_create_parameters_
);

void
uartio_destroy(
    CONCRETE_IO_HANDLE uartio_
);

void
uartio_destroyoption(
    const char * option_name_,
    const void * option_value_
);

void
uartio_dowork(
    CONCRETE_IO_HANDLE uartio_
);

int
uartio_open(
    CONCRETE_IO_HANDLE uartio_,
    ON_IO_OPEN_COMPLETE on_io_open_complete_,
    void * on_io_open_complete_context_,
    ON_BYTES_RECEIVED on_bytes_received_,
    void * on_bytes_received_context_,
    ON_IO_ERROR on_io_error_,
    void * on_io_error_context_
);

OPTIONHANDLER_HANDLE
uartio_retrieveoptions(
    CONCRETE_IO_HANDLE uartio_
);

int
uartio_send(
    CONCRETE_IO_HANDLE uartio_,
    const void * const buffer_,
    size_t buffer_size_,
    ON_SEND_COMPLETE on_send_complete_,
    void * callback_context_
);

int
uartio_setoption(
    CONCRETE_IO_HANDLE uartio_,
    const char * const option_name_,
    const void * const option_value_
);

static UartIoState _uartio, * _singleton = NULL;  // Allow state to be stored in the BSS memory

#pragma diag_push
/*
 * (ULP 7.1) Detected use of global variable "_uartio_interface_description"
 * within one function "uartio_get_interface_description".
 *
 * Recommend placing variable in the function locally
 */
#pragma diag_suppress 1534
static const IO_INTERFACE_DESCRIPTION _uartio_interface_description = {
    .concrete_io_close = uartio_close,
    .concrete_io_create = uartio_create,
    .concrete_io_destroy = uartio_destroy,
    .concrete_io_dowork = uartio_dowork,
    .concrete_io_open = uartio_open,
    .concrete_io_retrieveoptions = uartio_retrieveoptions,
    .concrete_io_send = uartio_send,
    .concrete_io_setoption = uartio_setoption
};
#pragma diag_pop

#define DMA_TRIGGERSOURCE_UART0_RX DMA_TRIGGERSOURCE_16 // From MSP430FR5969 datasheet.  Not in any headers
#define DMA_CHANNEL_UART0_RX DMA_CHANNEL_0
#define DMA_CHANNEL_UART0_RXSTATUS DMA_CHANNEL_1
#define UART_REGISTER_RX EUSCI_A1_BASE + OFS_UCAxRXBUF
#define UART_REGISTER_RXSTATUS  EUSCI_A1_BASE + OFS_UCAxSTATW

/******************************************************************************
 * Calculate the secondary modulation register value
 *
 * NOTE: Table 24-4 MSP430FR5969 User's Guide 24.3.10
 ******************************************************************************/
#pragma diag_push
/*
 * (ULP 5.2) Detected floating point operation(s).
 *
 * Recommend moving them to RAM during run time or not using as these are processing/power intensive
 */
#pragma diag_suppress 1531
static inline
uint8_t
secondModulationRegisterValueFromFractionalPortion (
    uint16_t fractional_portion
) {
    uint8_t mask_UCBRSx = 0;

    static const uint16_t lookup[][2] = {
        {9288, 0xFE},  // 11111110
        {9170, 0xFD},  // 11111101
        {9004, 0xFB},  // 11111011
        {8751, 0xF7},  // 11110111
        {8572, 0xEF},  // 11101111
        {8464, 0xDF},  // 11011111
        {8333, 0xBF},  // 10111111
        {8004, 0xEE},  // 11101110
        {7861, 0xED},  // 11101101
        {7503, 0xDD},  // 11011101
        {7147, 0xBB},  // 10111011
        {7001, 0xB7},  // 10110111
        {6667, 0xD6},  // 11010110
        {6432, 0xB6},  // 10110110
        {6254, 0xB5},  // 10110101
        {6003, 0xAD},  // 10101101
        {5715, 0x6B},  // 01101011
        {5002, 0xAA},  // 10101010
        {4378, 0x55},  // 01010101
        {4286, 0x53},  // 01010011
        {4003, 0x92},  // 10010010
        {3753, 0x52},  // 01010010
        {3575, 0x4A},  // 01001010
        {3335, 0x49},  // 01001001
        {3000, 0x25},  // 00100101
        {2503, 0x44},  // 01000100
        {2224, 0x22},  // 00100010
        {2147, 0x21},  // 00100001
        {1670, 0x11},  // 00010001
        {1430, 0x20},  // 00100000
        {1252, 0x10},  // 00010000
        {1001, 0x08},  // 00001000
        {835, 0x04},  // 00000100
        {715, 0x02},  // 00000010
        {529, 0x01},  // 00000001
        {0, 0x00},  // 00000000
    };
    
    for (uint16_t i = 0; i < COUNTOF(lookup); i++)
    {
        if (fractional_portion > lookup[i][0])
        {
            mask_UCBRSx = lookup[i][0];
            break;
        }
    }

    return mask_UCBRSx;
}
#pragma diag_pop


/******************************************************************************
 * Initialize EUSCI_A parameters required to communicate with the SIM808
 *
 * NOTE: MSP430FR5969 User's Guide 24.3.10
 ******************************************************************************/
static inline
void
initializeEusciAParametersForSMClkAtBaudRate(
    EUSCI_A_UART_initParam * const eusci_a_parameters_,
    uint32_t baud_rate_
) {
    float factor_N = 0.0f;
    float factor_N_oversampled = 0.0f;
    uint16_t mask_UCBRx;
    uint8_t mask_UCBRFx;
    uint8_t mask_UCBRSx;
    uint8_t mask_UCOS16;

    // Algorithm from User's Guide (Section 24.3.10)
#pragma diag_push
    /*
     * (ULP 5.1) Detected divide operation(s).
     *
     * Recommend moving them to RAM during run time or not using as these are processing/power intensive
     */
#pragma diag_suppress 1530
    /*
     * (ULP 5.2) Detected floating point operation(s).
     *
     * Recommend moving them to RAM during run time or not using as these are processing/power intensive
     */
#pragma diag_suppress 1531
    factor_N = (CS_getSMCLK() / (float)baud_rate_);

    if (factor_N >= 16) {
        mask_UCOS16 = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
        factor_N_oversampled = factor_N / 16;
        mask_UCBRx = (uint16_t)factor_N_oversampled;
        mask_UCBRFx = (uint8_t)((factor_N_oversampled - mask_UCBRx) * 16);
    } else {
        mask_UCOS16 = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;
        mask_UCBRx = (uint16_t)factor_N;
        mask_UCBRFx = 0x00;
    }
    mask_UCBRSx = secondModulationRegisterValueFromFractionalPortion((uint16_t)((factor_N - (uint16_t)factor_N)*10000));
#pragma diag_pop

    eusci_a_parameters_->selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    eusci_a_parameters_->clockPrescalar = mask_UCBRx;
    eusci_a_parameters_->firstModReg = mask_UCBRFx;
    eusci_a_parameters_->secondModReg = mask_UCBRSx;
    eusci_a_parameters_->parity = EUSCI_A_UART_NO_PARITY;
    eusci_a_parameters_->msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    eusci_a_parameters_->numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    eusci_a_parameters_->uartMode = EUSCI_A_UART_MODE;
    eusci_a_parameters_->overSampling = mask_UCOS16;
}


static
void
internal_uarito_close_callback_required_when_closed_from_uartio_destroy(
    void * context
) {
    (void)context;
}


void *
uartio_cloneoption(
    const char * option_name_,
    const void * option_value_
) {
    (void)option_name_, option_value_;
    return NULL;
}


int
uartio_close(
    CONCRETE_IO_HANDLE uartio_,
    ON_IO_CLOSE_COMPLETE on_io_close_complete_,
    void * callback_context_
) {
    int result;

    if (NULL == uartio_) {
        LogError("invalid arg to uartio_close");
        result = __FAILURE__;
    } else if (_singleton != uartio_) {
        LogError("invalid arg to uartio_close");
        result = __FAILURE__;
    } else if (NULL == on_io_close_complete_) {
        LogError("invalid arg to uartio_close");
        result = __FAILURE__;
    } else if (!_uartio.open) {
        LogError("uart not open");
        result = __FAILURE__;
    } else {
        _uartio.open = false;
        pingpong_disable(&_uartio.uart_rx_buffer);
        pingpong_disable(&_uartio.uart_rxstatus_buffer);
        on_io_close_complete_(callback_context_);
        result = 0;
    }

    return result;
}


CONCRETE_IO_HANDLE
uartio_create(
    void * io_create_parameters_
) {
    UARTIO_CONFIG * uartio_config = io_create_parameters_;
    UartIoState * result;  // Errors encountered during `uartio_create()` should NOT affect static singleton

    if (NULL == io_create_parameters_) {
        LogError("invalid arg to uartio_create");
        result = NULL;
    } else if (0 == uartio_config->baud_rate) {
        LogError("invalid arg to uartio_create");
        result = NULL;
    } else if (0 == uartio_config->ring_buffer_size) {
        LogError("invalid arg to uartio_create");
        result = NULL;
    } else if (NULL != _singleton) {
        LogError("invalid arg to uartio_create");
        result = NULL;
    } else if (0 != pingpong_alloc(&_uartio.uart_rx_buffer)) {
        LogError("pingpong_alloc failed");
        result = NULL;
    } else if (0 != pingpong_alloc(&_uartio.uart_rxstatus_buffer)) {
        pingpong_free(&_uartio.uart_rx_buffer);
        LogError("pingpong_alloc failed");
        result = NULL;
    } else {
        _uartio.config.baud_rate = uartio_config->baud_rate;
        _uartio.config.ring_buffer_size = uartio_config->ring_buffer_size;  // BKTODO:remove
        _singleton = &_uartio;
        result = _singleton;
    }

    return (CONCRETE_IO_HANDLE)result;
}


void
uartio_destroy(
    CONCRETE_IO_HANDLE uartio_
) {
    if (NULL == uartio_) {
        LogError("NULL handle passed to uartio_destroy!");
    } else if (_singleton != uartio_) {
        LogError("Invalid handle passed to uartio_destroy!");
    } else {
        // Best effort close, cannot check error conditions
        (void)uartio_close(uartio_, internal_uarito_close_callback_required_when_closed_from_uartio_destroy, NULL);
        pingpong_free(&_uartio.uart_rx_buffer);
        pingpong_free(&_uartio.uart_rxstatus_buffer);
        
        _singleton = NULL;
    }
}


void
uartio_destroyoption(
    const char * option_name_,
    const void * option_value_
) {
    (void)option_name_, option_value_;
}

// BKTODO: get rid of _uartio, cast instead.
// BKTODO: remove friggin underscored!
void
uartio_dowork(
    CONCRETE_IO_HANDLE uartio_
) {
    if (NULL == uartio_) {
        LogError("NULL handle passed to uartio_dowork!");
    } else if (_singleton != uartio_) {
        LogError("Invalid handle passed to uartio_dowork!");
    } else if (!_uartio.open) {
        LogError("Closed handle passed to uartio_dowork!");
    } else {
        if (pingpong_check_for_data(&_uartio.uart_rx_buffer))
        {
            uint8_t *rxBuffer, *rxStatusBuffer;
            size_t rxSize, rxStatusSize;

            // because we can't atomically disable both DMA channels, 
            // there's a change that we'll get slightly out-of-sync if
            // an interrupt happens between the disable calls.
            // Since we have the status buffer just for error conditions,
            // we don't care as long as we see it _sometime_.
            // This is because a single error would abort the entire 
            // transaction anyway.

            // BKTODO; why is the rxstatus buffer always empty?
            pingpong_disable(&_uartio.uart_rxstatus_buffer);
            pingpong_disable(&_uartio.uart_rx_buffer);

            pingpong_flipflop(&_uartio.uart_rx_buffer, &rxBuffer, &rxSize);
            pingpong_flipflop(&_uartio.uart_rxstatus_buffer, &rxStatusBuffer, &rxStatusSize);

            pingpong_enable(&_uartio.uart_rx_buffer);
            pingpong_enable(&_uartio.uart_rxstatus_buffer);

            uint8_t *p = rxStatusBuffer;
            bool error = false;
            for (size_t i = rxStatusSize; i != 0; i--)
            {
                if (*p & (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR))
                {
                    error = true;
                    break;
                }
            }

            if (error)
            {
                _uartio.on_io_error(_uartio.on_io_error_context);
            }
            else
            {
                _uartio.on_bytes_received(_uartio.on_bytes_received_context, rxBuffer, rxSize);
            }
        }
    }
}


const IO_INTERFACE_DESCRIPTION *
uartio_get_interface_description(
    void
) {
    return &_uartio_interface_description;
}


int
uartio_open(
    CONCRETE_IO_HANDLE uartio_,
    ON_IO_OPEN_COMPLETE on_io_open_complete_,
    void * on_io_open_complete_context_,
    ON_BYTES_RECEIVED on_bytes_received_,
    void * on_bytes_received_context_,
    ON_IO_ERROR on_io_error_,
    void * on_io_error_context_
) {
    int result;

    if (NULL == uartio_) {
        LogError("invalid arg to uartio_open");
        result = __FAILURE__;
    } else if (_singleton != uartio_) {
        LogError("invalid arg to uartio_open");
        result = __FAILURE__;
    } else if (NULL == on_io_open_complete_) {
        LogError("invalid arg to uartio_open");
        result = __FAILURE__;
    } else if (NULL == on_bytes_received_) {
        LogError("invalid arg to uartio_open");
        result = __FAILURE__;
    } else if (NULL == on_io_error_) {
        LogError("invalid arg to uartio_open");
        result = __FAILURE__;
    } else if (_uartio.open) {
        LogError("uart already open");
        result = __FAILURE__;
    } else {
        // Ensure the SMCLK is available to the UART module
        CS_enableClockRequest(CS_SMCLK);

        // Initialize UART responsible for communication with SIM808
        EUSCI_A_UART_initParam eusci_a_parameters = { 0 };
        initializeEusciAParametersForSMClkAtBaudRate(&eusci_a_parameters, _uartio.config.baud_rate);
        if (!EUSCI_A_UART_init(EUSCI_A1_BASE, &eusci_a_parameters)) {
            result = __FAILURE__;
        } else {
            _uartio.open = true;
            EUSCI_A_UART_enable(EUSCI_A1_BASE);
            EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
            _uartio.on_bytes_received = on_bytes_received_;
            _uartio.on_bytes_received_context = on_bytes_received_context_;
            _uartio.on_io_error = on_io_error_;
            _uartio.on_io_error_context = on_io_error_context_;
            
            pingpong_attach_to_register(&_uartio.uart_rx_buffer, DMA_CHANNEL_UART0_RX, DMA_TRIGGERSOURCE_UART0_RX, UART_REGISTER_RX );
            pingpong_attach_to_register(&_uartio.uart_rxstatus_buffer, DMA_CHANNEL_UART0_RXSTATUS, DMA_TRIGGERSOURCE_UART0_RX, UART_REGISTER_RXSTATUS);
            pingpong_enable(&_uartio.uart_rx_buffer);
            pingpong_enable(&_uartio.uart_rxstatus_buffer);
            result = 0;
        }
    }

    if (NULL != on_io_open_complete_) { on_io_open_complete_(on_io_open_complete_context_, ((0 == result) ? IO_OPEN_OK : IO_OPEN_ERROR)); }
    return result;
}


OPTIONHANDLER_HANDLE
uartio_retrieveoptions(
    CONCRETE_IO_HANDLE uartio_
) {
    OPTIONHANDLER_HANDLE options;

    if (NULL == uartio_) {
        options = NULL;
    } else if (_singleton != uartio_) {
        options = NULL;
    } else {
        options = OptionHandler_Create(uartio_cloneoption, uartio_destroyoption, uartio_setoption);
    }

    return options;
}


int
uartio_send(
    CONCRETE_IO_HANDLE uartio_,
    const void * buffer_,
    size_t buffer_size_,
    ON_SEND_COMPLETE on_send_complete_,
    void * callback_context_
) {
    int result;

    if (NULL == uartio_) {
        LogError("invalid arg to uartio_send");
        result = __FAILURE__;
    } else if (_singleton != uartio_) {
        LogError("invalid arg to uartio_send");
        result = __FAILURE__;
    } else if (NULL == buffer_) {
        LogError("invalid arg to uartio_send");
        result = __FAILURE__;
    } else if (0 == buffer_size_) {
        LogError("invalid arg to uartio_send");
        result = __FAILURE__;
    } else if (NULL == on_send_complete_) {
        LogError("invalid arg to uartio_send");
        result = __FAILURE__;
    } else if (!_uartio.open) {
        LogError("uart not open in uartio_send");
        result = __FAILURE__;
    } else {
        size_t i = 0;
        uint8_t * buffer = (uint8_t *)buffer_;
        for (; i < buffer_size_; ++i) {
            EUSCI_A_UART_transmitData(EUSCI_A1_BASE, buffer[i]);
        }
        result = 0;
    }

    if (NULL != on_send_complete_) { on_send_complete_(callback_context_, ((0 == result) ? IO_SEND_OK : IO_SEND_ERROR)); }
    return result;
}


int
uartio_setoption(
    CONCRETE_IO_HANDLE uartio_,
    const char * const option_name_,
    const void * const option_value_
) {
    (void)uartio_, option_name_, option_value_;
    return __FAILURE__;
}

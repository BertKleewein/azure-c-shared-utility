// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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

#ifdef _MSC_VER
#pragma warning(disable:4068)
#endif

typedef struct UartIoState {
    UARTIO_CONFIG config;
    uint8_t * eusci_a1_cache_buffer;
    uint8_t * eusci_a1_ring_buffer;
    bool eusci_a1_ring_buffer_full;
    size_t eusci_a1_ring_buffer_head;
    bool eusci_a1_ring_buffer_overflow;
    size_t eusci_a1_ring_buffer_tail;
    uint8_t eusci_a1_rx_error;
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


/******************************************************************************
 * Interrupt to signal SIM808 UART RX data is available
 ******************************************************************************/
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch (__even_in_range(UCA1IV, 8))
    {
    case 0x00: break;
    case 0x02:
#pragma diag_push
        /*
         * (ULP 10.1) ISR USCI_A1_ISR calls function EUSCI_A_UART_queryStatusFlags.
         *
         * Recommend moving function call away from ISR, or inlining the function, or using pragmas
         */
#pragma diag_suppress 1538
        _uartio.eusci_a1_rx_error |= EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, (EUSCI_A_UART_FRAMING_ERROR | EUSCI_A_UART_OVERRUN_ERROR | EUSCI_A_UART_PARITY_ERROR));
        _uartio.eusci_a1_ring_buffer[_uartio.eusci_a1_ring_buffer_head] = EUSCI_A_UART_receiveData(EUSCI_A1_BASE);
#pragma diag_pop
        _uartio.eusci_a1_ring_buffer_overflow = _uartio.eusci_a1_ring_buffer_full;
        _uartio.eusci_a1_ring_buffer_head = ((_uartio.config.ring_buffer_size - 1) & (_uartio.eusci_a1_ring_buffer_head + 1));
        _uartio.eusci_a1_ring_buffer_full = ((_uartio.eusci_a1_ring_buffer_head == _uartio.eusci_a1_ring_buffer_tail) || _uartio.eusci_a1_ring_buffer_overflow);
        break;
    case 0x04: break;
    case 0x06: break;
    case 0x08: break;
    default: __never_executed();
    }
}


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
	float fractional_portion_
) {
	uint8_t mask_UCBRSx;

	if ( fractional_portion_ >= 0.9288f ) {
        mask_UCBRSx = 0xFE;  // 11111110
	} else if ( fractional_portion_ >= 0.9170f ) {
        mask_UCBRSx = 0xFD;  // 11111101
	} else if ( fractional_portion_ >= 0.9004f ) {
        mask_UCBRSx = 0xFB;  // 11111011
	} else if ( fractional_portion_ >= 0.8751f ) {
        mask_UCBRSx = 0xF7;  // 11110111
	} else if ( fractional_portion_ >= 0.8572f ) {
        mask_UCBRSx = 0xEF;  // 11101111
	} else if ( fractional_portion_ >= 0.8464f ) {
        mask_UCBRSx = 0xDF;  // 11011111
	} else if ( fractional_portion_ >= 0.8333f ) {
        mask_UCBRSx = 0xBF;  // 10111111
	} else if ( fractional_portion_ >= 0.8004f ) {
        mask_UCBRSx = 0xEE;  // 11101110
	} else if ( fractional_portion_ >= 0.7861f ) {
        mask_UCBRSx = 0xED;  // 11101101
	} else if ( fractional_portion_ >= 0.7503f ) {
        mask_UCBRSx = 0xDD;  // 11011101
	} else if ( fractional_portion_ >= 0.7147f ) {
        mask_UCBRSx = 0xBB;  // 10111011
	} else if ( fractional_portion_ >= 0.7001f ) {
        mask_UCBRSx = 0xB7;  // 10110111
	} else if ( fractional_portion_ >= 0.6667f ) {
        mask_UCBRSx = 0xD6;  // 11010110
	} else if ( fractional_portion_ >= 0.6432f ) {
        mask_UCBRSx = 0xB6;  // 10110110
	} else if ( fractional_portion_ >= 0.6254f ) {
        mask_UCBRSx = 0xB5;  // 10110101
	} else if ( fractional_portion_ >= 0.6003f ) {
        mask_UCBRSx = 0xAD;  // 10101101
	} else if ( fractional_portion_ >= 0.5715f ) {
        mask_UCBRSx = 0x6B;  // 01101011
	} else if ( fractional_portion_ >= 0.5002f ) {
        mask_UCBRSx = 0xAA;  // 10101010
	} else if ( fractional_portion_ >= 0.4378f ) {
        mask_UCBRSx = 0x55;  // 01010101
	} else if ( fractional_portion_ >= 0.4286f ) {
        mask_UCBRSx = 0x53;  // 01010011
	} else if ( fractional_portion_ >= 0.4003f ) {
        mask_UCBRSx = 0x92;  // 10010010
	} else if ( fractional_portion_ >= 0.3753f ) {
        mask_UCBRSx = 0x52;  // 01010010
	} else if ( fractional_portion_ >= 0.3575f ) {
        mask_UCBRSx = 0x4A;  // 01001010
	} else if ( fractional_portion_ >= 0.3335f ) {
        mask_UCBRSx = 0x49;  // 01001001
	} else if ( fractional_portion_ >= 0.3000f ) {
        mask_UCBRSx = 0x25;  // 00100101
	} else if ( fractional_portion_ >= 0.2503f ) {
        mask_UCBRSx = 0x44;  // 01000100
	} else if ( fractional_portion_ >= 0.2224f ) {
        mask_UCBRSx = 0x22;  // 00100010
	} else if ( fractional_portion_ >= 0.2147f ) {
        mask_UCBRSx = 0x21;  // 00100001
	} else if ( fractional_portion_ >= 0.1670f ) {
        mask_UCBRSx = 0x11;  // 00010001
	} else if ( fractional_portion_ >= 0.1430f ) {
        mask_UCBRSx = 0x20;  // 00100000
	} else if ( fractional_portion_ >= 0.1252f ) {
        mask_UCBRSx = 0x10;  // 00010000
	} else if ( fractional_portion_ >= 0.1001f ) {
        mask_UCBRSx = 0x08;  // 00001000
	} else if ( fractional_portion_ >= 0.0835f ) {
        mask_UCBRSx = 0x04;  // 00000100
	} else if ( fractional_portion_ >= 0.0715f ) {
        mask_UCBRSx = 0x02;  // 00000010
	} else if ( fractional_portion_ >= 0.0529f ) {
        mask_UCBRSx = 0x01;  // 00000001
	} else {
        mask_UCBRSx = 0x00;  // 00000000
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
    mask_UCBRSx = secondModulationRegisterValueFromFractionalPortion(factor_N - (uint16_t)factor_N);
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
        result = __LINE__;
    } else if (_singleton != uartio_) {
        result = __LINE__;
    } else if (NULL == on_io_close_complete_) {
        result = __LINE__;
    } else if (!_uartio.open) {
        result = __LINE__;
    } else {
        // Wait for outstanding UART transactions to complete
        for (; 0x00 != EUSCI_A_UART_queryStatusFlags(EUSCI_A1_BASE, EUSCI_A_UART_BUSY););
        EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
        EUSCI_A_UART_disable(EUSCI_A1_BASE);
        _uartio.open = false;
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
        result = NULL;
    } else if (0 == uartio_config->baud_rate) {
        result = NULL;
    } else if (0 == uartio_config->ring_buffer_size) {
        result = NULL;
    } else if (NULL != _singleton) {
        result = NULL;
    // Confirm `uartio_config->ring_buffer_size` is a power of 2
    } else if (0 != (uartio_config->ring_buffer_size & (uartio_config->ring_buffer_size - 1))) {
        result = NULL;
    } else if (NULL == (_uartio.eusci_a1_ring_buffer = (uint8_t *)malloc(uartio_config->ring_buffer_size))) {
        result = NULL;
    } else if (NULL == (_uartio.eusci_a1_cache_buffer = (uint8_t *)malloc(uartio_config->ring_buffer_size))) {
        free(_uartio.eusci_a1_ring_buffer);
        result = NULL;
    } else {
        _uartio.config.baud_rate = uartio_config->baud_rate;
        _uartio.config.ring_buffer_size = uartio_config->ring_buffer_size;
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
        _singleton = NULL;
        free(_uartio.eusci_a1_ring_buffer);
        free(_uartio.eusci_a1_cache_buffer);
    }
}


void
uartio_destroyoption(
    const char * option_name_,
    const void * option_value_
) {
    (void)option_name_, option_value_;
}


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
        uint8_t error;
        size_t index = 0;

        // BEGIN CRITICAL SECTION
        EUSCI_A_UART_disableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT); {
            error = (_uartio.eusci_a1_ring_buffer_overflow | _uartio.eusci_a1_rx_error);
            _uartio.eusci_a1_ring_buffer_overflow = false;
            _uartio.eusci_a1_rx_error = 0x00;

            if ( (_uartio.eusci_a1_ring_buffer_tail != _uartio.eusci_a1_ring_buffer_head)
              || _uartio.eusci_a1_ring_buffer_full
            ) {
                // correct in case of overflow - OVERFLOW BYTES ARE LOST
                if (_uartio.eusci_a1_ring_buffer_full) {
                    _uartio.eusci_a1_ring_buffer_tail = _uartio.eusci_a1_ring_buffer_head;
                }

                do {
                    _uartio.eusci_a1_cache_buffer[index] = _uartio.eusci_a1_ring_buffer[_uartio.eusci_a1_ring_buffer_tail];
                    _uartio.eusci_a1_ring_buffer_tail = ((_uartio.config.ring_buffer_size - 1) & (_uartio.eusci_a1_ring_buffer_tail + 1));
                    ++index;
                } while (_uartio.eusci_a1_ring_buffer_head != _uartio.eusci_a1_ring_buffer_tail);
            }
            _uartio.eusci_a1_ring_buffer_full = false;
        } EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
        // END CRITICAL SECTION

        if (0 != index) { _uartio.on_bytes_received(_uartio.on_bytes_received_context, _uartio.eusci_a1_cache_buffer, index); }
        if (0x00 != error) { _uartio.on_io_error(_uartio.on_io_error_context); }
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
        result = __LINE__;
    } else if (_singleton != uartio_) {
        result = __LINE__;
    } else if (NULL == on_io_open_complete_) {
        result = __LINE__;
    } else if (NULL == on_bytes_received_) {
        result = __LINE__;
    } else if (NULL == on_io_error_) {
        result = __LINE__;
    } else if (_uartio.open) {
        result = __LINE__;
    } else {
        // Ensure the SMCLK is available to the UART module
        CS_enableClockRequest(CS_SMCLK);

        // Initialize UART responsible for communication with SIM808
        EUSCI_A_UART_initParam eusci_a_parameters = { 0 };
        initializeEusciAParametersForSMClkAtBaudRate(&eusci_a_parameters, _uartio.config.baud_rate);
        if (!EUSCI_A_UART_init(EUSCI_A1_BASE, &eusci_a_parameters)) {
            result = __LINE__;
        } else {
            _uartio.eusci_a1_ring_buffer_tail = _uartio.eusci_a1_ring_buffer_head;
            _uartio.eusci_a1_ring_buffer_full = false;
            _uartio.eusci_a1_ring_buffer_overflow = false;
            _uartio.eusci_a1_rx_error = 0x00;
            _uartio.open = true;
            EUSCI_A_UART_enable(EUSCI_A1_BASE);
            EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
            _uartio.on_bytes_received = on_bytes_received_;
            _uartio.on_bytes_received_context = on_bytes_received_context_;
            _uartio.on_io_error = on_io_error_;
            _uartio.on_io_error_context = on_io_error_context_;
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
        result = __LINE__;
    } else if (_singleton != uartio_) {
        result = __LINE__;
    } else if (NULL == buffer_) {
        result = __LINE__;
    } else if (0 == buffer_size_) {
        result = __LINE__;
    } else if (NULL == on_send_complete_) {
        result = __LINE__;
    } else if (!_uartio.open) {
        result = __LINE__;
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
    return __LINE__;
}


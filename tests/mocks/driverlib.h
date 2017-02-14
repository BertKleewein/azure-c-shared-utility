#ifndef MOCK_DRIVERLIB_H
#define MOCK_DRIVERLIB_H

#ifdef __cplusplus
  #include <cstdbool>
  #include <cstdint>
#else
  #include <stdbool.h>
  #include <stdint.h>
#endif

#define CS_ACLK 0x01
#define CS_SMCLK 0x04

#define EUSCI_A_UART_BUSY (0x0001)
#define EUSCI_A_UART_CLOCKSOURCE_SMCLK (0x0080)
#define EUSCI_A_UART_FRAMING_ERROR (0x0040)
#define EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION 0x00
#define EUSCI_A_UART_LSB_FIRST 0x00
#define EUSCI_A_UART_MODE (0x0000)
#define EUSCI_A_UART_NO_PARITY 0x00
#define EUSCI_A_UART_ONE_STOP_BIT 0x00
#define EUSCI_A_UART_OVERRUN_ERROR (0x0020)
#define EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION 0x01
#define EUSCI_A_UART_PARITY_ERROR (0x0010)
#define EUSCI_A_UART_RECEIVE_INTERRUPT (0x0001)
#define EUSCI_A1_BASE 0x05E0

#define GPIO_INPUT_PIN_LOW (0x00)
#define GPIO_INPUT_PIN_HIGH (0x01)
#define GPIO_PIN5 (0x0020)
#define GPIO_PIN6 (0x0040)
#define GPIO_PORT_P2 2
#define GPIO_PORT_P3 3
#define GPIO_PORT_P4 4
#define GPIO_SECONDARY_MODULE_FUNCTION (0x02)

#define TA3IV (0x1010)
#define TA3IV_NONE (0x0000)
#define TA3IV_TACCR1 (0x0002)
#define TA3IV_TAIFG (0x000E)
#define TA3IV_3 (0x0006)
#define TA3IV_4 (0x0008)
#define TA3IV_5 (0x000A)
#define TA3IV_6 (0x000C)
#define TIMER_A_CLOCKSOURCE_ACLK (1*0x100u)
#define TIMER_A_CLOCKSOURCE_DIVIDER_16 0x0F
#define TIMER_A_TAIE_INTERRUPT_ENABLE (0x0002)
#define TIMER_A_SKIP_CLEAR 0x00
#define TIMER_A3_BASE 0x0440

#define UCA1IV (0x2020)

static inline
uint16_t
interrupt_switch(
    uint16_t vector_,
    uint16_t flag_
) {
    (void)flag_;
    uint16_t flag;

    switch (vector_) {
      case TA3IV:
        flag = 0x000E;
        break;
      case UCA1IV:
        flag = 0x0002;
        break;
      default:
        flag = 0xFFFF;
    }

    return flag;
}

#define __interrupt
#define __even_in_range(x,y) interrupt_switch(x,y)
#define __never_executed()
#define __no_operation()

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EUSCI_A_UART_initParam {
    uint8_t selectClockSource;
    uint16_t clockPrescalar;
    uint8_t firstModReg;
    uint8_t secondModReg;
    uint8_t parity;
    uint16_t msborLsbFirst;
    uint16_t numberofStopBits;
    uint16_t uartMode;
    uint8_t overSampling;
} EUSCI_A_UART_initParam;

typedef struct Timer_A_initContinuousModeParam
{
    uint16_t clockSource;
    uint16_t clockSourceDivider;
    uint16_t timerInterruptEnable_TAIE;
    uint16_t timerClear;
    bool startTimer;
} Timer_A_initContinuousModeParam;

extern
void 
CS_enableClockRequest (
    uint8_t selectClock
);

extern
uint32_t
CS_getACLK(
    void
);

extern
uint32_t
CS_getSMCLK (
    void
);

extern
void
EUSCI_A_UART_disable (
    uint16_t baseAddress
);

extern
void
EUSCI_A_UART_disableInterrupt (
    uint16_t baseAddress,
    uint8_t mask
);

extern
void
EUSCI_A_UART_enable (
    uint16_t baseAddress
);

extern
void
EUSCI_A_UART_enableInterrupt (
    uint16_t baseAddress,
    uint8_t mask
);

extern
bool
EUSCI_A_UART_init (
    uint16_t baseAddress,
    EUSCI_A_UART_initParam * param
);

extern
uint8_t
EUSCI_A_UART_queryStatusFlags (
    uint16_t baseAddress,
    uint8_t mask
);

extern
uint8_t
EUSCI_A_UART_receiveData (
    uint16_t baseAddress
);

extern
void
EUSCI_A_UART_transmitData (
    uint16_t baseAddress,
    uint8_t transmitData
);


extern
uint8_t
GPIO_getInputPinValue (
    uint8_t selectedPort,
    uint16_t selectedPins
);

extern
void
GPIO_setAsInputPin (
    uint8_t selectedPort,
    uint16_t selectedPins
);

extern
void
GPIO_setAsOutputPin (
    uint8_t selectedPort,
    uint16_t selectedPins
);

extern
void
GPIO_setAsPeripheralModuleFunctionOutputPin (
    uint8_t selectedPort,
    uint16_t selectedPins,
    uint8_t mode
);

extern
void
GPIO_setOutputHighOnPin (
    uint8_t selectedPort,
    uint16_t selectedPins
);

extern
void
GPIO_setOutputLowOnPin (
    uint8_t selectedPort,
    uint16_t selectedPins
);

extern
void
Timer_A_stop (
    uint16_t baseAddress
);

extern
void
Timer_A_disableInterrupt (
    uint16_t baseAddress
);

extern
uint16_t
Timer_A_getCounterValue (
    uint16_t baseAddress
);

extern
void
Timer_A_initContinuousMode (
    uint16_t baseAddress,
    Timer_A_initContinuousModeParam * param
);

extern
void
TIMER3_A1_ISR (
    void
);

extern
void
USCI_A1_ISR (
    void
);

#ifdef __cplusplus
}
#endif

#endif // MOCK_DRIVERLIB_H

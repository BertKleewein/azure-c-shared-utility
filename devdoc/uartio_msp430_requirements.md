uartio
=============

## Overview
`uartio_msp430` implements a UART adapter for the MSP430 board using the TI `driverlib` utility.



## References

[MSP Driver Library official page](http://www.ti.com/tool/mspdriverlib)  
[Universal asynchronous receiver/transmitter - UART (generic information)](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver/transmitter)  
[XIO Interface](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/xio.h)  
[OPTIONHANDLER Interface](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/optionhandler.h)  



## Exposed API


### uartio_get_interface_description
```c
extern const IO_INTERFACE_DESCRIPTION * uartio_get_interface_description (void);
```

#### XIO Interface Contract
**SRS_UARTIO_27_000: [** `uartio_get_interface_description()` shall return a non-`NULL` pointer to an `IO_INTERFACE_DESCRIPTION` structure. **]**  
**SRS_UARTIO_27_001: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_close()`. **]**  
**SRS_UARTIO_27_002: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_create()`. **]**  
**SRS_UARTIO_27_003: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_destroy()`. **]**  
**SRS_UARTIO_27_004: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_dowork()`. **]**  
**SRS_UARTIO_27_005: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_open()`. **]**  
**SRS_UARTIO_27_006: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_retrieveoptions()`. **]**  
**SRS_UARTIO_27_007: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_send()`. **]**  
**SRS_UARTIO_27_008: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_setoption()`. **]**  

#### UARTIO Specific Implementation



## XIO API


### uartio_cloneoption
`uartio_cloneoption()` is the implementation provided to the option handler instance created as part of `uartio_retrieveoptions()`.
```c
void * uartio_cloneoption (const char * option_name, const void * option_value)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_009: [** If the `option_name` argument is NULL, `uartio_cloneoption()` shall return `NULL`. **]**  
**SRS_UARTIO_27_010: [** If the `option_value` argument is NULL, `uartio_cloneoption()` shall return `NULL`. **]**  
**SRS_UARTIO_27_011: [** If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_cloneoption()` shall return `NULL`. **]**  

#### UARTIO Specific Implementation


### uartio_close
`uartio_close()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_close` member.
```c
int uartio_close (CONCRETE_IO_HANDLE io_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void * on_io_close_complete_context)
```

#### XIO Interface Contract
**SRS_UARTIO_27_012: [** If the argument `io_handle` is NULL, `uartio_close()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_013: [** `uartio_close()` shall initiate closing the UART IO and on success it shall return 0. **]**  
**SRS_UARTIO_27_014: [** On success, `on_io_close_complete()` shall be called while passing as argument `on_io_close_complete_context`. **]**  
**SRS_UARTIO_27_015: [** If the argument `on_io_close_complete` is NULL, `uartio_close()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_016: [** If called when `uartio` is closed, `uartio_close()` shall fail and return a non-zero value. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_100: [** If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_close()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_101: [** `uartio_close()` shall await any outstanding bytes by polling `(uint8_t)EUSCI_A_UART_queryStatusFlags(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask , `EUSCI_A_UART_BUSY`, until it returns an empty mask. **]**  
**SRS_UARTIO_27_102: [** `uartio_close()` shall disable the UART interrupt (while caching the ring buffer) by calling `(void)EUSCI_A_UART_disableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. **]**  
**SRS_UARTIO_27_103: [** `uartio_close()` shall disable the UART by calling `(void)EUSCI_A_UART_disable(uint16_t baseAddress)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`. **]**  


### uartio_create
`uartio_create()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_create` member.
```c
CONCRETE_IO_HANDLE uartio_create (void * io_create_parameters);
```
The `io_create_parameters` will be in the form of `UARTIO_CONFIG *`
```c
typedef struct UARTIO_CONFIG_TAG {
    uint32_t baud_rate;
    size_t ring_buffer_size;
} UARTIO_CONFIG;
```

#### XIO Interface Contract
**SRS_UARTIO_27_017: [** If the argument `io_create_parameters` is `NULL`, `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_018: [** VALGRIND - When `uartio_create()` fails, all allocated resources up to that point shall be freed. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_104: [** If no errors are encountered, `uartio_create()` shall return a pointer to the single static instance. **]**  
**SRS_UARTIO_27_105: [** If the `baud_rate` member of the `UARTIO_CONFIG *` parameter is 0, then `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_106: [** If the `ring_buffer_size` member of the `UARTIO_CONFIG *` parameter is 0, then `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_107: [** If the `ring_buffer_size` member of the `UARTIO_CONFIG *` parameter is not a power of 2, then `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_108: [** If `uartio_create()` has previously been called without subsequently calling `uartio_destroy()`, then `uartio_create()` shall fail immediately and return `NULL`. **]**  
**SRS_UARTIO_27_109: [** If allocating memory for the ring buffer fails, then the `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_110: [** If allocating memory for the cache buffer fails, then the `uartio_create()` shall fail and return `NULL`. **]**  


### uartio_destroy
`uartio_destroy()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_destroy` member.
```c
void uartio_destroy (CONCRETE_IO_HANDLE io_handle);
```

#### XIO Interface Contract
**SRS_UARTIO_27_019: [** If the argument `io_handle` is `NULL`, `uartio_destroy()` shall do nothing. **]**  
**SRS_UARTIO_27_020: [** `uartio_destroy()` shall close the IO if it was open before freeing all the resources. **]**  
**SRS_UARTIO_27_021: [** NO OPTIONS - All options cached via `uartio_set_option` shall also be freed. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_111: [** If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_destroy()` shall do nothing. **]**  
**SRS_UARTIO_27_112: [** `uartio_destroy()` shall free the ring buffer. **]**  
**SRS_UARTIO_27_113: [** `uartio_destroy()` shall free the cache buffer. **]**  
**SRS_UARTIO_27_114: [** If `uartio_destroy()` has previously been called on a handle, then `uartio_destroy()` shall do nothing. **]**  

### uartio_destroyoption
`uartio_destroyoption()` is the implementation provided to the option handler instance created as part of `uartio_retrieveoptions()`.
```c
void uartio_destroyoption (const char * option_name, const void * option_value)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_022: [** If the `option_name` argument is `NULL`, `uartio_destroyoption()` shall do nothing. **]**  
**SRS_UARTIO_27_023: [** If the `option_value` argument is `NULL`, `uartio_destroyoption()` shall do nothing. **]**  
**SRS_UARTIO_27_024: [** If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_destroyoption()` shall do nothing. **]**  

#### UARTIO Specific Implementation


### uartio_dowork
`uartio_dowork()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_dowork` member.
```c
void uartio_dowork (CONCRETE_IO_HANDLE io_handle)
```

#### XIO Interface Contract
**SRS_UARTIO_27_025: [** If the `io_handle` argument is `NULL`, `uartio_dowork()` shall do nothing. **]**  
**SRS_UARTIO_27_026: [** If the IO is closed (and open has not been initiated), then `uartio_dowork` shall do nothing. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_115: [** If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_dowork()` shall do nothing. **]**  
**SRS_UARTIO_27_116: [** `uartio_dowork()` shall disable the UART interrupt while caching the ring buffer by calling `(void)EUSCI_A_UART_disableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. **]**  
**SRS_UARTIO_27_117: [** `uartio_dowork()` shall enable the UART interrupt after caching the ring buffer by calling `(void)EUSCI_A_UART_enableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. **]**  
**SRS_UARTIO_27_118: [** If the IO is open, `uartio_dowork()` shall indicate all bytes presented by the UART interrupt via the `on_bytes_received()` callback passed to `uartio_open()`. **]**  
**SRS_UARTIO_27_119: [** `uartio_dowork()` shall discard any bytes presented byte by the UART interrupt when the IO is closed. **]**  
**SRS_UARTIO_27_120: [** If a UART framing error is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. **]**  
**SRS_UARTIO_27_121: [** If a UART overrun error is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. **]**  
**SRS_UARTIO_27_122: [** If a UART parity error is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. **]**  
**SRS_UARTIO_27_123: [** If a ring buffer overflow is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. **]**  


### uartio_open
`uartio_open()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_open` member.
```c
int uartio_open (CONCRETE_IO_HANDLE io_handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void * on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void * on_bytes_received_context, ON_IO_ERROR on_io_error, void * on_io_error_context)
```

#### XIO Interface Contract
**SRS_UARTIO_27_027: [** If the argument `io_handle` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_028: [** If `uartio_open()` fails, the callback `on_io_open_complete()` shall be called, while passing `on_io_open_complete_context` and `IO_OPEN_ERROR` as arguments. **]**  
**SRS_UARTIO_27_029: [** If no errors are encountered, `uartio_open()` shall return 0. **]**  
**SRS_UARTIO_27_030: [** If `uartio_open()` succeeds, the callback `on_io_open_complete()` shall be called, while passing `on_io_open_complete_context` and `IO_OPEN_OK` as arguments. **]**  
**SRS_UARTIO_27_031: [** If the argument `on_io_open_complete` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_032: [** If the argument `on_bytes_received` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_033: [** If the argument `on_io_error` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_034: [** If `uartio_open()` is called while the IO is open, `uartio_open()` shall return a non-zero value without performing any work to open the IO. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_124: [** If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_open()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_125: [** `uartio_open()` shall enable clock request for the submodule clock by calling `(void)CS_enableClockRequest(uint8_t selectClock)` using the `CS_SMCLK` identifier. **]**  
**SRS_UARTIO_27_126: [** `uartio_open()` shall determine the clock speed of the submodule clock by calling `(uint32_t)CS_getSMCLK(void)`. **]**  
**SRS_UARTIO_27_127: [** `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_CLOCKSOURCE_SMCLK as the first member of the initialization parameters. **]**  
**SRS_UARTIO_27_128: [** `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_NO_PARITY as the fifth member of the initialization parameters. **]**  
**SRS_UARTIO_27_129: [** `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_LSB_FIRST as the sixth member of the initialization parameters. **]**  
**SRS_UARTIO_27_130: [** `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_ONE_STOP_BIT as the seventh member of the initialization parameters. **]**  
**SRS_UARTIO_27_131: [** `uartio_open()` shall call `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using EUSCI_A_UART_MODE as the eighth member of the initialization parameters. **]**  
**SRS_UARTIO_27_132: [** `uartio_open()` shall enable the UART by calling `(void)EUSCI_A_UART_enable(uint16_t baseAddress)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`. **]**  
**SRS_UARTIO_27_133: [** `uartio_open()` shall enable the UART interrupt by calling `(void)EUSCI_A_UART_enableInterrupt(uint16_t baseAddress, uint8_t mask)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the mask of the RX interrupt flag, `EUSCI_A_UART_RECEIVE_INTERRUPT`. **]**  
**SRS_UARTIO_27_134: [** `uartio_open()` shall fail if `false` is returned from calling `(bool)EUSCI_A_UART_init(uint16_t baseAddress, EUSCI_A_UART_initParam *param)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the calculated parameters. **]**  


### uartio_retrieveoptions
`uartio_retrieveoptions()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_retrieveoptions` member.  
```c
static OPTIONHANDLER_HANDLE uartio_retrieveoptions (CONCRETE_IO_HANDLE io_handle)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_035: [** If parameter `io_handle` is `NULL` then `uartio_retrieveoptions()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_036: [** `uartio_retrieveoptions()` shall produce an instance of `OPTIONHANDLER_HANDLE` by calling `OptionHandler_Create()`. **]**  
**SRS_UARTIO_27_037: [** If producing the `OPTIONHANDLER_HANDLE` fails, then `uartio_retrieveoptions()` shall fail and return `NULL`. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_135: [** If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_retrieveoptions()` shall fail and return a non-zero value. **]**  


### uartio_send
`uartio_send()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_send` member.
```c
int uartio_send (CONCRETE_IO_HANDLE io_handle, const void * buffer, size_t buffer_size, ON_SEND_COMPLETE on_send_complete, void * on_send_complete_context)
```

#### XIO Interface Contract
**SRS_UARTIO_27_038: [** If the argument `io_handle` is `NULL`, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_039: [** If `uartio_send()` fails, the callback `on_send_complete()` shall be called, while passing `on_send_complete_context` and `IO_SEND_ERROR` as arguments. **]**  
**SRS_UARTIO_27_040: [** If `uartio_send()` completes without errors, `on_send_complete()` shall be called while passing to it the `on_send_complete_context` value and `IO_SEND_OK` as arguments. **]**  
**SRS_UARTIO_27_041: [** If `uartio_send()` completes without errors, it shall return 0. **]**  
**SRS_UARTIO_27_042: [** If the argument `buffer` is `NULL`, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_043: [** If `buffer_size` is 0, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_044: [** If parameter `on_send_complete` is NULL, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_045: [** If `uartio_send()` is called when the IO is not open, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_049: [** If the buffer passed as argument `buffer` has NULL bytes (0x00), `uartio_send()` shall send those bytes to the UART controller. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_136: [** If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_137: [** `uartio_send()` shall send each byte of `buffer` by calling `(void)EUSCI_A_UART_transmitData(uint16_t baseAddress, uint8_t transmitData)` using the baseAddress of the UART controller, `EUSCI_A1_BASE`, and the byte to be sent, `buffer_size` times. **]**  


### uartio_setoption
`uartio_setoption()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_setoption` member.
```c
int uartio_setoption (CONCRETE_IO_HANDLE io_handle, const char * option_name, const void * option_value)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_046: [** If the argument `io_handle` is `NULL` `uartio_setoption()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_047: [** If the argument `option_name` is `NULL` `uartio_setoption()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_048: [** If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_setoption()` shall return a non-zero value. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_138: [** If the argument `io_handle` is NOT equal to the value returned by `uartio_create()`, then `uartio_setoption()` shall fail and return a non-zero value. **]**  

Options that shall be handled by uartio:  
[none]

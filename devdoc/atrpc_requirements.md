atrpc
=============

## Overview
`atrpc` implements a (R)emote (P)rocedure (C)all mechanism for (AT)tention commands used in cellular modems. The cellular modem is expected to be accessible via a serial connection adhering to the xio interface.


## References

AT Commands:  
- The Hayes Modem Technical Reference Manual, v1.0b (no longer hosted)  
- [(ITU-T Recommendation V.250) International Telecommunication Union Telecommunication Standarization Sector; Series V: Data Communication Over the Telephone Network: Control procedures - Serial asynchronous automatic dialling and control](https://www.itu.int/rec/T-REC-V.250-200307-I/en)  
- [(3GPP TS 27.007) 3rd Generation Partnership Project; Technical Specification Group Core Network and Terminals; AT command set for User Equipment (UE) (Release 14)](https://portal.3gpp.org/desktopmodules/Specifications/SpecificationDetails.aspx?specificationId=1515)  
- [SIM800 Series - AT Command Manual, v1.09](http://www.simcomm2m.com/UploadFile/TechnicalFile/SIM800%20Series_AT%20Command%20Manual_V1.09.pdf)  

RPC Info:  
- [Wikipedia: Remote Procedure Call](https://en.wikipedia.org/wiki/Remote_procedure_call)  

XIO Interface:  
- [XIO Interface](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/xio.h)  


## Exposed Types

```c
enum ta_result_code {
    OK = 0,
    CONNECT = 1,
    RING = 2,
    NO_CARRIER = 3,
    ERROR = 4,
    ATRPC_TIMEOUT = 5,
    NO_DIALTONE = 6,
    BUSY = 7,
    NO_ANSWER = 8,
    SIMCOM_PROCEEDING = 9,
};

typedef ATRPC_HANDLE void *;
typedef void(*TA_RESPONSE)(void * const context, const ta_result_code result_code, const char * const response);
```


## Exposed API

### atrpc_attention
`atrpc_attention()` will form an AT transaction and send it to the underlying xio layer.
```c
extern int atrpc_attention (ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, unsigned char * response_buffer, size_t response_buffer_size, TA_RESPONSE const ta_response, const void * const ta_response_context, CUSTOM_TA_RESULT_CODE_PARSER ta_result_code_parser, void * ta_result_code_parser_context);
```

**SRS_ATRPC_27_000: [** If the `handle` argument is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_001: [** If the `on_ta_response` argument is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_085: [** If `command_string_length` is not zero and `command_string` is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_002: [** If the `on_io_open_complete()` callback from the underlying xio has not been called, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_086: [** If `response_buffer_size` is not zero and `response_buffer` is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_003: [** If a command is currently outstanding, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_004: [** `atrpc_attention()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. **]**  
**SRS_ATRPC_27_005: [** If the call to `tickcounter_get_current_ms()` returns a non-zero value, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_006: [** `atrpc_attention()` store the command string, by calling `(void *)malloc(size_t size)` using `(command_string_length + 3)` for the `size` parameter. **]**  
**SRS_ATRPC_27_007: [** If the call to `malloc()` returns `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_008: [** `atrpc_attention()` shall call `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context)` using the xio handle returned from `xio_create()` for the handle parameter, and `AT<command_string>\r` for the `buffer` parameter, and `(command_string_length + 3)` for the `size` parameter. **]**  
**SRS_ATRPC_27_009: [** If the call to `xio_send()` returns a non-zero value, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_010: [** `atrpc_attention()` shall block until the `on_send_complete` callback passed to `xio_send()` returns. **]**  
**SRS_ATRPC_27_011: [** If no errors are encountered during execution, then `atrpc_attention()` shall return 0. **]**  
**SRS_ATRPC_27_012: [** VALGRIND - If the call to `xio_send()` returns a non-zero value, then the stored command string shall be freed. **]**  


### atrpc_close
`atrpc_close()` will close the AT RPC connection.
```c
extern int atrpc_close (ATRPC_HANDLE const handle);
```

**SRS_ATRPC_27_013: [** If the `handle` argument is `NULL`, then `atrpc_close()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_014: [** If `atrpc_open()` has not been called on the `handle`, `atrpc_close()` shall do nothing and return 0. **]**  
**SRS_ATRPC_27_015: [** If `atrpc_open()` has been called on the `handle` and the `on_open_complete` callback has not been called, `atrpc_close()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, and `ERROR_ATRPC` as the `result_code` parameter. **]**  
**SRS_ATRPC_27_016: [** `atrpc_close()` shall call `(int)xio_close(XIO_HANDLE handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void * on_io_close_complete_context)`. **]**  
**SRS_ATRPC_27_017: [** If the call to `xio_close()` returns a non-zero value, then `atrpc_close()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_018: [** `atrpc_close()` shall block until the `on_io_close_complete` callback passed to `xio_close()` completes. **]**  
**SRS_ATRPC_27_019: [** If no errors are encountered during execution, then `atrpc_close()` shall return 0. **]**  


### atrpc_create
`atrpc_create()` will allocate the memory necessary for all state necesary for AT RPC connection.
```c
extern ATRPC_HANDLE atrpc_create (void);
```
The `io_create_parameters` will be in the form of `UARTIO_CONFIG *`
```c
typedef struct UARTIO_CONFIG_TAG {
    uint32_t baud_rate;
    size_t ring_buffer_size;
} UARTIO_CONFIG;
```

**SRS_ATRPC_27_020: [** `atrpc_create()` shall call `malloc()` to allocate the memory required for the internal data structure. **]**  
**SRS_ATRPC_27_021: [** If `malloc()` fails to allocate the memory required for the internal data structure, then `atrpc_create()` shall fail and return a `NULL` handle. **]**  
**SRS_ATRPC_27_022: [** `atrpc_create()` shall create a tickcounter to support timeout functionality by calling `(TICKCOUNTER_HANDLE)tickcounter_create(void)`. **]**  
**SRS_ATRPC_27_023: [** If the call to `tickcounter_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. **]**  
**SRS_ATRPC_27_024: [** `atrpc_create()` shall acquire an xio interface to a modem chipset by calling `(IO_INTERFACE_DESCRIPTION *)uartio_get_interface_description()`. **]**  
**SRS_ATRPC_27_025: [** If the call to `uartio_get_interface_description()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. **]**  
**SRS_ATRPC_27_026: [** `atrpc_create()` shall create an xio connection to a modem chipset by calling `(XIO_HANDLE)xio_create(const IO_INTERFACE_DESCRIPTION * io_interface_description, const void * io_create_parameters)` using the interface description returned from `uartio_get_interface_description()` for `io_interface_description`. **]**  
**SRS_ATRPC_27_027: [** If the call to `xio_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. **]**  
**SRS_ATRPC_27_030: [** VALGRIND - When `atrpc_create()` returns a non-zero value, all allocated resources up to that point shall be freed. **]**  
**SRS_ATRPC_27_031: [** If no errors are encountered during execution, `atrpc_create()` shall return a handle to an AT RPC instance. **]**  


### atrpc_destroy
`atrpc_destroy()` will free all memory used by the AT RPC connection.
```c
extern void atrpc_destroy (ATRPC_HANDLE const handle);
```

**SRS_ATRPC_27_032: [** If the `handle` argument is `NULL`, then `atrpc_destroy()` shall do nothing. **]**  
**SRS_ATRPC_27_033: [** If `atrpc_open()` has previously been called and `atrpc_close()` has not been called on the `handle`, `atrpc_destroy()` shall call `(int)atrpc_close(ATRPC_HANDLE handle)` using the handle `argument` passed to `atrpc_destroy()` as the `handle` parameter. **]**  
**SRS_ATRPC_27_034: [** `atrpc_destroy()` shall call `(void)xio_destroy(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. **]**  
**SRS_ATRPC_27_036: [** `atrpc_destroy()` shall call `(void)tickcounter_destroy(TICKCOUNTER_HANDLE handle)` using the handle returned from the call to `tickcounter_create()` for the `handle` parameter. **]**  
**SRS_ATRPC_27_037: [** `atrpc_destroy()` shall free the memory required for current request. **]**  
**SRS_ATRPC_27_038: [** `atrpc_destroy()` shall free the memory required for the internal data structure. **]**  


### atrpc_dowork
`atrpc_dowork()` asynchronously processes transactions from the underlying xio layer.
```c
extern void atrpc_dowork (ATRPC_HANDLE const handle);
```

**SRS_ATRPC_27_039: [** If the `handle` argument is `NULL`, then `atrpc_dowork()` shall do nothing. **]**  
**SRS_ATRPC_27_040: [** `atrpc_dowork()` shall call `(void)xio_dowork(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. **]**  
**SRS_ATRPC_27_041: [** If `atrpc_open()` has been called on the `handle`, `atrpc_dowork()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. **]**  
**SRS_ATRPC_27_042: [** If `tickcounter_get_current_ms()` returns a non-zero value, then `dowork()` shall not attempt to calculate a timeout. **]**  
**SRS_ATRPC_27_043: [** If `attention()` was called with a timeout of 0, then `dowork()` shall not attempt to calculate a timeout. **]**  
**SRS_ATRPC_27_044: [** If `atrpc_open()` has been called on the `handle`, and the timeout value sent as `timeout_ms` to the originating `attention()` call is non-zero and has expired, then `atrpc_dowork()` shall free the stored command string. **]**  
**SRS_ATRPC_27_045: [** If `atrpc_open()` has been called on the `handle`, and the timeout value sent as `timeout_ms` to the originating `attention()` call is non-zero and has expired, then `atrpc_dowork()` shall call the terminal adapter response callback passed as `on_ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. **]**  


### atrpc_open
`atrpc_open()` will open the AT RPC connection.
```c
extern int atrpc_open (ATRPC_HANDLE const handle, TA_RESPONSE const on_open_complete, const void * const on_open_complete_context);
```

**SRS_ATRPC_27_046: [** If the `handle` argument is `NULL`, then `atrpc_open()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_047: [** If the `on_open_complete` argument is `NULL`, then `atrpc_open()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_048: [** If `atrpc_open()` has been called previously and `atrpc_close()` has not been called on the `handle`, `atrpc_open()` shall fail and return a non-zero value. **]**  
**SRS_ATRPC_27_049: [** `atrpc_open()` shall call `(int)xio_open(XIO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void * on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void * on_bytes_received_context, ON_IO_ERROR on_io_error, void * on_io_error_context)` using the handle returned from `xio_create()` as the `handle` parameter, the incoming `handle` parameter as the `on_bytes_received_context` parameter, and the incoming `handle` parameter as the `on_io_open_complete_context` parameter. **]**  
**SRS_ATRPC_27_050: [** If `xio_open()` returns a non-zero value, then `atrpc_open()` shall do nothing and return a non-zero value. **]**  
**SRS_ATRPC_27_077: [** If any errors are encountered, `atrpc_open()` shall call `on_open_complete` using the `on_open_complete_context`, and a `TA_RESULT_CODE` of `ERROR_ATRPC`. **]**  
**SRS_ATRPC_27_051: [** If no errors are encountered, `atrpc_open()` shall return 0. **]**  



## Callback behavior


### modem_on_bytes_received
`modem_on_bytes_received()` will be passed to the underlying xio during `xio_open()`.
```c
void modem_on_bytes_received (void * context, const unsigned char * buffer, size_t size);
```

**SRS_ATRPC_27_052: [** If `atrpc_open()` has not been called on the `handle` (passed in the callback context), then `modem_on_bytes_received()` shall discard all bytes. **]**  
**SRS_ATRPC_27_053: [** If the `on_open_complete()` callback has been called, then `modem_on_bytes_received()` shall store any bytes following the prefix of the `command_string` parameter passed to `attention()` along with the postfixed `<result code>"\r"` in the buffer supplied to `attention()`. **]**  
**SRS_ATRPC_27_054: [** If any bytes where captured, `modem_on_bytes_received()` shall call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the captured result code as the `result_code` parameter, a pointer to the buffer as the `message` parameter, and the size of the received message as size. **]**  
**SRS_ATRPC_27_080: [** If more bytes where captured than can fit in the supplied, `modem_on_bytes_received()` shall fill the buffer (discarding the remaining bytes) and call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the captured result code as the `result_code` parameter, a pointer to the buffer as the `message` parameter, and the size of the received message as size. **]**  
**SRS_ATRPC_27_081: [** When a `CUSTOM_RESULT_CODE_PARSER` callback is supplied to `attention()`, `modem_on_bytes_received()` shall call the callback with each byte to determine the end of a response instead of searching for a standard result code. **]**  
**SRS_ATRPC_27_082: [** When the `CUSTOM_RESULT_CODE_PARSER` callback indicates completion by returning a non-zero value, `modem_on_bytes_received()` shall return the result code value supplied by the callback as the result code sent to the `ON_TA_RESULT` callback. **]**  
**SRS_ATRPC_27_056: [** If the ping times-out when negotiating auto-baud, then `modem_on_bytes_received()` shall reissue the ping by calling `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context)` using the xio handle returned from `xio_create()` for the handle parameter, and `AT\r` for the `buffer` parameter, and `3` for the `size` parameter. **]**  
**SRS_ATRPC_27_057: [** If `atrpc_attention` returns a non-zero value, then `on_io_open_complete()` shall call the `on_open_complete` callback passed to `atrpc_open()` using the `on_open_complete_context` parameter passed to `atrpc_open()` as the `context` parameter, `ERROR_3GPP` as the `result_code` parameter, and `NULL` as the `response` parameter. **]**  
**SRS_ATRPC_27_058: [** During auto-baud negotiation, `modem_on_bytes_received()` shall accept "0\r" as a valid response. **]**  
**SRS_ATRPC_27_059: [** During auto-baud negotiation, `modem_on_bytes_received()` shall accept "\r\nOK\r\n" as a valid response. **]**  
**SRS_ATRPC_27_060: [** Once a complete response has been received, then `modem_on_bytes_received()` shall free the stored command string. **]**  
**SRS_ATRPC_27_061: [** When auto-baud negotiation has completed, then `modem_on_bytes_received()` shall normalize the ta responses by calling `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context)` using the xio handle returned from `xio_create()` during `atrpc_create()` for the handle parameter, and `ATE1V0\r` for the `buffer` parameter, and `7` for the `size` parameter. **]**  
**SRS_ATRPC_27_062: [** If the call to `attention()` returns a non-zero value, then `modem_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and `NULL` as the `response` parameter. **]**  
**SRS_ATRPC_27_063: [** Once the communication with the modem has been normalized, `modem_on_bytes_received()` shall write the active profile by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `&W` as the `command_string` parameter, `2` as the `command_string_length` parameter, `0` as the `timeout_ms` parameter, `modem_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter. **]**  
**SRS_ATRPC_27_064: [** If the call to `attention()` returns a non-zero value, then `modem_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to write the active profile!" as the `response` parameter. **]**  
**SRS_ATRPC_27_065: [** During auto-baud negotiation, if 50 or more time-outs occur, then `modem_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `response` parameter. **]**  
**SRS_ATRPC_27_066: [** Once the profile has been successfully stored, then `modem_on_bytes_received()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_OK` as the `result_code` parameter, and `NULL` as the `response` parameter. **]**  


### on_io_close_complete
`on_io_close_complete()` will be passed to the underlying xio during `xio_close()`.
```c
void on_io_close_complete (void * context);
```

**SRS_ATRPC_27_067: [** `on_io_close_complete()` shall call nothing. **]**  


### on_io_error
`on_io_error()` will be passed to the underlying xio during `xio_open()`.
```c
void on_io_error (void * context);
```

**SRS_ATRPC_27_068: [** `on_io_error()` shall free the stored command string. **]**  
**SRS_ATRPC_27_069: [** `on_io_error()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. **]**  


### on_io_open_complete
`on_io_open_complete()` will be passed to the underlying xio during `xio_open()`.
```c
void on_io_open_complete (void * context, IO_OPEN_RESULT open_result);
```

**SRS_ATRPC_27_070: [** If the `open_result` parameter is not `IO_OPEN_OK`, then `on_io_open_complete()` shall call the `on_open_complete` callback passed to `atrpc_open()` using the `on_open_complete_context` parameter passed to `atrpc_open()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `response` parameter. **]**  
**SRS_ATRPC_27_071: [** If the `open_result` parameter is `OPEN_OK`, then `on_io_open_complete()` shall initiate the auto-bauding procedure by calling `(void)atrpc_attention(void * context, const char * const command_string, const size_t command_string_length const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)` using the incoming `context` parameter as the `handle` parameter, `NULL` as the `command_string` parameter, `0` as the `command_string_length` parameter, `100` as the `timeout_ms` parameter, `modem_handshake` as the `ta_response` parameter, and `ta_response_context` as the `context` parameter. **]**  
**SRS_ATRPC_27_072: [** If `atrpc_attention` returns a non-zero value, then `on_io_open_complete()` shall call the `on_open_complete` callback passed to `atrpc_open()` using the `on_open_complete_context` parameter passed to `atrpc_open()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `response` parameter. **]**  


### on_send_complete
`on_send_complete()` will be passed to the underlying xio during `xio_send()`.
```c
void on_send_complete (void * context, IO_SEND_RESULT send_result);
```

**SRS_ATRPC_27_073: [** If the result of underlying xio `on_io_send_complete()` is `IO_SEND_OK`, then `on_send_complete()` shall call nothing. **]**  
**SRS_ATRPC_27_074: [** If the result of underlying xio `on_io_send_complete()` is `IO_SEND_CANCELLED`, then `on_send_complete()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. **]**  
**SRS_ATRPC_27_075: [** If the result of underlying xio `on_io_send_complete()` is not `IO_SEND_OK`, then `on_send_complete()` shall free the command string passed to `attention()`. **]**  
**SRS_ATRPC_27_076: [** If the result of underlying xio `on_io_send_complete()` is `IO_SEND_ERROR`, then `on_send_complete()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. **]**  


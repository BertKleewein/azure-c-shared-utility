socket_async
=================

## Overview

**socket_async** abstracts a non-blocking TCP or UDP socket while hiding OS implementation details. The socket handling code in `socket_async.c` is common to many environments, and
a system of header includes and #defines will adapt the code for each particular environment.

It is anticipated that socket_async.c will work for all non-Windows environments, and a socket_async_win32.c will be needed for Windows.
## References

[socket_async.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/socket_async.h)  
[sys/socket.h, a typical linux socket implementation](http://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html)

###   Exposed API

**SRS_SOCKET_ASYNC_30_001: [** The socket_async shall use the constants defined in `socket_async.h`.
```c
#define SOCKET_ASYNC_NULL_SOCKET -1
```
 **]**

**SRS_SOCKET_ASYNC_30_002: [** The socket_async shall implement the methods defined in `socket_async.h`.
```c
int socket_async_create(uint32_t host_ipv4, int port, bool is_UDP);
int socket_async_get_create_status(int sock);
int socket_async_send(int sock, void* buffer, size_t size);
int socket_async_receive(int sock, void* buffer, size_t size);
void socket_async_close(int sock);
```
 **]**


###   socket_async_create
`socket_async_create` creates a socket and sets its configuration, including setting the socket to non-blocking. It then binds the socket to the supplied `host_ipv4` and `port`, and finally connects the socket to the bound address.

If successful, it returns a non-negative integer to represent the socket's file descriptor. On failure, it returns SOCKET_ASYNC_NULL_SOCKET.

```c
int socket_async_create(uint32_t host_ipv4, int port, bool is_UDP);
```

**SRS_SOCKET_ASYNC_30_002: [** The `host_ipv4` parameter shall be the 32-bit IP V4 of the target server. **]**

**SRS_SOCKET_ASYNC_30_003: [** The `port` parameter shall be the port number for the target server. **]**

**SRS_SOCKET_ASYNC_30_004: [** The `is_UDP` parameter shall be `true` for a UDP connection, and `false` for TCP. **]**

**SRS_SOCKET_ASYNC_30_005: [** The keep-alive behavior of TCP sockets shall be set to a reasonable value. The specifics of this behavior are deliberately omitted from this spec. **]**

**SRS_SOCKET_ASYNC_30_006: [** The returned socket shall be non-blocking. **]**

**SRS_SOCKET_ASYNC_30_007: [** On success, the return value shall be the non-negative socket handle. **]**

**SRS_SOCKET_ASYNC_30_008: [** If socket binding fails, `socket_async_create` shall return SOCKET_ASYNC_NULL_SOCKET. **]**

**SRS_SOCKET_ASYNC_30_009: [** If socket connection fails, `socket_async_create` shall return SOCKET_ASYNC_NULL_SOCKET. **]**


###   socket_async_send
`socket_async_send` attempts to send `size` bytes from `buffer` to the host.

If successful, `socket_async_send` shall return the non-negative number of bytes sent (this value may be 0). On failure, it returns SOCKET_ASYNC_NULL_SOCKET.

```c
int socket_async_send(int sock, void* buffer, size_t size);
```

**SRS_SOCKET_ASYNC_30_010: [** The `sock` parameter shall be the socket to send the message to. **]**

**SRS_SOCKET_ASYNC_30_011: [** The `buffer` parameter shall contain the message to send to the target server. **]**

**SRS_SOCKET_ASYNC_30_012: [** The `size` parameter shall be the size of the message in bytes. **]**

**SRS_SOCKET_ASYNC_30_013: [** On success, the return value shall be the non-negative number of bytes queued for transmission (this value may be 0). **]**

**SRS_SOCKET_ASYNC_30_014: [** If `socket_async_send` fails, `socket_async_send` shall close the socket and return SOCKET_ASYNC_NULL_SOCKET. **]**

###   socket_async_receive
`socket_async_receive` attempts to receive up to `size` bytes into `buffer`.

If successful, `socket_async_receive` shall return the non-negative number of bytes received (this value may be 0). On failure, it returns SOCKET_ASYNC_NULL_SOCKET.

```c
int socket_async_receive(int sock, void* buffer, size_t size);
```

**SRS_SOCKET_ASYNC_30_015: [** The `sock` parameter shall be the socket to receive the message from. **]**

**SRS_SOCKET_ASYNC_30_016: [** The `buffer` parameter shall receive the message from the target server. **]**

**SRS_SOCKET_ASYNC_30_017: [** The `size` parameter shall be the size of the `buffer` in bytes. **]**

**SRS_SOCKET_ASYNC_30_018: [** On success, the return value shall be the non-negative number of bytes received into `buffer` (this value may be 0). **]**

**SRS_SOCKET_ASYNC_30_019: [** If `socket_async_receive` fails, `socket_async_receive` shall close the socket and return SOCKET_ASYNC_NULL_SOCKET. **]**


 ###   socket_async_close
 `socket_async_close` calls the underlying socket `close()` on the supplied socket. Parameter validation is deferred to the underlying call, so no validation is performed by `socket_async_close`.

 ```c
 void socket_async_close(int sock);
 ```

**SRS_SOCKET_ASYNC_30_020: [** The `sock` parameter shall be the integer file descriptor of the socket to be closed. **]**  

**SRS_SOCKET_ASYNC_30_021: [** `socket_async_close` shall call the underlying `close` method on the supplied socket. **]**  

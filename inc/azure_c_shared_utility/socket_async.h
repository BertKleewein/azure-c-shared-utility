// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/** @file ssl_socket.h
 *	@brief	 Implements socket creation for TLSIO adapters.
 */

#ifndef AZURE_SOCKET_ASYNC_H
#define AZURE_SOCKET_ASYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/umock_c_prod.h"

// socket_async exposes asynchronous socket operations while hiding OS-specifics. Committing to
// asynchronous operation also simplifies the interface compared to generic sockets.

define SOCKET_ASYNC_NULL_SOCKET -1

    /**
    * @brief	Create a non-blocking socket that is correctly configured for use by a TLSIO adapter.
    *
    * @param   host_ipv4	The IPv4 of the SSL server to be contacted.
    *
    * @param   port	The port of the SSL server to use.
    *
    * @param   is_UDP True for UDP, false for TCP.
    *
    * @return   @c An integer file descriptor (fd) if the API call
    *           is successful or SOCKET_ASYNC_NULL_SOCKET in case it fails. Error logging is
    *           performed by the underlying concrete implementation, so no
    *           further error logging is necessary. 
    */
    MOCKABLE_FUNCTION(, int, socket_async_create, uint32_t, host_ipv4, int, port, bool, is_UDP);

    /**
    * @brief	Send a message on the specified socket.
    *
    * @param    sock The socket to be used.
    *
    * @param    buffer The buffer containing the message to transmit.
    *
    * @param    size The number of bytes to transmit.
    *
    * @return   @c A non-negative integer N means that N bytes have been queued for transmission. The N == 0
    *           case implies that the socket's outgoing buffer was full.
    *           SOCKET_ASYNC_NULL_SOCKET means an unexpected error has occurred and the socket has been closed.
    */
    MOCKABLE_FUNCTION(, int, socket_async_send, int, sock, void*, buffer, size_t, size);

    /**
    * @brief	Receive a message on the specified socket.
    *
    * @param    sock The socket to be used.
    *
    * @param    buffer The buffer containing the message to receive.
    *
    * @param    size The buffer size in bytes.
    *
    * @return   @c A non-negative integer N means that N bytes received into buffer.
    *           SOCKET_ASYNC_NULL_SOCKET means an unexpected error has occurred and the socket has been closed.
    */
    MOCKABLE_FUNCTION(, int, socket_async_receive, int, sock, void*, buffer, size_t, size);


    /**
    * @brief	Close the socket returned by socket_async_create.
    *
    * @param   sock     The socket to be closed.
    */
    MOCKABLE_FUNCTION(, void, socket_async_close, int, sock);


#ifdef __cplusplus
}
#endif

#endif /* AZURE_SOCKET_ASYNC_H */

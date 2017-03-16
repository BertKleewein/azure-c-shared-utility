// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/** @file ssl_socket.h
 *	@brief	 Implements socket creation for TLSIO adapters.
 */

#ifndef AZURE_SSL_SOCKET_H
#define AZURE_SSL_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/umock_c_prod.h"


	/**
	* @brief	Create a non-blocking socket that is correctly configured for use by a TLSIO adapter.
	*
	* @param   serverName	The url of the SSL server to be contacted.
	*
	* @return	@c A non-zero integer file descriptor (fd) if the API call
	*          is successful or 0 in case it fails. Error logging is
	*          performed by the underlying concrete implementation, so no
	*          further error logging is necessary.
	*/
	MOCKABLE_FUNCTION(, int, SSL_Socket_Create, const char*, serverName);

	/**
	* @brief	Close the socket returned by SSL_Socket_Create.
	*
	* @param   serverName	The url of the SSL server to be contacted.
	*/
	MOCKABLE_FUNCTION(, void, SSL_Socket_Close, int, socket);


#ifdef __cplusplus
}
#endif

#endif /* AZURE_SSL_SOCKET_H */
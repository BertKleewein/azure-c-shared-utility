// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UARTIO_H
#define UARTIO_H

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
  #include <cstddef>
  #include <cstdint>
  extern "C" {
#else
  #include <stddef.h>
  #include <stdint.h>
#endif /* __cplusplus */

typedef struct UARTIO_CONFIG_TAG
{
	uint32_t baud_rate;
	size_t ring_buffer_size;
} UARTIO_CONFIG;

MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION *, uartio_get_interface_description);

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* UARTIO_H */

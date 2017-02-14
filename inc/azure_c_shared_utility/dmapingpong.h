// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef DMA_PINGPONG_H
#define DMA_PINGPONG_H

#include "azure_c_shared_utility/macro_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PingPongBuffer {
    uint8_t *buffer1;
    uint8_t *buffer2;
    uint8_t *currentWriteBuffer;
    uint16_t channel;
} PingPongBuffer;

#include "azure_c_shared_utility/umock_c_prod.h"

MOCKABLE_FUNCTION(, int, pingpong_alloc, PingPongBuffer *, pp);
MOCKABLE_FUNCTION(, void, pingpong_free, PingPongBuffer *, pp);
MOCKABLE_FUNCTION(, void, pingpong_attach_to_register, PingPongBuffer *, pp, uint8_t, channel, uint8_t, trigger, uint16_t, reg);
MOCKABLE_FUNCTION(, void, pingpong_enable, PingPongBuffer *, pp);
MOCKABLE_FUNCTION(, void, pingpong_disable, PingPongBuffer *, pp);
MOCKABLE_FUNCTION(, void, pingpong_flipflop, PingPongBuffer *, pp, unsigned char **, base, size_t *, size);
MOCKABLE_FUNCTION(, bool, pingpong_check_for_data, PingPongBuffer *, pp);

#ifdef __cplusplus
}
#endif

#endif /* DMA_PINGPONG_H */


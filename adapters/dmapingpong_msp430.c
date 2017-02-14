// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
  #include <cassert>
  #include <cstdbool>
  #include <cstddef>
  #include <cstdlib>
  #include <cstdint>
#else
  #include <assert.h>
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdlib.h>
  #include <stdint.h>
  #include <stdio.h>
#endif

#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/dmapingpong.h"
#include "driverlib.h"

#define INVALID_CHANNEL 0xff
#define PINGPONG_SIZE 32

int pingpong_alloc(PingPongBuffer *pp)
{
    int result;

    pp->channel = INVALID_CHANNEL;
    
    // BKTODO: it would be nice to put these pingpongs into RAM instead of FRAM
    if (NULL ==  (pp->buffer1 = malloc(PINGPONG_SIZE)))
    {
        LogError("allocation error");
        result = __FAILURE__;
    } 
    else if (NULL ==  (pp->buffer2 = malloc(PINGPONG_SIZE)))
    {
        free(pp->buffer1);
        LogError("allocation error");
        result = __FAILURE__;
    } 
    else
    {
        result = 0;
    }

    return result;

}


void pingpong_free(PingPongBuffer *pp)
{
        free(pp->buffer1);
        free(pp->buffer2);
}

void pingpong_attach_to_register(PingPongBuffer *pp, uint8_t channel, uint8_t trigger, uint16_t reg)
{
    DMA_initParam param;

    param.channelSelect = channel; 
    param.transferModeSelect = DMA_TRANSFER_BLOCK;
    param.transferSize = PINGPONG_SIZE;
    param.triggerSourceSelect = trigger;
    param.transferUnitSelect = DMA_SIZE_SRCWORD_DSTBYTE;
    param.triggerTypeSelect = DMA_TRIGGER_HIGH;
    DMA_init(&param);

    pp->channel = channel;
    pp->currentWriteBuffer = pp->buffer1;

    DMA_setSrcAddress(channel, (uint32_t)reg, DMA_DIRECTION_UNCHANGED);
    DMA_setDstAddress(channel, (uint32_t)pp->currentWriteBuffer, DMA_DIRECTION_INCREMENT);
    DMA_setTransferSize(channel, PINGPONG_SIZE);
}

void pingpong_enable(PingPongBuffer *pp)
{
    DMA_enableTransfers(pp->channel);
}

void pingpong_disable(PingPongBuffer *pp)
{
    if (pp->channel != INVALID_CHANNEL)
    {
        DMA_disableTransfers(pp->channel);
    }
}

void pingpong_flipflop(PingPongBuffer *pp, unsigned char **base, size_t *size)
{
    // Assume pingpongs are already disabled.  If they're not, bad things happen :(
    uint16_t remaining = DMA_getTransferSize(pp->channel);

    *base = pp->currentWriteBuffer;
    *size = PINGPONG_SIZE - remaining;

    uint8_t *newWriteBuffer = pp->buffer2;
    if (pp->currentWriteBuffer == pp->buffer2)
    {
        newWriteBuffer = pp->buffer1;
    }
    pp->currentWriteBuffer = newWriteBuffer;
    
    DMA_setDstAddress(pp->channel, (uint32_t)pp->currentWriteBuffer, DMA_DIRECTION_INCREMENT);
    DMA_setTransferSize(pp->channel, PINGPONG_SIZE);
}

bool pingpong_check_for_data(PingPongBuffer *pp)
{
    uint16_t remaining = DMA_getTransferSize(pp->channel);
    return (remaining != PINGPONG_SIZE);
}



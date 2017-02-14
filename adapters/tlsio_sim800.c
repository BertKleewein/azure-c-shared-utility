// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/cellchip.h"
#include "tlsio_sim800.h"
 
CONCRETE_IO_HANDLE tlsio_sim800_create(void* io_create_parameters);
void tlsio_sim800_destroy(CONCRETE_IO_HANDLE handle);
int tlsio_sim800_open(CONCRETE_IO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
int tlsio_sim800_close(CONCRETE_IO_HANDLE handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
int tlsio_sim800_send(CONCRETE_IO_HANDLE handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
void tlsio_sim800_dowork(CONCRETE_IO_HANDLE handle);
int tlsio_sim800_setoption(CONCRETE_IO_HANDLE handle, const char* optionName, const void* value);
OPTIONHANDLER_HANDLE tlsio_sim800_retrieveoptions(CONCRETE_IO_HANDLE handle);
// BKTODO: I can use unsignedIntToString from crtabstractions.h


static const IO_INTERFACE_DESCRIPTION tlsio_sim800_interface_description =
{
    tlsio_sim800_retrieveoptions,
    tlsio_sim800_create,
    tlsio_sim800_destroy,
    tlsio_sim800_open,
    tlsio_sim800_close,
    tlsio_sim800_send,
    tlsio_sim800_dowork,
    tlsio_sim800_setoption
};

typedef struct TLSIO_SIM800_INSTANCE_TAG
{
    CELLCHIP_HANDLE cellchip;
    const char* hostname;
    int port;
    
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;

    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;

    ON_IO_ERROR on_io_error;
    void* on_io_error_context;

    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    void* on_io_close_complete_context;

    ON_SEND_COMPLETE on_send_complete;
    void* on_send_complete_context;

} TLSIO_SIM800_INSTANCE;

// BKTODO: when  do we call the error handler?  This seems to overlap with other handlers.


const IO_INTERFACE_DESCRIPTION* tlsio_sim800_get_interface_description(void)
{
    return &tlsio_sim800_interface_description;
}

CONCRETE_IO_HANDLE tlsio_sim800_create(void* io_create_parameters)
{
    TLSIO_SIM800_INSTANCE *tlsio;
    TLSIO_CONFIG *config = (TLSIO_CONFIG*)io_create_parameters;

    if (NULL == config || NULL == config->hostname)
    {
        LogError("invalid args");
        tlsio = NULL;
    }
    else if (NULL == (tlsio = (TLSIO_SIM800_INSTANCE*)malloc(sizeof(TLSIO_SIM800_INSTANCE))))
    {
        LogError("Allocation failure");
    }
    else if (NULL == (tlsio->cellchip = cellchip_create()))
    {
        LogError("failure creating cellchip object");
        free(tlsio);
        tlsio = NULL;
    }
    else
    {
        tlsio->hostname = config->hostname;
        tlsio->port = config->port;
        
        tlsio->on_io_open_complete = NULL;
        tlsio->on_bytes_received = NULL;
        tlsio->on_io_error = NULL;
        tlsio->on_io_close_complete = NULL;
        tlsio->on_send_complete = NULL;
    }
    
    return (CONCRETE_IO_HANDLE)tlsio;
}

void tlsio_sim800_destroy(CONCRETE_IO_HANDLE handle)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)handle;
    
    if (NULL != tlsio)
    {
        if (NULL != tlsio->cellchip)
        {
            cellchip_destroy(tlsio->cellchip);
        }
    }
}

static void on_cellchip_connect_complete(void * context, CELLCHIP_RESULT_CODE cellchip_result)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)context;

    if (tlsio->on_io_open_complete)
    {
        tlsio->on_io_open_complete(tlsio->on_io_open_complete_context, cellchip_result == CELLCHIP_OK ? IO_OPEN_OK : IO_OPEN_ERROR);
        tlsio->on_io_open_complete = NULL;
    }
}

static void on_cellchip_attach_complete(void * context, CELLCHIP_RESULT_CODE cellchip_result)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)context;

    if (cellchip_result == CELLCHIP_OK)
    {
        if (0 != cellchip_tls_connect(tlsio->cellchip, tlsio->hostname, tlsio->port, on_cellchip_connect_complete, tlsio))
        {
            LogError("cellchip_tls_connect failed");
            if (tlsio->on_io_open_complete)
            {
                tlsio->on_io_open_complete(tlsio->on_io_open_complete_context, IO_OPEN_ERROR);
                tlsio->on_io_open_complete = NULL;
            }
        }
    }
    else
    {
        LogError("attach sequence failed");
        if (tlsio->on_io_open_complete)
        {
            tlsio->on_io_open_complete(tlsio->on_io_open_complete_context, IO_OPEN_ERROR);
            tlsio->on_io_open_complete = NULL;
        }
    }
}

static void on_cellchip_state_change(void *context, CELLCHIP_CONNECTION_STATE old_state, CELLCHIP_CONNECTION_STATE new_state)
{
}

static void on_cellchip_data_received(void* context, const uint8_t *data, size_t size)
{
}

static void on_cellchip_open_complete(void * context, CELLCHIP_RESULT_CODE cellchip_result)
{
    int result;
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)context;

    if (cellchip_result == CELLCHIP_OK)
    {
        if (0 != cellchip_attach_to_network(tlsio->cellchip, on_cellchip_attach_complete, tlsio))
        {
            LogError("cellchip_tls_connect returned failure");
            result = __FAILURE__;
        }
        else
        {
            result = 0;
        }
    }
    else 
    {
        LogError("cellchip_open failure");
        result = __FAILURE__;
    }


    if (result != 0 && tlsio->on_io_open_complete)
    {
        tlsio->on_io_open_complete(tlsio->on_io_open_complete_context, IO_OPEN_ERROR);
        tlsio->on_io_open_complete = NULL;
    }
}

int tlsio_sim800_open(CONCRETE_IO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)handle;
    int result;
    
    if (NULL == tlsio)
    {
        LogError("invalid arg to tlsio_sim800_open");
        result = __FAILURE__;
    }
    else
    {
        tlsio->on_io_open_complete = on_io_open_complete;
        tlsio->on_io_open_complete_context = on_io_open_complete_context;
        tlsio->on_bytes_received = on_bytes_received;
        tlsio->on_bytes_received_context = on_bytes_received_context;
        tlsio->on_io_error = on_io_error;
        tlsio->on_io_error_context = on_io_error_context;

        if (0 != cellchip_open(tlsio->cellchip, on_cellchip_open_complete, tlsio, on_cellchip_state_change, tlsio, on_cellchip_data_received, tlsio))
        {
            LogError("cellchip_open failed");
            result = __FAILURE__;
        }
    }

    return result;
}
       
int tlsio_sim800_close(CONCRETE_IO_HANDLE handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)handle;
    int result;
    
    if (NULL == tlsio)
    {
        LogError("invalid arg to tlsio_sim800_open");
        result = __FAILURE__;
    }
    else if (0 != cellchip_close(tlsio->cellchip))
    {
        LogError("cellchip_close failed");
        result = __FAILURE__;
    }
    else
    {
        result = 0;
    }
    
    if (on_io_close_complete)
    {
        on_io_close_complete(callback_context);
    }
    return result;
}


static void on_cellchip_send_complete(void* context, CELLCHIP_RESULT_CODE cellchip_result)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)context;
    if (tlsio->on_send_complete != NULL)
    {
        tlsio->on_send_complete(context, cellchip_result == CELLCHIP_OK ? IO_SEND_OK : IO_SEND_ERROR);
        tlsio->on_send_complete = NULL;
    }
}

int tlsio_sim800_send(CONCRETE_IO_HANDLE handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)handle;
    int result;
    
    if (NULL == tlsio || buffer == NULL || size == 0)
    {
        LogError("invalid arg to tlsio_sim800_send");
        result = __FAILURE__;
    } 
    else if (0 != cellchip_send(tlsio->cellchip, buffer, size, on_cellchip_send_complete, tlsio))
    {
        if (on_send_complete != NULL)
        {
            on_send_complete(on_send_complete_context, IO_SEND_ERROR);
        }
        LogError("cellchip_send failed");
        result = __FAILURE__;
    }

    return result;
}

void tlsio_sim800_dowork(CONCRETE_IO_HANDLE handle)
{
    TLSIO_SIM800_INSTANCE *tlsio = (TLSIO_SIM800_INSTANCE*)handle;
    
    if (NULL == tlsio)
    {
        LogError("invalid arg to tlsio_sim800_open");
    }
    else
    {
        cellchip_dowork(tlsio->cellchip);
    }
}

int tlsio_sim800_setoption(CONCRETE_IO_HANDLE handle, const char* optionName, const void* value)
{
    // BKTODO
    return __FAILURE__;
}

OPTIONHANDLER_HANDLE tlsio_sim800_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    // BKTODO
    return NULL;
}


// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "msp430.h"
#include "azure_c_shared_utility/xlogging.h"

#include "azure_c_shared_utility/threadapi.h"

void ThreadAPI_Sleep(unsigned int milliseconds)
{
    msp430_sleep(milliseconds);
}

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
    LogError("This platform does not support multi-thread function.");
    return THREADAPI_ERROR;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res)
{
    LogError("This platform does not support multi-thread function.");
    return THREADAPI_ERROR;
}

void ThreadAPI_Exit(int res)
{
    LogError("This platform does not support multi-thread function.");
}

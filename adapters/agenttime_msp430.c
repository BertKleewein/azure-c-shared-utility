// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/agenttime.h"

// mbed version of gmtime() returns NULL.
// system RTC should be set to UTC as its localtime 

time_t get_time(time_t* currentTime)
{
    // BKTODO
    return 0;
}

double get_difftime(time_t stopTime, time_t startTime)
{
    // BKTODO
    return 0;
}



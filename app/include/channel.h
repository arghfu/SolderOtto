#pragma once
#include <stdbool.h>

#include "pid.h"

#define CHANNEL_TIPS 2

typedef enum connection_state
{
    CHANNEL_DISCONNECTED = 0,
    CHANNEL_CONNECTED,
    CHANNEL_UNKNOWN,
    CHANNEL_T210,
    CHANNEL_T245,
    CHANNEL_AM120,
} connection_state_t;

typedef struct channel
{

    pid_t pid[CHANNEL_TIPS];

    bool enabled;
} solder_channel_t;

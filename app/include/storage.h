#ifndef FLASH_H
#define FLASH_H

#include "pid.h"
#include "channel.h"

int storage_init();
// int flash_write(int addr, int value);
// int flash_read(int addr);
int storage_get_pid_data(enum channel_type type, struct pid_data *data);
#endif // FLASH_H


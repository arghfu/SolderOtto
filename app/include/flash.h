#ifndef FLASH_H
#define FLASH_H

int flash_init();
int flash_write(int addr, int value);
int flash_read(int addr);

#endif // FLASH_H


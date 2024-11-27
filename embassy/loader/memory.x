MEMORY
{
  /* NOTE 1 K = 1 KiBi = 1024 bytes */
  FLASH                             : ORIGIN = 0x08000000, LENGTH = 64K
  BOOTLOADER_STATE                  : ORIGIN = 0x08010000, LENGTH = 64K
  ACTIVE                            : ORIGIN = 0x08020000, LENGTH = 188K
  DFU                               : ORIGIN = 0x0804F000, LENGTH = 196K
  RAM                         (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

__bootloader_state_start = ORIGIN(BOOTLOADER_STATE) - ORIGIN(FLASH);
__bootloader_state_end = ORIGIN(BOOTLOADER_STATE) + LENGTH(BOOTLOADER_STATE) - ORIGIN(FLASH);

__bootloader_active_start = ORIGIN(ACTIVE) - ORIGIN(FLASH);
__bootloader_active_end = ORIGIN(ACTIVE) + LENGTH(ACTIVE) - ORIGIN(FLASH);

__bootloader_dfu_start = ORIGIN(DFU) - ORIGIN(FLASH);
__bootloader_dfu_end = ORIGIN(DFU) + LENGTH(DFU) - ORIGIN(FLASH);



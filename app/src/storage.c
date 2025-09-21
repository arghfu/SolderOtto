#include <zephyr/logging/log.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include "storage.h"


LOG_MODULE_REGISTER(storage, CONFIG_APP_LOG_LEVEL);
#define STORAGE_DEVICE FIXED_PARTITION_DEVICE(storage)
#define STORAGE_OFFSET FIXED_PARTITION_OFFSET(storage)

#define FLASH_KEY_PID_T210 0x0101
#define FLASH_KEY_PID_T245 0x0102
#define FLASH_KEY_PID_AM120 0x0103

struct nvs_fs fs;

int storage_init()
{
    struct flash_pages_info info;

    fs.flash_device = STORAGE_DEVICE;
    if (!device_is_ready(fs.flash_device))
    {
        LOG_ERR("device not ready.");
        return 0;
    }

    fs.offset = STORAGE_OFFSET;
    int rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (rc)
    {
        LOG_ERR("Unable to get page info, rc=%d\n", rc);
        return 0;
    }

    fs.sector_size = info.size;
    fs.sector_count = 4;

    // rc = zms_mount(&fs);
    // if (rc)
    // {
    //     LOG_ERR("Unable to mount zms, rc=%d\n", rc);
    // }
    // LOG_INF("%d", STORAGE_OFFSET);
    //
    // ssize_t free_space = zms_calc_free_space(&fs);
    // printk("Free space in storage is %u bytes\n", free_space);

    return 0;
}

int storage_get_pid_data(struct pid_data* data)
{
    __ASSERT(data != NULL, "data pointer is null");

    return 0;
}

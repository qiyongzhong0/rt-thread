/*
 * File      : at_device_test.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-12-30     qiyongzhong       first version
 */

#include <rtthread.h>
#include <string.h>
#include <finsh.h>
#include <at_device.h>

#ifndef AT_TEST_DEVICE_NAME
#define AT_TEST_DEVICE_NAME      "bc26"
#endif

static int at_device_test(int argc, char **argv)
{
    static struct at_device *device = RT_NULL;
    int value = 0;
    int result = RT_EOK;
    char *result_str;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "probe"))
        {
            if (argc == 3)
            {
                device = (struct at_device *)at_device_get_by_name(AT_DEVICE_NAMETYPE_DEVICE, argv[2]);
                result_str = (device == RT_NULL) ? "failure" : "success";
                rt_kprintf("probe %s %s \n", argv[2], result_str);
            }
            else
            {
                rt_kprintf("at_device probe <at_devive_name>   - probe at_device by name\n");
            }
        }
        else
        {
            if (device == RT_NULL)
            {
                rt_kprintf("Please using 'at_device probe <at_devive_name>' first\n");
                return -RT_ERROR;
            }
            if (!strcmp(argv[1], "init"))
            {
                result = device->class->device_ops->init(device);
                result_str = (result == RT_EOK) ? "success" : "failure";
                rt_kprintf("%s device initialization %s \n", device->name, result_str);
            }
            else if (!strcmp(argv[1], "deinit"))
            {
                result = device->class->device_ops->deinit(device);
                result_str = (result == RT_EOK) ? "success" : "failure";
                rt_kprintf("%s device desinitialization %s \n", device->name, result_str);
            }
            else if (!strcmp(argv[1], "sleep"))
            {
                result = device->class->device_ops->control(device, AT_DEVICE_CTRL_SLEEP, RT_NULL);
                result_str = (result == RT_EOK) ? "success" : "failure";
                rt_kprintf("%s device sleep %s \n", device->name, result_str);
            }
            else if (!strcmp(argv[1], "wakeup"))
            {
                result = device->class->device_ops->control(device, AT_DEVICE_CTRL_WAKEUP, RT_NULL);
                result_str = (result == RT_EOK) ? "success" : "failure";
                rt_kprintf("%s device wake up %s \n", device->name, result_str);
            }
            else
            {
                rt_kprintf("Unknown command. Please enter 'at_device' for help\n");
            }
        }
    }
    else
    {
        rt_kprintf("\nUsage: \n");
        rt_kprintf("at_device probe <at_devive_name>    - probe at_device by name\n");
        rt_kprintf("at_device init                      - at_device initialization \n");
        rt_kprintf("at_device deinit                    - at_device desinitialization \n");
        rt_kprintf("at_device sleep                     - enable at_device entry into sleep \n");
        rt_kprintf("at_device wakeup                    - wake up at_device from sleep\n");
        result = -RT_ERROR;
    }
    return RT_EOK;
}
MSH_CMD_EXPORT_ALIAS(at_device_test, at_device, test at_device function);




/********************************************************************************
 * @file    ads1115.h
 * @brief
 *
 * @author  Ajay Kandagal <ajka9053@colorado.edu>
 * @date    29th Apr 2023
 * 
 * @ref     EmbeTronicX
 *          https://embetronicx.com/tutorials/linux/device-drivers/i2c-linux-device-
 *          driver-using-raspberry-pi/
 * 
 *          https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver/
 *          I2C-Linux-Device-Driver/I2C-Client-Driver
 ********************************************************************************/
#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/i2c.h>

//Remove below comment to enable debug
#define AESD_DEBUG 0

#undef PDEBUG
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

/*** ADS1115 Device ***/
#define I2C_BUS_AVAILABLE   (1)
#define SLAVE_DEVICE_NAME   ("ads1115")
#define ADS1115_SLAVE_ADDR  (0x48)

/*** ADS1115 Registers ***/
#define CONVERSION_REG      0x00
#define CONFIG_REG          0x01
#define CONFIG_REG_OS       0x80
#define CONFIG_REG_MUX_0    0x40
#define CONFIG_REG_MUX_1    0x50
#define CONFIG_REG_MUX_2    0x60
#define CONFIG_REG_MUX_3    0x70

typedef union {
    struct
    {
        uint8_t comp_que : 2;
        uint8_t comp_lat : 1;
        uint8_t comp_mode : 1;
        uint8_t comp_pol : 1;
        uint8_t dr : 3;
        uint8_t mode : 1;
        uint8_t pga : 3;
        uint8_t mux : 3;
        uint8_t os : 1;
    };
    uint16_t raw;
} ads1115_config;

struct ads1115_dev_t
{
    struct i2c_adapter *ads_i2c_adpater;
    struct i2c_client  *ads_i2c_client;
    struct cdev cdev;
    uint8_t mux_index;
};


#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */
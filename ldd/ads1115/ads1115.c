/********************************************************************************
 * @file    ads1115.c
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
#include "ads1115.h"

const uint8_t MUX_BY_CHANNEL[] = 
{
    CONFIG_REG_MUX_0,
    CONFIG_REG_MUX_1,
    CONFIG_REG_MUX_2,
    CONFIG_REG_MUX_3
};

static int ads1115_major = 0;
static int ads1115_minor = 0;

struct ads1115_dev_t ads1115_dev;

static struct i2c_board_info ads1115_i2c_board_info =
{
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, ADS1115_SLAVE_ADDR)
};

static const struct i2c_device_id ads1115_id[] = 
{
        { SLAVE_DEVICE_NAME, 0 },
        { }
};

MODULE_DEVICE_TABLE(i2c, ads1115_id);

int ads1115_open(struct inode *inode, struct file *filp)
{
    struct ads1115_dev_t *dev;

    PDEBUG("ads1115 open");

    if (filp->private_data == NULL)
    {
        dev = container_of(inode->i_cdev, struct ads1115_dev_t, cdev);
        filp->private_data = dev;
    }

    return 0;
}

int ads1115_release(struct inode *inode, struct file *filp)
{
    PDEBUG("ads1115 release");
    return 0;
}

ssize_t ads1115_read(struct file *filp, char __user *buf, size_t count,
                  loff_t *f_pos)
{
    struct ads1115_dev_t *dev = filp->private_data;
    uint16_t data;
    uint8_t buffer[3];

    buffer[0] = CONVERSION_REG;
    buffer[1] = 0x00;
    buffer[2] = 0x00;

    if (i2c_master_send(dev->ads_i2c_client, buffer, 1) < 1)
    {
        printk(KERN_ERR "ads1115: failed to start read");
    }

    if (i2c_master_recv(dev->ads_i2c_client, &buffer[1], 2) < 2)
    {
        printk(KERN_ERR "ads1115: failed to send read command");
    }
        
    data = (((uint16_t)buffer[1]) << 8) | buffer[2];

    if (copy_to_user((void *)buf, (void *)&data, 2))
    {
        return -EFAULT;
    }

    PDEBUG("ads1115 data: [%u] %u", dev->mux_index, data);

    return 0;
}

ssize_t ads1115_write(struct file *filp, const char __user *buf, size_t count,
                   loff_t *f_pos)
{
    struct ads1115_dev_t *dev = filp->private_data;
    uint8_t buffer[3];
    ads1115_config m_con;

    PDEBUG("write called");

    if (copy_from_user((void *)(&dev->mux_index), buf, 1))
    {
        return -EFAULT;
    }

    dev->mux_index = dev->mux_index - '0';
    m_con.raw = 0;
    m_con.os = 0;
    m_con.mux = 0x00;
    m_con.pga = 1;
    m_con.mode = 0;
    m_con.dr = 5;

    buffer[0] = CONFIG_REG;
    buffer[1] = (m_con.raw >> 8)  | MUX_BY_CHANNEL[dev->mux_index];
    buffer[2] = (m_con.raw & 0xff);

    if (i2c_master_send(dev->ads_i2c_client, buffer, 3) < 3)
    {
        printk(KERN_ERR "ads1115: Could not set mux change");
    }
    else
    {
        PDEBUG("Sucessfully set mux index %u", dev->mux_index);
    }

    return count;
}

struct file_operations ads1115_fops = 
{
    .owner = THIS_MODULE,
    .read = ads1115_read,
    .write = ads1115_write,
    .open = ads1115_open,
    .release = ads1115_release
};

static struct i2c_driver ads1115_driver =
{
    .driver = 
    {
        .name   = SLAVE_DEVICE_NAME,
        .owner  = THIS_MODULE,
    }
};

static int ads1115_setup_cdev(struct ads1115_dev_t *dev)
{
    int err, devno = MKDEV(ads1115_major, ads1115_minor);

    cdev_init(&dev->cdev, &ads1115_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &ads1115_fops;
    err = cdev_add(&dev->cdev, devno, 1);

    if (err)
        printk(KERN_ERR "ads1115: Error adding aesd cdev %d", err);

    return err;
}

int ads1115_init(void)
{
    dev_t dev = 0;
    int result;

    PDEBUG("loading ads1115 module");

    result = alloc_chrdev_region(&dev, ads1115_minor, 1, "ads1115");
    ads1115_major = MAJOR(dev);

    if (result < 0)
    {
        printk(KERN_ERR "ads1115: [Error] getting major %d\n", ads1115_major);
        return result;
    }

    memset(&ads1115_dev, 0, sizeof(struct ads1115_dev_t));

    result = ads1115_setup_cdev(&ads1115_dev);

    if (result)
    {
        unregister_chrdev_region(dev, 1);
    }

    /*** I2C initialization ***/
    ads1115_dev.ads_i2c_adpater = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if(ads1115_dev.ads_i2c_adpater == NULL)
    {
        printk(KERN_ERR "ads1115: [Error] getting i2c adapter");
        goto fail_exit;
    }

    ads1115_dev.ads_i2c_client = i2c_new_client_device(ads1115_dev.ads_i2c_adpater, &ads1115_i2c_board_info);
        
    if(ads1115_dev.ads_i2c_client == NULL)
    {
        printk(KERN_ERR "ads1115: [Error] creating ads1115 i2c client");
        goto fail_exit;
    }

    i2c_add_driver(&ads1115_driver);

    printk(KERN_NOTICE "ads1115: loaded ads1115 module %u.%u!", ads1115_major, ads1115_minor);
    return result;

fail_exit:
    printk(KERN_ERR "ads1115: failed to load ads1115 module!");
    return result;
}

void ads1115_exit(void)
{
    dev_t devno = MKDEV(ads1115_major, ads1115_minor);

    unregister_chrdev_region(devno, 1);
    cdev_del(&ads1115_dev.cdev);

    if (ads1115_dev.ads_i2c_client != NULL)
    {
        i2c_unregister_device(ads1115_dev.ads_i2c_client);
        i2c_del_driver(&ads1115_driver);
    }
}

module_init(ads1115_init);
module_exit(ads1115_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajay Kandagal <ajka9053@colorado.edu>");
MODULE_DESCRIPTION("I2C driver for ADS1115");
MODULE_VERSION("1.0");
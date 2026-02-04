#include "asm-generic/gpio.h"
#include "asm/uaccess.h"
#include <linux/module.h>
#include <linux/poll.h>
/*  */
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/fcntl.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include <linux/of_irq.h>
#include <linux/interrupt.h>

#define LOGDEV_CNT 1
#define LOGDEV_NAME "ap3216c"
struct LOGDEV{
    dev_t devid;
    int major;
    int minor;

    struct cdev cdev;
    struct class* class;
    struct device* device;

    struct i2c_client* client;
   
};

struct LOGDEV logdev;
static const struct i2c_device_id pony_temp_id[] = {
    { "pony,lp", 0 }, /* 名字要和设备树 compatible 后半部分一致 */
    { }
};

static const struct of_device_id pony_temp_of_match[] = {
    { .compatible = "pony,lp" },
    { /*  */ }
};

static int logdev_open(struct inode* inode,struct file* filp){
    struct LOGDEV* dev = container_of(inode->i_cdev, struct LOGDEV, cdev);
    
    filp->private_data = dev;
    return 0;
}



static int logdev_release(struct inode* inode, struct file* filp){
    return 0;        
   
}
static ssize_t logdev_read(struct file* filp, char __user* buffer, size_t cnt, loff_t* off){
    struct LOGDEV* dev = (struct LOGDEV*)filp->private_data;
    unsigned char reg_addrs[] = {0x0A, 0x0C, 0x0E}; // IR, ALS, PS 的起始地址
    unsigned char temp_data[2];
    short xyz[3];
    int i;

    for (i = 0; i < 3; i++) {
        struct i2c_msg msg[2];
        
        // 1. 写寄存器地址
        msg[0].addr = dev->client->addr;
        msg[0].flags = 0;
        msg[0].len = 1;
        msg[0].buf = &reg_addrs[i];

        // 2. 读 2 字节数据
        msg[1].addr = dev->client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 2;
        msg[1].buf = temp_data;

        // 执行单次转换
        if (i2c_transfer(dev->client->adapter, msg, 2) != 2) {
            return -EIO;
        }

        // 组合高低 8 位
        xyz[i] = (short)((temp_data[1] << 8) | temp_data[0]);
    }

    // 拷贝 6 字节到用户空间
    if (copy_to_user(buffer, xyz, sizeof(xyz)))
        return -EFAULT;

    return sizeof(xyz);
}
static const struct file_operations logdev_flops={
    .owner = THIS_MODULE,
    .open = logdev_open,
    .release = logdev_release,
    .read = logdev_read,
};

static int pony_probe(struct i2c_client* client, const struct i2c_device_id*id){
    //dev_info(&client->dev,"address:%x\r\n",client->addr);
    int ret = 0;
    logdev.major = 0;
    if(logdev.major){
        logdev.devid = MKDEV(logdev.major,0);
        register_chrdev_region(logdev.devid,LOGDEV_CNT,LOGDEV_NAME);
    }else{
        alloc_chrdev_region(&logdev.devid,0,LOGDEV_CNT,LOGDEV_NAME);
    }

    logdev.cdev.owner = THIS_MODULE;
    cdev_init(&logdev.cdev,&logdev_flops);
    cdev_add(&logdev.cdev,logdev.devid,LOGDEV_CNT);

    logdev.class = class_create(THIS_MODULE,"i2cs");

    logdev.device = device_create(logdev.class,NULL,logdev.devid,NULL,LOGDEV_NAME);
    
    logdev.client = client;
    i2c_smbus_write_byte_data(client,0x00,0x03);
    return 0;
}

static int pony_remove(struct i2c_client* client){
    
    device_destroy(logdev.class,logdev.devid);
    class_destroy(logdev.class);
    cdev_del(&logdev.cdev);
    unregister_chrdev_region(logdev.devid,LOGDEV_CNT);
    return 0;
}
static struct i2c_driver pony_temp_driver={
    .driver={
        .name = "pony_temp",
        .of_match_table = pony_temp_of_match,
    },
    .probe = pony_probe,
    .remove = pony_remove,
    .id_table = pony_temp_id, /* 2. 把这个表挂上去 */
};
module_i2c_driver(pony_temp_driver);
MODULE_LICENSE("GPL");
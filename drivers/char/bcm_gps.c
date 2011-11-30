/*****************************************************************************
* Copyright 2008 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/* This driver is now applied to BCM911160 and several changes have been made to
 * improve the readability and functionality of GPS
 */
/* This driver is based on bcm4760 GPS driver and include below all the fixes applied.
 * Different from BCM4760, BCM11211 have external UART pin attached to 4751 modules.
 * the power of the GPS is control through GPIO lines.
 */
/* linux/drivers/char/gps.c
 *
 * Implementation of the GPS driver.
 *
 * Changelog:
 *
 * 19-Feb-2009  KOM  Initial version for refs #811:
 *
 * 22-Feb-2009  KOM  GPS initialization modified.
 *                 ADDED::
 *                    - 02.001: Call gps_reset() and gps_power(0) added in gps_mod_init()
 *                 FIXME::
 *                    - 03.001: No print if KERN_WARNING is being used in printk()
 *                    - 03.002: Using gps_power(0) instead of gps_power(1) because serial driver
 *                              puts extra 0 during identification interruput in amba-pl011.c::pl011_startup()
 * 25-Mar-2009  KOM  Startup BUGFIXED in gps_mod_init() and gps_mod_exit(). FIXME Need fix in in amba-pl011.c.
 *                   ADDED in gps_mod_init()::
 *                    - gps_reset() added firsttime after power on otherwise nRESET deasserted
 *                    - After delay udelay(156) (5 RTS clks) nSTANDBY deasserted gps_power(1) for 80ms added
 *                    - nSTANDBY asserted gps_power(0) added because ser.driver puts extra 0 during identification interruput
 *                   ADDED in gps_mod_exit()::
 *                    - nSTANDBY asserted gps_power(0) added
 * 25-Mar-2009  KOM  FIXME: Temporary fix. Need fix Startup bug in amba-pl011.c.
 *                   Apparently optimization is wrong therefore I am using external IO functions __read...() and __write...() from gps.c.
 *                   I wil debug it.
 * 27-Apr-2009  KOM  - Startup BUGFIXED: mdelay(80) added IOW_GPS_ON:gps_power(1) and IOW_GPS_OFF:after gps_power(0)
 *                   - Unlock added before Deactivating GPS_RESET in gps_mod_init()
 * 05-May-2009  KOM  - udelay(156) added instead of mdelay(80) in IOW_GPS_OFF:after gps_power(0).
 *                   - PK_WARN and PK_DBG are nothing now.
 *                   - Using volatile xxx_save variables for I/O to/from CMU and PML blocks in gps_power(), gps_reset() and
 *                     gps_passthrough_mode() added. Startup problem should be fixed in gps_power(). I think so.
 *
 * 12-08-2009   JLH  Modified name of pl011_serial_get_port_info to bcm4760_uart_serial_get_port_info for new serial driver.
 */

/* Includes */ 
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/fs.h>
//#include <linux/sysrq.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/version.h>

#include <linux/major.h>
#include <asm/uaccess.h>
#include <asm/ioctls.h>

#include <linux/delay.h>

#include <linux/kernel.h>
#include <asm/gpio.h>

#include <linux/broadcom/gps.h>

#include <asm/io.h>
#include <asm/sizes.h>

#include <linux/platform_device.h>

/* Defines */
#define PFX "gps: "

#define PK_DBG_NONE(fmt, arg...)     do {} while (0)

//#define PK_DBG(fmt, arg...)       printk(KERN_DEBUG PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_DBG(fmt, arg...)         do {} while (0)
#define PK_ERR(fmt, arg...)               printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
//KOM FIXME 03.001:: No print if KERN_WARNING is being used
//#define PK_WARN(fmt, arg...)          printk(KERN_WARNING PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_WARN(fmt, arg...)          do {} while (0)
#define PK_NOTICE(fmt, arg...)          printk(KERN_NOTICE PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_INFO(fmt, arg...)          printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)

//#define dbg(x...) printk(KERN_EMERG PFX x)
//#define dbg(x...) printk(KERN_DEBUG PFX x)
//#define dbg(fmt, arg...) printk(KERN_EMERG PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define dbg(fmt, arg...) do {} while (0)

// define in linux_broadcom_gps_setting.h
//#define GPS_nRESET          GPIO59_KEY_OUT7_ETM14_USB1_PWRON
//#define GPS_nSTANDBY     GPIO25_SPI_SS0_B

static struct gps_platform_data *gpt_gps_platform_data = NULL;

#if 0
static void __iomem * gpio_base;

#define GPIO_DATA_L_OFFSET               0x0000  //gpio0-31
#define GPIO_DATA_H_OFFSET               0x0800     //gpio32-63

#define     gpio_read(offset)               readl(gpio_base+(offset))
#define gpio_write(offset, value)     writel((value), gpio_base+(offset))


//extern int bcm4760_uart_serial_get_port_info(int line,char *szBuf);


//KOM FIXME: __attribute__ ((noinline)) forces compilation error
//void gps_reset(void) __attribute__ ((noinline));
//static void gps_passthrough_mode(unsigned on) __attribute__ ((noinline));
//void gps_power(unsigned on) __attribute__ ((noinline));
extern void gps_power(unsigned on);
extern void gps_reset(void);
extern void gps_exit(void);

void gpio_set_pin(int gpio)
{
    volatile uint32_t value;
    int shift = ((gpio > 31)? (gpio - 32): gpio);
    int offset = ((gpio > 31) ? GPIO_DATA_H_OFFSET: GPIO_DATA_L_OFFSET)
    value = gpio_read(offset);
    if (value)
        value |= (1 << shift);
    gps_write(value, offset);
}

void gpio_clear_pin(int gpio)
{
    volatile uint32_t value;
    int shift = ((gpio > 31)? (gpio - 32): gpio);
    int offset = ((gpio > 31) ? GPIO_DATA_H_OFFSET: GPIO_DATA_L_OFFSET)
    value = gpio_read(offset);
    if (value)
        value &= ~(1 << shift);
    gps_write(value, offset);
}
#else
#define gpio_set_pin(gpio)     gpio_set_value(gpio, 1)
#define gpio_clear_pin(gpio)     gpio_set_value(gpio, 0)
#endif

/*   This function turns Standby GPS device on/off
 * Input:
 *       unsigned on - 1 is Standby GPS device off, 0 - on
 * Output:
 *       None
 * References:
 *       gps_ioctl, gps_mod_init
 * Global:
 *       pml_block_sw_isolate_save, cmu_block_bus_clk_stop_save
 * History :
 *       KOM::19-Feb-2009 Initial Version
 *       KOM::05-May-2009 Using volatile xxx_save for I/O CMU and PML blocks
 *                        Startup problem should be fixed in gps_power(). I think so.
 *                        Look at explanation below.
 */
void gps_power(struct gps_platform_data *pt_gps_platform_data, unsigned on)
{
    PK_DBG("Standby GPS device %s\n", on==0 ? "on" : "off");

    if (on)
    {
        gpio_set_pin(pt_gps_platform_data->gpio_power);
    }
    else
    {
        gpio_clear_pin(pt_gps_platform_data->gpio_power);
    }

    printk("gps_power(%d) after return nSTANDBY=%d\n", on, gpio_get_value(pt_gps_platform_data->gpio_power));
}

/*   This function resets GPS device
 * Input:
 *       None
 * Output:
 *       None
 * References:
 *       gps_ioctl, gps_mod_init
 * Global:
 *       cmu_block_reset1_save, unlock_gps_save
 * History :
 *       KOM::19-Feb-2009 Initial Version
 *       KOM::05-May-2009 Using volatile xxx_save for I/O CMU and PML blocks
 *                        Startup problem should be fixed in gps_power(). Look at explanation above.
 */

void gps_reset(struct gps_platform_data *pt_gps_platform_data)
{

   PK_DBG("Resetting GPS device\n");

    /* Pull down reset pin. */
    gpio_clear_pin(pt_gps_platform_data->gpio_reset);

    printk("gps_reset before return nRESET=%d\n", gpio_get_value(pt_gps_platform_data->gpio_reset));
    mdelay(50);

    /* Pull up reset pin. */
    gpio_set_pin(pt_gps_platform_data->gpio_reset);

    printk("gps_reset after return nRESET=%d\n", gpio_get_value(pt_gps_platform_data->gpio_reset));
}


/*   This function turns passthrough mode on/off for GPS device
 * Input:
 *       unsigned on - 1 is turn on, 0 - off
 * Output:
 *       None
 * References:
 *       gps_ioctl
 * Global:
 *       configure_gps_uart_save
 * History :
 *       KOM::19-Feb-2009 Initial Version
 *       KOM::05-May-2009 Using volatile xxx_save for I/O CMU and PML blocks
 *                        Startup problem should be fixed in gps_power(). Look at explanation above.
 */

static void gps_passthrough_mode(unsigned on)
{
    PK_DBG("Passthrough GPS/UART1 %s not implemented\n", on ? "on" : "off");
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int gps_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#else
static long gps_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    int ret = 0;
    //unsigned long flags;

    switch (cmd)
    {
        case IOW_GPS_ON:
            PK_WARN("WARNING: Not Ignoring IOW_GPS_ON\n");
            printk("GPS IOW_GPS_ON called\n");
            gps_power(gpt_gps_platform_data, 1);
            mdelay(80);
            break;
        case IOW_GPS_OFF:
            PK_WARN("WARNING: Not Ignoring IOW_GPS_OFF\n");
            printk("GPS IOW_GPS_OFF called\n");
            gps_power(gpt_gps_platform_data, 0);
            udelay(156);
            break;
        case IOW_GPS_RESET:
            printk("GPS IOW_GPS_RESET called\n");
            PK_WARN("WARNING: Not Ignoring IOW_GPS_RESET\n");
            gps_reset(gpt_gps_platform_data);
            break;
        case IOW_GPS_PASSTHROUGH_MODE_ON:
            PK_WARN("WARNING: Not Ignoring IOW_GPS_PASSTHROUGH_MODE_ON\n");
            printk("GPS IOW_GPS_PASSTHROUGH_MODE_ON called\n");
            gps_passthrough_mode(1);
            break;
        case IOW_GPS_PASSTHROUGH_MODE_OFF:
            printk("GPS IOW_GPS_PASSTHROUGH_MODE_OFF called\n");
            PK_WARN("WARNING: Not Ignoring IOW_GPS_PASSTHROUGH_MODE_OFF\n");
            gps_passthrough_mode(0);
            break;
        case IOR_GET_SERIAL_PORT_INFO:
            PK_ERR("IOR_GET_SERIAL_PORT_INFO not supported\n");
            ret = -EOPNOTSUPP;
            break;

        default:
            PK_ERR("Invalid ioctl command %u\n", cmd);
            ret = -EINVAL;
            break;
    }

    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
#   ifdef CONFIG_COMPAT
static long gps_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return gps_ioctl( file,
                      cmd,
                      (unsigned long)compat_ptr(arg) );
}
#   endif
#endif

static int gps_open(struct inode *inode, struct file *file)
{
    return nonseekable_open(inode, file);
}

static int gps_release(struct inode *inode, struct file *file)
{
    return 0;
}

/* Kernel interface */
static struct file_operations gps_fops = {
    .owner          = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    .ioctl          = gps_ioctl,
#else
    .unlocked_ioctl          = gps_ioctl,
#   ifdef CONFIG_COMPAT
    .compat_ioctl          = gps_compat_ioctl,
#   endif
#endif
    .open          = gps_open,
    .release     = gps_release,
};

static  char     gBanner[] __initdata = KERN_INFO "Broadcom GPS Driver: 1.03, June 27, 2011\n";

static  dev_t           gGpsDrvDevNum;
static  struct class   *gGpsDrvClass = NULL;
static  struct  cdev    gGpsDrvCDev;

static int setup_gpios(struct gps_platform_data *pt_platform_data)
{
   int rc_req, rc_dir;

    if (pt_platform_data == NULL)
    {
        return -1;
    }

    if (pt_platform_data->gpio_reset >= 0)
    {
        rc_req = gpio_request(pt_platform_data->gpio_reset, "gps reset");
        rc_dir = gpio_direction_output(pt_platform_data->gpio_reset, 1);

        if (rc_req != 0 || rc_dir != 0)
        {
            gpio_free(pt_platform_data->gpio_reset);
            return -1;
        }
    }

    if (pt_platform_data->gpio_power >= 0)
    {
        rc_req = gpio_request(pt_platform_data->gpio_power, "gps power");
        rc_dir = gpio_direction_output(pt_platform_data->gpio_power, 1);

        if (rc_req != 0 || rc_dir != 0)
        {
            gpio_free(pt_platform_data->gpio_reset);
            gpio_free(pt_platform_data->gpio_power);
            return -1;
        }
    }

    if (pt_platform_data->gpio_interrupt >= 0)
    {
        printk("gps.c %s() interrupt not supported\n", __FUNCTION__);
    }

    return 0;
}

static void free_gpios(struct gps_platform_data *pt_platform_data)
{
    gpio_free(pt_platform_data->gpio_reset);
    gpio_free(pt_platform_data->gpio_power);
}

#ifdef CONFIG_PM
static int gps_suspend(struct platform_device *dev, pm_message_t state)
{
    struct gps_platform_data *pt_gps_platform_data  = (struct gps_platform_data *)dev->dev.platform_data;

    printk("Broadcom GPS suspend\n");
    gps_power(pt_gps_platform_data, 0);
    return 0;
}
static int gps_resume(struct platform_device *dev)
{
    struct gps_platform_data *pt_gps_platform_data  = (struct gps_platform_data *)dev->dev.platform_data;

    printk("Broadcom GPS resume\n");
    gps_power(pt_gps_platform_data,1);
    mdelay(80);                         // allow 80ms for ASIC to come up
    gps_power(pt_gps_platform_data,0);                         // turn off  by default

    return 0;
}
#else
#define gps_suspend     NULL
#define gps_resume     NULL
#endif

void gps_exit(struct gps_platform_data *pt_gps_platform_data)
{
    printk("Broadcom GPS exit\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
    device_destroy( gGpsDrvClass, gGpsDrvDevNum );
#else
    class_device_destroy( gGpsDrvClass, gGpsDrvDevNum );
#endif
    class_destroy( gGpsDrvClass );

    gps_power(pt_gps_platform_data, 0);
    free_gpios(pt_gps_platform_data);

    cdev_del( &gGpsDrvCDev );

    unregister_chrdev_region( gGpsDrvDevNum, 1 );

    return;
}
static void gps_shutdown(struct platform_device *dev)
{
    struct gps_platform_data *pt_gps_platform_data  = (struct gps_platform_data *)dev->dev.platform_data;

    printk("Broadcom GPS shutdown\n");
    gps_exit(pt_gps_platform_data);
    return;
}

static int __devexit gps_remove(struct platform_device *dev)
{
   struct gps_platform_data *pt_gps_platform_data = (struct gps_platform_data *)dev->dev.platform_data;

    // remove the gps by power off the pin
    printk("Broadcom GPS remove\n");
    gps_power(pt_gps_platform_data, 0);
    return 0;
}

/*  The implementation is for SYSFS: start 
 *  the path would be /sys/devices/platform/gps/nSTDBY & /sys/devices/platform/gps/nRST 
 *  functions store_gps_nXXX is to set the corresponding nXXX GPIO pin to high
 *  functions show_gps_nXXX is to read the corresponding nXXX GPIO pin value
 */
static ssize_t store_gps_nstdby(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct gps_platform_data *pt_gps_platform_data = (struct gps_platform_data *)dev->platform_data;
    int n;

    if (count <= 0)
        return 0;

    sscanf(buf, "%d", &n);
    gps_power(pt_gps_platform_data, n == 0? 0: 1);

    return count;
}

static ssize_t show_gps_nstdby(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct gps_platform_data *pt_gps_platform_data = (struct gps_platform_data *)dev->platform_data;

    return (sprintf(buf, "%d\n", gpio_get_value(pt_gps_platform_data->gpio_power)));
}

static DEVICE_ATTR(nSTDBY, 0644, show_gps_nstdby, store_gps_nstdby);

static ssize_t store_gps_nrst(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct gps_platform_data *pt_gps_platform_data = (struct gps_platform_data *)dev->platform_data;
    int n;

    if (count <= 0)
        return 0;

    sscanf(buf, "%d", &n);
    printk("store_gps_nrst GPIO %d n=%d\n", pt_gps_platform_data->gpio_reset, n);
    if (n == 0)
        gpio_clear_pin(pt_gps_platform_data->gpio_reset);
    else
        gpio_set_pin(pt_gps_platform_data->gpio_reset);

    return count;
}

static ssize_t show_gps_nrst(struct device *dev, struct device_attribute *attr, char *buf)
{
   struct gps_platform_data *pt_gps_platform_data = (struct gps_platform_data *)dev->platform_data;

    return (sprintf(buf, "%d\n", gpio_get_value(pt_gps_platform_data->gpio_reset)));
}

static DEVICE_ATTR(nRST, 0644, show_gps_nrst, store_gps_nrst);

static void make_sysfs_files(struct device *dev)
{
    if (device_create_file(dev, &dev_attr_nSTDBY))
        printk("Could not create sysfs file for card_type\n");
    if (device_create_file(dev, &dev_attr_nRST))
        printk("Could not create sysfs file for open_ttys\n");
}

static void remove_sysfs_files(struct device *dev)
{
    device_remove_file(dev, &dev_attr_nSTDBY);
    device_remove_file(dev, &dev_attr_nRST);
}
/*  The implementation below is for SYSFS: end */

static int __devinit gps_probe(struct platform_device *p_dev)
{
    int     rc = 0;
    static int firsttime = 1;
    struct gps_platform_data *pt_gps_platform_data;

    printk( gBanner );

    if (p_dev->dev.platform_data == NULL)
    {  /* Need this information. */
       printk("%s() error p_dev->dev.platform_data == NULL\n", __FUNCTION__);
       return -1;
    }

    pt_gps_platform_data  = (struct gps_platform_data *)p_dev->dev.platform_data;
    gpt_gps_platform_data = pt_gps_platform_data;

    // dummy implmentation
    make_sysfs_files(&p_dev->dev);

    if ((rc = setup_gpios(pt_gps_platform_data)) != 0)
    {   /* Need this information. */
        printk("%s() error setup_gpios() failed\n", __FUNCTION__);
        return -1;
    }

    if (firsttime)
    {
        firsttime = 0;

        /* bootloader cold boot, reset GPS */
        gps_reset(pt_gps_platform_data);
    }

    udelay(156);                        // t2 (5 RTS clks) = Delay from the time when nRESET is deasserted 
                                        //     to the time when nSTANDBY is deasserted = 1/6 ms
    gps_power(pt_gps_platform_data, 1);
    mdelay(80);                         // allow 80ms for ASIC to come up 

    gps_power(pt_gps_platform_data, 0);  //turn off  by default

    /*
     * KOM FIXME 03.002:: Using gps_power(0) instead of gps_power(1) because serial driver 
     *                   puts extra 0 during identification interruput in amba-pl011.c::pl011_startup()
     * Use the statically assigned major number
    */
    if (( rc = alloc_chrdev_region( &gGpsDrvDevNum, 0, 1, GPS_DEVNAME )) < 0 )
    {
         PK_WARN("gps: Unable to register driver for major; err: %d\n", rc );
         goto free_sysfs_device;
    }

    cdev_init( &gGpsDrvCDev, &gps_fops );
    gGpsDrvCDev.owner = THIS_MODULE;

    if (( rc = cdev_add( &gGpsDrvCDev, gGpsDrvDevNum, 1 )) != 0 )
    {
        PK_WARN("Unable to add driver %d\n", rc );
        goto out_unregister;
    }

    // Now that we've added the device, create a class, so that udev will make the /dev entry

    gGpsDrvClass = class_create( THIS_MODULE, GPS_DEVNAME );
    if ( IS_ERR( gGpsDrvClass ))
    {
        PK_WARN("Unable to create class\n" );
        rc = -1;
        goto out_cdev_del;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
    device_create( gGpsDrvClass, NULL, gGpsDrvDevNum, NULL, GPS_DEVNAME);
#else
    class_device_create( gGpsDrvClass, NULL, gGpsDrvDevNum, NULL, GPS_DEVNAME );
#endif

    printk("Broadcom GPS Probe success\n");
    return 0;

out_cdev_del:
    cdev_del( &gGpsDrvCDev );

out_unregister:
    unregister_chrdev_region( gGpsDrvDevNum, 1 );

free_sysfs_device:
    //remove_sysfs_files(&gps_platform_device->dev);
    remove_sysfs_files(&p_dev->dev);

    free_gpios(pt_gps_platform_data);
    printk("Broadcom GPS Probe failure\n");
    return -1;
}

static struct platform_driver gps_driver = {
    .driver= {
          .name  = "gps",
          .owner = THIS_MODULE,
    },
    .probe      = gps_probe,
    .remove     = __devexit_p(gps_remove),
    .shutdown   = gps_shutdown,
    .suspend    = gps_suspend,
    .resume     = gps_resume,
};


static int __init gps_mod_init(void)
{
    int     rc = 0;

    rc = platform_driver_register(&gps_driver);
    if (rc)
    {
        printk(KERN_ERR "GPS: Unable to platform driver register\n");
        rc = -1;
    }

    printk("GPS init return %d\n", rc);
    return rc;
}

static void __exit gps_mod_exit(void)
{
    gps_exit(gpt_gps_platform_data);
}

module_init(gps_mod_init);
module_exit(gps_mod_exit);

MODULE_AUTHOR("BROADCOM");
MODULE_DESCRIPTION("BROADCOM BCM911160 GPS Driver");
MODULE_LICENSE("GPL");

/* EOF */

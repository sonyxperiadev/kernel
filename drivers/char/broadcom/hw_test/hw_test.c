/*******************************************************************************
Copyright 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/


/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <mach/rdb/brcm_rdb_i2c_mm_hs.h>
#include <linux/io.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <linux/ctype.h>
#include <mach/sdio_platform.h>
#include <linux/clk.h>
#include <plat/clock.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>

typedef enum{
    BSC_SPD_32K = 0,      /*< 32KHZ */
    BSC_SPD_50K,          /*< 50KHZ */
    BSC_SPD_100K,         /*< 100KHZ */
    BSC_SPD_230K,         /*< 230KHZ */
    BSC_SPD_380K,         /*< 380KHZ */
    BSC_SPD_400K,         /*< 400KHZ */
    BSC_SPD_430K,         /*< 430KHZ */
    BSC_SPD_HS,           /*< HIGH SPEED */
    BSC_SPD_100K_FPGA,    /*< 100KHZ based on a 26MHz incoming clock */
    BSC_SPD_400K_FPGA,    /*< 400KHZ based on a 26MHz incoming clock */
    BSC_SPD_HS_FPGA,       /*< HIGH SPEED based on a 26MHz incoming clock */
    BSC_SPD_MAXIMUM       /*< 460K, assume auto sense turn off. */
} BSC_SPEED_t;
/*  
 *  Prototypes - this would normally go in a .h file
 */
static int hw_test_open(struct inode *, struct file *);
static int hw_test_release(struct inode *, struct file *);
static ssize_t hw_test_read(struct file *, char *, size_t, loff_t *);
static ssize_t hw_test_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define HW_TEST_DEV_NAME "hw_testdev"	/* Dev name as it appears in /proc/devices   */

#define HW_TEST_DEBUG
#ifdef HW_TEST_DEBUG
    #define dbg_print(fmt, arg...) \
    printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
    #define dbg_print(fmt, arg...)   do { } while (0)
#endif

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int hw_test_major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  */

#define BSC_WRITE_REG_FIELD(addr,mask,shift,data) (writel((((data << shift) & (mask)) | ( readl(addr) & ~mask ) ) , addr)) 
#define BSC_READ_REG_FIELD(addr,mask,shift)       ((readl(addr) & mask ) >> shift )
#define BSC_WRITE_REG(addr,data)                  (writel(data,addr))
#define BSC_READ_REG(addr)						  (readl(addr))
#define MAX_PROC_NAME_SIZE        15

struct procfs
{
   char name[MAX_PROC_NAME_SIZE];
   struct proc_dir_entry *parent;
};
/*
 * BSC (I2C) private data structure
 */
struct bsc_i2c_dev
{
	struct device *dev;

	/* iomapped base virtual address of the registers */
	void __iomem *virt_base;

	/* I2C bus speed */
	enum bsc_bus_speed speed;

	/* Current I2C bus speed configured */
	enum bsc_bus_speed current_speed;

	/* flag to support dynamic bus speed configuration for multiple slaves */
	int dynamic_speed;

	/* the 8-bit master code (0000 1XXX, 0x08) used for high speed mode */
	unsigned char mastercode;

	/* to save the old BSC TIM register value */
	volatile uint32_t tim_val;

	/* flag to indicate whether the I2C bus is in high speed mode */
	unsigned int high_speed_mode;

	/* IRQ line number */
	int irq;

	/* Linux I2C adapter struct */
	struct i2c_adapter adapter;

	/* lock for data transfer */
	struct semaphore xfer_lock;

	/* to signal the command completion */
	struct completion	ses_done;

	struct procfs proc;

	volatile int debug;

	struct clk *bsc_clk;
	struct clk *bsc_apb_clk;
};

static int __bsc_i2c_get_client(struct device *dev, void *addrp)
{
    struct i2c_client *client = i2c_verify_client(dev);
    int addr = *(int *)addrp;

    if (client && client->addr == addr)
        return true;

    return 0;
}

static struct device *bsc_i2c_get_client(struct i2c_adapter *adapter,
                         int addr)
{
    return device_find_child(&adapter->dev, &addr,
                 __bsc_i2c_get_client);
}

int set_i2c_bus_speed(int bus, enum bsc_bus_speed speed, unsigned short addr)
{
	struct bsc_i2c_dev *dev;
	struct device *d;
	struct i2c_adapter *adap;
	if(bus > 2)
		return -ENODEV;
	adap = i2c_get_adapter(bus);
	if(!adap)
		return -ENODEV;
	dev = i2c_get_adapdata(adap);
	dev->speed = speed;
	d = bsc_i2c_get_client(adap, addr);
	if(d) {
		struct i2c_client *client = NULL;
		struct i2c_slave_platform_data *pd = NULL;
		client = i2c_verify_client(d);
		pd = (struct i2c_slave_platform_data *)client->dev.platform_data;
		if (pd) {
			pd->i2c_speed = speed;
		}
	}
	//dbg_print("i2c bus speed =%d\n", speed);
	  
	return 0;
}
/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int hw_test_open(struct inode *inode, struct file *file)
{

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int hw_test_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t hw_test_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	unsigned int *data, address;
	void  *virt_base;
	int i;
	data = (unsigned int *)buffer;
	address = data[0];
	dbg_print("Register %x \n", address);
	virt_base = ioremap(address, 1024);
	if(!virt_base)
	{
		dbg_print("ioremap of register space failed\n");
		return length;
	}
	for(i=0; i<length; i++)
		data[i]= BSC_READ_REG(virt_base+i*4);  
	iounmap(virt_base);
	return length;
}
/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
hw_test_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	
	void  *virt_base;
	char str[128];
	int   i, ret;
	char *result;
	int  device;
	unsigned int *data, address;
	unsigned long clock_rate;
	struct clk *clk;
	enum bsc_bus_speed speed;
	//dbg_print("command: %s \n", buff);
	if(buff != NULL && !strncmp(buff, "REG", 3))
	{
		data = (unsigned int *)buff;
		address = data[1];
		//dbg_print("Register %x data %x len %d\n", address, data[2], len);
		virt_base = ioremap(address, 1024);
		if(!virt_base)
		{
			dbg_print("ioremap of register space failed\n");
			return len;
		}
		for(i = 0; i<len; i++)
			BSC_WRITE_REG((virt_base+i*4),data[i+2]);
		iounmap(virt_base);
		return len;
	}
	len = strlen(buff);
	for(i = 0; i<len; i++)
		str[i] = toupper(buff[i]);
	result = str;
	while(isspace(*result) && (result != NULL))
		result++;
	if(!strncmp(result, "SDIO", 4) || !strncmp(result, "EMMC", 4))
	{
		if(!strncmp(result, "SDIO", 4))
		{
			clk = clk_get(NULL, SDIO1_PERI_CLK_NAME_STR);
			clock_rate = 48000000;
		}else
		{
			clk = clk_get(NULL, SDIO_DEV_TYPE_SDMMC);
			clock_rate = 52000000;
		}
		result += 4;
		while(isspace(*result) && (*result != 0))
			result++;
		i = simple_strtoul(result, NULL, 0);
		if( i == 0)
		{
			dbg_print("Invalid divisor.\n");
			return len;
		}
		clock_rate /= i;
		ret = clk_set_rate (clk, clock_rate);
		if(ret)
		{
			dbg_print("Set clock rate failed.\n");
			return len;
		}
		ret = clk_enable (clk);
		if(ret)
		{
			dbg_print("Enable clock rate failed.\n");
			return len;
		}
		dbg_print("Set clock rate: %ld\n", clk_get_rate(clk));
		return len;
	}
	if(result == NULL || strncmp(result, "I2C", 3))
	{
		dbg_print("Invalid parameters.\n");
		return len;
	}
	result += 3;
	while(isspace(*result) && (*result != 0))
		result++;
	if(*result == 0 || !isdigit(*result))
	{
		dbg_print("Invalid parameters.\n");
		return len;
	}
	device = *result++ - '0';
	if(device < 0 || device > 2 || !isspace(*result))
	{
		dbg_print("Invalid parameters.\n");
		return len;
	}
	while(isspace(*result) && (*result != 0))
		result++;
	address = simple_strtoul(result, NULL, 16);
	while(!isspace(*result) && (*result != 0))
		result++;
	while(isspace(*result) && (*result != 0))
		result++;
	if(*result == 0)
	{
		dbg_print("Invalid parameters.\n");
		return len;
	} 
	if(!strncmp(result, "400K", 4))
		speed =  BSC_SPD_400K;
	else if(!strncmp(result, "100K", 4))
		speed =  BSC_SPD_100K;
	else
	{
		speed =  BSC_SPD_50K;       /* Default speed */
	}
	/*
	if(speed == BSC_SPD_100K)
		dbg_print("Parameters: device = %x addr = %x speed = 100K \n", device, address);
	else
		dbg_print("Parameters: device = %x addr = %x speed = 400K \n", device, address);
	
	
	virt_base = ioremap(device, 1024);
	if(!virt_base)
	{
		dbg_print("ioremap of register space failed\n");
		return len;
	}
	*/
	set_i2c_bus_speed(device, speed, address);
	//iounmap(virt_base);

	return len;
}

static struct file_operations hw_test_fops =
{
    .open      = hw_test_open,
    .release   = hw_test_release,
    .write     = hw_test_write,
    .read      = hw_test_read,
};


int __init hw_test_init(void)
{
    int ret;

    dbg_print("HW_TEST driver Init\n");

    ret = register_chrdev(0, HW_TEST_DEV_NAME, &hw_test_fops);
    if (ret < 0)
        return -EINVAL;
    else
        hw_test_major = ret;

    dbg_print("HW_TEST driver Major = %d Init successfully\n",hw_test_major);

    return 0;

}

void __exit hw_test_exit(void)
{
    dbg_print("HW_TEST driver Exit\n");
    
   
    unregister_chrdev(hw_test_major, HW_TEST_DEV_NAME);
}

module_init(hw_test_init);
module_exit(hw_test_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("HARDWARE Test device driver");
MODULE_LICENSE("GPL");

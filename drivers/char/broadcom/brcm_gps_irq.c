/*
 * ADXL345/346 Three-Axis Digital Accelerometers
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (C) 2009 Michael Hennerich, Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/broadcom/gps.h>
#include <mach/pinmux.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/jiffies.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/unistd.h>
#include <linux/kthread.h>

/*#define CONFIG_NEW_GPSCHIP_I2C*/
#define GPS_VERSION	"2.00"


#if defined(CONFIG_NEW_GPSCHIP_I2C)
#define TX_SIZE					64
#define I2C_PACKET_SIZE			256
#define RX_SIZE					128
#else
#define TX_SIZE					64
#define I2C_PACKET_SIZE			64
#define RX_SIZE					256
#endif
#define I2C_MAX_SIZE			256
#define RX_BUFFER_LENGTH		16384
#define UDELAY_AFTER_I2C_READ	30

int hostwake_gpio;

struct gps_irq {
	wait_queue_head_t wait;
	int irq;
	int host_req_pin;
	struct miscdevice misc;
	struct i2c_client *client;

	unsigned char wr_buffer[TX_SIZE][I2C_MAX_SIZE];

	int wbuffer_rp;
	int wbuffer_wp;
	int txlength[TX_SIZE];

	unsigned char rd_buffer[RX_SIZE][I2C_PACKET_SIZE];
	unsigned char tmp[RX_BUFFER_LENGTH];

	int rbuffer_rp;
	int rbuffer_wp;
	int rxlength[RX_SIZE];
	int rxlength_rp;
	int rxlength_wp;
	struct work_struct read_task;
	struct work_struct write_task;
};

int cnt;
int cnt2;
int zero_read;
int first_time;

void write_workqueue(struct work_struct *work)
{
	struct gps_irq *ac_data =
		container_of(work, struct gps_irq, write_task);
	int ret;

	--cnt2;
	while (ac_data->wbuffer_rp != ac_data->wbuffer_wp)	{

		ret = i2c_master_send(ac_data->client,
				ac_data->wr_buffer[ac_data->wbuffer_rp],
				ac_data->txlength[ac_data->wbuffer_rp]);

		ac_data->wbuffer_rp = (ac_data->wbuffer_rp+1) & (TX_SIZE-1);
	}
}

void read_new(struct gps_irq *ac_data)
{
	int ret;
	int i;

	unsigned char plen;	  /* packet length */


	while (1) {
		ret = i2c_master_recv(ac_data->client,
			(char *)&ac_data->rd_buffer[ac_data->rbuffer_wp],
			1); /*lets read 1 byte first */

		plen = ac_data->rd_buffer[ac_data->rbuffer_wp][0];
		if (plen == 0)
			break;

		/* printk(KERN_INFO "read %d %",plen); */

		ret = i2c_master_recv(ac_data->client,
			(char *)&ac_data->rd_buffer[ac_data->rbuffer_wp],
			plen+1);
		/* now get rid of the length byte */
		for (i = 0; i < plen; ++i)
			ac_data->rd_buffer[ac_data->rbuffer_wp][i] =
			ac_data->rd_buffer[ac_data->rbuffer_wp][i+1];

		ac_data->rxlength[ac_data->rxlength_wp] = plen;
		ac_data->rbuffer_wp = (ac_data->rbuffer_wp+1)
				& (RX_SIZE-1);
		ac_data->rxlength_wp = (ac_data->rxlength_wp+1)
				& (RX_SIZE-1);

		wake_up_interruptible(&ac_data->wait);
	}
}

#ifdef POLLING
struct task_struct *poll_thread_task;

static int poll_thread(void *data)
{
	struct gps_irq *ac_data = (struct gps_irq *)data;
	while (1) {
		if (kthread_should_stop())
			break;
		read_new(ac_data);
		mdelay(50);
	}
	return 0;
}
#endif

void read_workqueue(struct work_struct *work)
{
	/* printk(KERN_INFO "read_workqueue 1\n"); */
	int i;
	int counter, ret;
	int kk;

	struct gps_irq *ac_data =
		container_of(work, struct gps_irq, read_task);
	--cnt;

#if defined(CONFIG_NEW_GPSCHIP_I2C)
	read_new(ac_data);
#else

	kk = 0;
	counter = 0;
	i = __gpio_get_value(ac_data->host_req_pin);
	if (i == 0)
		return;

	do	{
		ret = i2c_master_recv(ac_data->client,
			(char *)&ac_data->rd_buffer[ac_data->rbuffer_wp],
			I2C_PACKET_SIZE);

		if (ret != I2C_PACKET_SIZE) {
			printk(KERN_INFO "GPS read error\n");
			i = __gpio_get_value(ac_data->host_req_pin);
			continue;
		}

		if (ac_data->rd_buffer[ac_data->rbuffer_wp][0] == 0) {
			++zero_read;
			i = __gpio_get_value(ac_data->host_req_pin);
			continue;
		}

		if (counter < RX_SIZE)	{
			ac_data->rbuffer_wp = (ac_data->rbuffer_wp+1)
				& (RX_SIZE-1);

			if (ac_data->rbuffer_wp == ac_data->rbuffer_rp)
				printk(KERN_INFO "read_workqueue overrun error\n");

			ac_data->rxlength_wp = (ac_data->rxlength_wp+1) & (RX_SIZE-1);

			wake_up_interruptible(&ac_data->wait);
		}

		udelay(UDELAY_AFTER_I2C_READ);
		++counter;

		i = __gpio_get_value(ac_data->host_req_pin);
	}  while (i == 1);
	//printk(KERN_INFO "wake_up_interruptible %d \n",counter);
#endif
}

irqreturn_t gps_irq_handler(int irq, void *dev_id)
{
	struct gps_irq *ac_data = dev_id;

	schedule_work(&ac_data->read_task);
	++cnt;

	return IRQ_HANDLED;
}

static int gps_irq_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	/* This packet enables host req pin */

	struct gps_irq *ac_data = container_of(filp->private_data,
							   struct gps_irq,
							   misc);

	filp->private_data = ac_data;
	ac_data->rbuffer_rp = 0;
	ac_data->rbuffer_wp = 0;
	ac_data->rxlength_rp = 0;
	ac_data->rxlength_wp = 0;

	ac_data->wbuffer_rp = 0;
	ac_data->wbuffer_wp = 0;

	cnt = 0;
	cnt2 = 0;

	zero_read = 0;
	first_time = 1;

#ifdef POLLING
	poll_thread_task = kthread_run(&poll_thread, ac_data, "poll_monitor");
	if ((int)poll_thread_task == -ENOMEM)
		printk(KERN_INFO "gps poll thread is not created\n");
#endif

	return ret;
}

static int gps_irq_release(struct inode *inode, struct file *filp)
{
#ifdef POLLING
	if ((int)poll_thread_task != -ENOMEM)
		kthread_stop(poll_thread_task);
#endif

	printk(KERN_INFO "cnt=%d cnt2=%d zero read=%d\n", cnt, cnt2, zero_read);
	return 0;
}

static unsigned int gps_irq_poll(struct file *filp, poll_table * wait)
{
	struct gps_irq *ac_data = filp->private_data;

	poll_wait(filp, &ac_data->wait, wait);

	if (ac_data->rxlength_wp != ac_data->rxlength_rp)
		return POLLIN | POLLRDNORM;
	return 0;
}

static ssize_t gps_irq_read(struct file *filp,
			    char *buffer, size_t length, loff_t * offset)
{
	struct gps_irq *ac_data = filp->private_data;

	/* printk(KERN_INFO "irq read pointers %d %d",
		ac_data->rxlength_wp,
		ac_data->rxlength_rp); */

#if defined(CONFIG_NEW_GPSCHIP_I2C)
	int l = 0;
	int i, j;
	if (ac_data->rxlength_rp != ac_data->rxlength_wp) {
		i = ac_data->rxlength[ac_data->rxlength_rp];
		if (i>length)
		{
			memcpy(ac_data->tmp+l,
					&ac_data->rd_buffer[ac_data->rbuffer_rp],
						length);
			for (j=0;j<(i-length);++j)
				ac_data->rd_buffer[ac_data->rbuffer_rp][j]=ac_data->rd_buffer[ac_data->rbuffer_rp][j+length];
			l=length;
			ac_data->rxlength[ac_data->rxlength_rp]=i-length;
		}
		else
		{
			memcpy(ac_data->tmp+l,
					&ac_data->rd_buffer[ac_data->rbuffer_rp],
						i);
			l = i;
			ac_data->rbuffer_rp =
				(ac_data->rbuffer_rp+1) & (RX_SIZE-1);
			ac_data->rxlength_rp = (ac_data->rxlength_rp+1) & (RX_SIZE-1);
		}
		if (copy_to_user(buffer, ac_data->tmp, l) != 0)
			return 0;
		return l;
	} else
		return 0;
#else
	if (ac_data->rxlength_rp != ac_data->rxlength_wp) {
		//i = ac_data->rxlength[ac_data->rxlength_rp];

		memcpy(ac_data->tmp,
			&ac_data->rd_buffer[ac_data->rbuffer_rp],
			I2C_PACKET_SIZE);

		ac_data->rbuffer_rp =
			(ac_data->rbuffer_rp+1) & (RX_SIZE-1);
		
		ac_data->rxlength_rp = (ac_data->rxlength_rp+1) & (RX_SIZE-1);

		if (copy_to_user(buffer, ac_data->tmp, I2C_PACKET_SIZE) != 0)
			return 0;
		//printk(KERN_INFO "read2 %d bytes %x",l,ac_data->tmp[0]);
		return I2C_PACKET_SIZE;
	} else
		return 0;
#endif			   
}

static ssize_t gps_irq_write(struct file *filp, const char __user *buffer,
				   size_t length, loff_t *offset)
{
	struct gps_irq *ac_data = filp->private_data;

#if defined(CONFIG_NEW_GPSCHIP_I2C)

	unsigned char test[] = {
		0xfe, 0x00, 0xfd, 0xc0, 0x4c, 0x01, 0x00, 0x00, 0x00, 0xfc};

	if (first_time==1){
		i2c_master_send(ac_data->client, test, 10);
		mdelay(10);
		i2c_master_send(ac_data->client, test, 10);
		mdelay(10);
		first_time = 0;
	}
#endif

	if (length <= I2C_MAX_SIZE) {
		if (((ac_data->wbuffer_wp+1) & (TX_SIZE-1)) ==
			ac_data->wbuffer_rp)
			flush_scheduled_work();

		if (!copy_from_user(&ac_data->wr_buffer[ac_data->wbuffer_wp],
					buffer,
					length)) {
			/* printk(KERN_INFO "gps_irq_write %d %d %d\n",
				length,
				ac_data->wbuffer_rp,ac_data->wbuffer_wp);*/

			ac_data->txlength[ac_data->wbuffer_wp] =
				length;

			ac_data->wbuffer_wp = (ac_data->wbuffer_wp+1)
				& (TX_SIZE-1);

			if (ac_data->wbuffer_wp == ac_data->wbuffer_rp)
				printk(KERN_INFO "gps_irq_write overrun error\n");

			schedule_work(&ac_data->write_task);
			++cnt2;
		} else
			printk(KERN_INFO "gps_irq_write copy error\n");

		return length;
	} else
		return 0;

}

static const struct file_operations gps_irq_fops = {
	.owner = THIS_MODULE,
	.open = gps_irq_open,
	.release = gps_irq_release,
	.poll = gps_irq_poll,
	.read = gps_irq_read,
	.write = gps_irq_write
};

static int gps_hostwake_probe(struct i2c_client *client,
						const struct i2c_device_id *id)
{
	struct gps_platform_data *pdata;
	struct gps_irq *ac_data = kzalloc(sizeof(struct gps_irq), GFP_KERNEL);
	int irq;
	int ret;

	pdata = client->dev.platform_data;
	hostwake_gpio = pdata->gpio_interrupt;

	/* printk(KERN_INFO "GPS IRQ is %d\n", hostwake_gpio); */

	init_waitqueue_head(&ac_data->wait);
	gpio_request(hostwake_gpio, "gps_irq");
	gpio_direction_input(hostwake_gpio);

	irq = gpio_to_irq(hostwake_gpio);
	if (irq < 0) {
		kfree(ac_data);
		return -1;
	}
	ac_data->irq = irq;
	ac_data->host_req_pin = hostwake_gpio;
	ret = request_irq(irq, gps_irq_handler,
		  IRQF_TRIGGER_RISING, "gps_interrupt",
			  ac_data);

	ac_data->client = client;

	ac_data->misc.minor = MISC_DYNAMIC_MINOR;
	ac_data->misc.name = "gps_irq";
	ac_data->misc.fops = &gps_irq_fops;

	ret = misc_register(&ac_data->misc);

	ac_data->rbuffer_rp = 0;
	ac_data->rbuffer_wp = 0;

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	INIT_WORK(&ac_data->read_task, read_workqueue);
	INIT_WORK(&ac_data->write_task, write_workqueue);

	i2c_set_clientdata(client, ac_data);
	return 0;
}

static int gps_hostwake_remove(struct i2c_client *client)
{
	struct gps_platform_data *pdata;
	struct gps_irq *ac_data;

	pdata = client->dev.platform_data;

	ac_data = i2c_get_clientdata(client);
	free_irq(ac_data->irq, ac_data);
	misc_deregister(&ac_data->misc);
	kfree(ac_data);
	return 0;
}

static const struct i2c_device_id gpsi2c_id[] = {
	{"gpsi2c", 0},
	{}
};

static struct i2c_driver gps_driver = {
	.id_table = gpsi2c_id,
	.probe = gps_hostwake_probe,
	.remove = gps_hostwake_remove,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "gps-i2c",
		   },
};

static int gps_irq_init(void)
{
	printk(KERN_INFO "Generic GPS IRQ Driver v%s\n", GPS_VERSION);
	return i2c_add_driver(&gps_driver);
}

static void gps_irq_exit(void)
{
	i2c_del_driver(&gps_driver);
}

module_init(gps_irq_init);
module_exit(gps_irq_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Driver for gps host wake interrupt");

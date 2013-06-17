/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/mfd/core.h>
#include <linux/irq.h>
#include <asm/gpio.h>
#include <linux/proc_fs.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>


#define IRQ_TO_REG_INX(irq)  ((irq)/8)
#define IRQ_TO_REG_BIT(irq)  ((irq) % 8)

struct bcm590xx *info;

u16 bcm590xx_slave[] = {
	BCM590XX_SLAVE2_I2C_ADDRESS,
};

static int bcm590xx_find_slave(struct bcm590xx *bcm590xx, int slave)
{
	int i;
	//printk("%s: Slave 0x%x\n", __func__, slave);
	for (i = 0; i < BCM590XX_NUM_SLAVES; i++)
		if (bcm590xx->i2c_client[i].addr == slave)
			return i;

	pr_err("%s: wrong slave id %d\n", __func__, slave);
	return -EINVAL;
}

int bcm590xx_reg_read(struct bcm590xx *bcm590xx, int reg)
{
	int err;
	int i=0;
	u8 slave, regAddr;
	slave = BCM590XX_REG_SLAVE(reg);
	regAddr = BCM590XX_REG_DECODE(reg);
	i = bcm590xx_find_slave(bcm590xx, slave);
	if (i < 0)
		return i;
	mutex_lock(&bcm590xx->i2c_rw_lock);
	err = bcm590xx->read_dev(bcm590xx, regAddr, i);
	mutex_unlock(&bcm590xx->i2c_rw_lock);
	return err;

}
EXPORT_SYMBOL_GPL(bcm590xx_reg_read);

int bcm590xx_mul_reg_read(struct bcm590xx *bcm590xx, int reg, u32 length, u8 *val)
{
	int err, i=0;
	u8 slave, regAddr;
	slave = BCM590XX_REG_SLAVE(reg);
	regAddr = BCM590XX_REG_DECODE(reg);
	i = bcm590xx_find_slave(bcm590xx, slave);
	if (i < 0)
		return i;
	mutex_lock(&bcm590xx->i2c_rw_lock);
	err = bcm590xx->mul_read_dev(bcm590xx, regAddr, length, val, i);
	mutex_unlock(&bcm590xx->i2c_rw_lock);
	return err;
}
EXPORT_SYMBOL_GPL(bcm590xx_mul_reg_read);

int bcm590xx_reg_write(struct bcm590xx *bcm590xx, int reg, u16 val)
{
	int ret;
	int i=0;
	u8 slave, regAddr;
	slave = BCM590XX_REG_SLAVE(reg);
	regAddr = BCM590XX_REG_DECODE(reg);
	i = bcm590xx_find_slave(bcm590xx, slave);
	if (i < 0)
		return i;
	mutex_lock(&bcm590xx->i2c_rw_lock);
	ret = bcm590xx->write_dev(bcm590xx, regAddr, 1, val, i);

	if (ret)
		dev_err(bcm590xx->dev, "write to reg R%d failed\n", reg);
	mutex_unlock(&bcm590xx->i2c_rw_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(bcm590xx_reg_write);

int bcm590xx_mul_reg_write(struct bcm590xx *bcm590xx, int reg, u32 length, u8 *val)
{
	int err, i=0;
	u8 slave, regAddr;
	slave = BCM590XX_REG_SLAVE(reg);
	regAddr = BCM590XX_REG_DECODE(reg);
	i = bcm590xx_find_slave(bcm590xx, slave);
	if (i < 0)
		return i;
	mutex_lock(&bcm590xx->i2c_rw_lock);
	err = bcm590xx->mul_write_dev(bcm590xx, regAddr, length, val, i);
	mutex_unlock(&bcm590xx->i2c_rw_lock);
	return err;
}
EXPORT_SYMBOL_GPL(bcm590xx_mul_reg_write);

static	void bcm590xx_rd_cl_dis_intrs( struct bcm590xx *bcm590xx, int set_to_clear)
{
	unsigned int i = 0;
	unsigned int temp = 0;
	pr_debug("inside %s\n", __func__);
	/*Read & clear all interrupts */
	for (i = 0; i < BCM590XX_MAX_INT_REGS; i++) {
		temp = bcm590xx_reg_read(bcm590xx, (BCM590XX_INT_REG_BASE + i));
	}

	/*mask all interrupts */
	for (i = 0; i <= BCM590XX_MAX_INTMASK_REGS; i++) {
		if (set_to_clear)
			bcm590xx_reg_write(bcm590xx, (BCM590XX_INT_MASK_REG_BASE + i ), 0xFF );
		else
			bcm590xx_reg_write(bcm590xx, (BCM590XX_INT_MASK_REG_BASE + i ), 0x00 );
	}
}

static struct bcm_pmu_irq *bcm590xx_find_irq_handler(struct bcm590xx *bcm590xx, int irq)
{
	struct bcm_pmu_irq *p;
	struct bcm_pmu_irq *match = NULL;

	list_for_each_entry(p, &bcm590xx->irq_handlers, node) {
		if (p->irq == irq) {
			match = p;
			break;
		}
	}
	return match;
}

int bcm590xx_disable_irq(struct bcm590xx *bcm590xx, int irq)
{
	int regInx;
	int st;
	u8 reg_val;
	struct bcm_pmu_irq *handler;

	pr_debug(" Inside %s\n", __func__);

	if (irq < 0 || irq > BCM590XX_TOTAL_IRQ)
		return -EINVAL;

	regInx = IRQ_TO_REG_INX(irq);

	st = bcm590xx_reg_read(bcm590xx, regInx + BCM590XX_INT_MASK_REG_BASE);
	if (st < 0) {
		printk("bcm590xx_disable_irq : PMU reg read error !!!\n");
		return st;
	}
	reg_val = st;
	if (BCM590XX_INT_MASK_BIT)
		reg_val |= (1 << IRQ_TO_REG_BIT(irq));
	else
		reg_val &= ~(1 << IRQ_TO_REG_BIT(irq));
	st = bcm590xx_reg_write(bcm590xx, regInx + BCM590XX_INT_MASK_REG_BASE, reg_val);
	if (st < 0) {
		printk( "bcm590xx_disable_irq : PMU reg write error !!!\n");
		return st;
	}

	handler = bcm590xx_find_irq_handler(bcm590xx, irq);
	if (handler) {
		handler->irq_enabled = false;
	}
	return 0;
}

EXPORT_SYMBOL(bcm590xx_disable_irq);

int bcm590xx_enable_irq(struct bcm590xx *bcm590xx, int irq)
{
	int regInx;
	int st;
	u8 reg_val;
	struct bcm_pmu_irq *handler;

	pr_debug(" Inside %s, enabling irq %d \n", __func__, irq );

	if (irq < 0 || irq > BCM590XX_TOTAL_IRQ)
		return -EINVAL;

	regInx = IRQ_TO_REG_INX(irq);

	st = bcm590xx_reg_read(bcm590xx, regInx + BCM590XX_INT_MASK_REG_BASE);
	if (st < 0) {
		printk( "bcm590xx_enable_irq : PMU reg read error !!!\n");
		return st;
	}
	reg_val = st;
	if (BCM590XX_INT_MASK_BIT)
		reg_val &= ~(1 << IRQ_TO_REG_BIT(irq));
	else
		reg_val |= (1 << IRQ_TO_REG_BIT(irq));
	st = bcm590xx_reg_write(bcm590xx, regInx + BCM590XX_INT_MASK_REG_BASE, reg_val);
	if (st < 0) {
		printk( "bcm590xx_enable_irq : PMU reg write error !!!\n");
		return st;
	}

	handler = bcm590xx_find_irq_handler(bcm590xx, irq);
	if (!handler) {
		printk( "bcm590xx_enable_irq : Enabling PMU irq without registering handler!!!\n");
	} else {
		handler->irq_enabled = true;
	}
	return 0;
}
EXPORT_SYMBOL(bcm590xx_enable_irq);

int bcm590xx_request_irq(struct bcm590xx *bcm590xx, int irq, bool enable_irq,
			 void (*handler) (int, void *), void *data)
{
	struct bcm_pmu_irq *irq_info;
	pr_debug(" Inside %s Interrupt no. 0x%x\n", __func__, irq);
	if (irq < 0 || irq >= BCM590XX_TOTAL_IRQ|| !handler)
		return -EINVAL;
	if (WARN_ON(bcm590xx_find_irq_handler(bcm590xx, irq))) {
		printk( " %s: handler for irq : %x already registered !!!\n",
				__func__, irq);
		return -EBUSY;
	}

	irq_info = kzalloc(sizeof(struct bcm_pmu_irq), GFP_KERNEL);
	if (!irq_info)
		return -ENOMEM;

	irq_info->handler = handler;
	irq_info->data = data;
	irq_info->irq_enabled = enable_irq;
	irq_info->irq = irq;

	mutex_lock(&bcm590xx->list_lock);
	list_add(&irq_info->node, &bcm590xx->irq_handlers);
	mutex_unlock(&bcm590xx->list_lock);

	enable_irq ? bcm590xx_enable_irq(bcm590xx, irq) : bcm590xx_disable_irq(bcm590xx, irq);
	return 0;
}
EXPORT_SYMBOL(bcm590xx_request_irq);

int bcm590xx_free_irq(struct bcm590xx *bcm590xx, int irq)
{
	struct bcm_pmu_irq *irq_info;
	pr_debug(" Inside %s\n", __func__);
	if (irq < 0 || irq >= BCM590XX_TOTAL_IRQ)
		return -EINVAL;
	irq_info = bcm590xx_find_irq_handler(bcm590xx, irq);
	if (irq_info) {
		mutex_lock(&bcm590xx->list_lock);
		list_del(&irq_info->node);
		mutex_unlock(&bcm590xx->list_lock);
		kfree(irq_info);
	}

	/* disalbe IRQ as there is no handler */
	bcm590xx_disable_irq(bcm590xx, irq);
	return 0;
}
EXPORT_SYMBOL(bcm590xx_free_irq);

static void bcm590xx_irq_workq(struct work_struct *work)
{
	struct bcm590xx *bcm590xx = container_of(work, struct bcm590xx, work);
	int i;
	u8 intStatus[BCM590XX_MAX_INT_REGS];
	struct bcm_pmu_irq *handler;
	int int_state;
	pr_debug("inside %s\n", __func__);

	do {
		/* Read all interrupt status registers. All interrupt status registers are R&C */
		for (i = 0; i < BCM590XX_MAX_INT_REGS; i++) {
			intStatus[i] = bcm590xx_reg_read(bcm590xx, BCM590XX_INT_REG_BASE + i);
		}

		mutex_lock(&bcm590xx->list_lock);
		list_for_each_entry(handler, &bcm590xx->irq_handlers, node) {
			if (handler->irq_enabled &&
					(intStatus[IRQ_TO_REG_INX(handler->irq)] &
					 (1 << IRQ_TO_REG_BIT(handler->irq)))) {
				handler->handler(handler->irq, handler->data);
			}
		}
		mutex_unlock(&bcm590xx->list_lock);
		int_state = gpio_get_value(irq_to_gpio(bcm590xx->irq));
	} while(int_state == 0);
}

static irqreturn_t pmu_irq_handler(int irq, void *dev_id)
{
	struct bcm590xx *bcm590xx = dev_id;

	pr_debug("inside %s\n", __func__);
	if (queue_work(bcm590xx ->pmu_workqueue, &bcm590xx ->work) == 0) {
		pr_debug("%s: Work previously queued\n", __func__);
	}
	return IRQ_HANDLED;
}

static int bcm590xx_client_dev_register(struct bcm590xx *bcm590xx, const char *name)
{
	struct mfd_cell cell = { };
	pr_debug("inside %s\n", __func__);

	cell.name = name;
	return mfd_add_devices(bcm590xx->dev, -1, &cell, 1, NULL, 0);
}

/***********************PMU PROC DEBUG Interface***************************************/
static int bcm590xx_open(struct inode *inode, struct file *file)
{
	pr_debug("%s\n", __func__);
	file->private_data = PDE(inode)->data;

	return 0;
}

int bcm590xx_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static long bcm590xx_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	/* TODO: */
	pr_debug("inside %s\n", __func__);
	return 0;
}

#define MAX_USER_INPUT_LEN      100
#define MAX_REGS_READ_WRITE     1

enum pmu_debug_ops {
	PMUDBG_READ_REG = 0UL,
	PMUDBG_WRITE_REG,
};

struct pmu_debug {
	int read_write;
	int len;
	int addr;
	u8 val[MAX_REGS_READ_WRITE];
};

static void bcm590xx_dbg_usage(void)
{
	printk(KERN_INFO "Usage:\n");
	printk(KERN_INFO "Read a register: echo 0x0800 > /proc/pmu0\n");
	printk(KERN_INFO
			"Read multiple regs: echo 0x0800 -c 10 > /proc/pmu0\n");
	printk(KERN_INFO
			"Write multiple regs: echo 0x0800 0xFF 0xFF > /proc/pmu0\n");
	printk(KERN_INFO
			"Write single reg: echo 0x0800 0xFF > /proc/pmu0\n");
	printk(KERN_INFO "Max number of regs in single write is :%d\n",
			MAX_REGS_READ_WRITE);
	printk(KERN_INFO "Register address is encoded as follows:\n");
	printk(KERN_INFO "0xSSRR, SS: i2c slave addr, RR: register addr\n");
}

static int bcm590xx_dbg_parse_args(char *cmd, struct pmu_debug *dbg)
{
	char *tok;                 /* used to separate tokens             */
	const char ct[] = " \t";   /* space or tab delimits the tokens    */
	bool count_flag = false;   /* whether -c option is present or not */
	int tok_count = 0;         /* total number of tokens parsed       */
	int i = 0;

	dbg->len        = 0;

	/* parse the input string */
	while ((tok = strsep(&cmd, ct)) != NULL) {
		pr_debug("token: %s\n", tok);

		/* first token is always address */
		if (tok_count == 0) {
			sscanf(tok, "%x", &dbg->addr);
		} else if (strnicmp(tok, "-c", 2) == 0) {
			/* the next token will be number of regs to read */
			tok = strsep(&cmd, ct);
			if (tok == NULL)
				return -EINVAL;

			tok_count++;
			sscanf(tok, "%d", &dbg->len);
			count_flag = true;
			break;
		} else {
			int val;

			/* this is a value to be written to the pmu register */
			sscanf(tok, "%x", &val);
			if (i < MAX_REGS_READ_WRITE) {
				dbg->val[i] = val;
				i++;
			}
		}

		tok_count++;
	}

	/* decide whether it is a read or write operation based on the
	 * value of tok_count and count_flag.
	 * tok_count = 0: no inputs, invalid case.
	 * tok_count = 1: only reg address is given, so do a read.
	 * tok_count > 1, count_flag = false: reg address and atleast one
	 *     value is present, so do a write operation.
	 * tok_count > 1, count_flag = true: to a multiple reg read operation.
	 */
	switch (tok_count) {
		case 0:
			return -EINVAL;
		case 1:
			dbg->read_write = PMUDBG_READ_REG;
			dbg->len = 1;
			break;
		default:
			if (count_flag == true) {
				dbg->read_write = PMUDBG_READ_REG;
			} else {
				dbg->read_write = PMUDBG_WRITE_REG;
				dbg->len = i;
			}
	}

	return 0;
}

static ssize_t bcm590xx_write(struct file *file, const char __user *buffer,
	size_t len, loff_t *offset)
{
	struct bcm590xx *bcm590xx = file->private_data;
	struct pmu_debug dbg;
	char cmd[MAX_USER_INPUT_LEN];
	int ret, i;

	pr_debug("%s\n", __func__);

	if (!bcm590xx) {
		pr_err("%s: driver not initialized\n", __func__);
		return -EINVAL;
	}

	if (len > MAX_USER_INPUT_LEN)
		len = MAX_USER_INPUT_LEN;

	if (copy_from_user(cmd, buffer, len)) {
		pr_err("%s: copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	/* chop of '\n' introduced by echo at the end of the input */
	if (cmd[len - 1] == '\n')
		cmd[len - 1] = '\0';

	if (bcm590xx_dbg_parse_args(cmd, &dbg) < 0) {
		bcm590xx_dbg_usage();
		return -EINVAL;
	}

	pr_debug("operation: %s\n", (dbg.read_write == PMUDBG_READ_REG) ?
			"read" : "write");
	pr_debug("address  : 0x%x\n", dbg.addr);
	pr_debug("length   : %d\n", dbg.len);

	if (dbg.read_write == PMUDBG_READ_REG) {
		ret = bcm590xx_mul_reg_read(bcm590xx, dbg.addr, dbg.len, dbg.val);
		if (ret < 0) {
			pr_err("%s: pmu reg read failed\n", __func__);
			return -EFAULT;
		}

		for (i = 0; i < dbg.len; i++, dbg.addr++)
			printk(KERN_INFO "[%x] = 0x%02x\n", dbg.addr,
					dbg.val[i]);
	} else {
		ret = bcm590xx_mul_reg_write(bcm590xx, dbg.addr, dbg.len,
				dbg.val);
		if (ret < 0) {
			pr_err("%s: pmu reg write failed\n", __func__);
			return -EFAULT;
		}
	}

	*offset += len;

	return len;
}


static const struct file_operations bcm590xx_pmu_ops = {
	.open = bcm590xx_open,
	.unlocked_ioctl = bcm590xx_ioctl,
	.write = bcm590xx_write,
	.release = bcm590xx_release,
	.owner = THIS_MODULE,
};
/****************************************************************************************/
#define DEBUG_ON 	1

int bcm590xx_device_init(struct bcm590xx *bcm590xx, int irq,
		       struct bcm590xx_platform_data *pdata)
{
	int ret, i=1;
	u8 host_ctrl1;

	printk("REG: bcm590xx_device_init called bcm590xx = 0x%x\n", (u32)bcm590xx);
	while (i < BCM590XX_NUM_SLAVES) {
		bcm590xx->i2c_client[i].addr = bcm590xx_slave[i-1];
		bcm590xx->i2c_client[i].client =
			i2c_new_dummy(bcm590xx->i2c_client[0].client->adapter, bcm590xx_slave[i-1]);
		if (!bcm590xx->i2c_client[i].client) {
			pr_err("%s : failed to create new i2c slave \
					with addr 0x%x\n", __func__,
					bcm590xx->i2c_client[i].addr);
		}
		else {
			printk("%s: i2c client(0x%x) registered with slave id 0x%x\n", __func__,
					(unsigned int)bcm590xx->i2c_client[i].client,
					bcm590xx->i2c_client[i].addr );
			/* NOTE: Attach pdata for handling i2c slave speed selection */
			bcm590xx->i2c_client[i].client->dev.platform_data = pdata;
		}
		i++;
	}
	info = bcm590xx;
	INIT_LIST_HEAD(&bcm590xx->irq_handlers);
	INIT_WORK(&bcm590xx->work, bcm590xx_irq_workq);
	mutex_init(&bcm590xx->list_lock);
	mutex_init(&bcm590xx->i2c_rw_lock);
	bcm590xx->irq = irq;
#ifdef DEBUG_ON
	printk("%s: Slave at index 0 0x%x and at index 1 0x%x\n", __func__,
			bcm590xx->i2c_client[0].addr, bcm590xx->i2c_client[1].addr);
#endif
	bcm590xx->pmu_workqueue = create_workqueue("pmu_events");
	if (!bcm590xx->pmu_workqueue) {
		ret = -ENOMEM;
		pr_err("%s: failed to create work queue", __func__);
		goto err_create_workq;
	}

	/* get BCM590XX revision and config mode */
	ret = bcm590xx_reg_read(bcm590xx, BCM590XX_REG_PMUID);
	if (ret < 0) {
		printk(KERN_ERR "Failed to read ID: %d\n", ret);
		goto err;
	}
	else
	{
		printk("BCM590XX: Chip Version [0x%x]\n", ret);
	}

	if (pdata->pmu_event_cb) {
		ret = pdata->pmu_event_cb(BCM590XX_INITIALIZATION, 0);
		if (ret != 0) {
			dev_err(bcm590xx->dev, "Platform init() failed: %d\n",
					ret);
			goto err;
		}
	}
	// Register IRQ.
	if (irq > 0) {
		ret = request_irq(irq, pmu_irq_handler, IRQF_TRIGGER_FALLING|IRQF_NO_SUSPEND,
				"pmu-bcm590xx", bcm590xx);
		if (ret) {
			printk(KERN_ERR "%s can't get IRQ %d, ret %d\n", __func__, irq, ret);
			goto err;
		}
	}

	disable_irq(irq);

	/* Read, clear, and disable all the interrupts. */
	bcm590xx_rd_cl_dis_intrs(bcm590xx, BCM590XX_INT_MASK_BIT);


	/* By default disable the watchdog */
	host_ctrl1 = bcm590xx_reg_read(info, BCM590XX_REG_HOSTCTRL1);
	host_ctrl1 &= ~(1 << HOSTCTRL1_WDOGEN_OFFSET);
	bcm590xx_reg_write(info, BCM590XX_REG_HOSTCTRL1, (u8)host_ctrl1);

	printk("*************  PMU WATCHDOG REG  0x%x \r\n",	bcm590xx_reg_read(info, BCM590XX_REG_HOSTCTRL1));

	/* register proc interface */
	proc_create_data("pmu0", S_IRWXUGO, NULL, &bcm590xx_pmu_ops, bcm590xx);

	enable_irq(irq);

	for (i = 0; i < pdata->clients_num; i++) {
		ret = bcm590xx_client_dev_register(info, pdata->clients[i]);
		if (ret < 0)
			pr_err("failed to add `%s', err=%d\n",
					pdata->clients[i], ret);
	}
	printk("%s: SUCCESS\n", __func__);
	return 0;
err:
	destroy_workqueue(bcm590xx->pmu_workqueue);
err_create_workq:
	mutex_destroy(&bcm590xx->list_lock);
	mutex_destroy(&bcm590xx->i2c_rw_lock);
	i = 1;
	while (i < BCM590XX_NUM_SLAVES) {
		if (bcm590xx->i2c_client[i].client)
			i2c_unregister_device(bcm590xx->i2c_client[i].client);
		i++;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(bcm590xx_device_init);

void bcm590xx_device_exit(struct bcm590xx *bcm590xx)
{
	int i = 1;
	mutex_destroy(&bcm590xx->list_lock);
	mutex_destroy(&bcm590xx->i2c_rw_lock);
	destroy_workqueue(bcm590xx->pmu_workqueue);
	while (i < BCM590XX_NUM_SLAVES) {
		if (bcm590xx->i2c_client[i].client)
			i2c_unregister_device(bcm590xx->i2c_client[i].client);
		i++;
	}
}
EXPORT_SYMBOL_GPL(bcm590xx_device_exit);

void bcm590xx_shutdown(void)
{
	int host_ctrl1;

	host_ctrl1 = bcm590xx_reg_read(info, BCM590XX_REG_HOSTCTRL1);
	host_ctrl1 |= 1 << HOSTCTRL1_SHDWN_OFFSET;
	bcm590xx_reg_write(info, BCM590XX_REG_HOSTCTRL1, (u8)host_ctrl1);
}
EXPORT_SYMBOL_GPL(bcm590xx_shutdown);

int bcm590xx_wake_reason(void)
{
	int reason;
	reason = bcm590xx_reg_read(info, BCM590XX_REG_WAKE_REASON);
	if (reason < 0)
		return reason;
	if (reason & BCM590XX_POK_WAKE)
		return BCM590XX_POK_WAKE;
	if (reason & BCM590XX_CGPD_WAKE)
		return BCM590XX_CGPD_WAKE;
	if (reason & BCM590XX_UBPD_WAKE)
		return BCM590XX_UBPD_WAKE;
	if (reason & BCM590XX_RTC_ALARM_WAKE)
		return BCM590XX_RTC_ALARM_WAKE;
	if (reason & BCM590XX_AUXON_WAKE)
		return BCM590XX_AUXON_WAKE;
	if (reason & BCM590XX_GBAT_PLUGIN)
		return BCM590XX_GBAT_PLUGIN;
	return -EIO;
}

MODULE_DESCRIPTION("BCM590XX PMIC core driver");
MODULE_LICENSE("GPL");

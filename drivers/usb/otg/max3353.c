/*
 * max3353 - Maxim MAX3353 USB OTG Charge Pump with
 *           Switchable Pullup/Pulldown Registers.
 *
 *  General structure is inspired from (ulinux patch):
 *    max3353_mcf532x.c - Andrey Butok
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#include <linux/device.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/workqueue.h>

#include <linux/i2c/max3353.h>

#include <asm/gpio.h>
#include "kona_otg_cp.h"

#define MAX3353_PROC_MAX_BUF_SIZE         	256

struct max3353_procfs
{
   char *name;
   struct proc_dir_entry *parent;
};

struct max3353_device
{
	struct kona_otg_cp otg;
	enum kona_otg_cp_id id;
	enum kona_otg_cp_volt_state volt_state;
	struct max3353_procfs proc;
	struct max3353_platform_data *data;
	struct i2c_client *i2c;
	struct semaphore reg_read_lock;
	struct device *dev;
	struct workqueue_struct *workqueue;
	struct work_struct work_ist;
	struct work_struct work_vbus;
	int mode;
};

#define KLOG_PREFIX			MAX3353_DRIVER_NAME

#define KLOG_ERROR(fmt, args...)	KLOG(KERN_ERR, fmt, ## args)
#define KLOG_WARN(fmt, args...)		KLOG(KERN_WARNING, fmt, ## args)
#define KLOG_NOTICE(fmt, args...)	KLOG(KERN_NOTICE, fmt, ## args)
#define KLOG_INFO(fmt, args...)		KLOG(KERN_INFO, fmt, ## args)

/* #define DEBUG */
/* #define DEBUG_VERBOSE */

#ifdef DEBUG
	/*
	 * Add any normal log messages (syslogd) to the kernel log as well. Makes things easier
	 * to interpret if the syslogd messages are interleaved in the knllog.
	 */
    	#define KDBG(fmt, args...)	/* KNLLOG(fmt, ## args) */
	#define KLOG(lvl, fmt, args...) do \
					{ \
						/* KNLLOG(fmt, ## args);*/ \
						PRINTK(lvl, fmt, ## args); \
					} \
					while(0)
#else
    	#define KDBG(fmt...)		do{}while(0)
	#define KLOG			PRINTK
#endif

#define KDBG_ENTER()			KDBG_TRACE("enter\n")
#define KDBG_EXIT()			KDBG_TRACE("exit\n")

#ifdef DEBUG_VERBOSE
	#define KDBG_REG		KDBG
	#define KDBG_TRACE		KDBG
#else
	#define KDBG_REG(fmt...)	do{}while(0)
	#define KDBG_TRACE(fmt...)	do{}while(0)
#endif

#define PRINTK(lvl, fmt, args...)	printk( lvl KLOG_PREFIX ": %s: " fmt, __FUNCTION__, ## args )

void max3353_state_update(struct max3353_device *max);

/* Only support one device instance at this time */
static struct max3353_device *the_device = NULL;
static const char *max3353_mode_str[] = MAX3353_MODE_STR_INIT;

static const struct i2c_device_id max3353_i2c_id[] = {
	{ MAX3353_DRIVER_NAME, 0 },
	{ }
};

uint8_t max3353_reg_read(struct max3353_device *max, uint8_t reg)
{
	char data;
	int rc;

	/*
	 * Need to have some mutex protection here. It is possible for the
	 * address to get changed by some other register transaction in
	 * between the i2c_master_send() and the i2c_master_recv().
	 * Alternate method is to use i2c_transfer() instead of the
	 * i2c_master_send() / i2c_master_recv(). The i2c_transfer()
	 * will take ownership of the i2c bus until the transfer completes.
	 */
	if (down_interruptible(&max->reg_read_lock) != 0) {
		KLOG_WARN("reg_read_lock interrupted\n");
		return 0; 
	}
	
	data = reg;
	rc = i2c_master_send(max->i2c, &data, 1);
	if (rc <= 0) {
		KLOG_ERROR("i2c_master_send() failed, rc=%d\n", rc);
		up(&max->reg_read_lock);
		return rc;
	}
	rc = i2c_master_recv(max->i2c, &data, 1);
	if (rc <= 0) {
		KLOG_ERROR("i2c_master_recv() failed, rc=%d\n", rc);
		up(&max->reg_read_lock);
		return rc;
	}

	up(&max->reg_read_lock);

	KDBG_REG("reg=0x%02x data=0x%02x\n", reg, data);

	return (uint8_t)data;
}

int max3353_reg_write(struct max3353_device *max, uint8_t reg, uint8_t data)
{
	char buf[2];
	int rc;

	buf[0] = reg;
	buf[1] = data;
 
	rc = i2c_master_send(max->i2c, buf, sizeof(buf));
	if (rc <= 0) {
		KLOG_ERROR("i2c_master_send() failed, rc=%d\n", rc);
		return rc;
	}

	KDBG_REG("reg=0x%02x data=0x%02x\n", reg, data);

	return 0;
}

void max3353_reg_bit_clr(struct max3353_device *max, uint8_t reg, uint8_t data)
{
	max3353_reg_write(max, reg, max3353_reg_read(max, reg) & ~data);
}

void max3353_reg_bit_set(struct max3353_device *max, uint8_t reg, uint8_t data)
{
	max3353_reg_write(max, reg, max3353_reg_read(max, reg) | data);
}

#ifdef DEBUG
void max3353_reg_dump(struct max3353_device *max)
{
	uint8_t tmp0, tmp1, tmp2, tmp3;

	pr_info("max3353 register dump:\n");

	tmp0 = max3353_reg_read(max, MAX3353_MANUFACTURER_0);
	tmp1 = max3353_reg_read(max, MAX3353_MANUFACTURER_1);
	tmp2 = max3353_reg_read(max, MAX3353_MANUFACTURER_2);
	tmp3 = max3353_reg_read(max, MAX3353_MANUFACTURER_3);
	pr_info("\t\t Man| 0x%02X 0x%02X 0x%02X 0x%02X\n",
		tmp0, tmp1, tmp2, tmp3);

	tmp0 = max3353_reg_read(max, MAX3353_PRODUCT_ID_0);
	tmp1 = max3353_reg_read(max, MAX3353_PRODUCT_ID_1);
	tmp2 = max3353_reg_read(max, MAX3353_PRODUCT_ID_2);
	tmp3 = max3353_reg_read(max, MAX3353_PRODUCT_ID_3);
	pr_info("\t\t Pro| 0x%02X 0x%02X 0x%02X 0x%02X\n",
		tmp0, tmp1, tmp2, tmp3);

	tmp0 = max3353_reg_read(max, MAX3353_CONTROL_1);
	tmp1 = max3353_reg_read(max, MAX3353_CONTROL_2);
	pr_info("\t\t Con| 0x%02X 0x%02X\n", tmp0, tmp1);

	tmp0 = max3353_reg_read(max, MAX3353_STATUS);
	pr_info("\t\t Stat| 0x%02X\n", tmp0);

	/* Interrupt Latch register is read-to-clear, so don't read it
	 * here and mistakenly clear an interrupt condition.
	 */
	tmp0 = max3353_reg_read(max, MAX3353_INTERRUPT_MASK);
	tmp1 = max3353_reg_read(max, MAX3353_INTERRUPT_EDGE);
	pr_info("\t\t iME| 0x%02X 0x%02X\n", tmp0, tmp1);
}
#endif

#define VBUS_DRIVE_OFF(max)	max3353_reg_bit_clr(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_DRV)
#define VBUS_DRIVE_ON(max)	max3353_reg_bit_set(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_DRV)	

/* Set shutdown mode. */
void max3353_shutdown(struct max3353_device *max, int on)
{
	if (on)
		max3353_reg_bit_set(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_SDWN);
	else
		max3353_reg_bit_clr(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_SDWN);

}

void max3353_vbus_drive_off(struct work_struct *work)
{
	struct max3353_device *max = container_of(work, struct max3353_device, work_vbus);

	KLOG_INFO("Drive VBUS OFF\n");
	VBUS_DRIVE_OFF(max);
}


void max3353_vbus_drive_on(struct work_struct *work)
{
	struct max3353_device *max = container_of(work, struct max3353_device, work_vbus);

	KLOG_INFO("Drive VBUS ON\n");
	VBUS_DRIVE_ON(max);
}

#ifdef CONFIG_USB_OTG_R1_3

/* B-device: SRP VBUS pulse (n/a for OTG2.0): discharge VBUS before start */
static void max3353_vbus_discharge(struct max3353_device *max, int on)
{
	KDBG("on=%d\n", on);

	if (on)
		max3353_reg_bit_set(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_DISCHG);
	else
		max3353_reg_bit_clr(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_DISCHG);
}

/* B-device: SRP VBUS pulse (n/a for OTG2.0): Charge VBUS (externally timed) */
static void max3353_vbus_charge(struct max3353_device *max, int on)
{
	KDBG("on=%d\n", on);

	if (on)
		max3353_reg_bit_set(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_CHG2);
	else
		max3353_reg_bit_clr(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_CHG2);
}

/* B-device: SRP VBUS pulse (n/a for OTG2.0): Charge VBUS (internally timed) */
static void max3353_vbus_pulse(struct max3353_device *max, int on)
{
	KDBG("on=%d\n", on);

	/* Setting the CHG1 bit will create a 105ms VBUS pulse and then halt.
	 * If this bit is set, it needs to be cleared before another pulse
	 * can be started, even though the pulse operations may have ceased.
	 * Hence, always clear this bit, and then set it if needed.
	 */
	max3353_reg_bit_clr(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_CHG1);
	if (on)
		max3353_reg_bit_set(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_VBUS_CHG1);
}
#endif /* CONFIG_USB_OTG_R1_3 */

/* A-device: Always provide VBUS power */
void max3353_vbus_drive(struct max3353_device *max, int on)
{
	if (on) {
		KLOG_INFO("Drive VBUS ON\n");
		VBUS_DRIVE_ON(max);
	} else {
		KLOG_INFO("Drive VBUS OFF\n");
		VBUS_DRIVE_OFF(max);
	}
}

/****************************************************************************
 * NOTE:  three modes are possible here, only one of which
 * will be standards-conformant on any given system:
 *
 *  - OTG mode (dual-role), required if there's a Micro-AB connector
 *  - HOST mode, for when there's an A connector
 *  - DEVICE mode, for when there's a B/Mini-B/Micro-B connector
****************************************************************************/
void max3353_mode_otg(struct max3353_device *max)
{
	max3353_reg_write(max, MAX3353_INTERRUPT_MASK,
				MAX3353_INTERRUPT_MASK_ID_GND_EN |
				MAX3353_INTERRUPT_MASK_ID_FLOAT_EN |
				MAX3353_INTERRUPT_MASK_VBUS_VALID_EN |
				MAX3353_INTERRUPT_MASK_SESSION_VALID_EN |
				MAX3353_INTERRUPT_MASK_SESSION_END_EN);
}

int max3353_mode_host(struct max3353_device *max)
{
	max3353_reg_write(max, MAX3353_INTERRUPT_MASK,
				MAX3353_INTERRUPT_MASK_VBUS_VALID_EN |
				MAX3353_INTERRUPT_MASK_SESSION_VALID_EN |
				MAX3353_INTERRUPT_MASK_SESSION_END_EN);

	max3353_reg_write(max, MAX3353_CONTROL_1,
				MAX3353_CONTROL_1_DP_PULLDWN |
				MAX3353_CONTROL_1_DM_PULLDWN);

	max3353_vbus_drive(max, true);

	return 0;
}

int max3353_mode_peripheral(struct max3353_device *max)
{

	max3353_reg_write(max, MAX3353_INTERRUPT_MASK,
				MAX3353_INTERRUPT_MASK_VBUS_VALID_EN |
				MAX3353_INTERRUPT_MASK_SESSION_VALID_EN |
				MAX3353_INTERRUPT_MASK_SESSION_END_EN);
	return 0;
}

static int max3353_otg_state_poll(struct kona_otg_cp *otg_cp)
{
	struct max3353_device *max = container_of(otg_cp, struct max3353_device, otg);

	max3353_state_update(max);
	
	return 0;
}

static int max3353_otg_vbus_drive(struct kona_otg_cp *otg_cp, int on)
{
	struct max3353_device *max = container_of(otg_cp, struct max3353_device, otg);
	
	if (on)
		PREPARE_WORK(&max->work_vbus, max3353_vbus_drive_on);
	else
		PREPARE_WORK(&max->work_vbus, max3353_vbus_drive_off);
	
	KDBG("schedule VBUS work: on=%d\n", on);
	queue_work(max->workqueue, &max->work_vbus);

	return 0;
}

void max3353_state_set_id(struct max3353_device *max, uint8_t status)
{
	int rc;
	enum kona_otg_cp_id new_id = KONA_OTG_CP_ID_DEVICE_B;

	KDBG("status=0x%02x\n", status);

	if (status & MAX3353_STATUS_SESSION_ID_GND) {
		KDBG("id_state=USB_OTG_CP_ID_DEVICE_A\n");
		new_id = KONA_OTG_CP_ID_DEVICE_A;
	} else {
		KDBG("id_state=USB_OTG_CP_ID_DEVICE_B\n");
		new_id = KONA_OTG_CP_ID_DEVICE_B;
	}

	if (max->id != new_id) {
		max->id = new_id;
		rc = kona_otg_cp_id_change(&max->otg, new_id);
		if (rc < 0) {
			KLOG_ERROR("otg id change failed\n");
		}
	}
}

void max3353_state_set_volt(struct max3353_device *max, uint8_t status)
{
	int rc;
	enum kona_otg_cp_volt_state new_volt;

	KDBG("status=0x%02x\n", status);

	if (status & MAX3353_STATUS_VBUS_VALID) {
		KDBG("volt_state=USB_OTG_CP_VOLT_VBUS_VALID\n");
		new_volt = KONA_OTG_CP_VOLT_VBUS_VALID;
	}
	else if (status & MAX3353_STATUS_SESSION_VALID)
	{
		KDBG("volt_state=USB_OTG_CP_VOLT_SESS_VALID\n");
		new_volt = KONA_OTG_CP_VOLT_SESS_VALID;
	}
	else if (status & MAX3353_STATUS_SESSION_END)
	{
		KDBG("volt_state=USB_OTG_CP_VOLT_SESS_END\n");
		new_volt = KONA_OTG_CP_VOLT_SESS_END;
	}
	else
	{
		KDBG("volt_state=USB_OTG_CP_VOLT_SESS_DETECT\n");
		new_volt = KONA_OTG_CP_VOLT_SESS_DETECT;
	}

	if (max->volt_state != new_volt) {
		max->volt_state = new_volt;
		rc = kona_otg_cp_volt_change(&max->otg, new_volt);
		if (rc < 0) {
			KLOG_ERROR("otg voltage change failed\n");
		}
	}
}

void max3353_device_reset(struct max3353_device *max)
{
	/*
	 * Make sure all interrupts are off and cleared by default.
	 * Also put things into power down and stop any operations.
	 * Essentially make like power-on reset. Note that the
	 * interrupt latch is read-only and read-to-clear.
	 */

	max3353_reg_write(max, MAX3353_INTERRUPT_MASK, 0x0);
	max3353_reg_write(max, MAX3353_INTERRUPT_EDGE, 0x0);
	max3353_reg_read(max, MAX3353_INTERRUPT_LATCH);
	max3353_reg_write(max, MAX3353_CONTROL_2, MAX3353_CONTROL_2_SDWN);
	max3353_reg_write(max, MAX3353_CONTROL_1, 0x0);
}

int max3353_device_init(struct max3353_device *max)
{
	max3353_reg_write(max, MAX3353_INTERRUPT_EDGE,
				MAX3353_INTERRUPT_EDGE_VBUS_VALID_ED |
				MAX3353_INTERRUPT_EDGE_SESSION_VALID_ED);

	switch (max->mode)
	{
	case MAX3353_MODE_DEVICE:
		KLOG_INFO("mode=device\n");
		max3353_mode_peripheral(max);
		break;

	case MAX3353_MODE_HOST:
		KLOG_INFO("mode=host\n");
		max3353_mode_host(max);
		break;

	case MAX3353_MODE_OTG:
		KLOG_INFO("mode=otg\n");
		max3353_mode_otg(max);
		break;

	default:
		KLOG_ERROR("invalid mode=%d (%s %s %s)\n",
			max->mode,
			max3353_mode_str[0],
			max3353_mode_str[1],
			max3353_mode_str[2]
			);
		max->mode = MAX3353_MODE_OTG;
		KDBG_EXIT();		
		return -EOPNOTSUPP;
	}

	max3353_reg_bit_set(max, MAX3353_CONTROL_1, MAX3353_CONTROL_1_IRQ_MODE);
	max3353_shutdown(max, false);

#ifdef DEBUG
	/* dump registers */
	max3353_reg_dump(max);
#endif

	return 0;
}

irqreturn_t max3353_irq_isr(int irq, struct max3353_device *max)
{
	/*
	 * Cannot read using I2C in interrupt context (will block), so schedule
	 * work for the interrupt service thread (IST)
	 */
	KDBG("queue IST work\n");
	queue_work(max->workqueue, &max->work_ist);

	return IRQ_HANDLED;
}

void max3353_irq_ist(struct work_struct *work)
{
	struct max3353_device *max = container_of(work, struct max3353_device, work_ist);
	uint8_t latch;
	uint8_t status;
	uint8_t tmp;

	tmp = 0;
	latch = 0;
	do {
		/*
		 * Read of LATCH register has side effect of clearing it.
		 * However, it has been observed that if the register is
		 * not read until a zero value is returned, interrupts
		 * will not get re-enabled.
		 */
		tmp = max3353_reg_read(max, MAX3353_INTERRUPT_LATCH);
		latch |= tmp;
		KDBG_TRACE("latch=0x%02x tmp=0x%02x\n", latch, tmp);
	} while (tmp);

	status = max3353_reg_read(max, MAX3353_STATUS);

	if (latch & (MAX3353_INTERRUPT_LATCH_ID_FLOAT_RQ |
			MAX3353_INTERRUPT_LATCH_ID_GND_RQ)) {
		max3353_state_set_id(max, status);
	}

	if (latch & (MAX3353_INTERRUPT_LATCH_VBUS_VALID_RP |
			MAX3353_INTERRUPT_LATCH_SESSION_VALID_RP |
			MAX3353_INTERRUPT_LATCH_SESSION_END_RQ)) {
		max3353_state_set_volt(max, status);
	}
}

void max3353_state_update(struct max3353_device *max)
{
	uint8_t status;

	status = max3353_reg_read(max, MAX3353_STATUS);
	KDBG("status=0x%02x\n", status);
	max3353_state_set_id(max, status);
	max3353_state_set_volt(max, status);
}

int proc_mode_write(struct file *file,
			const char __user *buffer,
			unsigned long count,
			void *data)
{
	struct max3353_device *max = (struct max3353_device *)data;
	unsigned int mode;
	unsigned char kbuf[MAX3353_PROC_MAX_BUF_SIZE];
	int rc;


	if (count > MAX3353_PROC_MAX_BUF_SIZE)
		count = MAX3353_PROC_MAX_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		KLOG_ERROR("copy_from_user failed rc=%d\n", rc);
		return -EACCES;
	}

	if (sscanf(kbuf, "%u", &mode) != 1)
	{
		KLOG_ERROR("read mode failed\n");
		return -EINVAL;
	}

	max->mode = mode;
	if (max3353_device_init(max) < 0)
	{
		KLOG_ERROR("init mode failed\n");
		return -EFAULT;
	}

	return count;
}

int proc_mode_read(char *buffer,
			char **start,
			off_t off,
			int count,
			int *eof,
			void *data)
{
	unsigned int len = 0;
	struct max3353_device *max = (struct max3353_device *)data;


	if (off > 0)
		return 0;

	len += sprintf(buffer + len, "MAX3353 mode is %s\n", max3353_mode_str[max->mode]);

	return len;
}

int proc_reg_write(struct file *file,
			const char __user *buffer,
			unsigned long count,
			void *data)
{
	struct max3353_device *max = (struct max3353_device *)data;
	unsigned int write;
	unsigned int val;
	unsigned int tmp;
	unsigned int reg_addr;
	unsigned char kbuf[MAX3353_PROC_MAX_BUF_SIZE];
	int rc;


	if (count > MAX3353_PROC_MAX_BUF_SIZE)
		count = MAX3353_PROC_MAX_BUF_SIZE;

	rc = copy_from_user(kbuf, buffer, count);
	if (rc) {
		KLOG_ERROR("copy_from_user failed rc=%d\n", rc);
		return -EACCES;
	}

	if (sscanf(kbuf, "%u %x %x", &write, &reg_addr, &val) != 3) {
		KLOG_ERROR("usage: echo <0|1> <addr> <val> > %s [0-read, 1-write]\n", MAX3353_PROC_ENTRY_REG);
		return -EINVAL;
	}

	if (write) {
		KLOG_INFO("writing 0x%02x to reg 0x%02x...\n", val, reg_addr);
		max3353_reg_write(max, reg_addr, val);
		tmp = max3353_reg_read(max, reg_addr);
		KLOG_INFO("read back 0x%02X\n", tmp);
	}
	else {
		tmp = max3353_reg_read(max, reg_addr);
		KLOG_INFO("read 0x%02x from reg 0x%02x\n", tmp, reg_addr);
	}

	return count;
}

int proc_init(struct max3353_device *max)
{
	struct max3353_procfs *proc = &max->proc;
	struct proc_dir_entry *proc_mode;
	struct proc_dir_entry *proc_reg;
	int rc;


	proc->name = MAX3353_PROC_GLOBAL_PARENT_DIR;

	/* sub directory */
	proc->parent = proc_mkdir(proc->name, NULL);
	if (proc->parent == NULL) {
		KLOG_ERROR("proc_mkdir failed, name=%s\n", proc->name);
		return -ENOTDIR;
	}

	proc_mode = create_proc_entry(MAX3353_PROC_ENTRY_MODE, 0644, proc->parent);
	if (proc_mode == NULL) {
		KLOG_ERROR("create_proc_entry failed, name=%s\n", MAX3353_PROC_ENTRY_MODE);
		rc = -ENOENT;
		goto err_del_parent;
	}
	proc_mode->read_proc = proc_mode_read;
	proc_mode->write_proc = proc_mode_write;
	proc_mode->data = max;

	proc_reg = create_proc_entry(MAX3353_PROC_ENTRY_REG, 0644, proc->parent);
	if (proc_reg == NULL) {
		KLOG_ERROR("create_proc_entry failed, name=%s\n", MAX3353_PROC_ENTRY_REG);
		rc = -ENOENT;
		goto err_del_mode;
	}
	proc_reg->read_proc = NULL;
	proc_reg->write_proc = proc_reg_write;
	proc_reg->data = max;

	return 0;

err_del_mode:
	remove_proc_entry(MAX3353_PROC_ENTRY_MODE, proc->parent);

err_del_parent:
	remove_proc_entry(proc->name, NULL);

	return rc;
}

int proc_term(struct max3353_device *max)
{
	struct max3353_procfs *proc = &max->proc;

	remove_proc_entry(MAX3353_PROC_ENTRY_MODE, proc->parent);
	remove_proc_entry(MAX3353_PROC_ENTRY_REG, proc->parent);
	remove_proc_entry(proc->name, NULL);

	return 0;
}

static const struct kona_otg_cp_ops otg_cp_ops = {
	.state_poll = max3353_otg_state_poll,
	.vbus_drive = max3353_otg_vbus_drive,
};

static int __devinit max3353_i2c_probe(struct i2c_client *i2c,
		const struct i2c_device_id *id)
{
	struct max3353_device *max;
	int rc;

	if (the_device) {
		KLOG_ERROR("device already exists\n");
		return -EEXIST;
	}

	max = kzalloc(sizeof(struct max3353_device), GFP_KERNEL);
	if (!max) {
		KLOG_ERROR("kzalloc failed\n");
		return -ENOMEM;
	}

	max->i2c = i2c;
	max->dev = &i2c->dev;
	max->data = i2c->dev.platform_data;
	max->otg.name = MAX3353_DRIVER_NAME;
	max->otg.ops = &otg_cp_ops;

	i2c_set_clientdata(i2c, max);

	if (max->data == NULL) {
		KLOG_ERROR("no platform data\n");
		rc = -ENODEV;
		goto err_free_dev;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
	init_MUTEX(&max->reg_read_lock);
#else
	sema_init(&max->reg_read_lock, 1);
#endif

	/* If this is a true probe, verify the chip ... */
	if ((max3353_reg_read(max, MAX3353_MANUFACTURER_0) !=
		 MAX3353_MANUFACTURER_0_VALUE) ||
		(max3353_reg_read(max, MAX3353_MANUFACTURER_1) !=
		 MAX3353_MANUFACTURER_1_VALUE) ||
		(max3353_reg_read(max, MAX3353_MANUFACTURER_2) !=
		 MAX3353_MANUFACTURER_2_VALUE) ||
		(max3353_reg_read(max, MAX3353_MANUFACTURER_3) !=
		 MAX3353_MANUFACTURER_3_VALUE) ||
		(max3353_reg_read(max, MAX3353_PRODUCT_ID_0) !=
		 MAX3353_PRODUCT_ID_0_VALUE) ||
		(max3353_reg_read(max, MAX3353_PRODUCT_ID_1) !=
		 MAX3353_PRODUCT_ID_1_VALUE) ||
		(max3353_reg_read(max, MAX3353_PRODUCT_ID_2) !=
		 MAX3353_PRODUCT_ID_2_VALUE) ||
		(max3353_reg_read(max, MAX3353_PRODUCT_ID_3) !=
		 MAX3353_PRODUCT_ID_3_VALUE)) {
		KLOG_ERROR("max3353 id missing or incorrect\n");
		rc = -EFAULT;
		goto err_free_dev;
	}

	rc = proc_init(max);
	if (rc) {
		KLOG_ERROR("failed to init procfs\n");
		goto err_free_dev;
	}

	/*
	 * Create a strictly ordered work queue for usb_otg_cp_ops callbacks
	 * and possibly local IRQ processing. Can block on I2C reads, so need
	 * to ensure running in a context where blocking is allowed.
	 */
	max->workqueue = create_singlethread_workqueue(MAX3353_DRIVER_NAME);
	if (max->workqueue == NULL) {
		KLOG_ERROR("alloc_workqueue() failed\n");
		rc = -ENOEXEC;
		goto err_proc_term;
	}
	INIT_WORK(&max->work_vbus, max3353_vbus_drive_off);
	INIT_WORK(&max->work_ist, max3353_irq_ist);

	if (max->data->irq_gpio_num >= 0) {
		/* Using interrupts. Request GPIO usage, etc. */
		rc = gpio_request(max->data->irq_gpio_num, MAX3353_DRIVER_NAME "-irq");
		if (rc) {
			KLOG_ERROR("gpio_request(%d) for irq failed: rc=%d\n", 
					max->data->irq_gpio_num, rc);
			rc = -ENOENT;
			goto err_destroy_workqueue;
		}

		rc = gpio_direction_input(max->data->irq_gpio_num);
		if (rc) {
			KLOG_ERROR("gpio_direction_input(%d) for irq failed: rc=%d\n",
					max->data->irq_gpio_num, rc);
			rc = -EIO;
			goto err_free_gpio;
		}

		rc = request_irq(gpio_to_irq(max->data->irq_gpio_num),
				(irq_handler_t)max3353_irq_isr,
				IRQF_TRIGGER_FALLING,
				MAX3353_DRIVER_NAME,
				(void *)max);
		if (rc) {
			KLOG_ERROR("request_irq(%d) for gpio=%d failed: rc=%d\n",
					gpio_to_irq(max->data->irq_gpio_num), max->data->irq_gpio_num, rc);
			rc = -EINTR;
			goto err_free_gpio;
		}
	}

	/* now reset the chip */
	max3353_device_reset(max);

	max->mode = max->data->mode;
	max3353_device_init(max);
	max->id = KONA_OTG_CP_ID_UNKNOWN;
	max->volt_state = KONA_OTG_CP_VOLT_UNKNOWN;
	the_device = max;

	rc = kona_otg_cp_register(&max->otg);
	if (rc) {
		KLOG_ERROR("unable to register OTG cp rc=%d\n", rc);
		goto err_reset_device;
	}

	max3353_state_update(max);

	return 0;

err_reset_device:
	max3353_device_reset(max);

	if (max->data->irq_gpio_num >= 0)
		free_irq(gpio_to_irq(max->data->irq_gpio_num), max);

err_free_gpio:
	if (max->data->irq_gpio_num >= 0)
		gpio_free(max->data->irq_gpio_num);

err_destroy_workqueue:
	destroy_workqueue(max->workqueue);

err_proc_term:
	proc_term(max);

err_free_dev:
	i2c_set_clientdata(i2c, NULL);
	kfree(max);
	
	return rc;
}

static int __devexit max3353_i2c_remove(struct i2c_client *i2c)
{
	struct max3353_device *max = i2c_get_clientdata(i2c);

	max3353_device_reset(max);
	kona_otg_cp_unregister(&max->otg);
	
	if (max->data->irq_gpio_num >= 0) {
		free_irq(gpio_to_irq(max->data->irq_gpio_num), max);
		gpio_free(max->data->irq_gpio_num);
		destroy_workqueue(max->workqueue); 
	}
	proc_term(max);

	the_device = NULL;
	i2c_set_clientdata(i2c, NULL);
	kfree(max);
	
	return 0;
}

static struct i2c_driver max3353_i2c_driver =
{
	.driver = {
		.name = MAX3353_DRIVER_NAME,
	},
	.id_table = max3353_i2c_id,
	.class = I2C_CLASS_HWMON,
	.probe = max3353_i2c_probe,
	.remove	= __devexit_p(max3353_i2c_remove),
};

static int __init max3353_init(void)
{
	return i2c_add_driver(&max3353_i2c_driver);
}
subsys_initcall_sync(max3353_init);

static void __exit max3353_exit(void)
{
	i2c_del_driver(&max3353_i2c_driver);
}
module_exit(max3353_exit);

MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(i2c, max3353_i2c_id);

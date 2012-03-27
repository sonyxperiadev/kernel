/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed
*   to you under the terms of the GNU General Public License version 2,
*   available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*   (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
*   under a license other than the GPL, without Broadcom's express
*   prior written consent.
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/platform_device.h>

#include "config.h"

#define MAX_PROC_BUF_SIZE     32
#define MIN_MTT_SD_SIZE       10
#define MAX_MTT_SD_SIZE       2048
#define DEFAULT_MTT_SD_SIZE   2048

/* work queue for reading config file */
static struct delayed_work g_load_config_wq;
/* log configuration */
static struct BCMLOG_Config_t g_config;
/* procfs file */
static struct proc_dir_entry *g_proc_dir_entry = { 0 };

static char *bcmlog_name[] = {
	"none",		/* BCMLOG_OUTDEV_NONE */
	"panic",	/* BCMLOG_OUTDEV_PANIC */
	"sdcard",	/* BCMLOG_OUTDEV_SDCARD */
	"rndis",	/* BCMLOG_OUTDEV_RNDIS */
	"uart",		/* BCMLOG_OUTDEV_UART */
	"acm",		/* BCMLOG_OUTDEV_ACM */
	"stm",		/* BCMLOG_OUTDEV_STM */
	"custom",	/* BCMLOG_OUTDEV_CUSTOM */
};

static char *get_log_name(int idx)
{
	if (idx > BCMLOG_OUTDEV_STM)
		return NULL;

	return bcmlog_name[idx];
}

static int get_log_idx(char *name)
{
	int i;

	for (i = 0; i < BCMLOG_OUTDEV_MAX; i++) {
		if (!strncmp(name, bcmlog_name[i], strlen(bcmlog_name[i])))
			return i;
	}
	return BCMLOG_OUTDEV_NONE;
}

/**
 *	Set configuration defaults
 **/
static void SetConfigDefaults(void)
{
#ifdef CONFIG_APANIC_ON_MMC
	g_config.ap_crashlog.dev = BCMLOG_OUTDEV_PANIC;
#else
	g_config.ap_crashlog.dev = BCMLOG_OUTDEV_NONE;
#endif
#ifdef CONFIG_STM_DEFAULT
	/* run-time log to STM */
	g_config.runlog.dev = BCMLOG_OUTDEV_STM;
	g_config.cp_crashlog.dev = BCMLOG_OUTDEV_STM;
#else
	g_config.runlog.dev = BCMLOG_OUTDEV_STM;
	/* eventually set to BCMLOG_OUTDEV_PANIC once its working */
	g_config.cp_crashlog.dev = BCMLOG_OUTDEV_STM;
#endif
	/* Max size of MTT SD file (MB) */
	g_config.file_max = DEFAULT_MTT_SD_SIZE;
}

/**
 *	'safe' strncpy
 **/
static void safe_strncat(char *dst, const char *src, int len)
{
	int l_dst = strlen(dst);
	int l_src = strlen(src);
	if (l_dst + l_src < len)
		strncat(dst, src, len - l_dst - l_src);
}

/**
 *	build device status string and append to 'buf'
 **/
static void bld_device_status_str(char *buf, int len, char *label, int device,
				  int sd_max_size, int locked)
{
	char sd_max[5];

	safe_strncat(buf, label, len);

	switch (device) {
	case BCMLOG_OUTDEV_NONE:
		safe_strncat(buf, "-> disabled", len);
		break;
	case BCMLOG_OUTDEV_PANIC:
		safe_strncat(buf, "-> flash", len);
		break;
	case BCMLOG_OUTDEV_RNDIS:
		safe_strncat(buf, "-> RNDIS", len);
		break;
	case BCMLOG_OUTDEV_SDCARD:
		if (sd_max_size == 0) {
			safe_strncat(buf, "-> SD card", len);
		} else {
			sprintf(sd_max, "%d", sd_max_size);
			safe_strncat(buf, "-> SD card", len);
			safe_strncat(buf, " (Max file size : ", len);
			safe_strncat(buf, sd_max, len);
			safe_strncat(buf, " MB)", len);
		}
		break;
	case BCMLOG_OUTDEV_UART:
		safe_strncat(buf, "-> UART", len);
		break;
	case BCMLOG_OUTDEV_ACM:
		safe_strncat(buf, "-> ACM", len);
		break;
	case BCMLOG_OUTDEV_STM:
		safe_strncat(buf, "-> STM", len);
		break;
	case BCMLOG_OUTDEV_CUSTOM:
		safe_strncat(buf, "-> CUSTOM", len);
		break;
	default:
		safe_strncat(buf, "-> ERROR", len);
		break;
	}
	if (locked)
		safe_strncat(buf, " locked\n", len);
	else
		safe_strncat(buf, " unlocked\n", len);
}

/**
 *	proc read handler
 **/
static int proc_read(char *page, char **start, off_t offset, int count,
		     int *eof, void *data)
{
	*page = 0;
	bld_device_status_str(page, count, "  BMTT logging",
			      g_config.runlog.dev, g_config.file_max,
			      g_config.runlog.lock);
	bld_device_status_str(page, count, "  AP crash dump",
			      g_config.ap_crashlog.dev, 0,
			      g_config.ap_crashlog.lock);
	bld_device_status_str(page, count, "  CP crash dump",
			      g_config.cp_crashlog.dev, 0,
			      g_config.cp_crashlog.lock);
	*eof = 1;
	return 1 + strlen(page);
}

/**
 *	proc write handler
 *
 *		a - BMTT logging   -> RNDIS
 *		b - BMTT logging   -> USB serial
 *		c - BMTT logging   -> UART
 *		d - BMTT logging   -> SD card
 *		e - APP crash dump -> flash
 *		f - APP crash dump -> SD card
 *		g - APP crash dump -> disabled
 *		 h - Save for reboot
 *		 i - Restore defaults
 *		j - CP crash dump -> flash
 *		k - CP crash dump -> SD card
 *		l - CP crash dump -> disabled
 *		m - CP crash dump -> RNDIS
 *		n - BMTT logging   -> STM
 *		o - CP crash dump -> STM
 *		p - CP crash dump -> ACM
 *		s -  both BMTT and CP crash dump -> STM
 *		t -  BMTT logging   -> custom
 *		u -  APP crash dump -> custom
 *		v -  CP crash dump -> custom
 **/
static ssize_t proc_write(struct file *file, const char __user * buffer,
			  unsigned long count, void *data)
{
	int rc;
	int val;
	unsigned char kbuf[MAX_PROC_BUF_SIZE];

	if (count > 0) {
		memset(kbuf, 0, sizeof(kbuf));
		if (count > sizeof(kbuf) - 1)
			count = sizeof(kbuf) - 1;
		if (copy_from_user(kbuf, buffer, count))
			return -EFAULT;

		switch (*kbuf) {
		case 'a':
			if (!g_config.runlog.lock)
				g_config.runlog.dev = BCMLOG_OUTDEV_RNDIS;
			break;
		case 'b':
			if (!g_config.runlog.lock)
				g_config.runlog.dev = BCMLOG_OUTDEV_ACM;
			break;
		case 'c':
			if (!g_config.runlog.lock)
				g_config.runlog.dev = BCMLOG_OUTDEV_UART;
			break;
		case 'd':
			if (!g_config.runlog.lock)
				g_config.runlog.dev = BCMLOG_OUTDEV_SDCARD;
			break;
		case 'e':
			if (!g_config.ap_crashlog.lock)
				g_config.ap_crashlog.dev = BCMLOG_OUTDEV_PANIC;
			break;
		case 'g':
			if (!g_config.ap_crashlog.lock)
				g_config.ap_crashlog.dev = BCMLOG_OUTDEV_NONE;
			break;
		case 'h':
			BCMLOG_SaveConfig(1);
			break;
		case 'i':
			SetConfigDefaults();
			BCMLOG_SaveConfig(0);
			break;
		case 'j':
			if (!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_PANIC;
			break;
		case 'k':
			if (!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_SDCARD;
			break;
		case 'l':
			if (!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_NONE;
			break;
		case 'm':
			if (!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_RNDIS;
			break;
		case 'n':
			if (!g_config.runlog.lock)
				g_config.runlog.dev = BCMLOG_OUTDEV_STM;
			break;
		case 'o':
			if (!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_STM;
			break;
		case 'p':
			if (!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_ACM;
			break;
		case 's':
			if (!g_config.runlog.lock)
				g_config.runlog.dev = BCMLOG_OUTDEV_STM;
			if (!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_STM;
			break;
		case 't':
			if (g_config.runlog.handler && !g_config.runlog.lock)
				g_config.runlog.dev = BCMLOG_OUTDEV_CUSTOM;
			break;
		case 'u':
			if (g_config.ap_crashlog.handler &&
				!g_config.ap_crashlog.lock)
				g_config.ap_crashlog.dev = BCMLOG_OUTDEV_CUSTOM;
			break;
		case 'v':
			if (g_config.cp_crashlog.handler &&
				!g_config.cp_crashlog.lock)
				g_config.cp_crashlog.dev = BCMLOG_OUTDEV_CUSTOM;
			break;
		default:
			rc = kstrtoint(strstrip(kbuf), 10, &val);
			if ((rc == 0) && (val >= MIN_MTT_SD_SIZE)
			    && (val <= MAX_MTT_SD_SIZE))
				g_config.file_max = val;
			break;
		}
	}

	return count;
}

/**
*     Load configuration from persistent storage
**/
static int LoadConfigFromPersistentStorage(void)
{
	mm_segment_t oldfs;
	struct file *config_file;
	int rc = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	config_file = filp_open(BCMLOG_CONFIG_PS_FILE, O_RDONLY, 0);

	if (IS_ERR(config_file)) {
		rc = -1;
	} else {
		if (sizeof(struct BCMLOG_Config_t) !=
				config_file->f_op->read(config_file,
				(void *)&g_config,
				sizeof(struct BCMLOG_Config_t),
				&config_file->f_pos)) {
			rc = -1;
		}

		filp_close(config_file, NULL);
	}

	set_fs(oldfs);

	return rc;
}

/**
 *     Work thread to read configuration file.  Called at driver
 *     initialization, will reschedule itself periodically until
 *     configuration is successfully loaded, up to predetermined
 *     number of attempts.  Delay is necessary to wait until
 *     file system is available.
 *
 *     @param  (in)    ptr to work structure
 *     @note
 *             Function prototype as required by INIT_DELAYED_WORK macro.
 *     @note
 *             There is a one-second delay between each attemt.  It was
 *             determined emperically that 5 - 6 seconds is about
 *             the time required for FS to come up, so we'll give
 *             it up to 10 seconds.
 **/
static void try_load_config(struct work_struct *work)
{
	static int tries = 10;

	if (--tries > 0)
		if (LoadConfigFromPersistentStorage() < 0)
			schedule_delayed_work(&g_load_config_wq, 1 * HZ);
}

/**
*	Save or reset configuration persistent storage
 *	@param			saveFlag [in] if nonzero save configuration else reset
 *							configuration
 *	@return	0 on success, -1 on error
 **/
int BCMLOG_SaveConfig(int saveFlag)
{
	mm_segment_t oldfs;
	struct file *config_file;
	int rc = 0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	config_file =
	filp_open(BCMLOG_CONFIG_PS_FILE, O_WRONLY | O_TRUNC | O_CREAT,
	0666);

	if (IS_ERR(config_file))
		rc = -1;

	else {
		if (saveFlag)
			if (sizeof(struct BCMLOG_Config_t) !=
					config_file->f_op->write(config_file,
					(void *)&g_config,
					sizeof(struct
					BCMLOG_Config_t),
					&config_file->f_pos))
				rc = -1;

		/* if !saveFlag the file is truncated to zero bytes,
		* invalidating the configuration */
		filp_close(config_file, NULL);
	}

	set_fs(oldfs);

	return rc;
}

/**
 *	Enable or disable log ID
 *	@param		inLogId (in) log id
 *	@param		inEnable (in) nonzero to enable; zero to disable log ID
 **/
void BCMLOG_EnableLogId(unsigned short inLogId, int inEnable)
{
	unsigned long index;
	unsigned long bit;

	if (inLogId < BCMLOG_MAX_LOG_ID) {
		index = inLogId / BITS_PER_LONG;
		bit = inLogId % BITS_PER_LONG;

		if (inEnable)
			g_config.id_enable[index] |= (1 << bit);
		else
			g_config.id_enable[index] &= ~(1 << bit);
	}
}

/**
 *	Check if log id (sender) is enabled
 *	@param		inLogId (in) log id
 *	@return		1 if enabled; 0 if not enabled or invalid ID
 **/
int BCMLOG_LogIdIsEnabled(unsigned short inLogId)
{

	unsigned long index;
	unsigned long bit;

	if (inLogId >= BCMLOG_MAX_LOG_ID)
		return 0;

	index = inLogId / BITS_PER_LONG;
	bit = inLogId % BITS_PER_LONG;

	return g_config.id_enable[index] & (1 << bit) ? 1 : 0;
}

static ssize_t set_log_store(const char *buf, size_t size,
		struct log_type_t *log)
{
	char str[MAX_STR_NAME];
	int idx;

	if (size >= MAX_STR_NAME)
		return -EINVAL;

	if (sscanf(buf, "%s", str) == 1) {
		idx = get_log_idx(str);
		if (!log->lock && !((idx == BCMLOG_OUTDEV_CUSTOM) &&
				log->handler)) {
			log->dev = idx;
			return size;
		}
	}
	return -EINVAL;
}

static ssize_t bcmlog_log_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", get_log_name(g_config.runlog.dev));
}

static ssize_t bcmlog_log_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return set_log_store(buf, size, &g_config.runlog);
}

static ssize_t bcmlog_log_lock_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%0x\n", g_config.runlog.lock);
}

static ssize_t bcmlog_log_lock_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int res, val;

	res = kstrtoint(buf, 16, &val);
	g_config.runlog.lock = val;
	return size;
}

static ssize_t bcmlog_cp_crash_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", get_log_name(g_config.cp_crashlog.dev));
}

static ssize_t bcmlog_cp_crash_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return set_log_store(buf, size, &g_config.cp_crashlog);
}

static ssize_t bcmlog_cp_crash_lock_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%0x\n", g_config.cp_crashlog.lock);
}

static ssize_t bcmlog_cp_crash_lock_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int res, val;

	res = kstrtoint(buf, 16, &val);
	g_config.cp_crashlog.lock = val;
	return size;
}

static ssize_t bcmlog_ap_crash_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", get_log_name(g_config.ap_crashlog.dev));
}

static ssize_t bcmlog_ap_crash_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	return set_log_store(buf, size, &g_config.ap_crashlog);
}

static ssize_t bcmlog_ap_crash_lock_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%0x\n", g_config.ap_crashlog.lock);
}

static ssize_t bcmlog_ap_crash_lock_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int res, val;

	res = kstrtoint(buf, 16, &val);
	g_config.ap_crashlog.lock = val;
	return size;
}

static ssize_t bcmlog_file_base_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", g_config.file_base);
}

static ssize_t bcmlog_file_base_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	if (size >= MAX_STR_NAME)
		return -EINVAL;

	if (sscanf(buf, "%s", g_config.file_base) == 1)
		return size;
	return -EINVAL;
}

static ssize_t bcmlog_file_max_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%0x\n", g_config.file_max);
}

static ssize_t bcmlog_file_max_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int res, val;

	res = kstrtoint(buf, 16, &val);
	g_config.file_max = val;
	return size;
}


static ssize_t bcmlog_uart_dev_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", g_config.uart_dev);
}

static ssize_t bcmlog_uart_dev_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	if (size >= MAX_STR_NAME)
		return -EINVAL;

	if (sscanf(buf, "%s", g_config.uart_dev) == 1)
		return size;
	return -EINVAL;
}

static ssize_t bcmlog_acm_dev_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", g_config.acm_dev);
}

static ssize_t bcmlog_acm_dev_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	if (size >= MAX_STR_NAME)
		return -EINVAL;

	if (sscanf(buf, "%s", g_config.acm_dev) == 1)
		return size;
	return -EINVAL;
}

static DEVICE_ATTR(log, S_IRUGO | S_IWUSR, bcmlog_log_show,
					     bcmlog_log_store);

static DEVICE_ATTR(log_lock, S_IRUGO | S_IWUSR, bcmlog_log_lock_show,
						 bcmlog_log_lock_store);

static DEVICE_ATTR(cp_crash, S_IRUGO | S_IWUSR, bcmlog_cp_crash_show,
						  bcmlog_cp_crash_store);

static DEVICE_ATTR(cp_crash_lock, S_IRUGO | S_IWUSR, bcmlog_cp_crash_lock_show,
					     bcmlog_cp_crash_lock_store);

static DEVICE_ATTR(ap_crash, S_IRUGO | S_IWUSR, bcmlog_ap_crash_show,
						 bcmlog_ap_crash_store);

static DEVICE_ATTR(ap_crash_lock, S_IRUGO | S_IWUSR, bcmlog_ap_crash_lock_show,
					     bcmlog_ap_crash_lock_store);

static DEVICE_ATTR(file_base, S_IRUGO | S_IWUSR, bcmlog_file_base_show,
						 bcmlog_file_base_store);

static DEVICE_ATTR(file_max, S_IRUGO | S_IWUSR, bcmlog_file_max_show,
						 bcmlog_file_max_store);

static DEVICE_ATTR(uart_dev, S_IRUGO | S_IWUSR, bcmlog_uart_dev_show,
						 bcmlog_uart_dev_store);

static DEVICE_ATTR(acm_dev, S_IRUGO | S_IWUSR, bcmlog_acm_dev_show,
						 bcmlog_acm_dev_store);


char *BCMLOG_GetFileBase(void)
{
	return g_config.file_base;
}

char *BCMLOG_GetUartDev(void)
{
	return g_config.uart_dev;
}

char *BCMLOG_GetAcmDev(void)
{
	return g_config.acm_dev;
}

/**
 *	Initialize logging configuration.  Schedules a work thread to
 *	load the configuration file once the file system is readable.
 **/
void BCMLOG_InitConfig(void *h)
{
	int value;
	struct device * dev = (struct device *)h;
	/*
	 *      disable all AP logging (CP logging is
	 *      handled by CP) [MobC00126731]
	 */
	memset(&g_config, 0x00, sizeof(g_config));

	/*
	 *      set default configuration
	 */
	SetConfigDefaults();

	/*
	 *      create the procfs entry
	 */
	g_proc_dir_entry =
	    create_proc_entry(BCMLOG_CONFIG_PROC_FILE,
			      S_IRWXU | S_IRWXG | S_IRWXO, NULL);

	if (g_proc_dir_entry == NULL)
		remove_proc_entry(BCMLOG_CONFIG_PROC_FILE, NULL);

	else {
		g_proc_dir_entry->read_proc = proc_read;
		g_proc_dir_entry->write_proc = proc_write;
	}

	strncpy(g_config.file_base, BCMLOG_DEFAULT_FILE_BASE,
			strlen(BCMLOG_DEFAULT_FILE_BASE));
	strncpy(g_config.uart_dev, BCMLOG_DEFAULT_UART_DEV,
			strlen(BCMLOG_DEFAULT_UART_DEV));
	strncpy(g_config.acm_dev, BCMLOG_DEFAULT_ACM_DEV,
			strlen(BCMLOG_DEFAULT_ACM_DEV));

	value = device_create_file(dev, &dev_attr_log);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog log attribute\n");
	value = device_create_file(dev, &dev_attr_log_lock);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog log_lock attribute\n");
	value = device_create_file(dev, &dev_attr_cp_crash);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog cp crash log attribute\n");
	value = device_create_file(dev, &dev_attr_cp_crash_lock);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog cp crash log lock attribute\n");
	value = device_create_file(dev, &dev_attr_ap_crash);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog ap crash log attribute\n");
	value = device_create_file(dev, &dev_attr_ap_crash_lock);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog ap crash log lock attribute\n");
	value = device_create_file(dev, &dev_attr_file_base);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog file_base attribute\n");
	value = device_create_file(dev, &dev_attr_file_max);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog file max attribute\n");
	value = device_create_file(dev, &dev_attr_uart_dev);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog uart_dev attribute\n");
	value = device_create_file(dev, &dev_attr_acm_dev);
	if (value < 0)
		pr_err("BCMLOG Init failed to create bcmlog acm_dev attribute\n");

	/*
	 *	start a thread to attempt to load configuration when
	 *	filesystem ready
	 */
	INIT_DELAYED_WORK(&g_load_config_wq, try_load_config);

	schedule_delayed_work(&g_load_config_wq, 1*HZ);

}

int BCMLOG_GetRunlogDevice(void)
{
	return g_config.runlog.dev;
}

void BCMLOG_SetRunlogDevice(int run_log_dev)
{
	g_config.runlog.dev = run_log_dev;
}

int BCMLOG_GetSdFileMax(void)
{
	u64 int_max = INT_MAX;
	u64 size = (g_config.file_max * 1024 * 1024);

	return (int)min(size, int_max);
}

int BCMLOG_GetCpCrashLogDevice(void)
{
	return g_config.cp_crashlog.dev;
}

void BCMLOG_SetCpCrashLogDevice(int port)
{
	g_config.cp_crashlog.dev = port;
}

int BCMLOG_GetApCrashLogDevice(void)
{
	return g_config.ap_crashlog.dev;
}

int BCMLOG_IsUSBLog(void)
{
	if (BCMLOG_OUTDEV_RNDIS == g_config.runlog.dev
	    || BCMLOG_OUTDEV_ACM == g_config.runlog.dev)
		return 1;
	else
		return 0;
}

/**
 *	Register custom log handler
 **/
int BCMLOG_RegisterHandler(char log_type,
			   int (*handler) (const char *, unsigned int, char))
{
	int rc = 0;

	switch (log_type) {
	case BCMLOG_CUSTOM_RUN_LOG:
		g_config.runlog.handler = handler;
		break;
	case BCMLOG_CUSTOM_AP_CRASH_LOG:
		g_config.ap_crashlog.handler = handler;
		break;
	case BCMLOG_CUSTOM_CP_CRASH_LOG:
		g_config.cp_crashlog.handler = handler;
		break;
	default:
		rc = -1;
		break;
	}

	return rc;
}
EXPORT_SYMBOL(BCMLOG_RegisterHandler);

/**
 *	Call registered custom log handler
 **/
int BCMLOG_CallHandler(char log_type, const char *p_src, unsigned int len,
		       char payload_type)
{
	int rc = 0;

	switch (log_type) {
	case BCMLOG_CUSTOM_RUN_LOG:
		if (g_config.runlog.handler)
			rc = g_config.runlog.handler(p_src, len, payload_type);
		break;
	case BCMLOG_CUSTOM_AP_CRASH_LOG:
		if (g_config.ap_crashlog.handler)
			rc = g_config.ap_crashlog.handler(p_src, len,
							  payload_type);
		break;
	case BCMLOG_CUSTOM_CP_CRASH_LOG:
		if (g_config.cp_crashlog.handler)
			rc = g_config.cp_crashlog.handler(p_src, len,
							  payload_type);
		break;
	default:
		break;
	}

	return rc;
}

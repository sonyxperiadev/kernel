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

/**
 *	Set configuration defaults
 **/
static void SetConfigDefaults(void)
{
#ifdef CONFIG_APANIC_ON_MMC
	g_config.ap_crashlog_dev = BCMLOG_OUTDEV_PANIC;
#else
	g_config.ap_crashlog_dev = BCMLOG_OUTDEV_NONE;
#endif
#ifdef CONFIG_STM_DEFAULT
	/* run-time log to STM */
	g_config.runlog_dev = BCMLOG_OUTDEV_STM;
	g_config.cp_crashlog_dev = BCMLOG_OUTDEV_STM;
#else
	g_config.runlog_dev = BCMLOG_OUTDEV_STM;
	/* eventually set to BCMLOG_OUTDEV_PANIC once its working */
	g_config.cp_crashlog_dev = BCMLOG_OUTDEV_STM;
#endif
	/* Max size of MTT SD file (MB) */
	g_config.sd_file_max = DEFAULT_MTT_SD_SIZE;
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
				  int sd_max_size)
{
	char sd_max[5];

	safe_strncat(buf, label, len);

	switch (device) {
	case BCMLOG_OUTDEV_NONE:
		safe_strncat(buf, "-> disabled\n", len);
		break;
	case BCMLOG_OUTDEV_PANIC:
		safe_strncat(buf, "-> flash\n", len);
		break;
	case BCMLOG_OUTDEV_RNDIS:
		safe_strncat(buf, "-> RNDIS\n", len);
		break;
	case BCMLOG_OUTDEV_SDCARD:
		if (sd_max_size == 0) {
			safe_strncat(buf, "-> SD card\n", len);
		} else {
			sprintf(sd_max, "%d", sd_max_size);
			safe_strncat(buf, "-> SD card", len);
			safe_strncat(buf, " (Max file size : ", len);
			safe_strncat(buf, sd_max, len);
			safe_strncat(buf, " MB)\n", len);
		}
		break;
	case BCMLOG_OUTDEV_UART:
		safe_strncat(buf, "-> UART\n", len);
		break;
	case BCMLOG_OUTDEV_ACM:
		safe_strncat(buf, "-> ACM\n", len);
		break;
	case BCMLOG_OUTDEV_STM:
		safe_strncat(buf, "-> STM\n", len);
		break;
	case BCMLOG_OUTDEV_CUSTOM:
		safe_strncat(buf, "-> CUSTOM\n", len);
		break;
	default:
		safe_strncat(buf, "-> ERROR\n", len);
		break;
	}
}

/**
 *	proc read handler
 **/
static int proc_read(char *page, char **start, off_t offset, int count,
		     int *eof, void *data)
{
	*page = 0;
	bld_device_status_str(page, count, "  BMTT logging",
			      g_config.runlog_dev, g_config.sd_file_max);
	bld_device_status_str(page, count, "  AP crash dump",
			      g_config.ap_crashlog_dev, 0);
	bld_device_status_str(page, count, "  CP crash dump",
			      g_config.cp_crashlog_dev, 0);
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
 *		h - Save for reboot
 *		i - Restore defaults
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
			g_config.runlog_dev = BCMLOG_OUTDEV_RNDIS;
			break;
		case 'b':
			g_config.runlog_dev = BCMLOG_OUTDEV_ACM;
			break;
		case 'c':
			g_config.runlog_dev = BCMLOG_OUTDEV_UART;
			break;
		case 'd':
			g_config.runlog_dev = BCMLOG_OUTDEV_SDCARD;
			break;
		case 'e':
			g_config.ap_crashlog_dev = BCMLOG_OUTDEV_PANIC;
			break;
		case 'f':
			g_config.ap_crashlog_dev = BCMLOG_OUTDEV_SDCARD;
			break;
		case 'g':
			g_config.ap_crashlog_dev = BCMLOG_OUTDEV_NONE;
			break;
		case 'h':
			BCMLOG_SaveConfig(1);
			break;
		case 'i':
			SetConfigDefaults();
			BCMLOG_SaveConfig(0);
			break;
		case 'j':
			g_config.cp_crashlog_dev = BCMLOG_OUTDEV_PANIC;
			break;
		case 'k':
			g_config.cp_crashlog_dev = BCMLOG_OUTDEV_SDCARD;
			break;
		case 'l':
			g_config.cp_crashlog_dev = BCMLOG_OUTDEV_NONE;
			break;
		case 'm':
			g_config.cp_crashlog_dev = BCMLOG_OUTDEV_RNDIS;
			break;
		case 'n':
			g_config.runlog_dev = BCMLOG_OUTDEV_STM;
			break;
		case 'o':
			g_config.cp_crashlog_dev = BCMLOG_OUTDEV_STM;
			break;
		case 'p':
			g_config.cp_crashlog_dev = BCMLOG_OUTDEV_ACM;
			break;
		case 's':
			g_config.runlog_dev = BCMLOG_OUTDEV_STM;
			g_config.cp_crashlog_dev = BCMLOG_OUTDEV_STM;
			break;
		case 't':
			if (g_config.runlog_handler)
				g_config.runlog_dev = BCMLOG_OUTDEV_CUSTOM;
			break;
		case 'u':
			if (g_config.ap_crashlog_handler)
				g_config.ap_crashlog_dev = BCMLOG_OUTDEV_CUSTOM;
			break;
		case 'v':
			if (g_config.cp_crashlog_handler)
				g_config.cp_crashlog_dev = BCMLOG_OUTDEV_CUSTOM;
			break;
		default:
			rc = kstrtoint(strstrip(kbuf), 10, &val);
			if ((rc == 0) && (val >= MIN_MTT_SD_SIZE)
			    && (val <= MAX_MTT_SD_SIZE))
				g_config.sd_file_max = val;
			break;
		}
	}

	return count;
}

/**
 *	Load configuration from persistent storage
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
	}

	else {
		if (sizeof(struct BCMLOG_Config_t) !=
		    config_file->f_op->read(config_file, (void *)&g_config,
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
 *	Work thread to read configuration file.  Called at driver
 *	initialization, will reschedule itself periodically until
 *	configuration is successfully loaded, up to predetermined
 *	number of attempts.  Delay is necessary to wait until
 *	file system is available.
 *
 *	@param	(in)	ptr to work structure
 *	@note
 *		Function prototype as required by INIT_DELAYED_WORK macro.
 *	@note
 *		There is a one-second delay between each attemt.  It was
 *		determined emperically that 5 - 6 seconds is about
 *		the time required for FS to come up, so we'll give
 *		it up to 10 seconds.
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
 *	@param		saveFlag [in] if nonzero save configuration else reset
 *				configuration
 *	@return		0 on success, -1 on error
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

/**
 *	Initialize logging configuration.  Schedules a work thread to
 *	load the configuration file once the file system is readable.
 **/
void BCMLOG_InitConfig(void)
{
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

	if (g_proc_dir_entry == NULL) {
		remove_proc_entry(BCMLOG_CONFIG_PROC_FILE, NULL);
	}

	else {
		g_proc_dir_entry->read_proc = proc_read;
		g_proc_dir_entry->write_proc = proc_write;
	}

	/*
	 *      start a thread to attempt to load configuration
	 *      when filesystem ready
	 */
	INIT_DELAYED_WORK(&g_load_config_wq, try_load_config);

	schedule_delayed_work(&g_load_config_wq, 1 * HZ);
}

int BCMLOG_GetRunlogDevice(void)
{
	return g_config.runlog_dev;
}

void BCMLOG_SetRunlogDevice(int run_log_dev)
{
	g_config.runlog_dev = run_log_dev;
}

int BCMLOG_GetSdFileMax(void)
{
	u64 int_max = INT_MAX;
	u64 size = (g_config.sd_file_max * 1024 * 1024);

	return (int)min(size, int_max);
}

int BCMLOG_GetCpCrashLogDevice(void)
{
	return g_config.cp_crashlog_dev;
}

void BCMLOG_SetCpCrashLogDevice(int port)
{
	g_config.cp_crashlog_dev = port;
}

int BCMLOG_GetApCrashLogDevice(void)
{
	return g_config.ap_crashlog_dev;
}

int BCMLOG_IsUSBLog(void)
{
	if (BCMLOG_OUTDEV_RNDIS == g_config.runlog_dev
	    || BCMLOG_OUTDEV_ACM == g_config.runlog_dev)
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
		g_config.runlog_handler = handler;
		break;
	case BCMLOG_CUSTOM_AP_CRASH_LOG:
		g_config.ap_crashlog_handler = handler;
		break;
	case BCMLOG_CUSTOM_CP_CRASH_LOG:
		g_config.cp_crashlog_handler = handler;
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
		if (g_config.runlog_handler)
			rc = g_config.runlog_handler(p_src, len, payload_type);
		break;
	case BCMLOG_CUSTOM_AP_CRASH_LOG:
		if (g_config.ap_crashlog_handler)
			rc = g_config.ap_crashlog_handler(p_src, len,
							  payload_type);
		break;
	case BCMLOG_CUSTOM_CP_CRASH_LOG:
		if (g_config.cp_crashlog_handler)
			rc = g_config.cp_crashlog_handler(p_src, len,
							  payload_type);
		break;
	default:
		break;
	}

	return rc;
}

/*
 *  tv_intf_kernel.c - core tv_intf driver
 *
 *  Copyright (C) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This device provides a shared mechanism for writing to the mailboxes,
 * semaphores, doorbells etc. that are shared between the ARM and the VideoCore
 * processor
 */
#include <linux/module.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/ipc/ipc.h>

#include "tv_intf.h"
#include "tv_intf_regs.h"
#include "tv_intf_driver.h"

/* macros */
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned int)(unsigned char)(ch0) \
		| ((unsigned int)(unsigned char)(ch1) << 8) \
		| ((unsigned int)(unsigned char)(ch2) << 16) \
		| ((unsigned int)(unsigned char)(ch3) << 24 ))

#define TV_INTF_FOURCC MAKEFOURCC( 'T', 'V', 'I', 'F')

/* structure decalaration */
static struct {
	volatile unsigned char *base_address;
	struct resource *mem_res;
	unsigned int irq;
	struct proc_dir_entry *dump_info;
	struct semaphore work_status; /* 0 = busy processing, 1 = free */
	struct semaphore command_complete;
	int initialized;

   char cmd_buf[128];
}tv_intf_state;

typedef struct {
   const char *key;
   uint32_t ctrl_offset;
   uint32_t status_offset;
   uint32_t ctrl_bit;
} tv_dict_t;

static tv_dict_t cmd_dict[] =
   {
      {"TV_CTRL",          TV_INTF_OUTPUT_CTRL_OFFSET,         TV_INTF_OUTPUT_STATUS_OFFSET,         TV_INTF_OUTPUT_CHANGE},
      {"HDMI_RES_GROUP",   TV_INTF_HDMI_RES_GROUP_CTRL_OFFSET, TV_INTF_HDMI_RES_GROUP_STATUS_OFFSET, TV_INTF_HDMI_RES_GROUP_CHANGE},
      {"HDMI_RES_CODE",    TV_INTF_HDMI_RES_CODE_CTRL_OFFSET,  TV_INTF_HDMI_RES_CODE_STATUS_OFFSET,  TV_INTF_HDMI_RES_CODE_CHANGE},
      {"HDMI_EDID_BLOCK",  TV_INTF_HDMI_EDID_BLOCK_CTRL_OFFSET,TV_INTF_HDMI_EDID_BLOCK_STATUS_OFFSET,TV_INTF_HDMI_EDID_BLOCK_CHANGE},
      {"HDMI_OPTIONS.ASPECT",                   TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x00, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x00, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"HDMI_OPTIONS.VERTICAL_BAR_PRESENT",     TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x04, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x04, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"HDMI_OPTIONS.LEFT_BAR_WIDTH",           TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x08, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x08, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"HDMI_OPTIONS.RIGHT_BAR_WIDTH",          TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x0C, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x0C, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"HDMI_OPTIONS.HORIZONTAL_BAR_PRESENT",   TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x10, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x10, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"HDMI_OPTIONS.TOP_BAR_HEIGHT",           TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x14, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x14, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"HDMI_OPTIONS.BOTTOM_BAR_HEIGHT",        TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x18, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x18, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"HDMI_OPTIONS.OVERSCAN_FLAGS",           TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x1C, TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x1C, TV_INTF_HDMI_DISPLAY_OPTION_CHANGE},
      {"SDTV_MODE",        TV_INTF_SDTV_MODE_CTRL_OFFSET,         TV_INTF_SDTV_MODE_STATUS_OFFSET,          TV_INTF_SDTV_MODE_CHANGE},
      {"SDTV_ASPECT",      TV_INTF_SDTV_ASPECT_CTRL_OFFSET,       TV_INTF_SDTV_ASPECT_STATUS_OFFSET,        TV_INTF_SDTV_ASPECT_CHANGE},
      {"SDTV_CP_MODE",     TV_INTF_SDTV_MACROVISION_CTRL_OFFSET,  TV_INTF_SDTV_MACROVISION_STATUS_OFFSET,   TV_INTF_SDTV_MACROVISION_CHANGE},
      {NULL, 0}
   };

/* local helper */
static tv_dict_t *dict_lookup(tv_dict_t *dictionary, const char *key)
{
   tv_dict_t *dict_p = dictionary;

   while (dict_p->key != NULL) {
      if (!strcmp(dict_p->key, key))
         return dict_p;
      dict_p++;
   }
   return NULL;
}

static void test_tv_intf_vc_arm_lock(void)
{
    TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_VC_ARM_FLAG1_OFFSET ) = 1;
    TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_VC_ARM_TURN_OFFSET ) = 2;

    while(( TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_VC_ARM_FLAG0_OFFSET ) == 1) &&
          ( TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_VC_ARM_TURN_OFFSET ) == 2))
    {
       //_nop();
        schedule_timeout(1);
    }
}

static void test_tv_intf_vc_arm_unlock( void )
{
     TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_VC_ARM_FLAG1_OFFSET ) = 0;
}

/* global function */

int bcm2835_tv_intf(char *response, int maxlen, const char *format, ...)
{
	va_list args;
	int ret = 0, len;
   char buf[128];
   char *key_p, *value_p;

	tv_intf_print("-enter\n");
	if (!tv_intf_state.initialized) {
		printk(KERN_ERR"tv_intf driver not yet initialized\n");
		ret = -ENXIO;
	    goto done;
	}
	/* mark the tv_intf state as busy */
    down(&tv_intf_state.work_status);
    va_start(args, format);
    len = vsnprintf(buf, 127, format, args );
    va_end(args);
    buf[127] = '\0';

    strcpy(tv_intf_state.cmd_buf, buf );
    // Find beginning of key (strip leading whitespace, convert to uppercase)
    key_p = &buf[0];
    while (isspace(*key_p))
          key_p++;
    value_p = key_p;
    while (isalnum(*value_p) || (*value_p == '_') || (*value_p == '.')) {
        *value_p = toupper(*value_p);
        value_p++;
    }
    if (*value_p != '\0') {
        // skip trailing spaces
        while (isspace(*value_p))
            *value_p++ = '\0'; 
        if (*value_p == '=') {
           unsigned long value;
           tv_dict_t *dict_p = NULL;

           *value_p++ = '\0';
           dict_p = dict_lookup(cmd_dict, key_p);
           value = simple_strtol(value_p, (char**)NULL, 0);

           if (dict_p != NULL) {
               tv_intf_print("Setting %s (offset %x) to %lx\n", dict_p->key, dict_p->ctrl_offset, value );
               TV_INTF_REGISTER_RW(tv_intf_state.base_address, dict_p->ctrl_offset) = value;
               test_tv_intf_vc_arm_lock();
               TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_CTRL_CHANGE_OFFSET) |= dict_p->ctrl_bit;
               test_tv_intf_vc_arm_unlock();
               ipc_notify_vc_event(tv_intf_state.irq);
           }
           else
               tv_intf_print("Unknown value name (%s)\n", key_p);
        }
        else
            tv_intf_print("No \'=\' delimiter found. (key=%s, remaining=%s)\n", key_p, value_p);
    }
    else
       tv_intf_print("No value name found.\n");

	/* mark tv_intf state as free */
	up(&tv_intf_state.work_status);
   ret = -ENXIO;
done:	
	tv_intf_print("-exiting\n");
	return ret;
}
EXPORT_SYMBOL(bcm2835_tv_intf);

/* static functions */
static int dump_tv_intf_ipc_block( char *buffer, char **start, off_t offset, int bytes, int *eof, void *context )
{
   tv_dict_t *dict_p = cmd_dict;
	int len;
	int ret = 0;
   int i;
	*eof = 1;

	len = snprintf(buffer, bytes, "TV_Intf IPC block registers:\n");
	len++;
	ret += len;
	bytes -= len;
	buffer+= len;

   while (dict_p->key != NULL) {
	    len = snprintf(buffer, bytes, "%s=0x%04lx (ctrl=0x%04lx)\n", dict_p->key,
                      TV_INTF_REGISTER_RW(tv_intf_state.base_address, dict_p->status_offset),
                      TV_INTF_REGISTER_RW(tv_intf_state.base_address, dict_p->ctrl_offset));
	    len++;
	    ret += len;
	    bytes -= len;
	    buffer+= len;
       dict_p++;
   }

   // If HDMI is plugged in, dump the EDID information
   if (TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_OUTPUT_STATUS_OFFSET) & TV_INTF_STATUS_HDMI)
   {
      len = snprintf(buffer, bytes, "edid=   ");
      len++;
      ret += len;
      bytes -= len;
      buffer += len;
      for (i=0; i<128; i++)
      {
         if ((i != 0) && ((i & 0x0F) == 0))
         {
            len = snprintf(buffer, bytes, "\n        ");
            len++;
            ret += len;
            bytes -= len;
            buffer += len;
         }
         len = snprintf(buffer, bytes, "%02x ", TV_INTF_REGISTER_RW_BYTE(tv_intf_state.base_address, i+TV_INTF_EDID_OFFSET));
         len++;
         ret += len;
         bytes -= len;
         buffer += len;
      }
   }
   len = snprintf(buffer, bytes, "\nlast=%s\n", tv_intf_state.cmd_buf);
	len++;
	ret += len;
	bytes -= len;
	buffer+= len;

	return ret;
}

int proc_bcm2835_tv_intf(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char tv_intf_req[200], tv_intf_resp[100];
	int ret;

	if( count > (sizeof(tv_intf_req) - 1) ) {
		printk(KERN_ERR"%s:tv_intf max length=%d\n", __func__, sizeof(tv_intf_req));
		return -ENOMEM;
	}
	/* write data to buffer */
	if( copy_from_user( tv_intf_req, buffer, count) ) 
		return -EFAULT;
	tv_intf_req[count] = '\0';

	printk("tv_intf: %s\n",tv_intf_req);
	ret = bcm2835_tv_intf(tv_intf_resp, sizeof(tv_intf_resp), tv_intf_req);

	if ( ret > 0)
		printk(KERN_ERR"response: %s\n", tv_intf_resp);

	return count;
}

int bcm2835_tv_ioctl_get(TV_INTF_IOCTL_CTRLS_T *ctl)
{
    int ret = 0;
    
    ctl->output_ctrl       = TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_OUTPUT_STATUS_OFFSET);
    ctl->hdmi_res_group    = TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_RES_GROUP_STATUS_OFFSET);
    ctl->hdmi_res_code     = TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_RES_CODE_STATUS_OFFSET);
    ctl->hdmi_edid_block   = TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_EDID_BLOCK_STATUS_OFFSET);

    return ret;
}
EXPORT_SYMBOL(bcm2835_tv_ioctl_get);

int bcm2835_tv_ioctl_set(TV_INTF_IOCTL_CTRLS_T *ctl)
{
    uint32_t change_bits = 0;
    int ret = 0;

    if (ctl->output_ctrl != (TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_OUTPUT_CTRL_OFFSET) & 0xFFFF)) {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_OUTPUT_CTRL_OFFSET) = ctl->output_ctrl;
        change_bits |= TV_INTF_OUTPUT_CHANGE;
        tv_intf_print("Setting TV_INTF_OUTPUT_CTRL to %x\n", ctl->output_ctrl);
    }
    if (ctl->hdmi_res_group != TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_RES_GROUP_CTRL_OFFSET)) {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_RES_GROUP_CTRL_OFFSET) = ctl->hdmi_res_group;
        change_bits |= TV_INTF_HDMI_RES_GROUP_CHANGE;
        tv_intf_print("Setting TV_INTF_HDMI_RES_GROUP_CTRL to %x\n", ctl->hdmi_res_group);
    }
    if (ctl->hdmi_res_code != TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_RES_CODE_CTRL_OFFSET)) {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_RES_CODE_CTRL_OFFSET) = ctl->hdmi_res_code;
        change_bits |= TV_INTF_HDMI_RES_CODE_CHANGE;
        tv_intf_print("Setting TV_INTF_HDMI_RES_CODE_CTRL to %x\n", ctl->hdmi_res_code);
    }
    if (ctl->hdmi_edid_block != TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_EDID_BLOCK_CTRL_OFFSET)) {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_HDMI_EDID_BLOCK_CTRL_OFFSET) = ctl->hdmi_edid_block;
        change_bits |= TV_INTF_HDMI_EDID_BLOCK_CHANGE;
        tv_intf_print("Setting TV_INTF_HDMI_EDID_BLOCK_CTRL to %x\n", ctl->hdmi_edid_block);
    }
    if (change_bits) {
        tv_intf_print("Setting change bits (%x) and ringing doorbell\n", change_bits);
        test_tv_intf_vc_arm_lock();
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_CTRL_CHANGE_OFFSET) |= change_bits;
        test_tv_intf_vc_arm_unlock();
        ipc_notify_vc_event(tv_intf_state.irq);
    }

   return ret;
}
EXPORT_SYMBOL(bcm2835_tv_ioctl_set);

int bcm2835_tv_ioctl_edid_get(TV_INTF_IOCTL_EDID_T *edid)
{
   int ret = 0;

   memcpy(&edid->edid, (uint8_t *)&TV_INTF_REGISTER_RW_BYTE(tv_intf_state.base_address, TV_INTF_EDID_OFFSET), 128);
   return ret;
}
EXPORT_SYMBOL(bcm2835_tv_ioctl_edid_get);

int bcm2835_tv_ioctl_hdmi_opt_get(TV_INTF_IOCTL_HDMI_DISPLAY_OPTIONS_T *opt_p)
{
    int ret=0;

    opt_p->aspect                 = (TV_INTF_IOCTL_HDMI_ASPECT_T)TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x00);
    opt_p->vertical_bar_present   = TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x04);
    opt_p->left_bar_width         = TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x08);
    opt_p->right_bar_width        = TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x0C);
    opt_p->horizontal_bar_present = TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x10);
    opt_p->top_bar_height         = TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x14);
    opt_p->bottom_bar_height      = TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x18);
    opt_p->overscan_flags         = TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET+0x1c);
    return ret;
}
EXPORT_SYMBOL(bcm2835_tv_ioctl_hdmi_opt_get);

int bcm2835_tv_ioctl_hdmi_opt_set(TV_INTF_IOCTL_HDMI_DISPLAY_OPTIONS_T *opt_p)
{
    uint32_t change_bits = 0;
    int ret = 0;
    TV_INTF_IOCTL_HDMI_DISPLAY_OPTIONS_T cur;

    bcm2835_tv_ioctl_hdmi_opt_get(&cur);

    if ((opt_p->aspect != cur.aspect)                                  ||
        (opt_p->vertical_bar_present != cur.vertical_bar_present)      ||
        (opt_p->left_bar_width != cur.left_bar_width)                  ||
        (opt_p->right_bar_width != cur.right_bar_width)                ||
        (opt_p->horizontal_bar_present != cur.horizontal_bar_present)  ||
        (opt_p->top_bar_height != cur.top_bar_height)                  ||
        (opt_p->bottom_bar_height != cur.bottom_bar_height)            ||
        (opt_p->overscan_flags != cur.overscan_flags))
    {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x00) = opt_p->aspect;
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x04) = opt_p->vertical_bar_present;
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x08) = opt_p->left_bar_width;
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x0C) = opt_p->right_bar_width;
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x10) = opt_p->horizontal_bar_present;
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x14) = opt_p->top_bar_height;
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x18) = opt_p->bottom_bar_height;
        TV_INTF_REGISTER_RW(tv_intf_state.base_address,TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET+0x1C) = opt_p->overscan_flags;
        change_bits |= TV_INTF_HDMI_DISPLAY_OPTION_CHANGE;
    }

    if (change_bits) {
        tv_intf_print("Setting change bits (%x) and ringing doorbell\n", change_bits);
        test_tv_intf_vc_arm_lock();
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_CTRL_CHANGE_OFFSET) |= change_bits;
        test_tv_intf_vc_arm_unlock();
        ipc_notify_vc_event(tv_intf_state.irq);
    }
    return ret;
}
EXPORT_SYMBOL(bcm2835_tv_ioctl_hdmi_opt_set);

int bcm2835_tv_ioctl_sdtv_ctrls_get(TV_INTF_IOCTL_SDTV_CTRLS_T *ctrls_p)
{
    int ret=0;

    ctrls_p->mode    = (TV_INTF_IOCTL_SDTV_MODE_T)TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_SDTV_MODE_STATUS_OFFSET);
    ctrls_p->aspect  = (TV_INTF_IOCTL_SDTV_ASPECT_T)TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_SDTV_ASPECT_STATUS_OFFSET);
    ctrls_p->cp_mode = (TV_INTF_IOCTL_SDTV_MODE_T)TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_SDTV_MACROVISION_STATUS_OFFSET);

    return ret;
}
EXPORT_SYMBOL(bcm2835_tv_ioctl_sdtv_ctrls_get);

int bcm2835_tv_ioctl_sdtv_ctrls_set(TV_INTF_IOCTL_SDTV_CTRLS_T *ctrl_p)
{
    uint32_t change_bits = 0;
    int ret = 0;
    TV_INTF_IOCTL_SDTV_CTRLS_T cur;

    bcm2835_tv_ioctl_sdtv_ctrls_get(&cur);

    if (ctrl_p->mode != cur.mode)    {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_SDTV_MODE_CTRL_OFFSET) = ctrl_p->mode;
        change_bits |= TV_INTF_SDTV_MODE_CHANGE;
    }
    if (ctrl_p->aspect != cur.aspect)
    {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_SDTV_ASPECT_CTRL_OFFSET) = ctrl_p->aspect;
        change_bits |= TV_INTF_SDTV_ASPECT_CHANGE;
    }
    if (ctrl_p->cp_mode != cur.cp_mode)
    {
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_SDTV_MACROVISION_CTRL_OFFSET) = ctrl_p->cp_mode;
        change_bits |= TV_INTF_SDTV_MACROVISION_CHANGE;
    }

    if (change_bits) {
        tv_intf_print("Setting change bits (%x) and ringing doorbell\n", change_bits);
        test_tv_intf_vc_arm_lock();
        TV_INTF_REGISTER_RW(tv_intf_state.base_address, TV_INTF_CTRL_CHANGE_OFFSET) |= change_bits;
        test_tv_intf_vc_arm_unlock();
        ipc_notify_vc_event(tv_intf_state.irq);
    }
    return ret;
}
EXPORT_SYMBOL(bcm2835_tv_ioctl_sdtv_ctrls_set);

static irqreturn_t bcm2835_tv_intf_isr(int irq, void *dev_id)
{
	tv_intf_print("-enter");
	up( &tv_intf_state.command_complete);
	tv_intf_print("-done");
	
	return IRQ_HANDLED;
}

static int __devexit bcm2835_tv_intf_remove(struct platform_device *pdev)
{
	if( tv_intf_state.initialized ) {
		tv_intf_driver_exit();
		remove_proc_entry("bcm2835_tvif", NULL);
		free_irq(tv_intf_state.irq, NULL);
		release_region(tv_intf_state.mem_res->start, resource_size(tv_intf_state.mem_res));
		tv_intf_state.initialized = 0;
	}

	return 0;
}

static int __devinit bcm2835_tv_intf_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret = -ENOENT;

	tv_intf_print("probe=%p\n", pdev);
			
	/* initialize the tv_intf struct */
	memset(&tv_intf_state, 0, sizeof(tv_intf_state));

	/* get the tv_intf memory region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		ret = -ENODEV;
		goto err_platform_res;
	}

	tv_intf_print("tv_intf memory region start=%p end=%p\n", (void *)res->start,
			(void *)res->end);

	tv_intf_state.base_address = (void __iomem *)(res->start);

	/* Request memory region */
	tv_intf_state.mem_res = request_mem_region_exclusive(res->start, resource_size(res), "bcm2835tv_intf_regs");
	if (tv_intf_state.mem_res == NULL) {
		dev_err(&pdev->dev, "Unable to request tv_intf memory region\n");
		ret = -ENOMEM;
		goto err_mem_region;

	}
	
	/* find our ipc id */
	tv_intf_state.irq = platform_get_irq(pdev, 0);

	if( tv_intf_state.irq < 0 ) {
		dev_err(&pdev->dev, "failed to get irq for tv_intf fourcc=0x%x\n", TV_INTF_FOURCC);
		ret = -ENODEV;
		goto err_irq;
	}

	tv_intf_print("tv_intf irqno =%d\n", tv_intf_state.irq);

	/* initialize the semaphores */
	sema_init( &tv_intf_state.work_status, 1);
	sema_init( &tv_intf_state.command_complete, 0);

	/* register irq for tv_intf service */
	ret = request_irq( tv_intf_state.irq, bcm2835_tv_intf_isr,
			IRQF_DISABLED, "bcm2835 tv_intf interrupt", NULL);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to install tv_intf irq handler(%d)\n",
				tv_intf_state.irq);
		ret = -ENOENT;
		goto err_irq_handler;
	}

	/* create a proc entry */
	tv_intf_state.dump_info = create_proc_entry("bcm2835_tvif",
			(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP ), NULL);

	if( !tv_intf_state.dump_info ) {
		dev_err(&pdev->dev, "failed to create bcm2835_tv_intf proc entry\n");
		ret = -ENOENT;
		goto err_proc_entry;
	}
	else {
		tv_intf_state.dump_info->write_proc = proc_bcm2835_tv_intf;
		tv_intf_state.dump_info->read_proc = dump_tv_intf_ipc_block;
	}

	/* register the driver */
	ret = tv_intf_driver_init();
	if( ret < 0 ) {
		dev_err(&pdev->dev, "failed to register the driver\n");
		ret = -ENOENT;
		goto err_driver;
	}

	tv_intf_state.initialized = 1;
	return 0;

err_driver:
	/* remove proc entry */
	remove_proc_entry("bcm2835_tvif", NULL);
err_proc_entry:
	/* free irq */
	free_irq(tv_intf_state.irq, NULL);
err_irq_handler:
err_irq:
	/* release region */
	release_region(tv_intf_state.mem_res->start, resource_size(tv_intf_state.mem_res));
err_mem_region:
err_platform_res:	
	return ret;
}

static struct platform_driver bcm2835_tv_intf_driver = {
	.probe		= bcm2835_tv_intf_probe,
	.remove		= __devexit_p(bcm2835_tv_intf_remove),
	.driver		= {
		.name = "bcm2835_TVIF",
		.owner = THIS_MODULE,
	},
};

static char __initdata banner[] = "VC4 TV_Intf Driver,(C) 2010 Broadcom Corp\n";

static int __init bcm2835_tv_intf_init( void )
{
	printk(KERN_ERR"%s", banner);
	return platform_driver_register(&bcm2835_tv_intf_driver);
}

static void __exit bcm2835_tv_intf_exit( void )
{
	platform_driver_unregister(&bcm2835_tv_intf_driver);
}

module_init(bcm2835_tv_intf_init);
module_exit(bcm2835_tv_intf_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("TV_Intf Kernel Driver");

/* ************************************ The End ***************************************** */

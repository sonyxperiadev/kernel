/*
 *  gencmd_kernel.c - core gencmd driver
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
#include <linux/ipc/ipc.h>

#include "gencmd.h"
#include "gencmd_regs.h"
#include "gencmd_driver.h"

/* macros */
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned int)(unsigned char)(ch0) \
		| ((unsigned int)(unsigned char)(ch1) << 8) \
		| ((unsigned int)(unsigned char)(ch2) << 16) \
		| ((unsigned int)(unsigned char)(ch3) << 24 ))

#define GENCMD_FOURCC MAKEFOURCC( 'G', 'E', 'N', 'C')

/* structure decalaration */
static struct {
	volatile unsigned char *base_address;
	struct resource *mem_res;
	unsigned int irq;
	struct proc_dir_entry *dump_info;
	struct semaphore work_status; /* 0 = busy processing, 1 = free */
	struct semaphore command_complete;
	int initialized;
}gencmd_state;

/* global function */

int vc_gencmd(char *response, int maxlen, const char *format, ...)
{
	va_list args;
	int ret = 0,len;

	gencmd_print("-enter\n");
	if (!gencmd_state.initialized) {
		printk(KERN_ERR"gencmd driver not yet initialized\n");
		ret = -ENXIO;
	    goto done;
	}

	/* mark the gencmd state as busy */
    down(&gencmd_state.work_status);

	va_start(args, format);
	len = vsnprintf((char *)(gencmd_state.base_address+GENCMD_REQUEST_DATA_OFFSET) , GENCMD_MAX_LENGTH, format, args );
	len++;
	GENCMD_REGISTER_RW(gencmd_state.base_address, GENCMD_REQUEST_LENGTH_OFFSET) = len;

	/* ping videocore to process gencmd */
	GENCMD_REGISTER_RW(gencmd_state.base_address, GENCMD_CONTROL_OFFSET) = GENCMD_CONTROL_GO;
	
	/* ring the doorbell */
	ipc_notify_vc_event(gencmd_state.irq);
	
	/* wait for the done bit */
	gencmd_print("-waiting\n");
	ret = down_interruptible(&gencmd_state.command_complete);
	if (ret < 0) {
		printk(KERN_ERR"Failed to get the semaphore error code=%d\n", ret);
		goto release_sem;
	}

	while( !(GENCMD_REGISTER_RW(gencmd_state.base_address, GENCMD_CONTROL_OFFSET) & GENCMD_CONTROL_DONE) );
	gencmd_print("-done waiting\n");

	if( response ) {
		len = GENCMD_REGISTER_RW(gencmd_state.base_address, GENCMD_REPLY_LENGTH_OFFSET);
		if( len > maxlen)
			len = maxlen;
		strncpy(response, (char *)(gencmd_state.base_address + GENCMD_REPLY_DATA_OFFSET), len);
		gencmd_print("-resposne=%s\n", response);
		ret = len;
	}

	/* clear the done bit */
	GENCMD_REGISTER_RW(gencmd_state.base_address, GENCMD_CONTROL_OFFSET) = 0x0;//(~GENCMD_CONTROL_DONE);
	va_end(args);
release_sem:
	/* mark gencmd state as free */
	up(&gencmd_state.work_status);

done:	
	gencmd_print("-exiting\n");
	return ret;
}
EXPORT_SYMBOL(vc_gencmd);

/* static functions */
static int dump_gencmd_ipc_block( char *buffer, char **start, off_t offset, int bytes, int *eof, void *context )
{
	int len;
	int ret = 0;
	*eof = 1;

	len = snprintf(buffer, bytes, "Gencmd IPC block registers:\n");
	len++;
	ret += len;
	bytes -= len;
	buffer+= len;

	len = snprintf(buffer, bytes, "Control:0x%lx\n", GENCMD_REGISTER_RW(gencmd_state.base_address, GENCMD_CONTROL_OFFSET) );
	len++;
	ret += len;
	bytes -= len;
	buffer+= len;

	len = snprintf(buffer, bytes, "Last Gencmd:%s\n", (char *)((char *)gencmd_state.base_address + GENCMD_REQUEST_DATA_OFFSET));
	len++;
	ret += len;
	bytes -= len;
	buffer+= len;

	len = snprintf(buffer, bytes, "Last Gencmd Response:%s\n", (char *)((char *)gencmd_state.base_address + GENCMD_REPLY_DATA_OFFSET));
	len++;
	ret += len;
	bytes -= len;
	buffer+= len;

	return ret;
}

int proc_vc_gencmd(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char gencmd_req[200], gencmd_resp[100];
	int ret;

	if( count > (sizeof(gencmd_req) - 1) ) {
		printk(KERN_ERR"%s:gencmd max length=%d\n", __func__, sizeof(gencmd_req));
		return -ENOMEM;
	}
	/* write data to buffer */
	if( copy_from_user( gencmd_req, buffer, count) ) 
		return -EFAULT;
	gencmd_req[count] = '\0';

	printk("gencmd: %s\n",gencmd_req);
	ret = vc_gencmd(gencmd_resp, sizeof(gencmd_resp), gencmd_req);

	if ( ret > 0)
		printk(KERN_ERR"response: %s\n", gencmd_resp);

	return count;
}

static irqreturn_t bcm2708_gencmd_isr(int irq, void *dev_id)
{
	gencmd_print("-enter");
	up( &gencmd_state.command_complete);
	gencmd_print("-done");
	
	return IRQ_HANDLED;
}

static int __devexit bcm2708_gencmd_remove(struct platform_device *pdev)
{
	if( gencmd_state.initialized ) {
		gencmd_driver_exit();
		remove_proc_entry("vc_gencmd", NULL);
		free_irq(gencmd_state.irq, NULL);
		release_region(gencmd_state.mem_res->start, resource_size(gencmd_state.mem_res));
		gencmd_state.initialized = 0;
	}

	return 0;
}

static int __devinit bcm2708_gencmd_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret = -ENOENT;

	gencmd_print("probe=%p\n", pdev);
			
	/* initialize the gencmd struct */
	memset(&gencmd_state, 0, sizeof(gencmd_state));

	/* get the gencmd memory region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		ret = -ENODEV;
		goto err_platform_res;
	}

	gencmd_print("gencmd memory region start=%p end=%p\n", (void *)res->start,
			(void *)res->end);

	gencmd_state.base_address = (void __iomem *)(res->start);

	/* Request memory region */
	gencmd_state.mem_res = request_mem_region_exclusive(res->start, resource_size(res), "bcm2708gencmd_regs");
	if (gencmd_state.mem_res == NULL) {
		dev_err(&pdev->dev, "Unable to request gencmd memory region\n");
		ret = -ENOMEM;
		goto err_mem_region;

	}
	
	/* find our ipc id */
	gencmd_state.irq = platform_get_irq(pdev, 0);

	if( gencmd_state.irq < 0 ) {
		dev_err(&pdev->dev, "failed to get irq for gencmd fourcc=0x%x\n", GENCMD_FOURCC);
		ret = -ENODEV;
		goto err_irq;
	}

	gencmd_print("gencmd irqno =%d\n", gencmd_state.irq);

	/* initialize the semaphores */
	sema_init( &gencmd_state.work_status, 1);
	sema_init( &gencmd_state.command_complete, 0);

	/* register irq for gencmd service */
	ret = request_irq( gencmd_state.irq, bcm2708_gencmd_isr,
			IRQF_DISABLED, "bcm2708 gencmd interrupt", NULL);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to install gencmd irq handler(%d)\n",
				gencmd_state.irq);
		ret = -ENOENT;
		goto err_irq_handler;
	}

	/* create a proc entry */
	gencmd_state.dump_info = create_proc_entry("vc_gencmd",
			(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP ), NULL);

	if( !gencmd_state.dump_info ) {
		dev_err(&pdev->dev, "failed to create vc_gencmd proc entry\n");
		ret = -ENOENT;
		goto err_proc_entry;
	}
	else {
		gencmd_state.dump_info->write_proc = proc_vc_gencmd;
		gencmd_state.dump_info->read_proc = dump_gencmd_ipc_block;
	}

	/* register the driver */
	ret = gencmd_driver_init();
	if( ret < 0 ) {
		dev_err(&pdev->dev, "failed to register the driver\n");
		ret = -ENOENT;
		goto err_driver;
	}

	gencmd_state.initialized = 1;
	return 0;

err_driver:
	/* remove proc entry */
	remove_proc_entry("vc_gencmd", NULL);
err_proc_entry:
	/* free irq */
	free_irq(gencmd_state.irq, NULL);
err_irq_handler:
err_irq:
	/* release region */
	release_region(gencmd_state.mem_res->start, resource_size(gencmd_state.mem_res));
err_mem_region:
err_platform_res:	
	return ret;
}

static struct platform_driver bcm2708_gencmd_driver = {
	.probe		= bcm2708_gencmd_probe,
	.remove		= __devexit_p(bcm2708_gencmd_remove),
	.driver		= {
		.name = "bcm2835_GENC",
		.owner = THIS_MODULE,
	},
};

static char __initdata banner[] = "VC4 Gencmd Driver,(C) 2010 Broadcom Corp\n";

static int __init bcm2708_gencmd_init( void )
{
	printk(KERN_ERR"%s", banner);
	return platform_driver_register(&bcm2708_gencmd_driver);
}

static void __exit bcm2708_gencmd_exit( void )
{
	platform_driver_unregister(&bcm2708_gencmd_driver);
}

module_init(bcm2708_gencmd_init);
module_exit(bcm2708_gencmd_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("GenCmd Kernel Driver");

/* ************************************ The End ***************************************** */

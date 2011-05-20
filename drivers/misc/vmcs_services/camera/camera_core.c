/*
 *  camera_core.c - core ipc camera driver
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

#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/memory.h>

#include "camera.h"
#include "camera_regs.h"
#include "camera_driver.h"

/* macros */
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((unsigned int)(unsigned char)(ch0) \
		| ((unsigned int)(unsigned char)(ch1) << 8) \
		| ((unsigned int)(unsigned char)(ch2) << 16) \
		| ((unsigned int)(unsigned char)(ch3) << 24 ))

#define CAMERA_FOURCC MAKEFOURCC( 'C', 'A', 'M', 'R')

/* structure decalaration */
static struct {
	volatile unsigned char *base_address;
	struct resource *mem_res;
	unsigned int irq;
	struct proc_dir_entry *dump_info;
	struct semaphore work_status; /* 0 = busy processing, 1 = free */
	struct semaphore command_complete;
	int initialized;
}camera_state;

/* global function */

int vc_camera(char *response, int maxlen, const char *format, ...)
{
	int ret = 0;

	camera_print("-enter\n");
	if (!camera_state.initialized) {
		printk(KERN_ERR"camera driver not yet initialized\n");
		ret = -ENXIO;
		goto done;
	}

	/* mark the camera state as busy */
	down(&camera_state.work_status);

	/* ring the doorbell */
	ipc_notify_vc_event(camera_state.irq);

	/* mark camera state as free */
	up(&camera_state.work_status);

done:
	camera_print("-exiting\n");
	return ret;
}
EXPORT_SYMBOL(vc_camera);

int vc_camera_control(uint32_t enable)
{
	int ret = 0;
	uint32_t current_state = CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) & CAMERA_CONTROL_ENABLE_BIT;

	if( enable == current_state){
		camera_print(KERN_ERR"Camera already in required state\n");
		return ret;
	}
	if( enable )
		CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) |= CAMERA_CONTROL_ENABLE_BIT;
	else
		CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) &= ~CAMERA_CONTROL_ENABLE_BIT;

	/* mark the camera state as busy */
	down(&camera_state.work_status);

	/* ring the doorbell */
	printk(KERN_ERR"%s: Ringing the doorbell with enable=%d\n", __func__, enable);
	ipc_notify_vc_event(camera_state.irq);

	/* wait for the done bit */
	camera_print("-waiting\n");
	ret = down_interruptible(&camera_state.command_complete);
	if (ret < 0) {
		printk(KERN_ERR"Failed to get the semaphore error code=%d\n", ret);
	}

	/* mark camera state as free */
	up(&camera_state.work_status);

	return ret;
}
EXPORT_SYMBOL(vc_camera_control);

int vc_camera_take_picture(uint32_t width, uint32_t height, uint32_t max_size, void *memory)
{
	int ret = 0;
	uint32_t viewfinder_state = CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) & CAMERA_CONTROL_ENABLE_BIT;
	uint32_t capture_state = CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) & CAMERA_CONTROL_CAPTURE_BIT;
	uint32_t capture_addr;
	resource_size_t jpeg_size;
	void __iomem *jpeg_addr;

	if( !viewfinder_state){
		camera_print(KERN_ERR"Camera not running: can't take picture\n");
		return ret;
	}

	if(capture_state){
		camera_print(KERN_ERR"Camera capture in progress: try later\n");
		return ret;
	}

	camera_print("Setting the capture enable bit\n");
	CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) |= CAMERA_CONTROL_CAPTURE_BIT;

	/* mark the camera state as busy */
	down(&camera_state.work_status);

	/* ring the doorbell */
	capture_state = CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) & CAMERA_CONTROL_CAPTURE_BIT;
	printk(KERN_ERR"%s: Ringing the doorbell with capture %s\n", __func__, capture_state? "enable":"disable");
	ipc_notify_vc_event(camera_state.irq);

	/* wait for the done bit */
	camera_print("-waiting\n");
	ret = down_interruptible(&camera_state.command_complete);
	if (ret < 0) {
		printk(KERN_ERR"Failed to get the semaphore error code=%d\n", ret);
	}

	/* Capture done: copy the jpeg buffer */
	capture_addr = CAMERA_REGISTER_RW(camera_state.base_address, CAMERA_IMAGE_BUFFER_ADDRESS_OFFSET);
	jpeg_size =  CAMERA_REGISTER_RW(camera_state.base_address, CAMERA_IMAGE_SIZE_OFFSET);
	camera_print("Got jpeg buffer @ 0X%x and size = %d\n", capture_addr, jpeg_size);
#define VC_BASE_ADDR_IN_ARM_MAP UL(0x40000000) /* offset to the VC physical 0x0 in ARM's view */
#define VC_CACHE UL(0xC0000000) /* offset to the VC physical 0x0 in ARM's view */

	capture_addr = capture_addr - VC_CACHE + VC_BASE_ADDR_IN_ARM_MAP;

	if( !capture_addr || !jpeg_size || (jpeg_size > max_size)){
		printk(KERN_ERR"[%s] Capture Error\n", __func__);
		goto capture_error;
	}

	camera_print("Got jpeg buffer @ 0X%x and size = %d\n", capture_addr, jpeg_size);
	/* Request the jpeg buffer and ioremap it */
	if (!request_mem_region(capture_addr, jpeg_size, "jpeg_image")) {
		printk(KERN_ERR"Failed to request the capture region\n");
		goto capture_error;
	}

	jpeg_addr= ioremap(capture_addr, jpeg_size);
	if (!jpeg_addr) {
		printk(KERN_ERR"Failed to get the ioremapped address region\n");
		goto map_error;
	}

	printk(KERN_ERR"Copying data from %p to %p with size = %d", jpeg_addr, memory,jpeg_size);
	/* Copy the jpeg buffer from VC in user buffer */
	ret = copy_to_user(memory, jpeg_addr, jpeg_size);
	if( ret < 0 ) {
		printk(KERN_ERR"[%s] error in copying date to user buffer\n", __func__);
		goto copy_error;
	}

copy_error:
	iounmap(jpeg_addr);

map_error:
	release_mem_region(capture_addr, jpeg_size);

capture_error:
	/* Disable the caprute and ring the doorbell */
	CAMERA_REGISTER_RW(camera_state.base_address, CAMERA_CONTROL_OFFSET ) &= (~CAMERA_CONTROL_CAPTURE_BIT);
	capture_state = CAMERA_REGISTER_RW( camera_state.base_address, CAMERA_CONTROL_OFFSET ) & CAMERA_CONTROL_CAPTURE_BIT;
	printk(KERN_ERR"%s: Ringing the doorbell with capture %s\n", __func__, capture_state? "enable":"disable");
	ipc_notify_vc_event(camera_state.irq);

	/* mark camera state as free */
	up(&camera_state.work_status);

	return ret;
}
EXPORT_SYMBOL(vc_camera_take_picture);

/* static functions */
static int dump_camera_ipc_block( char *buffer, char **start, off_t offset, int bytes, int *eof, void *context )
{
	int ret = 0;

	return ret;
}

int proc_vc_camera(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char camera_req[200], camera_resp[100];
	int ret;
	static int enable = 0;

	if( count > (sizeof(camera_req) - 1) ) {
		printk(KERN_ERR"%s:camera max length=%d\n", __func__, sizeof(camera_req));
		return -ENOMEM;
	}
	/* write data to buffer */
	if( copy_from_user( camera_req, buffer, count) )
		return -EFAULT;
	camera_req[count] = '\0';

	printk("camera: %s\n",camera_req);
	ret = vc_camera_control(enable);

	if(enable)
		enable = 0;
	else
		enable = 1;

	if ( ret > 0)
		printk(KERN_ERR"response: %s\n", camera_resp);

	return count;
}

static irqreturn_t bcm2708_camera_isr(int irq, void *dev_id)
{
	camera_print("-enter");
	up( &camera_state.command_complete);
	camera_print("-done");

	return IRQ_HANDLED;
}

static int __devexit bcm2708_camera_remove(struct platform_device *pdev)
{
	if( camera_state.initialized ) {
		camera_driver_exit();
		remove_proc_entry("vc_camera", NULL);
		free_irq(camera_state.irq, NULL);
		release_region(camera_state.mem_res->start, resource_size(camera_state.mem_res));
		camera_state.initialized = 0;
	}

	return 0;
}

static int __devinit bcm2708_camera_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret = -ENOENT;

	camera_print("probe=%p\n", pdev);

	/* initialize the camera struct */
	memset(&camera_state, 0, sizeof(camera_state));

	/* get the camera memory region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		ret = -ENODEV;
		goto err_platform_res;
	}

	camera_print("camera memory region start=%p end=%p\n", (void *)res->start,
			(void *)res->end);

	camera_state.base_address = (void __iomem *)(res->start);

	/* Request memory region */
	camera_state.mem_res = request_mem_region_exclusive(res->start, resource_size(res), "bcm2708camera_regs");
	if (camera_state.mem_res == NULL) {
		dev_err(&pdev->dev, "Unable to request camera memory region\n");
		ret = -ENOMEM;
		goto err_mem_region;

	}

	/* find our ipc id */
	camera_state.irq = platform_get_irq(pdev, 0);

	if( camera_state.irq < 0 ) {
		dev_err(&pdev->dev, "failed to get irq for camera fourcc=0x%x\n", CAMERA_FOURCC);
		ret = -ENODEV;
		goto err_irq;
	}

	camera_print("camera irqno =%d\n", camera_state.irq);

	/* initialize the semaphores */
	sema_init( &camera_state.work_status, 1);
	sema_init( &camera_state.command_complete, 0);

	/* register irq for camera service */
	ret = request_irq( camera_state.irq, bcm2708_camera_isr,
			IRQF_DISABLED, "bcm2708 camera interrupt", NULL);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to install camera irq handler(%d)\n",
				camera_state.irq);
		ret = -ENOENT;
		goto err_irq_handler;
	}

	/* create a proc entry */
	camera_state.dump_info = create_proc_entry("vc_camera",
			(S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP ), NULL);

	if( !camera_state.dump_info ) {
		dev_err(&pdev->dev, "failed to create vc_camera proc entry\n");
		ret = -ENOENT;
		goto err_proc_entry;
	}
	else {
		camera_state.dump_info->write_proc = proc_vc_camera;
		camera_state.dump_info->read_proc = dump_camera_ipc_block;
	}

	/* register the driver */
	ret = camera_driver_init();
	if( ret < 0 ) {
		dev_err(&pdev->dev, "failed to register the driver\n");
		ret = -ENOENT;
		goto err_driver;
	}

	camera_state.initialized = 1;
	return 0;

err_driver:
	/* remove proc entry */
	remove_proc_entry("vc_camera", NULL);
err_proc_entry:
	/* free irq */
	free_irq(camera_state.irq, NULL);
err_irq_handler:
err_irq:
	/* release region */
	release_region(camera_state.mem_res->start, resource_size(camera_state.mem_res));
err_mem_region:
err_platform_res:
	return ret;
}

static struct platform_driver bcm2708_camera_driver = {
	.probe		= bcm2708_camera_probe,
	.remove		= __devexit_p(bcm2708_camera_remove),
	.driver		= {
		.name = "bcm2835_CAMR",
		.owner = THIS_MODULE,
	},
};

static char __initdata banner[] = "VC4 Camera Driver,(C) 2010 Broadcom Corp\n";

static int __init bcm2708_camera_init( void )
{
	printk(KERN_ERR"%s", banner);
	return platform_driver_register(&bcm2708_camera_driver);
}

static void __exit bcm2708_camera_exit( void )
{
	platform_driver_unregister(&bcm2708_camera_driver);
}

module_init(bcm2708_camera_init);
module_exit(bcm2708_camera_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Camera Kernel Driver");

/* ************************************ The End ***************************************** */

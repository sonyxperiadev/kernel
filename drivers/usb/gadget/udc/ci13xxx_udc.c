/*
 * ci13xxx_udc.c - MIPS USB IP core family device controller
 *
 * Copyright (C) 2008 Chipidea - MIPS Technologies, Inc. All rights reserved.
 *
 * Author: David Lopo
 *
 * Adaptation for QC MSM Chipidea controller based on mainline driver
 * Copyright (C) 2017 AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Description: MIPS USB IP core family device controller
 *              Currently it only supports IP part number CI13412
 *
 * This driver is composed of several blocks:
 * - HW:     hardware interface
 * - DBG:    debug facilities (optional)
 * - UTIL:   utilities
 * - ISR:    interrupts handling
 * - ENDPT:  endpoint operations (Gadget API)
 * - GADGET: gadget operations (Gadget API)
 * - BUS:    bus glue code, bus abstraction layer
 *
 * Compile Options
 * - CONFIG_USB_GADGET_DEBUG_FILES: enable debug facilities
 *
 * Main Features
 * - Chapter 9 & Mass Storage Compliance with Gadget File Storage
 * - Chapter 9 Compliance with Gadget Zero
 * - Normal & LPM support
 *
 * USBTEST Report
 * - OK: 0-12, 13 (STALL_IN defined) & 14
 * - Not Supported: 15 & 16 (ISO)
 *
 * TODO List
 * - GET_STATUS(device) - always reports 0
 * - Gadget API (majority of optional features)
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/ratelimit.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/pm_runtime.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include <linux/usb/msm_hsusb.h>

#include "ci13xxx_udc.h"

/* TODO: Implement BAM2BAM for RNDIS and advanced QC functions */
//#define USE_BAM2BAM

/******************************************************************************
 * DEFINE
 *****************************************************************************/

#define USB_MAX_TIMEOUT		25 /* 25msec timeout */
#define EP_PRIME_CHECK_DELAY	(jiffies + msecs_to_jiffies(1000))
#define MAX_PRIME_CHECK_RETRY	3 /*Wait for 3sec for EP prime failure */
#define EXTRA_ALLOCATION_SIZE	256

/* control endpoint description */
static const struct usb_endpoint_descriptor
ctrl_endpt_out_desc = {
	.bLength         = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes    = USB_ENDPOINT_XFER_CONTROL,
	.wMaxPacketSize  = cpu_to_le16(CTRL_PAYLOAD_MAX),
};

static const struct usb_endpoint_descriptor
ctrl_endpt_in_desc = {
	.bLength         = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes    = USB_ENDPOINT_XFER_CONTROL,
	.wMaxPacketSize  = cpu_to_le16(CTRL_PAYLOAD_MAX),
};

/* UDC descriptor */
static struct ci13xxx *_udc;

/******************************************************************************
 * HW block
 *****************************************************************************/

#define REMOTE_WAKEUP_DELAY	msecs_to_jiffies(200)

/* maximum number of enpoints: valid only after hw_device_reset() */
static void dbg_usb_op_fail(u8 addr, const char *name,
				const struct ci13xxx_ep *mep);

/* MSM specific */
#define ABS_AHBBURST        (0x0090UL)
#define ABS_AHBMODE         (0x0098UL)

/* UDC register map */
static uintptr_t ci_regs_nolpm[] = {
	[CAP_CAPLENGTH]		= 0x000UL,
	[CAP_HCCPARAMS]		= 0x008UL,
	[CAP_DCCPARAMS]		= 0x024UL,
	[CAP_TESTMODE]		= 0x038UL,
	[OP_USBCMD]		= 0x000UL,
	[OP_USBSTS]		= 0x004UL,
	[OP_USBINTR]		= 0x008UL,
	[OP_DEVICEADDR]		= 0x014UL,
	[OP_ENDPTLISTADDR]	= 0x018UL,
	[OP_PORTSC]		= 0x044UL,
	[OP_DEVLC]		= 0x084UL,
	[OP_ENDPTPIPEID]	= 0x0BCUL,
	[OP_USBMODE]		= 0x068UL,
	[OP_ENDPTSETUPSTAT]	= 0x06CUL,
	[OP_ENDPTPRIME]		= 0x070UL,
	[OP_ENDPTFLUSH]		= 0x074UL,
	[OP_ENDPTSTAT]		= 0x078UL,
	[OP_ENDPTCOMPLETE]	= 0x07CUL,
	[OP_ENDPTCTRL]		= 0x080UL,
};

static uintptr_t ci_regs_lpm[] = {
	[CAP_CAPLENGTH]		= 0x000UL,
	[CAP_HCCPARAMS]		= 0x008UL,
	[CAP_DCCPARAMS]		= 0x024UL,
	[CAP_TESTMODE]		= 0x0FCUL,
	[OP_USBCMD]		= 0x000UL,
	[OP_USBSTS]		= 0x004UL,
	[OP_USBINTR]		= 0x008UL,
	[OP_DEVICEADDR]		= 0x014UL,
	[OP_ENDPTLISTADDR]	= 0x018UL,
	[OP_PORTSC]		= 0x044UL,
	[OP_DEVLC]		= 0x084UL,
	[OP_ENDPTPIPEID]	= 0x0BCUL,
	[OP_USBMODE]		= 0x0C8UL,
	[OP_ENDPTSETUPSTAT]	= 0x0D8UL,
	[OP_ENDPTPRIME]		= 0x0DCUL,
	[OP_ENDPTFLUSH]		= 0x0E0UL,
	[OP_ENDPTSTAT]		= 0x0E4UL,
	[OP_ENDPTCOMPLETE]	= 0x0E8UL,
	[OP_ENDPTCTRL]		= 0x0ECUL,
};

static int hw_alloc_regmap(struct ci13xxx *ci, bool is_lpm)
{
	int i;

	kfree(ci->hw_bank.regmap);

	ci->hw_bank.regmap = kzalloc((OP_LAST + 1) * sizeof(void *),
				      GFP_KERNEL);
	if (!ci->hw_bank.regmap)
		return -ENOMEM;

	for (i = 0; i < OP_ENDPTCTRL; i++)
		ci->hw_bank.regmap[i] =
			(i <= CAP_LAST ? ci->hw_bank.cap : ci->hw_bank.op) +
			(is_lpm ? ci_regs_lpm[i] : ci_regs_nolpm[i]);

	for (; i <= OP_LAST; i++)
		ci->hw_bank.regmap[i] = ci->hw_bank.op +
			4 * (i - OP_ENDPTCTRL) +
			(is_lpm
			 ? ci_regs_lpm[OP_ENDPTCTRL]
			 : ci_regs_nolpm[OP_ENDPTCTRL]);

	return 0;
}

/**
 * hw_ep_bit: calculates the bit number
 * @num: endpoint number
 * @dir: endpoint direction
 *
 * This function returns bit number
 */
static inline int hw_ep_bit(int num, int dir)
{
	return num + (dir ? 16 : 0);
}

static int ep_to_bit(struct ci13xxx *ci, int n)
{
	int fill = 16 - ci->hw_ep_max / 2;

	if (n >= ci->hw_ep_max / 2)
		n += fill;

	return n;
}

/**
 * hw_msm_read: reads from register bitfield
 * @addr: address relative to bus map
 * @mask: bitfield mask
 *
 * This function returns register bitfield data
 */
static u32 hw_msm_read(u32 addr, u32 mask)
{
	struct ci13xxx *ci = _udc;
	void __iomem *base = ci->hw_bank.cap - DEF_CAPOFFSET;

	return ioread32(addr + base) & mask;
}

/**
 * hw_msm_write: writes to register bitfield
 * @addr: address relative to bus map
 * @mask: bitfield mask
 * @data: new data
 */
static void hw_msm_write(u32 addr, u32 mask, u32 data)
{
	struct ci13xxx *ci = _udc;
	void __iomem *base = ci->hw_bank.cap - DEF_CAPOFFSET;

	//iowrite32((ioread32(addr + base) & mask) | (data & mask),
	iowrite32(hw_msm_read(addr, ~mask) | (data & mask),
		  addr + base);
}

/**
 * hw_read: reads from a hw register
 * @reg:  register index
 * @mask: bitfield mask
 *
 * This function returns register contents
 */
static u32 hw_read(struct ci13xxx *ci, enum ci13xxx_regs reg, u32 mask)
{
	return ioread32(ci->hw_bank.regmap[reg]) & mask;
}

/**
 * hw_write: writes to a hw register
 * @reg:  register index
 * @mask: bitfield mask
 * @data: new value
 */
static void hw_write(struct ci13xxx *ci, enum ci13xxx_regs reg, u32 mask,
		     u32 data)
{
	if (~mask)
		data = (ioread32(ci->hw_bank.regmap[reg]) & ~mask)
			| (data & mask);

	iowrite32(data, ci->hw_bank.regmap[reg]);
}

/**
 * hw_test_and_clear: tests & clears a hw register
 * @reg:  register index
 * @mask: bitfield mask
 *
 * This function returns register contents
 */
static u32 hw_test_and_clear(struct ci13xxx *ci, enum ci13xxx_regs reg,
			     u32 mask)
{
	u32 val = ioread32(ci->hw_bank.regmap[reg]) & mask;

	iowrite32(val, ci->hw_bank.regmap[reg]);
	return val;
}

/**
 * hw_test_and_write: tests & writes a hw register
 * @reg:  register index
 * @mask: bitfield mask
 * @data: new value
 *
 * This function returns register contents
 */
static u32 hw_test_and_write(struct ci13xxx *ci, enum ci13xxx_regs reg,
			     u32 mask, u32 data)
{
	u32 val = hw_read(ci, reg, ~0);

	hw_write(ci, reg, mask, data);
	return (val & mask) >> __ffs(mask);
}

static int hw_device_init(struct ci13xxx *ci, void __iomem *base,
			uintptr_t cap_offset)
{
	u32 reg;

	/* bank is a module variable */
	ci->hw_bank.abs = base;

	ci->hw_bank.cap = ci->hw_bank.abs;
	ci->hw_bank.cap += cap_offset;
	ci->hw_bank.op = ci->hw_bank.cap + (ioread32(ci->hw_bank.cap) & 0xff);

	hw_alloc_regmap(ci, false);
	reg = hw_read(ci, CAP_HCCPARAMS, HCCPARAMS_LEN) >>
		__ffs(HCCPARAMS_LEN);
	ci->hw_bank.lpm  = reg;
	hw_alloc_regmap(ci, !!reg);
	ci->hw_bank.size = ci->hw_bank.op - ci->hw_bank.abs;
	ci->hw_bank.size += OP_LAST;
	ci->hw_bank.size /= sizeof(u32);

	reg = hw_read(ci, CAP_DCCPARAMS, DCCPARAMS_DEN) >>
		__ffs(DCCPARAMS_DEN);
	ci->hw_ep_max = reg * 2;   /* cache hw ENDPT_MAX */

	if (ci->hw_ep_max == 0 || ci->hw_ep_max > ENDPT_MAX)
		return -ENODEV;

	/* setup lock mode ? */

	/* ENDPTSETUPSTAT is '0' by default */

	/* HCSPARAMS.bf.ppc SHOULD BE zero for device */

	return 0;
}
/**
 * hw_device_reset: resets chip (execute without interruption)
 * @base: register base address
 *
 * This function returns an error code
 */
static int hw_device_reset(struct ci13xxx *ci)
{
	int delay_count = 25; /* 250 usec */

	/* should flush & stop before reset */
	hw_write(ci, OP_ENDPTFLUSH, ~0, ~0);
	hw_write(ci, OP_USBCMD, USBCMD_RS, 0);

	hw_write(ci, OP_USBCMD, USBCMD_RST, USBCMD_RST);
	while (hw_read(ci, OP_USBCMD, USBCMD_RST))
		udelay(10);             /* not RTOS friendly */
	if (delay_count < 0)
		pr_err("USB controller reset failed\n");

	if (ci->udc_driver->notify_event)
		ci->udc_driver->notify_event(ci,
			CI13XXX_CONTROLLER_RESET_EVENT);

	/* USBMODE should be configured step by step */
	hw_write(ci, OP_USBMODE, USBMODE_CM, USBMODE_CM_IDLE);
	hw_write(ci, OP_USBMODE, USBMODE_CM, USBMODE_CM_DEVICE);
	/* HW >= 2.3 */
	hw_write(ci, OP_USBMODE, USBMODE_SLOM, USBMODE_SLOM);

	/*
	 * ITC (Interrupt Threshold Control) field is to set the maximum
	 * rate at which the device controller will issue interrupts.
	 * The maximum interrupt interval measured in micro frames.
	 * Valid values are 0, 1, 2, 4, 8, 16, 32, 64. The default value is
	 * 8 micro frames. If CPU can handle interrupts at faster rate, ITC
	 * can be set to lesser value to gain performance.
	 */
	if (ci->udc_driver->nz_itc)
		hw_write(ci, OP_USBCMD, USBCMD_ITC_MASK,
			USBCMD_ITC(ci->udc_driver->nz_itc));
	else if (ci->udc_driver->flags & CI13XXX_ZERO_ITC)
		hw_write(ci, OP_USBCMD, USBCMD_ITC_MASK, USBCMD_ITC(0));

	if (hw_read(ci, OP_USBMODE, USBMODE_CM) != USBMODE_CM_DEVICE) {
		pr_err("cannot enter in device mode");
		pr_err("lpm = %i", ci->hw_bank.lpm);
		return -ENODEV;
	}

	return 0;
}

/**
 * hw_device_state: enables/disables interrupts & starts/stops device (execute
 *                  without interruption)
 * @dma: 0 => disable, !0 => enable and set dma engine
 *
 * This function returns an error code
 */
static int hw_device_state(struct ci13xxx *ci, u32 dma)
{
	if (dma) {
		if (!(ci->udc_driver->flags & CI13XXX_DISABLE_STREAMING)) {
			hw_write(ci, OP_USBMODE, USBMODE_SDIS, 0);
			pr_debug("%s(): streaming mode is enabled. USBMODE:%x\n",
				 __func__, hw_read(ci, OP_USBMODE, ~0));

		} else {
			hw_write(ci, OP_USBMODE, USBMODE_SDIS, USBMODE_SDIS);
			pr_debug("%s(): streaming mode is disabled. USBMODE:%x\n",
				__func__, hw_read(ci, OP_USBMODE, ~0));
		}

		hw_write(ci, OP_ENDPTLISTADDR, ~0, dma);


		/* Set BIT(31) to enable AHB2AHB Bypass functionality */
		if (ci->udc_driver->flags & CI13XXX_ENABLE_AHB2AHB_BYPASS) {
			hw_msm_write(ABS_AHBMODE, AHB2AHB_BYPASS, AHB2AHB_BYPASS);
			pr_debug("%s(): ByPass Mode is enabled. AHBMODE:%x\n",
					__func__, hw_msm_read(ABS_AHBMODE, ~0));
		}

		/* interrupt, error, port change, reset, sleep/suspend */
		hw_write(ci, OP_USBINTR, ~0,
			     USBi_UI|USBi_UEI|USBi_PCI|USBi_URI|USBi_SLI);
		hw_write(ci, OP_USBCMD, USBCMD_RS, USBCMD_RS);
	} else {
		hw_write(ci, OP_USBCMD, USBCMD_RS, 0);
		hw_write(ci, OP_USBINTR, ~0, 0);
		/* Clear BIT(31) to disable AHB2AHB Bypass functionality */
		if (ci->udc_driver->flags & CI13XXX_ENABLE_AHB2AHB_BYPASS) {
			hw_msm_write(ABS_AHBMODE, AHB2AHB_BYPASS, 0);
			pr_debug("%s(): ByPass Mode is disabled. AHBMODE:%x\n",
					__func__, hw_msm_read(ABS_AHBMODE, ~0));
		}
	}
	return 0;
}

static void debug_ept_flush_info(int ep_num, int dir)
{
	struct ci13xxx *ci = _udc;
	struct ci13xxx_ep *mep;

	if (dir)
		mep = &ci->ci13xxx_ep[ep_num + ci->hw_ep_max/2];
	else
		mep = &ci->ci13xxx_ep[ep_num];

	pr_err_ratelimited("USB Registers\n");
	pr_err_ratelimited("USBCMD:%x\n", hw_read(ci, OP_USBCMD, ~0));
	pr_err_ratelimited("USBSTS:%x\n", hw_read(ci, OP_USBSTS, ~0));
	pr_err_ratelimited("ENDPTLISTADDR:%x\n",
			hw_read(ci, OP_ENDPTLISTADDR, ~0));
	pr_err_ratelimited("PORTSC:%x\n", hw_read(ci, OP_PORTSC, ~0));
	pr_err_ratelimited("USBMODE:%x\n", hw_read(ci, OP_USBMODE, ~0));
	pr_err_ratelimited("ENDPTSTAT:%x\n", hw_read(ci, OP_ENDPTSTAT, ~0));

	dbg_usb_op_fail(0xFF, "FLUSHF", mep);
}
/**
 * hw_ep_flush: flush endpoint fifo (execute without interruption)
 * @num: endpoint number
 * @dir: endpoint direction
 *
 * This function returns an error code
 */
static int hw_ep_flush(struct ci13xxx *ci, int num, int dir)
{
	ktime_t start, diff;
	int n = hw_ep_bit(num, dir);
	struct ci13xxx_ep *mEp = &ci->ci13xxx_ep[n];

	/* Flush ep0 even when queue is empty */
	if (ci->skip_flush || (num && list_empty(&mEp->qh.queue)))
		return 0;

	start = ktime_get();
	do {
		/* flush any pending transfer */
		hw_write(ci, OP_ENDPTFLUSH, ~0, BIT(n));
		while (hw_read(ci, OP_ENDPTFLUSH, BIT(n))) {
			cpu_relax();
			diff = ktime_sub(ktime_get(), start);
			if (ktime_to_ms(diff) > USB_MAX_TIMEOUT) {
				printk_ratelimited(KERN_ERR
					"%s: Failed to flush ep#%d %s\n",
					__func__, num,
					dir ? "IN" : "OUT");
				debug_ept_flush_info(num, dir);
				ci->skip_flush = true;
				/* Notify to trigger h/w reset recovery later */
				if (ci->udc_driver->notify_event)
					ci->udc_driver->notify_event(ci,
						CI13XXX_CONTROLLER_ERROR_EVENT);
				return 0;
			}
		}
	} while (hw_read(ci, OP_ENDPTSTAT, BIT(n)));

	return 0;
}

/**
 * hw_ep_disable: disables endpoint (execute without interruption)
 * @num: endpoint number
 * @dir: endpoint direction
 *
 * This function returns an error code
 */
static int hw_ep_disable(struct ci13xxx *ci, int num, int dir)
{
	hw_ep_flush(ci, num, dir);
	hw_write(ci, OP_ENDPTCTRL + num,
		  dir ? ENDPTCTRL_TXE : ENDPTCTRL_RXE, 0);
	return 0;
}

/**
 * hw_ep_enable: enables endpoint (execute without interruption)
 * @num:  endpoint number
 * @dir:  endpoint direction
 * @type: endpoint type
 *
 * This function returns an error code
 */
static int hw_ep_enable(struct ci13xxx *ci, int num, int dir, int type)
{
	u32 mask, data;

	if (dir) {
		mask  = ENDPTCTRL_TXT;  /* type    */
		data  = type << __ffs(mask);

		mask |= ENDPTCTRL_TXS;  /* unstall */
		mask |= ENDPTCTRL_TXR;  /* reset data toggle */
		data |= ENDPTCTRL_TXR;
		mask |= ENDPTCTRL_TXE;  /* enable  */
		data |= ENDPTCTRL_TXE;
	} else {
		mask  = ENDPTCTRL_RXT;  /* type    */
		data  = type << __ffs(mask);

		mask |= ENDPTCTRL_RXS;  /* unstall */
		mask |= ENDPTCTRL_RXR;  /* reset data toggle */
		data |= ENDPTCTRL_RXR;
		mask |= ENDPTCTRL_RXE;  /* enable  */
		data |= ENDPTCTRL_RXE;
	}
	hw_write(ci, OP_ENDPTCTRL + num, mask, data);

	/* make sure endpoint is enabled before returning */
	mb();

	return 0;
}

/**
 * hw_ep_get_halt: return endpoint halt status
 * @num: endpoint number
 * @dir: endpoint direction
 *
 * This function returns 1 if endpoint halted
 */
static int hw_ep_get_halt(struct ci13xxx *ci, int num, int dir)
{
	u32 mask = dir ? ENDPTCTRL_TXS : ENDPTCTRL_RXS;

	return hw_read(ci, OP_ENDPTCTRL + num, mask) ? 1 : 0;
}

/**
 * hw_ep_prime: primes endpoint (execute without interruption)
 * @num:     endpoint number
 * @dir:     endpoint direction
 * @is_ctrl: true if control endpoint
 *
 * This function returns an error code
 */
static int hw_ep_prime(struct ci13xxx *ci, int num, int dir, int is_ctrl)
{
	int n = hw_ep_bit(num, dir);

	if (is_ctrl && dir == RX && hw_read(ci, OP_ENDPTSETUPSTAT, BIT(num)))
		return -EAGAIN;

	hw_write(ci, OP_ENDPTPRIME, ~0, BIT(n));

	if (is_ctrl && dir == RX  && hw_read(ci, OP_ENDPTSETUPSTAT, BIT(num)))
		return -EAGAIN;

	/* status shoult be tested according with manual but it doesn't work */
	return 0;
}

/**
 * hw_ep_set_halt: configures ep halt & resets data toggle after clear (execute
 *                 without interruption)
 * @num:   endpoint number
 * @dir:   endpoint direction
 * @value: true => stall, false => unstall
 *
 * This function returns an error code
 */
static int hw_ep_set_halt(struct ci13xxx *ci, int num, int dir, int value)
{
	u32 mask_xs, mask_xr;

	if (value != 0 && value != 1)
		return -EINVAL;

	do {
		enum ci13xxx_regs reg = OP_ENDPTCTRL + num;

		//if (hw_read(ci, OP_ENDPTSETUPSTAT, BIT(num)))
		//	return 0;

		mask_xs = dir ? ENDPTCTRL_TXS : ENDPTCTRL_RXS;
		mask_xr = dir ? ENDPTCTRL_TXR : ENDPTCTRL_RXR;

		/* data toggle - reserved for EP0 but it's in ESS */
		hw_write(ci, reg, mask_xs|mask_xr, value ? mask_xs : mask_xr);

	} while (value != hw_ep_get_halt(ci, num, dir));

	return 0;
}

/**
 * hw_is_port_high_speed: test if port is high speed
 *
 * This function returns true if high speed port
 */
static int hw_port_is_high_speed(struct ci13xxx *ci)
{
	return ci->hw_bank.lpm ? hw_read(ci, OP_DEVLC, DEVLC_PSPD) :
		hw_read(ci, OP_PORTSC, PORTSC_HSP);
}

/**
 * hw_port_test_get: reads port test mode value
 *
 * This function returns port test mode value
 */
static u8 hw_port_test_get(struct ci13xxx *ci)
{
	return hw_read(ci, OP_PORTSC, PORTSC_PTC) >> __ffs(PORTSC_PTC);
}

/**
 * hw_port_test_set: writes port test mode (execute without interruption)
 * @mode: new value
 *
 * This function returns an error code
 */
static int hw_port_test_set(struct ci13xxx *ci, u8 mode)
{
	const u8 TEST_MODE_MAX = 7;

	if (mode > TEST_MODE_MAX)
		return -EINVAL;

	hw_write(ci, OP_PORTSC, PORTSC_PTC, mode << __ffs(PORTSC_PTC));
	return 0;
}

/**
 * hw_read_intr_enable: returns interrupt enable register
 *
 * This function returns register data
 */
static u32 hw_read_intr_enable(struct ci13xxx *ci)
{
	return hw_read(ci, OP_USBINTR, ~0);
}

/**
 * hw_read_intr_status: returns interrupt status register
 *
 * This function returns register data
 */
static u32 hw_read_intr_status(struct ci13xxx *ci)
{
	return hw_read(ci, OP_USBSTS, ~0);
}

/**
 * hw_register_read: reads all device registers (execute without interruption)
 * @buf:  destination buffer
 * @size: buffer size
 *
 * This function returns number of registers read
 */
static size_t hw_register_read(struct ci13xxx *ci, u32 *buf, size_t size)
{
	unsigned i;

	if (size > ci->hw_bank.size)
		size = ci->hw_bank.size;

	for (i = 0; i < size; i++)
		buf[i] = hw_read(ci, i * sizeof(u32), ~0);

	return size;
}

/**
 * hw_register_write: writes to register
 * @addr: register address
 * @data: register value
 *
 * This function returns an error code
 */
static int hw_register_write(struct ci13xxx *ci, u16 addr, u32 data)
{
	/* align */
	addr /= sizeof(u32);

	if (addr >= ci->hw_bank.size)
		return -EINVAL;

	/* align */
	addr *= sizeof(u32);

	hw_write(ci, addr, ~0, data);
	return 0;
}

/**
 * hw_test_and_clear_complete: test & clear complete status (execute without
 *                             interruption)
 * @n: endpoint number
 *
 * This function returns complete status
 */
static int hw_test_and_clear_complete(struct ci13xxx *ci, int n)
{
	n = ep_to_bit(ci, n);
	return hw_test_and_clear(ci, OP_ENDPTCOMPLETE, BIT(n));
}

/**
 * hw_test_and_clear_intr_active: test & clear active interrupts (execute
 *                                without interruption)
 *
 * This function returns active interrutps
 */
static u32 hw_test_and_clear_intr_active(struct ci13xxx *ci)
{
	u32 reg = hw_read_intr_status(ci) & hw_read_intr_enable(ci);

	hw_write(ci, OP_USBSTS, ~0, reg);
	return reg;
}

/**
 * hw_test_and_clear_setup_guard: test & clear setup guard (execute without
 *                                interruption)
 *
 * This function returns guard value
 */
static int hw_test_and_clear_setup_guard(struct ci13xxx *ci)
{
	return hw_test_and_write(ci, OP_USBCMD, USBCMD_SUTW, 0);
}

/**
 * hw_test_and_set_setup_guard: test & set setup guard (execute without
 *                              interruption)
 *
 * This function returns guard value
 */
static int hw_test_and_set_setup_guard(struct ci13xxx *ci)
{
	return hw_test_and_write(ci, OP_USBCMD, USBCMD_SUTW, USBCMD_SUTW);
}

/**
 * hw_usb_set_address: configures USB address (execute without interruption)
 * @value: new USB address
 *
 * This function returns an error code
 */
static void hw_usb_set_address(struct ci13xxx *ci, u8 value)
{
	hw_write(ci, OP_DEVICEADDR, DEVICEADDR_USBADR,
		 value << __ffs(DEVICEADDR_USBADR));
}

/**
 * hw_usb_reset: restart device after a bus reset (execute without
 *               interruption)
 *
 * This function returns an error code
 */
static int hw_usb_reset(struct ci13xxx *ci)
{
	int delay_count = 10; /* 100 usec delay */

	hw_usb_set_address(ci, 0);

	/* ESS flushes only at end?!? */
	hw_write(ci, OP_ENDPTFLUSH,    ~0, ~0);   /* flush all EPs */

	/* clear complete status */
	hw_write(ci, OP_ENDPTCOMPLETE,  0,  0);   /* writes its content */

	/* wait until all bits cleared */
	while (delay_count-- && hw_read(ci, OP_ENDPTPRIME, ~0))
		udelay(10);
	if (delay_count < 0)
		pr_err("ENDPTPRIME is not cleared during bus reset\n");

	/* reset all endpoints ? */

	/* reset internal status and wait for further instructions
	   no need to verify the port reset status (ESS does it) */

	return 0;
}

/******************************************************************************
 * DBG block
 *****************************************************************************/
/**
 * show_device: prints information about device capabilities and status
 *
 * Check "device.h" for details
 */
static ssize_t show_device(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);
	struct usb_gadget *gadget = &ci->gadget;
	int n = 0;

	dbg_trace("[%s] %pK\n", __func__, buf);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		return 0;
	}

	n += scnprintf(buf + n, PAGE_SIZE - n, "speed             = %d\n",
		       gadget->speed);
	n += scnprintf(buf + n, PAGE_SIZE - n, "max_speed         = %d\n",
		       gadget->max_speed);
	n += scnprintf(buf + n, PAGE_SIZE - n, "is_otg            = %d\n",
		       gadget->is_otg);
	n += scnprintf(buf + n, PAGE_SIZE - n, "is_a_peripheral   = %d\n",
		       gadget->is_a_peripheral);
	n += scnprintf(buf + n, PAGE_SIZE - n, "b_hnp_enable      = %d\n",
		       gadget->b_hnp_enable);
	n += scnprintf(buf + n, PAGE_SIZE - n, "a_hnp_support     = %d\n",
		       gadget->a_hnp_support);
	n += scnprintf(buf + n, PAGE_SIZE - n, "a_alt_hnp_support = %d\n",
		       gadget->a_alt_hnp_support);
	n += scnprintf(buf + n, PAGE_SIZE - n, "name              = %s\n",
		       (gadget->name ? gadget->name : ""));

	return n;
}
static DEVICE_ATTR(device, S_IRUSR, show_device, NULL);

/**
 * show_driver: prints information about attached gadget (if any)
 *
 * Check "device.h" for details
 */
static ssize_t show_driver(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);
	struct usb_gadget_driver *driver = ci->driver;
	int n = 0;

	dbg_trace("[%s] %pK\n", __func__, buf);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		return 0;
	}

	if (driver == NULL)
		return scnprintf(buf, PAGE_SIZE,
				 "There is no gadget attached!\n");

	n += scnprintf(buf + n, PAGE_SIZE - n, "function  = %s\n",
		       (driver->function ? driver->function : ""));
	n += scnprintf(buf + n, PAGE_SIZE - n, "max speed = %d\n",
		       driver->max_speed);

	return n;
}
static DEVICE_ATTR(driver, S_IRUSR, show_driver, NULL);

/* Maximum event message length */
#define DBG_DATA_MSG   64UL

/* Maximum event messages */
#define DBG_DATA_MAX   128UL

/* Event buffer descriptor */
static struct {
	char     (buf[DBG_DATA_MAX])[DBG_DATA_MSG];   /* buffer */
	unsigned idx;   /* index */
	unsigned tty;   /* print to console? */
	rwlock_t lck;   /* lock */
} dbg_data = {
	.idx = 0,
	.tty = 0,
	.lck = __RW_LOCK_UNLOCKED(lck)
};

/**
 * dbg_dec: decrements debug event index
 * @idx: buffer index
 */
static void dbg_dec(unsigned *idx)
{
	*idx = (*idx - 1) & (DBG_DATA_MAX-1);
}

/**
 * dbg_inc: increments debug event index
 * @idx: buffer index
 */
static void dbg_inc(unsigned *idx)
{
	*idx = (*idx + 1) & (DBG_DATA_MAX-1);
}


static unsigned int ep_addr_txdbg_mask;
module_param(ep_addr_txdbg_mask, uint, S_IRUGO | S_IWUSR);
static unsigned int ep_addr_rxdbg_mask;
module_param(ep_addr_rxdbg_mask, uint, S_IRUGO | S_IWUSR);

static int allow_dbg_print(u8 addr)
{
	int dir, num;

	/* allow bus wide events */
	if (addr == 0xff)
		return 1;

	dir = addr & USB_ENDPOINT_DIR_MASK ? TX : RX;
	num = addr & ~USB_ENDPOINT_DIR_MASK;
	num = 1 << num;

	if ((dir == TX) && (num & ep_addr_txdbg_mask))
		return 1;
	if ((dir == RX) && (num & ep_addr_rxdbg_mask))
		return 1;

	return 0;
}

#define TIME_BUF_LEN  20
/*get_timestamp - returns time of day in us */
static char *get_timestamp(char *tbuf)
{
	unsigned long long t;
	unsigned long nanosec_rem;

	t = cpu_clock(smp_processor_id());
	nanosec_rem = do_div(t, 1000000000)/1000;
	scnprintf(tbuf, TIME_BUF_LEN, "[%5lu.%06lu] ", (unsigned long)t,
		nanosec_rem);
	return tbuf;
}

/**
 * dbg_print:  prints the common part of the event
 * @addr:   endpoint address
 * @name:   event name
 * @status: status
 * @extra:  extra information
 */
static void dbg_print(u8 addr, const char *name, int status, const char *extra)
{
	unsigned long flags;
	char tbuf[TIME_BUF_LEN];

	if (!allow_dbg_print(addr))
		return;

	write_lock_irqsave(&dbg_data.lck, flags);

	scnprintf(dbg_data.buf[dbg_data.idx], DBG_DATA_MSG,
		  "%s\t? %02X %-7.7s %4i ?\t%s\n",
		  get_timestamp(tbuf), addr, name, status, extra);

	dbg_inc(&dbg_data.idx);

	write_unlock_irqrestore(&dbg_data.lck, flags);

	if (dbg_data.tty != 0)
		pr_notice("%s\t? %02X %-7.7s %4i ?\t%s\n",
			  get_timestamp(tbuf), addr, name, status, extra);
}

/**
 * dbg_event: prints a generic event
 * @addr:   endpoint address
 * @name:   event name
 * @status: status
 */
static void dbg_event(u8 addr, const char *name, int status)
{
	if (name != NULL)
		dbg_print(addr, name, status, "");
}

/**
 * dbg_usb_op_fail: prints USB Operation FAIL event
 * @addr: endpoint address
 * @mEp:  endpoint structure
 */
static void dbg_usb_op_fail(u8 addr, const char *name,
				const struct ci13xxx_ep *mep)
{
	pr_debug("USB FAILURE: %s", name);
}

/**
 * show_events: displays the event buffer
 *
 * Check "device.h" for details
 */
static ssize_t show_events(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	unsigned long flags;
	unsigned i, j, n = 0;

	dbg_trace("[%s] %pK\n", __func__, buf);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		return 0;
	}

	read_lock_irqsave(&dbg_data.lck, flags);

	i = dbg_data.idx;
	for (dbg_dec(&i); i != dbg_data.idx; dbg_dec(&i)) {
		n += strlen(dbg_data.buf[i]);
		if (n >= PAGE_SIZE) {
			n -= strlen(dbg_data.buf[i]);
			break;
		}
	}
	for (j = 0, dbg_inc(&i); j < n; dbg_inc(&i))
		j += scnprintf(buf + j, PAGE_SIZE - j,
			       "%s", dbg_data.buf[i]);

	read_unlock_irqrestore(&dbg_data.lck, flags);

	return n;
}

/**
 * store_events: configure if events are going to be also printed to console
 *
 * Check "device.h" for details
 */
static ssize_t store_events(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	unsigned tty;

	dbg_trace("[%s] %pK, %d\n", __func__, buf, count);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		goto done;
	}

	if (sscanf(buf, "%u", &tty) != 1 || tty > 1) {
		dev_err(dev, "<1|0>: enable|disable console log\n");
		goto done;
	}

	dbg_data.tty = tty;
	dev_info(dev, "tty = %u", dbg_data.tty);

 done:
	return count;
}
static DEVICE_ATTR(events, S_IRUSR | S_IWUSR, show_events, store_events);

/**
 * show_port_test: reads port test mode
 *
 * Check "device.h" for details
 */
static ssize_t show_port_test(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);
	unsigned long flags;
	unsigned mode;

	dbg_trace("[%s] %pK\n", __func__, buf);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		return 0;
	}

	spin_lock_irqsave(&ci->lock, flags);
	mode = hw_port_test_get(ci);
	spin_unlock_irqrestore(&ci->lock, flags);

	return scnprintf(buf, PAGE_SIZE, "mode = %u\n", mode);
}

/**
 * store_port_test: writes port test mode
 *
 * Check "device.h" for details
 */
static ssize_t store_port_test(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);
	unsigned long flags;
	unsigned mode;

	dbg_trace("[%s] %pK, %d\n", __func__, buf, count);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		goto done;
	}

	if (sscanf(buf, "%u", &mode) != 1) {
		dev_err(dev, "<mode>: set port test mode");
		goto done;
	}

	spin_lock_irqsave(&ci->lock, flags);
	if (hw_port_test_set(ci, mode))
		dev_err(dev, "invalid mode\n");
	spin_unlock_irqrestore(&ci->lock, flags);

 done:
	return count;
}
static DEVICE_ATTR(port_test, S_IRUSR | S_IWUSR,
		   show_port_test, store_port_test);

/**
 * show_qheads: DMA contents of all queue heads
 *
 * Check "device.h" for details
 */
static ssize_t show_qheads(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);
	unsigned long flags;
	unsigned i, j, n = 0;

	dbg_trace("[%s] %pK\n", __func__, buf);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		return 0;
	}

	spin_lock_irqsave(&ci->lock, flags);
	for (i = 0; i < ci->hw_ep_max/2; i++) {
		struct ci13xxx_ep *mEpRx = &ci->ci13xxx_ep[i];
		struct ci13xxx_ep *mEpTx = &ci->ci13xxx_ep[i + ci->hw_ep_max/2];
		n += scnprintf(buf + n, PAGE_SIZE - n,
			       "EP=%02i: RX=%08X TX=%08X\n",
			       i, (u32)mEpRx->qh.dma, (u32)mEpTx->qh.dma);
		for (j = 0; j < (sizeof(struct ci13xxx_qh)/sizeof(u32)); j++) {
			n += scnprintf(buf + n, PAGE_SIZE - n,
				       " %04X:    %08X    %08X\n", j,
				       *((u32 *)mEpRx->qh.ptr + j),
				       *((u32 *)mEpTx->qh.ptr + j));
		}
	}
	spin_unlock_irqrestore(&ci->lock, flags);

	return n;
}
static DEVICE_ATTR(qheads, S_IRUSR, show_qheads, NULL);

/**
 * show_registers: dumps all registers
 *
 * Check "device.h" for details
 */
#define DUMP_ENTRIES	512
static ssize_t show_registers(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);
	unsigned long flags;
	u32 *dump;
	unsigned i, k, n = 0;

	dbg_trace("[%s] %pK\n", __func__, buf);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		return 0;
	}

	dump = kmalloc(sizeof(u32) * DUMP_ENTRIES, GFP_KERNEL);
	if (!dump) {
		dev_err(dev, "%s: out of memory\n", __func__);
		return 0;
	}

	spin_lock_irqsave(&ci->lock, flags);
	k = hw_register_read(ci, dump, DUMP_ENTRIES);
	spin_unlock_irqrestore(&ci->lock, flags);

	for (i = 0; i < k; i++) {
		n += scnprintf(buf + n, PAGE_SIZE - n,
			       "reg[0x%04X] = 0x%08X\n",
			       i * (unsigned)sizeof(u32), dump[i]);
	}
	kfree(dump);

	return n;
}

/**
 * store_registers: writes value to register address
 *
 * Check "device.h" for details
 */
static ssize_t store_registers(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);
	unsigned long addr, data, flags;

	dbg_trace("[%s] %pK, %d\n", __func__, buf, count);
	if (attr == NULL || buf == NULL) {
		dev_err(dev, "[%s] EINVAL\n", __func__);
		goto done;
	}

	if (sscanf(buf, "%li %li", &addr, &data) != 2) {
		dev_err(dev, "<addr> <data>: write data to register address");
		goto done;
	}

	spin_lock_irqsave(&ci->lock, flags);
	if (hw_register_write(ci, addr, data))
		dev_err(dev, "invalid address range\n");
	spin_unlock_irqrestore(&ci->lock, flags);

 done:
	return count;
}
static DEVICE_ATTR(registers, S_IRUSR | S_IWUSR,
		   show_registers, store_registers);

/**
 * show_requests: DMA contents of all requests currently queued (all endpts)
 *
 * Check "device.h" for details
 */
static ssize_t show_requests(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	return 0;
}
static DEVICE_ATTR(requests, S_IRUSR, show_requests, NULL);

static int ci13xxx_wakeup(struct usb_gadget *_gadget)
{
	struct ci13xxx *ci = container_of(_gadget, struct ci13xxx, gadget);
	unsigned long flags;
	int ret = 0;

	trace();

	spin_lock_irqsave(&ci->lock, flags);
	if (!ci->gadget.remote_wakeup) {
		ret = -EOPNOTSUPP;
		dbg_trace("remote wakeup feature is not enabled\n");
		goto out;
	}
	spin_unlock_irqrestore(&ci->lock, flags);

	pm_runtime_get_sync(&_gadget->dev);

	ci->udc_driver->notify_event(ci,
		CI13XXX_CONTROLLER_REMOTE_WAKEUP_EVENT);

	if (ci->transceiver)
		usb_phy_set_suspend(ci->transceiver, 0);

	spin_lock_irqsave(&ci->lock, flags);
	if (!hw_read(ci, OP_PORTSC, PORTSC_SUSP)) {
		ret = -EINVAL;
		dbg_trace("port is not suspended\n");
		pm_runtime_put(&_gadget->dev);
		goto out;
	}
	hw_write(ci, OP_PORTSC, PORTSC_FPR, PORTSC_FPR);

	pm_runtime_mark_last_busy(&_gadget->dev);
	pm_runtime_put_autosuspend(&_gadget->dev);
out:
	spin_unlock_irqrestore(&ci->lock, flags);
	return ret;
}

static void usb_do_remote_wakeup(struct work_struct *w)
{
	struct ci13xxx *ci = _udc;
	unsigned long flags;
	bool do_wake;

	/*
	 * This work can not be canceled from interrupt handler. Check
	 * if wakeup conditions are still met.
	 */
	spin_lock_irqsave(&ci->lock, flags);
	do_wake = ci->suspended && ci->gadget.remote_wakeup;
	spin_unlock_irqrestore(&ci->lock, flags);

	if (do_wake)
		ci13xxx_wakeup(&ci->gadget);
}

static ssize_t usb_remote_wakeup(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct ci13xxx *ci = container_of(dev, struct ci13xxx, gadget.dev);

	ci13xxx_wakeup(&ci->gadget);

	return count;
}
static DEVICE_ATTR(wakeup, S_IWUSR, 0, usb_remote_wakeup);

/**
 * dbg_create_files: initializes the attribute interface
 * @dev: device
 *
 * This function returns an error code
 */
__maybe_unused static int dbg_create_files(struct device *dev)
{
	int retval = 0;

	if (dev == NULL)
		return -EINVAL;
	retval = device_create_file(dev, &dev_attr_device);
	if (retval)
		goto done;
	retval = device_create_file(dev, &dev_attr_driver);
	if (retval)
		goto rm_device;
	retval = device_create_file(dev, &dev_attr_events);
	if (retval)
		goto rm_driver;
	retval = device_create_file(dev, &dev_attr_port_test);
	if (retval)
		goto rm_events;
	retval = device_create_file(dev, &dev_attr_qheads);
	if (retval)
		goto rm_port_test;
	retval = device_create_file(dev, &dev_attr_registers);
	if (retval)
		goto rm_qheads;
	retval = device_create_file(dev, &dev_attr_requests);
	if (retval)
		goto rm_registers;
	retval = device_create_file(dev, &dev_attr_wakeup);
	if (retval)
		goto rm_remote_wakeup;

	return 0;

rm_remote_wakeup:
	device_remove_file(dev, &dev_attr_wakeup);
 rm_registers:
	device_remove_file(dev, &dev_attr_registers);
 rm_qheads:
	device_remove_file(dev, &dev_attr_qheads);
 rm_port_test:
	device_remove_file(dev, &dev_attr_port_test);
 rm_events:
	device_remove_file(dev, &dev_attr_events);
 rm_driver:
	device_remove_file(dev, &dev_attr_driver);
 rm_device:
	device_remove_file(dev, &dev_attr_device);
 done:
	return retval;
}

/**
 * dbg_remove_files: destroys the attribute interface
 * @dev: device
 *
 * This function returns an error code
 */
__maybe_unused static int dbg_remove_files(struct device *dev)
{
	if (dev == NULL)
		return -EINVAL;
	device_remove_file(dev, &dev_attr_requests);
	device_remove_file(dev, &dev_attr_registers);
	device_remove_file(dev, &dev_attr_qheads);
	device_remove_file(dev, &dev_attr_port_test);
	device_remove_file(dev, &dev_attr_events);
	device_remove_file(dev, &dev_attr_driver);
	device_remove_file(dev, &dev_attr_device);
	device_remove_file(dev, &dev_attr_wakeup);
	return 0;
}

/******************************************************************************
 * UTIL block
 *****************************************************************************/

static int add_td_to_list(struct ci13xxx_ep *mEp, struct ci13xxx_req *mReq,
			  unsigned length)
{
	int i;
	u32 temp;
	struct td_node *lastnode, *node = kzalloc(sizeof(struct td_node),
						  GFP_ATOMIC);

	if (node == NULL)
		return -ENOMEM;

	node->ptr = dma_pool_alloc(mEp->td_pool, GFP_ATOMIC,
				   &node->dma);
	if (node->ptr == NULL) {
		kfree(node);
		return -ENOMEM;
	}

	memset(node->ptr, 0, sizeof(struct ci13xxx_td));
	node->ptr->token = cpu_to_le32(length << __ffs(TD_TOTAL_BYTES));
	node->ptr->token &= cpu_to_le32(TD_TOTAL_BYTES);
	node->ptr->token |= cpu_to_le32(TD_STATUS_ACTIVE);
	if (mEp->type == USB_ENDPOINT_XFER_ISOC && mEp->dir == TX) {
		u32 mul = mReq->req.length / mEp->ep.maxpacket;

		if (mReq->req.length == 0
				|| mReq->req.length % mEp->ep.maxpacket)
			mul++;
		node->ptr->token |= mul << __ffs(TD_MULTO);
	}

	temp = (u32) (mReq->req.dma + mReq->req.actual);
	if (length) {
		node->ptr->page[0] = cpu_to_le32(temp);
		for (i = 1; i < TD_PAGE_COUNT; i++) {
			u32 page = temp + i * CI13XXX_PAGE_SIZE;
			page &= ~TD_RESERVED_MASK;
			node->ptr->page[i] = cpu_to_le32(page);
		}
	}

	mReq->req.actual += length;

	if (!list_empty(&mReq->tds)) {
		/* get the last entry */
		lastnode = list_entry(mReq->tds.prev,
				struct td_node, td);
		lastnode->ptr->next = cpu_to_le32(node->dma);
	}

	INIT_LIST_HEAD(&node->td);
	list_add_tail(&node->td, &mReq->tds);

	return 0;
}

/**
 * _usb_addr: calculates endpoint address from direction & number
 * @ep:  endpoint
 */
static inline u8 _usb_addr(struct ci13xxx_ep *ep)
{
	return ((ep->dir == TX) ? USB_ENDPOINT_DIR_MASK : 0) | ep->num;
}

/**
 * _hardware_queue: configures a request at hardware level
 * @gadget: gadget
 * @mEp:    endpoint
 *
 * This function returns an error code
 */
static int _hardware_enqueue(struct ci13xxx_ep *mEp, struct ci13xxx_req *mReq)
{
	int ret = 0;
	struct ci13xxx *ci = mEp->udc;
	unsigned rest = mReq->req.length;
	int pages = TD_PAGE_COUNT;
	struct td_node *firstnode, *lastnode;

	trace("%pK, %pK", mEp, mReq);

	/* don't queue twice */
	if (mReq->req.status == -EALREADY)
		return -EALREADY;

	mReq->req.status = -EALREADY;

	ret = usb_gadget_map_request(&ci->gadget, &mReq->req, mEp->dir);
	if (ret)
		return ret;

	/*
	 * The first buffer could be not page aligned.
	 * In that case we have to span into one extra td.
	 */
	if (mReq->req.dma % PAGE_SIZE)
		pages--;

	if (rest == 0)
		add_td_to_list(mEp, mReq, 0);

	while (rest > 0) {
		unsigned count = min(mReq->req.length - mReq->req.actual,
					(unsigned)(pages * CI13XXX_PAGE_SIZE));
		add_td_to_list(mEp, mReq, count);
		rest -= count;
	}

	if (mReq->req.zero && mReq->req.length && mEp->dir == TX
	    && (mReq->req.length % mEp->ep.maxpacket == 0))
		add_td_to_list(mEp, mReq, 0);

	firstnode = list_first_entry(&mReq->tds, struct td_node, td);

	/* MSM Specific: updating the request as required for
	 * SPS mode. Enable MSM DMA engine according
	 * to the UDC private data in the request.
	 */
	if (CI13XX_REQ_VENDOR_ID(mReq->req.udc_priv) == MSM_VENDOR_ID) {
		if (mReq->req.udc_priv & MSM_SPS_MODE) {
			firstnode->ptr->token = cpu_to_le32(TD_STATUS_ACTIVE);
			if (mReq->req.udc_priv & MSM_IS_FINITE_TRANSFER)
				firstnode->ptr->next = cpu_to_le32(TD_TERMINATE);
			else
				firstnode->ptr->next = cpu_to_le32(
						MSM_ETD_TYPE | firstnode->dma);
			if (!mReq->req.no_interrupt)
				firstnode->ptr->token |= cpu_to_le32(MSM_ETD_IOC);
		}
		mReq->req.dma = 0;
	}

	lastnode = list_entry(mReq->tds.prev,
		struct td_node, td);

	lastnode->ptr->next = cpu_to_le32(TD_TERMINATE);
	if (!mReq->req.no_interrupt)
		lastnode->ptr->token |= cpu_to_le32(TD_IOC);

	wmb();

	/* Remote Wakeup */
	if (ci->suspended) {
		if (!ci->gadget.remote_wakeup) {
			mReq->req.status = -EAGAIN;
			return -EAGAIN;
		}

		usb_phy_set_suspend(ci->transceiver, 0);
		schedule_delayed_work(&ci->rw_work, REMOTE_WAKEUP_DELAY);
	}

	mReq->req.actual = 0;
	if (!list_empty(&mEp->qh.queue)) {
		struct ci13xxx_req *mReqPrev;
		int n = hw_ep_bit(mEp->num, mEp->dir);
		int tmp_stat;
		struct td_node *prevlastnode;
		u32 next = firstnode->dma & TD_ADDR_MASK;
		ktime_t start, diff;

		mReqPrev = list_entry(mEp->qh.queue.prev,
				struct ci13xxx_req, queue);
		prevlastnode = list_entry(mReqPrev->tds.prev,
				struct td_node, td);

		prevlastnode->ptr->next = cpu_to_le32(next);
		wmb();
		if (hw_read(ci, OP_ENDPTPRIME, BIT(n)))
			goto done;
		start = ktime_get();
		do {
			hw_write(ci, OP_USBCMD, USBCMD_ATDTW, USBCMD_ATDTW);
			tmp_stat = hw_read(ci, OP_ENDPTSTAT, BIT(n));
			diff = ktime_sub(ktime_get(), start);
			/* poll for max. 100ms */
			if (ktime_to_ms(diff) > USB_MAX_TIMEOUT) {
				if (hw_read(ci, OP_USBCMD, USBCMD_ATDTW))
					break;
				printk_ratelimited(KERN_ERR
				"%s:queue failed ep#%d %s\n",
				 __func__, mEp->num, mEp->dir ? "IN" : "OUT");
				return -EAGAIN;
			}
		} while (!hw_read(ci, OP_USBCMD, USBCMD_ATDTW));
		hw_write(ci, OP_USBCMD, USBCMD_ATDTW, 0);
		if (tmp_stat)
			goto done;
	}
#if 0
	/* Hardware may leave few TDs unprocessed, check and reprime with 1st */
	if (!list_empty(&mEp->qh.queue)) {
		struct ci13xxx_req *mReq_active, *mReq_next;
		u32 i = 0;

		/* Nothing to be done if hardware already finished this TD */
		if ((TD_STATUS_ACTIVE & mReq->ptr->token) == 0)
			goto done;

		/* Iterate forward to find first TD with ACTIVE bit set */
		mReq_active = mReq;
		list_for_each_entry(mReq_next, &mEp->qh.queue, queue) {
			i++;
			mEp->dTD_active_re_q_count++;
			if (TD_STATUS_ACTIVE & mReq_next->ptr->token) {
				mReq_active = mReq_next;
				dbg_event(_usb_addr(mEp), "ReQUE",
					  mReq_next->ptr->token);
				pr_debug("!!ReQ(%u-%u-%x)-%u!!\n", mEp->num,
					 mEp->dir, mReq_next->ptr->token, i);
				break;
			}
		}

		/*  QH configuration */
		mEp->qh.ptr->td.next = firstnode->dma;
		mEp->qh.ptr->td.token &= ~TD_STATUS;
		goto prime;
	}

	/*  QH configuration */
	mEp->qh.ptr->td.next   = cpu_to_le32(mReq->dma);    /* TERMINATE = 0 */

#else
	if (!list_empty(&mEp->qh.queue)) {
		struct ci13xxx_req *mReqPrev;
		int n = hw_ep_bit(mEp->num, mEp->dir);
		int tmp_stat;
		struct td_node *prevlastnode;
		u32 next = firstnode->dma & TD_ADDR_MASK;

		mReqPrev = list_entry(mEp->qh.queue.prev,
				struct ci13xxx_req, queue);
		prevlastnode = list_entry(mReqPrev->tds.prev,
				struct td_node, td);

		prevlastnode->ptr->next = cpu_to_le32(next);
		wmb();
		if (hw_read(ci, OP_ENDPTPRIME, BIT(n)))
			goto done;
		do {
			hw_write(ci, OP_USBCMD, USBCMD_ATDTW, USBCMD_ATDTW);
			tmp_stat = hw_read(ci, OP_ENDPTSTAT, BIT(n));
		} while (!hw_read(ci, OP_USBCMD, USBCMD_ATDTW));
		hw_write(ci, OP_USBCMD, USBCMD_ATDTW, 0);
		if (tmp_stat)
			goto done;
	}

	/*  QH configuration */
	mEp->qh.ptr->td.next = cpu_to_le32(firstnode->dma);
#endif


	if (CI13XX_REQ_VENDOR_ID(mReq->req.udc_priv) == MSM_VENDOR_ID) {
		if (mReq->req.udc_priv & MSM_SPS_MODE) {
			int i;
			mEp->qh.ptr->td.next   |= cpu_to_le32(MSM_ETD_TYPE);
			i = hw_read(ci, OP_ENDPTPIPEID +
						 mEp->num * sizeof(u32), ~0);
			/* Read current value of this EPs pipe id */
			i = (mEp->dir == TX) ?
				((i >> MSM_TX_PIPE_ID_OFS) & MSM_PIPE_ID_MASK) :
					(i & MSM_PIPE_ID_MASK);
			/* If requested pipe id is different from current,
			   then write it */
			if (i != (mReq->req.udc_priv & MSM_PIPE_ID_MASK)) {
				if (mEp->dir == TX)
					hw_write(ci, 
						OP_ENDPTPIPEID +
							mEp->num * sizeof(u32),
						MSM_PIPE_ID_MASK <<
							MSM_TX_PIPE_ID_OFS,
						(mReq->req.udc_priv &
						 MSM_PIPE_ID_MASK)
							<< MSM_TX_PIPE_ID_OFS);
				else
					hw_write(ci, 
						OP_ENDPTPIPEID +
							mEp->num * sizeof(u32),
						MSM_PIPE_ID_MASK,
						mReq->req.udc_priv &
							MSM_PIPE_ID_MASK);
			}
		}
	}

	mEp->qh.ptr->td.token &=
		cpu_to_le32(~(TD_STATUS_HALTED|TD_STATUS_ACTIVE));

	if (mEp->type == USB_ENDPOINT_XFER_ISOC && mEp->dir == RX) {
		u32 mul = mReq->req.length / mEp->ep.maxpacket;

		if (mReq->req.length == 0
				|| mReq->req.length % mEp->ep.maxpacket)
			mul++;
		mEp->qh.ptr->cap |= mul << __ffs(QH_MULT);
	}

//prime:
	wmb();   /* synchronize before ep prime */

	ret = hw_ep_prime(ci, mEp->num, mEp->dir,
			   mEp->type == USB_ENDPOINT_XFER_CONTROL);
done:
	return ret;
}

/*
 * free_pending_td: remove a pending request for the endpoint
 * @mEp: endpoint
 */
static void free_pending_td(struct ci13xxx_ep *mEp)
{
	struct td_node *pending = mEp->pending_td;

	dma_pool_free(mEp->td_pool, pending->ptr, pending->dma);
	mEp->pending_td = NULL;
	kfree(pending);
}

/**
 * _hardware_dequeue: handles a request at hardware level
 * @gadget: gadget
 * @mEp:    endpoint
 *
 * This function returns an error code
 */
static int _hardware_dequeue(struct ci13xxx_ep *mEp, struct ci13xxx_req *mReq)
{
	u32 tmptoken;
	struct td_node *node, *tmpnode;
	unsigned remaining_length;
	unsigned actual = mReq->req.length;

	if (mReq->req.status != -EALREADY)
		return -EINVAL;

	/* clean speculative fetches on req->ptr->token */
	mb();

	if (CI13XX_REQ_VENDOR_ID(mReq->req.udc_priv) == MSM_VENDOR_ID)
		if ((mReq->req.udc_priv & MSM_SPS_MODE) &&
			(mReq->req.udc_priv & MSM_IS_FINITE_TRANSFER))
			return -EBUSY;

	if (mReq->req.status != -EALREADY)
		return -EINVAL;

	mReq->req.status = 0;

	list_for_each_entry_safe(node, tmpnode, &mReq->tds, td) {
		tmptoken = le32_to_cpu(node->ptr->token);
		if ((TD_STATUS_ACTIVE & tmptoken) != 0) {
			mReq->req.status = -EALREADY;
			return -EBUSY;
		}
		remaining_length = (tmptoken & TD_TOTAL_BYTES);
		remaining_length >>= __ffs(TD_TOTAL_BYTES);
		actual -= remaining_length;

		mReq->req.status = tmptoken & TD_STATUS;
		if ((TD_STATUS_HALTED & mReq->req.status)) {
			mReq->req.status = -EPIPE;
			break;
		} else if ((TD_STATUS_DT_ERR & mReq->req.status)) {
			mReq->req.status = -EPROTO;
			break;
		} else if ((TD_STATUS_TR_ERR & mReq->req.status)) {
			mReq->req.status = -EILSEQ;
			break;
		}

		if (remaining_length) {
			if (mEp->dir) {
				mReq->req.status = -EPROTO;
				break;
			}
		}
		/*
		 * As the hardware could still address the freed td
		 * which will run the udc unusable, the cleanup of the
		 * td has to be delayed by one.
		 */
		if (mEp->pending_td)
			free_pending_td(mEp);

		mEp->pending_td = node;
		list_del_init(&node->td);
	}

	usb_gadget_unmap_request(&mEp->udc->gadget, &mReq->req, mEp->dir);

	mReq->req.actual += actual;

	if (mReq->req.status)
		return mReq->req.status;

	return mReq->req.actual;
}

/**
 * purge_rw_queue: Purge requests pending at the remote-wakeup
 * queue and send them to the HW.
 *
 * Go over all of the endpoints and push any pending requests to
 * the HW queue.
 */
static void purge_rw_queue(struct ci13xxx *ci)
{
	int i;
	struct ci13xxx_ep  *mEp  = NULL;
	struct ci13xxx_req *mReq = NULL;

	/*
	 * Go over all of the endpoints and push any pending requests to
	 * the HW queue.
	 */
	for (i = 0; i < ci->hw_ep_max; i++) {
		mEp = &ci->ci13xxx_ep[i];

		while (!list_empty(&ci->ci13xxx_ep[i].rw_queue)) {
			int retval;

			/* pop oldest request */
			mReq = list_entry(ci->ci13xxx_ep[i].rw_queue.next,
					  struct ci13xxx_req, queue);

			list_del_init(&mReq->queue);

			retval = _hardware_enqueue(mEp, mReq);

			if (retval != 0) {
				dbg_event(_usb_addr(mEp), "QUEUE", retval);
				mReq->req.status = retval;
				if (mReq->req.complete != NULL) {
					if (mEp->type ==
					    USB_ENDPOINT_XFER_CONTROL)
						usb_gadget_giveback_request(
							&(ci->ep0in->ep),
							&mReq->req);
					else
						usb_gadget_giveback_request(
							&mEp->ep,
							&mReq->req);
				}
				retval = 0;
			}

			if (!retval)
				list_add_tail(&mReq->queue, &mEp->qh.queue);

		}
	}

	ci->rw_pending = false;
}

/**
 * release_ep_request: Free and endpoint request and release
 * resources
 * @mReq: request
 * @mEp: endpoint
 *
 */
static void release_ep_request(struct ci13xxx_ep  *mEp,
			       struct ci13xxx_req *mReq)
{
	//struct ci13xxx_ep *mEpTemp = mEp;
	struct ci13xxx *ci = mEp->udc;

	unsigned val;

	/* MSM Specific: Clear end point specific register */
	if (CI13XX_REQ_VENDOR_ID(mReq->req.udc_priv) == MSM_VENDOR_ID) {
		if (mReq->req.udc_priv & MSM_SPS_MODE) {
			val = hw_read(ci, OP_ENDPTPIPEID +
				mEp->num * sizeof(u32),
				~0);

			if (val != MSM_EP_PIPE_ID_RESET_VAL)
				hw_write(ci, 
					OP_ENDPTPIPEID +
					 mEp->num * sizeof(u32),
					~0, MSM_EP_PIPE_ID_RESET_VAL);
		}
	}

	mReq->req.status = -ESHUTDOWN;

	usb_gadget_unmap_request(&mEp->udc->gadget, &mReq->req, mEp->dir);


	if (mReq->req.complete != NULL) {
		spin_unlock(mEp->lock);
		usb_gadget_giveback_request(&mEp->ep, &mReq->req);
		spin_lock(mEp->lock);
	}
/*
	if (mReq->req.complete != NULL) {
		spin_unlock(mEp->lock);
		if ((mEp->type == USB_ENDPOINT_XFER_CONTROL) &&
			mReq->req.length)
			mEpTemp = ci->ep0in;
		mReq->req.complete(&mEpTemp->ep, &mReq->req);
		if (mEp->type == USB_ENDPOINT_XFER_CONTROL)
			mReq->req.complete = NULL;
		spin_lock(mEp->lock);
	}
*/
}

/**
 * _ep_nuke: dequeues all endpoint requests
 * @mEp: endpoint
 *
 * This function returns an error code
 * Caller must hold lock
 */
static int _ep_nuke(struct ci13xxx_ep *mEp)
__releases(mEp->lock)
__acquires(mEp->lock)
{
	struct td_node *node, *tmpnode;

	trace("%pK", mEp);

	if (mEp == NULL)
		return -EINVAL;

	hw_ep_flush(mEp->udc, mEp->num, mEp->dir);

	while (!list_empty(&mEp->qh.queue)) {
		/* pop oldest request */
		struct ci13xxx_req *mReq =
			list_entry(mEp->qh.queue.next,
				   struct ci13xxx_req, queue);

		list_for_each_entry_safe(node, tmpnode, &mReq->tds, td) {
			dma_pool_free(mEp->td_pool, node->ptr, node->dma);
			list_del_init(&node->td);
			node->ptr = NULL;
			kfree(node);
		}

		list_del_init(&mReq->queue);

		release_ep_request(mEp, mReq);
	}

	/* Clear the requests pending at the remote-wakeup queue */
	while (!list_empty(&mEp->rw_queue)) {

		/* pop oldest request */
		struct ci13xxx_req *mReq =
			list_entry(mEp->rw_queue.next,
				   struct ci13xxx_req, queue);

		list_del_init(&mReq->queue);

		release_ep_request(mEp, mReq);
	}

	if (mEp->pending_td)
		free_pending_td(mEp);

	return 0;
}

static int _ep_set_halt(struct usb_ep *ep, int value, bool check_transfer)
{
	struct ci13xxx_ep *hwep = container_of(ep, struct ci13xxx_ep, ep);
	int direction, retval = 0;
	unsigned long flags;

	if (ep == NULL || hwep->ep.desc == NULL)
		return -EINVAL;

	if (usb_endpoint_xfer_isoc(hwep->ep.desc))
		return -EOPNOTSUPP;

	spin_lock_irqsave(hwep->lock, flags);

	if (value && hwep->dir == TX && check_transfer &&
		!list_empty(&hwep->qh.queue) &&
			!usb_endpoint_xfer_control(hwep->ep.desc)) {
		spin_unlock_irqrestore(hwep->lock, flags);
		return -EAGAIN;
	}

	direction = hwep->dir;
	do {
		retval |= hw_ep_set_halt(hwep->udc, hwep->num, hwep->dir, value);

		if (!value)
			hwep->wedge = 0;

		if (hwep->type == USB_ENDPOINT_XFER_CONTROL)
			hwep->dir = (hwep->dir == TX) ? RX : TX;

	} while (hwep->dir != direction);

	spin_unlock_irqrestore(hwep->lock, flags);
	return retval;
}

/**
 * _gadget_stop_activity: stops all USB activity, flushes & disables all endpts
 * @gadget: gadget
 *
 * This function returns an error code
 */
static int _gadget_stop_activity(struct usb_gadget *gadget)
{
	struct usb_ep *ep;
	struct ci13xxx    *ci = container_of(gadget, struct ci13xxx, gadget);
	unsigned long flags;

	trace("%pK", gadget);

	spin_lock_irqsave(&ci->lock, flags);
	ci->gadget.speed = USB_SPEED_UNKNOWN;
	ci->gadget.remote_wakeup = 0;
	ci->suspended = 0;
	ci->configured = 0;
	spin_unlock_irqrestore(&ci->lock, flags);

	//gadget->xfer_isr_count = 0;
	//ci->driver->disconnect(gadget);
	//usb_gadget_udc_reset(gadget, ci->driver);

/*
	spin_lock_irqsave(&ci->lock, flags);
	_ep_nuke(ci->ep0out);
	_ep_nuke(ci->ep0in);
	spin_unlock_irqrestore(&ci->lock, flags);
*/

	/* flush all endpoints */
	gadget_for_each_ep(ep, gadget) {
		usb_ep_fifo_flush(ep);
	}
	usb_ep_fifo_flush(&ci->ep0out->ep);
	usb_ep_fifo_flush(&ci->ep0in->ep);

	/* make sure to disable all endpoints */
	gadget_for_each_ep(ep, gadget) {
		usb_ep_disable(ep);
	}

	if (ci->status != NULL) {
		usb_ep_free_request(&ci->ep0in->ep, ci->status);
		ci->status = NULL;
	}

	return 0;
}

/******************************************************************************
 * ISR block
 *****************************************************************************/
/**
 * isr_reset_handler: USB reset interrupt handler
 * @ci: UDC device
 *
 * This function resets USB engine after a bus reset occurred
 */
static void isr_reset_handler(struct ci13xxx *ci)
__releases(ci->lock)
__acquires(ci->lock)
{
	int retval;

	trace("%pK", ci);

	if (ci == NULL) {
		err("EINVAL");
		return;
	}

	dbg_event(0xFF, "BUS RST", 0);

	spin_unlock(&ci->lock);

	if (ci->gadget.speed != USB_SPEED_UNKNOWN)
		usb_gadget_udc_reset(&ci->gadget, ci->driver);

	if (ci->suspended) {
		if (ci->udc_driver->notify_event)
			ci->udc_driver->notify_event(ci,
			CI13XXX_CONTROLLER_RESUME_EVENT);
		if (ci->transceiver)
			usb_phy_set_suspend(ci->transceiver, 0);
		ci->driver->resume(&ci->gadget);
		ci->suspended = 0;
	}

	/*stop charging upon reset */
	if (ci->transceiver)
		usb_phy_set_power(ci->transceiver, 100);

	retval = _gadget_stop_activity(&ci->gadget);
	if (retval)
		goto done;

	if (ci->rw_pending)
		purge_rw_queue(ci);

	ci->skip_flush = false;
	retval = hw_usb_reset(ci);
	if (retval)
		goto done;

	ci->status = usb_ep_alloc_request(&ci->ep0in->ep, GFP_ATOMIC);
	if (!ci->status) {
		retval = -ENOMEM;
	}
done:
	spin_lock(&ci->lock);

	if (retval)
		err("error: %i", retval);
}

/**
 * isr_resume_handler: USB PCI interrupt handler
 * @ci: UDC device
 *
 */
static void isr_resume_handler(struct ci13xxx *ci)
{
	ci->gadget.speed = hw_port_is_high_speed(ci) ?
		USB_SPEED_HIGH : USB_SPEED_FULL;
	if (ci->suspended) {
		spin_unlock(&ci->lock);
		if (ci->udc_driver->notify_event)
			ci->udc_driver->notify_event(ci,
			  CI13XXX_CONTROLLER_RESUME_EVENT);
		if (ci->transceiver)
			usb_phy_set_suspend(ci->transceiver, 0);
		ci->suspended = 0;
		ci->driver->resume(&ci->gadget);
		spin_lock(&ci->lock);

		if (ci->rw_pending)
			purge_rw_queue(ci);

	}
}

/**
 * isr_get_status_complete: get_status request complete function
 * @ep:  endpoint
 * @req: request handled
 *
 * Caller must release lock
 */
static void isr_get_status_complete(struct usb_ep *ep, struct usb_request *req)
{
	trace("%pK, %pK", ep, req);

	if (ep == NULL || req == NULL) {
		err("EINVAL");
		return;
	}

	if (req->status)
		err("GET_STATUS failed");
}

/**
 * _ep_queue: queues (submits) an I/O request to an endpoint
 *
 * Caller must hold lock
 */
static int _ep_queue(struct usb_ep *ep, struct usb_request *req,
		    gfp_t __maybe_unused gfp_flags)
{
	struct ci13xxx_ep  *mEp  = container_of(ep,  struct ci13xxx_ep, ep);
	struct ci13xxx_req *mReq = container_of(req, struct ci13xxx_req, req);
	int retval = 0;
	struct ci13xxx *ci = mEp->udc;

	trace("%pK, %pK, %X", ep, req, gfp_flags);

	if (ep == NULL || req == NULL || mEp->ep.desc == NULL)
		return -EINVAL;

	if (!ci->softconnect)
		return -ENODEV;

	if (mEp->type == USB_ENDPOINT_XFER_CONTROL) {
		if (req->length)
			mEp = (ci->ep0_dir == RX) ?
				ci->ep0out : ci->ep0in;
		if (!list_empty(&mEp->qh.queue)) {
			_ep_nuke(mEp);
			retval = -EOVERFLOW;
			warn("endpoint ctrl %X nuked", _usb_addr(mEp));
		}
	}

	if (usb_endpoint_xfer_isoc(mEp->ep.desc) &&
	    mReq->req.length > (1 + mEp->ep.mult) * mEp->ep.maxpacket) {
		err("request length too big for isochronous\n");
		return -EMSGSIZE;
	}

	/* first nuke then test link, e.g. previous status has not sent */
	if (!list_empty(&mReq->queue)) {
		err("request already in queue");
		return -EBUSY;
	}

	/* push request */
	mReq->req.status = -EINPROGRESS;
	mReq->req.actual = 0;

	if (ci->rw_pending) {
		list_add_tail(&mReq->queue, &mEp->rw_queue);
		return 0;
	}

	if (ci->suspended) {
		/* Remote Wakeup */
		if (!ci->gadget.remote_wakeup) {
			return -EAGAIN;
		}

		list_add_tail(&mReq->queue, &mEp->rw_queue);

		ci->rw_pending = true;
		schedule_delayed_work(&ci->rw_work,
				      REMOTE_WAKEUP_DELAY);

		return 0;
	}

	retval = _hardware_enqueue(mEp, mReq);

	if (retval == -EALREADY) {
		dbg_event(_usb_addr(mEp), "QUEUE", retval);
		retval = 0;
	}
	if (!retval)
		list_add_tail(&mReq->queue, &mEp->qh.queue);

	return retval;
}

/**
 * isr_get_status_response: get_status request response
 * @ci: ci struct
 * @setup: setup request packet
 *
 * This function returns an error code
 */
static int isr_get_status_response(struct ci13xxx *ci,
				   struct usb_ctrlrequest *setup)
__releases(mEp->lock)
__acquires(mEp->lock)
{
	struct ci13xxx_ep *mEp = ci->ep0in;
	struct usb_request *req = ci->status;
	int dir, num, retval;

	trace("%pK, %pK", mEp, setup);

	if (mEp == NULL || setup == NULL)
		return -EINVAL;

	spin_unlock(mEp->lock);
	req = usb_ep_alloc_request(&mEp->ep, GFP_ATOMIC);
	spin_lock(mEp->lock);
	if (req == NULL)
		return -ENOMEM;

	req->complete = isr_get_status_complete;
	req->length   = 2;
	req->buf      = kzalloc(req->length, GFP_ATOMIC);
	if (req->buf == NULL) {
		retval = -ENOMEM;
		goto err_free_req;
	}

	if ((setup->bRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE) {
		/* Assume that device is bus powered for now. */
		*(u16 *)req->buf = (ci->gadget.remote_wakeup << 1) |
			ci->gadget.is_selfpowered;
		retval = 0;
	} else if ((setup->bRequestType & USB_RECIP_MASK) \
		   == USB_RECIP_ENDPOINT) {
		dir = (le16_to_cpu(setup->wIndex) & USB_ENDPOINT_DIR_MASK) ?
			TX : RX;
		num =  le16_to_cpu(setup->wIndex) & USB_ENDPOINT_NUMBER_MASK;
		*(u16 *)req->buf = hw_ep_get_halt(ci, num, dir);
	}
	/* else do nothing; reserved for future use */

	retval = _ep_queue(&mEp->ep, req, GFP_ATOMIC);
	if (retval)
		goto err_free_buf;

	return 0;

 err_free_buf:
	kfree(req->buf);
 err_free_req:
	spin_unlock(mEp->lock);
	usb_ep_free_request(&mEp->ep, req);
	spin_lock(mEp->lock);
	return retval;
}

/**
 * isr_setup_status_complete: setup_status request complete function
 * @ep:  endpoint
 * @req: request handled
 *
 * Caller must release lock. Put the port in test mode if test mode
 * feature is selected.
 */
static void
isr_setup_status_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct ci13xxx *ci = req->context;
	unsigned long flags;

	trace("%pK, %pK", ep, req);

	if (ci->setaddr) {
		hw_usb_set_address(ci, ci->address);
		ci->setaddr = false;
		if (ci->address)
			usb_gadget_set_state(&ci->gadget, USB_STATE_ADDRESS);
	}

	spin_lock_irqsave(&ci->lock, flags);
	if (ci->test_mode)
		hw_port_test_set(ci, ci->test_mode);
	spin_unlock_irqrestore(&ci->lock, flags);
}

/**
 * isr_setup_status_phase: queues the status phase of a setup transation
 * @ci: ci struct
 *
 * This function returns an error code
 */
static int isr_setup_status_phase(struct ci13xxx *ci)
__releases(mEp->lock)
__acquires(mEp->lock)
{
	int retval;
	struct ci13xxx_ep *mEp;

	trace("%pK", ci);

	mEp = (ci->ep0_dir == TX) ? ci->ep0out : ci->ep0in;
	ci->status->context = ci;
	ci->status->complete = isr_setup_status_complete;
	//ci->status->length = 0;
/*
	spin_unlock(mEp->lock);
	retval = usb_ep_queue(&mEp->ep, ci->status, GFP_ATOMIC);
	spin_lock(mEp->lock);
// freezes the device for some reason
*/	retval = _ep_queue(&mEp->ep, ci->status, GFP_ATOMIC);

	return retval;
}

/**
 * isr_tr_complete_low: transaction complete low level handler
 * @mEp: endpoint
 *
 * This function returns an error code
 * Caller must hold lock
 */
static int isr_tr_complete_low(struct ci13xxx_ep *mEp)
__releases(mEp->lock)
__acquires(mEp->lock)
{
	struct ci13xxx_req *mReq, *mReqTemp;
	struct ci13xxx_ep *mEpTemp = mEp;
	int retval = 0;
	int req_dequeue = 1;
	struct ci13xxx *ci = mEp->udc;

	trace("%pK", mEp);

/* UNCOMMENT ME IF RNDIS PROBLEMS
	if (list_empty(&mEp->qh.queue))
		return 0;
*/

	list_for_each_entry_safe(mReq, mReqTemp, &mEp->qh.queue,
			queue) {
dequeue:
		retval = _hardware_dequeue(mEp, mReq);
		if (retval < 0) {
			/*
			 * FIXME: don't know exact delay
			 * required for HW to update dTD status
			 * bits. This is a temporary workaround till
			 * HW designers come back on this.
			 */
			if (retval == -EBUSY && req_dequeue &&
				(mEp->dir == 0 || mEp->num == 0)) {
				req_dequeue = 0;
				ci->dTD_update_fail_count++;
				mEp->dTD_update_fail_count++;
				udelay(10);
				goto dequeue;
			}
			break;
		}
		req_dequeue = 0;

		list_del_init(&mReq->queue);

		if (mReq->req.complete != NULL) {
			spin_unlock(mEp->lock);
			if ((mEp->type == USB_ENDPOINT_XFER_CONTROL) &&
					mReq->req.length)
				mEpTemp = ci->ep0in;
			usb_gadget_giveback_request(&mEpTemp->ep, &mReq->req);
			spin_lock(mEp->lock);
		}
	}

	if (retval == -EBUSY)
		retval = 0;

	return retval;
}

/**
 * isr_setup_packet_handler: setup packet handler
 * @ci: UDC descriptor
 *
 * This function handles setup packet 
 */
static void isr_setup_packet_handler(struct ci13xxx *ci)
__releases(ci->lock)
__acquires(ci->lock)
{
	struct ci13xxx_ep *hwep = &ci->ci13xxx_ep[0];
	struct usb_ctrlrequest req;
	int type, num, dir, err = -EINVAL;
	u8 tmode = 0;

	/*
	 * Flush data and handshake transactions of previous
	 * setup packet.
	 */
	_ep_nuke(ci->ep0out);
	_ep_nuke(ci->ep0in);

	/* read_setup_packet */
	do {
		hw_test_and_set_setup_guard(ci);
		memcpy(&req, &hwep->qh.ptr->setup, sizeof(req));
	} while (!hw_test_and_clear_setup_guard(ci));

	type = req.bRequestType;

	ci->ep0_dir = (type & USB_DIR_IN) ? TX : RX;

	switch (req.bRequest) {
	case USB_REQ_CLEAR_FEATURE:
		if (type == (USB_DIR_OUT|USB_RECIP_ENDPOINT) &&
				le16_to_cpu(req.wValue) ==
				USB_ENDPOINT_HALT) {
			if (req.wLength != 0)
				break;
			num  = le16_to_cpu(req.wIndex);
			dir = num & USB_ENDPOINT_DIR_MASK;
			num &= USB_ENDPOINT_NUMBER_MASK;
			if (dir) /* TX */
				num += ci->hw_ep_max / 2;
			if (!ci->ci13xxx_ep[num].wedge) {
				spin_unlock(&ci->lock);
				err = usb_ep_clear_halt(
					&ci->ci13xxx_ep[num].ep);
				spin_lock(&ci->lock);
				if (err)
					break;
			}
			err = isr_setup_status_phase(ci);
		} else if (type == (USB_DIR_OUT|USB_RECIP_DEVICE) &&
				le16_to_cpu(req.wValue) ==
				USB_DEVICE_REMOTE_WAKEUP) {
			if (req.wLength != 0)
				break;
			ci->gadget.remote_wakeup = 0;
			err = isr_setup_status_phase(ci);
		} else {
			goto delegate;
		}
		break;
	case USB_REQ_GET_STATUS:
		if (type != (USB_DIR_IN|USB_RECIP_DEVICE)   &&
		    type != (USB_DIR_IN|USB_RECIP_ENDPOINT) &&
		    type != (USB_DIR_IN|USB_RECIP_INTERFACE))
			goto delegate;
		if (le16_to_cpu(req.wLength) != 2 ||
		    le16_to_cpu(req.wValue)  != 0)
			break;
		err = isr_get_status_response(ci, &req);
		break;
	case USB_REQ_SET_ADDRESS:
		if (type != (USB_DIR_OUT|USB_RECIP_DEVICE))
			goto delegate;
		if (le16_to_cpu(req.wLength) != 0 ||
		    le16_to_cpu(req.wIndex)  != 0)
			break;
		ci->address = (u8)le16_to_cpu(req.wValue);
		ci->setaddr = true;
		err = isr_setup_status_phase(ci);
		break;
	case USB_REQ_SET_FEATURE:
		if (type == (USB_DIR_OUT|USB_RECIP_ENDPOINT) &&
				le16_to_cpu(req.wValue) ==
				USB_ENDPOINT_HALT) {
			if (req.wLength != 0)
				break;
			num  = le16_to_cpu(req.wIndex);
			dir = num & USB_ENDPOINT_DIR_MASK;
			num &= USB_ENDPOINT_NUMBER_MASK;
			if (dir) /* TX */
				num += ci->hw_ep_max / 2;

			spin_unlock(&ci->lock);
			err = _ep_set_halt(&ci->ci13xxx_ep[num].ep, 1, false);
			spin_lock(&ci->lock);
			if (!err)
				isr_setup_status_phase(ci);
		} else if (type == (USB_DIR_OUT|USB_RECIP_DEVICE)) {
			if (req.wLength != 0)
				break;
			switch (le16_to_cpu(req.wValue)) {
			case USB_DEVICE_REMOTE_WAKEUP:
				ci->gadget.remote_wakeup = 1;
				err = isr_setup_status_phase(ci);
				break;
			case USB_DEVICE_TEST_MODE:
				tmode = le16_to_cpu(req.wIndex) >> 8;
				switch (tmode) {
				case TEST_J:
				case TEST_K:
				case TEST_SE0_NAK:
				case TEST_PACKET:
				case TEST_FORCE_EN:
					ci->test_mode = tmode;
					err = isr_setup_status_phase(
							ci);
					break;
				default:
					break;
				}
			default:
				goto delegate;
			}
		} else {
			goto delegate;
		}
		break;
	default:
delegate:
		if (req.wLength == 0)   /* no data phase */
			ci->ep0_dir = TX;

		spin_unlock(&ci->lock);
		err = ci->driver->setup(&ci->gadget, &req);
		spin_lock(&ci->lock);
		break;
	}

	if (err < 0) {
		spin_unlock(&ci->lock);
		if (_ep_set_halt(&hwep->ep, 1, false))
			err("error: _ep_set_halt\n");
		spin_lock(&ci->lock);
	}
}

/**
 * isr_tr_complete_handler: transaction complete interrupt handler
 * @ci: UDC descriptor
 *
 * This function handles traffic events
 */
static void isr_tr_complete_handler(struct ci13xxx *ci)
__releases(ci->lock)
__acquires(ci->lock)
{
	unsigned i;
	int err;

	trace("%pK", ci);

	if (ci == NULL) {
		err("EINVAL");
		return;
	}

	for (i = 0; i < ci->hw_ep_max; i++) {
		struct ci13xxx_ep *mEp  = &ci->ci13xxx_ep[i];

		if (mEp->ep.desc == NULL)
			continue;   /* not configured */

		if (hw_test_and_clear_complete(ci, i)) {
			err = isr_tr_complete_low(mEp);
			if (mEp->type == USB_ENDPOINT_XFER_CONTROL) {
				if (err > 0)   /* needs status phase */
					err = isr_setup_status_phase(ci);
				if (err < 0) {
					dbg_event(_usb_addr(mEp),
						  "ERROR", err);
					spin_unlock(&ci->lock);
					if (_ep_set_halt(&mEp->ep, 1, false))
						err("error: _ep_set_halt");
					spin_lock(&ci->lock);
				}
			}
		}

		/* Only handle setup packet below */
		if (i == 0 &&
			hw_test_and_clear(ci, OP_ENDPTSETUPSTAT, BIT(0)))
			isr_setup_packet_handler(ci);
	}
}

/******************************************************************************
 * ENDPT block
 *****************************************************************************/
/**
 * ep_enable: configure endpoint, making it usable
 *
 * Check usb_ep_enable() at "usb_gadget.h" for details
 */
static int ep_enable(struct usb_ep *ep,
		     const struct usb_endpoint_descriptor *desc)
{
	struct ci13xxx_ep *mEp = container_of(ep, struct ci13xxx_ep, ep);
	int retval = 0;
	unsigned long flags;
	u32 cap = 0;

	trace("ep = %pK, desc = %pK", ep, desc);

	if (ep == NULL || desc == NULL)
		return -EINVAL;

	spin_lock_irqsave(mEp->lock, flags);

	/* only internal SW should enable ctrl endpts */

	if (!list_empty(&mEp->qh.queue)) {
		warn("enabling a non-empty endpoint!");
		spin_unlock_irqrestore(mEp->lock, flags);
		return -EBUSY;
	}

	mEp->ep.desc = desc;

	mEp->dir  = usb_endpoint_dir_in(desc) ? TX : RX;
	mEp->num  = usb_endpoint_num(desc);
	mEp->type = usb_endpoint_type(desc);

	mEp->ep.maxpacket = usb_endpoint_maxp(desc) & 0x07ff;
	mEp->ep.mult = QH_ISO_MULT(usb_endpoint_maxp(desc));

	dbg_event(_usb_addr(mEp), "ENABLE", 0);

	if (mEp->type == USB_ENDPOINT_XFER_CONTROL)
		cap |= QH_IOS;
	if (mEp->num)
		cap |= QH_ZLT;
	cap |= (mEp->ep.maxpacket << __ffs(QH_MAX_PKT)) & QH_MAX_PKT;
	/*
	 * For ISO-TX, we set mult at QH as the largest value, and use
	 * MultO at TD as real mult value.
	 */
	if (mEp->type == USB_ENDPOINT_XFER_ISOC && mEp->dir == TX)
		cap |= 3 << __ffs(QH_MULT);

	mEp->qh.ptr->cap = cpu_to_le32(cap);

	mEp->qh.ptr->td.next |= cpu_to_le32(TD_TERMINATE);   /* needed? */

	if (mEp->num != 0 && mEp->type == USB_ENDPOINT_XFER_CONTROL) {
		err("Set control xfer at non-ep0\n");
		retval = -EINVAL;
	}

	/*
	 * Enable endpoints in the HW other than ep0 as ep0
	 * is always enabled
	 */
	if (mEp->num)
		retval |= hw_ep_enable(mEp->udc, mEp->num, mEp->dir, mEp->type);

	spin_unlock_irqrestore(mEp->lock, flags);
	return retval;
}

/**
 * ep_disable: endpoint is no longer usable
 *
 * Check usb_ep_disable() at "usb_gadget.h" for details
 */
static int ep_disable(struct usb_ep *ep)
{
	struct ci13xxx_ep *mEp = container_of(ep, struct ci13xxx_ep, ep);
	int direction, retval = 0;
	unsigned long flags;

	trace("%pK", ep);

	if (ep == NULL)
		return -EINVAL;
	else if (mEp->ep.desc == NULL)
		return -EBUSY;

	spin_lock_irqsave(mEp->lock, flags);

	/* only internal SW should disable ctrl endpts */

	direction = mEp->dir;
	do {
		dbg_event(_usb_addr(mEp), "DISABLE", 0);

		retval |= _ep_nuke(mEp);
		retval |= hw_ep_disable(mEp->udc, mEp->num, mEp->dir);

		if (mEp->type == USB_ENDPOINT_XFER_CONTROL)
			mEp->dir = (mEp->dir == TX) ? RX : TX;

	} while (mEp->dir != direction);

	mEp->ep.desc = NULL;

	spin_unlock_irqrestore(mEp->lock, flags);
	return retval;
}

/**
 * ep_alloc_request: allocate a request object to use with this endpoint
 *
 * Check usb_ep_alloc_request() at "usb_gadget.h" for details
 */
static struct usb_request *ep_alloc_request(struct usb_ep *ep, gfp_t gfp_flags)
{
	struct ci13xxx_req *mReq = NULL;

	if (ep == NULL)
		return NULL;

	mReq = kzalloc(sizeof(struct ci13xxx_req), gfp_flags);
	if (mReq != NULL) {
		INIT_LIST_HEAD(&mReq->queue);
		INIT_LIST_HEAD(&mReq->tds);
	}

	return (mReq == NULL) ? NULL : &mReq->req;
}

/**
 * ep_free_request: frees a request object
 *
 * Check usb_ep_free_request() at "usb_gadget.h" for details
 */
static void ep_free_request(struct usb_ep *ep, struct usb_request *req)
{
	struct ci13xxx_ep  *mEp  = container_of(ep,  struct ci13xxx_ep, ep);
	struct ci13xxx_req *mReq = container_of(req, struct ci13xxx_req, req);
	struct td_node *node, *tmpnode;
	unsigned long flags;

	trace("%pK, %pK", ep, req);

	if (ep == NULL || req == NULL) {
		err("EINVAL");
		return;
	} else if (!list_empty(&mReq->queue)) {
		err("EBUSY");
		return;
	}

	spin_lock_irqsave(mEp->lock, flags);

	list_for_each_entry_safe(node, tmpnode, &mReq->tds, td) {
		dma_pool_free(mEp->td_pool, node->ptr, node->dma);
		list_del_init(&node->td);
		node->ptr = NULL;
		kfree(node);
	}

	kfree(mReq);

	dbg_event(_usb_addr(mEp), "FREE", 0);

	spin_unlock_irqrestore(mEp->lock, flags);
}

/**
 * ep_queue: queues (submits) an I/O request to an endpoint
 *
 * Check usb_ep_queue()* at usb_gadget.h" for details
 */
static int ep_queue(struct usb_ep *ep, struct usb_request *req,
		    gfp_t __maybe_unused gfp_flags)
{
	struct ci13xxx_ep  *mEp  = container_of(ep,  struct ci13xxx_ep, ep);
	struct ci13xxx *ci = mEp->udc;
	int retval = 0;
	unsigned long flags;

	trace("%pK, %pK, %X", ep, req, gfp_flags);

	if (ep == NULL || req == NULL || mEp->ep.desc == NULL)
		return -EINVAL;

	if (!ci->softconnect)
		return -ENODEV;

	spin_lock_irqsave(mEp->lock, flags);
	retval = _ep_queue(ep, req, gfp_flags);
	spin_unlock_irqrestore(mEp->lock, flags);
	return retval;
}

/**
 * ep_dequeue: dequeues (cancels, unlinks) an I/O request from an endpoint
 *
 * Check usb_ep_dequeue() at "usb_gadget.h" for details
 */
static int ep_dequeue(struct usb_ep *ep, struct usb_request *req)
{
	struct ci13xxx_ep  *mEp  = container_of(ep,  struct ci13xxx_ep, ep);
	struct ci13xxx_ep *mEpTemp = mEp;
	struct ci13xxx_req *mReq = container_of(req, struct ci13xxx_req, req);
	struct ci13xxx *ci = mEp->udc;
	unsigned long flags;
	struct td_node *node, *tmpnode;

	trace("%pK, %pK", ep, req);

	if (ci->udc_driver->in_lpm && ci->udc_driver->in_lpm(ci)) {
		dev_err(ci->transceiver->dev,
				"%s: Unable to dequeue while in LPM\n",
				__func__);
		return -EAGAIN;
	}
	spin_lock_irqsave(mEp->lock, flags);
	/*
	 * Only ep0 IN is exposed to composite.  When a req is dequeued
	 * on ep0, check both ep0 IN and ep0 OUT queues.
	 */
	if (ep == NULL || req == NULL || mReq->req.status != -EALREADY ||
		mEp->ep.desc == NULL || list_empty(&mReq->queue) ||
		(list_empty(&mEp->qh.queue) && ((mEp->type !=
			USB_ENDPOINT_XFER_CONTROL) ||
			list_empty(&ci->ep0out->qh.queue)))) {
		spin_unlock_irqrestore(mEp->lock, flags);
		return -EINVAL;
	}

	dbg_event(_usb_addr(mEp), "DEQUEUE", 0);

	if ((mEp->type == USB_ENDPOINT_XFER_CONTROL)) {
		hw_ep_flush(ci, ci->ep0out->num, RX);
		hw_ep_flush(ci, ci->ep0in->num, TX);
	} else {
		hw_ep_flush(mEp->udc, mEp->num, mEp->dir);
	}

	list_for_each_entry_safe(node, tmpnode, &mReq->tds, td) {
		dma_pool_free(mEp->td_pool, node->ptr, node->dma);
		list_del(&node->td);
		kfree(node);
	}

	/* pop request */
	list_del_init(&mReq->queue);

	usb_gadget_unmap_request(&mEp->udc->gadget, req, mEp->dir);

	req->status = -ECONNRESET;

	if (mReq->req.complete != NULL) {
		spin_unlock(mEp->lock);
		usb_gadget_giveback_request(&mEpTemp->ep, &mReq->req);
		spin_lock(mEp->lock);
	}

	spin_unlock_irqrestore(mEp->lock, flags);
	return 0;
}

/**
 * ep_set_halt: sets the endpoint halt feature
 *
 * Check usb_ep_set_halt() at "usb_gadget.h" for details
 */
static int ep_set_halt(struct usb_ep *ep, int value)
{
	struct ci13xxx_ep  *mEp  = container_of(ep,  struct ci13xxx_ep, ep);
	struct ci13xxx *ci = mEp->udc;

	if (ci->suspended) {
		dev_err(ci->transceiver->dev,
			"%s: Unable to halt EP while suspended\n", __func__);
		return -EINVAL;
	}
	return _ep_set_halt(ep, value, true);
}

/**
 * ep_set_wedge: sets the halt feature and ignores clear requests
 *
 * Check usb_ep_set_wedge() at "usb_gadget.h" for details
 */
static int ep_set_wedge(struct usb_ep *ep)
{
	struct ci13xxx_ep *mEp = container_of(ep, struct ci13xxx_ep, ep);
	unsigned long flags;

	trace("%pK", ep);

	if (ep == NULL || mEp->ep.desc == NULL)
		return -EINVAL;

	spin_lock_irqsave(mEp->lock, flags);

	dbg_event(_usb_addr(mEp), "WEDGE", 0);
	mEp->wedge = 1;

	spin_unlock_irqrestore(mEp->lock, flags);

	return usb_ep_set_halt(ep);
}

/**
 * ep_fifo_flush: flushes contents of a fifo
 *
 * Check usb_ep_fifo_flush() at "usb_gadget.h" for details
 */
static void ep_fifo_flush(struct usb_ep *ep)
{
	struct ci13xxx_ep *mEp = container_of(ep, struct ci13xxx_ep, ep);
	struct ci13xxx *ci = mEp->udc;
	unsigned long flags;

	trace("%pK", ep);

	if (ep == NULL) {
		err("%02X: -EINVAL", _usb_addr(mEp));
		return;
	}

	if (ci->udc_driver->in_lpm && ci->udc_driver->in_lpm(ci)) {
		dev_err(ci->transceiver->dev,
				"%s: Unable to fifo_flush while in LPM\n",
				__func__);
		return;
	}

	spin_lock_irqsave(mEp->lock, flags);

	dbg_event(_usb_addr(mEp), "FFLUSH", 0);
	/*
	 * _ep_nuke() takes care of flushing the endpoint.
	 * some function drivers expect udc to retire all
	 * pending requests upon flushing an endpoint.  There
	 * is no harm in doing it.
	 */
	_ep_nuke(mEp);

	spin_unlock_irqrestore(mEp->lock, flags);
}

/**
 * Endpoint-specific part of the API to the USB controller hardware
 * Check "usb_gadget.h" for details
 */
static const struct usb_ep_ops usb_ep_ops = {
	.enable	       = ep_enable,
	.disable       = ep_disable,
	.alloc_request = ep_alloc_request,
	.free_request  = ep_free_request,
	.queue	       = ep_queue,
	.dequeue       = ep_dequeue,
	.set_halt      = ep_set_halt,
	.set_wedge     = ep_set_wedge,
	.fifo_flush    = ep_fifo_flush,
};

/******************************************************************************
 * GADGET block
 *****************************************************************************/
static int ci13xxx_vbus_session(struct usb_gadget *_gadget, int is_active)
{
	struct ci13xxx *ci = container_of(_gadget, struct ci13xxx, gadget);
	unsigned long flags;
	int gadget_ready = 0;

	if (!(ci->udc_driver->flags & CI13XXX_PULLUP_ON_VBUS))
		return -EOPNOTSUPP;

	spin_lock_irqsave(&ci->lock, flags);
	ci->vbus_active = is_active;
	if (ci->driver)
		gadget_ready = 1;
	spin_unlock_irqrestore(&ci->lock, flags);

	if (!gadget_ready)
		return 0;

	if (is_active) {
		hw_device_reset(ci);
		if (ci->udc_driver->notify_event)
			ci->udc_driver->notify_event(ci,
				CI13XXX_CONTROLLER_CONNECT_EVENT);
		/* Enable BAM (if needed) before starting controller */
		if (ci->softconnect) {
#ifdef USE_BAM2BAM
			dbg_event(0xFF, "BAM EN2",
				_gadget->bam2bam_func_enabled);
			msm_usb_bam_enable(CI_CTRL,
				_gadget->bam2bam_func_enabled);
#endif
			hw_device_state(ci, ci->ep0out->qh.dma);
		}
		usb_gadget_set_state(_gadget, USB_STATE_POWERED);
	} else {
		if (ci->driver)
			ci->driver->disconnect(&ci->gadget);
		hw_device_state(ci, 0);
		_gadget_stop_activity(&ci->gadget);
		if (ci->udc_driver->notify_event)
			ci->udc_driver->notify_event(ci,
				CI13XXX_CONTROLLER_DISCONNECT_EVENT);

		usb_gadget_set_state(_gadget, USB_STATE_NOTATTACHED);
	}

	return 0;
}

#define VBUS_DRAW_BUF_LEN 10
#define MAX_OVERRIDE_VBUS_ALLOWED 900	/* 900 mA */
static char vbus_draw_mA[VBUS_DRAW_BUF_LEN];
module_param_string(vbus_draw_mA, vbus_draw_mA, VBUS_DRAW_BUF_LEN,
			S_IRUGO | S_IWUSR);

static int ci13xxx_vbus_draw(struct usb_gadget *_gadget, unsigned mA)
{
	struct ci13xxx *ci = container_of(_gadget, struct ci13xxx, gadget);
	unsigned int override_mA = 0;

	/* override param to draw more current if battery draining faster */
	if ((mA == CONFIG_USB_GADGET_VBUS_DRAW) &&
		(vbus_draw_mA[0] != '\0')) {
		if ((!kstrtoint(vbus_draw_mA, 10, &override_mA)) &&
				(override_mA <= MAX_OVERRIDE_VBUS_ALLOWED)) {
			mA = override_mA;
		}
	}

	if (ci->transceiver)
		return usb_phy_set_power(ci->transceiver, mA);
	return -ENOTSUPP;
}

static int ci13xxx_pullup(struct usb_gadget *_gadget, int is_active)
{
	struct ci13xxx *ci = container_of(_gadget, struct ci13xxx, gadget);
	unsigned long flags;

	spin_lock_irqsave(&ci->lock, flags);
	ci->softconnect = is_active;
	if (((ci->udc_driver->flags & CI13XXX_PULLUP_ON_VBUS) &&
			!ci->vbus_active) || !ci->driver) {
		spin_unlock_irqrestore(&ci->lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&ci->lock, flags);

	pm_runtime_get_sync(&_gadget->dev);

#ifdef USE_BAM2BAM
	/* Enable BAM (if needed) before starting controller */
	if (is_active) {
		dbg_event(0xFF, "BAM EN1", _gadget->bam2bam_func_enabled);
		msm_usb_bam_enable(CI_CTRL, _gadget->bam2bam_func_enabled);
	}
#endif
	spin_lock_irqsave(&ci->lock, flags);
	if (!ci->vbus_active) {
		spin_unlock_irqrestore(&ci->lock, flags);
		pm_runtime_put_sync(&_gadget->dev);
		return 0;
	}
	if (is_active) {
		spin_unlock(&ci->lock);
		if (ci->udc_driver->notify_event)
			ci->udc_driver->notify_event(ci,
				CI13XXX_CONTROLLER_CONNECT_EVENT);
		spin_lock(&ci->lock);
		hw_device_state(ci, ci->ep0out->qh.dma);
	} else {
		hw_device_state(ci, 0);
	}
	spin_unlock_irqrestore(&ci->lock, flags);

	pm_runtime_mark_last_busy(&_gadget->dev);
	pm_runtime_put_autosuspend(&_gadget->dev);

	return 0;
}

static int ci13xxx_selfpowered(struct usb_gadget *_gadget, int is_on)
{
	struct ci13xxx *ci = container_of(_gadget, struct ci13xxx, gadget);
	struct ci13xxx_ep *hwep = ci->ep0in;
	unsigned long flags;

	spin_lock_irqsave(hwep->lock, flags);
	_gadget->is_selfpowered = (is_on != 0);
	spin_unlock_irqrestore(hwep->lock, flags);

	return 0;
}

static int ci13xxx_start(struct usb_gadget *gadget,
			 struct usb_gadget_driver *driver);
static int ci13xxx_stop(struct usb_gadget *gadget);

/**
 * Device operations part of the API to the USB controller hardware,
 * which don't involve endpoints (or i/o)
 * Check  "usb_gadget.h" for details
 */
static const struct usb_gadget_ops usb_gadget_ops = {
	.vbus_session	= ci13xxx_vbus_session,
	.wakeup		= ci13xxx_wakeup,
	.set_selfpowered= ci13xxx_selfpowered,
	.vbus_draw	= ci13xxx_vbus_draw,
	.pullup		= ci13xxx_pullup,
	.udc_start	= ci13xxx_start,
	.udc_stop	= ci13xxx_stop,
};

/**
 * ci13xxx_start: register a gadget driver
 * @gadget: our gadget
 * @driver: the driver being registered
 *
 * Interrupts are enabled here.
 */
static int ci13xxx_start(struct usb_gadget *gadget,
			 struct usb_gadget_driver *driver)
{
	struct ci13xxx *ci = _udc;
	unsigned long flags = 0;
	int retval = -ENOMEM;

	trace("%pK", driver);

	if (driver->disconnect == NULL)
		return -EINVAL;

	//spin_lock_irqsave(&ci->lock, flags);

	info("hw_ep_max = %d", ci->hw_ep_max);

	//ci->gadget.dev.driver = NULL;

	//spin_unlock_irqrestore(&ci->lock, flags);

	pm_runtime_get_sync(&ci->gadget.dev);

	ci->ep0out->ep.desc = &ctrl_endpt_out_desc;
	retval = usb_ep_enable(&ci->ep0out->ep);
	if (retval)
		goto pm_put;

	ci->ep0in->ep.desc = &ctrl_endpt_in_desc;
	retval = usb_ep_enable(&ci->ep0in->ep);
	if (retval)
		goto pm_put;

	//spin_lock_irqsave(&ci->lock, flags);

	ci->gadget.ep0 = &ci->ep0in->ep;
	/* bind gadget */
	//driver->driver.bus     = NULL;
	//ci->gadget.dev.driver = &driver->driver;

	ci->driver = driver;
	if (ci->udc_driver->flags & CI13XXX_PULLUP_ON_VBUS) {
		if (ci->vbus_active) {
			if (ci->udc_driver->flags & CI13XXX_REGS_SHARED) {
				spin_lock_irqsave(&ci->lock, flags);
				hw_device_reset(ci);
			}
		} else {
			goto done;
		}
	}

	if (!ci->softconnect)
		goto done;

	retval = hw_device_state(ci, ci->ep0out->qh.dma);

done:
	if (spin_is_locked(&ci->lock))
		spin_unlock_irqrestore(&ci->lock, flags);

	if (ci->udc_driver->notify_event)
			ci->udc_driver->notify_event(ci,
				CI13XXX_CONTROLLER_UDC_STARTED_EVENT);
pm_put:
	pm_runtime_put_sync(&ci->gadget.dev);

	return retval;
}

/**
 * ci13xxx_stop: unregister a gadget driver
 *
 * Check usb_gadget_unregister_driver() at "usb_gadget.h" for details
 */
static int ci13xxx_stop(struct usb_gadget *gadget)
{
	struct ci13xxx *ci = _udc;
	//struct usb_gadget_driver *driver = ci->driver;
	unsigned long flags;

	trace("%pK", driver);

	spin_lock_irqsave(&ci->lock, flags);

	if (!(ci->udc_driver->flags & CI13XXX_PULLUP_ON_VBUS) ||
			ci->vbus_active) {
		hw_device_state(ci, 0);
		spin_unlock_irqrestore(&ci->lock, flags);
		_gadget_stop_activity(&ci->gadget);
		spin_lock_irqsave(&ci->lock, flags);
	}

	spin_unlock_irqrestore(&ci->lock, flags);

	usb_ep_free_request(&ci->ep0in->ep, ci->status);
	kfree(ci->status_buf);

	return 0;
}

/******************************************************************************
 * BUS block
 *****************************************************************************/
/**
 * udc_irq: global interrupt handler
 *
 * This function returns IRQ_HANDLED if the IRQ has been handled
 * It locks access to registers
 */
static irqreturn_t udc_irq(void)
{
	struct ci13xxx *ci = _udc;
	irqreturn_t retval;
	u32 intr;

	trace();

	if (ci == NULL)
		return IRQ_HANDLED;

	//if (!spin_is_locked(&ci->lock))
	spin_lock(&ci->lock);

	if (ci->udc_driver->in_lpm && ci->udc_driver->in_lpm(ci)) {
		spin_unlock(&ci->lock);
		return IRQ_NONE;
	}

	if (ci->udc_driver->flags & CI13XXX_REGS_SHARED) {
		if (hw_read(ci, OP_USBMODE, USBMODE_CM) !=
				USBMODE_CM_DEVICE) {
			spin_unlock(&ci->lock);
			return IRQ_NONE;
		}
	}
	intr = hw_test_and_clear_intr_active(ci);
	if (intr) {
		/* order defines priority - do NOT change it */
		if (USBi_URI & intr) {
			if (!hw_read(ci, OP_PORTSC, PORTSC_PR))
				pr_info("%s: USB reset interrupt is delayed\n",
								__func__);
			isr_reset_handler(ci);
		}
		if (USBi_PCI & intr) {
			isr_resume_handler(ci);
		}
		if (USBi_UI  & intr) {
			isr_tr_complete_handler(ci);
		}
		if (USBi_SLI & intr) {
			if (ci->gadget.speed != USB_SPEED_UNKNOWN &&
			    ci->driver->suspend && ci->vbus_active) {
				ci->suspended = 1;
				spin_unlock(&ci->lock);
				ci->driver->suspend(&ci->gadget);
				if (ci->udc_driver->notify_event)
					ci->udc_driver->notify_event(ci,
					CI13XXX_CONTROLLER_SUSPEND_EVENT);
				if (ci->transceiver)
					usb_phy_set_suspend(
						ci->transceiver, 1);
				usb_gadget_set_state(&ci->gadget,
						USB_STATE_SUSPENDED);
				spin_lock(&ci->lock);
			}
		}
		retval = IRQ_HANDLED;
	} else {
		retval = IRQ_NONE;
	}
	spin_unlock(&ci->lock);

	return retval;
}

static void destroy_eps(struct ci13xxx *ci)
{
	int i;

	for (i = 0; i < ci->hw_ep_max; i++) {
		struct ci13xxx_ep *mEp = &ci->ci13xxx_ep[i];

		if (mEp->pending_td)
			free_pending_td(mEp);

		dma_pool_free(ci->qh_pool, mEp->qh.ptr, mEp->qh.dma);
	}
}

static int init_eps(struct ci13xxx *ci)
{
	int retval = 0, i, j;

	for (i = 0; i < ci->hw_ep_max/2; i++) {
		for (j = RX; j <= TX; j++) {
			int k = i + j * ci->hw_ep_max/2;
			struct ci13xxx_ep *mEp = &ci->ci13xxx_ep[k];

			scnprintf(mEp->name, sizeof(mEp->name), "ep%i%s", i,
					(j == TX)  ? "in" : "out");

			mEp->udc          = ci;
			mEp->lock         = &ci->lock;
			mEp->td_pool      = ci->td_pool;

			mEp->ep.name      = mEp->name;
			mEp->ep.ops       = &usb_ep_ops;

			if (i == 0) {
				mEp->ep.caps.type_control = true;
			} else {
				mEp->ep.caps.type_iso = true;
				mEp->ep.caps.type_bulk = true;
				mEp->ep.caps.type_int = true;
			}

			if (j == TX)
				mEp->ep.caps.dir_in = true;
			else
				mEp->ep.caps.dir_out = true;

			/*
			 * for ep0: maxP defined in desc, for other
			 * eps, maxP is set by epautoconfig() called
			 * by gadget layer
			 */
			usb_ep_set_maxpacket_limit(&mEp->ep, (unsigned short)~0);

			INIT_LIST_HEAD(&mEp->qh.queue);
			mEp->qh.ptr = dma_pool_alloc(ci->qh_pool, GFP_KERNEL,
					&mEp->qh.dma);
			if (mEp->qh.ptr == NULL)
				retval = -ENOMEM;
			else
				memset(mEp->qh.ptr, 0, sizeof(*mEp->qh.ptr));

			/*
			 * set up shorthands for ep0 out and in endpoints,
			 * don't add to gadget's ep_list
			 */
			if (i == 0) {
				if (j == RX)
					ci->ep0out = mEp;
				else
					ci->ep0in = mEp;
				usb_ep_set_maxpacket_limit(&mEp->ep, CTRL_PAYLOAD_MAX);
				continue;
			}

			list_add_tail(&mEp->ep.ep_list, &ci->gadget.ep_list);
		}
	}

	return retval;
}

/**
 * udc_probe: parent probe must call this to initialize UDC
 * @dev:  parent device
 * @regs: registers base address
 * @name: driver name
 *
 * This function returns an error code
 * No interrupts active, the IRQ has not been requested yet
 * Kernel assumes 32-bit DMA operations by default, no need to dma_set_mask
 */
static int udc_probe(struct ci13xxx_udc_driver *driver, struct device *dev,
		void __iomem *regs, uintptr_t capoffset)
{
	struct ci13xxx *ci;
	int retval = 0, i;

	trace("%pK, %pK, %pK", dev, regs, driver->name);

	if (dev == NULL || regs == NULL || driver == NULL ||
			driver->name == NULL)
		return -EINVAL;

	ci = kzalloc(sizeof(struct ci13xxx), GFP_KERNEL);
	if (ci == NULL)
		return -ENOMEM;

	spin_lock_init(&ci->lock);
	ci->regs = regs;
	ci->udc_driver = driver;

	ci->gadget.ops          = &usb_gadget_ops;
	ci->gadget.speed        = USB_SPEED_UNKNOWN;
	ci->gadget.max_speed    = USB_SPEED_HIGH;
	ci->gadget.is_otg       = 0;
	ci->gadget.name         = driver->name;

	/* alloc resources */
	ci->qh_pool = dma_pool_create("ci13xxx_qh", dev,
				       sizeof(struct ci13xxx_qh),
				       64, CI13XXX_PAGE_SIZE);
	if (ci->qh_pool == NULL) {
		retval = -ENOMEM;
		goto free_udc;
	}

	ci->td_pool = dma_pool_create("ci13xxx_td", dev,
				       sizeof(struct ci13xxx_td),
				       64, CI13XXX_PAGE_SIZE);
	if (ci->td_pool == NULL) {
		retval = -ENOMEM;
		goto free_qh_pool;
	}

	INIT_DELAYED_WORK(&ci->rw_work, usb_do_remote_wakeup);

	retval = hw_device_init(ci, regs, capoffset);
	if (retval < 0)
		goto free_qh_pool;

	INIT_LIST_HEAD(&ci->gadget.ep_list);
	for (i = 0; i < ci->hw_ep_max; i++) {
		struct ci13xxx_ep *mEp = &ci->ci13xxx_ep[i];
		INIT_LIST_HEAD(&mEp->ep.ep_list);
		INIT_LIST_HEAD(&mEp->rw_queue);
	}

	retval = init_eps(ci);
	if (retval)
		goto free_dma_pools;

	ci->gadget.ep0 = &ci->ep0in->ep;

	if (ci->udc_driver->flags & CI13XXX_REQUIRE_TRANSCEIVER) {
		ci->transceiver = usb_get_phy(USB_PHY_TYPE_USB2);
		if (ci->transceiver == NULL) {
			retval = -ENODEV;
			goto destroy_eps;
		}
	}

	if (!(ci->udc_driver->flags & CI13XXX_REGS_SHARED)) {
		retval = hw_device_reset(ci);
		if (retval)
			goto put_transceiver;
	}

	if (ci->transceiver) {
		retval = otg_set_peripheral(ci->transceiver->otg,
						&ci->gadget);
		if (retval)
			goto put_transceiver;
	}

	retval = usb_add_gadget_udc(dev, &ci->gadget);
	if (retval)
		goto remove_trans;

#ifdef CONFIG_USB_GADGET_DEBUG_FILES
	retval = dbg_create_files(&ci->gadget.dev);
	if (retval) {
		pr_err("Registering sysfs files for debug failed!!!!\n");
		goto del_udc;
	}
#endif

	pm_runtime_no_callbacks(&ci->gadget.dev);
	pm_runtime_set_active(&ci->gadget.dev);
	pm_runtime_enable(&ci->gadget.dev);

	/* Use delayed LPM especially for composition-switch in LPM (suspend) */
	pm_runtime_set_autosuspend_delay(&ci->gadget.dev, 2000);
	pm_runtime_use_autosuspend(&ci->gadget.dev);

	_udc = ci;
	return retval;

del_udc:
	usb_del_gadget_udc(&ci->gadget);
remove_trans:
	if (ci->transceiver)
		otg_set_peripheral(ci->transceiver->otg, NULL);

	err("error = %i", retval);
put_transceiver:
	if (ci->transceiver)
		usb_put_phy(ci->transceiver);
destroy_eps:
	destroy_eps(ci);
free_dma_pools:
	dma_pool_destroy(ci->td_pool);
free_qh_pool:
	dma_pool_destroy(ci->qh_pool);
free_udc:
	kfree(ci);
	_udc = NULL;
	return retval;
}

/**
 * udc_remove: parent remove must call this to remove UDC
 *
 * No interrupts active, the IRQ has been released
 */
static void udc_remove(void)
{
	struct ci13xxx *ci = _udc;

	if (ci == NULL) {
		err("EINVAL");
		return;
	}

	usb_del_gadget_udc(&ci->gadget);

	if (ci->transceiver) {
		otg_set_peripheral(ci->transceiver->otg, &ci->gadget);
		usb_put_phy(ci->transceiver);
	}
#ifdef CONFIG_USB_GADGET_DEBUG_FILES
	dbg_remove_files(&ci->gadget.dev);
#endif
	destroy_eps(ci);
	dma_pool_destroy(ci->td_pool);
	dma_pool_destroy(ci->qh_pool);

	kfree(ci->hw_bank.regmap);
	kfree(ci);
	_udc = NULL;
}

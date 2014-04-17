/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/

#ifndef __PV_H__
#define __PV_H__

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#if defined(CONFIG_HAVE_CLK) && !defined(CONFIG_MACH_BCM_FPGA)
#define PV_HAS_CLK
#endif

struct pv_init_t {
	u8 id; /* 0:PV0, 1:PV1, 2:PV2 */
	u32 irq;  /*Interrupt ID*/
	u32 base_addr;
#ifdef PV_HAS_CLK
	char *apb_clk_name;
	char *pix_clk_name;
#endif
	void (*err_cb)(int err); /* Error Callback- run in ISR context */
	void (*eof_cb)(void); /* End-Of-Frame Callback- run in ISR context */
};

struct pv_config_t {
	u8 pclk_sel; /* DISP_CTRL type*/
	u8 pix_fmt; /* PIX_FMT type */
	bool cmd; /* 1: Command Mode, 0:Video Mode */
	bool cont; /* 1:Continuous Mode, 0:Single Shot Mode*/
	bool interlaced; /* 1:Interlaced Mode, 0:Progressive Mode*/
	u32 vsyncd; /* Vsync delay*/
	u8 pix_stretch; /* No. of times to repeat the same pixel*/
	u16 vs; /* Vertical Sync width */
	u16 vbp; /* Vertical Back Porch width */
	u16 vact; /* Vertical ACTive width */
	u16 vfp; /* Vertical Front Porch wdith*/
	u16 hs; /* Horizontal Sync width */
	u16 hbp; /* Horizontal Back Porch width */
	u16 hact; /* Horizontal ACTive width */
	u16 hfp; /* Horizontal Front Porch wdith*/
	u16 hbllp; /* Horizontal BLLP wdith*/
};

enum {
	DISP_CTRL_DSI,
	DISP_CTRL_SMI,
	PCLK_SEL_TYPE_MAX
};

enum {
	HDMI_VEC_SMI_24BPP,
	DSI_VIDEO_16BPP,
	DSI_CMD_16BPP,
	PACKED_DSI_VIDEO_18BPP,
	DSI_VIDEO_CMD_18_24BPP,
	PIX_FMT_TYPE_MAX
};

enum {
	PV_INIT_DONE,
	PV_STOPPED,
	PV_CONFIGURED,
	PV_ENABLED,
	PV_STOPPING,
	PV_MAX_STATE
};

enum {
	PV_RESET,
	PV_VID_CONFIG,
	PV_START,
	PV_RESUME_STREAM,
	PV_PAUSE_STREAM_SYNC,
	PV_STOP_EOF_ASYNC,
	PV_STOP_IMM,
	PV_MAX_EVENT
};

/*
 * Purpose: Intialise software for a particular PV block
 */
int pv_init(struct pv_init_t *init, struct pv_config_t **config);

/*
 * Purpose: Switch PV to one of the possible states
 */
int pv_change_state(int event, struct pv_config_t *config);

/*
 * Purpose: To get the state of PixelValve
 */
int pv_get_state(struct pv_config_t *config);

int check_pv_state(int event, struct pv_config_t *config);

extern int g_display_enabled;

#endif /* __PV_H__ */

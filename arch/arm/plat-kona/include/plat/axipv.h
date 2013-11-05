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

#ifndef __AXIPV_H__
#define __AXIPV_H__

#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/sysrq.h>
#include <mach/memory.h>
#include <asm/io.h>
#include <linux/time.h>
#include <mach/rdb/brcm_rdb_sysmap.h>

typedef u32 axipv_async_buf_t;

#define CM_PKT_SIZE_B	768
#define PV_START_THRESH_INT (1<<7)
#define AXIPV_DISABLED_INT (1<<6)
#define WATER_LVL2_INT	(1<<1)
#define TE_INT	(1<<0)

/* Once we change the Userspace Framebuffer.cpp to request for buffers
 * from display drivers, we can change this accordingly*/
#define AXIPV_MAX_DISP_BUFF_SUPP 2

#if defined(CONFIG_HAVE_CLK) && !defined(CONFIG_MACH_BCM_FPGA)
#define AXIPV_HAS_CLK
#endif


struct axipv_sync_buf_t {
	u32 addr;
	u32 xlen;
	u32 ylen;
};

struct axipv_init_t {
	u32 irq;
	u32 base_addr;
#ifdef AXIPV_HAS_CLK
	char *clk_name;
#endif
	void (*irq_cb)(int err);
	void (*release_cb)(u32 free_buf);
	void (*vsync_cb)(void);
};

struct axipv_config_t {
	union {
		axipv_async_buf_t async;
		struct axipv_sync_buf_t sync;
	} buff;
	u32 width;
	u32 height;
	bool cmd;
	bool test;
	bool async;
	bool bypassPV;
	u8 pix_fmt;
};

enum {
	AXIPV_PIXEL_FORMAT_24BPP_RGB,
	AXIPV_PIXEL_FORMAT_24BPP_BGR,
	AXIPV_PIXEL_FORMAT_16BPP_PACKED,
	AXIPV_PIXEL_FORMAT_16BPP_UNPACKED,
	AXIPV_PIXEL_FORMAT_18BPP_PACKED,
	AXIPV_PIXEL_FORMAT_18BPP_UNPACKED
};

enum {
	AXIPV_INIT_DONE,
	AXIPV_STOPPED,
	AXIPV_CONFIGURED,
	AXIPV_ENABLED,
	AXIPV_STOPPING,
	AXIPV_MAX_STATE,
	AXIPV_INVALID_STATE
};

enum {
	AXIPV_RESET,
	AXIPV_CONFIG,
	AXIPV_START,
	AXIPV_STOP_EOF,
	AXIPV_STOP_IMM,
	AXIPV_WAIT_INTR,
	AXIPV_MAX_EVENT
};

enum {
	AXIPV_SYNC,
	AXIPC_ASYNC,
};

int axipv_init(struct axipv_init_t *init, struct axipv_config_t **config);

int axipv_change_state(u32 event, struct axipv_config_t *config);

int axipv_set_mode(struct axipv_config_t *config, u32 mode);

int axipv_get_state(struct axipv_config_t *config);

int axipv_post(struct axipv_config_t *config);

void axipv_release_pixdfifo_ownership(struct axipv_config_t *config);

int axipv_check_completion(u32 event, struct axipv_config_t *config);

/***************************** DEBUG API **************************************/
/* These APIs are to be used to enable/disable the clock while debugging ONLY */
void enable_axipv_clk_debug(struct axipv_config_t *config);
void disable_axipv_clk_debug(struct axipv_config_t *config);
/***************************** DEBUG API **************************************/

extern int g_display_enabled;

#endif /* __AXIPV_H__ */

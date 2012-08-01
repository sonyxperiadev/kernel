#ifndef __AXIPV_H__
#define __AXIPV_H__

#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/io.h>

typedef u32 axipv_async_buf_t;

#define PV_START_THRESH_INT (1<<7)

/* Once we change the Userspace Framebuffer.cpp to request for buffers
 * from display drivers, we can change this accordingly*/
#define AXIPV_MAX_DISP_BUFF_SUPP 2

#if defined(CONFIG_HAVE_CLK) && !defined(CONFIG_MACH_HAWAII_FPGA)
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
	u8 pix_fmt;
};

enum {
	AXIPV_PIXEL_FORMAT_24BPP_RGB,
	AXIPV_PIXEL_FORMAT_24BPP_BGR,
	AXIPV_PIXEL_FORMAT_16BPP_PACKED,
	AXIPV_PIXEL_FORMAT_16BPP_UNPACKED,
	AXIPV_PIXEL_FORMAT_18BPP_PACKED,
	AXIPVPIXEL_FORMAT_18BPP_UNPACKED
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

#endif /* __AXIPV_H__ */

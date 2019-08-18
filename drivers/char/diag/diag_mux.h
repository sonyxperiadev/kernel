/* Copyright (c) 2014-2017, 2019 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef DIAG_MUX_H
#define DIAG_MUX_H
#define DIAG_USB_MODE			0
#define DIAG_MEMORY_DEVICE_MODE		1
#define DIAG_NO_LOGGING_MODE		2
#define DIAG_MULTI_MODE			3

#define DIAG_MUX_LOCAL		0
#define DIAG_MUX_LOCAL_LAST	1
#define DIAG_MUX_BRIDGE_BASE	DIAG_MUX_LOCAL_LAST
#define DIAG_MUX_MDM		(DIAG_MUX_BRIDGE_BASE)
#define DIAG_MUX_MDM2		(DIAG_MUX_BRIDGE_BASE + 1)
#define DIAG_MUX_SMUX		(DIAG_MUX_BRIDGE_BASE + 2)
#define DIAG_MUX_BRIDGE_LAST	(DIAG_MUX_BRIDGE_BASE + 3)

#ifndef CONFIG_DIAGFWD_BRIDGE_CODE
#define NUM_MUX_PROC		DIAG_MUX_LOCAL_LAST
#else
#define NUM_MUX_PROC		DIAG_MUX_BRIDGE_LAST
#endif

struct diag_mux_state_t {
	struct diag_logger_t *logger[NUM_MUX_PROC];
	struct diag_logger_t *usb_ptr;
	struct diag_logger_t *md_ptr;
	unsigned int mux_mask[NUM_MUX_PROC];
	unsigned int mode[NUM_MUX_PROC];
};

struct diag_mux_ops {
	int (*open)(int id, int mode);
	int (*close)(int id, int mode);
	int (*read_done)(unsigned char *buf, int len, int id);
	int (*write_done)(unsigned char *buf, int len, int buf_ctx,
			      int id);
};

struct diag_logger_ops {
	void (*open)(void);
	void (*close)(void);
	void (*open_device)(int id);
	void (*close_device)(int id);
	int (*queue_read)(int id);
	int (*write)(int id, unsigned char *buf, int len, int ctx);
	int (*close_peripheral)(int id, uint8_t peripheral);
};

struct diag_logger_t {
	int mode;
	struct diag_mux_ops *ops[NUM_MUX_PROC];
	struct diag_logger_ops *log_ops;
};

extern struct diag_mux_state_t *diag_mux;

int diag_mux_init(void);
void diag_mux_exit(void);
int diag_mux_register(int proc, int ctx, struct diag_mux_ops *ops);
int diag_mux_queue_read(int proc);
int diag_mux_write(int proc, unsigned char *buf, int len, int ctx);
int diag_mux_close_peripheral(int proc, uint8_t peripheral);
int diag_mux_open_all(struct diag_logger_t *logger);
int diag_mux_close_all(void);
int diag_mux_switch_logging(int proc, int *new_mode, int *peripheral_mask);
#endif

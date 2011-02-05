/*
 *  linux/include/linux/brcm_console.h
 *
 *  Copyright (C) 1993        Hamish Macdonald
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Changed:
 * 10-Mar-94: Arno Griffioen: Conversion for vt100 emulator port from PC LINUX
 */

/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use
of this software, this software is licensed to you under the terms of the GNU General Public
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software in any way
with any other Broadcom software provided under a license other than the GPL, without
Broadcom's express prior written consent.
*******************************************************************************************/

#ifndef _LINUX_CONSOLE_H_
#define _LINUX_CONSOLE_H_ 1

#include <linux/types.h>

enum {
	USB_RNDIS_OFF,
	USB_RNDIS_ON,
};

struct brcm_netconsole_callbacks 
{
	/** Start function for role change */
	int (*start) (void);
	/** Stop Function for role change */
	int (*stop) (void);	
};

struct vc_data;
struct console_font_op;
struct console_font; 
struct module;
struct tty_struct;

/*
 * this is what the terminal answers to a ESC-Z or csi0c query.
 */
#define VT100ID "\033[?1;2c"
#define VT102ID "\033[?6c"

struct brcm_consw {
	struct module *owner;
	const char *(*con_startup)(void);
	void	(*con_init)(struct vc_data *, int);
	void	(*con_deinit)(struct vc_data *);
	void	(*con_clear)(struct vc_data *, int, int, int, int);
	void	(*con_putc)(struct vc_data *, int, int, int);
	void	(*con_putcs)(struct vc_data *, const unsigned short *, int, int, int);
	void	(*con_cursor)(struct vc_data *, int);
	int	(*con_scroll)(struct vc_data *, int, int, int, int);
	void	(*con_bmove)(struct vc_data *, int, int, int, int, int, int);
	int	(*con_switch)(struct vc_data *);
	int	(*con_blank)(struct vc_data *, int, int);
	int	(*con_font_set)(struct vc_data *, struct console_font *, unsigned);
	int	(*con_font_get)(struct vc_data *, struct console_font *);
	int	(*con_font_default)(struct vc_data *, struct console_font *, char *);
	int	(*con_font_copy)(struct vc_data *, int);
	int     (*con_resize)(struct vc_data *, unsigned int, unsigned int,
			       unsigned int);
	int	(*con_set_palette)(struct vc_data *, unsigned char *);
	int	(*con_scrolldelta)(struct vc_data *, int);
	int	(*con_set_origin)(struct vc_data *);
	void	(*con_save_screen)(struct vc_data *);
	u8	(*con_build_attr)(struct vc_data *, u8, u8, u8, u8, u8, u8);
	void	(*con_invert_region)(struct vc_data *, u16 *, int);
	u16    *(*con_screen_pos)(struct vc_data *, int);
	unsigned long (*con_getxy)(struct vc_data *, unsigned long, int *, int *);
};

extern const struct brcm_consw *brcm_conswitchp;

extern const struct brcm_consw dummy_con;	/* dummy brcm_console buffer */
extern const struct brcm_consw vga_con;	/* VGA text brcm_console */
extern const struct brcm_consw newport_con;	/* SGI Newport brcm_console  */
extern const struct brcm_consw prom_con;	/* SPARC PROM brcm_console */

/* int con_is_bound(const struct brcm_consw *csw); */
/* int register_con_driver(const struct brcm_consw *csw, int first, int last); */
/* int unregister_con_driver(const struct brcm_consw *csw); */
int take_over_brcm_console(const struct brcm_consw *sw, int first, int last, int deflt);
void give_up_brcm_console(const struct brcm_consw *sw);
/* scroll */
/* #define SM_UP       (1) */
/* #define SM_DOWN     (2) */

/* cursor */
/* #define CM_DRAW     (1) */
/* #define CM_ERASE    (2) */
/* #define CM_MOVE     (3) */

/*
 * The interface for a brcm_console, or any other device that wants to capture
 * brcm_console messages (printer driver?)
 *
 * If a brcm_console driver is marked CON_BOOT then it will be auto-unregistered
 * when the first real brcm_console is registered.  This is for early-printk drivers.
 */

#define CON_PRINTBUFFER	(1)
#define CON_CONSDEV	(2) /* Last on the command line */
#define CON_ENABLED	(4)
#define CON_BOOT	(8)
#define CON_ANYTIME	(16) /* Safe to call when cpu is offline */
#define CON_BRL		(32) /* Used for a braille device */

struct brcm_console {
	char	name[16];
	void	(*write)(struct brcm_console *, const char *, unsigned);
	int	(*read)(struct brcm_console *, char *, unsigned);
	struct tty_driver *(*device)(struct brcm_console *, int *);
	void	(*unblank)(void);
	int	(*setup)(struct brcm_console *, char *);
	int	(*early_setup)(void);
	short	flags;
	short	index;
	int	cflag;
	void	*data;
	struct	 brcm_console *next;
};

extern int brcm_console_set_on_cmdline;

extern int add_preferred_brcm_console(char *name, int idx, char *options);
extern int update_brcm_console_cmdline(char *name, int idx, char *name_new, int idx_new, char *options);
extern void register_brcm_console(struct brcm_console *);
extern int unregister_brcm_console(struct brcm_console *);
extern struct brcm_console *brcm_console_drivers;
extern void acquire_brcm_console_sem(void);
extern int try_acquire_brcm_console_sem(void);
extern void release_brcm_console_sem(void);
extern void brcm_console_conditional_schedule(void);
extern void brcm_console_unblank(void);
extern struct tty_driver *brcm_console_device(int *);
extern void brcm_console_stop(struct brcm_console *);
extern void brcm_console_start(struct brcm_console *);
extern int is_brcm_console_locked(void);
extern int braille_register_brcm_console(struct brcm_console *, int index,
		char *brcm_console_options, char *braille_options);
extern int braille_unregister_brcm_console(struct brcm_console *);

extern int brcm_console_suspend_enabled;

/* Suspend and resume brcm_console messages over PM events */
extern void suspend_brcm_console(void);
extern void resume_brcm_console(void);

int mda_brcm_console_init(void);
void prom_con_init(void);

void vcs_make_sysfs(struct tty_struct *tty);
void vcs_remove_sysfs(struct tty_struct *tty);

/* Some debug stub to catch some of the obvious races in the VT code */
#if 1
#define WARN_CONSOLE_UNLOCKED()	WARN_ON(!is_brcm_console_locked() && !oops_in_progress)
#else
#define WARN_CONSOLE_UNLOCKED()
#endif

/* VESA Blanking Levels */
#define VESA_NO_BLANKING        0
#define VESA_VSYNC_SUSPEND      1
#define VESA_HSYNC_SUSPEND      2
#define VESA_POWERDOWN          3

#endif /* _LINUX_CONSOLE_H */

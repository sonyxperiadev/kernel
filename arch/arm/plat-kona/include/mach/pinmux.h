/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#ifndef __PINMUX_H_
#define __PINMUX_H_

#include <asm/io.h>
#include <mach/chip_pinmux.h>

/* Pull up/down*/
#define	PULL_UP_ON		1
#define	PULL_UP_OFF		0

#define	PULL_DN_ON		1
#define	PULL_DN_OFF		0

/*drive strength */
#define	DRIVE_STRENGTH_2MA	0
#define	DRIVE_STRENGTH_4MA	1
#define	DRIVE_STRENGTH_6MA	2
#define	DRIVE_STRENGTH_8MA	3
#define	DRIVE_STRENGTH_10MA	4
#define	DRIVE_STRENGTH_12MA	5
#define	DRIVE_STRENGTH_14MA	6
#define	DRIVE_STRENGTH_16MA	7

/* declare configuration for a single pin */
#define	PIN_CFG(ball, f, hys, dn, up, rc, ipd, drv) 			\
	{								\
		.name		=	PN_##ball,			\
		.func		=	PF_##f,				\
		.reg.b		=	{				\
			.hys_en		=	hys,			\
			.pull_dn	=	PULL_DN_##dn,		\
			.pull_up	=	PULL_UP_##up,		\
			.slew_rate_ctrl	=	rc,			\
			.input_dis	=	ipd,			\
			.drv_sth	=	DRIVE_STRENGTH_##drv,	\
		},							\
	}

/* BSC pad registers are different */
#define PIN_BSC_CFG(ball, f, v)						\
	{								\
		.name		=	PN_##ball,			\
		.func		=	PF_##f, 			\
		.reg.val	=	v,				\
	}

/* pin description */
struct pin_desc {
	enum PIN_NAME name;
	unsigned int reg_offset;
	enum PIN_FUNC f_tbl[MAX_ALT_FUNC];
};

/* chip-level pin description */
struct chip_pin_desc {
	const struct pin_desc *desc_tbl;
	void __iomem *base;	/* pad control registers virtual base */
};

/* chip pin-mux register */
union pinmux_reg {
	unsigned int val;
	struct {
		unsigned drv_sth:3;
		unsigned input_dis:1;
		unsigned slew_rate_ctrl:1;
		unsigned pull_up:1;
		unsigned pull_dn:1;
		unsigned hys_en:1;
		unsigned sel:3;
	} b;
};

/* board-level or use-case based configuration */
struct pin_config {
	enum PIN_NAME name;
	enum PIN_FUNC func;

	union pinmux_reg reg;
};

extern struct chip_pin_desc g_chip_pin_desc;

int pinmux_init(void);

/* chip level init */
int pinmux_chip_init(void);

/* board level init */
int pinmux_board_init(void);

/*
  get pin configuration at run time
  caller provides pin ball name
*/
int pinmux_get_pin_config(struct pin_config *config);

/*
  set pin configuration at run time
  caller fills pin_configuration, except sel, which will derived from func in this routine.
*/
int pinmux_set_pin_config(struct pin_config *config);

static inline int is_ball_valid(enum PIN_NAME name)
{
	return name < PN_MAX;
}

int pinmux_find_gpio(enum PIN_NAME name, unsigned *gpio,
		     enum PIN_FUNC *PF_gpio);

#ifdef CONFIG_KONA_ATAG_DT
uint32_t get_dts_pinmux_value(uint32_t index);
uint32_t get_dts_pinmux_nr(void);
#endif /* CONFIG_KONA_ATAG_DT */

#endif /*__PINMUX_H_ */

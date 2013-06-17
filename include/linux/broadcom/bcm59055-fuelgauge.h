/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/bcm59055-fuelgauge.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
* bcm59055-fuelgauge.h
*
* PURPOSE:
*
*
*
* NOTES:
*
* ****************************************************************************/

#ifndef __BCM59055_FG_H__
#define __BCM59055_FG_H__

enum {
	FAST_CALIBRATION = 0,
	LONG_CALIBRATION,
};

enum {
	CONTINUOUS_MODE,
	SYNCHRONOUS_MODE,
};

#define FGHOSTEN		(0x1)

#define FGOPMODCRTL0	(0x01)
#define FGOPMODCRTL1	(0x01 << 1)
#define FGOPMODCRTL2	(0x01 << 2)
#define FGOPMODCRTL3	(0x01 << 3)
#define FGSYNCMODE		(0x1 << 4)
#define FGMODON			(0x1 << 5)

#define FGTRIM			(0x1)
#define FGCAL			(0x1 << 1)
#define LONGCAL			(0x1 << 2)
#define FG1PTCAL		(0x1 << 3)
#define FGRESET			(0x1 << 4)
#define FGFRZREAD		(0x1 << 5)
#define FGFRZSMPL		(0x1 << 6)
#define FGFORCECAL		(0x1 << 7)

enum {
	FG_COMB_RATE_2HZ,
	FG_COMB_RATE_4HZ,
	FG_COMB_RATE_8HZ,
	FG_COMB_RATE_16HZ,
};

#define FG_COMB_RATE_MASK	0x3

#define FGRDVALID		(0x1 << 7)

enum fg_smpl_type {
	fg_smpl_acc,
	fg_smpl_cal,
	fg_smpl_raw
};

/* EXPORTED FUNCTIONS */
extern int bcm59055_fg_enable(void);
extern int bcm59055_fg_disable(void);
extern int bcm59055_fg_set_cont_mode(void);
extern int bcm59055_fg_set_sync_mode(bool modulator_on);
extern int bcm59055_fg_enable_modulator(bool enable);
extern int bcm59055_fg_offset_cal(bool longcal);
extern int bcm59055_fg_1point_cal(void);
extern int bcm59055_fg_force_cal(void);
extern int bcm59055_fg_set_comb_rate(int rate);
extern int bcm59055_fg_init_read(void);
extern int bcm59055_fg_read_soc(u32 *fg_accm, u16 *fg_cnt, u16 *fg_sleep_cnt);
extern int bcm59055_fg_reset(void);
extern int bcm59055_fg_read_sample(enum fg_smpl_type type, s16 * val);
extern int bcm59055_fg_set_fgfrzsmpl(void);
extern int bcm59055_fg_write_gain_trim(u8 gain);
extern int bcm59055_fg_write_offset_trim(u8 offset);
extern int bcm59055_fg_read_offset(s16 *offset);

#endif


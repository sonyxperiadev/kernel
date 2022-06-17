/* SPDX-License-Identifier: GPL-2.0 */
/*
 * CS40L20/CS40L25/CS40L25A/CS40L25B Wavetable Definitions
 *
 *
 * Copyright (C) 2020-2021 Cirrus Logic, Inc. and
 *                         Cirrus Logic International Semiconductor Ltd.
 */

#ifndef CS40L25_WAVETABLE_H
#define CS40L25_WAVETABLE_H

#include <linux/bitops.h>

#define WT_ENTRY_MAX		128
#define WT_ENTRY_SIZE_WORDS	3
#define WT_ENTRY_SIZE_BYTES	(WT_ENTRY_SIZE_WORDS * sizeof(u32))

#define WT_FLAG_CLICK		BIT(15)
#define WT_FLAG_CLAB		BIT(14)
#define WT_FLAG_4KHZ		BIT(13)
#define WT_FLAG_F0		BIT(12)
#define WT_FLAG_NB_CLAB		BIT(11)
#define WT_FLAG_META		BIT(10)

#define WT_FLAG_TERMINATOR	0xFFFF

enum wt_type {
	WT_TYPE_V4_PCM			= 0,
	WT_TYPE_V4_PWLE			= 1,
	WT_TYPE_V4_PCM_F0_REDC		= 2,
	WT_TYPE_V4_PCM_F0_REDC_VAR	= 3,
	WT_TYPE_V4_COMPOSITE		= 4,
	WT_TYPE_V5_PCM_F0_REDC_Q	= 5,
	WT_TYPE_V5_PWLE_LONG		= 6,
	WT_TYPE_V5_PWLE_LINEAR		= 7,
	WT_TYPE_V6_PCM_F0_REDC		= 8,
	WT_TYPE_V6_PCM_F0_REDC_VAR	= 9,
	WT_TYPE_V6_COMPOSITE		= 10,
	WT_TYPE_V6_PCM_F0_REDC_Q	= 11,
	WT_TYPE_V6_PWLE			= 12,

	WT_TYPE_TERMINATOR		= 0xFF,
};

struct wt_entry {
	enum wt_type type;
	u16 flags;
	u32 index;
	u32 size;

	void *data;
};

struct wt_wavetable {
	struct wt_entry waves[WT_ENTRY_MAX];
	int nwaves;

	/* Sizes in bytes including padding bytes */
	int bytes;
	int byteslimit;
};

static inline int wt_get_space(struct wt_wavetable *table)
{
	return table->byteslimit - table->bytes;
}

#define WT_WAVELEN_MAX		GENMASK(21, 0)
#define WT_WAVELEN_INDEFINITE	BIT(22)
#define WT_WAVELEN_CALCULATED	BIT(23)

#define WT_REPEAT_LOOP_MARKER	0xFF

#define WT_MAX_SECTIONS		255

#define WT_T10_FLAG_DURATION	BIT(8)

struct wt_type10_comp_section {
	u8 amplitude;
	u8 index;
	u8 repeat;
	u8 flags;
	u16 delay;
	u16 duration;
};

struct wt_type10_comp {
	u32 wlength;
	u8 nsections;
	u8 repeat;

	struct wt_type10_comp_section sections[WT_MAX_SECTIONS];
};

#define WT_T12_FLAG_CHIRP	BIT(7)
#define WT_T12_FLAG_BRAKE	BIT(6)
#define WT_T12_FLAG_AMP_REG	BIT(5)

struct wt_type12_pwle_section {
	u16 time;
	u16 level;
	u16 frequency;
	u8 flags;
	u32 vbtarget;
};

struct wt_type12_pwle {
	u32 wlength;
	u8 repeat;
	u16 wait;
	u8 nsections;

	struct wt_type12_pwle_section sections[WT_MAX_SECTIONS];
};

#endif

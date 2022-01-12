/* SPDX-License-Identifier: GPL-2.0
 *
 * linux/platform_data/cs40l2x.h -- Platform data for
 * CS40L20/CS40L25/CS40L25A/CS40L25B
 *
 * Copyright 2018 Cirrus Logic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __CS40L2X_H
#define __CS40L2X_H

struct cs40l2x_br_desc {
	bool present;
	bool enable;
	unsigned int thld1;
	unsigned int max_att;
	unsigned int atk_vol;
	unsigned int atk_rate;
	unsigned int wait;
	unsigned int rel_rate;
	bool mute_enable;
};

struct cs40l2x_platform_data {
	unsigned int boost_ind;
	unsigned int boost_cap;
	unsigned int boost_ipk;
	unsigned int boost_ctl;
	unsigned int boost_ovp;
	unsigned int boost_clab;
	bool refclk_gpio2;
	unsigned int f0_default;
	unsigned int f0_min;
	unsigned int f0_max;
	unsigned int redc_default;
	unsigned int redc_min;
	unsigned int redc_max;
	unsigned int q_default;
	unsigned int q_min;
	unsigned int q_max;
	bool redc_comp_disable;
	bool comp_disable;
	bool dyn_f0_disable;
	bool open_wt_disable;
	unsigned int gpio1_rise_index;
	unsigned int gpio1_fall_index;
	unsigned int gpio1_fall_timeout;
	unsigned int gpio1_mode;
	unsigned int gpio2_rise_index;
	unsigned int gpio2_fall_index;
	unsigned int gpio3_rise_index;
	unsigned int gpio3_fall_index;
	unsigned int gpio4_rise_index;
	unsigned int gpio4_fall_index;
	unsigned int gpio_indv_enable;
	unsigned int gpio_indv_pol;
	bool hiber_enable;
	unsigned int asp_bclk_freq;
	bool asp_bclk_inv;
	bool asp_fsync_inv;
	unsigned int asp_fmt;
	unsigned int asp_slot_num;
	unsigned int asp_slot_width;
	unsigned int asp_samp_width;
	unsigned int asp_timeout;
	bool vpbr_enable;
	bool vbbr_enable;
	unsigned int vpbr_thld1;
	unsigned int vbbr_thld1;
	struct cs40l2x_br_desc vpbr_config;
	struct cs40l2x_br_desc vbbr_config;
	unsigned int fw_id_remap;
	bool amp_gnd_stby;
	bool auto_recovery;
};

#endif /* __CS40L2X_H */

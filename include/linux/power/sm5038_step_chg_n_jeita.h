/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NOTE: This file has been modified by Sony Corporation.
 * Modifications are Copyright 2021 Sony Corporation,
 * and licensed under the license of the file.
 */

#ifndef __SM5038_STEP_CHG_N_JEITA_H__
#define __SM5038_STEP_CHG_N_JEITA_H__

#define STEP_CHG_HYSTERISIS_DELAY_US		5000000 /* 5 secs */

#define GET_CONFIG_JEITA_DELAY_MS		2000
#define GET_CONFIG_JEITA_RETRY_COUNT	50

#define MAX_STEP_CHG_ENTRIES		8	/* It support 8 ranges at max. */

#define MAX_JEITA_TEMPERATURE_HOT_TH		600

#define TEMPERATURE_HYS			10	/* 1 degree */


enum {
	INDEX_JIETA_TEMP_RANGE_5_TO_12_DEGREE = 0,
	INDEX_JIETA_TEMP_RANGE_12_TO_15_DEGREE,
	INDEX_JIETA_TEMP_RANGE_15_TO_20_DEGREE,
	INDEX_JIETA_TEMP_RANGE_20_TO_45_DEGREE,
	INDEX_JIETA_TEMP_RANGE_45_TO_55_DEGREE,
	INDEX_JIETA_TEMP_RANGE_55_MAX,	
};
 
struct chg_jeita_param {
	u32				psy_prop;
	char			*prop_name;
	int				hys;
};

struct range_data {
	int min_threshold;
	int max_threshold;
	u32 value;
};

struct step_chg_cfg {
	struct chg_jeita_param	param;
	struct range_data	fastchg_cfg[MAX_STEP_CHG_ENTRIES];
};

struct jeita_fcc_cfg {
	struct chg_jeita_param	param;
	struct range_data		fcc_cfg[MAX_STEP_CHG_ENTRIES];
};

struct jeita_fv_cfg {
	struct chg_jeita_param	param;
	struct range_data		fv_cfg[MAX_STEP_CHG_ENTRIES];
};

struct sm5038_chg_jeita_info {

	struct device		*dev;

	/* *********** STEP_CHG *********** */
	ktime_t			step_last_update_time;

	bool			step_chg_enable;
	bool 			step_chg_cfg_valid;

	bool 			step_chg_based_on_soc;
	bool 			step_chg_based_on_vbat_avg;
	bool 			step_chg_based_on_ocv;

	struct wakeup_source	*step_chg_ws;

	struct step_chg_cfg		*step_chg_config;

	int 			step_chg_index;

	/* *********** JEITA ************* */
	ktime_t			jeita_last_update_time;

	bool			sw_jeita_enable;
	bool			sw_jeita_cfg_valid;
	bool			jeita_config_read_ok;

	struct jeita_fcc_cfg	*jeita_fcc_config;
	struct jeita_fv_cfg		*jeita_fv_config;

	int				jeita_fcc_index;
	int				jeita_fv_index;

	/* *********** COMMON ***********  */
	int 			get_config_retry_count;

	struct power_supply	*sm5038_chg_psy;
	struct power_supply	*sm5038_typec_psy;	


	/* delay work */
	struct delayed_work	get_config_work;
	struct delayed_work	update_step_work;

	struct notifier_block	nb;

	unsigned int battery_temp;

	

};


static struct sm5038_chg_jeita_info *gJeita_info;


int sm5038_step_chg_n_jeita_init(struct device *dev, bool step_chg_enable, bool sw_jeita_enable);
void sm5038_step_chg_n_jeita_deinit(void);

#endif /* __SM5038_STEP_CHG_N_JEITA_H__ */

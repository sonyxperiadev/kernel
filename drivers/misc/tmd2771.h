#ifndef __TK_PSENSOR_TMD2771_H
#define __TK_PSENSOR_TMD2771_H

#define TAOS_DEVICE_ID		"tmd2771"
#define TAOS_ID_NAME_SIZE	10
#define TAOS_DEVICE_ADDR1	0x29
#define TAOS_DEVICE_ADDR2	0x39
#define TAOS_DEVICE_ADDR3	0x49

#define TAOS_INT_GPIO		89
#define ALS_PS_INT		gpio_to_irq(TAOS_INT_GPIO)

struct taos_cfg {
	u32 calibrate_target;
	u16 als_time;
	u16 scale_factor;
	u16 gain_trim;
	u8 filter_history;
	u8 filter_count;
	u8 gain;
	u16 prox_threshold_hi;
	u16 prox_threshold_lo;
	u16 als_threshold_hi;
	u16 als_threshold_lo;
	u8 prox_int_time;
	u8 prox_adc_time;
	u8 prox_wait_time;
	u8 prox_intr_filter;
	u8 prox_config;
	u8 prox_pulse_cnt;
	u8 prox_gain;
};

struct taos_prox_info {
	u16 prox_clear;
	u16 prox_data;
	int prox_event;
};

struct tmd2771_platform_data {
	struct taos_cfg *cfg_data;
};

#endif /* __TK_PSENSOR_TMD2771_H */

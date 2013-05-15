/*
 * as3649.h - platform data structure for as3649 led controller
 *
 * Copyright (C) 2012 Ulrich Herrmann <ulrich.herrmann@austriamicrosystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#ifndef __LINUX_AS3649_H
#define __LINUX_AS3649_H

#include <linux/device.h>

struct as3649_platform_data {
	void (*init)(struct device *dev, bool on); /* callback to set
					AS3649 enable pin high/low*/
	void (*enable)(struct device *dev, bool on); /* callback to get/
					release resources for AS3649*/
	int autosuspend_delay_ms; /* For pm_runtime_set_autosuspend_delay*/
	bool use_tx_mask; /* enable current reduction in flash mode using
			     TXMASK_TORCH pin */
	u16 I_limit_mA; /* AS3649: 2500, 2900, 3300, 3700 mA for the coil*/
	u16 txmasked_current_mA; /* 31,63,...471, 31.4 mA steps:
	current reduction in flash mode if TXMASK_TORCH=1 and use_tx_mask==1 */
	u16 vin_low_v_mV; /* 0=off, 3000, 3070, 3140, 3220, 3300, 3380,
				 3470 mV limit output current in flash mode
				 to above VBAT drop below vin_low_v_mV*/
	u8 strobe_type; /* 0=edge, 1=level */
	bool freq_switch_on; /* enable 1/4MHz switch; improve efficiency if
		1==dcdc_skip_enable by using 1MHz and 4MHz operation*/

	u16 ntc_current_uA; /* 0 .. 600uA, 40uA steps. */
	u8  ntc_on; /* Enable overtemperature protection, torch/flash is
		       turned off when (ntc_current_uA * R_ntc > 1Volt)*/
	bool dcdc_skip_enable; /* If set, the DCDC can use pulseskip
	resulting in good efficiency but increased noise. If false, the
	AS3649 will force PWM operation resulting in reduced efficiency but
	improved noise. If 0==dcdc_skip_enable set diag_pulse_force_dcdc_on=1
	and diag_pulse_min_on_increase=3 */

	/* LED configuration, two identical leds must be connected. */
	u16 max_peak_current_mA; /* maximum LED current per current source */
	u16 max_peak_duration_ms; /* the maximum duration max_peak_current_mA
				     can be applied */
	u16 max_sustained_current_mA; /* This leds maximum sustained current
					 in mA per current source */
	u16 min_current_mA; /* This leds minimum current in mA, desired
			       values smaller than this will be realised
			       using PWM. */
	u16 vf_mV; /* This leds forward voltage at maximum current */
	bool load_balance_on; /* only relevant for unmatched LED forward
				 voltages and two LED operation */

	/* Following settings are used while diagnostic pulse is executed */
	u8 diag_pulse_time; /* 0->4ms 1->8ms, 3->12ms, duration of diagnostic
			       pulse to evaluate the system for the main flash*/
	u8 diag_pulse_vcompl_adj; /* 0..15, consult AMS for exact value*/
	u16 diag_vin_low_v_mV; /* see vin_low_v_mV: This value is used when
				  doing diagnostic pulse */
	u8 diag_pulse_min_on_increase; /* 0..3, , only relevant if
					  dcdc_skip_enable==0 */
	bool diag_pulse_force_dcdc_on; /* force dcdc operation during
					  diagnostic pulse, only relevant if
					  dcdc_skip_enable=0 */
};
#endif /* __LINUX_AS3649_H */


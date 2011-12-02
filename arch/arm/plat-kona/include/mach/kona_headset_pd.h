/* Header file for HEASET DRIVER */

#ifndef __KONA_HEADSET_PD__
#define __KONA_HEADSET_PD__

struct kona_headset_pd {
	/* The value for this -
	 * 1: If the GPIO state read for the GPIO line
	 * connected for the HS detect is 0(LOW) for HS
	 * insert and 1(HIGH) for HS remove
	 * 0: If the GPIO state read for the GPIO line
	 * connected for the HS detect is 0(LOW) for HS
	 * remove and 1(HIGH) for HS insert
	 */
	int hs_default_state;
	/*
	 * Because of the resistor in the MIC_IN line the actual ground is not 0,
	 * but a small offset is added to it. We call this as phone_ref_offset.
	 * This value will be used by the headset driver, in the logic that
	 * reads the ADC value from MIC_IN and does the calculation to find
	 * out the type of accessory connected.
	 */
	int phone_ref_offset;
};

#endif /*  __KONA_HEADSET_PD__ */

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

	/*
	 * accessory insertion/removal method -
	 * If gpio is used program this variable as 1
	 * if not program this as 0.
	 */
	int gpio_for_accessory_detection;

	/*
	 * Pointer to a 2D array, in which the second dimension is 2 (min and
	 * max ADC value for a given button press).
	 * This is HW specific and hence passed from the board file.
	 * If this argument is not passed the driver uses a default
	 * home brewn value. This may or may not work properly for a given HW
	 * platform.
	 */
	unsigned int (*button_adc_values_low)[2];

	unsigned int (*button_adc_values_high)[2];
	/*
	 * GPIO number to ground the mic line.
	 */
	int gpio_mic_gnd;
	char *ldo_id;

};
/* This API is used by Audio code to switch
 * the mic bias when required.
 */
int switch_bias_voltage(int mic_status);

/*
 * NOTE:
 * Note that the following configurations are supported in the driver
 * 1) Config 1
 *      a) GPIO for accessory insertion
 *      b) COMP1 for accessory type detection (on open cable).
 *      c) COMP2 for button press/removal detection.
 * 3) Config 2
 *      a) COMP1 for accessory insertion/removal.
 *      c) COMP2 for button press/removal detection.
 *
 * Backward compatibility - In the board file rhea_ray.c,
 * gpio_for_accessory_detection is not initialized and COMP1 IRQ is not
 * passed. In this case kona_headset.c will continue to use
 * GPIO for accessory detection and COMP2 for type detection and button press
 * detection.
 *
 * For Config 1 - From the board file gpio_for_accessory_detection should be
 * initialized to 1. COMP1 IRQ is passed through 'resources' structure.
 *
 * For Config 2 - Initialize gpio_for_accessory_detection to zero. COMP1 IRQ
 * is passed through 'resources' structure. The kona_headset_multi_button.c
 * implementation adapts itself to support accessory insertion/removal using
 * COMP1
 */
#endif /*  __KONA_HEADSET_PD__ */

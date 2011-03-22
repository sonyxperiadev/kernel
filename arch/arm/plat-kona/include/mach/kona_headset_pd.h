/* Header file for HEASET DRIVER */

#ifndef __KONA_HEADSET_PD__
#define __KONA_HEADSET_PD__

struct kona_headset_pd {
	int hs_default_state;	/* The value for this -
				 * 1: If the GPIO state read for the GPIO line
				 * connected for the HS detect is 0(LOW) for HS
				 * insert and 1(HIGH) for HS remove
				 * 0: If the GPIO state read for the GPIO line
				 * connected for the HS detect is 0(LOW) for HS
				 * remove and 1(HIGH) for HS insert
				 */
};

#endif /*  __KONA_HEADSET_PD__ */

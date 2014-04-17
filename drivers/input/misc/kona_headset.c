/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/input/misc/kona_headset.c
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

/* This should be defined before kernel.h is included */
/*#define DEBUG*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif
#include <linux/clk.h>
#include <linux/err.h>
#include <mach/io_map.h>
#include <mach/kona_headset_pd.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb/brcm_rdb_auxmic.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>

#include <linux/broadcom/bcmpmu_audio.h>

#include <plat/chal/chal_aci.h>
#include <plat/kona_mic_bias.h>

/* TODO:
 * 1. There is a issue now with the interrupts being issued from ACI on headset
 *insert/remove. ACI issues an interrupt for both Headset insert/remove and
 * Headset button press. But the driver uses interrupts issued by ACI to only
 *detect Button press. So when the headset is removed/inserted slowly the ACI
 *interrupt issued for the HS remove/insert is serviced before the interrupt
 *issued on the GPIO line.
 * As a work around till the actual fix is figured out, the delayed work queue
 *for the HS Button press is scheduled after a delay to make sure that even if
 *the HSB IRQ is serviced before the HS ISR, the GPIO status read in
 *delayed work queue before sending the event to the input sub-system reads
 *right. But still on slow HS insertion, 1 HS button interrupt is serviced.
 * 2. Add DEBOUNCE_TIME as a part of the platform data in the board file to be
 *accessed in the driver - to make it board specific and not driver generic
 */

 /*
  * Driver HW resource usage info:
  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  * GPIO  - Accessory insertion/removal detection
  *
  * COMP2 - In case if an open cable such as AD52 is plugged in first
  *         to further detect a headset/headphone, when the item is plugged
  *         to the open cable
  *
  * COMP2 - If the connected accesory is a headset configured to detect
  *         button press
  *
  * For button release detection previously COMP1 INV ISR was used, but that
  * logic does not work well if the MIC BIAS is configured in periodic
  * measurement mode. So the button press work queue is used detect the button
  * release by reading the raw output of COMP2
  */

/*
 * The gpio_set_debounce expects the debounce argument in micro seconds
 * Previously the kona implementation which is called from gpio_set_debounce
 * was expecting the argument as milliseconds which was incorrect.
 * The commit 38598aea6cb57290ef6ed3518b75b9f398b7694f fixed it. Hence we have
 * to change to the correct way of passing the time in microseconds resolution.
 */
#define DEBOUNCE_TIME	(64000)
#define KEY_DETECT_DELAY	msecs_to_jiffies(128)
#define ACCESSORY_INSERTION_REMOVE_SETTLE_TIME	msecs_to_jiffies(500)

struct mic_t {
	int hsirq;
	int hsbirq_press;
	int hsbirq_release;
	int auxmic_base;
	int aci_base;
	CHAL_HANDLE aci_chal_hdl;
	struct kona_headset_pd *headset_pd;
#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	struct delayed_work accessory_detect_work;
	struct delayed_work button_press_work;
	struct input_dev *headset_button_idev;
	int hs_state;
	int button_state;
	/*
	 * 1 - mic bias is ON
	 * 0 - mic bias is OFF
	 */
	int mic_bias_status;
};


static struct mic_t *mic_dev;

enum hs_type {
	DISCONNECTED = 0,	/* Nothing is connected  */
	HEADPHONE = 1,		/* The one without MIC   */
	OPEN_CABLE,		/* Not sent to userland  */
	HEADSET,		/* The one with MIC      */
	UNSUPPORTED,		/* Unsupported accessory connected */
	/* If more HS types are required to be added
	 *add here, not below HS_TYPE_MAX
	 */
	HS_TYPE_MAX,
	HEADSET_DETECTED,
	HEADPHONE_DETECTED,
	OPEN_CABLE_DETECTED
};

enum button_state {
	BUTTON_RELEASED = 0,
	BUTTON_PRESSED
};

/*
 * After configuring the ADC, it takes different 'time' for the
 * ADC to settle depending on the HS type. The time outs are
 *in milli seconds
 */
#define DET_PLUG_CONNECTED_SETTLE    80	/* in HAL_ACC_DET_PLUG_CONNECTED */
#define DET_OPEN_CABLE_SETTLE        20	/* in HAL_ACC_DET_OPEN_CABLE */
#define DET_BASIC_CARKIT_SETTLE      20	/* in HAL_ACC_DET_BASIC_CARKIT */

/*
 * Button/Hook Filter configuration
 * 1024 / (Filter block frequencey) = 1024 / 32768 => 31ms
 */
#define ACC_HW_COMP1_FILTER_WIDTH   1024
/*
 * Accessory Detecting voltage
 *
 * Voltage defined in mv
 * The voltage levels below are for Headphone,
 * Headset, ECI accessory and Video cable detection
 * based on customer spec.
 */
#define MIC_CHANGE_DETECTION_THRESHOLD  20
#define HEADPHONE_DETECT_LEVEL_MIN      0
#define HEADPHONE_DETECT_LEVEL_MAX      40
#define VIDEO_CABLE_DETECT_LEVEL_MIN	41
#define VIDEO_CABLE_DETECT_LEVEL_MAX	90
#define ECI_ACCESSORY_DETECT_LEVEL_MIN  91
#define ECI_ACCESSORY_DETECT_LEVEL_MAX  599

#define OPENCABLE_DETECT_LEVEL_MIN      1900
#define OPENCABLE_DETECT_LEVEL_MAX      5000

#define BASIC_HEADSET_DETECT_LEVEL_MIN  0
#define BASIC_HEADSET_DETECT_LEVEL_MAX  200

/* Accessory Hardware configuration support variables */
/* ADC */
static const CHAL_ACI_filter_config_adc_t aci_filter_adc_config = { 0, 0x0B };

static const CHAL_ACI_filter_config_comp_t comp_values_for_button_press = {
	CHAL_ACI_FILTER_MODE_INTEGRATE,
	CHAL_ACI_FILTER_RESET_FIRMWARE,
	0,			/* = S */
	0xFE,			/* = T */
	0x500,			/* = M = 1280 / 32768 => 39ms */
	ACC_HW_COMP1_FILTER_WIDTH	/* = MT */
};

/* COMP2 */
static const CHAL_ACI_filter_config_comp_t comp_values_for_type_det = {
	CHAL_ACI_FILTER_MODE_INTEGRATE,
	CHAL_ACI_FILTER_RESET_FIRMWARE,
	0,			/* = S  */
	0xFE,			/* = T  */
	0x700,			/* = M  */
	0x650			/* = MT */
};

/* MIC bias */

/*
 * Based on the power consumptio analysis. Program the MIC BIAS probe cycle to
 * be 128ms. In this the mesaurement interval is 64ms and the measurement delay
 * is 8ms.
 *
 * Mic Bias Power down control  and peridoic measurement control looks like
 * this
 *
 *  --                      -------------------------------
 *    | <-------------->   | <---------------------------> |
 *    |     64 ms          |    128-64= 64ms               |
 *     --------------------
 *            -------------
 *    <----->| <---------> |
 *      8ms  |   56ms      |
 * ----------               ---------------------
 *
 */
static CHAL_ACI_micbias_config_t aci_mic_bias_high = {
	CHAL_ACI_MIC_BIAS_ON,
	CHAL_ACI_MIC_BIAS_2_1V,
	CHAL_ACI_MIC_BIAS_PRB_CYC_128MS,
	CHAL_ACI_MIC_BIAS_MSR_DLY_8MS,
	CHAL_ACI_MIC_BIAS_MSR_INTVL_64MS,
	CHAL_ACI_MIC_BIAS_1_MEASUREMENT
};

static CHAL_ACI_micbias_config_t aci_mic_bias_low = {
	CHAL_ACI_MIC_BIAS_DISCONTINUOUS,
	CHAL_ACI_MIC_BIAS_0_45V,
	CHAL_ACI_MIC_BIAS_PRB_CYC_32MS,
	CHAL_ACI_MIC_BIAS_MSR_DLY_4MS,
	CHAL_ACI_MIC_BIAS_MSR_INTVL_16MS,
	CHAL_ACI_MIC_BIAS_1_MEASUREMENT
};
/* These are two threshold values programmed for the COMP
 * to generate interrupt. Value @ index 0 is for 0.4V
 * of MIC Bias and @index 1 is for 2.1V of Mic Bias.
*/
static int button_voltage_range[] = {50, 600};

/* Vref */
static CHAL_ACI_vref_config_t aci_vref_config = { CHAL_ACI_VREF_OFF };

static int aci_interface_init(struct mic_t *mic);
static int aci_interface_init_micbias_off(struct mic_t *mic);

static void __low_power_mode_config(void);

/* Function to dump the HW regs */
#ifdef DEBUG
static struct clk *audioh_apb_clk;

static void dump_hw_regs(struct mic_t *p)
{
	int i;

	pr_info("\r\n Dumping MIC BIAS registers \r\n");
	for (i = 0x0; i <= 0x28; i += 0x04) {
		pr_info("Addr: 0x%x  OFFSET: 0x%x  Value:0x%x \r\n",
			p->auxmic_base + i, i, readl(p->auxmic_base + i));
	}

	pr_info("\r\n \r\n");
	pr_info("Dumping ACI registers \r\n");
	for (i = 0x30; i <= 0xD8; i += 0x04) {
		pr_info("Addr: 0x%x  OFFSET: 0x%x  Value:0x%x \r\n",
			p->aci_base + i, i, readl(p->aci_base + i));
	}

	for (i = 0x400; i <= 0x420; i += 0x04) {
		pr_info("Addr: 0x%x  OFFSET: 0x%x Value:0x%x \r\n",
			p->aci_base + i, i, readl(p->aci_base + i));
	}

	if (audioh_apb_clk != NULL) {
		clk_enable(audioh_apb_clk);

		/* AudioH registers */
		pr_info("Addr: 0x%x  value 0x%x \r\n", KONA_AUDIOH_VA + 0x200,
			readl(KONA_AUDIOH_VA + 0x200));
		pr_info("Addr: 0x%x  value 0x%x \r\n", KONA_AUDIOH_VA + 0x208,
			readl(KONA_AUDIOH_VA + 0x208));
		pr_info("Addr: 0x%x  value 0x%x \r\n", KONA_AUDIOH_VA + 0x20C,
			readl(KONA_AUDIOH_VA + 0x20C));
		pr_info("Addr: 0x%x  value 0x%x \r\n", KONA_AUDIOH_VA + 0x210,
			readl(KONA_AUDIOH_VA + 0x210));

		clk_disable(audioh_apb_clk);
	}
	pr_info("\r\n \r\n");

	pr_info("\r\n \r\n");
}
#endif

/* Low level functions called to detect the accessory type */
static int aci_hw_config(int hst)
{
	int time_to_settle = 0;

	pr_debug("\nDetect acs type aci_hw_config\n");
	if (mic_dev == NULL) {
		pr_err("aci_adc_config: invalid mic_dev handle \r\n");
		return -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("aci_adc_config: Invalid CHAL handle \r\n");
		return -EFAULT;
	}

	switch (hst) {
	case HEADPHONE:

		pr_debug("aci_hw_config: Configuring for headphone \r\n");
		aci_mic_bias_high.mode = CHAL_ACI_MIC_BIAS_ON;
		/* Setup MIC bias */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
				    CHAL_ACI_BLOCK_GENERIC,
					&aci_mic_bias_high);
		/* TODO: Setup the Interrupt Source, is this required ??? */

		/* Power up Digital block */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ENABLE,
				    CHAL_ACI_BLOCK_DIGITAL);

		/* Power up the ADC */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ENABLE,
				    CHAL_ACI_BLOCK_ADC);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
				    CHAL_ACI_BLOCK_ADC,
				    CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
				    CHAL_ACI_BLOCK_ADC, &aci_filter_adc_config);

		time_to_settle = DET_PLUG_CONNECTED_SETTLE;
		break;

	case OPEN_CABLE:

		pr_debug("aci_hw_config: Configuring for open cable \r\n");

		/* Powerup ADC */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ENABLE,
				    CHAL_ACI_BLOCK_ADC);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
				    CHAL_ACI_BLOCK_ADC,
				    CHAL_ACI_BLOCK_ADC_HIGH_VOLTAGE);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
				    CHAL_ACI_BLOCK_ADC, &aci_filter_adc_config);

		time_to_settle = DET_OPEN_CABLE_SETTLE;
		break;

	case HEADSET:

		pr_debug("aci_hw_config: Configuring for HEADSET \r\n");

		/* Turn OFF MIC Bias */
		aci_mic_bias_high.mode = CHAL_ACI_MIC_BIAS_GND;
		/* Setup MIC bias */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
				    CHAL_ACI_BLOCK_GENERIC,
					&aci_mic_bias_high);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_MIC_POWERDOWN_HIZ_IMPEDANCE,
				    CHAL_ACI_BLOCK_GENERIC, 0);
		time_to_settle = DET_BASIC_CARKIT_SETTLE;
		break;

	default:
		/* TODO: Logically what does this mean ???? */
		break;
	}

	/*
	 * Wait till the ADC settles, the timings varies for different types
	 *of headset.
	 */
	/* TODO:
	 * There is a settling time after which a call back is
	 *invoked, does this call back trigger any other HW config or
	 *its just a notification to the upper layer ????
	 */
	if (time_to_settle != 0)
		msleep(time_to_settle);

	return 0;
}

static int aci_hw_read(int hst)
{
	int status = -1;
	int mic_level;

	if (mic_dev == NULL) {
		pr_err("aci_adc_config: invalid mic_dev handle \r\n");
		return -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("aci_adc_config: Invalid CHAL handle \r\n");
		return -EFAULT;
	}

	/*
	 * What is phone_ref_offset?
	 *
	 *Because of the resistor in the MIC IN line the actual ground is not 0,
	 *but a small offset is added to it. We call this as
	 *phone_ref_offset.
	 * This needs to be subtracted from the measured voltage to determine
	 *the correct value. This will vary for different HW based on the
	 *resistor values used. So by default this variable is 0, if no one
	 *initializes it. For boards on which this resistor is present this
	 *value should be passed from the board specific data structure
	 *
	 * In the below logic, if mic_level read is less than or equal to 0
	 *then we don't do anything.
	 * If the read value is greater than  phone_ref_offset then subtract
	 *this offset from the value read, otherwise mic_level is zero
	 */
	mic_level = chal_aci_block_read(mic_dev->aci_chal_hdl,
					CHAL_ACI_BLOCK_ADC,
					CHAL_ACI_BLOCK_ADC_RAW);
	pr_debug(" ++ aci_hw_read: mic_level before calc %d \r\n", mic_level);
	mic_level = mic_level <= 0 ? mic_level :
	    ((mic_level > mic_dev->headset_pd->phone_ref_offset) ?
	     (mic_level - mic_dev->headset_pd->phone_ref_offset) : 0);
	pr_debug(" ++ aci_hw_read: mic_level after calc %d \r\n", mic_level);

	switch (hst) {
	case HEADPHONE:
		if ((mic_level >= HEADPHONE_DETECT_LEVEL_MIN &&
		     mic_level <= HEADPHONE_DETECT_LEVEL_MAX)
		    ||
		    (mic_level >= VIDEO_CABLE_DETECT_LEVEL_MIN &&
		     mic_level <= VIDEO_CABLE_DETECT_LEVEL_MAX)
			||
		    (mic_level >= ECI_ACCESSORY_DETECT_LEVEL_MIN &&
		     mic_level <= ECI_ACCESSORY_DETECT_LEVEL_MAX))
			status = HEADPHONE;
		break;
	case OPEN_CABLE:
		if (mic_level >= OPENCABLE_DETECT_LEVEL_MIN &&
		    mic_level <= OPENCABLE_DETECT_LEVEL_MAX)
			status = OPEN_CABLE;
		break;
	case HEADSET:
		if (mic_level >= BASIC_HEADSET_DETECT_LEVEL_MIN &&
		    mic_level <= BASIC_HEADSET_DETECT_LEVEL_MAX)
			status = HEADSET;
		break;
	default:
		/* TODO: Logically what does this mean ???? */
		break;
	}

	pr_debug("status is %d mic_level is %d \r\n", status, mic_level);
	return status;
}

/* Function that is invoked when the user tries to read from sysfs interface */
int detect_hs_type(struct mic_t *mic_dev)
{
	int i;
	int type;

	pr_debug("\nCheck detect_hs_type\n");
	if (mic_dev == NULL) {
		pr_err("mic_dev is empty \r\n");
		return 0;
	}

	/*
	 * Configure the ACI block of ADC for detecting a particular type of
	 *accessory
	 */
	for (i = HEADPHONE; i < HS_TYPE_MAX; i++) {
		/* Configure the ADC to check a given HS type */
		aci_hw_config(i);

		/*
		 * Now, read back to check whether the given accessory is
		 *present. If yes, then break the loop. If not, continue
		 */
		type = aci_hw_read(i);
		if (type == i)
			break;
	}			/* end of loop to check the devices */
	return type;
}

static int is_accessory_supported(struct mic_t *mic_dev)
{
	int mic_level1;
	int mic_level2;
	int ret;

#if defined(CONFIG_BCMPMU_AUDIO)
#if defined(CONFIG_MFD_BCM59055)
	BCMPMU_Audio_HS_Param hs_param;
#endif
#endif

	/* Power ON Mic BIAS */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			    CHAL_ACI_BLOCK_GENERIC,
					&aci_mic_bias_high);


	/* Power up Digital block */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_ENABLE,
			    CHAL_ACI_BLOCK_DIGITAL);

	/* Power up the ADC */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_ENABLE, CHAL_ACI_BLOCK_ADC);

	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
			    CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_FULL_RANGE);

	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
			    CHAL_ACI_BLOCK_ADC, &aci_filter_adc_config);
	msleep(80);

	mic_level1 = chal_aci_block_read(mic_dev->aci_chal_hdl,
					 CHAL_ACI_BLOCK_ADC,
					 CHAL_ACI_BLOCK_ADC_RAW);
	mic_level1 = mic_level1 <= 0 ? mic_level1 :
	    ((mic_level1 > mic_dev->headset_pd->phone_ref_offset) ?
	     (mic_level1 - mic_dev->headset_pd->phone_ref_offset) : 0);
	pr_debug(" ++ %s(): mic_level1 after calc %d \r\n", __func__,
		 mic_level1);

#if defined(CONFIG_BCMPMU_AUDIO)
#if defined(CONFIG_MFD_BCM59055)
	hs_param = bcmpmu_get_hs_param_from_audio_driver();
	/*if during MP3, plug in headset, accessory manager notifies
	audio device driver, audio device driver power on HS amp
	and set up HS gains.*/
	if (hs_param.hs_power == 1) {

		/*if headset amp is aleady powered on by audio
		device driver for FM radio or MP3 playback.*/
		/*allow some time for audio device driver
		to finish setting HS gains.*/
		msleep(30);

		/* mute HS output to make HS type detection work.*/
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);

	} else {
#endif
#endif

		/* Turn On the HP Power Amplifier */
		bcmpmu_audio_init();
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);
		bcmpmu_hs_power((void *)1);

#if defined(CONFIG_BCMPMU_AUDIO)
#if defined(CONFIG_MFD_BCM59055)
	}
#endif
#endif
	msleep(50);

	/* Read mic level again */
	mic_level2 = chal_aci_block_read(mic_dev->aci_chal_hdl,
					 CHAL_ACI_BLOCK_ADC,
					 CHAL_ACI_BLOCK_ADC_RAW);
	mic_level2 = mic_level2 <= 0 ? mic_level2 :
	    ((mic_level2 > mic_dev->headset_pd->phone_ref_offset) ?
	     (mic_level2 - mic_dev->headset_pd->phone_ref_offset) : 0);
	pr_debug(" ++ %s(): mic_level2 after calc %d \r\n", __func__,
		 mic_level2);

	if ((mic_level1 - mic_level2) >= MIC_CHANGE_DETECTION_THRESHOLD) {
		pr_debug("%s(): swapped pin order headset\r\n", __func__);
		ret = 0;
	} else {
		pr_debug("%s(): Supported accessory \r\n", __func__);
		ret = 1;
	}

#if defined(CONFIG_BCMPMU_AUDIO)
#if defined(CONFIG_MFD_BCM59055)

	if (hs_param.hs_power == 1) {

		/*if headset amp is aleady powered on by audio device driver,
		for FM radio or MP3 playback.*/
		msleep(20);

		/*restore the gain which was set by audio device driver */
		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, hs_param.hs_gain_left);

	} else {
#endif
#endif

		bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);
		bcmpmu_hs_power((void *)0);
		bcmpmu_audio_deinit();

#if defined(CONFIG_BCMPMU_AUDIO)
#if defined(CONFIG_MFD_BCM59055)
	}
#endif
#endif

	return ret;
}

/*------------------------------------------------------------------------------
    Function name   : button_press_work_func
    Description     : Work function that will send the button press/release
		evetns to the input sub-system.
    Return type     : void
------------------------------------------------------------------------------*/
static void button_press_work_func(struct work_struct *work)
{
	struct mic_t *p = container_of(work, struct mic_t,
				       button_press_work.work);
	int comp_status;
	unsigned long headset_state = gpio_get_value(irq_to_gpio(p->hsirq));
	headset_state = (headset_state ^ p->headset_pd->hs_default_state);

	if (headset_state == 0) {
		pr_err("Spurious button interrupts \r\n");
		return;
	}

	/*
	 * Just to be sure check whether this is happened when the
	 * Headset accessory is inserted,
	 * If its triggered when the connected accessory is NOT a headset
	 *_OR_ when none of the accessory is conneceted its spurious
	 *
	 */
	comp_status = chal_aci_block_read(p->aci_chal_hdl,
					  CHAL_ACI_BLOCK_COMP2,
					  CHAL_ACI_BLOCK_COMP_RAW);

   /* What we observe is that while we plug-out the Headset from the
    * connector, sometimes we get COMP1 interrupts indicating
    * button press/release. While removing the headset there is some
    * disturbance on the MIC line and if it happens to match the
    * threshold programmed for button press/release the
    * interrupt is fired.
    * So practically we cannot avoid this situation.
    * To handle this, we update the state of the accessory
    * (connected / unconnected)in gpio_isr(). Once the button ISR
    * happens, we read the ADC values etc immediately but we
    * delay notifying this to the upper layers by
    * 80 msec(this value is arrived at by experiment)
    * So that if this is a spurious
    * notification, the GPIO ISR would be fired by this time
    * and would have updated the accessory state as disconnected.
    * In such cases the upper layer is not notified.
    */
	usleep_range(80000, 90000);

	if (p->hs_state != DISCONNECTED) {
		if (comp_status == CHAL_ACI_BLOCK_COMP_LINE_LOW) {
			if ((p->hs_state == HEADSET) &&
			    (p->button_state == BUTTON_RELEASED)) {
				p->button_state = BUTTON_PRESSED;
				pr_info(" Sending Key Press\r\n");
				input_report_key(p->headset_button_idev,
					KEY_MEDIA, 1);
				input_sync(p->headset_button_idev);
			}
			schedule_delayed_work(&(p->button_press_work),
				      KEY_DETECT_DELAY);
		} else {
			p->button_state = BUTTON_RELEASED;
			pr_info(" Sending Key Release\r\n");
			input_report_key(p->headset_button_idev, KEY_MEDIA, 0);
			input_sync(p->headset_button_idev);
		}
	}
}

/*------------------------------------------------------------------------------
    Function name   : accessory_detect_work_func
    Description     : Work function that will set the state of the headset
		switch dev and enable/disable the HS button interrupt
    Return type     : void
------------------------------------------------------------------------------*/
static void accessory_detect_work_func(struct work_struct *work)
{
	struct mic_t *p = container_of(work, struct mic_t,
				       accessory_detect_work.work);
	unsigned headset_state = gpio_get_value(irq_to_gpio(p->hsirq));
	pr_debug("SWITCH WORK GPIO STATE: 0x%x default state 0x%x \r\n",
		 headset_state, p->headset_pd->hs_default_state);
	headset_state = (headset_state ^ p->headset_pd->hs_default_state);
	pr_debug("\n\naccessory_detect_work_func headset_state=%d\n",
		 headset_state);

	if (headset_state == 1) {

		pr_info(" ACCESSORY INSERTED \r\n");
		pr_debug
		    ("0. Interrupt status before detecting hs_type 0x%x \r\n",
		     readl(p->aci_base + ACI_INT_OFFSET));

		aci_interface_init(p);

#ifdef DEBUG
		pr_info("After initialzing the ACI interface \r\n");
		dump_hw_regs(p);
#endif

		/* Check whether the connected accessory is supported first */
		if (is_accessory_supported(p) == 0) {
			pr_info("%s(): swapped pin order headset detected\r\n",
				__func__);
			if ((p->headset_pd->gpio_mic_gnd)
			    && (gpio_is_valid(p->headset_pd->gpio_mic_gnd))) {
				pr_info
				    ("%s(): "
				    "mic2gnd forcing enabled with GPIO %d\r\n",
				     __func__, p->headset_pd->gpio_mic_gnd);
				gpio_set_value(p->headset_pd->gpio_mic_gnd, 1);
			} else {
				pr_info("%s(): ACCESSORY IS NOT SUPPORTED \r\n",
					__func__);

				p->hs_state = UNSUPPORTED;
				/*
				 * Bit map of the state variable
				 * 0xFF - Accessory not supported
				 * There by we keep enough room to add other
				 * accessory types between 2..0xFF
				 * if needed in future
				 */
				switch_set_state(&(p->sdev), 0xFF);
				aci_interface_init_micbias_off(p);
				return;
			}
		}

		p->hs_state = detect_hs_type(p);
		pr_debug("\nHeadset inserted with hs_state=%d\n", p->hs_state);
		switch (p->hs_state) {

		case OPEN_CABLE:
			pr_debug
			    ("Open cable reconfig threshold values of COMP1\r\n");

			/*
			 * Put the MIC BIAS in Discontinuous measurement mode
			 * to detect further accessory insertion
			 */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
					    CHAL_ACI_BLOCK_GENERIC,
						&aci_mic_bias_low);

			/* Configure the comparator 2 for type detection */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
					    CHAL_ACI_BLOCK_COMP2,
					    &comp_values_for_type_det);

			/*
			 * Set the threshold value for accessory insertion
			 * detection
			 */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
					    CHAL_ACI_BLOCK_COMP2, 1900);

			/* No notification needed for the userland */

#ifdef DEBUG
			pr_info("Configured for Open cable \r\n");
			dump_hw_regs(p);
#endif
			/* Clear pending interrupts if any */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
					    CHAL_ACI_BLOCK_COMP);

			/* Enable COMP2 interrupt for accessory detection */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
					    CHAL_ACI_BLOCK_COMP2);
			break;

		case HEADSET:
			pr_debug
			    ("Detected headset config for button press \r\n");

			/* Put back the aci interface to be able to detect the
			 *button press. Especially this functions puts the
			 * COMP2 and MIC BIAS values to be able to detect
			 *button press
			 */
			p->button_state = BUTTON_RELEASED;

			/*
			 * Put the MIC BIAS in Discontinuous measurement mode
			 * to detect button press
			 */
			aci_mic_bias_low.mode = CHAL_ACI_MIC_BIAS_DISCONTINUOUS;
			chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
					    CHAL_ACI_BLOCK_GENERIC,
						&aci_mic_bias_low);

			__low_power_mode_config();

			/* Configure the comparator 2 for button press */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
					    CHAL_ACI_BLOCK_COMP2,
					    &comp_values_for_button_press);

			/* Set the threshold value for button press */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
					    CHAL_ACI_BLOCK_COMP2,
						button_voltage_range[0]);

			/*
			 * A settling time is required here. The call to
			 * configure the COMP2, MIC BIAS trigggers COMP2 and
			 * COMP2 INV interrupts.
			 * Note that some times the interrupt gets
			 * triggered after some time say 't'. If this 't'
			 * happens before the next call to clear spurious
			 * interrupts everything is OK, but if not it will
			 * trigger unwanted button press, release events. So
			 * its better to allow the work to wait until the
			 * settling time and then clear the interrupts
			 */
			msleep(100);

			/* Clear pending interrupts if any */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
					    CHAL_ACI_BLOCK_COMP);

#ifdef DEBUG
			pr_info("Configured for button press \r\n");
			dump_hw_regs(p);
#endif

			/* Enable COMP2 interrupt for button press */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
					    CHAL_ACI_BLOCK_COMP2);
#ifdef CONFIG_SWITCH
			/*
			 * While notifying this to the android world we need to
			 * pass the accessory type as Androind understands.
			 * The Android 'state' variable is a bitmap as defined
			 * below.
			 * BIT 0 : 1 - Headset (with MIC) connected
			 * BIT 1 : 1 - Headphone (the one without MIC) is
			 *             connected
			 */
			switch_set_state(&(p->sdev), 1);
#endif
			break;
		case HEADPHONE:
			pr_debug("\n\nCase HEADPHONE\n");

			/* Turn OFF MIC Bias */
			aci_interface_init_micbias_off(p);

			/* Clear pending interrupts if any */
			chal_aci_block_ctrl(p->aci_chal_hdl,
					    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
					    CHAL_ACI_BLOCK_COMP);

			/* No need to enable the button press/release irq */
#ifdef CONFIG_SWITCH
			/*
			 * While notifying this to the android world we need to
			 * pass the accessory type as Androind understands.
			 * The Android 'state' variable is a bitmap as defined
			 * below.
			 * BIT 0 : 1 - Headset (with MIC) connected
			 * BIT 1 : 1 - Headphone (the one without MIC) is
			 *             connected
			 */
			switch_set_state(&(p->sdev), 2);
#endif
			break;
		default:
			pr_info("%s():Unknown accessory type %d \r\n", __func__,
				p->hs_state);
			break;
		}

		pr_debug("Interrupt status after detecting hs_type 0x%x \r\n",
			 readl(p->aci_base + ACI_INT_OFFSET));

	} else {
		pr_info(" ACCESSORY REMOVED \r\n");
		/*
		 * Turn OFF MIC Bias. In case of Headphone and
		 * unsupported Headset, we would have turned it OFF
		 * earlier
		 */
			pr_debug(" ACCESSORY REMOVED... p->hs_state"
				"is %d \r\n", p->hs_state);
			aci_interface_init_micbias_off(p);
		/* Inform userland about accessory removal */
		p->hs_state = DISCONNECTED;
		p->button_state = BUTTON_RELEASED;
#ifdef CONFIG_SWITCH
		switch_set_state(&(p->sdev), p->hs_state);
#endif
		/* Clear pending interrupts */
		chal_aci_block_ctrl(p->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
				    CHAL_ACI_BLOCK_COMP);

		/* Disable the COMP2, COMP2 INV interrupt */
		chal_aci_block_ctrl(p->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
				    CHAL_ACI_BLOCK_COMP);

		if ((p->headset_pd->gpio_mic_gnd)
		    && (gpio_is_valid(p->headset_pd->gpio_mic_gnd)))
			gpio_set_value(p->headset_pd->gpio_mic_gnd, 0);
	}
}

/*------------------------------------------------------------------------------
    Function name   : hs_unregswitchdev
    Description     : Unregister input device for headset
    Return type     : int
------------------------------------------------------------------------------*/
int hs_unregswitchdev(struct mic_t *p)
{
#ifdef CONFIG_SWITCH
	cancel_delayed_work_sync(&p->accessory_detect_work);
	switch_dev_unregister(&p->sdev);
#endif
	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : hs_switchinit
    Description     : Register sysfs device for headset
		It uses class switch from kernel/common/driver/switch
    Return type     : int
------------------------------------------------------------------------------*/
#ifdef CONFIG_SWITCH
int hs_switchinit(struct mic_t *p)
{
	int result = 0;
	p->sdev.name = "h2w";

	result = switch_dev_register(&p->sdev);
	if (result < 0)
		return result;

	INIT_DELAYED_WORK(&(p->accessory_detect_work),
			  accessory_detect_work_func);
	return 0;
}
#endif

/*------------------------------------------------------------------------------
    Function name   : hs_unreginputdev
    Description     : unregister and free the input device for headset button
    Return type     : int
------------------------------------------------------------------------------*/
static int hs_unreginputdev(struct mic_t *p)
{
	cancel_delayed_work_sync(&p->button_press_work);
	input_unregister_device(p->headset_button_idev);
	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : hs_inputdev
    Description     : Create and Register input device for headset button
    Return type     : int
------------------------------------------------------------------------------*/
static int hs_inputdev(struct mic_t *p)
{
	int result = 0;

	/* Allocate struct for input device */
	p->headset_button_idev = input_allocate_device();
	if ((p->headset_button_idev) == NULL) {
		pr_err("%s: Not enough memory\n", __func__);
		result = -EINVAL;
		goto inputdev_err;
	}

	/* specify key event type and value for it -
	 * Since we have only one button on headset, value KEY_MEDIA is sent */
	set_bit(EV_KEY, p->headset_button_idev->evbit);
	set_bit(KEY_MEDIA, p->headset_button_idev->keybit);
	p->headset_button_idev->name = "bcm_headset";
	p->headset_button_idev->phys = "headset/input0";
	p->headset_button_idev->id.bustype = BUS_HOST;
	p->headset_button_idev->id.vendor = 0x0001;
	p->headset_button_idev->id.product = 0x0100;

	/* Register input device for headset */
	result = input_register_device(p->headset_button_idev);
	if (result) {
		pr_err("%s: Failed to register device\n", __func__);
		hs_unreginputdev(p);
		goto inputdev_err;
	}

	INIT_DELAYED_WORK(&(p->button_press_work), button_press_work_func);

inputdev_err:
	return result;
}

/*------------------------------------------------------------------------------
    Function name   : gpio_isr
    Description     : interrupt handler
    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t gpio_isr(int irq, void *dev_id)
{
	struct mic_t *p = (struct mic_t *)dev_id;
	if (gpio_get_value(irq_to_gpio(p->hsirq)) == p->headset_pd->hs_default_state)
		p->hs_state = DISCONNECTED;
	pr_debug("HS ISR GPIO STATE: 0x%x \r\n",
		 gpio_get_value(irq_to_gpio(p->hsirq)));

	schedule_delayed_work(&(p->accessory_detect_work),
			      ACCESSORY_INSERTION_REMOVE_SETTLE_TIME);
	return IRQ_HANDLED;
}

/*------------------------------------------------------------------------------
    Function name   : comp2_isr
    Description     : interrupt handler
    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t comp2_isr(int irq, void *dev_id)
{
	struct mic_t *p = (struct mic_t *)dev_id;

	pr_debug("In COMP 2  ISR 0x%x ... \r\n",
		 readl(p->aci_base + ACI_INT_OFFSET));

	/* Acknowledge & clear the interrupt */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
			    CHAL_ACI_BLOCK_COMP2);

	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
			    CHAL_ACI_BLOCK_COMP2);

	pr_debug("After clearing 0x%x ... \r\n",
		 readl(p->aci_chal_hdl + ACI_INT_OFFSET));

	if (p->hs_state == HEADSET) {
		pr_debug("button press scheduling button_press_work \r\n");

		schedule_delayed_work(&(p->button_press_work), 0);

		/* Re-enable COMP2 Interrupts */
		chal_aci_block_ctrl(p->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
				    CHAL_ACI_BLOCK_COMP2);

	} else if (p->hs_state == OPEN_CABLE) {
		pr_debug("scheduling switch work \r\n");
		schedule_delayed_work(&(p->accessory_detect_work),
				      ACCESSORY_INSERTION_REMOVE_SETTLE_TIME);
	} else {
		pr_debug("Spurious\r\n");

		chal_aci_block_ctrl(p->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
				    CHAL_ACI_BLOCK_COMP2);
	}
	return IRQ_HANDLED;
}

static int aci_interface_init_micbias_off(struct mic_t *p)
{
	if (p == NULL)
		return -1;

	/* First disable all the interrupts */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
			    CHAL_ACI_BLOCK_COMP);

	/* Clear pending interrupts if any */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
			    CHAL_ACI_BLOCK_COMP);

	/* Configure the comparator 2 for button press */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
			    CHAL_ACI_BLOCK_COMP2,
			    &comp_values_for_button_press);

	/*
	 * Connect P_MIC_DATA_IN to P_MIC_OUT  and P_MIC_OUT to COMP2
	 * Note that one API can do this.
	 */
	chal_aci_set_mic_route(p->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

	aci_vref_config.mode = CHAL_ACI_VREF_OFF;
	chal_aci_block_ctrl(p->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_VREF,
			    CHAL_ACI_BLOCK_GENERIC, &aci_vref_config);

		aci_mic_bias_low.mode = CHAL_ACI_MIC_BIAS_OFF;
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			    CHAL_ACI_BLOCK_GENERIC,
				&aci_mic_bias_low);

	/* Switch OFF Mic BIAS only if its not already OFF */
	if (p->mic_bias_status == 1) {
		kona_mic_bias_off();
		p->mic_bias_status = 0;
	}

	return 0;
}

static int aci_interface_init(struct mic_t *p)
{
	if (p == NULL)
		return -1;

/*
 * IMPORTANT
 *---------
 * Configuring these AUDIOH MIC registers was required to get ACI interrupts
 *for button press. Really not sure about the connection.
 * But this logic was taken from the BLTS code and if this is not
 *done then we were not getting the ACI interrupts for button press.
 *
 * Looks like if the Audio driver init happens this is not required, in
 *case if Audio driver is not included in the build then this macro should
 *be included to get headset working.
 *
 * Ideally if a macro is used to control brcm audio driver inclusion that does
 * AUDIOH init, then we	 don't need another macro here, it can be something
 *like #ifndef CONFING_BRCM_AUDIOH
 *
 */
#ifdef CONFIG_HS_PERFORM_AUDIOH_SETTINGS
	/* AUDIO settings */
	writel(AUDIOH_AUDIORX_VRX1_AUDIORX_VRX_SEL_MIC1B_MIC2_MASK,
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_VRX1_OFFSET);
	writel(0x0, KONA_AUDIOH_VA + AUDIOH_AUDIORX_VRX2_OFFSET);
	writel((AUDIOH_AUDIORX_VREF_AUDIORX_VREF_POWERCYCLE_MASK |
		AUDIOH_AUDIORX_VREF_AUDIORX_VREF_FASTSETTLE_MASK),
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_VREF_OFFSET);
	writel((AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK |
		AUDIOH_AUDIORX_VMIC_AUDIORX_MIC_EN_MASK),
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_VMIC_OFFSET);
	writel(AUDIOH_AUDIORX_BIAS_AUDIORX_BIAS_PWRUP_MASK,
	       KONA_AUDIOH_VA + AUDIOH_AUDIORX_BIAS_OFFSET);
#endif

	/* First disable all the interrupts */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
			    CHAL_ACI_BLOCK_COMP);

	/* Clear pending interrupts if any */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
			    CHAL_ACI_BLOCK_COMP);

	pr_debug("=== aci_interface_init: Interrupts disabled \r\n");

	/* Turn ON only if its not already ON */
	if (p->mic_bias_status == 0) {
		kona_mic_bias_on();
		p->mic_bias_status = 1;
	}

	/*
	 * This ensures that the timing parameters are configured
	 *properly. Note that only when the mode is
	 * CHAL_ACI_MIC_BIAS_DISCONTINUOUS, this is done.
	 */
	aci_mic_bias_low.mode = CHAL_ACI_MIC_BIAS_ON;
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			    CHAL_ACI_BLOCK_GENERIC,
				&aci_mic_bias_low);

	pr_debug("=== aci_interface_init: MIC BIAS settings done \r\n");

	/* Configure the comparator 2 for button press */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
			    CHAL_ACI_BLOCK_COMP2,
			    &comp_values_for_button_press);

	pr_debug
	    ("=== aci_interface_init: ACI Block2 comprator2 configured \r\n");

	/*
	 * Connect P_MIC_DATA_IN to P_MIC_OUT  and P_MIC_OUT to COMP2
	 * Note that one API can do this.
	 */
	chal_aci_set_mic_route(p->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

	pr_debug("=== aci_interface_init: Configured MIC route \r\n");

	/* Fast power up the Vref of ADC block */
	/*
	 * NOTE:
	 * This chal call was failing becuase internally this call
	 *was configuring AUDIOH registers as well. We have commmented
	 *configuring AUDIOH reigsrs in CHAL and it works OK
	 */
	aci_vref_config.mode = CHAL_ACI_VREF_FAST_ON;
	chal_aci_block_ctrl(p->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_VREF,
			    CHAL_ACI_BLOCK_GENERIC, &aci_vref_config);

	pr_debug("=== aci_interface_init: Configured Vref and ADC \r\n");

	/* Set the threshold value for button press */
	chal_aci_block_ctrl(p->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
			    CHAL_ACI_BLOCK_COMP2,
				button_voltage_range[0]);

	pr_debug("=== Configured the threshold value for button press\r\n");

	return 0;
}

/*------------------------------------------------------------------------------
    Function name   : headset_hw_init
    Description     : Hardware initialization sequence
    Return type     : int
------------------------------------------------------------------------------*/
static int headset_hw_init(struct mic_t *mic)
{
	int status = 0;
	unsigned hs_gpio;

	/* Initial settings for GPIO */
	hs_gpio = irq_to_gpio(mic->hsirq);
	pr_debug("\nheadset_hw_init hs_gpio=%d\n", hs_gpio);
	/* Request the gpio
	 * Note that this is an optional call for setting direction/debounce
	 *values. But set debounce will throw out warning messages if we
	 *call gpio_set_debounce without calling gpio_request.
	 * Note that it just throws out Warning messages and proceeds
	 *to auto request the same. We are adding this call here to
	 *suppress the warning message.
	 */
	status = gpio_request(hs_gpio, "hs_detect");
	if (status < 0) {
		pr_err("%s: gpio request failed \r\n", __func__);
		return status;
	}

	/* Set the GPIO debounce */
	status = gpio_set_debounce(hs_gpio, DEBOUNCE_TIME);
	if (status < 0) {
		pr_err("%s: gpio set debounce failed\n", __func__);
		return status;
	}

	/* Set the GPIO direction input */
	status = gpio_direction_input(hs_gpio);
	if (status < 0) {
		pr_err("%s: gpio set direction input failed\n", __func__);
		return status;
	}

	if ((mic->headset_pd->gpio_mic_gnd)
	    && (gpio_is_valid(mic->headset_pd->gpio_mic_gnd))) {
		status =
		    gpio_request_one(mic->headset_pd->gpio_mic_gnd,
				     GPIOF_DIR_OUT | GPIOF_INIT_LOW,
				     "HEADSET_MIC_GND");
		if (status < 0) {
			pr_err("%s: failed to get MIC to GND gpio %d\n",
			       __func__, mic->headset_pd->gpio_mic_gnd);
			return status;
		}
	}

	pr_info("headset_hw_init: gpio config done \r\n");

	/* Configure AUDIOH CCU for clock policy */
	/*
	 * Remove the entire Audio CCU config policy settings and AUDIOH
	 *initialization sequence once they are being done as a part of PMU and
	 *audio driver settings
	 */
#ifdef CONFIG_HS_PERFORM_AUDIOH_SETTINGS
	writel(0xa5a501, KONA_HUB_CLK_VA);
	writel(0x6060606,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY1_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY2_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY3_MASK1_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY1_MASK2_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY2_MASK2_OFFSET);
	writel(0x7fffffff,
	       KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY3_MASK2_OFFSET);
	writel(0x3, KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET);
	writel(0x1ff, KONA_HUB_CLK_VA + KHUB_CLK_MGR_REG_AUDIOH_CLKGATE_OFFSET);
	writel(0x100A00, KONA_HUB_CLK_VA + 0x104);
	writel(0x40000100, KONA_HUB_CLK_VA + 0x124);
#endif
	aci_interface_init_micbias_off(mic);

	return status;
}

static int hs_remove(struct platform_device *pdev)
{
	struct mic_t *mic;

	mic = platform_get_drvdata(pdev);

	free_irq(mic->hsirq, mic);
	free_irq(mic->hsbirq_press, mic);
	free_irq(mic->hsbirq_release, mic);

	if ((mic->headset_pd->gpio_mic_gnd)
	    && (gpio_is_valid(mic->headset_pd->gpio_mic_gnd)))
		gpio_free(mic->headset_pd->gpio_mic_gnd);

	hs_unreginputdev(mic);
	hs_unregswitchdev(mic);

	kfree(mic);
	return 0;
}

static int __init hs_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *mem_resource;
	struct mic_t *mic;

	mic = kzalloc(sizeof(struct mic_t), GFP_KERNEL);
	if (!mic)
		return -ENOMEM;

	mic_dev = mic;

	if (pdev->dev.platform_data)
		mic->headset_pd = pdev->dev.platform_data;
	else {
		/* The driver depends on the platform data (board specific)
		 *information to know two things
		 * 1) The GPIO state that determines accessory
		 *    insertion (HIGH or LOW)
		 * 2) The resistor value put on the MIC_IN line.
		 *
		 * So if the platform data is not present, do not proceed.
		 */
		pr_err("Platform data not present, could not proceed \r\n");
		return -EINVAL;
	}

	/*
	 * Assume that mic bias is ON, so that while initialization we can
	 * turn this OFF and put it in known state.
	 */
	mic->mic_bias_status = 1;

	/* Initialize the switch dev for headset */
#ifdef CONFIG_SWITCH
	ret = hs_switchinit(mic);
	if (ret < 0)
		goto err2;
#endif

	/* Initialize a input device for the headset button */
	ret = hs_inputdev(mic);
	if (ret < 0) {
		hs_unregswitchdev(mic);
		goto err2;
	}

	/* Get the Headset and HS button IRQ */

	/* Insertion detection irq */
	mic->hsirq = platform_get_irq(pdev, 0);
	if (!mic->hsirq) {
		ret = -EINVAL;
		goto err1;
	}
	pr_info("HS irq %d\n", mic->hsirq);

	/* Button press irq */
	mic->hsbirq_press = platform_get_irq(pdev, 1);
	if (!mic->hsbirq_press) {
		ret = -EINVAL;
		goto err1;
	}
	pr_info("HSB press irq %d\n", mic->hsbirq_press);

	/* Button release irq */
	mic->hsbirq_release = platform_get_irq(pdev, 2);
	if (!mic->hsbirq_release) {
		ret = -EINVAL;
		goto err1;
	}
	pr_info("HSB release irq %d\n", mic->hsbirq_release);

	/* Get the base address for AUXMIC and ACI control registers */
	mem_resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_resource) {
		ret = -ENXIO;
		goto err1;
	}
	mic->auxmic_base = HW_IO_PHYS_TO_VIRT(mem_resource->start);

	pr_info("auxmic base is 0x%x\n", mic->auxmic_base);

	mem_resource = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem_resource) {
		ret = -ENXIO;
		goto err1;
	}
	mic->aci_base = HW_IO_PHYS_TO_VIRT(mem_resource->start);

	pr_info("aci base is 0x%x\n", mic->aci_base);

	/* Perform CHAL initialization */
	mic->aci_chal_hdl = chal_aci_init(mic->aci_base);

	/* Hardware initialization */
	ret = headset_hw_init(mic);
	if (ret < 0)
		goto err1;

	pr_info("Headset HW init done \r\n");

#ifdef DEBUG
	dump_hw_regs(mic);
#endif

	/*
	 * Please note that all the HS accessory interrupts
	 *should be requested with _NO_SUSPEND option because even if
	 *the system goes to suspend we want this interrupts to be active
	 */

	/* Request the IRQ for accessory insertion detection */
	ret =
	    request_irq(mic->hsirq, gpio_isr,
			(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			 IRQF_NO_SUSPEND), "headset_detect", mic);
	if (ret < 0) {
		pr_err("%s(): request_irq() failed for headset %s: %d\n",
		       __func__, "irq", ret);
		goto err1;
	}

	/* Request the IRQ for HS Button press */
	ret =
	    request_irq(mic->hsbirq_press, comp2_isr, IRQF_NO_SUSPEND,
			"aci_hs_button_press", mic);
	if (ret < 0) {
		pr_err("%s(): request_irq() failed for headset %s: %d\n",
		       __func__, "button press", ret);
		/* Free the HS IRQ if the HS Button IRQ request fails */
		free_irq(mic->hsirq, mic);
		goto err1;
	}

	mic->hs_state = DISCONNECTED;
	mic->button_state = BUTTON_RELEASED;

	/* Store the mic structure data as private driver data for later use */
	platform_set_drvdata(pdev, mic);

	/*
	 * Its important to understand why we schedule the accessory detection
	 *work queue from here.
	 *
	 * From the schematics the GPIO status should be
	 * 1 - nothing iserted
	 * 0 - accessory inserted
	 *
	 * The pull up for the GPIO is connecte to 1.8 V that is source by the
	 * PMU. But the PM chip's init happens after headset insertion, so
	 *reading the GPIO value during init may not give us the correct
	 *status (will read 0 always). Later after the init when the GPIO
	 *gets the 1.8 V an interrupt would be triggered for rising edge and
	 *the GPIO ISR would schedule the work queue. But if the accessory is
	 *kept connected assuming the PMU to trigger the ISR is like taking a
	 *chance. Also, if for some reason PMU init is moved before head set
	 *driver init then the GPIO state would not change after headset
	 *driver init and the GPIO interrupt may not be triggered.
	 * Its safe to schedule detection work here becuase
	 *during bootup, irrespective of the GPIO interrupt we'll detect the
	 *accessory type. (Even if the interrupt occurs no harm done since
	 *the work queue will be any way executed only once).
	 */
	schedule_delayed_work(&(mic->accessory_detect_work),
			      ACCESSORY_INSERTION_REMOVE_SETTLE_TIME);

	return ret;
err1:
	hs_unregswitchdev(mic);
	hs_unreginputdev(mic);
err2:
	kfree(mic);
	return ret;
}

/*
 *Note that there is a __refdata added to the headset_driver platform driver
 *structure. What is the meaning for it and why its required.
 *
 *The probe function:
 *From the platform driver documentation its advisable to keep the probe
 *function of a driver in the __init section if the device is NOT hot pluggable.
 *Note that in headset case even though the headset is hot pluggable, the driver
 *is not. That is a new node will not be created and the probe will not be
 *called again. So it makes sense to keep the hs_probe in __init section so as
 *to reduce the driver's run time foot print.
 *
 *The Warning message:
 *But since the functions address (reference) is stored in a structure that
 *will be available even after init (in case of remove, suspend etc) there
 *is a Warning message from the compiler
 *
 *The __refdata keyword can be used to suppress this warning message. Tells the
 *compiler not to throw out this warning. And in this scenario even though
 *we store the function pointer from __init section to the platform driver
 *structure that lives after __init, we wont be referring the probe function
 *in the life time until headset_driver lives, so its OK to suppress.
 */
static struct platform_driver __refdata headset_driver = {
	.probe = hs_probe,
	.remove = hs_remove,
	.driver = {
		   .name = "konaaciheadset",
		   .owner = THIS_MODULE,
		   },
};

int switch_bias_voltage(int mic_status)
{

	switch (mic_status) {
	case 1:
	/*Mic will be used. Boost voltage */
			/* Set the threshold value for button press */
		pr_info("Setting Bias to 2.1V\r\n");
		aci_mic_bias_high.mode = CHAL_ACI_MIC_BIAS_ON;
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
		    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
		    CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias_high);
		/* Power up Digital block */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ENABLE,
				    CHAL_ACI_BLOCK_DIGITAL);

		/* Power up the ADC */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ENABLE,
				    CHAL_ACI_BLOCK_ADC);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
				    CHAL_ACI_BLOCK_ADC,
				    CHAL_ACI_BLOCK_ADC_HIGH_VOLTAGE);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
				CHAL_ACI_BLOCK_ADC, &aci_filter_adc_config);
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
				CHAL_ACI_BLOCK_COMP2,
				button_voltage_range[1]);
		break;
	case 0:
		pr_info("Setting Bias to 0.45V \r\n");
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				    CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
				    CHAL_ACI_BLOCK_ADC,
				    CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE);
		aci_mic_bias_low.mode =
				CHAL_ACI_MIC_BIAS_DISCONTINUOUS;
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
				CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias_low);
			__low_power_mode_config();
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
				CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
				CHAL_ACI_BLOCK_COMP2,
				button_voltage_range[0]);
	break;

	default:
	break;
	}
	return 0;
}

static void __low_power_mode_config(void)
{
		/*Use this for lowest
		 * current consumption & button
		 * functionality in 0.45V mode
		 *
		 * Offset 0x28: Disable weak sleep mode
		 * Offset 0xC4: Impedance set to low
		 */
		writel(0, mic_dev->aci_base + 0xD8);
		writel(1, mic_dev->aci_base + 0xc4);
}

#ifdef DEBUG

struct kobject *hs_kobj;

static ssize_t
hs_regdump_func(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	dump_hw_regs(mic_dev);
	return n;
}

static ssize_t
hs_regwrite_func(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t n)
{
	unsigned int reg_off;
	unsigned int val;

	if (sscanf(buf, "%x %x", &reg_off, &val) != 2) {
		pr_info
		    ("Usage: echo reg_offset value > /sys/hs_debug/hs_regwrite \r\n");
		return n;
	}
	pr_info("Writing 0x%x to Address 0x%x \r\n", val,
		mic_dev->aci_base + reg_off);
	writel(val, mic_dev->aci_base + reg_off);
	return n;
}

static ssize_t
hs_config_amp_func(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t n)
{
	unsigned int val;
	int gain;

	if (sscanf(buf, "%d", &val) != 1) {
		pr_info("Usage: echo [1/0] > /sys/hs_debug/hs_config_amp \r\n");
		return n;
	}

	gain = (val == 1) ? PMU_HSGAIN_66DB_N : PMU_HSGAIN_MUTE;
	bcmpmu_hs_power((void *)val);
	bcmpmu_hs_set_gain(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE);
	msleep(50);

	return n;
}

static ssize_t
hs_read_adc_func(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t n)
{
	int val;
	int adc_range;

	if (sscanf(buf, "%d", &val) != 1) {
		pr_info
		    ("Usage: echo [1(full range)/0(low voltage)] > /sys/hs_debug/hs_read_adc \r\n");
		return n;
	}

	/* Setup MIC bias */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			    CHAL_ACI_BLOCK_GENERIC,
				&aci_mic_bias_high);
	/* TODO: Setup the Interrupt Source, is this required ??? */

	/* Power up Digital block */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_ENABLE,
			    CHAL_ACI_BLOCK_DIGITAL);

	/* Power up the ADC */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_ENABLE, CHAL_ACI_BLOCK_ADC);

	adc_range =
	    (val == 0) ? CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE :
	    CHAL_ACI_BLOCK_ADC_HIGH_VOLTAGE;

	pr_info("Configuring ADC range for %d \r\n", adc_range);
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl,
			    CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
			    CHAL_ACI_BLOCK_ADC, adc_range);
	msleep(80);

	val = chal_aci_block_read(mic_dev->aci_chal_hdl,
				  CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_RAW);

	pr_info("Value read from ADC %d \r\n", val);
	return n;
}

static ssize_t
hs_switch_func(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t n)
{
	int val;

	if (sscanf(buf, "%d", &val) != 1) {
		pr_info
		    ("Usage: echo [1(high)/2(low)]"
				"> /sys/hs_debug/hs_switch \r\n");
		return n;
	}
	switch_bias_voltage(val);
	return n;
}

static DEVICE_ATTR(hs_regdump, 0666, NULL, hs_regdump_func);
static DEVICE_ATTR(hs_regwrite, 0666, NULL, hs_regwrite_func);
static DEVICE_ATTR(hs_config_amp, 0666, NULL, hs_config_amp_func);
static DEVICE_ATTR(hs_read_adc, 0666, NULL, hs_read_adc_func);
static DEVICE_ATTR(hs_switch, 0666, NULL, hs_switch_func);

static struct attribute *hs_attrs[] = {
	&dev_attr_hs_regdump.attr,
	&dev_attr_hs_regwrite.attr,
	&dev_attr_hs_config_amp.attr,
	&dev_attr_hs_read_adc.attr,
	&dev_attr_hs_switch.attr,
	NULL,
};

static struct attribute_group hs_attr_group = {
	.attrs = hs_attrs,
};

static int __init hs_sysfs_init(void)
{
	hs_kobj = kobject_create_and_add("hs_debug", NULL);
	if (!hs_kobj)
		return -ENOMEM;
	return sysfs_create_group(hs_kobj, &hs_attr_group);
}

static void __exit hs_sysfs_exit(void)
{
	sysfs_remove_group(hs_kobj, &hs_attr_group);
}
#endif

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_init
    Description     : Initialize the driver
    Return type     : int
------------------------------------------------------------------------------*/
int __init kona_aci_hs_module_init(void)
{
	pr_debug("\n\nkona_aci_hs_module_init\n");
#ifdef DEBUG
	hs_sysfs_init();
#endif
	return platform_driver_register(&headset_driver);
}

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_exit
    Description     : clean up
    Return type     : int
------------------------------------------------------------------------------*/
void __exit kona_aci_hs_module_exit(void)
{
#ifdef DEBUG
	hs_sysfs_exit();
#endif
	return platform_driver_unregister(&headset_driver);
}

module_init(kona_aci_hs_module_init);
module_exit(kona_aci_hs_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Headset plug and button detection");

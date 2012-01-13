/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/input/misc/brcm_headset.c
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
/* #define DEBUG */

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
#include <mach/io_map.h>
#include <mach/kona_headset_pd.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb/brcm_rdb_auxmic.h>
#include <mach/rdb/brcm_rdb_audioh.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>

#include <plat/chal/chal_aci.h>

/*
 * The gpio_set_debounce expects the debounce argument in micro seconds
 * Previously the kona implementation which is called from gpio_set_debounce
 * was expecting the argument as milliseconds which was incorrect. 
 * The commit 38598aea6cb57290ef6ed3518b75b9f398b7694f fixed it. Hence we have to 
 * change to the correct way of passing the time in microseconds resolution.
 */
#define DEBOUNCE_TIME	(64000)
#define KEY_PRESS_REF_TIME	msecs_to_jiffies(100)
#define KEY_DETECT_DELAY	msecs_to_jiffies(128)
#define ACCESSORY_INSERTION_REMOVE_SETTLE_TIME 	msecs_to_jiffies(500)

/*
 * After configuring the ADC, it takes different 'time' for the 
 * ADC to settle depending on the HS type. The time outs are 
 * in milli seconds
 */

#define DET_PLUG_INSERTION_SETTLE       250     /* Plug insertion setteling time */
#define DET_PLUG_CONNECTED_SETTLE       80      /* in HAL_ACC_DET_PLUG_CONNECTED */
#define DET_HEADPHONE_SETTLE            5       /* in HAL_ACC_DET_HEADPHONE */
#define DET_OPEN_CABLE_SETTLE           20      /* in HAL_ACC_DET_OPEN_CABLE */
#define DET_HEADSET_SETTLE         	20      /* in HAL_ACC_DET_BASIC_CARKIT */
#define FINAL_HEADSET_SETTLE            40      /* Headset mode */
#define FINAL_OPENCABLE_SETTLE          40      /* Open Cable mode */

/*
 * Button/Hook Filter configuration
 */
#define ACC_HW_COMP1_FILTER_WIDTH   1024    /* = 1024 / (Filter block frequencey) = 1024 / 32768 => 31ms */

/* 
 * Accessory Detecting voltage
 *
 * Voltage defined in mv 
 */
#define HEADPHONE_DETECT_LEVEL_MIN      0
#define HEADPHONE_DETECT_LEVEL_MAX      40
#define HEADPHONE_DETECT_LEVEL2_MIN     91
#define HEADPHONE_DETECT_LEVEL2_MAX     599

#define OPENCABLE_DETECT_LEVEL_MIN      1900
#define OPENCABLE_DETECT_LEVEL_MAX      5000

#define BASIC_HEADSET_DETECT_LEVEL_MIN  0
#define BASIC_HEADSET_DETECT_LEVEL_MAX  200

struct mic_t {
	int gpio_irq;
	int hsbirq_press;
	int hsbirq_release;
	int comp1_irq;
	int auxmic_base;
	int aci_base;
	CHAL_HANDLE aci_chal_hdl;
	struct kona_headset_pd *headset_pd;
#ifdef CONFIG_SWITCH
	struct switch_dev sdev;
#endif
	struct delayed_work accessory_detect_work;
	struct delayed_work button_press_work;
	struct delayed_work button_release_work;
	struct input_dev *headset_button_idev;
	int hs_state;
	int button_state;
	int button_pressed;
};

static struct mic_t *mic_dev = NULL;

enum hs_type {
	DISCONNECTED = 0, /* Nothing is connected  */
	HEADPHONE = 1,    /* The one without MIC   */
	OPEN_CABLE,	  /* Not sent to userland  */
	HEADSET,	  /* The one with MIC 	   */
	/* If more HS types are required to be added
	 * add here, not below HS_TYPE_MAX
	 */
	HS_TYPE_MAX,
};

enum button_name {
	BUTTON_SEND_END,
	BUTTON_VOLUME_UP,
	BUTTON_VOLUME_DOWN,
	BUTTON_NAME_MAX
};

enum button_state {
	BUTTON_RELEASED = 0,
	BUTTON_PRESSED
};

/*
 * Default table used if the platform does not pass one
 */ 
static unsigned int button_adc_values_no_resistor [3][2] = 
{
	/* SEND/END Min, Max*/
	{0,	104},
	/* Volume Up  Min, Max*/
	{139,	270},
	/* Volue Down Min, Max*/
	{330,	680},
};

/* Accessory Hardware configuration support variables */
/* ADC */
static const CHAL_ACI_filter_config_adc_t aci_filter_adc_config = {0, 0x0B};

static const CHAL_ACI_filter_config_comp_t comp_values_for_button_press = {
	CHAL_ACI_FILTER_MODE_INTEGRATE,
	CHAL_ACI_FILTER_RESET_FIRMWARE,
	0,       /* = S */
	0xFE,    /* = T */
	0x500,   /* = M = 1280 / 32768 => 39ms */
	ACC_HW_COMP1_FILTER_WIDTH /* = MT */
};

/* COMP2 */
static const CHAL_ACI_filter_config_comp_t comp_values_for_type_det = {
	CHAL_ACI_FILTER_MODE_INTEGRATE,
	CHAL_ACI_FILTER_RESET_FIRMWARE,
	0,     /* = S  */
	0xFE,  /* = T  */
	0x700, /* = M  */
	0x650  /* = MT */
};

/* MIC bias */
static CHAL_ACI_micbias_config_t aci_mic_bias = {
	CHAL_ACI_MIC_BIAS_OFF,
	CHAL_ACI_MIC_BIAS_2_1V,
	CHAL_ACI_MIC_BIAS_PRB_CYC_256MS,
	CHAL_ACI_MIC_BIAS_MSR_DLY_4MS,
	CHAL_ACI_MIC_BIAS_MSR_INTVL_64MS,
	CHAL_ACI_MIC_BIAS_1_MEASUREMENT
};

static CHAL_ACI_micbias_config_t aci_init_mic_bias = {
		CHAL_ACI_MIC_BIAS_ON,
		CHAL_ACI_MIC_BIAS_2_1V,
		CHAL_ACI_MIC_BIAS_PRB_CYC_128MS,
		CHAL_ACI_MIC_BIAS_MSR_DLY_32MS,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_128MS,
		CHAL_ACI_MIC_BIAS_1_MEASUREMENT
	};

/* Vref */
static CHAL_ACI_vref_config_t aci_vref_config = {CHAL_ACI_VREF_OFF};

static int __headset_hw_init (struct mic_t *mic);

/* Function to dump the HW regs */
#ifdef DEBUG
static void dump_hw_regs (struct mic_t *p)
{
	int i;

	printk ("\r\n Dumping MIC BIAS registers \r\n");
	for (i=0x0; i <=0x28; i+= 0x04){
		printk("Addr: 0x%x  OFFSET: 0x%x  Value:0x%x \r\n",p->auxmic_base+i,i,readl(p->auxmic_base+i)); 
	}

	printk ("\r\n \r\n");
	printk ("Dumping ACI registers \r\n");
	for (i=0x30; i <=0xD8; i+= 0x04){
		printk("Addr: 0x%x  OFFSET: 0x%x  Value:0x%x \r\n",p->aci_base+i,i,readl(p->aci_base+i)); 
	}
		
	for (i=0x400; i <=0x420; i+= 0x04){
		printk("Addr: 0x%x  OFFSET: 0x%x Value:0x%x \r\n",p->aci_base+i,i,readl(p->aci_base+i)); 
	}
	printk ("\r\n \r\n");
}
#endif

/* Low level functions called to detect the accessory type */
static int config_adc_for_accessory_detection(int hst)
{
	int time_to_settle = 0;

	if (mic_dev == NULL) {
		pr_err("%s():Invalid mic_dev handle \r\n", __func__ );
		return  -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("%s():Invalid CHAL handle \r\n", __func__);
		return -EFAULT;
	}

	switch (hst) {
	case HEADPHONE:

		pr_debug("config_adc_for_accessory_detection: Configuring for headphone \r\n");
		/* Setup MIC bias */
		aci_mic_bias.mode = CHAL_ACI_MIC_BIAS_ON;
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias);
		/* TODO: Setup the Interrupt Source, is this required ??? */

		/* Power up Digital block */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE, 
			CHAL_ACI_BLOCK_DIGITAL);

		/* Power up the ADC */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE, 
			CHAL_ACI_BLOCK_ADC);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
			CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
			CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_ADC,
			&aci_filter_adc_config);

		time_to_settle = DET_PLUG_CONNECTED_SETTLE;
		break;

	case OPEN_CABLE:

		pr_debug("config_adc_for_accessory_detection: Configuring for open cable \r\n");

		/* Powerup ADC */
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE,
			CHAL_ACI_BLOCK_ADC);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
			CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_HIGH_VOLTAGE);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
			CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_ADC,
			&aci_filter_adc_config);

		time_to_settle = DET_OPEN_CABLE_SETTLE;
		break;

	case HEADSET:

		pr_debug("config_adc_for_accessory_detection: Configuring for HEADSET \r\n");

		/* Turn OFF MIC Bias */
		aci_mic_bias.mode = CHAL_ACI_MIC_BIAS_GND;
		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
			CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias);

		chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
			CHAL_ACI_BLOCK_ACTION_MIC_POWERDOWN_HIZ_IMPEDANCE,
			CHAL_ACI_BLOCK_GENERIC,
			0);

		time_to_settle = DET_HEADSET_SETTLE;
		break;

	default:
		/* TODO: Logically what does this mean ???? */
		break;
	}

	/* 
	 * Wait till the ADC settles, the timings varies for different types
	 * of headset. 
	 */
	/* TODO:
	 * There is a settling time after which a call back is
	 * invoked, does this call back trigger any other HW config or
	 * its just a notification to the upper layer ????
	 */
	if (time_to_settle != 0)
		msleep(time_to_settle);

	return 0;
}

static int config_adc_for_bp_detection(void)
{
	if (mic_dev == NULL) {
		pr_err("%s(): invalid mic_dev handle \r\n", __func__);
		return  -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("%s(): Invalid CHAL handle \r\n", __func__);
		return -EFAULT;
	}

	/* 
	 * TODO: As of now this function uses the same MIC BIAS settings
	 * and filter settings as accessory detection, this might need
	 * fine tuning. 
	 */
	pr_debug("Configuring ADC for button press detection \r\n");

	/* Setup MIC bias */
	aci_mic_bias.mode = CHAL_ACI_MIC_BIAS_ON;
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
		CHAL_ACI_BLOCK_GENERIC, &aci_mic_bias);

	/* Power up Digital block */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE, 
		CHAL_ACI_BLOCK_DIGITAL);

	/* Power up the ADC */
	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ENABLE, 
		CHAL_ACI_BLOCK_ADC);

	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
		CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE);

	chal_aci_block_ctrl(mic_dev->aci_chal_hdl, 
		CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_ADC,
		&aci_filter_adc_config);

	/* 
	 * Wait till the ADC settles, the timings might need fine tuning
	 */
	msleep(20);

	return 0;
}


static int read_adc_for_accessory_detection(int hst)
{
	int status = -1;
	int mic_level;

	if (mic_dev == NULL) {
		pr_err("aci_adc_config: invalid mic_dev handle \r\n");
		return  -EFAULT;
	}

	if (mic_dev->aci_chal_hdl == NULL) {
		pr_err("aci_adc_config: Invalid CHAL handle \r\n");
		return -EFAULT;
	}

	/*
	 * What is phone_ref_offset?
	 *
	 * Because of the resistor in the MIC IN line the actual ground is not 0,
	 * but a small offset is added to it. We call this as
	 * phone_ref_offset.
	 * This needs to be subtracted from the measured voltage to determine
	 * the correct value. This will vary for different HW based on the
	 * resistor values used. So by default this variable is 0, if no one
	 * initializes it. For boards on which this resistor is present this
	 * value should be passed from the board specific data structure
	 *
	 * In the below logic, if mic_level read is less than or equal to 0
	 * then we don't do anything.
	 * If the read value is greater than  phone_ref_offset then subtract this offset
	 * from the value read, otherwise mic_level is zero
	 */
	mic_level = chal_aci_block_read(mic_dev->aci_chal_hdl,
		CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_RAW);
	pr_debug(" ++ read_adc_for_accessory_detection: mic_level before calc %d \r\n", mic_level);
	mic_level = mic_level <= 0 ? mic_level :
		((mic_level > mic_dev->headset_pd->phone_ref_offset) ?
		(mic_level - mic_dev->headset_pd->phone_ref_offset) : 0);
	pr_debug(" ++ read_adc_for_accessory_detection: mic_level after calc %d \r\n", mic_level);

	switch (hst) {
	case HEADPHONE:
		if (( mic_level >= HEADPHONE_DETECT_LEVEL_MIN && 
		      mic_level <= HEADPHONE_DETECT_LEVEL_MAX)
		    ||
		    (mic_level >= HEADPHONE_DETECT_LEVEL2_MIN && 
		     mic_level <= HEADPHONE_DETECT_LEVEL2_MAX))
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

	if (mic_dev == NULL) {
		pr_err("mic_dev is empty \r\n");
		return 0;
	}

	/* 
	 * Configure the ACI block of ADC for detecting a particular type of
	 * accessory
	 */
	for (i=HEADPHONE;i<HS_TYPE_MAX;i++) {
		/* Configure the ADC to check a given HS type */
		config_adc_for_accessory_detection(i);

		/* 
		 * Now, read back to check whether the given accessory is
		 * present. If yes, then break the loop. If not, continue
		 */
		if ( (type=read_adc_for_accessory_detection(i)) == i) {
			break;
		} 

	} /* end of loop to check the devices */

	return type;
}


int detect_button_pressed (struct mic_t *mic_dev)
{
	int i;
	int mic_level;

	if (mic_dev == NULL) {
		pr_err("mic_dev is empty \r\n");
		return 0;
	}

	/*
	 * What is phone_ref_offset?
	 *
	 * Because of the resistor in the MIC IN line the actual ground is not 0,
	 * but a small offset is added to it. We call this as
	 * phone_ref_offset.
	 * This needs to be subtracted from the measured voltage to determine
	 * the correct value. This will vary for different HW based on the
	 * resistor values used. So by default this variable is 0, if no one
	 * initializes it. For boards on which this resistor is present this
	 * value should be passed from the board specific data structure
	 *
	 * In the below logic, if mic_level read is less than or equal to 0
	 * then we don't do anything.
	 * If the read value is greater than  phone_ref_offset then subtract this offset
	 * from the value read, otherwise mic_level is zero
	 */
	mic_level = chal_aci_block_read(mic_dev->aci_chal_hdl,
		CHAL_ACI_BLOCK_ADC, CHAL_ACI_BLOCK_ADC_RAW);
	pr_debug("%s(): mic_level before calc %d \r\n", __func__, mic_level);
	mic_level = mic_level <= 0 ? mic_level :
		((mic_level > mic_dev->headset_pd->phone_ref_offset) ?
		(mic_level - mic_dev->headset_pd->phone_ref_offset) : 0);
	pr_debug("%s():mic_level after calc %d \r\n", __func__, mic_level);


	/* Find out what is the button pressed */

	/* Take the table based on what is passed from the board */
	for (i=BUTTON_SEND_END;i<BUTTON_NAME_MAX;i++) {
		if((mic_level >= mic_dev->headset_pd->button_adc_values[i][0]) &
		   (mic_level <= mic_dev->headset_pd->button_adc_values[i][1])) 
			break;
	}

	return i; 
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
	int err = 0;
	int button_name;

	/* 
	 * Just to be sure check whether this is happened when the
	 * Headset accessory is inserted,
	 * If its triggered when the connected accessory is NOT a headset
	 * _OR_ when none of the accessory is conneceted its spurious
	 *
	 */
#if 0
	if ((p->hs_state == HEADSET) && (p->button_state == BUTTON_RELEASED)) {
		p->button_state = BUTTON_PRESSED;
		pr_info(" Sending Key Press\r\n");
		input_report_key(p->headset_button_idev, KEY_SEND, 1);
		input_sync(p->headset_button_idev);
	}
#endif

	if (p->hs_state == HEADSET) {
		/* Find out the type of button pressed by reading the ADC values */
		button_name = detect_button_pressed(p);

		/* 
	 	 * Store which button is being pressed (KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_SEND) 
	 	 * in the context structure 
	 	 */
	 	switch (button_name) {
			case BUTTON_SEND_END:
				p->button_pressed = KEY_SEND;
				break;
			case BUTTON_VOLUME_UP:
				p->button_pressed = KEY_VOLUMEUP;
				break;
			case BUTTON_VOLUME_DOWN:
				p->button_pressed = KEY_VOLUMEDOWN;
				break;
			default:
				pr_err("Button type not supported \r\n");
				err = 1;
				break;
	 	}

		if (err)
			goto out;

		/* Notify the same to input sub-system */
		p->button_state = BUTTON_PRESSED;
		pr_info(" Sending Key Press\r\n");
		pr_info("\n Button pressed =%d \n", button_name);
		input_report_key(p->headset_button_idev, p->button_pressed, 1);
		input_sync(p->headset_button_idev);
	} else {
		pr_err("Button press work scheduled when the accessory type is NOT Headset .. spurious \r\n");
	}
out:
	return;
}

static void button_release_work_func (struct work_struct *work)
{
	struct mic_t *p = container_of(work, struct mic_t,
						button_release_work.work);

	/* 
	 * Just to be sure check whether this is happened when the
	 * Headset accessory is inserted,
	 * If its triggered when the connected accessory is NOT a headset
	 * _OR_ when none of the accessory is conneceted its spurious
	 *
	 */
#if 0
	if ((p->hs_state == HEADSET) && (p->button_state == BUTTON_PRESSED)) {
		p->button_state = BUTTON_RELEASED;
		pr_info(" Sending Key Release\r\n");
		input_report_key(p->headset_button_idev, KEY_SEND, 0);
		input_sync(p->headset_button_idev);
	}
#endif

	/* Do the following only for headset */
	if (p->hs_state == HEADSET) {
		/* 
	 	 * Find out which button is being pressed from the context structure 
	 	 * Notify the corresponding release event to the input sub-system
	 	 */
		p->button_state = BUTTON_RELEASED;
		pr_info(" Sending Key Release\r\n");
		input_report_key(p->headset_button_idev, p->button_pressed, 0);
		input_sync(p->headset_button_idev);
	}else {
		pr_err("Button release work scheduled when the accessory type is NOT Headset .. spurious \r\n");
	}

	return;
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
	unsigned int accessory_inserted;

	/*
	 * If the platform supports GPIO for accessory insertion/removal, use
	 * the state of the GPIO to decide whether accessory is inserted or
	 * removed
	 */
	if (p->headset_pd->gpio_for_accessory_detection == 1) {
		accessory_inserted = gpio_get_value(irq_to_gpio(p->gpio_irq));

		pr_debug("SWITCH WORK GPIO STATE: 0x%x default state 0x%x \r\n", accessory_inserted,  p->headset_pd->hs_default_state); 

		accessory_inserted = (accessory_inserted ^ p->headset_pd->hs_default_state);
	} else {
		/*
	 	 * If not, i.e if COMP1 interrupt is used for accessory
	 	 * insertion/removal detection, then there is no way to know why this
	 	 * interrupt is caused, use software state to decide. That is, the
	 	 * hs_sate is initialized to DISCONNECTED. So, if the interrupt
	 	 * arrives now the accessory is inserted, otheriwse the accessory is
	 	 * disconnected
	 	 */
		 accessory_inserted = (p->hs_state == DISCONNECTED)?1:0;
	}

	if (accessory_inserted == 1) {
		pr_info(" ACCESSORY INSERTED \r\n");
		pr_debug("0. Interrupt status before detecting hs_type 0x%x \r\n",
			readl(p->aci_base + ACI_INT_OFFSET));

		p->hs_state = detect_hs_type(p);
		
		switch(p->hs_state) {

		case OPEN_CABLE:
			/* Configure the COMP1 threshold for accessory detection*/
			pr_debug("Detected Open cable enabling reconfig threshold values of COMP1\r\n" );

			/* Configure the comparator 1 for type detection */
			chal_aci_block_ctrl(p->aci_chal_hdl,
				CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_COMP1,
				&comp_values_for_type_det);

			/* Set the threshold value for type detection */
			chal_aci_block_ctrl(p->aci_chal_hdl,
				CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
				CHAL_ACI_BLOCK_COMP1,
				1900);

			/* No notification needed for the userland */

#ifdef DEBUG
			printk("Configured for Open cable \r\n");
			dump_hw_regs(p);
#endif
			/* Clear pending interrupts if any */
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
				CHAL_ACI_BLOCK_COMP);

			/* Enable COMP1 interrupt for accessory detection */
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
				CHAL_ACI_BLOCK_COMP1);
			break;

		case HEADSET:
			pr_debug("accessory_detect_work_func: Detected headset config for button press \r\n");

			/* Put back the aci interface to be able to detect the
			 * button press. Especially this functions puts the
			 * COMP2 and MIC BIAS values to be able to detect
			 * button press
			 */
			p->button_state = BUTTON_RELEASED;
			__headset_hw_init (p);

			/*
			 * A settling time is required here. The call to
			 * __headset_hw_init invokes COMP2 & COMP2 INV
			 * interrupts. Note that some times the interrupt gets
			 * triggered after some time say 't'. If this 't'
			 * happens before the next call to clear spurious
			 * interrupts everything is OK, but if not it will
			 * trigger unwanted button press, release events. So
			 * its better to allow the work to wait until the
			 * settling time and then clear the interrupts
			 */
			msleep(DET_HEADSET_SETTLE);

			/* Clear pending interrupts if any */
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
				CHAL_ACI_BLOCK_COMP);

#ifdef DEBUG
			pr_info("After configuring the ACI interface for button press \r\n");
			dump_hw_regs(p);
#endif

			/* Enable COMP2 interrupt for button press */
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
				CHAL_ACI_BLOCK_COMP2);

			/* Enable COMP2 INV interrupt for button release  */
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
				CHAL_ACI_BLOCK_COMP2_INV);

			/* Configure the ADC to read button press values */
			config_adc_for_bp_detection();

			/* Fall through to send the update to userland */
		case HEADPHONE:

			/* Clear pending interrupts if any */
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
				CHAL_ACI_BLOCK_COMP);

			/* No need to enable the button press/release irq */ 
#ifdef CONFIG_SWITCH
			/*
	 		 * While notifying this to the androind world we need to pass
	 		 * the accessory typ as Androind understands. The Android
	 		 * 'state' variable is a bitmap as defined below.
	 		 * BIT 0 : 1 - Headset (with MIC) connected
	 		 * BIT 1 : 1 - Headphone (the one without MIC) is connected
	 		 */
			switch_set_state(&(p->sdev), (p->hs_state==HEADSET)?1:2);
#endif

			/* 
			 * If GPIO is not used for accessory
			 * insertion/removal, enable COMP1 Interrupts for 
			 * accessory removal 
			 */
			if (p->headset_pd->gpio_for_accessory_detection == 1) {
				chal_aci_block_ctrl(p->aci_chal_hdl,	
					CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
					CHAL_ACI_BLOCK_COMP1);
			}

			break;
		default:
			pr_err("%s():Unknown accessory type %d \r\n",__func__, p->hs_state);
			break;
		}

		pr_debug("Interrupt status after detecting hs_type 0x%x \r\n",
			readl(p->aci_base + ACI_INT_OFFSET));
		
	} else {
		pr_info(" ACCESSORY REMOVED \r\n");
		if(p->hs_state == DISCONNECTED ) {
			pr_err("Accessory removed spurious event \r\n");
		} else {
			/* Inform userland about accessory removal */ 
			p->hs_state = DISCONNECTED; 
			p->button_state = BUTTON_RELEASED;
#ifdef CONFIG_SWITCH
			switch_set_state(&(p->sdev),p->hs_state);
#endif
			/* Clear pending interrupts */		
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
				CHAL_ACI_BLOCK_COMP);

			/* Disable the COMP2, COMP2 INV interrupt */
			chal_aci_block_ctrl(p->aci_chal_hdl,	
				CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
				CHAL_ACI_BLOCK_COMP);
		}
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

	INIT_DELAYED_WORK(&(p->accessory_detect_work), accessory_detect_work_func);
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
	cancel_delayed_work_sync(&p->button_release_work);
	input_unregister_device(p->headset_button_idev);
	input_free_device(p->headset_button_idev);
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
	 * Since we have only one button on headset,value KEY_SEND is sent */
	set_bit(EV_KEY, p->headset_button_idev->evbit);
	set_bit(KEY_SEND, p->headset_button_idev->keybit);
	set_bit(KEY_VOLUMEDOWN, p->headset_button_idev->keybit);
	set_bit(KEY_VOLUMEUP, p->headset_button_idev->keybit);

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
	INIT_DELAYED_WORK(&(p->button_release_work), button_release_work_func);

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

	pr_debug("HS ISR GPIO STATE: 0x%x \r\n", gpio_get_value(irq_to_gpio(p->gpio_irq)));

	schedule_delayed_work(&(p->accessory_detect_work), ACCESSORY_INSERTION_REMOVE_SETTLE_TIME);

	return IRQ_HANDLED;
}

/*------------------------------------------------------------------------------
    Function name   : comp1_isr
    Description     : interrupt handler
    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t comp1_isr(int irq, void *dev_id)
{
	struct mic_t *p = (struct mic_t *)dev_id;

	pr_debug("COMP1 ISR fired \r\n");

	/* Acknowledge & clear the interrupt */
	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
		CHAL_ACI_BLOCK_COMP1);	

	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
		CHAL_ACI_BLOCK_COMP1);	

	pr_debug("After clearing 0x%x ... \r\n", readl(p->aci_chal_hdl + ACI_INT_OFFSET));

	schedule_delayed_work(&(p->accessory_detect_work), ACCESSORY_INSERTION_REMOVE_SETTLE_TIME);

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

	pr_debug("In COMP 2  ISR 0x%x ... \r\n", readl(p->aci_base + ACI_INT_OFFSET));

	/* Acknowledge & clear the interrupt */
	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
		CHAL_ACI_BLOCK_COMP2);	

	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
		CHAL_ACI_BLOCK_COMP2);	

	pr_debug("After clearing 0x%x ... \r\n", readl(p->aci_chal_hdl + ACI_INT_OFFSET));

#if 0
	if (p->hs_state == HEADSET) {
		pr_debug("button press scheduling button_press_work \r\n"); 

		schedule_delayed_work(&(p->button_press_work), KEY_DETECT_DELAY);

		/* Re-enable COMP2 Interrupts */
		chal_aci_block_ctrl(p->aci_chal_hdl,	
			CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
			CHAL_ACI_BLOCK_COMP2);

	} else if (p->hs_state == OPEN_CABLE){
		pr_debug("scheduling switch work \r\n");	
		schedule_delayed_work(&(p->accessory_detect_work), ACCESSORY_INSERTION_REMOVE_SETTLE_TIME);
	} else {
		pr_debug("Spurious\r\n");

		chal_aci_block_ctrl(p->aci_chal_hdl,	
			CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
			CHAL_ACI_BLOCK_COMP2);
	}
#endif

	if (p->hs_state == HEADSET) {
		pr_debug("button press scheduling button_press_work \r\n"); 

		schedule_delayed_work(&(p->button_press_work), KEY_DETECT_DELAY);

		/* Re-enable COMP2 Interrupts */
		chal_aci_block_ctrl(p->aci_chal_hdl,	
			CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
			CHAL_ACI_BLOCK_COMP2);

	} else {
		pr_debug("Spurious\r\n");

		chal_aci_block_ctrl(p->aci_chal_hdl,	
			CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
			CHAL_ACI_BLOCK_COMP2);
	}

	return IRQ_HANDLED;
}

/*------------------------------------------------------------------------------
    Function name   : comp2_inv_isr
    Description     : interrupt handler
    Return type     : irqreturn_t
------------------------------------------------------------------------------*/
irqreturn_t comp2_inv_isr(int irq, void *dev_id)
{
	struct mic_t *p = (struct mic_t *)dev_id;

	/* Acknowledge & clear the interrupt */
	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
		CHAL_ACI_BLOCK_COMP2_INV);	

	pr_debug("comp2_inv_isr occured \r\n");

	if (p->hs_state == HEADSET) 
		schedule_delayed_work(&(p->button_release_work), KEY_DETECT_DELAY);
	else 
		pr_err("comp2_inv_isr is spurious \r\n");

	/* Re-enable COMP2 Interrupts */
	chal_aci_block_ctrl(p->aci_chal_hdl,	
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
		CHAL_ACI_BLOCK_COMP2_INV);

	return IRQ_HANDLED;
}


static int __headset_hw_init (struct mic_t *p)
{
	if (p == NULL)
		return -1;

/*
 * IMPORTANT
 * ---------
 * Configuring these AUDIOH MIC registers was required to get ACI interrupts
 * for button press. Really not sure about the connection.
 * But this logic was taken from the BLTS code and if this is not
 * done then we were not getting the ACI interrupts for button press.
 *
 * Looks like if the Audio driver init happens this is not required, in
 * case if Audio driver is not included in the build then this macro should
 * be included to get headset working.
 *
 * Ideally if a macro is used to control brcm audio driver inclusion that does
 * AUDIOH init, then we	 don't need another macro here, it can be something
 * like #ifndef CONFING_BRCM_AUDIOH
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

	pr_debug ("=== __headset_hw_init: Interrupts disabled \r\n");

	/*
	 * This ensures that the timing parameters are configured
	 * properly. Note that only when the mode is 
	 * CHAL_ACI_MIC_BIAS_DISCONTINUOUS, this is done.
	 */
	aci_init_mic_bias.mode = CHAL_ACI_MIC_BIAS_DISCONTINUOUS;
        chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
		CHAL_ACI_BLOCK_GENERIC, &aci_init_mic_bias);

	/* Turn ON the MIC BIAS and put it in continuous mode */ 
	/*
	 * NOTE:
	 * This chal call was failing becuase internally this call
	 * was configuring AUDIOH registers as well. We have commmented
	 * configuring AUDIOH reigsrs in CHAL and it works OK
	 */
	aci_init_mic_bias.mode = CHAL_ACI_MIC_BIAS_ON;
        chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
		CHAL_ACI_BLOCK_GENERIC, &aci_init_mic_bias);

	pr_debug ("=== __headset_hw_init: MIC BIAS settings done \r\n");

	/* Configure comparator 1 for accessory detection */
	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_COMP1,
		&comp_values_for_type_det);
	
	pr_debug ("=== __headset_hw_init: ACI Block1 comprator1 configured \r\n");

	/* Configure the comparator 2 for button press */
	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER, CHAL_ACI_BLOCK_COMP2,
		&comp_values_for_button_press);

	pr_debug ("=== __headset_hw_init: ACI Block2 comprator2 configured \r\n");

	/* 
	 * Connect P_MIC_DATA_IN to P_MIC_OUT  and P_MIC_OUT to COMP2
	 * Note that one API can do this.
	 */
	chal_aci_set_mic_route (p->aci_chal_hdl, CHAL_ACI_MIC_ROUTE_MIC);

	/* Ensure that P_MIC_DATA_IN is connected to both COMP1
	 * and COMP2. On Rhea by default P_MIC_DATA_IN is always routed to
	 * COMP1, the above code routes it to COMP2 also.
	 * Note that COMP1 is used for accessory detection and COMP2 is used
	 * for button press detection.
	 */

	pr_debug ("=== __headset_hw_init: Configured MIC route \r\n");

	/* Fast power up the Vref of ADC block */
	/*
	 * NOTE:
	 * This chal call was failing becuase internally this call
	 * was configuring AUDIOH registers as well. We have commmented
	 * configuring AUDIOH reigsrs in CHAL and it works OK
	 */
	 aci_vref_config.mode = CHAL_ACI_VREF_FAST_ON;
	 chal_aci_block_ctrl(p->aci_chal_hdl, CHAL_ACI_BLOCK_ACTION_VREF,
	 	CHAL_ACI_BLOCK_GENERIC, &aci_vref_config);

	pr_debug ("=== __headset_hw_init: Configured Vref and ADC \r\n");

	/* Power down the MIC Bias and put in HIZ */ 
	chal_aci_block_ctrl(p->aci_chal_hdl, 
		CHAL_ACI_BLOCK_ACTION_MIC_POWERDOWN_HIZ_IMPEDANCE,
		CHAL_ACI_BLOCK_GENERIC,
		TRUE);

	pr_debug ("=== __headset_hw_init: powered down MIC BIAS and put in High impedence state \r\n");

	/* Set the threshold value for button press */
	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
		CHAL_ACI_BLOCK_COMP2,
		600);
	pr_debug ("=== __headset_hw_init: Configured the threshold value for button press\r\n");

	/* Set the threshold value for accessory type detection */
	chal_aci_block_ctrl(p->aci_chal_hdl,
		CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
		CHAL_ACI_BLOCK_COMP1,
		1900);

	pr_debug ("=== __headset_hw_init: Configured the threshold value for type detection\r\n");
 
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

	/* Configure AUDIOH CCU for clock policy */
	/* 
	 * Remove the entire Audio CCU config policy settings and AUDIOH
	 * initialization sequence once they are being done as a part of PMU and
	 * audio driver settings
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
	__headset_hw_init (mic);

	/* 
	 * If the platform uses GPIO for insetion/removal detection configure
	 * the same. Otherwise use COMP1 for interrupt detection
	 */
	if (mic->headset_pd->gpio_for_accessory_detection == 1) {

		pr_info("%s() - Platform configures gpio %d for accessory insertion detection \r\n", __func__, mic->gpio_irq);

		hs_gpio = irq_to_gpio(mic->gpio_irq);

		/* Request the gpio 
	 	 * Note that this is an optional call for setting direction/debounce
	 	 * values. But set debounce will throw out warning messages if we 
	 	 * call gpio_set_debounce without calling gpio_request. 
	 	 * Note that it just throws out Warning messages and proceeds
	 	 * to auto request the same. We are adding this call here to 
	 	 * suppress the warning message.
	 	*/
		status = gpio_request (hs_gpio, "hs_detect");
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

		pr_info("headset_hw_init: gpio config done \r\n");
	} else {
		pr_info("%s() - Platform uses COMP1 for accessory insertion/removal \r\n", __func__);
		/* 
		 * This platform does not have GPIO for accessory
		 * insertion/removal detection, use COMP1 for this reason.
		 * Enable COMP1 interrupt for accessory detection 
		 */
		chal_aci_block_ctrl(mic->aci_chal_hdl,	
			CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
			CHAL_ACI_BLOCK_COMP1);
	}

	return status;
}

static int hs_remove(struct platform_device *pdev)
{
	struct mic_t *mic;

	mic = platform_get_drvdata(pdev);

	free_irq(mic->gpio_irq, mic);
	free_irq(mic->hsbirq_press, mic);
	free_irq(mic->hsbirq_release, mic);

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
	int irq_resource_num = 0;

	mic = kzalloc(sizeof(struct mic_t), GFP_KERNEL);
	if (!mic)
		return -ENOMEM;

	mic_dev = mic;

	if (pdev->dev.platform_data)
		mic->headset_pd = pdev->dev.platform_data;
	else {
		/* The driver depends on the platform data (board specific)
		 * information to know two things
		 * 1) The GPIO state that determines accessory insertion (HIGH or LOW)
		 * 2) The resistor value put on the MIC_IN line.
		 *
		 * So if the platform data is not present, do not proceed.
		 */
		 pr_err("hs_probe: Platform data not present, could not proceed \r\n");
		 ret = EINVAL;
		 goto err2;
	}

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

	/*
	 * If on the given platform GPIO is used for accessory
	 * insertion/removal detection get the GPIO IRQ to be
	 * used for the same.
	 */
	if (mic->headset_pd->gpio_for_accessory_detection == 1) {
		/* Insertion detection irq */
		mic->gpio_irq = platform_get_irq(pdev, irq_resource_num);
		if (!mic->gpio_irq) {
			ret = -EINVAL;
			goto err1;
		}
		irq_resource_num++;
		pr_info("HS GPIO irq %d\n", mic->gpio_irq);
	}

	pr_info("%s() GPIO used for accessory insertion %d (1 - yes, 0 - no)",
		__func__,  mic->headset_pd->gpio_for_accessory_detection);

	if (mic->headset_pd->button_adc_values == NULL) {
		
		mic->headset_pd->button_adc_values = button_adc_values_no_resistor;

		pr_info("%s(): WARNING Board specific button adc values are not passed  \
		using the default one, this may not work correctly for your \
		platform \r\n", __func__);
	}
	
	/* Button press irq */
	mic->hsbirq_press = platform_get_irq(pdev, irq_resource_num);
	if (!mic->hsbirq_press) {
		ret = -EINVAL;
		goto err1;
	}
	irq_resource_num++;
	pr_info("HSB press COMP2 irq %d\n", mic->hsbirq_press);

	/* Button release irq */
	mic->hsbirq_release = platform_get_irq(pdev, irq_resource_num);
	if (!mic->hsbirq_release) {
		ret = -EINVAL;
		goto err1;
	}
	irq_resource_num++;
	pr_info("HSB release COMP2 irq inv %d\n", mic->hsbirq_release);

	/* Get COMP1 IRQ */
	mic->comp1_irq = platform_get_irq(pdev,irq_resource_num);
	if (!mic->comp1_irq) {
		ret = -EINVAL;
		goto err1;
	}
	irq_resource_num++;
	pr_info("COMP1 irq %d\n", mic->comp1_irq);

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
	 * should be requested with _NO_SUSPEND option because even if
	 * the system goes to suspend we want this interrupts to be active
	 */

	/* Request the IRQ for accessory insertion detection */
	ret =
	    request_irq(mic->gpio_irq, gpio_isr,
			(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING |
			 IRQF_NO_SUSPEND), "aci_accessory_detect", mic);
	if (ret < 0) {
		pr_err("%s(): request_irq() failed for headset %s: %d\n",
			__func__, "irq", ret);
		goto err1;
	}

	ret =
	    request_irq(mic->comp1_irq, comp1_isr,
			 IRQF_NO_SUSPEND, "aci_accessory_detect_comp", mic);
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
		free_irq(mic->gpio_irq, mic);
		free_irq(BCM_INT_ID_AUXMIC_COMP1, mic);
		goto err1;
	}

	/* Request the IRQ for HS Button release */
	ret =
	    request_irq(mic->hsbirq_release, comp2_inv_isr, IRQF_NO_SUSPEND,
			"aci_hs_button_release", mic);
	if (ret < 0) {
		pr_err("%s(): request_irq() failed for headset %s: %d\n",
			__func__, "button release", ret);
		/* Free the HS IRQ if the HS Button IRQ request fails */
		free_irq(mic->gpio_irq, mic);
		free_irq(BCM_INT_ID_AUXMIC_COMP1, mic);
		free_irq(mic->hsbirq_press, mic);
		goto err1;
	}

	mic->hs_state = DISCONNECTED;
	mic->button_state = BUTTON_RELEASED;

	/* Store the mic structure data as private driver data for later use */
	platform_set_drvdata(pdev, mic);

	/* 
	 * Its important to understand why we schedule the accessory detection
	 * work queue from here.
	 *
	 * From the schematics the GPIO status should be 
	 * 1 - nothing iserted 
	 * 0 - accessory inserted
	 *
	 * The pull up for the GPIO is connecte to 1.8 V that is source by the
	 * PMU. But the PM chip's init happens after headset insertion, so
	 * reading the GPIO value during init may not give us the correct
	 * status (will read 0 always). Later after the init when the GPIO
	 * gets the 1.8 V an interrupt would be triggered for rising edge and
	 * the GPIO ISR would schedule the work queue. But if the accessory is
	 * kept connected assuming the PMU to trigger the ISR is like taking a
	 * chance. Also, if for some reason PMU init is moved before head set
	 * driver init then the GPIO state would not change after headset
	 * driver init and the GPIO interrupt may not be triggered. 
	 * Its safe to schedule detection work here becuase
	 * during bootup, irrespective of the GPIO interrupt we'll detect the
	 * accessory type. (Even if the interrupt occurs no harm done since
	 * the work queue will be any way executed only once). 
	 */
	schedule_delayed_work(&(mic->accessory_detect_work), ACCESSORY_INSERTION_REMOVE_SETTLE_TIME);

	return ret;
err1:
	pr_err("%s(): Error driver not loaded \r\n", __func__);
	hs_unregswitchdev(mic);
	hs_unreginputdev(mic);
err2:
	kfree(mic);
	return ret;
}

/*
 * Note that there is a __refdata added to the headset_driver platform driver
 * structure. What is the meaning for it and why its required.
 *
 * The probe function: 
 * From the platform driver documentation its advisable to keep the probe
 * function of a driver in the __init section if the device is NOT hot pluggable. 
 * Note that in headset case even though the headset is hot pluggable, the driver 
 * is not. That is a new node will not be created and the probe will not be called 
 * again. So it makes sense to keep the hs_probe in __init section so as to 
 * reduce the driver's run time foot print.
 * 
 * The Warning message:
 * But since the functions address (reference) is stored in a structure that
 * will be available even after init (in case of remove, suspend etc) there
 * is a Warning message from the compiler
 *
 * The __refdata keyword can be used to suppress this warning message. Tells the
 * compiler not to throw out this warning. And in this scenario even though
 * we store the function pointer from __init section to the platform driver
 * structure that lives after __init, we wont be referring the probe function
 * in the life time until headset_driver lives, so its OK to suppress.
 */
static struct platform_driver __refdata headset_driver = {
	.probe = hs_probe,
	.remove = hs_remove,
	.driver = {
		   .name = "konaaciheadset",
		   .owner = THIS_MODULE,
		   },
};

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_init
    Description     : Initialize the driver
    Return type     : int
------------------------------------------------------------------------------*/
int __init kona_aci_hs_module_init(void)
{
 	return platform_driver_register(&headset_driver);
}

/*------------------------------------------------------------------------------
    Function name   : kona_hs_module_exit
    Description     : clean up
    Return type     : int
------------------------------------------------------------------------------*/
void __exit kona_aci_hs_module_exit(void)
{
	return platform_driver_unregister(&headset_driver);
}

module_init(kona_aci_hs_module_init);
module_exit(kona_aci_hs_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Headset plug and button detection");

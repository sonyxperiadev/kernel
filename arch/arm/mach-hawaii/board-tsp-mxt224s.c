/*
 * linux/arch/arm/mach-xxxx/xxxx
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT224S
#include <linux/regulator/consumer.h>
#include <linux/input/mxt224s.h>

#define TSP_INT_GPIO_PIN   	(73)
#define MXT_TAG "MXT224S"

u8 touch_pressed;
extern enum cable_type_t set_cable_status;

static struct regulator *mxt_vdd_regulator;
static struct regulator *mxt_vio_regulator;

static int mxt_setup_power(struct device *dev, bool setup)
{
	int min_uV, max_uV;
	int ret;

	if (setup) {
		min_uV = max_uV = 3300000;
		mxt_vdd_regulator = regulator_get(dev, "gpldo2_uc");

		if (IS_ERR(mxt_vdd_regulator)) {
			ret = PTR_ERR(mxt_vdd_regulator);
			printk(KERN_ERR "%s: %s: fail to get mxt_vdd_regulator "
			       "(%d)\n", MXT_TAG, __func__, ret);
			return ret;
		}

		ret = regulator_set_voltage(mxt_vdd_regulator, min_uV, max_uV);
		if (ret < 0) {
			printk(KERN_ERR "%s: %s: fail to set mxt_vdd_regulator"
			       " to %d, %d (%d)\n", MXT_TAG, __func__, min_uV,
			       max_uV, ret);
			goto err_set_vdd_voltage;
		}
		printk(KERN_INFO "%s: %s: set vdd to %d uV - %d uV (%d)\n",
		       MXT_TAG, __func__, min_uV, max_uV, ret);

		min_uV = max_uV = 1800000;
		mxt_vio_regulator = regulator_get(dev, "micldo_uc");
		if (IS_ERR(mxt_vio_regulator)) {
			ret = PTR_ERR(mxt_vio_regulator);
			printk(KERN_ERR "%s: %s: fail to get mxt_vio_regulator "
			       "(%d)\n", MXT_TAG, __func__, ret);
			goto err_get_vio_regulator;
		}
		ret = regulator_set_voltage(mxt_vio_regulator, min_uV,
					    max_uV);
		if (ret < 0) {
			printk(KERN_ERR "%s: %s: fail to set mxt_vio_regulator "
			       "to %d, %d (%d)\n", MXT_TAG, __func__, min_uV,
			       max_uV, ret);
			goto err_set_vio_voltage;
		}
		printk(KERN_INFO "%s: %s: set vled to %d uV - %d uV (%d)\n",
		       MXT_TAG, __func__, min_uV, max_uV, ret);
	} else {
		regulator_force_disable(mxt_vdd_regulator);
		regulator_put(mxt_vdd_regulator);

		regulator_force_disable(mxt_vio_regulator);
		regulator_put(mxt_vio_regulator);
	}

	return 0;

err_set_vio_voltage:
	regulator_put(mxt_vio_regulator);
err_get_vio_regulator:
err_set_vdd_voltage:
	regulator_put(mxt_vdd_regulator);

	return ret;
}

static void mxt_power_con(bool on)
{
	if (mxt_vio_regulator == NULL || mxt_vdd_regulator == NULL) {
		printk(KERN_ERR "%s: %s: no regulator\n", MXT_TAG,  __func__);
		return;
	}

	if (on) {
		printk("%s: power on", __func__);
		regulator_enable(mxt_vio_regulator);
		regulator_enable(mxt_vdd_regulator);
	} else {
		printk("%s: power off", __func__);
		regulator_disable(mxt_vdd_regulator);
		regulator_disable(mxt_vio_regulator);
	}

	printk(KERN_INFO "%s: %s: %s\n", MXT_TAG,  __func__,
	       (on) ? "on" : "off");
}

/*
	Configuration for MXT Touchscreen
*/
#define MXT_MAX_FINGERS			10
#define MXT_CHRGTIME_BATT		25
#define MXT_CHRGTIME_CHRG		60
#define MXT_THRESHOLD_BATT		60
#define MXT_THRESHOLD_CHRG		70
#define MXT_CALCFG_BATT			210
#define MXT_CALCFG_CHRG			210

static u8 t7_config[] = {
	GEN_POWERCONFIG_T7,
	0x20,			/* IDLEACQINT */
	0xFF,			/* ACTVACQINT */
	0x32,			/* ACTV2IDLETO */
	0x03			/* CFG */
};

static u8 t8_config[] = {
	GEN_ACQUISITIONCONFIG_T8,
	0x01,			/* CHRGTIME */
	0x00,
	0x05,			/* TCHDRIFT */
	0x01,			/* DRIFTST */
	0x00,			/* TCHAUTOCAL */
	0x00,			/* SYNC */
	0x0A,			/* ATCHCALST */
	0x1E,			/* ATCHCALSTHR */
	0x05,			/* ATCHFRCCALTHR */
	0x97			/* ATCHFRCCALRATIO */
};

static u8 t9_config[] = {
	TOUCH_MULTITOUCHSCREEN_T9,
	0x83,			/* CTRL */
	0x00,			/* XORIGIN */
	0x00,			/* YORIGIN */
	0x13,			/* XSIZE */
	0x0B,			/* YSIZE */
	0x00,			/* AKSCFG */
	0x70,			/* BLEN */
	0x37,			/* TCHTHR */
	0x02,			/* TCHDI */
	0x01,			/* ORIENT */
	0x0A,			/* MRGTIMEOUT */
	0x0A,			/* MOVHYSTI */
	0x01,			/* MOVHYSTN */
	0x30,			/* MOVFILTER */
	0x0A,			/* NUMTOUCH */
	0x0F,			/* MRGHYST */
	0x1E,			/* MRGTHR */
	0x0A,			/* AMPHYST */
	0x1F,			/* XRANGE */
	0x03,			/* XRANGE */
	0xDF,			/* YRANGE */
	0x01,			/* YRANGE */
	0xF6,			/* XLOCLIP */
	0xF6,			/* XHICLIP */
	0xFB,			/* YLOCLIP */
	0xFB,			/* YHICLIP */
	0x00,			/* XEDGECTRL */
	0x00,			/* XEDGEDIST */
	0x00,			/* YEDGECTRL */
	0x00,			/* YEDGEDIST */
	0x12,			/* JUMPLIMIT */
	0x0F,			/* TCHHYST */
	0x00,			/* XPITCH */
	0x00,			/* YPITCH */
	0x00,			/* NEXTTCHDI */
	0x00,			/* CFG */
};

static u8 t9_config_ta[] = {
	TOUCH_MULTITOUCHSCREEN_T9,
	0x83,			/* CTRL */
	0x00,			/* XORIGIN */
	0x00,			/* YORIGIN */
	0x13,			/* XSIZE */
	0x0B,			/* YSIZE */
	0x00,			/* AKSCFG */
	0x70,			/* BLEN */
	0x37,			/* TCHTHR */
	0x02,			/* TCHDI */
	0x01,			/* ORIENT */
	0x0A,			/* MRGTIMEOUT */
	0x0A,			/* MOVHYSTI */
	0x01,			/* MOVHYSTN */
	0x30,			/* MOVFILTER */
	0x0A,			/* NUMTOUCH */
	0x0F,			/* MRGHYST */
	0x1E,			/* MRGTHR */
	0x0A,			/* AMPHYST */
	0x1F,			/* XRANGE */
	0x03,			/* XRANGE */
	0xDF,			/* YRANGE */
	0x01,			/* YRANGE */
	0xF6,			/* XLOCLIP */
	0xF6,			/* XHICLIP */
	0xFB,			/* YLOCLIP */
	0xFB,			/* YHICLIP */
	0x00,			/* XEDGECTRL */
	0x00,			/* XEDGEDIST */
	0x00,			/* YEDGECTRL */
	0x00,			/* YEDGEDIST */
	0x12,			/* JUMPLIMIT */
	0x0F,			/* TCHHYST */
	0x00,			/* XPITCH */
	0x00,			/* YPITCH */
	0x00,			/* NEXTTCHDI */
	0x00,			/* CFG */
};


static u8 t15_config[] = {
	TOUCH_KEYARRAY_T15,
	0,			/* CTRL */
	0,			/* XORIGIN */
	0,			/* YORIGIN */
	0,			/* XSIZE */
	0,			/* YSIZE */
	0,			/* AKSCFG */
	0,			/* BLEN */
	0,			/* TCHTHR */
	0,			/* TCHDI */
	0,
	0
};

static u8 t18_config[] = {
	SPT_COMCONFIG_T18,
	0,			/* CTRL */
	0			/* COMMAND */
};

static u8 t19_config[] = {
	SPT_GPIOPWM_T19,
	0,			/* CTRL */
	0,			/* REPORTMASK */
	0,			/* DIR */
	0,			/* INTPULLUP */
	0,			/* OUT */
	0			/* WAKE */
};

static u8 t23_config[] = {
	TOUCH_PROXIMITY_T23,
	0,			/* CTRL */
	0,			/* XORIGIN */
	0,			/* YORIGIN */
	0,			/* XSIZE */
	0,			/* YSIZE */
	0,
	0,			/* BLEN */
	0,			/* FXDDTHR */
	0,			/* FXDDTHR */
	0,			/* FXDDI */
	0,			/* AVERAGE */
	0,			/* MVNULLRATE */
	0,			/* MVNULLRATE */
	0,			/* MVDTHR */
	0			/* MVDTHR */
};

static u8 t25_config[] = {
	SPT_SELFTEST_T25,
	0,			/* CTRL */
	0,			/* CMD */
	0,			/* UPSIGLIM */
	0,			/* UPSIGLIM */
	0,			/* LOSIGLIM */
	0,			/* LOSIGLIM */
	0,			/* ... */
	0,			/* ... */
	0,			/* ... */
	0,			/* ... */
	0,			/* ... */
	0,			/* ... */
	0,			/* ... */
	0,			/* ... */
	0			/* PINDWELLUS */
};
/*
static u8 t35_config[] = {
	SPT_GENERICDATA_T35,
	0x1C,
	0x00,
	0x01,
	0x02,
	0x09,
	0x0A
};

static u8 t35_config_ta[] = {
	SPT_GENERICDATA_T35,
	0x0C,
	0x00,
	0x01,
	0x02,
	0x05,
	0x06
};
*/
static u8 t40_config[] = {
	PROCI_GRIPSUPPRESSION_T40,
	0,			/* CTRL */
	0,			/* XLOGRIP */
	0,			/* XHIGRIP */
	0,			/* YLOGRIP */
	0			/* YHIGRIP */
};

static u8 t42_config[] = {
	PROCI_TOUCHSUPPRESSION_T42,
	0,			/* CTRL */
	0,			/* APPRTHR */
	0,			/* MAXAPPRAREA */
	0,			/* MAXTCHAREA */
	0,			/* SUPSTRENGTH */
	0,			/* SUPEXTTO */
	0,			/* MAXNUMTCHS */
	0,			/* SHAPESTRENGTH */
	0,			/* SUPDIST */
	0			/* DISTHYST */
};

static u8 t46_config[] = {
	SPT_CTECONFIG_T46,
	0x04,			/* CTRL */
	0x00,
	0x10,			/* IDLESYNCSPERX */
	0x20,			/* ACTVSYNCSPERX */
	0x00,			/* ADCSPERSYNC */
	0x01,			/* PULSESPECRADC */
	0x03,			/* XSLEW */
	0x00,			/* SYNCDELAY */
	0x00,			/* SYNCDELAY */
	0x01			/* XVOLTAGE */
};

static u8 t46_config_ta[] = {
	SPT_CTECONFIG_T46,
	0x04,			/* CTRL */
	0x00,
	0x10,			/* IDLESYNCSPERX */
	0x20,			/* ACTVSYNCSPERX */
	0x00,			/* ADCSPERSYNC */
	0x01,			/* PULSESPECRADC */
	0x03,			/* XSLEW */
	0x00,			/* SYNCDELAY */
	0x00,			/* SYNCDELAY */
	0x01			/* XVOLTAGE */
};

static u8 t47_config[] = {
	PROCI_STYLUS_T47,
	0x00,			/* CTRL */
	0x28,			/* CONTMIN */
	0x3c,			/* CONTMAX */
	0x0a,			/* STABILITY */
	0x02,			/* MAXTCHAREA */
	0x14,			/* AMPLTHR */
	0x00,			/* STYSHAPE */
	0x78,			/* HOVERSUP */
	0x02,			/* CONFTHR */
	0x18,			/* SYNCSPERX */
	0x00,			/* XPOSADJ */
	0x00,			/* YPOSADJ */
	0x00,			/* CFG */
	0x00,			/* RESERVED */
	0x00,			/* RESERVED */
	0x00,			/* RESERVED */
	0x00,			/* RESERVED */
	0x00,			/* RESERVED */
	0x00,			/* RESERVED */
	0x00,			/* RESERVED */
	0x00,			/* SUPSTYTO */
	0x00			/* MAXNUMSTY */
};

static u8 t55_config[] = {
	ADAPTIVE_T55,
	0,			/* CTRL */
	0,			/* TARGETTHR */
	0,			/* THRADJLIM */
	0,			/* RESETSTEPTIME */
	0,			/* FORCECHGDIST */
	0			/* FORCECHGTIME */
};

static u8 t56_config[] = {
	PROCI_SHIELDLESS_T56,
	0x03,			/* CTRL */
	0x00,
	0x01,			/* OPTINT */
	0x1d,			/* INTTIME */
	0x09,			/* INTDELAY */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x09,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* MULTICUTGC */
	0x00,			/* GCLIMIT */
	0x01,			/* NCNCL */
	0x00,			/* TOUCHBIAS */
	0x00,			/* BASESCALE */
	0x00,			/* SHIFTLIMIT */
	0x00,			/* YLONOISEMUL */
	0x00,			/* YLONOISEMUL */
	0x00,			/* YLONOISEDIV */
	0x00,			/* YLONOISEDIV */
	0x00,			/* YHINOISEMUL */
	0x00,			/* YHINOISEMUL */
	0x00,			/* YHINOISEDIV */
	0x00			/* YHINOISEDIV */
};
static u8 t56_config_ta[] = {
	PROCI_SHIELDLESS_T56,
	0x03,			/* CTRL */
	0x00,
	0x01,			/* OPTINT */
	0x16,			/* INTTIME */
	0x0A,			/* INTDELAY */
	0x0A,			/* ... */
	0x0A,			/* ... */
	0x09,			/* ... */
	0x0A,			/* ... */
	0x0A,			/* ... */
	0x0A,			/* ... */
	0x0B,			/* ... */
	0x0B,			/* ... */
	0x0B,			/* ... */
	0x0C,			/* ... */
	0x0B,			/* ... */
	0x0C,			/* ... */
	0x0D,			/* ... */
	0x0D,			/* ... */
	0x0E,			/* ... */
	0x0E,			/* ... */
	0x0F,			/* ... */
	0x0F,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x00,			/* MULTICUTGC */
	0x00,			/* GCLIMIT */
	0x01,			/* NCNCL */
	0x00,			/* TOUCHBIAS */
	0x00,			/* BASESCALE */
	0x00,			/* SHIFTLIMIT */
	0x00,			/* YLONOISEMUL */
	0x00,			/* YLONOISEMUL */
	0x00,			/* YLONOISEDIV */
	0x00,			/* YLONOISEDIV */
	0x00,			/* YHINOISEMUL */
	0x00,			/* YHINOISEMUL */
	0x00,			/* YHINOISEDIV */
	0x00			/* YHINOISEDIV */
};

static u8 t57_config[] = {
	PROCI_EXTRATOUCHSCREENDATA_T57,
	0xE3,			/* CTRL */
	0x1E,			/* AREATHR */
	0x00			/* AREAHYST */
};

static u8 t61_config[] = {
	SPT_TIMER_T61,
	3,			/* CTRL */
	0,			/* CMD */
	0,			/* MODE */
	0,			/* PERIOD */
	0			/* PERIOD */
};

static u8 t62_config[] = {
	PROCG_NOISESUPPRESSION_T62,
	0x4F,			/* CTRL */
	0x01,			/* CALCFG1 */
	0x00,			/* CALCFG2 */
	0x00,			/* CALCFG3 */
	0x0A,			/* CFG1 */
	0x00,
	0x00,
	0x00,			/* BASEFREQ */
	0x05,			/* MAXSELFREQ */
	0x00,			/* FREQ */
	0x00,			/* FREQ */
	0x00,			/* FREQ */
	0x00,			/* FREQ */
	0x05,			/* FREQ */
	0x05,			/* HOPCNT */
	0x00,
	0x0A,			/* HOPCNTPER */
	0x05,			/* HOPEVALTO */
	0x05,			/* HOPST */
	0x80,			/* NLGAIN */
	0x0F,			/* MINNLTHR */
	0x14,			/* INCNLTHR */
	0x30,			/* ADCSPERXTHR */
	0x0F,			/* NLTHRMARGIN */
	0x3F,			/* MAXADCSPERX */
	0x18,			/* ACTVADCSVLDNOD */
	0x10,			/* IDLEADCSVLDNOD */
	0x04,			/* MINGCLIMIT */
	0x64,			/* MAXGCLIMIT */
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x60,			/* T9SETTINGS */
	0x37,			/* ... */
	0x02,			/* ... */
	0x05,			/* ... */
	0x01,			/* ... */
	0x30,			/* ... */
	0x0A,			/* ... */
	0x0F,			/* ... */
	0x0F,			/* ... */
	0x1E,			/* ... */
	0x1E,			/* ... */
	0xF6,			/* ... */
	0xF6,			/* ... */
	0xF2,			/* ... */
	0x3E,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x12,			/* ... */
	0x0A,			/* ... */
	0x00			/* ... */
};

static u8 t62_config_ta[] = {
	PROCG_NOISESUPPRESSION_T62,
	0x4F,			/* CTRL */
	0x01,			/* CALCFG1 */
	0x00,			/* CALCFG2 */
	0x00,			/* CALCFG3 */
	0x0A,			/* CFG1 */
	0x00,
	0x00,
	0x00,			/* BASEFREQ */
	0x05,			/* MAXSELFREQ */
	0x00,			/* FREQ */
	0x00,			/* FREQ */
	0x00,			/* FREQ */
	0x00,			/* FREQ */
	0x05,			/* FREQ */
	0x05,			/* HOPCNT */
	0x00,
	0x0A,			/* HOPCNTPER */
	0x05,			/* HOPEVALTO */
	0x05,			/* HOPST */
	0x80,			/* NLGAIN */
	0x0F,			/* MINNLTHR */
	0x14,			/* INCNLTHR */
	0x30,			/* ADCSPERXTHR */
	0x0F,			/* NLTHRMARGIN */
	0x3F,			/* MAXADCSPERX */
	0x18,			/* ACTVADCSVLDNOD */
	0x10,			/* IDLEADCSVLDNOD */
	0x04,			/* MINGCLIMIT */
	0x64,			/* MAXGCLIMIT */
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x60,			/* T9SETTINGS */
	0x37,			/* ... */
	0x02,			/* ... */
	0x05,			/* ... */
	0x01,			/* ... */
	0x30,			/* ... */
	0x0A,			/* ... */
	0x0F,			/* ... */
	0x0F,			/* ... */
	0x1E,			/* ... */
	0x1E,			/* ... */
	0xF6,			/* ... */
	0xF6,			/* ... */
	0xF2,			/* ... */
	0x3E,			/* ... */
	0x00,			/* ... */
	0x00,			/* ... */
	0x12,			/* ... */
	0x0A,			/* ... */
	0x00			/* ... */
};

static u8 end_config[] = { RESERVED_T255 };

static u8 end_config_ta[] = { RESERVED_T255 };

static struct mxt_config_struct config_data[] = {
	MXT_CONFIG(t7, 0),
	MXT_CONFIG(t8, 0),
	MXT_CONFIG(t9, 0),
	MXT_CONFIG(t15, 0),
	MXT_CONFIG(t18, 0),
	MXT_CONFIG(t19, 0),
	MXT_CONFIG(t23, 0),
	MXT_CONFIG(t25, 0),
	MXT_CONFIG(t40, 0),
	MXT_CONFIG(t42, 0),
	MXT_CONFIG(t46, 0),
	MXT_CONFIG(t47, 0),
	MXT_CONFIG(t55, 0),
	MXT_CONFIG(t56, 0),
	MXT_CONFIG(t57, 0),
	MXT_CONFIG(t61, 0),
	MXT_CONFIG(t62, 0),
	MXT_CONFIG(end, 0)
};

static struct mxt_config_struct ta_config_data[] = {
/*	MXT_CONFIG_TA(t9, 0),*/
/*	MXT_CONFIG_TA(t46, 0),*/
	MXT_CONFIG_TA(t56, 0),
/*	MXT_CONFIG_TA(t62, 0),*/
	MXT_CONFIG_TA(end, 0)
};

static struct mxt_platform_data mxt_pdata = {
	.num_of_fingers = MXT_MAX_FINGERS,
	.gpio_read_done = TSP_INT_GPIO_PIN,
	.config = config_data,
	.ta_config = ta_config_data,
	.config_ver = "I8190_At_0308",
	.min_x = 0,
	.max_x = 480,
	.min_y = 0,
	.max_y = 800,
	.min_z = 0,
	.max_z = 255,
	.min_w = 0,
	.max_w = 30,
	.exit_flag = 0,
	.setup_power = mxt_setup_power,
	.power_con = mxt_power_con,
	.touchscreen_is_pressed = &touch_pressed,
	.initial_cable_status = (u8 *)&set_cable_status,
};
/*"Atmel MXT224S"*/
static struct i2c_board_info __initdata tsp_i2c_devices[] = {
	  {
			I2C_BOARD_INFO(MXT_DEV_NAME, 0x4A),
			.platform_data	= &mxt_pdata,
			.irq = gpio_to_irq(TSP_INT_GPIO_PIN),
	  },
};

void mxt_init(void)
{
	gpio_request(TSP_INT_GPIO_PIN, "TSP_INT");
	mxt_pdata.exit_flag = (u8)0;

	i2c_register_board_info(3, tsp_i2c_devices, ARRAY_SIZE(tsp_i2c_devices));
}
#endif	/*CONFIG_TOUCHSCREEN_ATMEL_MXT224S*/

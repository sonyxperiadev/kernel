/* ************************************************************************
 *       Filename:  drv2624-ram.c
 *    Description:
 *        Version:  1.0
 *        Created:  01/28/21 18:55:42
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  Gust Zhang
 *        Company:  Texas Instruments Inc.
 * ************************************************************************/

#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include "drv2624.h"

#define LRA_PERIOD_SCALE_NS 		(24390)
#define LRA_PERIOD_SCALE_UNIT		(1000000000)
/* 24.615us * 2000 = 49230ns*/
#define OL_LRA_PERIOD_SCALE_NSX2	(49230)
#define OL_LRA_PERIOD_SCALE_NS		(24615)
#define OL_LRA_PERIOD_FACTOR		(2)
#define OL_LRA_PERIOD_SCALE_UNIT	(2000000000)
#define FRES_FACTOR					(7)

/*Kernel parameter which is got from UEFI to distingusih different haptic*/
static char haptic_mode[4];
module_param_string(haptic, haptic_mode, sizeof(haptic_mode), 0600);

static bool drv26xx_volatile(struct device *dev, unsigned int reg)
{
	return true;
}

static bool drv26xx_writeable(struct device *dev, unsigned int reg)
{
	return true;
}

static struct regmap_config drv2624_i2c_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.writeable_reg = drv26xx_writeable,
	.volatile_reg = drv26xx_volatile,
	.cache_type = REGCACHE_NONE,
	.max_register = 256,
};

static int drv2624_reg_read(struct drv2624_data *pDRV2624, unsigned char reg)
{
	unsigned int val;
	int nResult = -1;
	int retry_count = DRV26XX_I2C_RETRY_COUNT;

	while (retry_count--) {
		mutex_lock(&pDRV2624->reg_lock);
		nResult = regmap_read(pDRV2624->mpRegmap, reg, &val);
		mutex_unlock(&pDRV2624->reg_lock);
		if (nResult >= 0) {
			dev_info(pDRV2624->dev, "%s:%u: Reg[0x%02x]=0x%02x\n",
				__func__, __LINE__, reg, val);
			return val;
		} else {
			if(retry_count != 0) msleep(10);
		}
	}
	if(nResult < 0) dev_err(pDRV2624->dev, "%s:%u: I2C error %d\n",
		__func__, __LINE__, nResult);
	return nResult;
}

static int drv2624_reg_write(struct drv2624_data *pDRV2624,
			     unsigned char reg, unsigned char val)
{
	int nResult = -1;
	int retry_count = DRV26XX_I2C_RETRY_COUNT;
	dev_info(pDRV2624->dev, "%s:%u: Reg[0x%02x]=0x%02x\n",
		__func__, __LINE__, reg, val);
	while (retry_count--) {
		mutex_lock(&pDRV2624->reg_lock);
		nResult = regmap_write(pDRV2624->mpRegmap, reg, val);
		mutex_unlock(&pDRV2624->reg_lock);
		if (nResult >= 0) break;
		else {
			if(retry_count != 0) msleep(10);
		}
	}
	if(nResult < 0) dev_err(pDRV2624->dev, "%s:%u: I2C error %d\n",
		__func__, __LINE__, nResult);
	return nResult;
}

static int drv2624_bulk_write(struct drv2624_data *pDRV2624,
				unsigned char reg, const u8 *buf,
				unsigned int count)
{
	int nResult = -1, i = 0;
	int retry_count = DRV26XX_I2C_RETRY_COUNT;
	while (retry_count--) {
		mutex_lock(&pDRV2624->reg_lock);
		nResult = regmap_bulk_write(pDRV2624->mpRegmap, reg, buf, count);
		mutex_unlock(&pDRV2624->reg_lock);
		if (nResult >= 0) break;
		else {
			if(retry_count != 0) msleep(20);
		}
	}
	if (nResult >= 0) {
		for (i = 0; i < count; i++)
			dev_info(pDRV2624->dev, "%s, Reg[0x%02x]=0x%02x\n", __func__,
				reg + i, buf[i]);
	} else {
		dev_err(pDRV2624->dev, "%s:%u: reg=0%02x, count=%d error %d\n",
			__func__, __LINE__, reg, count, nResult);
	}
	return nResult;
}

static int drv2624_set_bits(struct drv2624_data *pDRV2624,
			    unsigned char reg, unsigned char mask,
			    unsigned char val)
{
	int nResult = -1;
	int retry_count = DRV26XX_I2C_RETRY_COUNT;
	dev_info(pDRV2624->dev, "%s:%u: Reg[0x%02x]:M=0x%02x, V=0x%02x\n",
		__func__, __LINE__, reg, mask, val);
	while (retry_count--) {
		mutex_lock(&pDRV2624->reg_lock);
		nResult = regmap_update_bits(pDRV2624->mpRegmap, reg, mask, val);
		mutex_unlock(&pDRV2624->reg_lock);
		if (nResult >= 0) break;
		else {
			if(retry_count != 0) msleep(20);
		}
	}
	if(nResult < 0)	dev_err(pDRV2624->dev, "%s:%u: I2C error %d\n",
		__func__, __LINE__, nResult);
	return nResult;
}

static void drv2624_set_basic_reg(struct drv2624_data *pDRV2624)
{
	drv2624_set_bits(pDRV2624, DRV2624_R0X09,
		DRV2624_R0X09_UVLO_THRES_MSK,
		DRV2624_R0X09_UVLO_THRES_3_2V);
	drv2624_set_bits(pDRV2624, DRV2624_R0X0D,
		DRV2624_R0X0D_PLAYBACK_INTERVAL_MSK,
		(true == pDRV2624->bInternval_1ms)?
		DRV2624_R0X0D_PLAYBACK_INTERVAL_1MS:DRV2624_R0X0D_PLAYBACK_INTERVAL_5MS);

	drv2624_set_bits(pDRV2624, DRV2624_R0X23,
		DRV2624_R0X23_BEMF_GAIN_MSK,
		DRV2624_R0X23_BEMF_GAIN_30X_LRA);
}

static void drv2624_hw_reset(struct drv2624_data *pDRV2624)
{
	dev_dbg(pDRV2624->dev, "%s: %u! \n", __func__, __LINE__);
	gpio_direction_output(pDRV2624->msPlatData.mnGpioNRST, 0);
	usleep_range(5000, 5050);
	gpio_direction_output(pDRV2624->msPlatData.mnGpioNRST, 1);
	usleep_range(2000, 2050);
}

static void drv2624_parse_dt(struct device *dev, struct drv2624_data *pDRV2624)
{
	struct device_node *np = dev->of_node;
	struct drv2624_platform_data *pPlatData = &pDRV2624->msPlatData;
	int rc = 0;
	unsigned int value = 0;

	rc = of_property_read_u32(np, "ti,i2c_addr", &value);
	if (rc) {
		dev_err(pDRV2624->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,i2c_addr", np->full_name, rc);
		value = 0x5A;
	}
	dev_info(pDRV2624->dev, "ti,i2c_addr=%x\n", value);
	pDRV2624->client->addr = value;
	pPlatData->mnGpioNRST = of_get_named_gpio(np, "ti,reset-gpio", 0);
	if (!gpio_is_valid(pPlatData->mnGpioNRST)) {
		dev_err(pDRV2624->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,reset-gpio", np->full_name, pPlatData->mnGpioNRST);
	} else {
		dev_info(pDRV2624->dev, "ti,reset-gpio=%d\n",
			pPlatData->mnGpioNRST);
	}

	rc = of_property_read_u32(np, "ti,smart-loop", &value);
	if (rc) {
		dev_info(pDRV2624->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,smart-loop", np->full_name, rc);
		value = 1;
	}

	pPlatData->mnLoop = value & 0x01;
	dev_info(pDRV2624->dev, "ti,smart-loop=%d\n", pPlatData->mnLoop);

	rc = of_property_read_u32(np, "ti,interval-ms", &value);
	if (rc) {
		dev_info(pDRV2624->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,interval", np->full_name, rc);
		value = 1;
	}
	pDRV2624->bInternval_1ms = value == 5?false:true;
	dev_info(pDRV2624->dev, "ti,interval-ms=%d ms\n",
		pDRV2624->bInternval_1ms==true?1:5);

	rc = of_property_read_u32(np, "ti,actuator", &value);
	if (rc) {
		dev_err(pDRV2624->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,actuator", np->full_name, rc);
		value = LRA;
	}

	pPlatData->msActuator.mnActuatorType = value & 0x01;
	dev_info(pDRV2624->dev, "ti,actuator=%d\n",
		pPlatData->msActuator.mnActuatorType);

	rc = of_property_read_u32(np, "ti,odclamp-voltage", &value);
	if (rc) {
		dev_err(pDRV2624->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,odclamp-voltage", np->full_name, rc);
		value = 254;//2.54V
	}

	if (value < 254 || value > 500) {
		dev_err(pDRV2624->dev,
			"ERROR, ti,lra-odclamp-voltage=%d, out of range, set to default value\n",
			value);
		value = 254;
	}

	pPlatData->msActuator.mnOverDriveClampVoltage = (value * 1000) / 2122;

	dev_info(pDRV2624->dev, "ti,odclamp-voltage=%d, R=0x%02x\n",
		value, pPlatData->msActuator.mnOverDriveClampVoltage);

	if (pPlatData->msActuator.mnActuatorType == LRA) {
		rc = of_property_read_u32(np, "ti,lra-frequency", &value);
		if (rc) {
			dev_err(pDRV2624->dev,
				"Looking up %s property in node %s failed %d\n",
				"ti,lra-frequency", np->full_name, rc);
			value = 150;
		}

		if ((value < 45) || (value > 255)) {
			dev_err(pDRV2624->dev,
				"ERROR, ti,lra-frequency=%d, out of range, set to default value\n",
				value);
			value = 150;
		}

		pPlatData->msActuator.mnLRAFreq = value;
		pPlatData->msActuator.mnLRAPeriod = 10000 / value + 1;
		dev_info(pDRV2624->dev, "ti,lra-frequency = %uHz ti,lra-period = %ums\n",
			pPlatData->msActuator.mnLRAFreq, pPlatData->msActuator.mnLRAPeriod);

		rc = of_property_read_u32(np, "ti,sample-time", &pPlatData->msActuator.mnSampleTime);
		if (rc) {
			dev_err(pDRV2624->dev,
				"Looking up %s property in node %s failed %d\n",
				"ti,sample-time", np->full_name, rc);
		}
		//us
		switch (pPlatData->msActuator.mnSampleTime) {
			case 150:
				pPlatData->msActuator.mnSampleTimeReg = 0;
				break;
			case 200:
				pPlatData->msActuator.mnSampleTimeReg = 4;
				break;
			case 250:
				pPlatData->msActuator.mnSampleTimeReg = 8;
				break;
			default:
				pPlatData->msActuator.mnSampleTime = 300;
				pPlatData->msActuator.mnSampleTimeReg = 0xc;
				break;
		}
		dev_info(pDRV2624->dev, "ti,Sampletime=%d, R=0x%02x\n",
			pPlatData->msActuator.mnSampleTime, pPlatData->msActuator.mnSampleTimeReg);
		rc = of_property_read_u32(np, "ti,rated-voltage", &value);
		if (rc) {
			dev_err(pDRV2624->dev,
				"Looking up %s property in node %s failed %d\n",
				"ti,rated-voltage", np->full_name, rc);
			value = 180;
		}
		if ((value < 180) || (value > 350)) {
			dev_err(pDRV2624->dev,
				"ERROR, ti,rated-voltage=%d, out of range, set to default value\n",
				value);
			value = 180; //1.8V
		}

		pPlatData->msActuator.mnRatedVoltage =
		    value * int_sqrt(1000000-(4*pPlatData->msActuator.mnSampleTime+300)*pPlatData->msActuator.mnLRAFreq)/2058;
		dev_info(pDRV2624->dev, "ti,rated-voltage=%d, R=0x%02x\n",
			value, pPlatData->msActuator.mnRatedVoltage);
	}
	pPlatData->mnGpioINT = of_get_named_gpio(np, "ti,irq-gpio", 0);
	if (!gpio_is_valid(pPlatData->mnGpioINT)) {
		dev_err(pDRV2624->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,irq-gpio", np->full_name, pPlatData->mnGpioINT);
	} else
		dev_info(pDRV2624->dev, "ti,irq-gpio=%d\n",
			pPlatData->mnGpioINT);
}

static unsigned char drv_get_bf_value(const unsigned char bf,
	const unsigned char reg_value)
{
    unsigned char msk, value;

    /*
     * bitfield enum:
     * - 0..3  : len
     * - 4..7  : pos
     */
    unsigned char len = bf & 0x0f;
    unsigned char pos = (bf >> 4) & 0x0f;

    msk = ((1 << (len + 1)) - 1) << pos;
    value = (reg_value & msk) >> pos;

    return value;
}

static unsigned int drv26xx_ram_get_effect_timems(struct drv2624_data* pDRV2624,
	unsigned char effect, unsigned char* fw, int buf_size)
{
	unsigned short header_address, tmp;
	unsigned short address = 0;
	unsigned char effect_repeats = 0;
	unsigned int effect_size = 0;
	unsigned int i = 0;
	unsigned int ticks = 0;
	unsigned int playback_interval = 0;
	if (fw == NULL) return 0;
	header_address = effect * 3 + 1;
	if ((unsigned short)buf_size > header_address + 2) {
		tmp = fw[header_address];
		address = tmp << 8 | fw[header_address + 1];
		effect_repeats = (fw[header_address + 2] & 0xe0) >> 5;
		effect_size = fw[header_address + 2] & 0x1f;
	}
	else {
		dev_err(pDRV2624->dev, "%s:ram data missing\n", __func__);
		return 0;
	}
	for (i = 0; i < effect_size / 2; i++) {
		if ((unsigned short)buf_size > address + (i * 2) + 1)
			ticks += fw[address + (i * 2) + 1];
		else {
			dev_err(pDRV2624->dev, "%s:ram data error\n", __func__);
			return 0;
		}
	}
	playback_interval = (true == pDRV2624->bInternval_1ms) ? 1 : 5;
	dev_info(pDRV2624->dev, "%s: ticks = %d , effect_repeats = %d\n", __func__, ticks, effect_repeats);
	dev_info(pDRV2624->dev, "%s: playback_interval = %d\n", __func__, playback_interval);
	return ticks * (effect_repeats + 1) * playback_interval;
}


/* drv2624_ram_load:
* This function is called by the
* request_firmware_nowait function as soon
* as the firmware has been loaded from the file.
* The firmware structure contains the data and$
* the size of the firmware loaded.
*
* @fw: pointer to firmware file to be dowloaded
* @context: pointer variable to drv2624 data
*/
static void drv2624_ram_load(const struct firmware *pFW, void *pContext)
{
	struct drv2624_data *pDRV26xx = (struct drv2624_data*)pContext;
	struct drv2624_platform_data *pPlatData = NULL;
	struct drv26xx_RAMwaveforms* wvfm = NULL;
	struct drv26xx_RAMwaveform_info** ram_wvfm_info = NULL;
	unsigned char* buf = NULL;
	int cur = 0;
	int rest_size = 0;
	unsigned short i = 0;

	if (!pFW || !pFW->data || !pDRV26xx) {
		pr_info("%s:%u:Failed to read firmware\n",
			__func__, __LINE__);
		return;
	}
	buf = (unsigned char*)pFW->data;
	pPlatData = &pDRV26xx->msPlatData;
	wvfm = &(pDRV26xx->ramwvfm);
	if (cur + 4 > pFW->size) {
		dev_err(pDRV26xx->dev, "%s:%u:bin file error!\n",
			__func__, __LINE__);
		goto EXIT;
	}
	wvfm->rambin_name = kmemdup(RAM_BIN_FILE, sizeof(RAM_BIN_FILE), GFP_KERNEL);
	if (NULL == wvfm->rambin_name) {
        pr_err("%s:%u:FW memory failed!\n", __func__, __LINE__);
        goto EXIT;
    }
	wvfm->fb_brake_factor = drv_get_bf_value(DRV26XX_BF_FB_BRAKE_FACTOR, buf[cur]);
	wvfm->hybrid_loop = drv_get_bf_value(DRV26XX_BF_HYBRID_LOOP, buf[cur]);
	wvfm->auto_brake = drv_get_bf_value(DRV26XX_BF_AUTO_BRAKE, buf[cur]);
	wvfm->auto_brake_standby = drv_get_bf_value(DRV26XX_BF_AUTO_BRAKE_STANDBY, buf[cur]);
	cur++;
	wvfm->rated_Voltage = (unsigned int)buf[cur];
	cur++;
	wvfm->overDrive_Voltage = (unsigned int)buf[cur];
	cur++;
	wvfm->lra_f0 = buf[cur];
	cur ++;
	wvfm->nWaveforms = buf[cur];

	pPlatData->msActuator.mnCalRatedVoltage =
		    wvfm->rated_Voltage * int_sqrt(1000000-(4*pPlatData->msActuator.mnSampleTime+300)*pPlatData->msActuator.mnLRAFreq)/2058;
	pPlatData->msActuator.mnCalOverDriveClampVoltage = (wvfm->overDrive_Voltage * 1000) / 2122;

	dev_info(pDRV26xx->dev, "fb_brake_factor = 0x%02x\n", wvfm->fb_brake_factor);
	dev_info(pDRV26xx->dev, "hybrid_loop = 0x%02x:%s\n", wvfm->hybrid_loop,
		(wvfm->hybrid_loop == 1) ? "Enable" : "Disable");
	dev_info(pDRV26xx->dev, "auto_brake = 0x%02x:%s\n", wvfm->auto_brake,
		(wvfm->auto_brake == 1) ? "Enable" : "Disable");
	dev_info(pDRV26xx->dev, "auto_brake_standby = 0x%02x:%s\n", wvfm->auto_brake_standby,
		(wvfm->auto_brake_standby == 1) ? "Enable" : "Disable");
	dev_info(pDRV26xx->dev, "rated_Voltage = %u\n", wvfm->rated_Voltage);
	dev_info(pDRV26xx->dev, "overDrive_Voltage = %u\n", wvfm->overDrive_Voltage);
	dev_info(pDRV26xx->dev, "lra_f0 = %d\n", wvfm->lra_f0);
	dev_info(pDRV26xx->dev, "nWaveforms = %d\n", wvfm->nWaveforms);

	ram_wvfm_info = (struct drv26xx_RAMwaveform_info **)kzalloc(wvfm->nWaveforms,
		sizeof(struct drv26xx_RAMwaveform_info *));

	if (NULL == ram_wvfm_info) {
		dev_err(pDRV26xx->dev, "%s:%u:wvfm %u kalloc error!\n", __func__, __LINE__, i);
		wvfm->nWaveforms = 0;
		goto EXIT;
	}

	for (i = 0; i < wvfm->nWaveforms; i++) {
		if (cur + 3 > pFW->size) {
			dev_err(pDRV26xx->dev, "%s:%u:wvfm %u bin file error!\n", __func__, __LINE__, i);
			break;
		}
		ram_wvfm_info[i] = (struct drv26xx_RAMwaveform_info*)kzalloc(1,
			sizeof(struct drv26xx_RAMwaveform_info));
		if(NULL == ram_wvfm_info[i]) {
			dev_err(pDRV26xx->dev, "%s:%u:wvfm %u kalloc error!\n", __func__, __LINE__, i);
			wvfm->nWaveforms = (i > 0) ? i - 1 :
				0;
			break;
		}
		cur += 2;
		ram_wvfm_info[i]->duration = SMS_HTONS(buf[cur], buf[cur - 1]);
		cur++;
		ram_wvfm_info[i]->brake = drv_get_bf_value(DRV26XX_BF_WVFM_BRAKE, buf[cur]);
		ram_wvfm_info[i]->loop_mod = drv_get_bf_value(DRV26XX_BF_WVFM_LOOP_MOD, buf[cur]);
		ram_wvfm_info[i]->wv_shape = drv_get_bf_value(DRV26XX_BF_WVFM_WV_SHAPE, buf[cur]);
#if 1
		dev_info(pDRV26xx->dev, "wvfm = %u dur = %u\n", i, ram_wvfm_info[i]->duration);
		dev_info(pDRV26xx->dev, "ram_wvfm_info[%u]->brake = 0x%02x:%s\n",
			i, ram_wvfm_info[i]->brake,
			(ram_wvfm_info[i]->brake == 1) ? "Enable" : "Disable");
		dev_info(pDRV26xx->dev, "ram_wvfm_info[%u]->loop_mod = 0x%02x:%s\n",
			i, ram_wvfm_info[i]->loop_mod,
			(ram_wvfm_info[i]->loop_mod == 1) ?  "Open": "Close");
		dev_info(pDRV26xx->dev, "ram_wvfm_info[%u]->wv_shape = 0x%02x:%s\n",
			i, ram_wvfm_info[i]->wv_shape,
			(ram_wvfm_info[i]->wv_shape == 1) ? "Sine" : "Square");
#endif
	}
	cur++;
	rest_size = (int)pFW->size - cur;
	if (rest_size < DRV26XX_RAM_SIZE) {
		unsigned char* pBuf = &buf[cur];
		mutex_lock(&pDRV26xx->haptic_lock);
		drv2624_reg_write(pDRV26xx, DRV2624_R0XFD_RAM_ADDR_UPPER, 0);
		drv2624_reg_write(pDRV26xx, DRV2624_R0XFE_RAM_ADDR_LOWER, 0);
		for (i = 0; i < rest_size; i++)
			drv2624_reg_write(pDRV26xx,
				DRV2624_R0XFF_RAM_DATA, pBuf[i]);
		mutex_unlock(&pDRV26xx->haptic_lock);
		for (i = 0; i < (unsigned short)wvfm->nWaveforms; i++) {
			ram_wvfm_info[i]->mnEffectTimems = drv26xx_ram_get_effect_timems(pDRV26xx,
				(unsigned char)i, &buf[cur], rest_size);
		}
	}
	else {
		dev_err(pDRV26xx->dev,"%s, ERROR!! firmware size %d too big\n",
			__func__, (int)(pFW->size - cur));
	}
	wvfm->ram_wvfm_info = ram_wvfm_info;
EXIT:
	if (pFW) release_firmware(pFW);
}

static void drv2624_init(struct drv2624_data *pDRV2624)
{
	struct drv2624_platform_data *pDrv2624Platdata = &pDRV2624->msPlatData;
	struct actuator_data *pActuator = &(pDrv2624Platdata->msActuator);
	int nResult = 0;
	unsigned int DriveTime = 0;

	drv2624_set_bits(pDRV2624, DRV2624_R0X07, DRV2624_R0X07_TRIG_PIN_FUNC_MSK,
		DRV2624_R0X07_TRIG_PIN_FUNC_INT);

	drv2624_set_bits(pDRV2624, DRV2624_R0X08,
		DRV2624_R0X08_LRA_ERM_MSK |
		DRV2624_R0X08_AUTO_BRK_OL_MSK |
		DRV2624_R0X08_CTL_LOOP_MSK,
		(pActuator->mnActuatorType << DRV2624_R0X08_LRA_ERM_SFT) |
		(pDrv2624Platdata->mnLoop << DRV2624_R0X08_CTL_LOOP_SFT) |
		DRV2624_R0X08_AUTO_BRK_OL_EN);

	if (pActuator->mnActuatorType == LRA) {
		/*****************************************/
		/* DriveTime(ms) = 0.5 *(1/LRA) * 1000   */
		/* DriveTime(bit) = DriveTime(ms)*10 - 5 */
		/*****************************************/
		DriveTime = 5 * (1000 - pActuator->mnLRAFreq) / pActuator->mnLRAFreq;
		drv2624_set_bits(pDRV2624, DRV2624_R0X27,
			DRV2624_R0X27_DRIVE_TIME_MSK | DRV2624_R0X27_LRA_MIN_FREQ_SEL_MSK,
			DriveTime | ((pActuator->mnLRAFreq < 125)?DRV2624_R0X27_LRA_MIN_FREQ_SEL_45HZ:0));
		pActuator->openLoopPeriod =
			(unsigned short)((unsigned int)LRA_PERIOD_SCALE_UNIT /
					(OL_LRA_PERIOD_SCALE_NS * pActuator->mnLRAFreq));

		nResult = drv2624_set_bits(pDRV2624, DRV2624_R0X2E_OL_LRA_PERIOD_H, 0x03,
					 (pActuator->openLoopPeriod & 0x0300) >> 8);
		if (nResult < 0) {
			dev_err(pDRV2624->dev,
				"%s:%u set bits not Done nResult = %d\n",
				__func__, __LINE__, nResult);
			return;
		}
		nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X2F_OL_LRA_PERIOD_L,
					  (pActuator->openLoopPeriod & 0x00ff));
		if (nResult < 0) {
			dev_err(pDRV2624->dev,
				"%s:%u set bits not Done nResult = %d\n",
				__func__, __LINE__, nResult);
			return;
		}
	}
	drv2624_set_bits(pDRV2624, DRV2624_R0X29,
		DRV2624_R0X29_SAMPLE_TIME_MSK,
		pActuator->mnSampleTimeReg);
	drv2624_set_basic_reg(pDRV2624);
}

static void drv2624_vibrator_enable(struct led_classdev *led_cdev,
				enum led_brightness value)
{
	struct drv2624_data *pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
	hrtimer_cancel(&pDRV2624->haptics_timer);
	mutex_lock(&pDRV2624->haptic_lock);
	pDRV2624->state = value;
	pDRV2624->duration = value;
	mutex_unlock(&pDRV2624->haptic_lock);
	schedule_work(&pDRV2624->vibrator_work);
}

static ssize_t drv2624_state_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);

	return snprintf(buf, 16, "%d\n", pDRV2624->state);
}
static ssize_t drv2624_state_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("%s:%u!\n", __func__, __LINE__);
	return count;
}
static ssize_t drv2624_activate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);

	/* For now nothing to show */
	return snprintf(buf, 16, "%d\n", pDRV2624->state);
}
static ssize_t drv2624_activate_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
	unsigned int val = 0;
	int rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val != 0 && val != 1)
		return count;

	pr_info("%s: value=%d\n", __func__, val);

	mutex_lock(&pDRV2624->haptic_lock);
	hrtimer_cancel(&pDRV2624->haptics_timer);

	pDRV2624->state = val;
	mutex_unlock(&pDRV2624->haptic_lock);
	schedule_work(&pDRV2624->vibrator_work);

	return count;
}

static ssize_t drv2624_duration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);

	ktime_t time_rem;
	s64 time_ms = 0;

	if (hrtimer_active(&pDRV2624->haptics_timer)) {
		time_rem = hrtimer_get_remaining(&pDRV2624->haptics_timer);
		time_ms = ktime_to_ms(time_rem);
	}

	return snprintf(buf, 16, "%lld\n", time_ms);

}

static ssize_t drv2624_duration_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
	int value;
	int rc = kstrtouint(buf, 0, &value);

	if (rc < 0) {
		dev_err(pDRV2624->dev, "%s:%u: rc = %d\n", __func__,
				__LINE__, rc);
		return rc;
	}
	dev_info(pDRV2624->dev, "%s:%u: duration = %d, buf %s", __func__,
				__LINE__, value, buf);
	/* setting 0 on duration is NOP for now */
	if (value <= 0) return count;
	 else if(value > 0 && value <=20) pDRV2624->waveform_id = 3;
	 		else if(value > 20 && value <=30) pDRV2624->waveform_id = 2;
					else if(value > 30 && value <=60) pDRV2624->waveform_id = 1;
							else pDRV2624->waveform_id = 4;

	pDRV2624->duration = value;

	return count;
}

/**
 *
 * bRTP = NO == 0; Enable all interrupt of DRV2624
 * bRTP = 1 == 1; Only Enable critical interrupt,  PROCESS_DONE and PRG_ERROR
 *
 **/
static int drv2624_enableIRQ(struct drv2624_data *pDRV2624)
{
	int nResult = 0;
	unsigned char bitSet = (pDRV2624->bRTPmode == false) ? DRV2624_R0X02_INTZ_ENABLE :
		(DRV2624_R0X02_INTZ_ENABLE|DRV2624_R0X02_INTZ_PROCESS_DONE_DISABLE);

	if(gpio_is_valid(pDRV2624->msPlatData.mnGpioINT)) {
		if (pDRV2624->mbIRQEnabled)
			goto end;

		nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X01_STATUS);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: I2C read error\n", __func__,
				__LINE__);
			goto end;
		}
		nResult = drv2624_set_bits(pDRV2624,
					DRV2624_R0X02_INTZ,
					DRV2624_R0X02_INTZ_MSK, bitSet);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: I2C set bit error\n", __func__,
				__LINE__);
			goto end;
		}
		enable_irq(pDRV2624->mnIRQ);
		pDRV2624->mbIRQEnabled = true;
	}
end:
	return nResult;
}

static void drv2624_disableIRQ(struct drv2624_data *pDRV2624)
{
	int nResult = 0;
	if(gpio_is_valid(pDRV2624->msPlatData.mnGpioINT)) {
		if(pDRV2624->mbIRQEnabled) {
			disable_irq_nosync(pDRV2624->mnIRQ);
			drv2624_reg_write(pDRV2624, DRV2624_R0X02_INTZ,
				DRV2624_R0X02_INTZ_DISABLE);
			nResult = drv2624_set_bits(pDRV2624,
						DRV2624_R0X02_INTZ,
						DRV2624_R0X02_INTZ_MSK, DRV2624_R0X02_INTZ_DISABLE);
			if (nResult < 0) {
				dev_err(pDRV2624->dev, "%s:%u: I2C set bit error\n", __func__,
					__LINE__);
			} else pDRV2624->mbIRQEnabled = false;
		}
	}
}

static int drv2624_set_go_bit(struct drv2624_data *pDRV2624, unsigned char val)
{
	int nResult = 0, value = 0;
	int retry = DRV26XX_GO_BIT_MAX_RETRY_CNT;

	val &= DRV2624_R0X0C_GO_MSK;
	dev_info(pDRV2624->dev, "%s, go val = %d\n", __func__, val);
	nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X0C_GO, val);
	if (nResult < 0)
		goto end;

	usleep_range(DRV26XX_GO_BIT_CHECK_INTERVAL, DRV26XX_GO_BIT_CHECK_INTERVAL);
	do {
		value = drv2624_reg_read(pDRV2624, DRV2624_R0X0C_GO);
		if (value < 0) {
			nResult = value;
			break;
		}
		if ((value & DRV2624_R0X0C_GO_MSK) == val) break;
		else {
			dev_info(pDRV2624->dev, "%s, GO bit %d\n", __func__, value);

			nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X0C_GO, val);
 			if (nResult < 0) break;
			usleep_range(DRV26XX_GO_BIT_CHECK_INTERVAL, DRV26XX_GO_BIT_CHECK_INTERVAL);
		}
		retry--;
	} while (retry > 0);

	if(nResult == 0) {
		if (retry != 0) {
			pDRV2624->mbWork = (0x0 == val)?false:true;
			if (val) {
				nResult = drv2624_enableIRQ(pDRV2624);
			} else {
				drv2624_disableIRQ(pDRV2624);
			}
			dev_info(pDRV2624->dev, "%s:%u: retry = %d, value = %d pull go bit success!\n",
				__func__, __LINE__, retry, value);
		} else {
			if(val) {
				dev_err(pDRV2624->dev, "%s:%u: retry = %d, Set go bit to %d failed!\n",
					__func__, __LINE__, retry, val);
			} else {
				if(pDRV2624->bRTPmode == false) {
#if 1
					value = drv2624_reg_read(pDRV2624, DRV2624_R0X07);
					if (value < 0) {
						dev_err(pDRV2624->dev, "%s:%u: I2C read error\n", __func__,
							__LINE__);
						goto end;
					}
					nResult = drv2624_set_bits(pDRV2624, DRV2624_R0X07, DRV2624_R0X07_MODE_MSK,
						DRV2624_R0X07_MODE_RTP_MODE);
					if (nResult < 0) {
						dev_err(pDRV2624->dev, "%s:%u: I2C set error\n", __func__,
							__LINE__);
						goto end;
					}
#else
					/* set device to standby mode */
					drv2624_set_bits(pDRV2624, DRV2624_R0X08,
						 DRV2624_R0X08_AUTO_BRK_INTO_STBY_MSK,
						 DRV2624_R0X08_WITHOUT_AUTO_BRK_INTO_STBY);
#endif
					nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X0C_GO, val);
 					if (nResult < 0) {
						dev_err(pDRV2624->dev, "%s:%u: I2C write error\n", __func__,
							__LINE__);
						goto end;
					}
#if 1
					nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X07, value);
					if (nResult < 0) {
						dev_err(pDRV2624->dev, "%s:%u: I2C write error\n", __func__,
							__LINE__);
						goto end;
					}
#endif
				} else {
					dev_err(pDRV2624->dev, "%s:%u: retry = %d, Set go bit to %d failed!\n",
						__func__, __LINE__, retry, val);
				}
			}
		}
	}

end:
	return nResult;
}

static int drv262x_auto_calibrate(struct drv2624_data *pDRV2624)
{
	int nResult = 0;
	int retry = 9;
	struct drv2624_platform_data *pDrv2624Platdata = &pDRV2624->msPlatData;
	struct actuator_data *pActuator = &(pDrv2624Platdata->msActuator);

	dev_info(pDRV2624->dev, "%s enter!\n", __func__);
	mutex_lock(&pDRV2624->haptic_lock);
/**
 * Set MODE register to Auto Level Calibration Routine
 * and choose Trigger Function Internal
 **/
	nResult = drv2624_set_bits(pDRV2624,
		DRV2624_R0X07, DRV2624_R0X07_MODE_MSK,
		DRV2624_R0X07_MODE_AUTO_LVL_CALIB_RTN);/*0x4B*/
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s:%u set bits not Done nResult = %d\n",
			__func__, __LINE__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}
	nResult = drv2624_set_bits(pDRV2624, DRV2624_R0X2E_OL_LRA_PERIOD_H, 0x03,
				 (pActuator->openLoopPeriod & 0x0300) >> 8);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s:%u set bits not Done nResult = %d\n",
			__func__, __LINE__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}
	nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X2F_OL_LRA_PERIOD_L,
				  (pActuator->openLoopPeriod & 0x00ff));
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s:%u set bits not Done nResult = %d\n",
			__func__, __LINE__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}
	nResult = drv2624_set_bits(pDRV2624,
		DRV2624_R0X08, DRV2624_R0X08_CTL_LOOP_MSK,
		DRV2624_R0X08_CTL_LOOP_OPEN_LOOP);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s:%u set bits not Done nResult = %d\n",
			__func__, __LINE__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}

	nResult = drv2624_set_bits(pDRV2624,
		DRV2624_R0X2A, DRV2624_R0X2A_AUTO_CAL_TIME_MSK,
		DRV2624_R0X2A_AUTO_CAL_TIME_TRIGGER_CRTLD);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s:%u set bits not Done nResult = %d\n",
			__func__, __LINE__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}

	nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X1F_RATED_VOLTAGE,
		pActuator->mnCalRatedVoltage);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s: mnCalRatedVoltage set failed nResult = %d\n",
			__func__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}

	nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X20_OD_CLAMP,
		pActuator->mnCalOverDriveClampVoltage);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s: mnCalOverDriveClampVoltage set failed nResult = %d\n",
			__func__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}

	nResult = drv2624_set_go_bit(pDRV2624, DRV2624_R0X0C_GO_BIT);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s: calibrate go bit not Done nResult = %d\n",
			__func__, nResult);
		mutex_unlock(&pDRV2624->haptic_lock);
		goto end;
	}
	mutex_unlock(&pDRV2624->haptic_lock);
	while(retry && pDRV2624->mbWork == true) {
		retry--;
		msleep(200); /* waiting auto calibration finished */
	}

end:
	if (nResult < 0)
		dev_err(pDRV2624->dev, "%s: Calibtion Done nResult = %d\n",
			__func__, nResult);
	return nResult;
}

static int drv262x_update_f0(struct drv2624_data *pDRV2624)
{
	int nResult = -1;
	if(pDRV2624) {
		dev_info(pDRV2624->dev, "%s: enter!\n", __func__);

		nResult =
		    drv2624_reg_write(pDRV2624, DRV2624_R0X2E_OL_LRA_PERIOD_H,
		    	pDRV2624->mAutoCalData.mnOL_LraPeriod.msb);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: write f0_msb failed\n", __func__, __LINE__);
			goto end;
		}

		nResult =
		    drv2624_reg_write(pDRV2624, DRV2624_R0X2F_OL_LRA_PERIOD_L,
		    	pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: write f0_lsb failed\n", __func__, __LINE__);
			goto end;
		}
		nResult = 0;
	} else {
		dev_err(pDRV2624->dev, "%s:%u:drv2624_data is NULL !\n", __func__, __LINE__);
	}
end:
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s:%u: Failed to update f0 !\n", __func__, __LINE__);
	} else {
		dev_info(pDRV2624->dev, "%s:%u: F0 is updated OL_MSB:%d OL_LSB:%d\n",
			__func__, __LINE__, pDRV2624->mAutoCalData.mnOL_LraPeriod.msb,
			pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb);
	}
	return nResult;
}

static int drv2624_get_calibration_result(struct drv2624_data *pDRV2624)
{
	int nResult = -1, calibrated_f0 = 0, ol_lra_reg = 0, lra_reg = 0;
	struct drv26xx_RAMwaveforms* wvfm = NULL;

	if(NULL == pDRV2624) {
		pr_err( "%s:%u:drv2624_data is NULL !\n", __func__, __LINE__);
		goto end;
	}
	dev_info(pDRV2624->dev, "%s: enter!\n", __func__);
	wvfm = &(pDRV2624->ramwvfm);
	nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X01_STATUS);
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s: nResult = %d\n", __func__, nResult);
		goto end;
	}
	pDRV2624->mAutoCalData.mnDoneFlag = (nResult & DRV2624_R0X01_STATUS_DIAG_RESULT_MSK);

	if(pDRV2624->mAutoCalData.mnDoneFlag) {
		dev_err(pDRV2624->dev, "%s:%u: calibration failed\n", __func__, __LINE__);
		goto end;
	}
	nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X21_CAL_COMP);
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s:%u: read cal_comp failed\n", __func__, __LINE__);
		goto end;
	}
	pDRV2624->mAutoCalData.mnCalComp = nResult;

	nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X22_CAL_BEMF);
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s:%u: read cal_bemf failed\n", __func__, __LINE__);
		goto end;
	}
	pDRV2624->mAutoCalData.mnCalBemf = nResult;

	nResult =
		drv2624_reg_read(pDRV2624, DRV2624_R0X23) & DRV2624_R0X23_BEMF_GAIN_MSK;
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s:%u: read cal_gain failed\n", __func__, __LINE__);
		goto end;
	}
	pDRV2624->mAutoCalData.mnCalGain = nResult;
	nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X05_LRA_PERIOD_H);
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s:%u: read f0_msb failed\n", __func__, __LINE__);
		goto end;
	}
	pDRV2624->mAutoCalData.mnLraPeriod.msb =
		nResult & DRV2624_R0X05_LRA_PERIOD_H_MSK;

	nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X06_LRA_PERIOD_L);
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s:%u: read f0_lsb failed\n", __func__, __LINE__);
		goto end;
	}
	pDRV2624->mAutoCalData.mnLraPeriod.lsb = nResult;

	lra_reg = pDRV2624->mAutoCalData.mnLraPeriod.msb << 8 | pDRV2624->mAutoCalData.mnLraPeriod.lsb;

	if(lra_reg) {

		calibrated_f0 = LRA_PERIOD_SCALE_UNIT/(lra_reg * LRA_PERIOD_SCALE_NS);
#if 1
		ol_lra_reg = (OL_LRA_PERIOD_FACTOR*lra_reg*LRA_PERIOD_SCALE_NS)/OL_LRA_PERIOD_SCALE_NSX2;
		if(wvfm && wvfm->lra_f0 & FRES_FACTOR) {
			ol_lra_reg += wvfm->lra_f0;
		} else {
			ol_lra_reg+=1;
		}
#else
		ol_lra_reg = OL_LRA_PERIOD_SCALE_UNIT/(calibrated_f0 * OL_LRA_PERIOD_SCALE_NS);
#endif
		pDRV2624->mAutoCalData.mnOL_LraPeriod.msb =
			(ol_lra_reg >> 8) & DRV2624_R0X2E_OL_LRA_PERIOD_H_MSK;

		pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb =
			ol_lra_reg & DRV2624_R0X2F_OL_LRA_PERIOD_L_MSK;

		dev_info(pDRV2624->dev, "%s:%u: calibrated_f0 = %d lra_period_h = 0x%02x "
			"lra_period_l = 0x%02x ol_lra_period_h = 0x%02x ol_lra_period_l = 0x%02x\n",
			__func__, __LINE__, calibrated_f0,
			pDRV2624->mAutoCalData.mnLraPeriod.msb,
			pDRV2624->mAutoCalData.mnLraPeriod.lsb,
			pDRV2624->mAutoCalData.mnOL_LraPeriod.msb,
			pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb);
	}else {
		dev_err(pDRV2624->dev,"%s:%u: f0 is zero\n", __func__, __LINE__);
	}
end:
	nResult = drv2624_set_go_bit(pDRV2624, DRV2624_R0X0C_NGO_BIT);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s: cali NGO bit set failed, nResult = %d\n",
			__func__, nResult);
	}
	/* updata f0*/
	if(ol_lra_reg) {
		nResult = drv262x_update_f0(pDRV2624);
		pDRV2624->mAutoCalData.mnCnt++;
	}
	return nResult;
}

static int drv2624_get_diag_result(struct drv2624_data *pDRV2624)
{
	int nResult = -1;

	if(pDRV2624) {
		nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X01_STATUS);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: Diagnostic fail\n", __func__, __LINE__);
			goto end;
		}
		pDRV2624->mDiagResult.mnResult = nResult & DRV2624_R0X01_STATUS_DIAG_RESULT_MSK;
		if (pDRV2624->mDiagResult.mnResult) {
			dev_err(pDRV2624->dev, "%s:%u: Diagnostic fail\n", __func__, __LINE__);
			goto end;
		} else {
			nResult = drv2624_reg_read(pDRV2624, DRV2624_RX03_DIAG_Z_RESULT);
			if (nResult < 0) {
				dev_err(pDRV2624->dev, "%s:%u: Read DIAG_Z fail\n", __func__, __LINE__);
				goto end;
			}
			pDRV2624->mDiagResult.mnDiagZ = nResult;

			nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X30_CURRENT_K);
			if (nResult < 0) {
				dev_err(pDRV2624->dev, "%s:%u: Read Current_K fail\n", __func__, __LINE__);
				goto end;
			}
			pDRV2624->mDiagResult.mnCurrentK = nResult;

			pDRV2624->mDiagResult.mnRemohm = 478430 * pDRV2624->mDiagResult.mnDiagZ /
					(4 * pDRV2624->mDiagResult.mnCurrentK + 719);
			pDRV2624->mDiagResult.mnCnt++;
			dev_info(pDRV2624->dev,
				"%s: ZResult=0x%02x, CurrentK=0x%02x, Re = %d mohm\n",
				__func__, pDRV2624->mDiagResult.mnDiagZ,
				pDRV2624->mDiagResult.mnCurrentK, pDRV2624->mDiagResult.mnRemohm);
			nResult = 0;
		}
	}else {
		dev_err(pDRV2624->dev, "%s:%u: drv2624_data is NULL\n", __func__, __LINE__);
	}
end:
	return nResult;
}

static int drv262x_run_diagnostics(struct drv2624_data *pDRV2624)
{
	int nResult = 0;
	dev_info(pDRV2624->dev, "%s\n", __func__);
	nResult = drv2624_set_bits(pDRV2624,
		DRV2624_R0X07, DRV2624_R0X07_MODE_MSK,
		DRV2624_R0X07_MODE_DIAG_RTN);
	if (nResult < 0) {
		dev_err(pDRV2624->dev, "%s:%u: Diag start failed\n", __func__, __LINE__);
		goto end;
	}
	drv2624_reg_read(pDRV2624, DRV2624_R0X07);

	nResult = drv2624_set_go_bit(pDRV2624, DRV2624_R0X0C_GO_BIT);
	if (nResult < 0) {
		dev_err(pDRV2624->dev,
			"%s:%u: calibrate go bit not Done nResult = %d\n",
			__func__, __LINE__, nResult);
		goto end;
	}
end:
	return nResult;
}

/*show calibrtion*/
static ssize_t drv2624_calib_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			//mutex_lock(&pDRV2624->haptic_lock);
			dev_info(pDRV2624->dev,
				"%s:%u: Cal_Result: 0x%02x, mnCnt: %u, CalComp: 0x%02x, CalBemf: 0x%02x, "
				"CalGain: 0x%02x, f0_data_msb:0x%02x, f0_data_lsb:0x%02x\n",
				__func__, __LINE__, pDRV2624->mAutoCalData.mnDoneFlag,
				pDRV2624->mAutoCalData.mnCnt,
				pDRV2624->mAutoCalData.mnCalComp,
				pDRV2624->mAutoCalData.mnCalBemf,
				pDRV2624->mAutoCalData.mnCalGain,
				pDRV2624->mAutoCalData.mnOL_LraPeriod.msb,
				pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb);

			switch (pDRV2624->mAutoCalData.mnDoneFlag) {
				case DRV2624_R0X01_STATUS_DIAG_RESULT_NOK:
					len = scnprintf(buf, 32, "calibration failed!\n");
					break;
				case 0xFF:
					len = scnprintf(buf, 32, "non-calibration!\n");
					break;
				default:
					len = scnprintf(buf, 128, "Calibration data is from 0x%02x, CalComp:0x%02x, "
						"CalBemf:0x%02x, CalGain:0x%02x, f0_data_msb:0x%02x, f0_data_lsb:0x%02x\n",
						pDRV2624->mAutoCalData.mnDoneFlag,
						pDRV2624->mAutoCalData.mnCalComp,
						pDRV2624->mAutoCalData.mnCalBemf,
						pDRV2624->mAutoCalData.mnCalGain,
						pDRV2624->mAutoCalData.mnOL_LraPeriod.msb,
						pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb);
					break;
			}
			//mutex_unlock(&pDRV2624->haptic_lock);
		}else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	}else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

/**
 * store calibration
 *
 **/
static ssize_t drv2624_calib_store (
		struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			pDRV2624->bRTPmode = false;
			nResult = drv262x_auto_calibrate(pDRV2624);

			if (nResult<0) {
				dev_err(pDRV2624->dev,"%s:%u: run calibrate err nResult=%d\n",
					__func__, __LINE__, nResult);
			} else {
				if(pDRV2624->mbWork == true) {
					mutex_lock(&pDRV2624->haptic_lock);
					nResult = drv2624_get_calibration_result(pDRV2624);
					mutex_unlock(&pDRV2624->haptic_lock);
					if (nResult<0) {
						dev_err(pDRV2624->dev,"%s: %u: get calibration result err nResult=%d\n",
							__func__,__LINE__, nResult);
					}
				} else {
					pDRV2624->mAutoCalData.mnDoneFlag = DRV2624_R0X01_STATUS_DIAG_RESULT_NOK;
					dev_err(pDRV2624->dev,"%s: %u: get calibration failed probably by interrupt\n",
							__func__,__LINE__);
				}
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}
	return count;
}

static ssize_t drv2624_diag_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	int retry = 9;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			mutex_lock(&pDRV2624->haptic_lock);
			pDRV2624->bRTPmode = false;
			nResult = drv262x_run_diagnostics(pDRV2624);
			if (nResult<0) {
				dev_err(pDRV2624->dev,"%s: run diag err nResult=%d\n",__func__, nResult);
			}
			mutex_unlock(&pDRV2624->haptic_lock);

			while(retry && pDRV2624->mbWork == true) {
				retry--;
				msleep(200); /* waiting auto calibration finished */
			}
			if(pDRV2624->mbWork == true) {
				mutex_lock(&pDRV2624->haptic_lock);
				nResult = drv2624_get_diag_result(pDRV2624);

				nResult = drv2624_set_go_bit(pDRV2624, DRV2624_R0X0C_NGO_BIT);
				if (nResult < 0) {
					dev_err(pDRV2624->dev,
						"%s: diag go bit not Done nResult = %d\n",
						__func__, nResult);
				}
				mutex_unlock(&pDRV2624->haptic_lock);
			} else {
				pDRV2624->mDiagResult.mnResult = DRV2624_R0X01_STATUS_DIAG_RESULT_NOK;
				dev_err(pDRV2624->dev,"%s: %u: diag failed probably by interrupt\n",
					__func__,__LINE__);
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return count;
}

static ssize_t
drv2624_diag_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			//mutex_lock(&pDRV2624->haptic_lock);
			dev_info(pDRV2624->dev,
				"%s:%u: mnResult: 0x%x, mnCnt: 0x%x,\
				mnDiagZ: 0x%x, mnCurrentK:0x%x,mnRemohm:%u mohm\n",
				__func__, __LINE__, pDRV2624->mDiagResult.mnResult,
				pDRV2624->mDiagResult.mnCnt,
				pDRV2624->mDiagResult.mnDiagZ,
				pDRV2624->mDiagResult.mnCurrentK,
				pDRV2624->mDiagResult.mnRemohm);
			len = scnprintf(buf, 50, "%x %x %u\n",
				pDRV2624->mDiagResult.mnDiagZ,
				pDRV2624->mDiagResult.mnCurrentK,
				pDRV2624->mDiagResult.mnRemohm);
			//mutex_unlock(&pDRV2624->haptic_lock);
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

static ssize_t
drv2624_reg_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;
	const int size = PAGE_SIZE;
	int n_result = 0;
	int i = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			for (i = 0; i <= 0x30; i++) {
				n_result = drv2624_reg_read(pDRV2624, i);
				if (n_result < 0) {
					dev_info(pDRV2624->dev,
						"%s:%u: read register failed\n", __func__, __LINE__);
					len += scnprintf(buf+len, size-len, "Read Register0x%02x failed\n", i);
					break;
				}
				//12 bytes
				if(len + 12 <= size) {
					len += scnprintf(buf+len, size-len, "R0x%02x:0x%02x\n",
						i, n_result);
				} else {
					dev_info(pDRV2624->dev,"%s:%u: mem is not enough: PAGE_SIZE = %lu\n",
						 __func__,__LINE__, PAGE_SIZE);
					break;
				}
			}
			if(n_result >= 0) {
				for (i = 0xFD; i <= 0xFF; i++) {
					n_result = drv2624_reg_read(pDRV2624, i);
					if (n_result < 0) {
						dev_info(pDRV2624->dev,
						"%s:%u: read register failed\n", __func__, __LINE__);
						len += scnprintf(buf+len, size-len, "Read Register0x%02x failed\n", i);
						break;
					}
				//12 bytes
					if(len + 12 <= size) {
						len += scnprintf(buf+len, size-len, "R0x%02x:0x%02x\n",
							i, n_result);
					} else {
						dev_info(pDRV2624->dev,"%s:%u: mem is not enough: PAGE_SIZE = %lu\n",
							 __func__,__LINE__, PAGE_SIZE);
						break;
					}
				}
			}
	  } else {
		  pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
	  }
  } else {
	  pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
  }

  return len;
}

static ssize_t drv2624_reg_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	unsigned char databuf[2] = {0, 0};

	if(led_cdev && count > 2) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			if (sscanf(buf, "0x%hhx 0x%hhx", &databuf[0], &databuf[1]) == 2) {
				nResult = drv2624_reg_write(pDRV2624,
						(unsigned char)databuf[0],
						(unsigned char)databuf[1]);
				if(nResult) {
					pr_err("%s:%u: I2C error!\n", __func__, __LINE__);
				}
			} else {
				pr_err("%s:%u: can't scan the value\n",
					__func__, __LINE__);
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return nResult;
}

static ssize_t
drv2624_CalComp_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			switch (pDRV2624->mAutoCalData.mnDoneFlag) {
				case DRV2624_R0X01_STATUS_DIAG_RESULT_NOK:
					len = scnprintf(buf, 32, "calibration failed!\n");
					break;
				case 0xFF:
					len = scnprintf(buf, 32, "non-calibration!\n");
					break;
				default:
					len = scnprintf(buf, 128, "Calibration data is from %s\n"
						"CalComp:0x%x\n",
						(DRV2624_R0X01_STATUS_DIAG_RESULT_OK == pDRV2624->mAutoCalData.mnDoneFlag)?
						"calibration":"file",
						pDRV2624->mAutoCalData.mnCalComp);
					break;
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

static ssize_t drv2624_CalComp_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	unsigned char data = 0x0;

	if(led_cdev && count > 2) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			if (sscanf(buf, "0x%hhx", &data) == 1) {
				pDRV2624->mAutoCalData.mnDoneFlag = 0x1;

				nResult = drv2624_reg_write(pDRV2624,
					DRV2624_R0X21_CAL_COMP, data);
				if(nResult) {
					pr_err("%s:%u: I2C error!\n", __func__, __LINE__);
				} else {
					pDRV2624->mAutoCalData.mnCalComp = data;
				}
			} else {
				pr_err("%s:%u: can't scan the value\n",
							__func__, __LINE__);
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return count;
}

static ssize_t
drv2624_CalBemf_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			switch (pDRV2624->mAutoCalData.mnDoneFlag) {
				case DRV2624_R0X01_STATUS_DIAG_RESULT_NOK:
					len = scnprintf(buf, 32, "calibration failed!\n");
					break;
				case 0xFF:
					len = scnprintf(buf, 32, "non-calibration!\n");
					break;
				default:
					len = scnprintf(buf, 128, "Calibration data is from %s\n"
						"CalBemf:0x%x\n",
						(DRV2624_R0X01_STATUS_DIAG_RESULT_OK == pDRV2624->mAutoCalData.mnDoneFlag)?
						"calibration":"file",
						pDRV2624->mAutoCalData.mnCalBemf);
					break;
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

static ssize_t drv2624_CalBemf_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	unsigned char data = 0x0;

	if(led_cdev && count > 2) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			if (sscanf(buf, "0x%hhx", &data) == 1) {
				pDRV2624->mAutoCalData.mnDoneFlag = 0x1;

				nResult = drv2624_reg_write(pDRV2624,
					DRV2624_R0X22_CAL_BEMF, data);
				if(nResult) {
					pr_err("%s:%u: I2C error!\n", __func__, __LINE__);
				} else {
					pDRV2624->mAutoCalData.mnCalBemf = data;
				}
			} else {
				pr_err("%s:%u: can't scan the value\n",
							__func__, __LINE__);
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return count;
}

static ssize_t
drv2624_CalGain_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			switch (pDRV2624->mAutoCalData.mnDoneFlag) {
				case DRV2624_R0X01_STATUS_DIAG_RESULT_NOK:
					len = scnprintf(buf, 32, "calibration failed!\n");
					break;
				case 0xFF:
					len = scnprintf(buf, 32, "non-calibration!\n");
					break;
				default:
					len = scnprintf(buf, 128, "Calibration data is from %s\n"
						"CalGain:0x%x\n",
						(DRV2624_R0X01_STATUS_DIAG_RESULT_OK == pDRV2624->mAutoCalData.mnDoneFlag)?
						"calibration":"file", pDRV2624->mAutoCalData.mnCalGain);
					break;
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

static ssize_t drv2624_CalGain_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	unsigned char data = 0x0;

	if(led_cdev && count > 1) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			if (sscanf(buf, "0x%hhx", &data) == 1) {
				pDRV2624->mAutoCalData.mnDoneFlag = 0x1;

				nResult = drv2624_set_bits(pDRV2624,
					DRV2624_R0X23, DRV2624_R0X23_BEMF_GAIN_MSK, data);
				if(nResult) {
					pr_err("%s:%u: I2C error!\n", __func__, __LINE__);
				} else {
					pDRV2624->mAutoCalData.mnCalGain = data;
				}
			} else {
				pr_err("%s:%u: can't scan the value\n",
							__func__, __LINE__);
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return count;
}

static ssize_t
drv2624_f0_msb_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			switch (pDRV2624->mAutoCalData.mnDoneFlag) {
				case DRV2624_R0X01_STATUS_DIAG_RESULT_NOK:
					len = scnprintf(buf, 32, "calibration failed!\n");
					break;
				case 0xFF:
					len = scnprintf(buf, 32, "non-calibration!\n");
					break;
				default:
					len = scnprintf(buf, 128, "Calibration data is from %s\n"
						"f0_msb:0x%x\n",
						(DRV2624_R0X01_STATUS_DIAG_RESULT_OK == pDRV2624->mAutoCalData.mnDoneFlag)?
						"calibration":"file", pDRV2624->mAutoCalData.mnOL_LraPeriod.msb);
					break;
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

static ssize_t drv2624_f0_msb_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	unsigned char data = 0x0;

	if(led_cdev && count > 1) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			if (sscanf(buf, "0x%hhx", &data) == 1) {
				pDRV2624->mAutoCalData.mnDoneFlag = 0x1;

				nResult = drv2624_set_bits(pDRV2624,
					DRV2624_R0X2E_OL_LRA_PERIOD_H,
					DRV2624_R0X2E_OL_LRA_PERIOD_H_MSK, data);
				if(nResult) {
					pr_err("%s:%u: I2C error!\n", __func__, __LINE__);
				} else {
					pDRV2624->mAutoCalData.mnOL_LraPeriod.msb = data;
				}
			} else {
				pr_err("%s:%u: can't scan the value\n",
							__func__, __LINE__);
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return count;
}

static ssize_t
drv2624_f0_lsb_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			switch (pDRV2624->mAutoCalData.mnDoneFlag) {
				case DRV2624_R0X01_STATUS_DIAG_RESULT_NOK:
					len = scnprintf(buf, 32, "calibration failed!\n");
					break;
				case 0xFF:
					len = scnprintf(buf, 32, "non-calibration!\n");
					break;
				default:
					len = scnprintf(buf, 128, "Calibration data is from %s\n"
						"f0_lsb:0x%x\n",
						(DRV2624_R0X01_STATUS_DIAG_RESULT_OK == pDRV2624->mAutoCalData.mnDoneFlag)?
						"calibration":"file", pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb);
					break;
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

static ssize_t drv2624_f0_lsb_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	unsigned char data = 0x0;

	if(led_cdev && count > 1) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			if (sscanf(buf, "0x%hhx", &data) == 1) {
				pDRV2624->mAutoCalData.mnDoneFlag = 0x1;

				nResult = drv2624_reg_write(pDRV2624,
					DRV2624_R0X2F_OL_LRA_PERIOD_L, data);
				if(nResult) {
					pr_err("%s:%u: I2C error!\n", __func__, __LINE__);
				} else {
					pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb = data;
				}
			} else {
				pr_err("%s:%u: can't scan the value\n",
							__func__, __LINE__);
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return count;
}

static ssize_t
drv2624_f0_data_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			switch (pDRV2624->mAutoCalData.mnDoneFlag) {
				case DRV2624_R0X01_STATUS_DIAG_RESULT_NOK:
					len = scnprintf(buf, 32, "calibration failed!\n");
					break;
				case 0xFF:
					len = scnprintf(buf, 32, "non-calibration!\n");
					break;
				default: {
					int f0 = (pDRV2624->mAutoCalData.mnOL_LraPeriod.msb << 8
						| pDRV2624->mAutoCalData.mnOL_LraPeriod.lsb) * OL_LRA_PERIOD_SCALE_NSX2;
					if(f0 != 0) {
						f0 = OL_LRA_PERIOD_SCALE_UNIT/f0;
						len = scnprintf(buf, 128, "f0 data is from %s, f0 = %d\n",
							(DRV2624_R0X01_STATUS_DIAG_RESULT_OK == pDRV2624->mAutoCalData.mnDoneFlag)?
							"calibration":"file", f0);
					} else {
						len = scnprintf(buf, 128, "f0 data error, mnOL_LraPeriod is zero!\n");
					}
				}
					break;
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

static ssize_t
drv2624_gain_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	ssize_t len = 0;

	if(led_cdev) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			switch (pDRV2624->gain) {
				case 0:
					len = scnprintf(buf, 16, "0\n");
					break;
				case 1:
					len = scnprintf(buf, 16, "1\n");
					break;
				case 2:
					len = scnprintf(buf, 16, "2\n");
					break;
#if 0
				case 3:
					len = scnprintf(buf, 32, "3\n");
					break;
#endif
				default:
					len = scnprintf(buf, 32, "Invalid Gain!\n");
					break;
			}
		} else {
			pr_err("%s:%u: drv2624 is NULL\n", __func__, __LINE__);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL\n", __func__, __LINE__);
	}
	return len;
}

ssize_t i2c_address_show(struct device *dev,
	  struct device_attribute *attr, char *buf)
{
   struct led_classdev *led_cdev = dev_get_drvdata(dev);
   struct drv2624_data *pDRV2624 = NULL;
   struct i2c_client *pClient = NULL;
   const int size = 32;
   int n = 0;
   unsigned short addr = 0;
   if( led_cdev != NULL) {
   		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
			pClient = pDRV2624->client;
			addr = pClient->addr;
		}
	}
   n += scnprintf(buf, size, "Active SmartPA-0x%02x\n\r", addr);

   return n;

}

static ssize_t drv2624_gain_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = -1;
	unsigned int val;
	
	nResult = kstrtouint(buf, 0, &val);
	if (nResult < 0)
		return nResult;

	if(led_cdev && count >= 1) {
		pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);
		if(pDRV2624) {
				switch(val){
					
					case 0:
						nResult = drv2624_set_bits(pDRV2624, DRV2624_R0X0D,DRV2624_R0X0D_DIG_MEM_GAIN_MASK,
											DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_100);
					break;
					
					#if 0
					case 0:
						nResult = drv2624_set_bits(pDRV2624, DRV2624_R0X0D,DRV2624_R0X0D_DIG_MEM_GAIN_MASK,
											DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_75);
					break;
					#endif
					case 1:
						nResult = drv2624_set_bits(pDRV2624, DRV2624_R0X0D,DRV2624_R0X0D_DIG_MEM_GAIN_MASK,
											DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_50);
					break;

					case 2:
						nResult = drv2624_set_bits(pDRV2624, DRV2624_R0X0D,DRV2624_R0X0D_DIG_MEM_GAIN_MASK,
											DRV2624_R0X0D_DIG_MEM_GAIN_STRENGTH_25);
					break;
					default:
						pr_err("%s:%u: Invalid haptic strength num!\n",__func__, __LINE__);
					break;
				}
				if (nResult == 0)
					pDRV2624->gain = val;
			
		} else {
			pr_err("%s:%u: drv2624 is NULL nResult=%d\n",
				__func__, __LINE__, nResult);
		}
	} else {
		pr_err("%s:%u: led_dev is NULL nResult=%d\n",
			__func__, __LINE__, nResult);
	}

	return count;
}

static ssize_t rambininfo_list_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct drv2624_data *pDRV2624 = NULL;
	struct drv26xx_RAMwaveforms* wvfm = NULL;
	struct drv26xx_RAMwaveform_info** ram_wvfm_info = NULL;
	int n = 0, i = 0;

	if(led_cdev == NULL) {
		if(n + 128 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE -n, "%s:%u:ERROR: Can't find led_cdev handle!\n",
				__func__, __LINE__);
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n",
				__LINE__, __func__);
			n = PAGE_SIZE;
		}
		goto EXIT;
	}

	pDRV2624 = container_of(led_cdev, struct drv2624_data, led_dev);

	if (pDRV2624==NULL) {
		if(n + 128 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE -n, "%s:%u:ERROR: Can't find drv2624_data handle!\n",
				__func__, __LINE__);
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n",
				__LINE__, __func__);
			n = PAGE_SIZE;
		}
		goto EXIT;
	}
	wvfm = &(pDRV2624->ramwvfm);

	if(wvfm->rambin_name) {
		if(n + 32 + strlen(wvfm->rambin_name)+1 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE -n, "Rambin File name: %s\n",
				wvfm->rambin_name);
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n\r",__LINE__, __func__);
			n = PAGE_SIZE;
			goto EXIT;
		}
	} else {
		if(n + 42 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE -n, "ERROR: Rambin file name is NULL!\n");
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n",__LINE__, __func__);
			n = PAGE_SIZE;
		}
		goto EXIT;
	}
	ram_wvfm_info = wvfm->ram_wvfm_info;
	for(i = 0; i < wvfm->nWaveforms; i++) {
		if(n + 16 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE -n, "Wave %d", i);
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n",__LINE__, __func__);
			n = PAGE_SIZE;
			break;
		}

		if(n + 64 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE-n, ": duration = %d mnEffectTimems = %u\n",
				ram_wvfm_info[i]->duration, ram_wvfm_info[i]->mnEffectTimems);
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n",__LINE__, __func__);
			n = PAGE_SIZE;
			break;
		}
	}
EXIT:
	return n;
}

static DEVICE_ATTR(state, 0664, drv2624_state_show, drv2624_state_store);
static DEVICE_ATTR(activate, 0664, drv2624_activate_show, drv2624_activate_store);
static DEVICE_ATTR(duration, 0664, drv2624_duration_show, drv2624_duration_store);
static DEVICE_ATTR(calib, 0664, drv2624_calib_show, drv2624_calib_store);
static DEVICE_ATTR(diag, 0664, drv2624_diag_show, drv2624_diag_store);
static DEVICE_ATTR(reg, 0664, drv2624_reg_show, drv2624_reg_store);
static DEVICE_ATTR(CalComp, 0664, drv2624_CalComp_show, drv2624_CalComp_store);
static DEVICE_ATTR(CalBemf, 0664, drv2624_CalBemf_show, drv2624_CalBemf_store);
static DEVICE_ATTR(CalGain, 0664, drv2624_CalGain_show, drv2624_CalGain_store);
static DEVICE_ATTR(f0_msb, 0664, drv2624_f0_msb_show, drv2624_f0_msb_store);
static DEVICE_ATTR(f0_lsb, 0664, drv2624_f0_lsb_show, drv2624_f0_lsb_store);
static DEVICE_ATTR(f0_data, 0664, drv2624_f0_data_show, NULL);
static DEVICE_ATTR(gain, 0664, drv2624_gain_show, drv2624_gain_store);
static DEVICE_ATTR(rambininfo_list, 0664, rambininfo_list_show, NULL);
static DEVICE_ATTR(i2caddr, 0664, i2c_address_show, NULL);

static struct attribute *drv2624_led_dev_fs_attrs[] = {
	&dev_attr_state.attr,
	&dev_attr_activate.attr,
	&dev_attr_duration.attr,
	&dev_attr_gain.attr,
	&dev_attr_calib.attr,
	&dev_attr_diag.attr,
	&dev_attr_reg.attr,
	&dev_attr_CalComp.attr,
	&dev_attr_CalBemf.attr,
	&dev_attr_CalGain.attr,
	&dev_attr_f0_msb.attr,
	&dev_attr_f0_lsb.attr,
	&dev_attr_rambininfo_list.attr,
	&dev_attr_f0_data.attr,
	&dev_attr_i2caddr.attr,
	NULL,
};
static struct attribute_group drv2624_led_dev_fs_attr_group = {
	.attrs = drv2624_led_dev_fs_attrs,
};

static const struct attribute_group *drv2624_led_dev_fs_attr_groups[] = {
	&drv2624_led_dev_fs_attr_group,
	NULL,
};

/**
 * 1. Do work due to pDRV2624->mnWorkMode set before.
 * 2. For WORK_EFFECTSEQUENCER, WORK_CALIBRATION and WORK_DIAGNOSTIC
 *    check the GO bit until the process in DRV2624 has completed.
 * 3. For WORK_VIBRATOR, Stop DRV2624 directly.
 **/
static void vibrator_irq_routine(struct work_struct *work)
{
	struct drv2624_data *pDRV2624 =
	    container_of(work, struct drv2624_data, vibrator_irq_work);
	int nResult = 0, workmode = 0;
	if(pDRV2624 == NULL) {
		dev_err(pDRV2624->dev, "%s:%u: drv2624_data is NULL\n", __func__,
			__LINE__);
		return;
	}

	if(pDRV2624->mbWork == false) {
		dev_info(pDRV2624->dev, "%s:%u: Read Go bit is STOP\n",
			__func__, __LINE__);
		goto EXIT;
	}
	workmode = drv2624_reg_read(pDRV2624, DRV2624_R0X07);
	if(workmode < 0) {
		dev_err(pDRV2624->dev, "%s:%u: Read DRV2624_R0X07 error\n", __func__,
			__LINE__);
		goto EXIT;
	} else {
		workmode &= DRV2624_R0X07_MODE_MSK;
		dev_info(pDRV2624->dev, "%s:%u:WorkMode=0x%x\n", __func__,__LINE__,
			workmode);
	}
	if(gpio_is_valid(pDRV2624->msPlatData.mnGpioINT) && true == pDRV2624->mbIRQEnabled) {
		nResult =
		    drv2624_reg_read(pDRV2624, DRV2624_R0X01_STATUS);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: Read DRV2624_R0X01_STATUS error\n",
				__func__, __LINE__);
			goto EXIT;
		}
		drv2624_disableIRQ(pDRV2624);
		dev_info(pDRV2624->dev, "%s:%u: status=0x%x\n",
			__func__, __LINE__, nResult);
		if (nResult & DRV2624_R0X01_STATUS_OC_DETECT_MSK)
			dev_err(pDRV2624->dev,
				"ERROR, Over Current detected!!\n");
		if (nResult & DRV2624_R0X01_STATUS_OVER_TEMP_MSK)
			dev_err(pDRV2624->dev,
				"ERROR, Over Temperature detected!!\n");
		if (nResult & DRV2624_R0X01_STATUS_UVLO_MSK)
			dev_err(pDRV2624->dev, "ERROR, VDD drop observed!!\n");
		if (nResult & DRV2624_R0X01_STATUS_PRG_ERROR_MSK)
			dev_err(pDRV2624->dev, "ERROR, PRG error!!\n");
	}
	mutex_lock(&pDRV2624->haptic_lock);
	nResult = drv2624_set_go_bit(pDRV2624, DRV2624_R0X0C_NGO_BIT);
	mutex_unlock(&pDRV2624->haptic_lock);
	switch (workmode) {
		case DRV2624_R0X07_MODE_RTP_MODE:
			break;
		case DRV2624_R0X07_MODE_WVFRM_SEQ_MODE:
			break;
		default:
			break;
	}
EXIT:
	return;
}

static int drv2624_haptic_play_go(struct drv2624_data *pDRV2624, bool flag)
{
	int nResult = 0;
	dev_info(pDRV2624->dev,"%s enter, flag = %d\n", __func__, flag);
	if (!flag) {
		pDRV2624->current_ktime = ktime_get();
		dev_info(pDRV2624->dev,"%s:%u:pDRV2624->current_us=%lld\n",
				__func__, __LINE__, ktime_to_us(pDRV2624->current_ktime));

		pDRV2624->interval_us = ktime_to_us(ktime_sub(pDRV2624->current_ktime,
			pDRV2624->pre_enter_ktime));
		if (pDRV2624->interval_us < 2000) {
			dev_info(pDRV2624->dev, "%s:%u:pDRV2624->interval_us = %u\n",
					__func__, __LINE__, pDRV2624->interval_us);
			usleep_range(2000, 2000);
		}
	}
	if (true == flag) {
		nResult = drv2624_set_go_bit(pDRV2624, DRV2624_R0X0C_GO_BIT);
		if (nResult < 0) {
			dev_err(pDRV2624->dev,
				"%s:%u: go bit not Done nResult = %d\n",
				__func__, __LINE__, nResult);
		} else {
			pDRV2624->pre_enter_ktime = ktime_get();
			dev_info(pDRV2624->dev, "%s:%u: pDRV2624->current_us = %lld\n",
					__func__, __LINE__, ktime_to_us(pDRV2624->pre_enter_ktime));
		}
	} else {
		nResult = drv2624_set_go_bit(pDRV2624, DRV2624_R0X0C_NGO_BIT);
		if (nResult < 0) {
			dev_err(pDRV2624->dev,
				"%s:%u: go bit not Done nResult = %d\n",
				__func__, __LINE__, nResult);
		}
	}
	return nResult;
}

unsigned short drv26xx_get_ram_wvfm_duration(struct drv2624_data* pDRV26xx, unsigned short waveform_id)
{
    unsigned short i = 0, duration = 0;
    struct drv26xx_RAMwaveforms* wvfm = &(pDRV26xx->ramwvfm);
    struct drv26xx_RAMwaveform_info** ram_wvfm_info = pDRV26xx->ramwvfm.ram_wvfm_info;
    if(waveform_id >= wvfm->nWaveforms)
        dev_info(pDRV26xx->dev, "No such waveform_id = 0x%02x (should be between from 0 to 0x%02x)\n",
            waveform_id, wvfm->nWaveforms - 1);
    else {
	 	if (ram_wvfm_info) {
	        for (i = 0; i < wvfm->nWaveforms; i++) {
	            if (waveform_id == i) {
	                duration = ram_wvfm_info[i]->duration;
					break;
	            }
	        }
	    }
    }
	return duration;
}

void drv26xx_select_ram_wvfm(struct drv2624_data* pDRV26xx, unsigned short waveform_id)
{
    unsigned short i = 0;
    struct drv26xx_RAMwaveforms* wvfm = &(pDRV26xx->ramwvfm);
    struct drv26xx_RAMwaveform_info** ram_wvfm_info = pDRV26xx->ramwvfm.ram_wvfm_info;
    if(waveform_id >= wvfm->nWaveforms)
        dev_info(pDRV26xx->dev, "No such waveform_id = 0x%02x (should be between from 0 to 0x%02x)\n",
            waveform_id, wvfm->nWaveforms - 1);
#if 0
   	dev_info(pDRV26xx->dev, "fb_brake_factor = 0x%02x\n", wvfm->fb_brake_factor);
    dev_info(pDRV26xx->dev, "hybrid_loop = 0x%02x:%s\n", wvfm->hybrid_loop,
        (wvfm->hybrid_loop == 1) ? "Enable" : "Disable");
   	dev_info(pDRV26xx->dev, "auto_brake = 0x%02x:%s\n", wvfm->auto_brake,
        (wvfm->auto_brake == 1) ? "Enable" : "Disable");
    dev_info(pDRV26xx->dev, "auto_brake_standby = 0x%02x:%s\n", wvfm->auto_brake_standby,
        (wvfm->auto_brake_standby == 1) ? "Enable" : "Disable");
#endif
	drv2624_set_bits(pDRV26xx, DRV2624_R0X08,
		DRV2624_R0X08_HYBRID_LOOP_MSK|DRV2624_R0X08_AUTO_BRK_OL_MSK|DRV2624_R0X08_AUTO_BRK_INTO_STBY_MSK,
		((wvfm->hybrid_loop<<DRV2624_R0X08_HYBRID_LOOP_SFT)&DRV2624_R0X08_HYBRID_LOOP_MSK)|
		((wvfm->auto_brake<<DRV2624_R0X08_AUTO_BRK_OL_SFT)&DRV2624_R0X08_AUTO_BRK_OL_MSK)|
		((wvfm->auto_brake_standby<<DRV2624_R0X08_AUTO_BRK_INTO_STBY_SFT)&DRV2624_R0X08_AUTO_BRK_INTO_STBY_MSK));
    drv2624_set_bits(pDRV26xx, DRV2624_R0X23, DRV2624_R0X23_FB_BRAKE_FACTOR_MSK,
		(wvfm->fb_brake_factor<<DRV2624_R0X23_FB_BRAKE_FACTOR_SFT)&DRV2624_R0X23_FB_BRAKE_FACTOR_MSK);
    if (ram_wvfm_info) {
        for (i = 0; i < wvfm->nWaveforms; i++) {
            if (waveform_id == i) {
                dev_info(pDRV26xx->dev, "wvfm = %u\n\tmnEffectTimems = %u dur = %u\n",
                    i, ram_wvfm_info[i]->mnEffectTimems, ram_wvfm_info[i]->duration);
#if 1
				dev_info(pDRV26xx->dev, "\trtp_wvfm_info[%u]->brake = 0x%02x:%s\n",
                    i, ram_wvfm_info[i]->brake,
                    (ram_wvfm_info[i]->brake == 1) ? "Enable" : "Disable");
                dev_info(pDRV26xx->dev, "\trtp_wvfm_info[%u]->loop_mod = 0x%02x:%s\n",
                    i, ram_wvfm_info[i]->loop_mod,
                    (ram_wvfm_info[i]->loop_mod == 0) ? "Close" : "Open");
                dev_info(pDRV26xx->dev, "\trtp_wvfm_info[%u]->wv_shape = 0x%02x:%s\n",
                    i, ram_wvfm_info[i]->wv_shape,
                    (ram_wvfm_info[i]->wv_shape == 0) ? "Square" : "Sine");
#endif
				drv2624_set_bits(pDRV26xx, DRV2624_R0X08,
					DRV2624_R0X08_AUTO_BRK_OL_MSK | DRV2624_R0X08_CTL_LOOP_MSK,
					((ram_wvfm_info[i]->brake<<DRV2624_R0X08_AUTO_BRK_OL_SFT)&DRV2624_R0X08_AUTO_BRK_OL_MSK)|
					((ram_wvfm_info[i]->loop_mod<<DRV2624_R0X08_CTL_LOOP_SFT)&DRV2624_R0X08_CTL_LOOP_MSK));
				drv2624_set_bits(pDRV26xx, DRV2624_R0X2C,
					DRV2624_R0X2C_LRA_WAVE_SHAPE_MSK,
					ram_wvfm_info[i]->wv_shape & DRV2624_R0X2C_LRA_WAVE_SHAPE_MSK);
				break;
            }
        }
    }
}

static int drv2624_set_waveform(struct drv2624_data *pDRV2624,
	struct drv2624_waveform_sequencer *pSequencer)
{
	int nResult = 0;
	int i = 0;
	unsigned char loop[2] = { 0 };
	unsigned char effects[DRV2624_SEQUENCER_SIZE] = { 0 };
	unsigned char len = 0;

	drv2624_set_bits(pDRV2624, DRV2624_R0X07, DRV2624_R0X07_MODE_MSK,
		DRV2624_R0X07_MODE_WVFRM_SEQ_MODE);
	for (i = 0; i < DRV2624_SEQUENCER_SIZE; i++) {
		if (pSequencer->msWaveform[i].mnEffect != 0) {
			len++;
			if (i < 4)
				loop[0] |= (pSequencer->msWaveform[i].mnLoop << (2 * i));
			else
				loop[1] |= (pSequencer->msWaveform[i].mnLoop << (2 * (i - 4)));
			effects[i] = pSequencer->msWaveform[i].mnEffect;
		} else
			break;
	}
	for(i = 0; i < len; i++)
		dev_info(pDRV2624->dev, "%s:len =%d, effects[%d] = %d\n", __func__, len,
			i, effects[i]);
	if (len == 1) {
		drv26xx_select_ram_wvfm(pDRV2624, effects[0] - 1);
		nResult =
		    drv2624_reg_write(pDRV2624, DRV2624_R0X0F_SEQ1, effects[0]);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u:sequence error\n", __func__,__LINE__);
			goto end;
		}
		nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X17_WAV1_4_SEQ_LOOP, loop[0]);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u:sequence error\n", __func__,__LINE__);
			goto end;
		}
		nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X10_SEQ2, 0);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u:sequence error\n", __func__,__LINE__);
			goto end;
		}
	} else {
		nResult =
		    drv2624_bulk_write(pDRV2624, DRV2624_R0X0F_SEQ1,
				       effects, len);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u:sequence error\n", __func__,__LINE__);
			goto end;
		}
		if (len > 1) {
			if ((len - 1) <= 4)
				drv2624_reg_write(pDRV2624, DRV2624_R0X17_WAV1_4_SEQ_LOOP,
						  loop[0]);

			else
				drv2624_bulk_write(pDRV2624, DRV2624_R0X0F_SEQ1,
						   loop, 2);
		}
	}
end:
	return nResult;
}


static void vibrator_routine(struct work_struct *work)
{
	struct drv2624_data *pDRV2624 =
	    container_of(work, struct drv2624_data, vibrator_work);
	struct drv2624_platform_data *pPlatData = NULL;
	struct actuator_data *pActuator = NULL;
	int nResult = 0;

	if(pDRV2624 == NULL) {
		pr_err("%s:%u: drv2624_data is NULL\n", __func__, __LINE__);
		return;
	}
	pPlatData = &pDRV2624->msPlatData;
	pActuator = &(pPlatData->msActuator);
	mutex_lock(&pDRV2624->haptic_lock);
	drv2624_haptic_play_go(pDRV2624, false);
	dev_info(pDRV2624->dev, "%s:%u: pDRV2624->state = %d\n", __func__,
			__LINE__, pDRV2624->state);
	if(pDRV2624->state) {
		pDRV2624->bRTPmode = false;
		nResult = drv2624_set_bits(pDRV2624,
			DRV2624_R0X19_WAV_SEQ_MAIN_LOOP,
			DRV2624_R0X19_WAV_SEQ_MAIN_LOOP_MSK,
			pDRV2624->waveform_id == 4 ?
			DRV2624_R0X19_WAV_SEQ_MAIN_LOOP_INFINITE:
			DRV2624_R0X19_WAV_SEQ_MAIN_LOOP_ONCE);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: I2C set bit error\n", __func__,
				__LINE__);
			goto EXIT;
		}

		nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X1F_RATED_VOLTAGE,
			pActuator->mnRatedVoltage);
		if (nResult < 0) {
			dev_err(pDRV2624->dev,
				"%s: mnRatedVoltage set failed nResult = %d\n",
				__func__, nResult);
			goto EXIT;
		}

		nResult = drv2624_reg_write(pDRV2624, DRV2624_R0X20_OD_CLAMP,
			pActuator->mnOverDriveClampVoltage);
		if (nResult < 0) {
			dev_err(pDRV2624->dev,
				"%s: mnCalOverDriveClampVoltage set failed nResult = %d\n",
				__func__, nResult);
			goto EXIT;
		}

		memset(&pDRV2624->msWaveformSequencer, 0,
				       sizeof(struct drv2624_waveform_sequencer));
		pDRV2624->msWaveformSequencer.msWaveform[0].mnEffect = pDRV2624->waveform_id;
		pDRV2624->msWaveformSequencer.msWaveform[0].mnLoop = 0;
		pDRV2624->msWaveformSequencer.msWaveform[1].mnEffect = 0;
		pDRV2624->msWaveformSequencer.msWaveform[1].mnLoop = 0;
		nResult = drv2624_set_waveform(pDRV2624,
							 &pDRV2624->msWaveformSequencer);
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "Configurate predefined effect %d failed, nResult=%d\n",
				pDRV2624->msWaveformSequencer.msWaveform[0].mnEffect, nResult);
		}
		drv2624_haptic_play_go(pDRV2624, true);
		if(pDRV2624->waveform_id != 4)
			pDRV2624->duration = drv26xx_get_ram_wvfm_duration(pDRV2624,
				pDRV2624->waveform_id - 1);
		dev_info(pDRV2624->dev, "%s:%u:duration = %d\n",__func__, __LINE__,
				pDRV2624->duration);
		hrtimer_start(&pDRV2624->haptics_timer,
			ktime_set(pDRV2624->duration / 1000,
			(pDRV2624->duration % 1000) * 1000000),
			HRTIMER_MODE_REL);
	}
EXIT:
	mutex_unlock(&pDRV2624->haptic_lock);
}

static irqreturn_t drv2624_irq_handler(int irq, void *dev_id)
{
	struct drv2624_data *pDRV2624 = (struct drv2624_data *)dev_id;
	dev_info(pDRV2624->dev, "%s: enter\n", __func__);
	schedule_work(&pDRV2624->vibrator_irq_work);
	return IRQ_HANDLED;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	struct drv2624_data *pDRV2624 =
	    container_of(timer, struct drv2624_data, haptics_timer);
	dev_info(pDRV2624->dev, "%s\n", __func__);
	pDRV2624->state = 0;
	schedule_work(&pDRV2624->vibrator_work);
	return HRTIMER_NORESTART;
}

static int drv2624_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct drv2624_data *pDRV2624 = NULL;
	int nResult = 0;
	bool bLedRegister = false;

	dev_info(&client->dev, "%s enter\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "%s:%u:I2C check failed\n", __func__,__LINE__);
		nResult = -ENODEV;
		goto ERROR;
	}

	pDRV2624 = devm_kzalloc(&client->dev,
				sizeof(struct drv2624_data), GFP_KERNEL);
	if (pDRV2624 == NULL) {
		dev_err(&client->dev, "%s:%u:error\n", __func__,__LINE__);
		nResult = -ENOMEM;
		goto ERROR;
	}

	pDRV2624->dev = &client->dev;
	pDRV2624->client = client;
	pDRV2624->mbIRQEnabled = false;
	dev_set_drvdata(&client->dev, pDRV2624);
	pDRV2624->mpRegmap = devm_regmap_init_i2c(client, &drv2624_i2c_regmap);
	if (IS_ERR(pDRV2624->mpRegmap)) {
		nResult = PTR_ERR(pDRV2624->mpRegmap);
		dev_err(pDRV2624->dev, "%s:Failed to allocate register map: %d\n",
			__func__, nResult);
		goto ERROR;
	}

	if (client->dev.of_node) {
		dev_info(pDRV2624->dev, "of node parse\n");
		drv2624_parse_dt(&client->dev, pDRV2624);
	} else {
		dev_err(pDRV2624->dev, "%s: ERROR no platform data\n",
			__func__);
	}
	mutex_init(&pDRV2624->reg_lock);
	mutex_init(&pDRV2624->haptic_lock);
	if (gpio_is_valid(pDRV2624->msPlatData.mnGpioNRST)) {
		nResult =
		    gpio_request(pDRV2624->msPlatData.mnGpioNRST,
				 "DRV2624-NRST");
		if (nResult < 0) {
			dev_err(pDRV2624->dev, "%s:%u: GPIO %d request NRST error\n",
				__func__, __LINE__, pDRV2624->msPlatData.mnGpioNRST);
			goto ERROR;
		}
		drv2624_hw_reset(pDRV2624);
	}
	nResult = drv2624_reg_read(pDRV2624, DRV2624_R0X00);
	if (DRV2624_R0X00_CHIP_ID_REV == nResult) {
		dev_info(pDRV2624->dev, "%s:%u: CHIP_ID & REV = 0x%02x\n",
			__func__, __LINE__, nResult);
		pDRV2624->mnDeviceID = nResult;
	} else {
		dev_err(pDRV2624->dev, "%s:%u: device_id(0x%02x) fail\n",
			__func__, __LINE__,	nResult);
		goto ERROR;
	}
	drv2624_init(pDRV2624);

	pDRV2624->led_dev.name = "drv26xx_haptic";
    pDRV2624->led_dev.brightness_set = drv2624_vibrator_enable;
    pDRV2624->led_dev.groups = drv2624_led_dev_fs_attr_groups;
	nResult = devm_led_classdev_register(&pDRV2624->client->dev, &pDRV2624->led_dev);
    if (nResult) {
	    dev_err(pDRV2624->dev,"%s:%u:Failed to create led classdev: %d\n",
			__func__, __LINE__, nResult);
		goto ERROR;
	}
	bLedRegister = true;

	hrtimer_init(&pDRV2624->haptics_timer, CLOCK_MONOTONIC,
	     HRTIMER_MODE_REL);
	pDRV2624->haptics_timer.function = vibrator_timer_func;


	if (gpio_is_valid(pDRV2624->msPlatData.mnGpioINT)) {
		nResult =
			gpio_request(pDRV2624->msPlatData.mnGpioINT, "DRV2624-IRQ");
		if (nResult == 0) {
			gpio_direction_input(pDRV2624->msPlatData.mnGpioINT);
			pDRV2624->mnIRQ = gpio_to_irq(pDRV2624->msPlatData.mnGpioINT);
			dev_info(pDRV2624->dev, "irq = %u \n", pDRV2624->mnIRQ);
			nResult =
				request_threaded_irq(pDRV2624->mnIRQ, drv2624_irq_handler,
					NULL, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					client->name, pDRV2624);
			if (nResult == 0) {
				drv2624_disableIRQ(pDRV2624);
			} else {
				dev_err(pDRV2624->dev, "request_irq failed, %d\n",
					nResult);
			}
		} else {
			dev_err(pDRV2624->dev,
				"%s:%u: GPIO %d request INT error,nResult = %d\n", __func__, __LINE__,
				pDRV2624->msPlatData.mnGpioINT,nResult);
				goto ERROR;
		}
	}

	INIT_WORK(&pDRV2624->vibrator_irq_work, vibrator_irq_routine);
	INIT_WORK(&pDRV2624->vibrator_work, vibrator_routine);

	nResult = request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
					  RAM_BIN_FILE, &(client->dev),
					  GFP_KERNEL, pDRV2624,
					  drv2624_ram_load);
	if (nResult != 0) {
		dev_err(&client->dev,
			"%s: %u: nResult = %d: request drv2624_ram_load error, pls check!\n",
			__func__, __LINE__, nResult);
            goto ERROR;
	}

	pDRV2624->mAutoCalData.mnDoneFlag = 0xFF;
ERROR:
	if(nResult == 0) {
		dev_info(&client->dev, "%s successfully!\n", __func__);
	} else {
		if(pDRV2624 != NULL) {
			mutex_destroy(&pDRV2624->reg_lock);
			mutex_destroy(&pDRV2624->haptic_lock);
			if(bLedRegister)
				devm_led_classdev_unregister(&pDRV2624->client->dev, &pDRV2624->led_dev);
			if (gpio_is_valid(pDRV2624->msPlatData.mnGpioINT))
				gpio_free(pDRV2624->msPlatData.mnGpioINT);
			if (gpio_is_valid(pDRV2624->msPlatData.mnGpioNRST))
				gpio_free(pDRV2624->msPlatData.mnGpioNRST);

			devm_kfree(&client->dev, pDRV2624);
			pDRV2624 = NULL;
		}
		dev_err(&client->dev, "%s failed!\n", __func__);
	}
	return nResult;
}

static void drv26xx_ram_waveform_remove(struct drv2624_data* pDRV26xx)
{
	int i = 0;
	struct drv26xx_RAMwaveform_info** ram_wvfm_info = NULL;
	struct drv26xx_RAMwaveforms* wvfm = NULL;

	if(pDRV26xx == NULL) return;
	ram_wvfm_info = pDRV26xx->ramwvfm.ram_wvfm_info;
	wvfm = &pDRV26xx->ramwvfm;
	if (NULL != wvfm->rambin_name) kfree(wvfm->rambin_name);
	if (ram_wvfm_info) {
		for (i = 0; i < pDRV26xx->ramwvfm.nWaveforms; i++) {
			if (ram_wvfm_info[i] != NULL) {
				kfree(ram_wvfm_info[i]);
			}
		}
		kfree(ram_wvfm_info);
	}
}

static int drv2624_remove(struct i2c_client *client)
{
	struct drv2624_data *pDRV2624 = i2c_get_clientdata(client);
	if(pDRV2624 != NULL) {
		hrtimer_cancel(&pDRV2624->haptics_timer);
		cancel_work_sync(&pDRV2624->vibrator_irq_work);
		cancel_work_sync(&pDRV2624->vibrator_work);
		mutex_destroy(&pDRV2624->reg_lock);
		mutex_destroy(&pDRV2624->haptic_lock);
		devm_led_classdev_unregister(&pDRV2624->client->dev, &pDRV2624->led_dev);
		drv26xx_ram_waveform_remove(pDRV2624);
		if (gpio_is_valid(pDRV2624->msPlatData.mnGpioINT))
			gpio_free(pDRV2624->msPlatData.mnGpioINT);
		if (gpio_is_valid(pDRV2624->msPlatData.mnGpioNRST))
			gpio_free(pDRV2624->msPlatData.mnGpioNRST);
	}
	return 0;
}

static const struct i2c_device_id drv2624_id[] = {
	{ "drv2624", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, drv2624_id);

#ifdef CONFIG_OF
static const struct of_device_id of_drv2624_leds_match[] = {
	{ .compatible = "ti,haptic" },
	{},
};

MODULE_DEVICE_TABLE(of, of_drv2624_leds_match);
#endif

static struct i2c_driver drv2624_driver = {
	.driver = {
		.name	= "drv2624",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(of_drv2624_leds_match),
#endif
	},
	.probe		= drv2624_probe,
	.remove		= drv2624_remove,
	.id_table	= drv2624_id,
};
#if 0
module_i2c_driver(drv2624_driver);
#else
static int __init drv2624_i2c_init(void)
{
	int ret = 0;

	pr_info("%s:%u\n",__func__, __LINE__);
#if 0
	if (haptic_mode[0] == 0) {
		pr_info("%s:%u: no TI haptic!\n",__func__, __LINE__);
		return -ENODEV;
	}
#endif
	ret = i2c_add_driver(&drv2624_driver);
	if (ret) {
		pr_err("%s:%u:fail to add drv2624 device into i2c ret = 0x%x\n",
			__func__, __LINE__, ret);
	}

	return ret;
}

late_initcall_sync(drv2624_i2c_init);

static void __exit drv2624_i2c_exit(void)
{
	i2c_del_driver(&drv2624_driver);
}
module_exit(drv2624_i2c_exit);
#endif

MODULE_DESCRIPTION("Texas Instruments DRV2624 Haptic Driver");
MODULE_AUTHOR("XXX");
MODULE_LICENSE("GPL");

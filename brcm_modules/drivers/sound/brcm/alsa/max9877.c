/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/
#include "audio_drv.h"
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>

#define MAX9877_CONTROL_BYTES  2

#define MAX9877_OPERATION       0x80
#define MAX9877_SHUTDOWN        0x00

#define MAX9877_INPUT_MODE            0x00
#define MAX9877_SPEAKER_VOL           0x01
#define MAX9877_LEFT_HEADSET_VOL      0x02
#define MAX9877_RIGHT_HEADSET_VOL     0x03
#define MAX9877_OUTPUT_MODE           0x04

#define MAX9877_MAX_VOLUME 31

/* MAX9877_INPUT_MODE */
#define MAX9877_PGAINB(X)	(X << 0)
#define MAX9877_PGAINB_MASK	(0x3)
#define MAX9877_PGAINA(X)	(X << 2)
#define MAX9877_PGAINA_MASK	(0xC)
#define MAX9877_INB             (1 << 4)
#define MAX9877_INA             (1 << 5)
#define MAX9877_ZCD             (1 << 6)

/* MAX9877_OUTPUT_MODE */
#define MAX9877_OUTMODE_MASK    (0xf)
#define MAX9877_OUTMODE(X)	(X)
#define MAX9877_OSC_MASK        (3 << 4)
#define MAX9877_OSC(X)	        (X << 4)
#define MAX9877_OSC_OFFSET	(4)
#define MAX9877_BYPASS		(1 << 6)
#define MAX9877_SHDN		(1 << 7)

struct max9877_dev {
	int out_mode;
	struct i2c_client *max9877_client;
	struct mutex max9877_lock;
};

static struct max9877_dev *amp_dev;
static u8 max9877_regs[5] = { 0x40, 0x00, 0x00, 0x00, 0x49 };
static AudioExtPreGainMAX9877_t max9877_pregain;

static void max9877_write_regs(void)
{
	unsigned int i;
	u8 data[6];
	data[0] = MAX9877_INPUT_MODE;
	for (i = 0; i < ARRAY_SIZE(max9877_regs); i++)
		data[i + 1] = max9877_regs[i];

	if (i2c_master_send(amp_dev->max9877_client, data, 6) != 6)
		pr_info("i2c write failed\n");
}

static void max9877_dump_regs(void)
{
	u8 data[6];
	int i;

	data[0] = MAX9877_INPUT_MODE;
	i2c_master_send(amp_dev->max9877_client, data, 1);

	memset(data, 0, 6);
	if (i2c_master_recv(amp_dev->max9877_client, data, 5) != 5)
		pr_info("max9877: i2c read failed\n");

	pr_info("Read data: ");
	for (i = 0; i < ARRAY_SIZE(max9877_regs); i++)
		pr_info("%02x ", data[i]);
	pr_info("\n");
}
 
static int AudioExtDev_GetOutput(void)
{
	return amp_dev->out_mode;
}

static void AudioExtDev_SetOutput(enum audio_ext_output output)
{
	if (output > MAX9877_OUTPUT_INVALID)
		output = MAX9877_OUTPUT_INVALID;

	amp_dev->out_mode = output;
}


int max9877_get_osc_mode(void)
{
	u8 value = (max9877_regs[MAX9877_OUTPUT_MODE] & MAX9877_OSC_MASK);
	value = value >> MAX9877_OSC_OFFSET;
	return value;
}

EXPORT_SYMBOL(max9877_get_osc_mode);

int max9877_set_osc_mode(int osc_mode)
{
	osc_mode = MAX9877_OSC(osc_mode);
	if ((max9877_regs[MAX9877_OUTPUT_MODE] & MAX9877_OSC_MASK) == osc_mode)
		return 0;

	mutex_lock(&amp_dev->max9877_lock);
	max9877_regs[MAX9877_OUTPUT_MODE] &= ~MAX9877_OSC_MASK;
	max9877_regs[MAX9877_OUTPUT_MODE] |= osc_mode;
	mutex_unlock(&amp_dev->max9877_lock);
	max9877_write_regs();
	return 0;
}

EXPORT_SYMBOL(max9877_set_osc_mode);

void AudioExtDev_SetVolume(UInt8 extVol)
{
	if (extVol > MAX9877_SPKVOL_N00DB)
		extVol = MAX9877_SPKVOL_N00DB;

	pr_info("AudioExtDev_SetVolume: extVol=%d", extVol);

	mutex_lock(&amp_dev->max9877_lock);
	switch(AudioExtDev_GetOutput()) {
		case MAX9877_OUTPUT_LOUDSPK:
		case MAX9877_OUTPUT_HANDSET:
			max9877_regs[MAX9877_SPEAKER_VOL] = extVol;
			break;
		case MAX9877_OUTPUT_HEADSET:
			max9877_regs[MAX9877_LEFT_HEADSET_VOL] = extVol;
			max9877_regs[MAX9877_RIGHT_HEADSET_VOL] = extVol;
			break;
		case MAX9877_OUTPUT_HEADSET_AND_LOUDSPK:
			max9877_regs[MAX9877_SPEAKER_VOL] = extVol;
			max9877_regs[MAX9877_LEFT_HEADSET_VOL] = extVol;
			max9877_regs[MAX9877_RIGHT_HEADSET_VOL] = extVol;
			break;
			
		default:
			max9877_regs[MAX9877_SPEAKER_VOL] = MAX9877_SPKVOL_MUTE;
			max9877_regs[MAX9877_LEFT_HEADSET_VOL] = MAX9877_SPKVOL_MUTE;
			max9877_regs[MAX9877_RIGHT_HEADSET_VOL] = MAX9877_SPKVOL_MUTE;
			break;
	}
	mutex_unlock(&amp_dev->max9877_lock);
	max9877_write_regs();
}
EXPORT_SYMBOL(AudioExtDev_SetVolume);

void AudioExtDev_PowerOn(AudioMode_t audio_mode)
{
	unsigned int in_val = max9877_regs[MAX9877_INPUT_MODE], 
		     out_val = max9877_regs[MAX9877_OUTPUT_MODE];

	pr_info("AudioExtDev_PowerOn mode=%d->", audio_mode);

	switch (audio_mode) {
	case MAX9877_OUTPUT_LOUDSPK:
		in_val &= ~MAX9877_INA;
		in_val |= MAX9877_INB;
		out_val &= ~(MAX9877_OUTMODE_MASK | MAX9877_BYPASS);
		out_val |= (MAX9877_SHDN | MAX9877_OUTMODE(AMMP_OUT_B_SPK));
		break;
	case MAX9877_OUTPUT_HANDSET:
		in_val &= ~MAX9877_INA;
		in_val |= MAX9877_INB;
		out_val &= ~(MAX9877_OUTMODE_MASK | MAX9877_SHDN);
		out_val |= (MAX9877_BYPASS | MAX9877_OUTMODE(AMMP_OUT_B_SPK));
		break;
	case MAX9877_OUTPUT_HEADSET:
		in_val &= ~(MAX9877_INA | MAX9877_INB);
		out_val &= ~(MAX9877_OUTMODE_MASK | MAX9877_BYPASS);
		out_val |= (MAX9877_SHDN | MAX9877_OUTMODE(AMMP_OUT_A_LEFT_HP_RIGHT_HP));
		break;
	case MAX9877_OUTPUT_HEADSET_AND_LOUDSPK:
		in_val &= ~(MAX9877_INA | MAX9877_INB);
		out_val &= ~(MAX9877_OUTMODE_MASK | MAX9877_BYPASS);
		out_val |= (MAX9877_SHDN | MAX9877_OUTMODE(AMMP_OUT_A_SPK_LEFT_HP_RIGHT_HP));
		break;
	default:		// invalid output
		out_val &= ~(MAX9877_OUTMODE_MASK | MAX9877_SHDN);
		out_val |= (MAX9877_BYPASS | MAX9877_OUTMODE(AMMP_OUT_NONE));
		break;
	}
	if ((max9877_regs[MAX9877_INPUT_MODE] == in_val)
	    && (max9877_regs[MAX9877_OUTPUT_MODE] == out_val))
		return;

	in_val |= MAX9877_ZCD | 
			MAX9877_PGAINA(AudioExtDev_GetPreGain()) |
			MAX9877_PGAINB(AudioExtDev_GetPreGain());

	AudioExtDev_SetOutput(audio_mode);
	mutex_lock(&amp_dev->max9877_lock);
	max9877_regs[MAX9877_INPUT_MODE] = in_val;
	max9877_regs[MAX9877_OUTPUT_MODE] = out_val;
	mutex_unlock(&amp_dev->max9877_lock);
	max9877_write_regs();
}

EXPORT_SYMBOL(AudioExtDev_PowerOn);

void AudioExtDev_PowerOff(void)
{
	mutex_lock(&amp_dev->max9877_lock);
	max9877_regs[MAX9877_SPEAKER_VOL] = MAX9877_SPKVOL_MUTE;
	max9877_regs[MAX9877_LEFT_HEADSET_VOL] = MAX9877_SPKVOL_MUTE;
	max9877_regs[MAX9877_RIGHT_HEADSET_VOL] = MAX9877_SPKVOL_MUTE;
	max9877_regs[MAX9877_OUTPUT_MODE] &= ~(MAX9877_SHDN);
	mutex_unlock(&amp_dev->max9877_lock);

	max9877_write_regs();
}
EXPORT_SYMBOL(AudioExtDev_PowerOff);

void AudioExtDev_SetPreGain(AudioExtPreGainMAX9877_t pregain)
{
	if(pregain > MAX9877_PGAIN_P20DB) 
		max9877_pregain = MAX9877_PGAIN_P20DB;

	max9877_pregain = pregain;
}
EXPORT_SYMBOL(AudioExtDev_SetPreGain);

AudioExtPreGainMAX9877_t  AudioExtDev_GetPreGain(void)
{
	return max9877_pregain;
}
EXPORT_SYMBOL(AudioExtDev_GetPreGain);

#ifdef CONFIG_PM
static int max9877_suspend(struct i2c_client *client, pm_message_t state)
{
	return 0;
}

static int max9877_resume(struct i2c_client *client)
{
	return 0;
}
#endif
static int __devinit max9877_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	pr_info("-->>probing max 9877\n");

	amp_dev = kzalloc(sizeof(struct max9877_dev), GFP_KERNEL);
	amp_dev->max9877_client = client;
	amp_dev->out_mode = AMMP_OUT_AB_SPK;
	mutex_init(&amp_dev->max9877_lock);
	i2c_set_clientdata(amp_dev->max9877_client, amp_dev);

	/*Config init values */
	max9877_regs[MAX9877_INPUT_MODE] = MAX9877_ZCD
	    | MAX9877_PGAINB(MAX9877_PGAIN_P00DB) | MAX9877_PGAINA(MAX9877_PGAIN_P00DB);
	max9877_regs[MAX9877_SPEAKER_VOL] = MAX9877_SPKVOL_MUTE;
	max9877_regs[MAX9877_LEFT_HEADSET_VOL] = MAX9877_SPKVOL_MUTE;
	max9877_regs[MAX9877_RIGHT_HEADSET_VOL] = MAX9877_SPKVOL_MUTE;
	max9877_regs[MAX9877_OUTPUT_MODE] = MAX9877_OUTMODE(AMMP_OUT_AB_SPK);

	max9877_set_osc_mode(AMMP_OSC_00);

	return 0;
}

static int __devexit max9877_remove(struct i2c_client *client)
{
	struct max9877_dev *r_dev = i2c_get_clientdata(client);
	r_dev->max9877_client = NULL;
	return 0;
}

static const struct i2c_device_id max9877_i2c_id[] = {
	{"max9877", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, max9877_i2c_id);

static struct i2c_driver max9877_i2c_driver = {
	.driver = {
		   .name = "max9877",
		   .owner = THIS_MODULE,
		   },
	.probe = max9877_probe,
	.remove = max9877_remove,
#ifdef CONFIG_PM
	.suspend = max9877_suspend,
	.resume = max9877_resume,
#endif
	.id_table = max9877_i2c_id,
};

static int __init max9877_init(void)
{
	return i2c_add_driver(&max9877_i2c_driver);
}

static void __exit max9877_exit(void)
{
	i2c_del_driver(&max9877_i2c_driver);
}

subsys_initcall(max9877_init);
module_exit(max9877_exit);

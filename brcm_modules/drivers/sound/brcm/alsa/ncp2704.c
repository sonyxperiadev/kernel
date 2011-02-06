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
#include <linux/delay.h>

#define NCP2704_INPUT_MODE            0x00
#define NCP2704_SPEAKER_VOL           0x01
#define NCP2704_LEFT_HEADSET_VOL      0x02
#define NCP2704_RIGHT_HEADSET_VOL     0x03
#define NCP2704_OUTPUT_MODE           0x04
#define NCP2704_LDO_MODE              0x05
#define NCP2704_STATUS_MODE           0x06
#define NCP2704_ACNT_MODE             0x07
#define NCP2704_ACONFA_MODE           0x08
#define NCP2704_ACONFR_MODE           0x09
#define NCP2704_ACONFH_MODE           0x0A
#define NCP2704_EMI_MODE              0x0B

#define NCP2704_MAX_VOLUME 31

/* NCP2704_INPUT_MODE */
#define NCP2704_PGAIN2G(X)	(X << 0)
#define NCP2704_PGAIN2G_MASK	(0x3)
#define NCP2704_PGAIN1G(X)	(X << 2)
#define NCP2704_IN2G        (1 << 4)
#define NCP2704_IN1G        (1 << 5)

/* NCP2704_OUTPUT_MODE */
#define NCP2704_OUTMODE_MASK    (0xf)
#define NCP2704_OUTMODE(X)	(X)
#define NCP2704_SD		(1 << 5)
#define MCP2704_RESET		(1 << 6)

/* MCP2704_ACNT_MODE */
#define AGCE_ENABLE (1 << 7)    /*AGC enable*/
#define AGCE_DISABLE (0 << 7)   /*AGC disable*/
#define NCL_ENABLE (1 << 6)     /*NC/L enable*/
#define NCL_DISABLE (0 << 6)    /*NC/L disable*/
#define PWR(X)  (X << 3)
#define THD(X)  (X)

/*MCP2704_ACONFA_MODE*/
#define ACONFA_00 0x00 
#define ACONFA_01 0x01 
#define ACONFA_02 0x02
#define ACONFA_03 0x03

/*MCP2704_ACONFR_MODE*/
#define ACONFR_00 0x00 
#define ACONFR_01 0x01 
#define ACONFR_02 0x02
#define ACONFR_03 0x03

/*MCP2704_ACONFH_MODE*/
#define ACONFH_00 0x00 
#define ACONFH_01 0x01 
#define ACONFH_02 0x02
#define ACONFH_03 0x03

/* NCP2704_LDO_MODE */
#define LD0_00 0x00 /*1.3 */
#define LD0_01 0x01 /*1.4 */
#define LD0_02 0x02 /*1.5 */
#define LD0_03 0x03 /*1.6 */
#define LD0_04 0x04 /*1.7 */
#define LD0_05 0x05 /*1.8 */
#define LD0_06 0x06 /*1.9 */
#define LD0_07 0x07 /*2.0 */
#define LD0_08 0x08 /*2.1 */
#define LD0_09 0x09 /*2.2 */
#define LD0_10 0x0a /*2.3 */
#define LD0_11 0x0b /*2.4 */
#define LD0_12 0x0c /*2.5 */
#define LD0_13 0x0d /*2.6 */


/* NCP2704_EMI_MODE */
#define EMI_20 0x00 /*20 */
#define EMI_15 0x01 /*15 */
#define EMI_10 0x02 /*15 */
#define EMI_05 0x03 /*15 */
#define EMI_01 0x04 /*15 */

struct ncp2704_dev {
	int out_mode;
	struct i2c_client *ncp2704_client;
	struct mutex ncp2704_lock;
};

static struct ncp2704_dev *amp_dev;
static u8 ncp2704_regs[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x3f, 0x01 ,0x01, 0x00, 0x00 };
                             /*Input,Speaker,Left,    Right,  Ouput  ,LDO,  Status,  Acnt, AconfA,AconfR,AconfH,EMI*/
static AudioExtPreGainNCP2704_t NCP2704_pregain;

static void ncp2704_write_regs(void)
{
	unsigned int i;
	u8 data[13];
	data[0] = NCP2704_INPUT_MODE;
	for (i = 0; i < ARRAY_SIZE(ncp2704_regs); i++)
		data[i + 1] = ncp2704_regs[i];

	if (i2c_master_send(amp_dev->ncp2704_client, data, 13) != 13)
		pr_info("i2c write failed\n");
}

static void ncp2704_dump_regs(void)
{
	u8 data[13];
	int i;

	data[0] = NCP2704_INPUT_MODE;
	i2c_master_send(amp_dev->ncp2704_client, data, 1);

	memset(data, 0, 13);
	if (i2c_master_recv(amp_dev->ncp2704_client, data, 12) != 12)
		pr_info("ncp2704: i2c read failed\n");

	pr_info("Read data: ");
	for (i = 0; i < ARRAY_SIZE(ncp2704_regs); i++)
		pr_info("%02x ", data[i]);
	pr_info("\n");
}
 
static int AudioExtDev_GetOutput(void)
{
	return amp_dev->out_mode;
}

static void AudioExtDev_SetOutput(enum audio_ext_output output)
{
    pr_info("AudioExtDev_SetOutput output=%d",output);
	if (output > NCP2704_OUTPUT_INVALID)
		output = NCP2704_OUTPUT_INVALID;

	amp_dev->out_mode = output;
}

static void ncp2704_spk_volume_stepping(u8 spk_vol_old, u8 spk_vol_new)
{
    u8 data[2], step = 1;

	/* Speaker Volume */
    data[0] = NCP2704_SPEAKER_VOL;
	step = (spk_vol_new > spk_vol_old)? 1: -1;
	for(data[1] = spk_vol_old ; ; data[1] += step)	{
		if (i2c_master_send(amp_dev->ncp2704_client, data, 2) != 2)
			pr_info("i2c write failed\n");
		udelay(1);
		if (data[1] == spk_vol_new)
			break;
	}
}

static void ncp2704_hp_volume_stepping(u8 hp_vol_old, u8 hp_vol_new)
{
    u8 data[3], hp_vol, step = 1;

	/* HeadPhone Left/Right Volume */
    data[0] = NCP2704_LEFT_HEADSET_VOL;
	step = (hp_vol_new > hp_vol_old)? 1: -1;
	for(hp_vol = hp_vol_old ; ; hp_vol += step)	{
		data[1] = hp_vol;	/* Left Volume */
		data[2] = hp_vol;	/* Right Volume */
		if (i2c_master_send(amp_dev->ncp2704_client, data, 3) != 3)
			pr_info("i2c write failed\n");
		udelay(1);
		if (hp_vol == hp_vol_new)
			break;
	}
}

void AudioExtDev_SetVolume(UInt8 extVol)
{
	u8 hp_vol_old, spk_vol_old;

	if (extVol > NCP2704_SPKVOL_N00DB)
		extVol = NCP2704_SPKVOL_N00DB;

	pr_info("AudioExtDev_SetVolume: extVol=%d", extVol);

	spk_vol_old = ncp2704_regs[NCP2704_SPEAKER_VOL];
	hp_vol_old = ncp2704_regs[NCP2704_LEFT_HEADSET_VOL];

	mutex_lock(&amp_dev->ncp2704_lock);
	switch(AudioExtDev_GetOutput()) {
		case NCP2704_OUTPUT_LOUDSPK:
		case NCP2704_OUTPUT_HANDSET:
			ncp2704_regs[NCP2704_SPEAKER_VOL] = extVol;
			break;
		case NCP2704_OUTPUT_HEADSET:
			ncp2704_regs[NCP2704_LEFT_HEADSET_VOL] = extVol;
			ncp2704_regs[NCP2704_RIGHT_HEADSET_VOL] = extVol;
			break;
		default:
			ncp2704_regs[NCP2704_SPEAKER_VOL] = NCP2704_SPKVOL_MUTE;
			ncp2704_regs[NCP2704_LEFT_HEADSET_VOL] = NCP2704_SPKVOL_MUTE;
			ncp2704_regs[NCP2704_RIGHT_HEADSET_VOL] = NCP2704_SPKVOL_MUTE;
			break;
	}
	mutex_unlock(&amp_dev->ncp2704_lock);

	/* POP suppression: speaker and headphone volume is changed in steps */
	if (spk_vol_old != ncp2704_regs[NCP2704_SPEAKER_VOL])	{
		ncp2704_spk_volume_stepping(spk_vol_old,
			ncp2704_regs[NCP2704_SPEAKER_VOL]);
	}
	if (hp_vol_old != ncp2704_regs[NCP2704_LEFT_HEADSET_VOL])	{
		ncp2704_hp_volume_stepping(hp_vol_old,
			ncp2704_regs[NCP2704_LEFT_HEADSET_VOL]);
	}
}
EXPORT_SYMBOL(AudioExtDev_SetVolume);

void AudioExtDev_PowerOn(AudioMode_t audio_mode)
{
	unsigned int in_val = ncp2704_regs[NCP2704_INPUT_MODE], 
		     out_val = ncp2704_regs[NCP2704_OUTPUT_MODE];

	pr_info("AudioExtDev_PowerOn mode=%d->", audio_mode);

	switch (audio_mode) {
	case NCP2704_OUTPUT_LOUDSPK:
		in_val &= ~NCP2704_IN1G;
		in_val |= NCP2704_IN2G;
		out_val &= ~NCP2704_OUTMODE_MASK;
		out_val |= (NCP2704_SD | NCP2704_OUTMODE(AMMP_OUT_IN2DIFF_SPK));
		break;
	case NCP2704_OUTPUT_HANDSET:
		in_val &= ~NCP2704_IN1G;
		in_val |= NCP2704_IN2G;
		out_val &= ~(NCP2704_OUTMODE_MASK | NCP2704_SD);
		out_val |= NCP2704_OUTMODE(AMMP_OUT_IN2DIFF_SPK);
		break;
	case NCP2704_OUTPUT_HEADSET:
		in_val &= ~(NCP2704_IN1G | NCP2704_IN2G);
		out_val &= ~(NCP2704_OUTMODE_MASK);
		out_val |= (NCP2704_SD | NCP2704_OUTMODE(AMMP_OUT_IN1A_LEFT_HP_IN1B_RIGHT_HP));
		break;
	default:		// invalid output
		out_val &= ~(NCP2704_OUTMODE_MASK | NCP2704_SD);
		out_val |= NCP2704_OUTMODE(AMMP_OUT_NONE);
		break;
	}
	if ((ncp2704_regs[NCP2704_INPUT_MODE] == in_val)
	    && (ncp2704_regs[NCP2704_OUTPUT_MODE] == out_val))
		return;
	
	in_val |= NCP2704_PGAIN2G(AudioExtDev_GetPreGain()) |
			NCP2704_PGAIN1G(AudioExtDev_GetPreGain());

   AudioExtDev_SetOutput(audio_mode);
	mutex_lock(&amp_dev->ncp2704_lock);
	ncp2704_regs[NCP2704_INPUT_MODE] = in_val;
	ncp2704_regs[NCP2704_OUTPUT_MODE] = out_val;
	mutex_unlock(&amp_dev->ncp2704_lock);
	ncp2704_write_regs();
	/* NCP2704: after nSHDN = 1, delay is required to supress pop noise
	 * This delay is set to 100 milliseconds. Can be fine-tuned further
	 */
	msleep(100);  
}

EXPORT_SYMBOL(AudioExtDev_PowerOn);

void AudioExtDev_PowerOff(void)
{
	mutex_lock(&amp_dev->ncp2704_lock);
	ncp2704_regs[NCP2704_SPEAKER_VOL] = NCP2704_SPKVOL_MUTE;
	ncp2704_regs[NCP2704_LEFT_HEADSET_VOL] = NCP2704_SPKVOL_MUTE;
	ncp2704_regs[NCP2704_RIGHT_HEADSET_VOL] = NCP2704_SPKVOL_MUTE;
	ncp2704_regs[NCP2704_OUTPUT_MODE] &=~(NCP2704_SD);
	mutex_unlock(&amp_dev->ncp2704_lock);

	ncp2704_write_regs();
}

EXPORT_SYMBOL(AudioExtDev_PowerOff);

void AudioExtDev_SetPreGain(AudioExtPreGainNCP2704_t pregain)
{
	if(pregain > NCP2704_PGAIN_P20DB) 
	   NCP2704_pregain = NCP2704_PGAIN_P20DB;

   NCP2704_pregain = pregain;
}
EXPORT_SYMBOL(AudioExtDev_SetPreGain);

AudioExtPreGainNCP2704_t  AudioExtDev_GetPreGain(void)
{
	return NCP2704_pregain;
}
EXPORT_SYMBOL(AudioExtDev_GetPreGain);

#ifdef CONFIG_PM
static int ncp2704_suspend(struct i2c_client *client, pm_message_t state)
{
	return 0;
}

static int ncp2704_resume(struct i2c_client *client)
{
	return 0;
}
#endif
static int __devinit ncp2704_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	pr_info("-->>probing ncp2704 \n");

	amp_dev = kzalloc(sizeof(struct ncp2704_dev), GFP_KERNEL);
	amp_dev->ncp2704_client = client;
	amp_dev->out_mode = AMMP_OUT_IN2DIFF_SPK;
	mutex_init(&amp_dev->ncp2704_lock);
	i2c_set_clientdata(amp_dev->ncp2704_client, amp_dev);

	/*Config init values */
	ncp2704_regs[NCP2704_INPUT_MODE] =  NCP2704_PGAIN1G(NCP2704_PGAIN_P00DB) | NCP2704_PGAIN2G(NCP2704_PGAIN_P00DB);;
	ncp2704_regs[NCP2704_SPEAKER_VOL] = NCP2704_SPKVOL_MUTE;
	ncp2704_regs[NCP2704_LEFT_HEADSET_VOL] = NCP2704_SPKVOL_MUTE;
	ncp2704_regs[NCP2704_RIGHT_HEADSET_VOL] = NCP2704_SPKVOL_MUTE;
	ncp2704_regs[NCP2704_OUTPUT_MODE] = NCP2704_OUTMODE(AMMP_OUT_IN2DIFF_SPK);
	ncp2704_regs[NCP2704_LDO_MODE] =  0x00;
	ncp2704_regs[NCP2704_ACNT_MODE] = AGCE_DISABLE |NCL_DISABLE|PWR(7)|THD(7);
	ncp2704_regs[NCP2704_ACONFA_MODE] = ACONFA_00;
	ncp2704_regs[NCP2704_ACONFR_MODE] = ACONFR_00;
	ncp2704_regs[NCP2704_ACONFH_MODE] = ACONFH_00;
	ncp2704_regs[NCP2704_EMI_MODE] = EMI_20;
	ncp2704_write_regs();
	return 0;
}

static int __devexit ncp2704_remove(struct i2c_client *client)
{
	struct ncp2704_dev *r_dev = i2c_get_clientdata(client);
	r_dev->ncp2704_client = NULL;
	return 0;
}

static const struct i2c_device_id ncp2704_i2c_id[] = {
	{"ncp2704", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ncp2704_i2c_id);

static struct i2c_driver ncp2704_i2c_driver = {
	.driver = {
		   .name = "ncp2704",
		   .owner = THIS_MODULE,
		   },
	.probe = ncp2704_probe,
	.remove = ncp2704_remove,
#ifdef CONFIG_PM
	.suspend = ncp2704_suspend,
	.resume = ncp2704_resume,
#endif
	.id_table = ncp2704_i2c_id,
};

static int __init ncp2704_init(void)
{
	return i2c_add_driver(&ncp2704_i2c_driver);
}

static void __exit ncp2704_exit(void)
{
	i2c_del_driver(&ncp2704_i2c_driver);
}

subsys_initcall(ncp2704_init);
module_exit(ncp2704_exit);

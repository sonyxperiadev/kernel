/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
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

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "auddrv_def.h"

#include "audio_controller.h"
#include "audio_ddriver.h"
#include "bcm_audio_devices.h"

#include "caph_common.h"

extern int LaunchAudioCtrlThread(void);


//  Module declarations.
//
MODULE_AUTHOR("Broadcom MPS-Audio");
MODULE_DESCRIPTION("Broadcom CAPH sound interface");
MODULE_LICENSE("GPL");

//global
int gAudioDebugLevel = 2; 



static brcm_alsa_chip_t *sgpCaph_chip=NULL;


extern int BrcmCreateAuddrv_testSysFs(struct snd_card *card); //brcm_auddrv_test.c

//extern int BrcmCreateControlSysFs(struct snd_card *card); //brcm_alsa_ctl.c
//--------------------------------------------------------------------

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverProbe
//
//  Description: 'probe' call back function
//
//------------------------------------------------------------
static int __devinit DriverProbe(struct platform_device *pdev)
{
	struct snd_card *card;
	int err;
	
	printk(KERN_INFO "ALSA:In Driver Probe:\n");
	
	BCM_AUDIO_DEBUG("\n %lx:DriverProbe \n",jiffies);
		
	err = -ENODEV;
    
    err = -ENOMEM;  
	err  = snd_card_create(SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1, 
		THIS_MODULE, sizeof(brcm_alsa_chip_t), &card);
		
	if (!card)
      goto err;
            
	sgpCaph_chip = (brcm_alsa_chip_t*)card->private_data;
	sgpCaph_chip->card = card;
	
	card->dev = &pdev->dev;
	strncpy(card->driver, pdev->dev.driver->name, sizeof(card->driver));

	//PCM interface	
	err = PcmDeviceNew(card);
	if (err)
    	goto err;
    //CTRL interface	
	err = ControlDeviceNew(card);
	if (err)
    	goto err;
	


	//TODO: other interface
	
	
	strcpy(card->driver, "Broadcom");
	strcpy(card->shortname, "Broadcom ALSA");
	sprintf(card->longname, "Broadcom ALSA PCM %i", 0);
	
	
	err = snd_card_register(card);
	if (err==0)
	{
      int ret;
      platform_set_drvdata(pdev, card);

      ret = BrcmCreateAuddrv_testSysFs(card);
      if(ret!=0)
 	  	BCM_AUDIO_DEBUG("ALSA DriverProbe Error to create sysfs for Auddrv test ret = %d\n", ret);
      
#if 0      
	ret = BrcmCreateControlSysFs(card);
	if(ret!=0)
		BCM_AUDIO_DEBUG("ALSA DriverProbe Error to create sysfs for FMDirectPlay ret = %d\n", ret);
#endif
      return 0;
	}

err:
	BCM_AUDIO_DEBUG("\n probe failed =%d\n",err);
	if (card)
		snd_card_free(card);
	
	sgpCaph_chip=NULL;
	return err;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverRemove
//
//  Description: 'remove' call back function
//
//------------------------------------------------------------
static int DriverRemove(struct platform_device *pdev)
{
	return 0;
}


static int DriverSuspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DriverResume
//
//  Description: 'resume' call back function
//
//------------------------------------------------------------
static int DriverResume(struct platform_device *pdev)
{
	return 0;
}

//---------------------------------------------------------------------------
// File opeations for audio logging

//Platform device structure
static struct platform_device sgPlatformDevice =
{
       .name           = "brcm_caph_device",
       .id             = -1,
};


//Platfoorm driver structure
static struct platform_driver sgPlatformDriver =
{
	.probe		= DriverProbe,
	.remove 	= DriverRemove,
	.suspend	= DriverSuspend,
	.resume		= DriverResume,
	.driver		= 
		{
		.name	= "brcm_caph_device",
		.owner	= THIS_MODULE,
		},
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ModuleInit
//
//  Description: Module initialization
//
//------------------------------------------------------------
static int __devinit ALSAModuleInit(void)
{
	int err = 0;
	
	
	BCM_AUDIO_DEBUG(KERN_INFO "ALSA Module init called:\n");


	err =  platform_device_register(&sgPlatformDevice);
       BCM_AUDIO_DEBUG("\n %lx:device register done %d\n",jiffies,err);
	if (err)
        {
	       return err;
	}
	printk(KERN_INFO "platform device register done:\n");
	
	err = platform_driver_register(&sgPlatformDriver);
	BCM_AUDIO_DEBUG("\n %lx:driver register done %d\n",jiffies,err);
	if(err) 
	 {
		return err;
	}
        printk(KERN_INFO "platform_driver_register done:\n");
        
    LaunchAudioCtrlThread();

	return err;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ModuleExit
//
//  Description: Module de-initialization
//
//------------------------------------------------------------
static void __devexit ALSAModuleExit(void)
{
	//int err;
	
	BCM_AUDIO_DEBUG("\n %lx:ModuleExit\n",jiffies);

	snd_card_free(sgpCaph_chip->card);
	
	platform_driver_unregister(&sgPlatformDriver);

	platform_device_unregister(&sgPlatformDevice);
    //TerminateAudioHalThread();
	
	BCM_AUDIO_DEBUG("\n %lx:exit done \n",jiffies);
}

module_init(ALSAModuleInit);
module_exit(ALSAModuleExit);


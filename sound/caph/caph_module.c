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

#include <linux/broadcom/bcm_major.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "drv_caph.h"


#include "audio_controller.h"
#include "dspif_voice_play.h"
#include "audio_ddriver.h"
#include "bcm_audio_devices.h"

#include "shared.h"
#include "auddrv_audlog.h"

#include "caph_common.h"
#include "bcm_audio.h"

extern int LaunchAudioCtrlThread(void);

extern int TerminateAudioHalThread(void);


//  Module declarations.
//
MODULE_AUTHOR("Broadcom MPS-Audio");
MODULE_DESCRIPTION("Broadcom CAPH sound interface");
MODULE_LICENSE("GPL");

//global
int gAudioDebugLevel = 0; 
static brcm_alsa_chip_t *sgpCaph_chip=NULL;

// AUDIO LOGGING

#define DATA_TO_READ 4
int logging_link[LOG_STREAM_NUMBER] = {0,0,0,0};

// wait queues
extern wait_queue_head_t bcmlogreadq;
int audio_data_arrived = 0;

extern UInt16 *bcmlog_stream_area;


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
// File operations for audio logging

static int
BCMAudLOG_open(struct inode *inode, struct file *file)
{

	BCM_AUDIO_DEBUG("\n BCMLOG_open \n");

    return 0;
} 


/****************************************************************************
*
*  BCMLOG_read
*
***************************************************************************/

static int
BCMAudLOG_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	BCM_AUDIO_DEBUG("\n BCMLOG_read \n");

    if ( wait_event_interruptible(bcmlogreadq, (audio_data_arrived != 0)))
    {   
        //(" Wait for read  ...\n");
        return -ERESTARTSYS;
    }   
    audio_data_arrived = 0;  

	ret = copy_to_user(buf, "read", 4); 
    return DATA_TO_READ;
          
}

/****************************************************************************
*
*  BCMLOG_release
*
***************************************************************************/

static int
BCMAudLOG_release(struct inode *inode, struct file *file)
{
	BCM_AUDIO_DEBUG("\n BCMLOG_release \n");

    return 0;

} /* BCMLOG_release */

/****************************************************************************
*
*  BCMLOG_mmap
*
***************************************************************************/

static int BCMAudLOG_mmap(struct file *filp, struct vm_area_struct *vma)
{
    int ret;
    long length = vma->vm_end - vma->vm_start;

    BCM_AUDIO_DEBUG("\n BCMLOG_mmap \n");

   // check length - do not allow larger mappings than the number of pages allocated 
    if (length > (PAGE_SIZE + (sizeof(LOG_FRAME_t)*4)) )
    {
        DEBUG("\n Failed at page boundary \n\r");
        return -EIO;
    }


   // map the whole physically contiguous area in one piece
    ret = remap_pfn_range(vma, vma->vm_start,
                    virt_to_phys((void *)bcmlog_stream_area) >> PAGE_SHIFT,
                    length, vma->vm_page_prot);

    if(ret != 0) {
            DEBUG("\n BCMLOG_mmap_kmem -EAGAIN \r\n");
            return -EAGAIN;
   	   }

	 return 0;
}


/****************************************************************************
*
*  BCMLOG_ioctl - IOCTL to set switch to kernel mode for
                1. Audio logging setup
                2. Dump Audio data to MTT &  Copy audio data to user space
                3. Stop and reset audio logging
*
***************************************************************************/

static int BCMAudLOG_ioctl(struct inode *inode, struct file *file,
             unsigned int cmd, unsigned long arg)
{

    int rtn = 0;
	BCM_AUDIO_DEBUG("\n BCMLOG_ioctl cmd=0x%x\n",cmd);

    switch (cmd)
    {
        case BCM_LOG_IOCTL_CONFIG_CHANNEL:
	        break;
 		
		case BCM_LOG_IOCTL_START_CHANNEL:
        {
                                                               
            AUDDRV_CFG_LOG_INFO *p_log_info2 = (AUDDRV_CFG_LOG_INFO *) (arg) ;       
			logging_link[p_log_info2->log_link-1] = 1;
            rtn= AUDDRV_AudLog_Start(p_log_info2->log_link,p_log_info2->log_capture_point,p_log_info2->log_consumer,(char *) NULL);
            if (rtn < 0 )
            {
                BCM_AUDIO_DEBUG("\n Couldnt setup channel \n");
                rtn = -1;
            }
         }		       
    	 break;
		
		case BCM_LOG_IOCTL_STOP:
        {
            AUDDRV_CFG_LOG_INFO *p_log_info3 = (AUDDRV_CFG_LOG_INFO *) (arg) ;
            logging_link[p_log_info3->log_link-1] = 0;
            rtn = AUDDRV_AudLog_Stop(p_log_info3->log_link);
            if (rtn < 0 )
            {
                rtn = -1;
            }
        }
        break;
		default:
		{
			BCM_AUDIO_DEBUG("\n Wrong IOCTL cmd \n");
			rtn = -1;
		}
		break;
    }	
	return rtn;
}

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


static struct class *audlog_class;
//--------------------------------------------------------------------
// File operations for audio logging
static struct file_operations bcmlog_fops =
{
    .owner =    THIS_MODULE,
    .open =    BCMAudLOG_open,
    .read =    BCMAudLOG_read,
    .ioctl =   BCMAudLOG_ioctl,
    .mmap =    BCMAudLOG_mmap,
    .release = BCMAudLOG_release,
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
	   return err;
			
	err = platform_driver_register(&sgPlatformDriver);
	BCM_AUDIO_DEBUG("\n %lx:driver register done %d\n",jiffies,err);
	if(err) 
		return err;
		
	LaunchAudioCtrlThread();
        
    // Device for audio logging

    if ((err = register_chrdev(BCM_ALSA_LOG_MAJOR, "bcm_audio_log", &bcmlog_fops)) < 0)
    {
        return err;
    }  
    audlog_class = class_create(THIS_MODULE, "bcm_audio_log");
    if (IS_ERR(audlog_class))
    {
        return PTR_ERR(audlog_class);
    }

    device_create(audlog_class, NULL, MKDEV(BCM_ALSA_LOG_MAJOR, 0),NULL, "bcm_audio_log");
    init_waitqueue_head(&bcmlogreadq);
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
    TerminateAudioHalThread();
	
	BCM_AUDIO_DEBUG("\n %lx:exit done \n",jiffies);
}

module_init(ALSAModuleInit);
module_exit(ALSAModuleExit);


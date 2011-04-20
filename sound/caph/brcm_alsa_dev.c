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

//#include "mobcom_types.h"
//#include "resultcode.h"
//#include "audio_consts.h"

#include "brcm_alsa.h"
//#include "brcm_audio_thread.h"

extern int  ControlDeviceNew(struct snd_card *card)


#if KISHORE_COMMENT
#include <linux/broadcom/hw_cfg.h>
#include <linux/broadcom/bcm_major.h>
#include "audio_ipc_consts.h"
#include "shared.h"
#include "memmap.h"
#include "hal_audio_config.h"
#include "audio_consts.h"
#include <linux/broadcom/bcm_fuse_sysparm.h>



#include "brcm_alsa_pcg.h"
#include "audvoc_drv.h"
#include "hal_audio.h"

extern SysAudioParm_t* SYSPARM_GetAudioParmAccessPtr(UInt8 AudioMode);

Result_t HAL_AUDIO_Ctrl_Blocking(
	HAL_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	);

extern AUDIO_CHANNEL_t AUDIO_DRV_ConvertModeChnl(AudioMode_t);

#endif

	
//  Module declarations.
//
MODULE_AUTHOR("Broadcom MobMM");
MODULE_DESCRIPTION("Broadcom AP sound interface");
MODULE_LICENSE("GPL");
//MODULE_SUPPORTED_DEVICE("{{ALSA,Broadcom AP soundcard}}");

//global
brcm_alsa_chip_t *g_brcm_alsa_chip=NULL;
int debug = 2; 

//
static char *id = NULL;
static int enable = 1;
static int index = 0;

//
#if 0
module_param(index, int, 0444);
MODULE_PARM_DESC(index, "Index value for Broadcom soundcard.");
module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for Broadcom soundcard.");
module_param(enable, bool, 0444);
MODULE_PARM_DESC(enable, "Enable the Broadcom soundcard.");
module_param(debug, int, 0444);
MODULE_PARM_DESC(debug, "debug value for Broadcom soundcard.");

#endif
//--------------------------------------------------------------------
// AUDIO LOGGING

#if KISHORE_COMMENT

#define DATA_TO_READ 4
int logging_link[LOG_STREAM_NUMBER] = {0,0,0,0};

// wait queues
extern wait_queue_head_t bcmlogreadq;

static int audio_stop = 0;
int audio_data_arrived = 0;


// IOCTL for audio logging
#define LOG_IOCTL_CONFIG_CHANNEL                105
#define LOG_IOCTL_START_CHANNEL                 106
#define LOG_IOCTL_STOP                          107


extern UInt16 *bcmlog_stream_area;

#endif

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
	
#if 1
	 printk(KERN_INFO "ALSA:In Driver Probe:\n");
	
	DEBUG("\n %lx:DriverProbe \n",jiffies);
		
	err = -ENODEV;
	if (!enable)
      return err;
    
    err = -ENOMEM;  
	err  = snd_card_create(SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1, 
		THIS_MODULE, sizeof(brcm_alsa_chip_t), &card);
		
	if (!card)
      goto err;
            
	g_brcm_alsa_chip = (brcm_alsa_chip_t*)card->private_data;
	g_brcm_alsa_chip->card = card;
	
	card->dev = &pdev->dev;
	strncpy(card->driver, pdev->dev.driver->name, sizeof(card->driver));

#if 1

	//PCM interface	
	err = PcmDeviceNew(card);
	if (err)
    	goto err;
#endif	
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

#if 1
      ret = BrcmCreateAuddrv_testSysFs(card);
      if(ret!=0)
 	  	DEBUG("ALSA DriverProbe Error to create sysfs for Auddrv test ret = %d\n", ret);
      
#endif      
#if 0      
	ret = BrcmCreateControlSysFs(card);
	if(ret!=0)
		DEBUG("ALSA DriverProbe Error to create sysfs for FMDirectPlay ret = %d\n", ret);
#endif
      return 0;
	}

err:
	DEBUG("\n probe failed =%d\n",err);
	if (card)
		snd_card_free(card);
	
	g_brcm_alsa_chip=NULL;
#endif
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


static int DriverSuspend(
		struct platform_device *pdev, pm_message_t state)
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

#if KISHORE_COMMENT
static int
BCMLOG_open(struct inode *inode, struct file *file)
{

    return 0;
} /* BCMLOG_open */


/****************************************************************************
*
*  BCMLOG_read
*
***************************************************************************/
BCMLOG_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{


    if ( wait_event_interruptible(bcmlogreadq, (audio_data_arrived != 0)))
    {   
        //(" Wait for read  ...\n");
        return -ERESTARTSYS;
    }   
    audio_data_arrived = 0;  

	copy_to_user(buf, "read", 4); 
    return DATA_TO_READ;
               
}

/****************************************************************************
*
*  BCMLOG_release
*
***************************************************************************/

static int
BCMLOG_release(struct inode *inode, struct file *file)
{

    return 0;

} /* BCMLOG_release */

/****************************************************************************
*
*  BCMLOG_mma
*
***************************************************************************/

static int BCMLOG_mmap(struct file *filp, struct vm_area_struct *vma)
{


    int ret;
    long length = vma->vm_end - vma->vm_start;

    /* check length - do not allow larger mappings than the number of pages allocated */
    if (length > (PAGE_SIZE + (sizeof(LOG_FRAME_t)*4)) )
    {
        DEBUG("\n Failed at page boundary \n\r");
        return -EIO;
    }


    /* map the whole physically contiguous area in one piece */
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

static int
BCMLOG_ioctl(struct inode *inode, struct file *file,
             unsigned int cmd, UInt16 arg)
{

    int rtn = 0;


    switch (cmd)
    {

        case LOG_IOCTL_CONFIG_CHANNEL:
        {

            AUDVOC_CFG_LOG_INFO *p_log_info1 = (AUDVOC_CFG_LOG_INFO *) (arg) ;
            rtn= Audio_configure_log_channel (p_log_info1);
            if (rtn < 0 )
            {
                pr_info("\n Couldnt setup channel \n\n");
                rtn = -1;
            }

        }
        break;
 		
		case LOG_IOCTL_START_CHANNEL:
        {
                                                               
            AUDVOC_CFG_LOG_INFO *p_log_info2 = (AUDVOC_CFG_LOG_INFO *) (arg) ;

			logging_link[p_log_info2->log_link-1] = 1;
            rtn= Audio_start_log_channel(p_log_info2);
            if (rtn < 0 )
            {
                pr_info("\n Couldnt setup channel \n\n");
                rtn = -1;
            }
        }
        break;
		
		case LOG_IOCTL_STOP:
        {
            AUDVOC_CFG_LOG_INFO *p_log_info3 = (AUDVOC_CFG_LOG_INFO *) (arg) ;
            logging_link[p_log_info3->log_link-1] = 0;
            rtn = Audio_stop_log_channel(p_log_info3);
            if (rtn < 0 )
            {
                rtn = -1;
            }
        }
        break;
    }
    return(rtn);
}
//---------------------------------------------------------------------------


/**************************************************************************
*
*  BCMPCG_open
*
***************************************************************************/

static int
BCMPCG_open(struct inode *inode, struct file *file)
{

    return 0;
} /* BCMPCG_open */

/****************************************************************************
*
*  BCMPCG_release
*
***************************************************************************/

static int
BCMPCG_release(struct inode *inode, struct file *file)
{

    return 0;

} /* BCMPCG_release */



/****************************************************************************
*
*  BCMPCG_ioctl - TODO - lots of stuff needs to be filled in
*
***************************************************************************/

static int
BCMPCG_ioctl(struct inode *inode, struct file *file,
             unsigned int cmd, UInt32 arg)
{
    int rtn = 1;
    HAL_AUDIO_Control_Params_un_t audioParam;
    HAL_AUDIO_Get_Param_t getParam;
    // DEBUG(" BCMPCG_ioctl:  cmd: %d  arg =%d  \r\n",_IOC_NR(cmd),  arg);

    const UInt16 volume_max = SYSPARM_GetAudioParmAccessPtr(AUDIO_MODE_HANDSET)->voice_volume_max;

    switch (cmd)
    {
    case PCG_IOCTL_SETMODE:
    {
        UInt32 pcg_arg;
        AudioClientID_en_t audioID = AUDIO_ID_CALL;

        //per pcg request, need to keep vol same among audio modes
        getParam.paramType = GET_AUDIO_VOLUMELEVEL;
        HAL_AUDIO_Ctrl_Blocking(ACTION_AUD_GetParam, &getParam, NULL);

        copy_from_user(&pcg_arg, (UInt32 *)arg, sizeof(UInt32));
        // printk(" BCMPCG_ioctl:  pcg_arg=%d  \r\n", pcg_arg);
        audioParam.param_pathCfg.audioID = audioID;
        audioParam.param_pathCfg.outputChnl = (AudioMode_t)pcg_arg;

        DEBUG("AUD:PCG_IOCTL_SETMODE: *(getParam.paramPtr)=%d\n", *(getParam.paramPtr));
        audioParam.param_pathCfg.volumeLevel = (UInt8)(*(getParam.paramPtr));
        HAL_AUDIO_Ctrl_Blocking(ACTION_AUD_EnablePath, &audioParam, NULL);

        rtn = 1;
    }
    break;

    case PCG_IOCTL_GETMODE:
    {
        UInt32 curmode;
        getParam.paramType = GET_AUDIO_MODE;
        if( (HAL_AUDIO_Ctrl_Blocking( ACTION_AUD_GetParam, &getParam, NULL))!=0)
            rtn = -1;
		
        curmode = *getParam.paramPtr;
        copy_to_user((UInt32 *)arg, &curmode, sizeof(UInt32));

        DEBUG("AUD:PCG_IOCTL_GETMODE curmode=%x  \n",curmode);
        rtn=1;
    }
    break;


    case PCG_IOCTL_GETVOL:
    {
        UInt32 volume;
        getParam.paramType= GET_AUDIO_VOLUMELEVEL;
        if ((rtn=HAL_AUDIO_Ctrl_Blocking(ACTION_AUD_GetParam, &getParam, NULL)) !=0)
            rtn=-1;

        volume = *getParam.paramPtr;
        copy_to_user( (UInt32 *)arg, &volume, sizeof(UInt32));
	pr_info("AUD:PCG_IOCTL_GETVOL volume=%x  \n",volume);

        rtn = 1;
    }
    break;


    case PCG_IOCTL_SETVOL:
    {
        //UInt32 volume_max=0 ;

        UInt32 spkvol;
        copy_from_user(&spkvol, (UInt32 *)arg, sizeof(UInt32));

        //volume_max = (UInt32)SYSPARM_GetAudioParmAccessPtr(AUDIO_MODE_HANDSET)->voice_volume_max;

        if(spkvol >volume_max)
            spkvol = volume_max;
		
        DEBUG("AUD:  PCG_IOCTL_SETVOL   spkvol=%d  volume_max=%d  \r\n", spkvol,volume_max );
        //  HAL_AUDIO_Ctrl( ACTION_AUD_SetVolumeWithPath, &audioParam, NULL);
        if ((rtn = HAL_AUDIO_Ctrl_Blocking(ACTION_AUD_SetSpeakerVol, &spkvol, NULL)) !=0)
            rtn = -1;
    }
    break;

    default:
        DEBUG("Unrecognized ioctl: '0x%x'  \n", cmd);
        return -ENOTTY;
    }

    return (rtn);

} /* lcd_ioctl */

#endif


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DeviceRelease
//
//  Description: 'release' call back function for platform device
//
//------------------------------------------------------------
static void DeviceRelease(struct device *pdev)
{
       DEBUG("\n TO DO:DeviceRelease\n");
}



//Platform device structure
static struct platform_device sgPlatformDevice =
{
       .name           = "brcm_alsa_device",
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
		.name	= "brcm_alsa_device",
		.owner	= THIS_MODULE,
		},
};
#if KISHORE_COMMENT
static struct file_operations bcmpcg_fops =
{
    .owner=  THIS_MODULE,
    .open =   BCMPCG_open,
    .ioctl  =   BCMPCG_ioctl,
    .mmap = NULL,
    .release = BCMPCG_release,
};

static struct class *aud_class;
static struct class *audlog_class;

//--------------------------------------------------------------------
// File operations for audio logging
static struct file_operations bcmlog_fops =
{
    .owner=    THIS_MODULE,
    .open =    BCMLOG_open,
    .read =    BCMLOG_read,
    .ioctl  =   BCMLOG_ioctl,
    .mmap =    BCMLOG_mmap,
    .release =         BCMLOG_release,
};

#endif
//--------------------------------------------------------------------


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
	
	
#if 1	
	DEBUG("\n %lx:ModuleInit debg=%d id=%s\n",jiffies,debug,id);
	 printk(KERN_INFO "ALSA Module init called:\n");


	err =  platform_device_register(&sgPlatformDevice);
       DEBUG("\n %lx:device register done %d\n",jiffies,err);
	if (err)
        {
	       return err;
	}
	printk(KERN_INFO "platform device register done:\n");
	
	err = platform_driver_register(&sgPlatformDriver);
	DEBUG("\n %lx:driver register done %d\n",jiffies,err);
	if(err) 
	 {
		return err;
	}
        printk(KERN_INFO "platform_driver_register done:\n");
        
#endif
    //LaunchAudioCtrlThread();

#if KISHORE_COMMENT
    if ((err = register_chrdev(BCM_ALSA_PCG_MAJOR, "bcm_alsa_pcg", &bcmpcg_fops)) < 0)
    {
        return err;
    }

    aud_class = class_create(THIS_MODULE, "bcm_alsa_pcg");
    if (IS_ERR(aud_class))
    {
        return PTR_ERR(aud_class);
    }
    device_create(aud_class, NULL, MKDEV(BCM_ALSA_PCG_MAJOR, 0),NULL, "bcm_alsa_pcg");

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

	// Initialize the AUDIO device

	audvoc_init();
#endif

	
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
	
	DEBUG("\n %lx:ModuleExit\n",jiffies);

	snd_card_free(g_brcm_alsa_chip->card);
	
	platform_driver_unregister(&sgPlatformDriver);

	platform_device_unregister(&sgPlatformDevice);
    //TerminateAudioHalThread();
	
	// unInitialize the AUDIO device
#if KISHORE_COMMENT
	audvoc_deinit();
#endif
	DEBUG("\n %lx:exit done \n",jiffies);
}

module_init(ALSAModuleInit);
module_exit(ALSAModuleExit);

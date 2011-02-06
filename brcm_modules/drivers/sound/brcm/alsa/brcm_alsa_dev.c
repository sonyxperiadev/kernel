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
#include <linux/wait.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>

#include <linux/broadcom/hw_cfg.h>
#include <linux/broadcom/bcm_major.h>
#include "audio_ipc_consts.h"
#include "shared.h"
#include "memmap.h"
#include "hal_audio_config.h"
#include "audio_consts.h"
#include <linux/broadcom/bcm_fuse_sysparm.h>


#include "brcm_alsa.h"
#include "brcm_alsa_pcg.h"
#include "audvoc_drv.h"
#include "hal_audio.h"
#include "vpu.h"

extern SysAudioParm_t* SYSPARM_GetAudioParmAccessPtr(UInt8 AudioMode);

Result_t HAL_AUDIO_Ctrl_Blocking(
	HAL_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	);
AudioMode_t	AUDIO_GetAudioMode();

extern AUDIO_CHANNEL_t AUDIO_DRV_ConvertModeChnl(AudioMode_t);
extern int BrcmCreateControlSysFs(struct snd_card *card); //brcm_alsa_ctl.c
extern 	UInt16	AUDIO_Util_Convert( UInt16 input, UInt16 scale_in, UInt16 scale_out);
	
//  Module declarations.
//
MODULE_AUTHOR("Broadcom MobMM");
MODULE_DESCRIPTION("Broadcom AP sound interface");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA,Broadcom AP soundcard}}");

//global
brcm_alsa_chip_t *g_brcm_alsa_chip=NULL;
int debug = 2; 

//
static char *id = NULL;
static int enable = 1;
static int index = 0;

//
module_param(index, int, 0444);
MODULE_PARM_DESC(index, "Index value for Broadcom soundcard.");
module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for Broadcom soundcard.");
module_param(enable, bool, 0444);
MODULE_PARM_DESC(enable, "Enable the Broadcom soundcard.");
module_param(debug, int, 0444);
MODULE_PARM_DESC(debug, "debug value for Broadcom soundcard.");


//--------------------------------------------------------------------
// AUDIO LOGGING

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

#define VOIP_IOCTL_ENABLE                111
#define VOIP_IOCTL_DISABLE               112
#define readbuffnodata 99 
static HAL_AUDIO_Param_PathCfg_t   stVoIP_path;     //for VPM only
static UInt16	 voip_codec_type;

// loopback test code from dsp team
#define WBAMR_DATA_SIZE		320*35
UInt16 wbamr_downlinkdata[WBAMR_DATA_SIZE];
UInt16 wbamr_uplinkldata[WBAMR_DATA_SIZE];

UInt16 wbamr_tempdata [160];

UInt16 wbamr_downlink_index=0;
UInt16 wbamr_uplink_index=0;

UInt16 wbamr_frame_count=0;

UInt16 wbamr_data[WBAMR_DATA_SIZE];
UInt16 wbamr_dl_index=0;
UInt16 wbamr_ul_index=0;

int readbuffLen;
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
      ret = BrcmCreateControlSysFs(card);
	  if(ret!=0)
	  	DEBUG("ALSA DriverProbe Error to create sysfs for FMDirectPlay ret = %d\n", ret);
      return 0;
	}

err:
	DEBUG("\n probe failed =%d\n",err);
	if (card)
		snd_card_free(card);
	
	g_brcm_alsa_chip=NULL;
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

//Platform device data
typedef struct _PlatformDevData_t
{
	int init;	
} PlatformDevData_t;

static PlatformDevData_t sgDeviceInfo =
{
	.init = 0,
};                                

//---------------------------------------------------------------------------
// File opeations for audio logging


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

#define pgprot_cached(prot) \
__pgprot((pgprot_val(prot) & ~L_PTE_MT_MASK) | L_PTE_MT_WRITETHROUGH)

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

	vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
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


/**************************************************************************
*
*  BCMVOIP_open
*
***************************************************************************/
static int
BCMVOIP_open(struct inode *inode, struct file *file)
{
    return 0;
} /* BCMVOIP_open */

/****************************************************************************
*
*  BCMVOIP_release
*
***************************************************************************/
static int
BCMVOIP_release(struct inode *inode, struct file *file)
{
    return 0;
} /* BCMVOIP_release */

/****************************************************************************
*
*  BCMVOIP_read
*
***************************************************************************/
BCMVOIP_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    //pr_info("BCMVOIP_read:%d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",wbamr_uplink_index,
      //wbamr_uplinkldata[wbamr_uplink_index],wbamr_uplinkldata[wbamr_uplink_index+1], wbamr_uplinkldata[wbamr_uplink_index+2],wbamr_uplinkldata[wbamr_uplink_index+3],
      //wbamr_uplinkldata[wbamr_uplink_index+4],wbamr_uplinkldata[wbamr_uplink_index+5], wbamr_uplinkldata[wbamr_uplink_index+6],wbamr_uplinkldata[wbamr_uplink_index+7]);

	 readbuffLen = wbamr_ul_index - wbamr_uplink_index ;
	
    if(readbuffLen == 0)
    { 
        //if the buff no data return 99;
        return readbuffnodata;
    }
    else
    {
        copy_to_user(buf,wbamr_uplinkldata+wbamr_uplink_index,320);
        wbamr_uplink_index +=160;
        if( wbamr_uplink_index >= WBAMR_DATA_SIZE ) wbamr_uplink_index = 0;
    }
    
    return 320;
 }

/****************************************************************************
* 
*  BCMVOIP_write
*
***************************************************************************/
BCMVOIP_write(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    copy_from_user(wbamr_downlinkdata+wbamr_downlink_index, buf, count);
	
    //pr_info("BCMVOIP_write: %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",wbamr_downlink_index,
	  //wbamr_downlinkdata[wbamr_downlink_index],wbamr_downlinkdata[wbamr_downlink_index+1], wbamr_downlinkdata[wbamr_downlink_index+2],wbamr_downlinkdata[wbamr_downlink_index+3],
	  //wbamr_downlinkdata[wbamr_downlink_index+4],wbamr_downlinkdata[wbamr_downlink_index+5], wbamr_downlinkdata[wbamr_downlink_index+6],wbamr_downlinkdata[wbamr_downlink_index+7]); 

	wbamr_downlink_index=wbamr_downlink_index+(count>>1);

   if( wbamr_downlink_index >= WBAMR_DATA_SIZE ) 
	{
        wbamr_downlink_index = 0;
	}

    return 320;
}

/****************************************************************************
* 
*  VoIPTelephony_Render_Fill
*
***************************************************************************/
static Boolean VoIPTelephony_Render_Fill(
			     UInt8*			ppDst,	 // pointer to start of speech data
			     UInt32         nFrames	 // number of frames
			     )
{
    UInt16 *pDst; 
    int buffLen = wbamr_downlink_index- wbamr_dl_index;
 
	if(buffLen == 0)
	{
	    //pr_info("VoIPTelephony_Render_Fill buffLen=%d r\n",buffLen); 
        memset(wbamr_tempdata, 0, sizeof(wbamr_tempdata));
		pDst=&wbamr_tempdata[0]; 
        *((UInt32 *)ppDst)= (UInt32)pDst; 
    }
	else
    {
        pDst=wbamr_downlinkdata+wbamr_dl_index; 
        *((UInt32 *)ppDst)= (UInt32)pDst; 
	    // loopback test from dsp team 
	    if ((voip_codec_type&0xf000) == 0x1000)
		    wbamr_dl_index += 160;  
	    if ((voip_codec_type&0xf000) == 0x2000)
		    wbamr_dl_index += 80; 
	    if ((voip_codec_type&0xf000) == 0x3000)
		    wbamr_dl_index += 32; 

 	    if( wbamr_dl_index >= WBAMR_DATA_SIZE ) 
 		{
 		    wbamr_dl_index = 0;
 		}
    }

 	return 1; 				
}

/****************************************************************************
* 
*  VoIPTelephony_Capture_Dump 
*
***************************************************************************/
static Boolean VoIPTelephony_Capture_Dump(
		UInt8*			pSrc,		// pointer to start of speech data
		UInt32			data_len	// data length
		)
{

  	memcpy(wbamr_uplinkldata+wbamr_ul_index, pSrc, data_len<<1);

	//pr_info("AUDIO_Captur:%d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",wbamr_ul_index,
		//wbamr_uplinkldata[wbamr_ul_index],wbamr_uplinkldata[wbamr_ul_index+1], wbamr_uplinkldata[wbamr_ul_index+2],wbamr_uplinkldata[wbamr_ul_index+3],
		//wbamr_uplinkldata[wbamr_ul_index+4],wbamr_uplinkldata[wbamr_ul_index+5], wbamr_uplinkldata[wbamr_ul_index+6],wbamr_uplinkldata[wbamr_ul_index+7]); 

	// repeat dsp test harness 
	if ((voip_codec_type&0xf000) == 0x1000)
        wbamr_ul_index += 160; 
	 
	if( wbamr_ul_index >= WBAMR_DATA_SIZE ) wbamr_ul_index = 0;

    return 1; 
}

static Boolean VoIPTelephony_Render_Fill_Test(
			     UInt8*			ppDst,	 // pointer to start of speech data
			     UInt32			nFrames	 // number of frames
			     )
{
  	UInt16 *pDst; 

	//pr_info("AUDIO_Render: %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n\r", wbamr_dl_index,
		//wbamr_data[wbamr_dl_index],wbamr_data[wbamr_dl_index+1], wbamr_data[wbamr_dl_index+2],wbamr_data[wbamr_dl_index+3],
		//wbamr_data[wbamr_dl_index+4],wbamr_data[wbamr_dl_index+5], wbamr_data[wbamr_dl_index+6],wbamr_data[wbamr_dl_index+7]); 

	pDst=&wbamr_data[wbamr_dl_index]; 

 	*((UInt32 *)ppDst)= (UInt32)pDst; 

	// loopback test from dsp team
	if ((voip_codec_type&0xf000) == 0x1000)
		wbamr_dl_index += 160;  
	if ((voip_codec_type&0xf000) == 0x2000)
		wbamr_dl_index += 80; 
	if ((voip_codec_type&0xf000) == 0x3000)
		wbamr_dl_index += 32; 

 	if( wbamr_dl_index >= WBAMR_DATA_SIZE ) wbamr_dl_index = 0;

 	return 1; 
					
}


static Boolean VoIPTelephony_Capture_Dump_Test(
		UInt8*			pSrc,		// pointer to start of speech data
		UInt32			data_len	// data length
		)
{

 	memcpy(wbamr_data+wbamr_ul_index, pSrc, data_len<<1);
	
	//pr_info("AUDIO_Captur: %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n\r", wbamr_ul_index,
		//wbamr_data[wbamr_ul_index],wbamr_data[wbamr_ul_index+1], wbamr_data[wbamr_ul_index+2],wbamr_data[wbamr_ul_index+3],
		//wbamr_data[wbamr_ul_index+4],wbamr_data[wbamr_ul_index+5], wbamr_data[wbamr_ul_index+6],wbamr_data[wbamr_ul_index+7]); 

	if ((voip_codec_type&0xf000) == 0x1000)
		wbamr_ul_index += 160;   
       else if ((voip_codec_type&0xf000) == 0x2000)
       {
				//tx=>rx
		if( wbamr_data[wbamr_ul_index+2] == 0 )			//tx dtx_enable=0
		{
			wbamr_data[wbamr_ul_index] = 0;				//rx BFI=0
			wbamr_data[wbamr_ul_index+1] = 0;			//rx SID=0
			wbamr_data[wbamr_ul_index+2] = 0;			//rx TAF=0
		}
		else
		{
			if( wbamr_data[wbamr_ul_index+1] == 0 )			//tx SP=0, dtx
			{ 
				if( wbamr_frame_count == 0 )
				{
					wbamr_data[wbamr_ul_index] = 0;				//rx BFI=0
					wbamr_data[wbamr_ul_index+1] = 2;			//rx SID=2
					wbamr_data[wbamr_ul_index+2] = 1;			//rx TAF=1
				}
				else
				{
					wbamr_data[wbamr_ul_index] = 1;				//rx BFI=1
					wbamr_data[wbamr_ul_index+1] = 0;			//rx SID=0
					wbamr_data[wbamr_ul_index+2] = 0;			//rx TAF=0
				}
				wbamr_frame_count++;
				if( wbamr_frame_count == 24 ) wbamr_frame_count = 0;
			}
			else if( wbamr_data[wbamr_ul_index+1] == 1 )			//tx SP=1, voice
			{ 
				wbamr_data[wbamr_ul_index] = 0;				//rx BFI=0
				wbamr_data[wbamr_ul_index+1] = 0;			//rx SID=0
				wbamr_data[wbamr_ul_index+2] = 0;			//rx TAF=0
				wbamr_frame_count = 0;
			}
		}
		wbamr_ul_index += 80; 
       }
	else if ((voip_codec_type&0xf000) == 0x3000)
	{
		if( wbamr_data[wbamr_ul_index] == 1 ) wbamr_data[wbamr_ul_index] = 4;
		else if ( wbamr_data[wbamr_ul_index] == 2 ) wbamr_data[wbamr_ul_index] = 5;
		else if ( wbamr_data[wbamr_ul_index] == 3 ) wbamr_data[wbamr_ul_index] = 7;
		wbamr_ul_index += 32; 
	}
	
	if( wbamr_ul_index >= WBAMR_DATA_SIZE ) wbamr_ul_index = 0;
  
	return 1;
}


 static int
BCMVOIP_ioctl(struct inode *inode, struct file *file,
             unsigned int cmd, UInt32 arg)
{
    int rtn = 1;
    HAL_AUDIO_Control_Params_un_t audioParam;
    HAL_AUDIO_Get_Param_t getParam;
    int i;
    DEBUG(" BCMVOIP_ioctl:  cmd: %d  arg =%d  \r\n",_IOC_NR(cmd),  arg);

    const UInt16 volume_max = SYSPARM_GetAudioParmAccessPtr(AUDIO_MODE_HANDSET)->voice_volume_max;

    switch (cmd)
    {
        case VOIP_IOCTL_ENABLE:
        {
            DEBUG(" VOIP_IOCTL_ENABLE \r\n");
            stVoIP_path.audioID = AUDIO_ID_VoIP_TELEPHONY;//AUDIO_ID_VoIP_LOOPBACK;//AUDIO_ID_CALL; //change to AUDIO_ID_VoIP_LOOPBACK later;
            stVoIP_path.outputChnl = AUDIO_CHNL_SPEAKER;
            stVoIP_path.inputMIC = 0;
            stVoIP_path.volumeLevel = 30;

            memset(wbamr_downlinkdata, 0, sizeof(wbamr_downlinkdata)); 
            memset(wbamr_uplinkldata, 0, sizeof(wbamr_uplinkldata)); 

		
            wbamr_dl_index = 0;
            wbamr_downlink_index = WBAMR_DATA_SIZE/2; 
            wbamr_ul_index = WBAMR_DATA_SIZE/2;
            wbamr_uplink_index = 0;;
            voip_codec_type =0x1000;

#if 1 //loopback test from dsp team
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))
	{
		    Unpaged_SharedMem_t *unpage_sm = SHAREDMEM_GetUnpagedSharedMemPtr();
		    UInt16 *ptr, i;

		    ptr = (UInt16*)&(unpage_sm->VOPI_DL_buf.voip_frame.frame_pcm[0]);
		    for(i=0; i<160; i++) *ptr=0xffff;
	}
#endif
	
            for(i=0; i<WBAMR_DATA_SIZE; i++)
	        {
                wbamr_data[i]=0;		
	        }

	        if( (voip_codec_type&0xf000) == 0x2000 )		//FR
	        {
		        for(i=0; i<WBAMR_DATA_SIZE; i+=80)
		        {
			        wbamr_data[i]=1;		//BFI=1	
			        wbamr_data[i+1]=0;		//SID=0
			        wbamr_data[i+2]=0;		//taf=0
		        }
	        }
	        else if( (voip_codec_type&0xf000) == 0x3000 )	//AMR-NB
	        {
		        for(i=0; i<WBAMR_DATA_SIZE; i+=32)
		        {
			        wbamr_data[i]=7;		
			        wbamr_data[i+1]=7;		
		        }
	        }
#endif
            HAL_AUDIO_Ctrl( ACTION_AUD_EnablePath, &stVoIP_path, NULL); 
	        VoIP_StartTelephony(VoIPTelephony_Capture_Dump, VoIPTelephony_Render_Fill, voip_codec_type,0, 0);
				
            rtn = 1;
        }
        break;

        case VOIP_IOCTL_DISABLE:
        {
            HAL_AUDIO_Ctrl(ACTION_AUD_DisablePath, &stVoIP_path, NULL ); 
	         VoIP_StopTelephony(); 
            rtn=1;
        }
        break;

        default:
            DEBUG("Unrecognized voipioctl: '0x%x'  \n", cmd);
            return -ENOTTY;
    }

    return (rtn);

} /* voip_ioctl */

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

    const UInt16 volume_max = SYSPARM_GetAudioParmAccessPtr(AUDIO_GetAudioMode())->voice_volume_max;

    switch (cmd)
    {
    case PCG_IOCTL_SETMODE:
    { 
        UInt32 pcg_arg;
        AudioClientID_en_t audioID = AUDIO_ID_CALL8K;
        copy_from_user(&pcg_arg, (UInt32 *)arg, sizeof(UInt32));

        if ( AUDIO_MODE_NUMBER <= pcg_arg )
		 {
		     audioID = AUDIO_ID_CALL16K;
		 }

        //per pcg request, need to keep vol same among audio modes
        getParam.paramType = GET_AUDIO_VOLUMELEVEL;
        HAL_AUDIO_Ctrl_Blocking(ACTION_AUD_GetParam, &getParam, NULL);

        // printk(" BCMPCG_ioctl:  pcg_arg=%d  \r\n", pcg_arg);
        audioParam.param_pathCfg.audioID = audioID;
        audioParam.param_pathCfg.outputChnl =AUDIO_DRV_ConvertModeChnl(pcg_arg);

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

    case PCG_IOCTL_MAUDTST:
    {
	    int i;
	    arg_t voip;
        copy_from_user(&voip, arg, sizeof(voip));

        pr_info("AUD:PCG_IOCTL_MAUDTST VOIP val1 =%d val2=%d val3=%d val4=%d val5=%d val6=%d  \n",voip.val1,voip.val2,voip.val3,voip.val4,voip.val5,voip.val6);
	
	    switch (voip.val1) 
  	    {
           case 123:
    		       stVoIP_path.audioID = AUDIO_ID_VoIP_LOOPBACK;//AUDIO_ID_VoIP_LOOPBACK;//AUDIO_ID_CALL; //change to AUDIO_ID_VoIP_LOOPBACK later;
    			   stVoIP_path.outputChnl = voip.val3;
    			   stVoIP_path.inputMIC = 0;
    			   stVoIP_path.volumeLevel = voip.val4;

    			   memset(wbamr_data, 0, sizeof(wbamr_data)); 
		
    			   wbamr_dl_index = 0;
    			   wbamr_ul_index = WBAMR_DATA_SIZE/2;
    			   break;

           case 99:
			       HAL_AUDIO_Ctrl(ACTION_AUD_DisablePath, &stVoIP_path, NULL );
			       VoIP_StopTelephony(); 
    		       return 1; 
  		     default:
               return 0; 
  	    }

	    voip_codec_type = (UInt32)voip.val2;

#if 1 //loopback test from dsp team
#if ((defined(_BCM2153_) && CHIP_REVISION >= 52) || (defined(BCM2153_FAMILY_BB_CHIP_BOND)))
	{
		Unpaged_SharedMem_t *unpage_sm = SHAREDMEM_GetUnpagedSharedMemPtr();
		UInt16 *ptr, i;

		ptr = (UInt16*)&(unpage_sm->VOPI_DL_buf.voip_frame.frame_pcm[0]);
		for(i=0; i<160; i++) *ptr=0xffff;
	}
#endif
	
        for(i=0; i<WBAMR_DATA_SIZE; i++)
        {
           wbamr_data[i]=0;		
	    }				

	    if( (voip_codec_type&0xf000) == 0x2000 )		//FR
	    {
		    for(i=0; i<WBAMR_DATA_SIZE; i+=80)
		    {
			     wbamr_data[i]=1;		//BFI=1	
			     wbamr_data[i+1]=0;		//SID=0
			     wbamr_data[i+2]=0;		//taf=0
		    }					
	    }
	    else if( (voip_codec_type&0xf000) == 0x3000 )	//AMR-NB
	    {
		     for(i=0; i<WBAMR_DATA_SIZE; i+=32)
		     {
			      wbamr_data[i]=7;		
			      wbamr_data[i+1]=7;		
		     }				
	    }
#endif
	    HAL_AUDIO_Ctrl( ACTION_AUD_EnablePath, &stVoIP_path, NULL);
	    VoIP_StartTelephony(VoIPTelephony_Capture_Dump_Test, VoIPTelephony_Render_Fill_Test, voip_codec_type,0, 0);
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

#ifndef USE_VOLUME_CONTROL_IN_DB		
		spkvol = AUDIO_Util_Convert( spkvol, volume_max, AUDIO_VOLUME_MAX );
#endif		
        DEBUG("AUD:  PCG_IOCTL_SETVOL   spkvol=%d  volume_max=%d  \r\n", spkvol,volume_max );
        //  HAL_AUDIO_Ctrl( ACTION_AUD_SetVolumeWithPath, &audioParam, NULL);
        if ((rtn = HAL_AUDIO_Ctrl_Blocking(ACTION_AUD_SetSpeakerVol, &spkvol, NULL)) !=0)
            rtn = -1;
    }
    break;
	case PCG_IOCTL_SetExtPAVol:
	{
		UInt32 extvol;
		HAL_AUDIO_Param_Tune_t tuneCfg = {0};
		copy_from_user(&extvol, (UInt32 *)arg, sizeof(UInt32));
	
		DEBUG("AUD:  PCG_IOCTL_SetExtPAVol	extvol=%d  \r\n", extvol);

		tuneCfg.paramType = HAL_AUDIO_TUNE_PARAM_EXTAMP_PGA;
		tuneCfg.paramValue = extvol;
		if ((rtn = HAL_AUDIO_Ctrl_Blocking(ACTION_AUD_TUNE, &tuneCfg, NULL)) !=0)
			rtn = -1;
	}
	break;
    default:
        DEBUG("Unrecognized ioctl: '0x%x'  \n", cmd);
        return -ENOTTY;
    }

    return (rtn);

} /* lcd_ioctl */



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DeviceRelease
//
//  Description: 'release' call back function for platform device
//
//------------------------------------------------------------
static void DeviceRelease(struct device *pdev)
{
	DEBUG("\n TO DO:what am i supposed to do\n");	
}

//Platform device structure
static struct platform_device sgPlatformDevice =
{
	.name		= "brcm_alsa_device",
	.dev		= 
	{
		.platform_data	= &sgDeviceInfo,
		.release = DeviceRelease,
	},
	.id		= -1,
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

static struct file_operations bcmpcg_fops =
{
    .owner=  THIS_MODULE,
    .open =   BCMPCG_open,
    .ioctl  =   BCMPCG_ioctl,
    .mmap = NULL,
    .release = BCMPCG_release,
};


static struct file_operations bcmvoip_fops =
{
    .owner=  THIS_MODULE,
    .open =  BCMVOIP_open,
    .ioctl=  BCMVOIP_ioctl,
    .write=  BCMVOIP_write,
    .read =  BCMVOIP_read,
    .mmap =  NULL,
    .release = BCMVOIP_release,

};
static struct class *audvoip_class;

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
//--------------------------------------------------------------------


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ModuleInit
//
//  Description: Module initialization
//
//------------------------------------------------------------
static int __devinit ModuleInit(void)
{
	int err = 1;
	
	DEBUG("\n %lx:ModuleInit debg=%d id=%s\n",jiffies,debug,id);


	
	err =  platform_device_register(&sgPlatformDevice);
	DEBUG("\n %lx:device register done %d\n",jiffies,err);
	if (err)
		return err;
		
	err = platform_driver_register(&sgPlatformDriver);
	DEBUG("\n %lx:driver register done %d\n",jiffies,err);

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
	
	if ((err = register_chrdev(BCM_ALSA_VOIP_MAJOR, "bcm_alsa_voip", &bcmvoip_fops)) < 0)
    {
        return err;
    }

    audvoip_class = class_create(THIS_MODULE, "bcm_alsa_voip");
    if (IS_ERR(audvoip_class))
    {
        return PTR_ERR(audvoip_class);
    }
    device_create(audvoip_class, NULL, MKDEV(BCM_ALSA_VOIP_MAJOR, 0),NULL, "bcm_alsa_voip");

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
	
	return err;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ModuleExit
//
//  Description: Module de-initialization
//
//------------------------------------------------------------
static void __devexit ModuleExit(void)
{
	//int err;
	
	DEBUG("\n %lx:ModuleExit\n",jiffies);

	if(g_brcm_alsa_chip->pWorkqueue_PCM)
		destroy_workqueue(g_brcm_alsa_chip->pWorkqueue_PCM);
	snd_card_free(g_brcm_alsa_chip->card);
	
	platform_driver_unregister(&sgPlatformDriver);
	platform_device_unregister(&sgPlatformDevice);
	
	// unInitialize the AUDIO device

	audvoc_deinit();

	DEBUG("\n %lx:exit done \n",jiffies);
}

module_init(ModuleInit);
module_exit(ModuleExit);

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



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>

#include <linux/moduleparam.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>

#include <mach/irqs.h>
#include <asm/irq.h>
#include <linux/version.h>
#include <linux/broadcom/bcm_major.h>

#include <cfg_global.h>

#include "mobcom_types.h"
#include "brcm_alsa.h"
#include "cirbuf.h"
#include "audvoc_consts.h"
#include "shared.h"
#include "memmap.h"

#define	BRCMAACENC_DEV_NAME	"BRCMAACENC"

#define	AACEnc_MASK_CODEC_OPENED	0x00000001
#define	AACEnc_MASK_CODEC_INSTALLED	0x00000002
#define	AACEnc_MASK_CODEC_STARTED	0x00000004
#define	AACEnc_MASK_CODEC_DONE		0x00000008
#define	AACEnc_MASK_CODEC_CANCELED 0x00000010
#define	AACEnc_MASK_CODEC_OUTFULL	0x00000020
#define	AACEnc_MASK_CODEC_INEMPTY	0x00000040

#define	BrcmAACEncVersion_Major		1
#define	BrcmAACEncVersion_Minor		0
#define	BrcmAACEncVersion_SubMinor	0

#define	BrcmAACEncVersion	(((BrcmAACEncVersion_Major)<<16)|((BrcmAACEncVersion_Minor)<<8)|(BrcmAACEncVersion_SubMinor))

enum	AACEncIoctrlCode
{
	AACEncIoctrl_Start = 1,
	AACEncIoctrl_Stop,
	AACEncIoctrl_Pause,
	AACEncIoctrl_SetParms,
	AACEncIoctrl_GetParms,
	AACEncIoctrl_SetOutPutParm,
	AACEncIoctrl_GetOutPutParm,
	AACEncIoctrl_LoadCodecImage,
	AACEncIoctrl_GetStatus,
	AACEncIoctrl_GetVersion,
	AACEncIoctrl_SETEOS
		
};


typedef	struct 	_TBrcmAACEncParms
{
	int			source;		//0 ARM 1 MIC
	unsigned int	bitrate;
	unsigned int	samplerate;
	unsigned int	channels;
	unsigned int	enctype;
	unsigned int	inThreshold;
	unsigned int	outThreshold;
	unsigned int	intFrames;	// 1 ENCODE_DONE interrupt per 1 frame

}TBrcmAACEncParms, *PTBrcmAACEncParms;


typedef	struct _TDspCodecImage {
	unsigned int index;		/* W: DSP index */
	unsigned char name[64];		/* W: ID (e.g. file name) */
	unsigned char __user *image;	/* W: binary image */
	int length;			/* W: size of image in bytes */
	unsigned long driver_data;	/* W: driver-specific data */
}TDspCodecImage, *PTDspCodecImage;


typedef	struct _TBrcmAACEncStatus
{
	UInt32	u32Flags;
	int		iWriteSpace;
	int		iReadData;
	UInt32	u32Frames;

}TBrcmAACEncStatus, *PTBrcmAACEncStatus;




typedef	struct _TBrcmAACEnc
{
	TCirBuf	mCirbufIn;
	TCirBuf	mCirbufOut;
	volatile UInt16  * pDoneFlag;
	volatile UInt16  * pInSrc;	//input source: 0ARM 1: MIC
	volatile Shared_poly_audio_t  *pg4_sm;
	
	TBrcmAACEncParms	mParms;
	struct fasync_struct *pfasync;
	
	UInt32	u32Flags;
	UInt32	u32WrLen; //debug
	UInt32	u32TotalFrames;
	spinlock_t spinlock_aac;
}TBrcmAACEnc;

TBrcmAACEnc gAACEnc;
int AUDVOC_AACENC_downloadCodec (TBrcmAACEnc *pAacEnc, UInt8 *pImgBuf, int length);

//external 
void post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2); // audvoc_drv.c


/* module description */
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("BroadCom MobCOM Audio");
MODULE_DESCRIPTION("AAC DSP encoder");

#define DRV_NAME "BRCMAACENC_DRV"
#define DEV_NAME "BRCMAACENC_DEV"


/* and this is our MAJOR; use 0 for dynamic allocation (recommended)*/
#ifndef BCM_AACENC_MAJOR
#define BCM_AACENC_MAJOR 0
#endif

static int giBcmAACEncMajor = BCM_AACENC_MAJOR;
#define	PDEBUG   printk


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: StartAACEnc
//
//  Description: Post message to DSP to start AAC encoding
//
//------------------------------------------------------------
int	StartAACEnc(AACENCMODE_t	encmode)
{
	CmdQ_t msg;
	msg.cmd = COMMAND_AUDIO_TASK_START_REQUEST;
	msg.arg0 = encmode;
	msg.arg1 = 0;
	msg.arg2 = 0;

	post_msg( msg.cmd, msg.arg0, msg.arg1, msg.arg2); 

	return 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: DspCodecMsgHandler
//
//  Description: Handle status message from DSP
//
//------------------------------------------------------------

void	DspCodecMsgHandler(StatQ_t *pstatus_msg)
{
	TBrcmAACEnc	*pAacEnc = &gAACEnc;

//	if(STATUS_PRAM_CODEC_INPUT_EMPTY==pstatus_msg->status)
//	DEBUG("DspCodecMsgHandler: msg = 0x%x,arg0 = 0x%x,arg1 = 0x%x,arg2 = 0x%x\n", pstatus_msg->status, pstatus_msg->arg0, pstatus_msg->arg1, pstatus_msg->arg2);		
	if( !(pAacEnc->u32Flags & AACEnc_MASK_CODEC_OPENED) )
	{
		DEBUG("DspCodecMsgHandler: received msg before device open!\n");
		return;
	}

	switch(pstatus_msg->status)
	{
		case STATUS_TEST_PDMA:
			//indicate encoder image is ready
			spin_lock(&pAacEnc->spinlock_aac);
			if((pstatus_msg->arg0 == 0xaaaa) && (pstatus_msg->arg1 == 0xbbbb))
				pAacEnc->u32Flags |= AACEnc_MASK_CODEC_INSTALLED;
			else
				;//printk("status message above\n");//display status message
			spin_unlock(&pAacEnc->spinlock_aac);
			break;
		case STATUS_PRAM_CODEC_DONE_RECORD:
			spin_lock(&pAacEnc->spinlock_aac);
			pAacEnc->u32TotalFrames++;
			spin_unlock(&pAacEnc->spinlock_aac);
#if 0
			{
				int framelength;
				framelength = pstatus_msg->arg2 - pstatus_msg->arg1;
				if(framelength<0)
					framelength += 12*1024;
				if(framelength<10)
					DEBUG("ERROR DspCodecMsgHandler: msg = 0x%x,arg0 = 0x%x,arg1 = 0x%x,arg2 = 0x%x\n", pstatus_msg->status, pstatus_msg->arg0, pstatus_msg->arg1, pstatus_msg->arg2);		
			}
#endif
			break;			
		case STATUS_PRAM_CODEC_INPUT_LOW:
			break;
		case STATUS_PRAM_CODEC_INPUT_EMPTY:
			//signal for requesting more PCM data
			spin_lock(&pAacEnc->spinlock_aac);
			pAacEnc->u32Flags |= AACEnc_MASK_CODEC_INEMPTY;
			spin_unlock(&pAacEnc->spinlock_aac);
			kill_fasync(&pAacEnc->pfasync, SIGIO, POLL_IN);
			break;
			
		case STATUS_PRAM_CODEC_OUTPUT_FULL:
			spin_lock(&pAacEnc->spinlock_aac);
			pAacEnc->u32Flags |= AACEnc_MASK_CODEC_OUTFULL;
			spin_unlock(&pAacEnc->spinlock_aac);
			//signal for encoded data ready
			kill_fasync(&pAacEnc->pfasync, SIGIO, POLL_IN);

			break;
		case STATUS_PRAM_CODEC_OUTPUT_LOW:
			break;

		case STATUS_PRAM_CODEC_DONEPLAY:
			//finish last frame
			pr_info("\nDspCodecMsgHandler:EOS\n");	
			spin_lock(&pAacEnc->spinlock_aac);
			pAacEnc->u32Flags |= AACEnc_MASK_CODEC_DONE;
			spin_unlock(&pAacEnc->spinlock_aac);
			kill_fasync(&pAacEnc->pfasync, SIGIO, POLL_IN);
			
			break;
		case STATUS_PRAM_CODEC_CANCELPLAY:
			//cancel processing
			spin_lock(&pAacEnc->spinlock_aac);
			pAacEnc->u32Flags |= AACEnc_MASK_CODEC_CANCELED;
			spin_unlock(&pAacEnc->spinlock_aac);
			break;
		default:
			pr_info("\nDspCodecMsgHandler:Unexpected message\n");		
			break;
	}


}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: brcm_aacenc_read
//
//  Description: Get encoded data from DSP
//
//------------------------------------------------------------
static ssize_t brcm_aacenc_read(struct file * file, char __user *_buf,
			      size_t count, loff_t *offset)
{

	UInt32	size;
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;
	
//	DEBUG("brcm_aacenc_read: user buf=0x%x count =%d\n", (unsigned int)_buf, (int)count);
	spin_lock(&pAacEnc->spinlock_aac);
	size = CBufRead(&pAacEnc->mCirbufIn, _buf, count);
	//wake up
	StartAACEnc(AACENC_MODE_ENCODE);
	pAacEnc->u32Flags &= ~AACEnc_MASK_CODEC_OUTFULL;
	spin_unlock(&pAacEnc->spinlock_aac);
	return size;	
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: brcm_aacenc_write
//
//  Description: Feed PCM data to DSP
//
//------------------------------------------------------------
static ssize_t brcm_aacenc_write(struct file * file, const char __user *_buf,
			       size_t count, loff_t *offset)
{
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;
	UInt32	size;
	
//	DEBUG("brcm_aacenc_write: user buf=0x%x count =%d\n", (unsigned int)_buf, (int)count);
	spin_lock(&pAacEnc->spinlock_aac);
	size = CBufWrite(&pAacEnc->mCirbufOut, _buf, count);
	pAacEnc->u32WrLen += count;
	if( !(pAacEnc->u32Flags & AACEnc_MASK_CODEC_STARTED) )
	{
		//start encoding
		StartAACEnc(AACENC_MODE_INIT);
		pAacEnc->u32Flags |= AACEnc_MASK_CODEC_STARTED;
	}
	else 
		StartAACEnc(AACENC_MODE_ENCODE);		//wake up
	pAacEnc->u32Flags &= ~AACEnc_MASK_CODEC_INEMPTY;
	if(size!=count)
		DEBUG("brcm_aacenc_write: expected count =%d written=%d\n", (int)count, size);
	spin_unlock(&pAacEnc->spinlock_aac);
	return size;	
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ConfigOutChannel
//
//  Description: Configure PCM data channel
//
//      PCM channel
//      For ARM, it is output, for DSP it is Input channel
//------------------------------------------------------------
static int	ConfigOutChannel(TBrcmAACEnc	*pAacEnc)
{
	SharedMem_t *pg0_sm = SHAREDMEM_GetSharedMemPtr();
	PAGE18_SharedMem_t *pg18_sm = SHAREDMEM_GetPage18SharedMemPtr(); // encoder input buffer is page 18,19,20
	Unpaged_SharedMem_t *unpage_sm = SHAREDMEM_GetUnpagedSharedMemPtr();

	CBufInit(&pAacEnc->mCirbufOut, 
		    (UInt16 *)&pg18_sm->shared_PRAM_InputBuffer3[0], //buffer addr
		    3*AUDIO_SIZE_PER_PAGE,					//size
		    & pg0_sm->shared_NEWAUD_InBuf_in[0],		//write index
		    & pg0_sm->shared_NEWAUD_InBuf_out[0]);		//read index
	pAacEnc->pDoneFlag = & unpage_sm->shared_encodedSamples_done_flag[0];
	*pAacEnc->pDoneFlag = 0;
	pAacEnc->pInSrc	= &pg0_sm->shared_AACEnc_PCM_SOURCE;
	CBufReset(&pAacEnc->mCirbufOut);
	pAacEnc->pg4_sm = SHAREDMEM_GetPage4SharedMemPtr();
	
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: ConfigOutChannel
//
//  Description: Configure encoded data channel
//
//      PCM channel
//      For ARM, it is input, for DSP it is output channel
//------------------------------------------------------------
static int	ConfigInChannel(TBrcmAACEnc	*pAacEnc)
{
	SharedMem_t *pg0_sm = SHAREDMEM_GetSharedMemPtr();
	Shared_poly_audio_t *pg4_sm = SHAREDMEM_GetPage4SharedMemPtr();
	PAGE15_SharedMem_t *pg15_sm = SHAREDMEM_GetPage15SharedMemPtr(); // encoder output buffer is page 15,16,17
	Unpaged_SharedMem_t *unpage_sm = SHAREDMEM_GetUnpagedSharedMemPtr();


	CBufInit(&pAacEnc->mCirbufIn, 
		    (UInt16 *)&pg15_sm->shared_PRAM_InputBuffer0[0], //buffer addr
		    3*AUDIO_SIZE_PER_PAGE,					//size
		    & unpage_sm->shared_encodedSamples_buffer_in[0],		//write index
		    & unpage_sm->shared_encodedSamples_buffer_out[0]);		//read index
	CBufReset(&pAacEnc->mCirbufIn);


	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: brcm_aacenc_open
//
//  Description: Open the device
//
//------------------------------------------------------------
static int brcm_aacenc_open(struct inode *inode, struct file * file)
{
	TBrcmAACEnc	*pAacEnc = &gAACEnc;

	memset(pAacEnc, 0, sizeof(TBrcmAACEnc));
	pAacEnc->u32Flags |= AACEnc_MASK_CODEC_OPENED|AACEnc_MASK_CODEC_INEMPTY;

	file->private_data = pAacEnc;
		
	DEBUG("brcm_aacenc_open pAacEnc %x \n", (unsigned int)file->private_data);

	//Config Input/Output channel
	ConfigOutChannel(pAacEnc);
	ConfigInChannel(pAacEnc);
	pAacEnc->spinlock_aac = SPIN_LOCK_UNLOCKED;

	return 0;	
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: brcm_aacenc_release
//
//  Description: Close the device
//
//------------------------------------------------------------
static int brcm_aacenc_release(struct inode *inode, struct file * file)
{
	int err = -ENXIO;

	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;

	DEBUG("brcm_aacenc_release total write length=0x%x, totalframes=%d\n", pAacEnc->u32WrLen, pAacEnc->u32TotalFrames);
	memset(pAacEnc, 0, sizeof(TBrcmAACEnc));
	
	return err;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: GetParms
//
//  Description: Get parameters
//
//------------------------------------------------------------
static	int	GetParms(struct file * file, TBrcmAACEncParms __user * _pa)
{
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;
	
	if (0 != copy_to_user(_pa, &pAacEnc->mParms, sizeof(TBrcmAACEncParms)))
		return -EFAULT;
	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: SetParms
//
//  Description: Set parameters
//
//------------------------------------------------------------
static	int	SetParms(struct file * file, TBrcmAACEncParms __user *_pa)
{
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;
	TBrcmAACEncParms	*pParms = &pAacEnc->mParms;
	int err;
	//Get data from user space
	err = copy_from_user(pParms, _pa,  sizeof(TBrcmAACEncParms));
	
	if (!err)
	{
		//set to hardware
		// 1: configure sampling rate, bitrate, number of channels, and encoding type
		*pAacEnc->pInSrc = pParms->source;
		pAacEnc->pg4_sm->shared_AACEnc_SamplingFrequency = pParms->samplerate;
		pAacEnc->pg4_sm->shared_AACEnc_BitRate = pParms->bitrate;
		pAacEnc->pg4_sm->shared_AACEnc_NoOfChannels = pParms->channels;
		pAacEnc->pg4_sm->shared_AACEnc_EncodingType = pParms->enctype;	//	VBR = 0, ABR = 1, FIXED_R = 2
		pAacEnc->pg4_sm->shared_Inbuf_LOW_Sts_TH = pParms->inThreshold;	
		pAacEnc->pg4_sm->shared_Outbuf_LOW_Sts_TH = pParms->outThreshold;
		pAacEnc->pg4_sm->shared_Outbuf_Freq_Sts_TH = pParms->intFrames; //should be at least 1, correct me, Frank

		PDEBUG("insrc=%d, samplerate =%d, bitrate=%d, channels=%d enctype=%d,  inThreshold=%d, outThreshold=%d, intFrames=%d\n", 
				*pAacEnc->pInSrc,
				pAacEnc->pg4_sm->shared_AACEnc_SamplingFrequency,
				pAacEnc->pg4_sm->shared_AACEnc_BitRate,
				pAacEnc->pg4_sm->shared_AACEnc_NoOfChannels,
				pAacEnc->pg4_sm->shared_AACEnc_EncodingType,
				pAacEnc->pg4_sm->shared_Inbuf_LOW_Sts_TH,
				pAacEnc->pg4_sm->shared_Outbuf_LOW_Sts_TH,
				pAacEnc->pg4_sm->shared_Outbuf_Freq_Sts_TH
				);

		

	}
	return err;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: LoadCodecImage
//
//  Description: Load DSP encoder image
//
//------------------------------------------------------------
static	int	LoadCodecImage(struct file * file, TDspCodecImage __user *_pa)
{
	TDspCodecImage	img;
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;
	UInt8 *codec_image;	// defined in audvoc_codec.c. Come back to see what is it later....

	
	if ( copy_from_user(&img, _pa,  sizeof(TDspCodecImage)) )
	{
		PDEBUG("Can not copy_from_user TDspCodecImage %d\n", img.length);
		return -EFAULT;
	}
	//set to hardware
	codec_image = (UInt8 *)kmalloc(img.length, GFP_KERNEL| GFP_DMA);
	if(codec_image==NULL)
	{
		PDEBUG("Can not allocate memory for CODEC image %d\n", img.length);
		return -1;
	}
	if ( copy_from_user(codec_image, img.image,  img.length) )
	{
		PDEBUG("Can not copy_from_user codec image %d\n", img.length);
		return -EFAULT;
	}
	
	AUDVOC_AACENC_downloadCodec(pAacEnc, codec_image, img.length);

	{
		int try =5;
		while(try--)
		{
			if(pAacEnc->u32Flags & AACEnc_MASK_CODEC_INSTALLED)
			{
				PDEBUG("Wait for CODEC downloading ready try=%d\n", try);
				kfree(codec_image);
				return 0;
			}
			else
				msleep(20);
		}
		PDEBUG("Wait for CODEC downloading timeout try=%d\n", try);
	}

	kfree(codec_image);
	
	return -2;
}

//#define	AAC_IN_BUF_GUARD		(4*1024)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: GetEncStatus
//
//  Description: Get encoder status
//
//------------------------------------------------------------
static int GetEncStatus(struct file * file, TBrcmAACEncStatus __user *_pa)
{
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;
	TBrcmAACEncStatus	encstatus;

	spin_lock(&pAacEnc->spinlock_aac);
	encstatus.u32Flags = pAacEnc->u32Flags;
	encstatus.u32Frames = pAacEnc->u32TotalFrames;
//	PDEBUG("GetEncStatus 1\n");
/*
	{
		TCirBuf	*pC;
		pC = &(pAacEnc->mCirbufIn);
		PDEBUG("GetEncStatus In: buf 0x%x size %d RI 0x%x WI 0x%x mask 0x%x *WI %x *RI %x\n", 
			pC->pBuf, pC->buffer_size, pC->pRdIndex, pC->pWrIndex, pC->size_mask, *pC->pWrIndex, *pC->pRdIndex);
		pC = &(pAacEnc->mCirbufOut);
		PDEBUG("GetEncStatus In: buf 0x%x size %d RI 0x%x WI 0x%x mask 0x%x *WI %x *RI %x\n", 
			pC->pBuf, pC->buffer_size, pC->pRdIndex, pC->pWrIndex, pC->size_mask, *pC->pWrIndex, *pC->pRdIndex);
	}
*/	
	encstatus.iReadData = 2 * CBufReadAvaiable(&pAacEnc->mCirbufIn);

//	PDEBUG("GetEncStatus 2\n");
	encstatus.iWriteSpace = 2 * CBufWriteAvaiable(&pAacEnc->mCirbufOut);
//	encstatus.iWriteSpace -= AAC_IN_BUF_GUARD;
	if(encstatus.iWriteSpace<0)
		encstatus.iWriteSpace = 0;
/*
        if(!(encstatus.u32Flags & AACEnc_MASK_CODEC_STARTED))
        {
                if( encstatus.iWriteSpace>(2*AUDIO_SIZE_PER_PAGE) )
			encstatus.iWriteSpace = (2*AUDIO_SIZE_PER_PAGE);
        }
*/

//	PDEBUG("GetEncStatus 3\n");
	spin_unlock(&pAacEnc->spinlock_aac);
	if (0 != copy_to_user(_pa, &encstatus, sizeof(TBrcmAACEncStatus)))
		return -EFAULT;
//	PDEBUG("GetEncStatus 4\n");
	return 0;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Function Name: brcm_aacenc_ioctl
//
//  Description: Handle I/O control code
//
//------------------------------------------------------------
static long brcm_aacenc_ioctl(struct file * file, unsigned int cmd,
			    unsigned long arg)
{
//	long	err=0;
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;

	void __user *argp = (void __user *)arg;
//	PDEBUG("brcm_aacenc_ioctl cmd=%d\n", cmd);
	switch (cmd) {
		case AACEncIoctrl_GetVersion:
			return put_user(BrcmAACEncVersion, (int __user *)argp);

			break;

		case AACEncIoctrl_Start:
			break;
		case AACEncIoctrl_Stop:
			break;
			
		case AACEncIoctrl_Pause:
			break;
		case AACEncIoctrl_SetParms:
			SetParms(file, argp);
			break;
		case AACEncIoctrl_GetParms:
			GetParms(file, argp);			
			break;

	
		case AACEncIoctrl_SetOutPutParm:
			break;

		case AACEncIoctrl_GetOutPutParm:
			break;
		case AACEncIoctrl_GetStatus:
			GetEncStatus(file, argp);
			break;
			
		case AACEncIoctrl_LoadCodecImage:
			LoadCodecImage(file, argp);
			break;
		case AACEncIoctrl_SETEOS:
			*pAacEnc->pDoneFlag = 1;
			break;
		default:
			return -ENOTTY;

	}
	return 0;
}

static int brcm_aacenc_mmap(struct file * file, struct vm_area_struct * vma)
{
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;
	return -ENXIO;
}


/*------------------------------------------------------------------------------
    Function name   : brcm_aacenc_fasync
    Description     : Method for signing up for a interrupt

    Return type     : int
------------------------------------------------------------------------------*/

static int brcm_aacenc_fasync(int fd, struct file *file, int mode)
{
	TBrcmAACEnc	*pAacEnc = (TBrcmAACEnc	*)file->private_data;

        PDEBUG("brcm_aacenc_fasync called %d %x %d %x\n",
               fd, (u32) file, mode, (u32) pAacEnc->pfasync);
        return fasync_helper(fd, file, mode, &pAacEnc->pfasync);
}

static const struct file_operations brcm_aacenc_f_ops =
{
	.owner = 	THIS_MODULE,
	.read = 	 brcm_aacenc_read,
	.write =	 brcm_aacenc_write,
	.open =		 brcm_aacenc_open,
	.release =	 brcm_aacenc_release,
	.unlocked_ioctl =	 brcm_aacenc_ioctl,
	.mmap =		 brcm_aacenc_mmap,
	.fasync = brcm_aacenc_fasync,
};



/*------------------------------------------------------------------------------
    Function name   : BrcmAACEncModuleInit
    Description     : Initialize the driver

    Return type     : int
------------------------------------------------------------------------------*/

int __init BrcmAACEncModuleInit(void)
//int  BrcmAACEncModuleInit(void)
{
    int result = 0;

    /* if you want to test the module, you obviously need to "mknod". */
    PDEBUG("BrcmAACEncModuleInit\n");



    result = register_chrdev(giBcmAACEncMajor, BRCMAACENC_DEV_NAME, &brcm_aacenc_f_ops);
    if(result < 0)
    {
        pr_info(BRCMAACENC_DEV_NAME": unable to get major %d\n", giBcmAACEncMajor);
        return result;
    }
    else if(result != 0)    /* this is for dynamic major */
    {
        giBcmAACEncMajor = result;
    }



    pr_info(BRCMAACENC_DEV_NAME": module inserted. Major = %d\n", giBcmAACEncMajor);

    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : BrcmAACEncModuleExit
    Description     : clean up

    Return type     : int
------------------------------------------------------------------------------*/

void __exit BrcmAACEncModuleExit(void)
//void  BrcmAACEncModuleExit(void)
{

    /* free the encoder IRQ */
    //free_irq(dev->irq, (void *) dev);
    unregister_chrdev(giBcmAACEncMajor, BRCMAACENC_DEV_NAME);

    pr_info(BRCMAACENC_DEV_NAME": module removed\n");
    return;
}


#define	SOFT_COPY
// AAC Encoder source stored in ARM
UInt8 aacenc_codec_src[] = 
{
	0,0,0,0
//	#include "aacenc_image.txt"
};

UInt32 aacenc_image_size = sizeof(aacenc_codec_src);


// download the encoder to DSP. 
int AUDVOC_AACENC_downloadCodec (TBrcmAACEnc *pAacEnc, UInt8 *pImgBuf, int length)
{
	// DSP command message to start download
	CmdQ_t msg;

	UInt16 addrH, addrL, len;	// information sent to DSP command message
	UInt32 aacenc_addr;

	
	// Copied from dsp_test.c. but WHY???
	// The first part of codec image is the program to download the real codec image. 
	// So we download it to DSP first, and then DSP will run this program and finish the download.
	//
	aacenc_addr = virt_to_phys(pImgBuf); 
	addrH = (aacenc_addr >> 17) & 0xFFFF;
	addrL = (aacenc_addr & 0x1FFFF) >> 1;
#ifdef	SOFT_COPY
	len = 0x8000 | 0x400;
#else
	len = 0x400; // this is the length for the program of downloading the real codec image...
#endif

	//	Load AAC Encoder to DSP
	post_msg(COMMAND_START_PRAM_FUNCT, addrH, addrL, len);
	
	return 0;
}


module_init(BrcmAACEncModuleInit);
module_exit(BrcmAACEncModuleExit);


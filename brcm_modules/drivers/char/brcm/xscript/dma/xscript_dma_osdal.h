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

//***************************************************************************
/**
*
*  @file   xscript_dma_osdal.h
*
*  @brief  Interface to the xscript DMA kernel module.
*
*   @note	This driver is typically used for handling xScript DMA kernel module  APIs.
*
****************************************************************************/
#define TEST_DEVICE_NAME        "dma-test"
#define MAG_NUM                 'd'
#define DMAMAJOR     			0 

#define DMA_NOT_DONE         0
#define DMA_DONE_SUCCESS     1
#define DMA_DONE_FAILURE     2
#define SOURCE_WIDTH    	 4


/* put ioctl flags here, use the _IO macro which is 
 found in linux/ioctl.h, takes a letter, and an 
 integer */
#define DMA_MEM_TO_MEM  _IO(MAG_NUM,1)
#define DMA_MEM_TO_BUFLIST  _IO(MAG_NUM,2)
#define DMA_MEM_TO_MULTILLI  _IO(MAG_NUM,3)
#define DMA_MEM_TO_CIRCULAR  _IO(MAG_NUM,4)

/* interface for passing structures between user 
 space and kernel space easily */

struct tmod_interface {
		int     in_len;         // input data length
    	int    	*in_data;        // input data
        int     out_rc;         // return code from the test
        int     in_len_0;        // input data0 length
        int     *in_data_0;       // input data0
};
typedef struct tmod_interface tmod_interface_t;

/* DMA buffer */
typedef struct
{
    unsigned int       *virtPtr;
    dma_addr_t  physPtr;
} DMA_Buffer_t;


 



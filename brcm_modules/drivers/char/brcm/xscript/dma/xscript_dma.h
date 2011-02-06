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
*  @file   xscript_dma.h
*
*  @brief  Interface to the xscript DMA kernel module.
*
*   @note	This driver is typically used for handling xScript DMA kernel module  APIs.
*
****************************************************************************/
#define TEST_DEVICE_NAME             "dma-test"
#define MAG_NUM                 'd'
#define DMAMAJOR     0 // 241

/* put ioctl flags here, use the _IO macro which is 
 found in linux/ioctl.h, takes a letter, and an 
 integer */
#define DMA_MEM_TO_MEM_BURST  _IO(MAG_NUM,1)
#define DMA_MEM_TO_MEM_QUERY  _IO(MAG_NUM,2)
#define DMA_REQUEST_MAX_CHANNELS _IO(MAG_NUM,3)
/* interface for passing structures between user 
 space and kernel space easily */

struct tmod_interface {
		int     in_len;         // input data length
    	int    	*in_data;        // input data
        int     out_rc;         // return code from the test
        int     in_len_0;        // input data0 length
        int     *in_data_0;       // input data0
        int     in_len_1;        // input data1 length
        int     *in_data_1;       // input data1
};
typedef struct tmod_interface tmod_interface_t;

/* definition of DMA Linked List Item (LLI) */
typedef struct DMA_LLI
{
    unsigned int     source;     // source address
    unsigned int     dest;       // dest address
    unsigned int     link;       // link to next LLI
    unsigned int     control;    // control word
} DMA_LLI_t;

/* DMA buffer */
typedef struct
{
    size_t      sizeInBytes;
    unsigned int       *virtPtr;
    dma_addr_t  physPtr;
} DMA_Buffer_t;

 



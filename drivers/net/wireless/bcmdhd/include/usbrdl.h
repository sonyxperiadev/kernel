/*
 * Broadcom USB remote download definitions
 *
 * Copyright (C) 2014, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: usbrdl.h 349211 2012-08-07 09:45:24Z $
 */

#ifndef _USB_RDL_H
#define _USB_RDL_H


#define DL_GETSTATE		0	
#define DL_CHECK_CRC		1	
#define DL_GO			2	
#define DL_START		3	
#define DL_REBOOT		4	
#define DL_GETVER		5	
#define DL_GO_PROTECTED		6	
#define DL_EXEC			7	
#define DL_RESETCFG		8	
#define DL_DEFER_RESP_OK	9	

#define	DL_HWCMD_MASK		0xfc	
#define	DL_RDHW			0x10	
#define	DL_RDHW32		0x10	
#define	DL_RDHW16		0x11	
#define	DL_RDHW8		0x12	
#define	DL_WRHW			0x14	
#define DL_WRHW_BLK 	0x13	

#define DL_CMD_RDHW		1	
#define DL_CMD_WRHW		2	



#define DL_WAITING	0	
#define DL_READY	1	
#define DL_BAD_HDR	2	
#define DL_BAD_CRC	3	
#define DL_RUNNABLE	4	
#define DL_START_FAIL	5	
#define DL_NVRAM_TOOBIG	6	
#define DL_IMAGE_TOOBIG	7	

#define TIMEOUT		5000	

struct bcm_device_id {
	char	*name;
	uint32	vend;
	uint32	prod;
};

typedef struct {
	uint32	state;
	uint32	bytes;
} rdl_state_t;

typedef struct {
	uint32	chip;		
	uint32	chiprev;	
	uint32  ramsize;    
	uint32  remapbase;   
	uint32  boardtype;   
	uint32  boardrev;    
} bootrom_id_t;


typedef struct {
	uint32	cmd;		
	uint32	addr;		
	uint32	len;		
	uint32	data;		
} hwacc_t;


typedef struct {
	uint32  cmd;            
	uint32  addr;           
	uint32  len;            
	uint8   data[1];                
} hwacc_blk_t;



#define QUERY_STRING_MAX 32
typedef struct {
	uint32  cmd;                    
	char    var[QUERY_STRING_MAX];  
} nvparam_t;

typedef void (*exec_fn_t)(void *sih);

#define USB_CTRL_IN (USB_TYPE_VENDOR | 0x80 | USB_RECIP_INTERFACE)
#define USB_CTRL_OUT (USB_TYPE_VENDOR | 0 | USB_RECIP_INTERFACE)

#define USB_CTRL_EP_TIMEOUT 500 

#define RDL_CHUNK	1500  


#define TRX_OFFSETS_DLFWLEN_IDX	0	
#define TRX_OFFSETS_JUMPTO_IDX	1	
#define TRX_OFFSETS_NVM_LEN_IDX	2	
#ifdef BCMTRXV2

#define TRX_OFFSETS_DSG_LEN_IDX	3	
#define TRX_OFFSETS_CFG_LEN_IDX	4	
#endif 

#define TRX_OFFSETS_DLBASE_IDX  0       

#endif  

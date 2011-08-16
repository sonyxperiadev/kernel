/*****************************************************************************
* Copyright 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/****************************************************************************/
/**
*  @file    chal_mqos.c
*
*  @brief   Supports EMI QoS register configuration and access
*
*  @note
*
*/
/****************************************************************************/

/*
 * ===========================================================================
 *  include header file declarations
 */
#include <plat/chal/chal_common.h>
//#include <plat/csp/chipregHw_inline.h>
#include <chal/chal_memc.h>
#include <chal/chal_memc_ddr3.h>
#include <chal/chal_mqos.h>
//#include <csp/delay.h>
//#include <linux/string.h>

#include <mach/io_map.h>

/*
#define CHAL_MQOS_DEBUG_ENABLE
	
*/

#ifdef CHAL_MQOS_DEBUG_ENABLE
#define CHAL_MQOS_DEBUG_PRINT(msg, ...) printf(char * msg, ...)
#else
#define CHAL_MQOS_DEBUG_PRINT(msg, ...)
#endif

/****************************************************************************/
/**
*  @brief  chal_mqos_get_handle
*
*  @return
*/
/****************************************************************************/
void chal_mqos_get_handle(CHAL_MQOS_t * mqos_data)
{
#ifdef CHAL_MQOS_DEBUG_ENABLE
	
#endif
	switch(mqos_data->memc_type)
	{
		case CHAL_MQOS_MEMTYPE_VC4_EMI_SECURE:
		case CHAL_MQOS_MEMTYPE_VC4_EMI_OPEN:
			mqos_data->entry_block  = (CHAL_MQOS_CAM_ENTRY_t *) (KONA_MEMC1_NS_VA + CSR_CAM_UPDATE_OFFSET);
			mqos_data->enable_block = (CHAL_MQOS_CAM_ENABLE_t *) (KONA_MEMC1_NS_VA + CSR_CAM_ENABLE_0_OFFSET);
			break;
		case CHAL_MQOS_MEMTYPE_SYS_EMI_SECURE:
		case CHAL_MQOS_MEMTYPE_SYS_EMI_OPEN:
		default:
			mqos_data->entry_block  = (CHAL_MQOS_CAM_ENTRY_t *) (KONA_MEMC0_NS_VA + CSR_CAM_UPDATE_OFFSET);
			mqos_data->enable_block = (CHAL_MQOS_CAM_ENABLE_t *) (KONA_MEMC0_NS_VA + CSR_CAM_ENABLE_0_OFFSET);
	}

	return;
}


/****************************************************************************/
/**
*  @brief  chal_mqos_update_enable
*		
*  @return
*/
/****************************************************************************/
void chal_mqos_update_enable(CHAL_MQOS_t * mqos_data)
{
/* enable update.  When activated, the CAM entries are being updated 
	and the Demesh will not accept new commands from the AXI ports.
	Note: Firmware should wait ~100ns after setting this signal before updating the CAM */
	mqos_data->entry_block->CAM_UPDATE = 1;

	return;
}

/****************************************************************************/
/**
*  @brief  chal_mqos_update_disable
*		
*  @return
*/
/****************************************************************************/
void chal_mqos_update_disable(CHAL_MQOS_t * mqos_data)
{
/* enable update.  When activated, the CAM entries are being updated 
	and the Demesh will not accept new commands from the AXI ports.
	Note: Firmware should wait ~100ns after setting this signal before updating the CAM */
	mqos_data->entry_block->CAM_UPDATE = 0;

	return;
}

/****************************************************************************/
/**
*  @brief  chal_mqos_entry_config
*		entry_cfg_str should be constructed with the macro CHAL_MQOS_CAM_ENTRY_CFG_STR()
*		enable_cfg_str should be constructed with the macro CHAL_MQOS_CAM_TAG_CFG_STR()
*  @return
*/
/****************************************************************************/
void chal_mqos_entry_config(
		CHAL_MQOS_t * mqos_data,
		unsigned int entry_cfg_str,
		unsigned int enable_cfg_str,
		CHAL_MQOS_CAM_ENTRY_ID_e id)
{
	unsigned int * entry; 
	unsigned int * enable; 
	CHAL_MQOS_CAM_ENTRY_t	* entry_block;
	CHAL_MQOS_CAM_ENABLE_t	* enable_block;

	entry_block = mqos_data->entry_block;
	enable_block = mqos_data->enable_block;
	entry  = (unsigned int * ) &entry_block->CAM_ENTRY_0;
	enable = (unsigned int * ) &enable_block->CAM_ENABLE_0;

#ifdef CHAL_MQOS_DEBUG_ENABLE
	if(id > CHAL_MQOS_CAM_ENTRY_ID_MAX){
		CHAL_MQOS_DEBUG_PRINT("CAM ENTRY id exceeds the the max.\n" );
	return;
	}
#endif

	entry[id] = entry_cfg_str;
	enable[id] = enable_cfg_str;
#if 0
	/* enable update.  When activated, the CAM entries are being updated 
	and the Demesh will not accept new commands from the AXI ports.
	Note: Firmware should wait ~100ns after setting this signal before updating the CAM */
	mqos_data->entry_block->CAM_UPDATE = 1;
#endif
	return;
}


/****************************************************************************/
/**
*  @brief  chal_mqos_block_config
*
*  @return
*/
/****************************************************************************/
void chal_mqos_enable(
		CHAL_MQOS_t * mqos_data,
		unsigned int enable_cfg_str,
		CHAL_MQOS_CAM_ENTRY_ID_e id)
{

	unsigned int * enable =(unsigned int * ) &mqos_data->enable_block->CAM_ENABLE_0;

#ifdef CHAL_MQOS_DEBUG_ENABLE
	if(id > CHAL_MQOS_CAM_ENTRY_ID_MAX){
		CHAL_MQOS_DEBUG_PRINT("CAM ENTRY id exceeds the the max.\n" );
	return;
	}else {
		CHAL_MQOS_DEBUG_PRINT("CAM ENTRY %: enable 0x%X, tag 0x%X. \n", 
				id,
				(enable_cfg_str & CSR_CAM_ENABLE_0_CAM_TAG_ENABLE_0_MASK) >> CSR_CAM_ENABLE_0_CAM_TAG_ENABLE_0_SHIFT,
				(CSR_CAM_ENABLE_0_CAM_TAG_0_MASK & enable_cfg_str));
	}
#endif

	enable[id] = enable_cfg_str;
	
	return;
}

/****************************************************************************/
/**
*  @brief  chal_mqos_stop
*
*  @return
*/
/****************************************************************************/
void chal_mqos_stop(CHAL_MQOS_t * mqos_data)
{
	(void) mqos_data;  /* temporary to avoid compilation warning*/
}

/****************************************************************************/
/**
*  @brief  chal_mqos_print_raw
*
*  @return
*/
/****************************************************************************/
void chal_mqos_print_raw(
			CHAL_MQOS_t * mqos_data,
			int (*fpPrint) (const char *, ...))
{
		fpPrint( "CAM_ENTRY_0 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_0, mqos_data->entry_block->CAM_ENTRY_0);
		fpPrint( "CAM_ENTRY_1 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_1, mqos_data->entry_block->CAM_ENTRY_1);
		fpPrint( "CAM_ENTRY_2 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_2, mqos_data->entry_block->CAM_ENTRY_2);
		fpPrint( "CAM_ENTRY_3 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_3, mqos_data->entry_block->CAM_ENTRY_3);
		fpPrint( "CAM_ENTRY_4 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_4, mqos_data->entry_block->CAM_ENTRY_4);
		fpPrint( "CAM_ENTRY_5 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_5, mqos_data->entry_block->CAM_ENTRY_5);
		fpPrint( "CAM_ENTRY_6 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_6, mqos_data->entry_block->CAM_ENTRY_6);
		fpPrint( "CAM_ENTRY_7 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_7, mqos_data->entry_block->CAM_ENTRY_7);
		fpPrint( "CAM_ENTRY_8 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_8, mqos_data->entry_block->CAM_ENTRY_8);
		fpPrint( "CAM_ENTRY_9 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_9, mqos_data->entry_block->CAM_ENTRY_9);
		fpPrint( "CAM_ENTRY_10 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_10, mqos_data->entry_block->CAM_ENTRY_10);
		fpPrint( "CAM_ENTRY_11 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_11, mqos_data->entry_block->CAM_ENTRY_11);
		fpPrint( "CAM_ENTRY_12 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_12, mqos_data->entry_block->CAM_ENTRY_12);
		fpPrint( "CAM_ENTRY_13 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_13, mqos_data->entry_block->CAM_ENTRY_13);
		fpPrint( "CAM_ENTRY_14 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_14, mqos_data->entry_block->CAM_ENTRY_14);
		fpPrint( "CAM_ENTRY_15 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->entry_block->CAM_ENTRY_15, mqos_data->entry_block->CAM_ENTRY_15);

		fpPrint( "CAM_ENABLE_0 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_0 , mqos_data->enable_block->CAM_ENABLE_0  );
		fpPrint( "CAM_ENABLE_1 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_1 , mqos_data->enable_block->CAM_ENABLE_1  );
		fpPrint( "CAM_ENABLE_2 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_2 , mqos_data->enable_block->CAM_ENABLE_2  );
		fpPrint( "CAM_ENABLE_3 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_3 , mqos_data->enable_block->CAM_ENABLE_3  );
		fpPrint( "CAM_ENABLE_4 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_4 , mqos_data->enable_block->CAM_ENABLE_4  );
		fpPrint( "CAM_ENABLE_5 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_5 , mqos_data->enable_block->CAM_ENABLE_5  );
		fpPrint( "CAM_ENABLE_6 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_6 , mqos_data->enable_block->CAM_ENABLE_6  );
		fpPrint( "CAM_ENABLE_7 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_7 , mqos_data->enable_block->CAM_ENABLE_7  );
		fpPrint( "CAM_ENABLE_8 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_8 , mqos_data->enable_block->CAM_ENABLE_8  );
		fpPrint( "CAM_ENABLE_9 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_9 , mqos_data->enable_block->CAM_ENABLE_9  );
		fpPrint( "CAM_ENABLE_10 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_10, mqos_data->enable_block->CAM_ENABLE_10 );
		fpPrint( "CAM_ENABLE_11 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_11, mqos_data->enable_block->CAM_ENABLE_11 );
		fpPrint( "CAM_ENABLE_12 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_12, mqos_data->enable_block->CAM_ENABLE_12 );
		fpPrint( "CAM_ENABLE_13 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_13, mqos_data->enable_block->CAM_ENABLE_13 );
		fpPrint( "CAM_ENABLE_14 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_14, mqos_data->enable_block->CAM_ENABLE_14 );
		fpPrint( "CAM_ENABLE_15 @0x%X: 0x%X\n",	(unsigned int) &mqos_data->enable_block->CAM_ENABLE_15, mqos_data->enable_block->CAM_ENABLE_15 );
}

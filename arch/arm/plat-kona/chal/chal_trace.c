/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*  @file   chal_trace.c
*
*  @brief  OS independent code of TRACE/DEBUG hardware abstraction APIs.
*
*  @note
*
****************************************************************************/

#define UNDER_LINUX		/* Only supports Rhea */
#define FUSE_APPS_PROCESSOR

#include <mach/memory.h>
#include "linux/broadcom/mobcom_types.h"
#include "plat/chal/chal_common.h"
#include "plat/chal/chal_trace.h"
#include "mach/rdb/brcm_rdb_util.h"
#include "mach/rdb/brcm_rdb_atbfilter.h"
#include <mach/io_map.h>
#if defined(_HERA_)
#include "mach/rdb/brcm_rdb_apbtoatb.h"
#include "mach/rdb/brcm_rdb_atb2pti.h"
#include "mach/rdb/brcm_rdb_atb2ocp.h"
#include "mach/rdb/brcm_rdb_wgm_ocp2atb.h"
#include "mach/rdb/brcm_rdb_hubocp2atb.h"
#endif /* defined(_HERA_) */
#include "mach/rdb/brcm_rdb_cstf.h"
#include "mach/rdb/brcm_rdb_axitp1.h"
#include "mach/rdb/brcm_rdb_gictr.h"
#include "mach/rdb/brcm_rdb_pwrmgr.h"
#include "mach/rdb/brcm_rdb_cti.h"
#include "mach/rdb/brcm_rdb_etb.h"
#include "mach/rdb/brcm_rdb_etb2axi.h"
#if !defined(_SAMOA_)
#include "mach/rdb/brcm_rdb_globperf.h"
#endif /* !defined(_SAMOA_) */
#if !defined(_HERA_)
#include "mach/rdb/brcm_rdb_atb_stm.h"
#include "mach/rdb/brcm_rdb_swstm.h"
#include "mach/rdb/brcm_rdb_chipreg.h"
#endif /* !defined(_HERA_) */

/*=======================================================
*  Local Definition
* ========================================================
*/
#define CHAL_TRACE_REG_DATA_FIELD_SET(r, f, d) ((((BRCM_REGTYPE(r))d) \
		<< BRCM_FIELDSHIFT(r, f)) & BRCM_FIELDMASK(r, f))
#define CHAL_TRACE_REG_DATA_FIELD_GET(r, f, d) ((((BRCM_REGTYPE(r))d) & \
		BRCM_FIELDMASK(r, f)) >> BRCM_FIELDSHIFT(r, f))

/*=======================================================
*  Local Variables
* =======================================================
*/
/* CHAL must be designed as a very simple interface with zero or
* minimal intelligence. */
/*CHAL must not have any local storage (global variables, arrays,
*	etc. within CHAL module), unless absolutely necessary
*	(for complex blocks). */
/* CHAL must not maintain any information about the handle.
*	The handle itself can
*	serve as the base address of a hardware block. */

/******************************************************************************
*  Local Functions
******************************************************************************
*/
static cUInt32 chal_trace_funnel_get_baseaddr(CHAL_HANDLE handle,
					      CHAL_TRACE_FUNNEL_t funnel_type);
static cUInt32 chal_trace_axitrace_get_baseaddr(CHAL_HANDLE handle,
						CHAL_TRACE_AXITRACE_t
						axitrace_type);

/*===========================================================================
* Function Definition
*===========================================================================
*/
/**************************************************************************
*
* Function Name: chal_trace_init
*
* Description:
*
*************************************************************************
*/
cBool chal_trace_init(CHAL_TRACE_DEV_t *pTraceDev_baseaddr)
{

	/* All register config values taken from T32 script */

#if defined(CONFIG_ARCH_RHEA)
	BRCM_WRITE_REG_FIELD(KONA_CHIPREG_VA, CHIPREG_PERIPH_SPARE_CONTROL1,
			     PTI_CLK_IS_IDLE, 1);
#elif defined(CONFIG_ARCH_ISLAND)
	BRCM_WRITE_REG_FIELD(KONA_CHIPREG_VA, CHIPREG_ARM_PERI_CONTROL,
			     PTI_CLK_IS_IDLE, 1);
#endif

	/* Config ATB Filter rm id's for STM */
	BRCM_WRITE_REG(KONA_ATBFILTER_VA, ATBFILTER_ATB_FILTER, 0x203);
	/* Config Funnels */
	BRCM_WRITE_REG(KONA_FUNNEL_VA, CSTF_FUNNEL_CONTROL, 0xe40);
	BRCM_WRITE_REG(KONA_FIN_FUNNEL_VA, CSTF_FUNNEL_CONTROL, 0xe02);
	/* Config STM */
	BRCM_WRITE_REG(KONA_STM_VA, ATB_STM_CONFIG, 0x102);
	BRCM_WRITE_REG(KONA_SWSTM_VA, SWSTM_R_CONFIG, 0x82);
	BRCM_WRITE_REG(KONA_SWSTM_ST_VA, SWSTM_R_CONFIG, 0x82);

	return TRUE;
}

#if defined(_HERA_)
/**************************************************************************
*
* Function Name: chal_trace_cfg_hub_atb2pti
*
* Description: Config hub  ATB to PTI Configuration
*
**************************************************************************
*/
cBool chal_trace_cfg_hub_atb2pti(CHAL_HANDLE handle, cBool twobit_mode,
				 cBool intlv_mode, cBool pti_en,
				 cUInt8 match_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ATB2PTI_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB2PTI_base, ATB2PTI_CONFIG,
				     TWOBIT_MODE, twobit_mode);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB2PTI_base, ATB2PTI_CONFIG,
				     INTLV_MODE, intlv_mode);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB2PTI_base, ATB2PTI_CONFIG,
				     PTI_EN, pti_en);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB2PTI_base, ATB2PTI_CONFIG,
				     MATCH_ID, match_id);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cfg_hub_atb2ocp_sw_id
*
* Description: Config hub ATB to OCP SW Source ID's
*
**************************************************************************
*/
cBool chal_trace_cfg_hub_atb2ocp_sw_id(CHAL_HANDLE handle, cUInt8 sw,
				       cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ATB2OCP_base) {
		switch (sw) {
		case 0:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_SW, SW0_ID, atb_id);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_SW, SW1_ID, atb_id);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_SW, SW2_ID, atb_id);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_SW, SW3_ID, atb_id);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cfg_hub_atb2ocp_filter_id
*
* Description: Config hub ATB to OCP Filter-Out ID's
*
**************************************************************************
*/
cBool chal_trace_cfg_hub_atb2ocp_filter_id(CHAL_HANDLE handle, cUInt8 filter,
					   cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ATB2OCP_base) {
		switch (filter) {
		case 0:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_FILTER, FILTER0_ID,
					     atb_id);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_FILTER, FILTER1_ID,
					     atb_id);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_FILTER, FILTER2_ID,
					     atb_id);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(pTraceDev->ATB2OCP_base,
					     ATB2OCP_FILTER, FILTER3_ID,
					     atb_id);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cfg_fabric_apb2atb
*
* Description: Config fabric APB to ATB atb ID
*
**************************************************************************
*/
cBool chal_trace_cfg_fabric_apb2atb_atb_id(CHAL_HANDLE handle, cUInt8 cpu_n,
					   cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->APB2ATB_base) {
		switch (cpu_n) {
		case 0:
			BRCM_WRITE_REG_FIELD(pTraceDev->APB2ATB_base,
					     APBTOATB_R_CPU_0_ID, CPU_0_ID,
					     atb_id);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(pTraceDev->APB2ATB_base,
					     APBTOATB_R_CPU_1_ID, CPU_1_ID,
					     atb_id);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(pTraceDev->APB2ATB_base,
					     APBTOATB_R_CPU_2_ID, CPU_2_ID,
					     atb_id);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(pTraceDev->APB2ATB_base,
					     APBTOATB_R_CPU_3_ID, CPU_3_ID,
					     atb_id);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cfg_fabric_apb2atb_stall_mode
*
* Description: Config fabric APB to ATB stall mode
*
**************************************************************************
*/
cBool chal_trace_cfg_fabric_apb2atb_stall_mode(CHAL_HANDLE handle,
					       cBool stall_mode)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->APB2ATB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->APB2ATB_base,
				     APBTOATB_R_CPU_0_ID, STALL_MODE,
				     stall_mode);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cfg_comms_ocp2atb
*
* Description: Config COMMS OCP to ATB
*
**************************************************************************
*/
cBool chal_trace_cfg_comms_ocp2atb(CHAL_HANDLE handle, cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->WGM_OCP2ATB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->WGM_OCP2ATB_base,
				     WGM_OCP2ATB_ATBID_REG, ATBID, atb_id);
		status = TRUE;
	}

	return status;
}

/***************************************************************************
*
* Function Name: chal_trace_cfg_hub_ocp2atb
*
* Description: Config HUB OCP to ATB
*
***************************************************************************
*/
cBool chal_trace_cfg_hub_ocp2atb(CHAL_HANDLE handle, cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->HUBOCP2ATB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->HUBOCP2ATB_base,
				     HUBOCP2ATB_CONFIG, ATB_ID, atb_id);
		status = TRUE;
	}

	return status;
}
#endif /* defined(_HERA_) */

/**************************************************************************
*
* Function Name: chal_trace_funnel_set_enable
*
* Description: Config Funnel_Control bits CSTF Control
* Register enable bits for each port
*
**************************************************************************
*/
cBool chal_trace_funnel_set_enable(CHAL_HANDLE handle,
				   CHAL_TRACE_FUNNEL_t funnel_type,
				   cUInt8 port_n, cBool enable)
{
	cUInt32 base_address = 0;
	cBool status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		switch (port_n) {
		case 0:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_0, enable);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_1, enable);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_2, enable);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_3, enable);
			status = TRUE;
			break;

		case 4:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_4, enable);
			status = TRUE;
			break;

		case 5:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_5, enable);
			status = TRUE;
			break;

		case 6:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_6, enable);
			status = TRUE;
			break;

		case 7:
			BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
					     ENABLE_SLAVE_PORT_7, enable);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_set_min_hold_time
*
* Description: Config Funnel_Control bits CSTF Control
* Register Minimum_hold_time_3_0 field
*
**************************************************************************
*/
cBool chal_trace_funnel_set_min_hold_time(CHAL_HANDLE handle,
					  CHAL_TRACE_FUNNEL_t funnel_type,
					  cUInt8 min_hold_time)
{
	cUInt32 base_address = 0;
	cBool status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		BRCM_WRITE_REG_FIELD(base_address, CSTF_FUNNEL_CONTROL,
				     MINIMUM_HOLD_TIME_3_0, min_hold_time);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_set_priority
*
* Description: Config Priority_Control bits CSTF Priority Control Register
*
**************************************************************************
*/
cBool chal_trace_funnel_set_priority(CHAL_HANDLE handle,
				     CHAL_TRACE_FUNNEL_t funnel_type,
				     cUInt8 port_n, cUInt8 priority)
{
	cUInt32 base_address = 0;
	cBool status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		switch (port_n) {
		case 0:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_0,
					     priority);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_1,
					     priority);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_2,
					     priority);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_3,
					     priority);
			status = TRUE;
			break;

		case 4:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_4,
					     priority);
			status = TRUE;
			break;

		case 5:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_5,
					     priority);
			status = TRUE;
			break;

		case 6:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_6,
					     priority);
			status = TRUE;
			break;

		case 7:
			BRCM_WRITE_REG_FIELD(base_address,
					     CSTF_PRIORITY_CONTROL, PRIPORT_7,
					     priority);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_set_itctrl
*
* Description: Config ItCtrl bits Integration Mode Control Registers.
*
**************************************************************************
*/
cBool chal_trace_funnel_set_itctrl(CHAL_HANDLE handle,
				   CHAL_TRACE_FUNNEL_t funnel_type, cUInt8 mode)
{
	cUInt32 base_address = 0;
	cBool status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		BRCM_WRITE_REG_FIELD(base_address, CSTF_ITCTRL, MODE, mode);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_set_claim_tag_set
*
* Description: Config Claim_Tag_Set bits Claim Tag Set Registers.
*
**************************************************************************
*/
cBool chal_trace_funnel_set_claim_tag_set(CHAL_HANDLE handle,
					  CHAL_TRACE_FUNNEL_t funnel_type,
					  cUInt8 ctv)
{
	cUInt32 base_address = 0;
	cBool status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		BRCM_WRITE_REG_FIELD(base_address, CSTF_CLAIM_TAG_SET, CTV,
				     ctv);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_set_claim_tag_clear
*
* Description: Config Claim_Tag_Clear bits Claim Tag Clear Registers.
*
**************************************************************************
*/
cBool chal_trace_funnel_set_claim_tag_clear(CHAL_HANDLE handle,
					    CHAL_TRACE_FUNNEL_t funnel_type,
					    cUInt8 ctv)
{
	cUInt32 base_address;
	cBool status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		BRCM_WRITE_REG_FIELD(base_address, CSTF_CLAIM_TAG_CLEAR, CTV,
				     ctv);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_set_lock_access
*
* Description: Set LockAccess bits Lock Access Registers.
*
**************************************************************************
*/
cBool chal_trace_funnel_set_lock_access(CHAL_HANDLE handle,
					CHAL_TRACE_FUNNEL_t funnel_type,
					cUInt32 write_access_code)
{
	cUInt32 base_address;
	cBool status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		BRCM_WRITE_REG_FIELD(base_address, CSTF_LOCKACCESS,
				     WRITE_ACCESS_CODE, write_access_code);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_get_lock_status
*
* Description: Read LockStatus bits Lock Status Registers.
*
**************************************************************************
*/
cUInt32 chal_trace_funnel_get_lock_status(CHAL_HANDLE handle,
					  CHAL_TRACE_FUNNEL_t funnel_type)
{
	cUInt32 base_address;
	cUInt32 lock_status = 0;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address)
		lock_status = BRCM_READ_REG(base_address, CSTF_LOCKSTATUS);

	return lock_status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_locked
*
* Description: Return TRUE when the lock mechanism is implemented
* and device write access is locked.
*
**************************************************************************
*/
cBool chal_trace_funnel_locked(CHAL_HANDLE handle,
			       CHAL_TRACE_FUNNEL_t funnel_type)
{
	cUInt32 base_address;
	cBool lock_status = FALSE;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		if ((BRCM_READ_REG_FIELD
		     (base_address, CSTF_LOCKSTATUS, LOCK_MECHANISM))
		    &&
		    (BRCM_READ_REG_FIELD
		     (base_address, CSTF_LOCKSTATUS, LOCK_STATUS))) {
			lock_status = TRUE;
		}
	}

	return lock_status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_get_auth_status
*
* Description: Read AuthStatus bits Authentication Status Registers.
*
**************************************************************************
*/
cUInt32 chal_trace_funnel_get_auth_status(CHAL_HANDLE handle,
					  CHAL_TRACE_FUNNEL_t funnel_type)
{
	cUInt32 base_address;
	cUInt32 auth_status = 0;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address)
		auth_status = BRCM_READ_REG(base_address, CSTF_AUTHSTATUS);

	return auth_status;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_get_device_id
*
* Description: Read Device_ID bits Component Configuration Registers.
*
**************************************************************************
*/
cUInt32 chal_trace_funnel_get_device_id(CHAL_HANDLE handle,
					CHAL_TRACE_FUNNEL_t funnel_type)
{
	cUInt32 base_address;
	cUInt32 device_id = 0;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address)
		device_id = BRCM_READ_REG(base_address, CSTF_DEVICE_ID);

	return device_id;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_get_device_type_identifier
*
* Description: Read Device_Type_Identifier bits Device
* Type Identifier Registers.
*
**************************************************************************
*/
cUInt32 chal_trace_funnel_get_device_type_identifier(CHAL_HANDLE handle,
						     CHAL_TRACE_FUNNEL_t
						     funnel_type)
{
	cUInt32 base_address;
	cUInt32 device_type_id = 0;

	base_address = chal_trace_funnel_get_baseaddr(handle, funnel_type);

	if (base_address) {
		device_type_id =
		    BRCM_READ_REG(base_address, CSTF_DEVICE_TYPE_IDENTIFIER);
	}

	return device_type_id;
}

/**************************************************************************
*
* Function Name: chal_trace_funnel_get_baseaddr
*
* Description: returns the base address of the register
* block for each funnel_type .
*
***************************************************************************
*/
static cUInt32 chal_trace_funnel_get_baseaddr(CHAL_HANDLE handle,
					      CHAL_TRACE_FUNNEL_t funnel_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 base_address = 0;

	if (funnel_type < CHAL_TRACE_MAX_FUNNEL)
		base_address = pTraceDev->FUNNEL_base[funnel_type];

	return base_address;
}

/**************************************************************************
*
* Function Name: chal_trace_gic_set_config
*
* Description: Set GIC Trace Config
*
**************************************************************************
*/
cBool chal_trace_gic_set_config(CHAL_HANDLE handle,
				CHAL_TRACE_GIC_CONF_t *p_gic_config)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;
	cUInt32 gic_config;

	if (pTraceDev->GICTR_base) {
		gic_config =
		    CHAL_TRACE_REG_DATA_FIELD_SET(GICTR_GIC_CONFIG, LATENCY_EN,
						  p_gic_config->
						  LATENCY_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(GICTR_GIC_CONFIG, TRACE_EN,
						  p_gic_config->
						  TRACE_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(GICTR_GIC_CONFIG, CTRL_SRC,
						  p_gic_config->CTRL_SRC);

		BRCM_WRITE_REG(pTraceDev->GICTR_base, GICTR_GIC_CONFIG,
			       gic_config);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_gic_get_config
*
* Description: Get GIC Trace Config
*
**************************************************************************
*/
cBool chal_trace_gic_get_config(CHAL_HANDLE handle,
				CHAL_TRACE_GIC_CONF_t *p_gic_config)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;
	cUInt32 gic_config;

	if (pTraceDev->GICTR_base) {

		gic_config =
		    BRCM_READ_REG(pTraceDev->GICTR_base, GICTR_GIC_CONFIG);

		p_gic_config->LATENCY_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(GICTR_GIC_CONFIG, LATENCY_EN,
						  gic_config);
		p_gic_config->TRACE_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(GICTR_GIC_CONFIG, TRACE_EN,
						  gic_config);
		p_gic_config->CTRL_SRC =
		    CHAL_TRACE_REG_DATA_FIELD_GET(GICTR_GIC_CONFIG, CTRL_SRC,
						  gic_config);

		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_gic_set_outid
*
* Description: Set GIC Trace Output ID's
*
**************************************************************************
*/
cBool chal_trace_gic_set_outid(CHAL_HANDLE handle, cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->GICTR_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->GICTR_base, GICTR_GIC_OUTIDS,
				     RATB_ID, atb_id);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_gic_set_cmd
*
* Description: Set GIC trace Local Command
*
**************************************************************************
*/
cBool chal_trace_gic_set_cmd(CHAL_HANDLE handle, CHAL_TRACE_LOCAL_CMD_t command)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->GICTR_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->GICTR_base, GICTR_GIC_CMD,
				     LOCAL_CMD, command);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_gic_get_status
*
* Description: Get GIC Trace Status
*
***************************************************************************
*/
cBool chal_trace_gic_get_status(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->GICTR_base) {
		if (BRCM_READ_REG_FIELD
		    (pTraceDev->GICTR_base, GICTR_GIC_STATUS,
		     STATE_LATENCY_EN)) {
			status = TRUE;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_gic_get_latency
*
* Description: Get FIQ/IRQ Latency
*
***************************************************************************
*/
cUInt32 chal_trace_gic_get_latency(CHAL_HANDLE handle,
				   CHAL_TRACE_GIC_LAT_t fiq_irq)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 latency = 0;

	if (pTraceDev->GICTR_base) {
		switch (fiq_irq) {
		case CHAL_TRACE_GIC_FIQ_LAT0:
			latency =
			    BRCM_READ_REG(pTraceDev->GICTR_base,
					  GICTR_GIC_FIQ_LAT0);
			break;

		case CHAL_TRACE_GIC_IRQ_LAT0:
			latency =
			    BRCM_READ_REG(pTraceDev->GICTR_base,
					  GICTR_GIC_IRQ_LAT0);
			break;

		case CHAL_TRACE_GIC_FIQ_LAT1:
			latency =
			    BRCM_READ_REG(pTraceDev->GICTR_base,
					  GICTR_GIC_FIQ_LAT1);
			break;

		case CHAL_TRACE_GIC_IRQ_LAT1:
			latency =
			    BRCM_READ_REG(pTraceDev->GICTR_base,
					  GICTR_GIC_IRQ_LAT1);
			break;

		default:
			break;
		}
	}

	return latency;
}

/**************************************************************************
*
* Function Name: chal_trace_pwrmgr_set_atb_id
*
* Description: Set PM ATB trace ID
*
***************************************************************************
*/
cBool chal_trace_pwrmgr_set_atb_id(CHAL_HANDLE handle, cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->GICTR_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->PWRMGR_base,
				     PWRMGR_PC_PIN_OVERRIDE_CONTROL, ATB_ID,
				     atb_id);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_set_atm_config
*
* Description: Set AXI Trace Config
*
**************************************************************************
*/
cBool chal_trace_axi_set_atm_config(CHAL_HANDLE handle,
				    CHAL_TRACE_AXITRACE_t axitrace_type,
				    CHAL_TRACE_ATM_CONF_t *p_atm_config)
{
	cUInt32 base_address;
	cBool status = FALSE;
	cUInt32 atm_config;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address) {

		atm_config =
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, BUSY_ID,
						  p_atm_config->
						  BUSY_ID) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG,
						  OUTS_THRESH,
						  p_atm_config->
						  OUTS_THRESH) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG,
						  OUTS_FILTERED,
						  p_atm_config->
						  OUTS_FILTERED) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, OUTS_EN,
						  p_atm_config->
						  OUTS_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, RDLAT_MODE,
						  p_atm_config->
						  RDLAT_MODE) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG,
						  LATENCY_FILTERED,
						  p_atm_config->
						  LATENCY_FILTERED) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, LATENCY_EN,
						  p_atm_config->
						  LATENCY_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG,
						  BUSY_FILTERED,
						  p_atm_config->
						  BUSY_FILTERED) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, BUSY_EN,
						  p_atm_config->
						  BUSY_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG,
						  BEATS_FILTERED,
						  p_atm_config->
						  BEATS_FILTERED) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, BEATS_EN,
						  p_atm_config->
						  BEATS_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG,
						  CMDS_FILTERED,
						  p_atm_config->
						  CMDS_FILTERED) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, CMDS_EN,
						  p_atm_config->
						  CMDS_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, TRACE_EN,
						  p_atm_config->
						  TRACE_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, FLUSH,
						  p_atm_config->
						  FLUSH) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, SAT_EN,
						  p_atm_config->
						  SAT_EN) |
		    CHAL_TRACE_REG_DATA_FIELD_SET(AXITP1_ATM_CONFIG, CTRL_SRC,
						  p_atm_config->CTRL_SRC);

		BRCM_WRITE_REG(base_address, AXITP1_ATM_CONFIG, atm_config);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_get_atm_config
*
* Description: Get AXI Trace Config
*
**************************************************************************
*/
cBool chal_trace_axi_get_atm_config(CHAL_HANDLE handle,
				    CHAL_TRACE_AXITRACE_t axitrace_type,
				    CHAL_TRACE_ATM_CONF_t *p_atm_config)
{
	cUInt32 base_address;
	cBool status = FALSE;
	cUInt32 atm_config;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address) {

		atm_config = BRCM_READ_REG(base_address, AXITP1_ATM_CONFIG);

		p_atm_config->BUSY_ID =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, BUSY_ID,
						  atm_config);
		p_atm_config->OUTS_THRESH =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG,
						  OUTS_THRESH, atm_config);
		p_atm_config->OUTS_FILTERED =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG,
						  OUTS_FILTERED, atm_config);
		p_atm_config->OUTS_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, OUTS_EN,
						  atm_config);
		p_atm_config->RDLAT_MODE =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, RDLAT_MODE,
						  atm_config);
		p_atm_config->LATENCY_FILTERED =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG,
						  LATENCY_FILTERED, atm_config);
		p_atm_config->LATENCY_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, LATENCY_EN,
						  atm_config);
		p_atm_config->BUSY_FILTERED =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG,
						  BUSY_FILTERED, atm_config);
		p_atm_config->BUSY_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, BUSY_EN,
						  atm_config);
		p_atm_config->BEATS_FILTERED =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG,
						  BEATS_FILTERED, atm_config);
		p_atm_config->BEATS_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, BEATS_EN,
						  atm_config);
		p_atm_config->CMDS_FILTERED =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG,
						  CMDS_FILTERED, atm_config);
		p_atm_config->CMDS_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, CMDS_EN,
						  atm_config);
		p_atm_config->TRACE_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, TRACE_EN,
						  atm_config);
		p_atm_config->FLUSH =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, FLUSH,
						  atm_config);
		p_atm_config->SAT_EN =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, SAT_EN,
						  atm_config);
		p_atm_config->CTRL_SRC =
		    CHAL_TRACE_REG_DATA_FIELD_GET(AXITP1_ATM_CONFIG, CTRL_SRC,
						  atm_config);

		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_set_atm_outid
*
* Description: Set AXI Trace Output ID's
*
**************************************************************************
*/
cBool chal_trace_axi_set_atm_outid(CHAL_HANDLE handle,
				   CHAL_TRACE_AXITRACE_t axitrace_type,
				   cUInt8 w_atb_id, cUInt8 r_atb_id)
{
	cUInt32 base_address;
	cBool status = FALSE;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address) {
		BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_OUTIDS, WATB_ID,
				     w_atb_id);
		BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_OUTIDS, RATB_ID,
				     r_atb_id);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_set_atm_cmd
*
* Description: Set AXI trace Local Command
*
**************************************************************************
*/
cBool chal_trace_axi_set_atm_cmd(CHAL_HANDLE handle,
				 CHAL_TRACE_AXITRACE_t axitrace_type,
				 CHAL_TRACE_LOCAL_CMD_t cmd)
{
	cUInt32 base_address;
	cBool status = FALSE;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address) {
		BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_CMD, LOCAL_CMD,
				     cmd);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_get_status
*
* Description: Return AXI Trace Status
*
**************************************************************************
*/
cUInt32 chal_trace_axi_get_status(CHAL_HANDLE handle,
				  CHAL_TRACE_AXITRACE_t axitrace_type)
{
	cUInt32 base_address;
	cUInt32 atm_status = 0;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address)
		atm_status = BRCM_READ_REG(base_address, AXITP1_ATM_STATUS);

	return atm_status;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_get_status_sat
*
* Description: Return AXI Trace Status SAT_STOPPED field
*
**************************************************************************
*/
cUInt32 chal_trace_axi_get_status_sat(CHAL_HANDLE handle,
				      CHAL_TRACE_AXITRACE_t axitrace_type)
{
	cUInt32 base_address;
	cUInt32 atm_status = 0;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address) {
		atm_status =
		    BRCM_READ_REG_FIELD(base_address, AXITP1_ATM_STATUS,
					SAT_STOPPED);
	}

	return atm_status;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_get_count
*
* Description: Return AXI Trace Performance Count
*
**************************************************************************
*/
cUInt32 chal_trace_axi_get_count(CHAL_HANDLE handle,
				 CHAL_TRACE_AXITRACE_t axitrace_type,
				 CHAL_TRACE_AXITRACE_COUNT_t counter)
{
	cUInt32 base_address;
	cUInt32 atm_count = 0;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address) {
		switch (counter) {
		case WRITE_COMMANDS:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WRCMDS);
			break;

		case READ_COMMANDS:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RDCMDS);
			break;

		case WRITE_ADDRESS_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_AWCYCLES);
			break;

		case READ_ADDRESS_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_ARCYCLES);
			break;

		case WRITE_DATA_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WCYCLES);
			break;

		case READ_DATA_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RCYCLES);
			break;

		case WRITE_RESPONSE_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_BCYCLES);
			break;

		case WRITE_ADDRESS_BUSY_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_AWBUSY);
			break;

		case READ_ADDRESS_BUSY_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_ARBUSY);
			break;

		case WRITE_DATA_BUSY_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WBUSY);
			break;

		case READ_DATA_BUSY_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RBUSY);
			break;

		case WRITE_RESPONSE_BUSY_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_BBUSY);
			break;

		case WRITE_LATENCY_SUM_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WRSUM);
			break;

		case READ_LATENCY_SUM_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RDSUM);
			break;

		case WRITE_LATENCY_MINIMUM_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WRMIN);
			break;

		case READ_LATENCY_MINIMUM_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RDMIN);
			break;

		case WRITE_LATENCY_MAXIMUM_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WRMAX);
			break;

		case READ_LATENCY_MAXIMUM_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RDMAX);
			break;

		case WRITE_DATA_BEATS:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WRBEATS);
			break;

		case READ_DATA_BEATS:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RDBEATS);
			break;

		case WRITE_OUTSTANDING_COMMANDS_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_WROUTS);
			break;

		case READ_OUTSTANDING_COMMANDS_CYCLES:
			atm_count =
			    BRCM_READ_REG(base_address, AXITP1_ATM_RDOUTS);
			break;

		default:
			/* invalid input param */
			break;
		}
	}

	return atm_count;
}

/**************************************************************************
*
* Function Name: chal_trace_axi_set_filter
*
* Description: Set AXI filter configuratin/address low/address high
*
**************************************************************************
*/
cBool chal_trace_axi_set_filter(CHAL_HANDLE handle,
				CHAL_TRACE_AXITRACE_t axitrace_type,
				cUInt8 filter_num,
				CHAL_TRACE_AXI_FILTER_CONF_t *filter_config,
				cUInt32 addr_low, cUInt32 addr_high)
{
	cUInt32 base_address;
	cBool status = FALSE;

	base_address = chal_trace_axitrace_get_baseaddr(handle, axitrace_type);

	if (base_address) {
		switch (filter_num) {
		case 0:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     TRIGGER_EN_0,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     FILTER_SEC_0,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     FILTER_OPEN_0,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     FILTER_ID_0,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     ID_MASK_0, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     FILTER_LEN_0,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     LEN_MODE_0,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     FILTER_WRITE_0,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_0,
					     FILTER_READ_0,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_0,
					     ADDR_LOW_0, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_0, ADDR_HIGH_0,
					     addr_high);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     TRIGGER_EN_1,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     FILTER_SEC_1,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     FILTER_OPEN_1,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     FILTER_ID_1,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     ID_MASK_1, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     FILTER_LEN_1,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     LEN_MODE_1,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     FILTER_WRITE_1,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_1,
					     FILTER_READ_1,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_1,
					     ADDR_LOW_1, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_1, ADDR_HIGH_1,
					     addr_high);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     TRIGGER_EN_2,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     FILTER_SEC_2,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     FILTER_OPEN_2,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     FILTER_ID_2,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     ID_MASK_2, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     FILTER_LEN_2,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     LEN_MODE_2,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     FILTER_WRITE_2,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_2,
					     FILTER_READ_2,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_2,
					     ADDR_LOW_2, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_2, ADDR_HIGH_2,
					     addr_high);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     TRIGGER_EN_3,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     FILTER_SEC_3,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     FILTER_OPEN_3,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     FILTER_ID_3,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     ID_MASK_3, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     FILTER_LEN_3,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     LEN_MODE_3,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     FILTER_WRITE_3,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_3,
					     FILTER_READ_3,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_3,
					     ADDR_LOW_3, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_3, ADDR_HIGH_3,
					     addr_high);
			status = TRUE;
			break;

		case 4:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     TRIGGER_EN_4,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     FILTER_SEC_4,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     FILTER_OPEN_4,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     FILTER_ID_4,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     ID_MASK_4, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     FILTER_LEN_4,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     LEN_MODE_4,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     FILTER_WRITE_4,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_4,
					     FILTER_READ_4,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_4,
					     ADDR_LOW_4, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_4, ADDR_HIGH_4,
					     addr_high);
			status = TRUE;
			break;

		case 5:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     TRIGGER_EN_5,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     FILTER_SEC_5,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     FILTER_OPEN_5,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     FILTER_ID_5,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     ID_MASK_5, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     FILTER_LEN_5,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     LEN_MODE_5,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     FILTER_WRITE_5,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_5,
					     FILTER_READ_5,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_5,
					     ADDR_LOW_5, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_5, ADDR_HIGH_5,
					     addr_high);
			status = TRUE;
			break;

		case 6:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     TRIGGER_EN_6,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     FILTER_SEC_6,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     FILTER_OPEN_6,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     FILTER_ID_6,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     ID_MASK_6, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     FILTER_LEN_6,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     LEN_MODE_6,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     FILTER_WRITE_6,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_6,
					     FILTER_READ_6,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_6,
					     ADDR_LOW_6, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_6, ADDR_HIGH_6,
					     addr_high);
			status = TRUE;
			break;

		case 7:
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     TRIGGER_EN_7,
					     filter_config->TRIGGER_EN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     FILTER_SEC_7,
					     filter_config->FILTER_SEC);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     FILTER_OPEN_7,
					     filter_config->FILTER_OPEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     FILTER_ID_7,
					     filter_config->FILTER_ID);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     ID_MASK_7, filter_config->ID_MASK);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     FILTER_LEN_7,
					     filter_config->FILTER_LEN);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     LEN_MODE_7,
					     filter_config->LEN_MODE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     FILTER_WRITE_7,
					     filter_config->FILTER_WRITE);
			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_FILTER_7,
					     FILTER_READ_7,
					     filter_config->FILTER_READ);

			BRCM_WRITE_REG_FIELD(base_address, AXITP1_ATM_ADDRLOW_7,
					     ADDR_LOW_7, addr_low);
			BRCM_WRITE_REG_FIELD(base_address,
					     AXITP1_ATM_ADDRHIGH_7, ADDR_HIGH_7,
					     addr_high);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_axitrace_get_baseaddr
*
* Description: returns the base address of the register
* block for each axitrace .
*
***************************************************************************
*/
static cUInt32 chal_trace_axitrace_get_baseaddr(CHAL_HANDLE handle,
						CHAL_TRACE_AXITRACE_t
						axitrace_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 base_address = 0;

	if (axitrace_type < CHAL_TRACE_MAX_AXITRACE)
		base_address = pTraceDev->AXITRACE_base[axitrace_type];

	return base_address;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_control
*
* Description: Global enable.
*
**************************************************************************
*/
cBool chal_trace_cti_set_control(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type,
				 cBool enable)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_CTICONTROL, GLBEN, enable);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_int_ack
*
* Description: Acknowledges nCTIIRQ outputs.
*
**************************************************************************
*/
cBool chal_trace_cti_set_int_ack(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type,
				 cUInt8 int_ack)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_CTIINTACK, INTACK, int_ack);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_app_set
*
* Description: CTI Application Trigger Set
*
**************************************************************************
*/
cBool chal_trace_cti_set_app_set(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type,
				 cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_CTIAPPSET, APPSET, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_app_clear
*
* Description: CTI Application Trigger Clear
*
**************************************************************************
*/
cBool chal_trace_cti_set_app_clear(CHAL_HANDLE handle,
				   CHAL_TRACE_CTI_t cti_type, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_CTIAPPCLEAR, APPCLEAR, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_app_pulse
*
* Description: CTI Application Pulse
*
**************************************************************************
*/
cBool chal_trace_cti_set_app_pulse(CHAL_HANDLE handle,
				   CHAL_TRACE_CTI_t cti_type, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_CTIAPPPULSE, APPULSE, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_in_en
*
* Description: CTI Trigger n to Channel Enable
*
**************************************************************************
*/
cBool chal_trace_cti_set_in_en(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type,
			       cUInt8 n_trigger, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		switch (n_trigger) {
		case 0:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN0, TRIGINEN0, channel);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN1, TRIGINEN1, channel);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN2, TRIGINEN2, channel);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN3, TRIGINEN3, channel);
			status = TRUE;
			break;

		case 4:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN4, TRIGINEN4, channel);
			status = TRUE;
			break;

		case 5:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN5, TRIGINEN5, channel);
			status = TRUE;
			break;

		case 6:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN6, TRIGINEN6, channel);
			status = TRUE;
			break;

		case 7:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIINEN7, TRIGINEN7, channel);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_out_en
*
* Description: CTI Channel to Trigger n Enable
*
**************************************************************************
*/
cBool chal_trace_cti_set_out_en(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type,
				cUInt8 n_trigger, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		switch (n_trigger) {
		case 0:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN0, TRIGOUTEN0,
					     channel);
			status = TRUE;
			break;

		case 1:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN1, TRIGOUTEN1,
					     channel);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN2, TRIGOUTEN2,
					     channel);
			status = TRUE;
			break;

		case 3:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN3, TRIGOUTEN3,
					     channel);
			status = TRUE;
			break;

		case 4:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN4, TRIGOUTEN4,
					     channel);
			status = TRUE;
			break;

		case 5:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN5, TRIGOUTEN5,
					     channel);
			status = TRUE;
			break;

		case 6:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN6, TRIGOUTEN6,
					     channel);
			status = TRUE;
			break;

		case 7:
			BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
					     CTI_CTIOUTEN7, TRIGOUTEN7,
					     channel);
			status = TRUE;
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_trig_in_status
*
* Description: CTI Trigger In Status
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_trig_in_status(CHAL_HANDLE handle,
					  CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_CTITRIGINSTATUS);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_trig_out_status
*
* Description: CTI Trigger Out Status
*
***************************************************************************
*/
cUInt32 chal_trace_cti_get_trig_out_status(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_CTITRIGOUTSTATUS);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_ch_in_status
*
* Description: CTI Channel in Status
*
***************************************************************************
*/
cUInt32 chal_trace_cti_get_ch_in_status(CHAL_HANDLE handle,
					CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_CTICHINSTATUS);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_ch_out_status
*
* Description: CTI Channel Out Status
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_ch_out_status(CHAL_HANDLE handle,
					 CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_CTICHOUTSTATUS);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_ch_gate
*
* Description: CTI Channel Gate
*
***************************************************************************
*/
cBool chal_trace_cti_set_ch_gate(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type,
				 cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG(pTraceDev->CTI_base[cti_type], CTI_CTICHGATE,
			       channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_asic_control
*
* Description: ASIC Control / Enables edge detection for trigger output n
*
**************************************************************************
*/
cBool chal_trace_cti_set_asic_control(CHAL_HANDLE handle,
				      CHAL_TRACE_CTI_t cti_type,
				      cUInt8 triger_output)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG(pTraceDev->CTI_base[cti_type], CTI_ASICCTL,
			       triger_output);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_it_ch_in_ack
*
* Description: Integ. Test Channel In Ack
*
***************************************************************************
*/
cBool chal_trace_cti_set_it_ch_in_ack(CHAL_HANDLE handle,
				      CHAL_TRACE_CTI_t cti_type, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_ITCHINACK, CTCHINACK, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_it_trig_in_ack
*
* Description: Integ. Test Trigger In Ack
*
**************************************************************************
*/
cBool chal_trace_cti_set_it_trig_in_ack(CHAL_HANDLE handle,
					CHAL_TRACE_CTI_t cti_type,
					cUInt8 trigger)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_ITTRIGINACK, CTTRIGINACK, trigger);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_it_ch_out
*
* Description: Integ. Test Channel Out
*
***************************************************************************
*/
cBool chal_trace_cti_set_it_ch_out(CHAL_HANDLE handle,
				   CHAL_TRACE_CTI_t cti_type, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type], CTI_ITCHOUT,
				     CTCHOUT, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_it_trig_out
*
* Description: Integ. Test Trigger Out
*
**************************************************************************
*/
cBool chal_trace_cti_set_it_trig_out(CHAL_HANDLE handle,
				     CHAL_TRACE_CTI_t cti_type, cUInt8 trigger)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_ITTRIGOUT, CTTRIGOUT, trigger);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_it_ch_out_ack
*
* Description: Integ. Test Channel Out Ack
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_it_ch_out_ack(CHAL_HANDLE handle,
					 CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_ITCHOUTACK);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_it_trig_out_ack
*
* Description: Integ. Test Trigger Out Ack
*
***************************************************************************
*/
cUInt32 chal_trace_cti_get_it_trig_out_ack(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_ITTRIGOUTACK);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_it_ch_in
*
* Description: Integ. Test Channel In
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_it_ch_in(CHAL_HANDLE handle,
				    CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type], CTI_ITCHIN);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_it_trig_in
*
* Description: Integ. External Output Control
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_it_trig_in(CHAL_HANDLE handle,
				      CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type], CTI_ITTRIGIN);
	}

	return status;
}

/***************************************************************************
*
* Function Name: chal_trace_cti_set_int_mode_control
*
* Description: Integragtion mode enable.
*
**************************************************************************
*/
cBool chal_trace_cti_set_int_mode_control(CHAL_HANDLE handle,
					  CHAL_TRACE_CTI_t cti_type,
					  cBool enable)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type], CTI_ICTRL,
				     INTEG_EN, enable);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_claim_set
*
* Description: Claim Tag Set
*
**************************************************************************
*/
cBool chal_trace_cti_set_claim_set(CHAL_HANDLE handle,
				   CHAL_TRACE_CTI_t cti_type, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_CLAIMSET, CLAIM_TAGSA, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_claim_clr
*
* Description: Claim Tag Clear
*
***************************************************************************
*/
cBool chal_trace_cti_set_claim_clr(CHAL_HANDLE handle,
				   CHAL_TRACE_CTI_t cti_type, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_CLAIMCLR, CLAIM_TAGSB, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_set_lock_access
*
* Description: Lock Access
*
**************************************************************************
*/
cBool chal_trace_cti_set_lock_access(CHAL_HANDLE handle,
				     CHAL_TRACE_CTI_t cti_type, cUInt32 control)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->CTI_base[cti_type]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->CTI_base[cti_type],
				     CTI_LOCKACCESS, LOCK_ACCESS, control);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_lock_status
*
* Description: Lock Status
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_lock_status(CHAL_HANDLE handle,
				       CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_LOCKSTATUS);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_auth_status
*
* Description: Authentication Status
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_auth_status(CHAL_HANDLE handle,
				       CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
				  CTI_AUTHSTATUS);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_dev_id
*
* Description: Device ID
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_dev_id(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type], CTI_DEVID);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_dev_type
*
* Description: Device Type
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_dev_type(CHAL_HANDLE handle,
				    CHAL_TRACE_CTI_t cti_type)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		status =
		    BRCM_READ_REG(pTraceDev->CTI_base[cti_type], CTI_DEVTYPE);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_per_id
*
* Description: Peripheral ID n
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_per_id(CHAL_HANDLE handle, CHAL_TRACE_CTI_t cti_type,
				  cUInt8 n_peripheral)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		switch (n_peripheral) {
		case 0:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_PERID0);
			break;

		case 1:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_PERID1);
			break;

		case 2:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_PERID2);
			break;

		case 3:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_PERID3);
			break;

		case 4:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_PERID4);
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_per_id
*
* Description: Component ID n
*
**************************************************************************
*/
cUInt32 chal_trace_cti_get_comp_id(CHAL_HANDLE handle,
				   CHAL_TRACE_CTI_t cti_type,
				   cUInt8 n_component)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->CTI_base[cti_type]) {
		switch (n_component) {
		case 0:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_COMPID0);
			break;

		case 1:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_COMPID1);
			break;

		case 2:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_COMPID2);
			break;

		case 3:
			status =
			    BRCM_READ_REG(pTraceDev->CTI_base[cti_type],
					  CTI_COMPID3);
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_ram_depth
*
* Description: Ram Depth
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_ram_depth(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_RDP);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_ram_read_data
*
* Description: Status
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_status(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_STS);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_ram_read_data
*
* Description: RAM Read Data
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_ram_read_data(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_RRD);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_ram_read_pointer
*
* Description: RAM Read Pointer
*
***************************************************************************
*/
cBool chal_trace_etb_set_ram_read_pointer(CHAL_HANDLE handle, cUInt32 rrp)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_RRP, RRP, rrp);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_ram_write_pointer
*
* Description: RAM Write Pointer
*
**************************************************************************
*/
cBool chal_trace_etb_set_ram_write_pointer(CHAL_HANDLE handle, cUInt32 rwp)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_RWP, RWP, rwp);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_trigger_counter
*
* Description: Trigger Counter
*
**************************************************************************
*/
cBool chal_trace_etb_set_trigger_counter(CHAL_HANDLE handle, cUInt32 trg)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_TRG, TRG, trg);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_control
*
* Description: Trace Capture Enable
*
***************************************************************************
*/
cBool chal_trace_etb_set_control(CHAL_HANDLE handle, cBool enable)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_CTL, TRACECAPTEN,
				     enable);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_ram_write_data
*
* Description: RAM Write Data
*
**************************************************************************
*/
cBool chal_trace_etb_set_ram_write_data(CHAL_HANDLE handle, cUInt32 rwd)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_RWD, RWD, rwd);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_ff_status
*
* Description: Formatter and Flush Status
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_ff_status(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_FFSR);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_ff_control
*
* Description: Formatter and Flush Control
*
***************************************************************************
*/
cBool chal_trace_etb_set_ff_control(CHAL_HANDLE handle,
				    CHAL_TRACE_ETB_FF_CONF_t *ffcr)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, STOPTRIG,
				     ffcr->StopTrig);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, STOPFL,
				     ffcr->StopFl);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, TRIGFL,
				     ffcr->TrigFl);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, TRIGEVT,
				     ffcr->TrigEvt);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, TRIGIN,
				     ffcr->Trigin);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, FONMAN,
				     ffcr->FOnMan);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, FONTRIG,
				     ffcr->FOnTrig);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, FONFLIN,
				     ffcr->FOnFlIn);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, ENFCONT,
				     ffcr->EnFCont);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_FFCR, ENFTC,
				     ffcr->EnFTC);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_it_misc_op0
*
* Description: Integ. Test Misc. Output 0
*
**************************************************************************
*/
cBool chal_trace_etb_set_it_misc_op0(CHAL_HANDLE handle, cBool full,
				     cBool acq_comp)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_ITMISCOP0, FULL,
				     full);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_ITMISCOP0,
				     ACQCOMP, acq_comp);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_it_tr_fl_in_ack
*
* Description: Integ. Test Trigger In and Flush In Ack
*
**************************************************************************
*/
cBool chal_trace_etb_set_it_tr_fl_in_ack(CHAL_HANDLE handle, cBool trig,
					 cBool flush)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_ITTRFLINACK,
				     FLUSHINACK, flush);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_ITTRFLINACK,
				     TRIGINACK, trig);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_it_tr_fl_in
*
* Description: Integ. Test Trigger In and Flush In
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_it_tr_fl_in(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_ITTRFLIN);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_it_atb_data0
*
* Description: Integ. Test ATB Data 0
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_it_atb_data0(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_ITATBDATA0);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_it_atb_ctrl2
*
* Description: Integ. Test ATB Control 2
*
**************************************************************************
*/
cBool chal_trace_etb_set_it_atb_ctrl2(CHAL_HANDLE handle, cBool afvalids,
				      cBool atreadys)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_ITATBCTR2,
				     AFVALIDS, afvalids);
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_ITATBCTR2,
				     ATREADYS, atreadys);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_it_atb_ctrl1
*
* Description: Integ. Test ATB Control 1
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_it_atb_ctrl1(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_ITATBCTR1);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_it_atb_ctrl0
*
* Description: Integ. Test ATB Control 0
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_it_atb_ctrl0(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_ITATBCTR0);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_int_mode_control
*
* Description: Integragtion mode enable.
*
**************************************************************************
*/
cBool chal_trace_etb_set_int_mode_control(CHAL_HANDLE handle, cBool enable)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_ICTRL, INTEG_EN,
				     enable);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_claim_set
*
* Description: Claim Tag Set
*
***************************************************************************
*/
cBool chal_trace_etb_set_claim_set(CHAL_HANDLE handle, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_CLAIMSET,
				     CLAIM_TAGS_SET, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_claim_clr
*
* Description: Claim Tag Clear
*
**************************************************************************
*/
cBool chal_trace_etb_set_claim_clr(CHAL_HANDLE handle, cUInt8 channel)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_CLAIMCLR,
				     CLAIM_TAGS_CLR, channel);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_set_lock_access
*
* Description: Lock Access
*
***************************************************************************
*/
cBool chal_trace_etb_set_lock_access(CHAL_HANDLE handle, cUInt32 control)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB_base, ETB_LOCKACCESS,
				     LOCK_ACCESS, control);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_lock_status
*
* Description: Lock Status
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_lock_status(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_LOCKSTATUS);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_auth_status
*
* Description: Authentication Status
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_auth_status(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_AUTHSTATUS);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_dev_id
*
* Description: Device ID
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_dev_id(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_DEVID);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_dev_type
*
* Description: Device Type
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_dev_type(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base)
		status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_DEVTYPE);

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_per_id
*
* Description: Peripheral ID n
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_per_id(CHAL_HANDLE handle, cUInt8 n_peripheral)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base) {
		switch (n_peripheral) {
		case 0:
			status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_PERID0);
			break;

		case 1:
			status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_PERID1);
			break;

		case 2:
			status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_PERID2);
			break;

		case 3:
			status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_PERID3);
			break;

		case 4:
			status = BRCM_READ_REG(pTraceDev->ETB_base, ETB_PERID4);
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb_get_comp_id
*
* Description: Component ID n
*
**************************************************************************
*/
cUInt32 chal_trace_etb_get_comp_id(CHAL_HANDLE handle, cUInt8 n_component)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB_base) {
		switch (n_component) {
		case 0:
			status =
			    BRCM_READ_REG(pTraceDev->ETB_base, ETB_COMPID0);
			break;

		case 1:
			status =
			    BRCM_READ_REG(pTraceDev->ETB_base, ETB_COMPID1);
			break;

		case 2:
			status =
			    BRCM_READ_REG(pTraceDev->ETB_base, ETB_COMPID2);
			break;

		case 3:
			status =
			    BRCM_READ_REG(pTraceDev->ETB_base, ETB_COMPID3);
			break;

		default:
			break;
		}
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb2axi_set_config
*
* Description: ETB to AXI Configuration
*
**************************************************************************
*/
cBool chal_trace_etb2axi_set_config(CHAL_HANDLE handle, cBool flush)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB2AXI_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB2AXI_base, ETB2AXI_CONFIG,
				     FLUSH, flush);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_etb2axi_set_wr_ptr
*
* Description: ETB to AXI Configuration
*
**************************************************************************
*/
cBool chal_trace_etb2axi_set_wr_ptr(CHAL_HANDLE handle, cUInt32 wr_ptr)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ETB2AXI_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ETB2AXI_base, ETB2AXI_WRPTR,
				     AXI_WRPTR, wr_ptr);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_cti_get_lock_status
*
* Description: Device Type
*
**************************************************************************
*/
cUInt32 chal_trace_etb2axi_get_status(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->ETB2AXI_base)
		status = BRCM_READ_REG(pTraceDev->ETB2AXI_base, ETB2AXI_STATUS);

	return status;
}

#if !defined(_SAMOA_)
/**************************************************************************
*
* Function Name: chal_trace_globperf_set_config
*
* Description: Set GLOBPERF_GLB_CONFIG - Counter Config
*
***************************************************************************
*/
cBool chal_trace_globperf_set_config(CHAL_HANDLE handle, cBool counter_stop_en,
				     cUInt32 timeout)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->GLOBPERF_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->GLOBPERF_base,
				     GLOBPERF_GLB_CONFIG, COUNTER_STOP_EN,
				     counter_stop_en);
		BRCM_WRITE_REG_FIELD(pTraceDev->GLOBPERF_base,
				     GLOBPERF_GLB_CONFIG, TIMEOUT, timeout);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_globperf_set_cmd
*
* Description: Set GLOBPERF_GLB_CMD - Global Command
*
**************************************************************************
*/
cBool chal_trace_globperf_set_cmd(CHAL_HANDLE handle, CHAL_TRACE_GLB_CMD_t cmd)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->GLOBPERF_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->GLOBPERF_base, GLOBPERF_GLB_CMD,
				     GLOBAL_COMMAND, cmd);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_globperf_get_status
*
* Description: Return GLOBPERF_GLB_STATUS - Request Status
*
**************************************************************************
*/
cUInt32 chal_trace_globperf_get_status(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->GLOBPERF_base) {
		status =
		    BRCM_READ_REG(pTraceDev->GLOBPERF_base,
				  GLOBPERF_GLB_STATUS);
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_globperf_get_count
*
* Description: Return GLOBPERF_GLB_COUNT - Counter Status
*
***************************************************************************
*/
cUInt32 chal_trace_globperf_get_count(CHAL_HANDLE handle)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 status = 0;

	if (pTraceDev->GLOBPERF_base) {
		status =
		    BRCM_READ_REG(pTraceDev->GLOBPERF_base, GLOBPERF_GLB_COUNT);
	}

	return status;
}
#endif /* !defined(_SAMOA_) */

#if !defined(_HERA_)
/**************************************************************************
*
* Function Name: chal_trace_atb_stm_set_config
*
* Description: Set ATB_STM Config
*
**************************************************************************
*/
cBool chal_trace_atb_stm_set_config(CHAL_HANDLE handle, cBool twobit_mode,
				    cUInt8 break_limit, cUInt8 output_mode,
				    cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ATB_STM_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_CONFIG,
				     TWOBIT_MODE, twobit_mode);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_CONFIG,
				     BREAK_LIMIT, break_limit);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_CONFIG,
				     OUTPUT_MODE, output_mode);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_CONFIG,
				     ATB_OUT_ID, atb_id);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_atb_stm_set_en
*
* Description: Set STM_EN_LO/HI
*
**************************************************************************
*/
cBool chal_trace_atb_stm_set_en(CHAL_HANDLE handle, cUInt32 low_half,
				cUInt32 high_half)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ATB_STM_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_EN_LO,
				     MASTER_EN_LO, low_half);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_EN_HI,
				     MASTER_EN_HI, high_half);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_atb_stm_get_en
*
* Description: Return STM_EN_LO/HI
*
**************************************************************************
*/
cUInt32 chal_trace_atb_stm_get_en(CHAL_HANDLE handle, cBool high)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 reg_value = 0;

	if (pTraceDev->ATB_STM_base) {
		if (high) {
			reg_value =
			    BRCM_READ_REG_FIELD(pTraceDev->ATB_STM_base,
						ATB_STM_EN_HI, MASTER_EN_HI);
		} else {
			reg_value =
			    BRCM_READ_REG_FIELD(pTraceDev->ATB_STM_base,
						ATB_STM_EN_LO, MASTER_EN_LO);
		}
	}

	return reg_value;
}

/**************************************************************************
*
* Function Name: chal_trace_atb_stm_set_sw
*
* Description: Set STM_SW_LO/HI
*
**************************************************************************
*/
cBool chal_trace_atb_stm_set_sw(CHAL_HANDLE handle, cUInt32 low_half,
				cUInt32 high_half)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->ATB_STM_base) {
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_SW_LO,
				     MASTER_SW_LO, low_half);
		BRCM_WRITE_REG_FIELD(pTraceDev->ATB_STM_base, ATB_STM_SW_HI,
				     MASTER_SW_HI, high_half);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_atb_stm_get_sw
*
* Description: Return STM_SW_LO/HI
*
***************************************************************************
*/
cUInt32 chal_trace_atb_stm_get_sw(CHAL_HANDLE handle, cBool high)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cUInt32 reg_value = 0;

	if (pTraceDev->ATB_STM_base) {
		if (high) {
			reg_value =
			    BRCM_READ_REG_FIELD(pTraceDev->ATB_STM_base,
						ATB_STM_SW_HI, MASTER_SW_HI);
		} else {
			reg_value =
			    BRCM_READ_REG_FIELD(pTraceDev->ATB_STM_base,
						ATB_STM_SW_LO, MASTER_SW_LO);
		}
	}

	return reg_value;
}

/**************************************************************************
*
* Function Name: chal_trace_sw_stm_set_config
*
* Description: Set SWSTM Config
*
**************************************************************************
*/
cBool chal_trace_sw_stm_set_config(CHAL_HANDLE handle,
				   CHAL_TRACE_SWSTM_t swstm_st,
				   cBool stall_mode, cUInt8 atb_id)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->SW_STM_base[swstm_st]) {
		BRCM_WRITE_REG_FIELD(pTraceDev->SW_STM_base[swstm_st],
				     SWSTM_R_CONFIG, STALL_MODE, stall_mode);
		BRCM_WRITE_REG_FIELD(pTraceDev->SW_STM_base[swstm_st],
				     SWSTM_R_CONFIG, ATB_ID, atb_id);
		status = TRUE;
	}

	return status;
}

/**************************************************************************
*
* Function Name: chal_trace_sw_stm_write
*
* Description: Creates x-byte value with Channel y.
*
**************************************************************************
*/
cBool chal_trace_sw_stm_write(CHAL_HANDLE handle, CHAL_TRACE_SWSTM_t swstm_st,
			      cUInt8 n_channel, cUInt8 n_bytes, cUInt32 value)
{
	CHAL_TRACE_DEV_t *pTraceDev = (CHAL_TRACE_DEV_t *) handle;
	cBool status = FALSE;

	if (pTraceDev->SW_STM_base[swstm_st]) {
		switch (n_bytes) {
			/* Use BRCM_WRITE_REG_IDX even though the
			* registers are not indexed registers. */
		case 1:
			BRCM_WRITE_REG_IDX(pTraceDev->SW_STM_base[swstm_st],
					   SWSTM_R_VAL_1BYTE_CHAN00, n_channel,
					   value);
			status = TRUE;
			break;

		case 2:
			BRCM_WRITE_REG_IDX(pTraceDev->SW_STM_base[swstm_st],
					   SWSTM_R_VAL_2BYTE_CHAN00, n_channel,
					   value);
			status = TRUE;
			break;

		case 4:
			BRCM_WRITE_REG_IDX(pTraceDev->SW_STM_base[swstm_st],
					   SWSTM_R_VAL_4BYTE_CHAN00, n_channel,
					   value);
			status = TRUE;
			break;

		default:
			break;

		}
	}

	return status;
}
#endif /* !defined(_HERA_) */

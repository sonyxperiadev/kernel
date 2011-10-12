/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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

/*
 * ===========================================================================
 *  inlcude header file declarations
 */

#include <plat/chal/chal_common.h>
#include <chal/chal_defs.h>
#include <chal/chal_ipc.h>
#include <linux/string.h>
#include <mach/io_map.h>
//#include <mach/csp/mm_io.h>
#include <mach/rdb/brcm_rdb_ipcopen.h>
#include <mach/rdb/brcm_rdb_ipcsec.h>

#define BCM_DBG_ENTER()
#define BCM_DBG_EXIT()

/*
 * ===========================================================================
 *  global variable declarations
 *
 */

/*
 * ===========================================================================
 *  extern variable declarations
 *
 */

/*
 * ===========================================================================
 *  static function prototype declarations
 *
 */

/*
 * ===========================================================================
 *  local macro declarations
 *
 */
#if 1
#define IPC_DBG_OUT(a) {BCM_DBG_OUT(a);}
#else
#define IPC_DBG_OUT(a) {}
#endif


typedef struct CHAL_IPC_DEV_T
{
    uint32_t             ipc_open_reg_base;
    uint32_t             ipc_secure_reg_base;
    CHAL_IPC_CONFIG_T   ipc_config;

} CHAL_IPC_DEV_T;


/*
 * ===========================================================================
 *  static variables declarations
 *
 */

static CHAL_IPC_DEV_T ipcDevice;

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_config
 *
 *  Description:
 *
 * ******************************************************************************
 */
CHAL_IPC_HANDLE chal_ipc_config (
    CHAL_IPC_CONFIG_T *pConfig
    )
{
   CHAL_UNUSED( pConfig );

   BCM_DBG_ENTER();

   ipcDevice.ipc_open_reg_base = KONA_IPC_NS_VA;
   ipcDevice.ipc_secure_reg_base = KONA_IPC_S_VA;

   BCM_DBG_EXIT();
   return( (CHAL_IPC_HANDLE)&ipcDevice );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_write_mailbox
 *
 *  Description:
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_write_mailbox (
    CHAL_IPC_HANDLE handle,
    IPC_MAILBOX_ID mailboxId,
    uint32_t value
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   /* Check boundry conditions
    * IPC_MAILBOX_ID is an unsigned value and does not need to check less than zero */
   if ( mailboxId >= IPC_MAILBOX_ID_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   device = (CHAL_IPC_DEV_T*)handle;

   CHAL_REG_WRITE32( device->ipc_secure_reg_base + IPCSEC_IPCMAIL0_OFFSET + mailboxId*sizeof(uint32_t), value );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_read_mailbox
 *
 *  Description:
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_read_mailbox (
    CHAL_IPC_HANDLE handle,
    IPC_MAILBOX_ID mailboxId,
    uint32_t *value
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   /* Check boundry conditions
    * IPC_MAILBOX_ID is an unsigned value and does not need to check less than zero */
   if ( mailboxId >= IPC_MAILBOX_ID_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   device = (CHAL_IPC_DEV_T*)handle;

   *value = CHAL_REG_READ32( device->ipc_secure_reg_base + IPCSEC_IPCMAIL0_OFFSET + mailboxId*sizeof(uint32_t) );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}


/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_query_wakeup_vc
 *
 *  Description: Queries and returns the value of the wakeup register.
 *
 * ******************************************************************************
 */

BCM_ERR_CODE chal_ipc_query_wakeup_vc (
    CHAL_IPC_HANDLE handle,
    uint32_t *result
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   *result = CHAL_REG_READ32( device->ipc_secure_reg_base + IPCSEC_IPCAWAKE_OFFSET );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_wakeup_vc
 *
 *  Description: Videocore wakeup
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_wakeup_vc (
    CHAL_IPC_HANDLE handle,
    uint32_t address
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   CHAL_REG_WRITE32(device->ipc_secure_reg_base + IPCSEC_IPCAWAKE_OFFSET,
      address | IPCSEC_IPCAWAKE_WAKEUP_MASK );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_sleep_vc
 *
 *  Description: VideoCore sleep
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_sleep_vc (
    CHAL_IPC_HANDLE handle
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   CHAL_REG_WRITE32( device->ipc_secure_reg_base + IPCSEC_IPCAWAKE_OFFSET,
      CHAL_REG_READ32( device->ipc_secure_reg_base + IPCSEC_IPCAWAKE_OFFSET ) &
      ~IPCSEC_IPCAWAKE_WAKEUP_MASK );

   return BCM_SUCCESS;
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_int_vcset
 *
 *  Description: Videocore interrupt set
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_int_vcset (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   if ( irqNum >= IPC_INTERRUPT_SOURCE_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   /* Note: since IPCOPEN_IPCASET_OFFSET is a write-only register it is not
    * valid to read the register then OR the appropriate bit and then write the
    * new value.  Simply write the appropriate bit to set the interrupt */
   CHAL_REG_WRITE32(device->ipc_open_reg_base + IPCOPEN_IPCASET_OFFSET, 1 << irqNum );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_int_clr
 *
 *  Description: Clear ARM interrupt
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_int_clr (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   if ( irqNum >= IPC_INTERRUPT_SOURCE_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   /* Note: since IPCOPEN_IPCACLR_OFFSET is a write-only register it is not
    * valid to read the register then OR the appropriate bit and then write the
    * new value.  Simply write the appropriate bit to clear the interrupt */
   CHAL_REG_WRITE32(device->ipc_open_reg_base + IPCOPEN_IPCACLR_OFFSET, 1 << irqNum );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_int_mode
 *
 *  Description: Set ARM interrupt to be secure or open
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_int_secmode (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE irqNum,
    IPC_INTERRUPT_MODE intMode
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   if ( irqNum >= IPC_INTERRUPT_SOURCE_MAX )
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   if ( intMode == IPC_INTERRUPT_MODE_OPEN )
   {
      CHAL_REG_CLRBIT32(device->ipc_secure_reg_base + IPCSEC_IPCASECURE_OFFSET, 1 << irqNum );
   }
   else if ( intMode == IPC_INTERRUPT_MODE_SECURE )
   {
      CHAL_REG_SETBIT32(device->ipc_secure_reg_base + IPCSEC_IPCASECURE_OFFSET, 1 << irqNum );
   }
   else
   {
      BCM_DBG_EXIT();
      return( BCM_ERROR );
   }

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_get_int_status
 *
 *  Description: Get ARM interrupt status
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_get_int_status (
    CHAL_IPC_HANDLE handle,
    uint32_t *status
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   *status = CHAL_REG_READ32( device->ipc_open_reg_base + IPCOPEN_IPCASTATUS_OFFSET );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_get_int_source
 *
 *  Description: Get ARM interrupt source
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_get_int_source (
    CHAL_IPC_HANDLE handle,
    IPC_INTERRUPT_SOURCE *source
    )
{
   uint32_t status;
   IPC_INTERRUPT_SOURCE i = IPC_INTERRUPT_SOURCE_0;

   chal_ipc_get_int_status( handle, &status );

   for ( i = IPC_INTERRUPT_SOURCE_0; i < IPC_INTERRUPT_SOURCE_MAX; i++ )
   {
      if ( status & ( IPC_INTERRUPT_STATUS_ENABLED << i ) )
      {
         *source = i;
         return BCM_SUCCESS;
      }
   }

   *source = IPC_INTERRUPT_SOURCE_NULL;
   return BCM_ERROR;
}


/*
 * ******************************************************************************
 *
 *  Function Name:  chal_ipc_get_error_status
 *
 *  Description: Get ARM error status
 *
 * ******************************************************************************
 */
BCM_ERR_CODE chal_ipc_get_error_status (
    CHAL_IPC_HANDLE handle,
    uint32_t *status
    )
{
   CHAL_IPC_DEV_T *device;

   BCM_DBG_ENTER();

   device = (CHAL_IPC_DEV_T*)handle;

   *status = CHAL_REG_READ32(device->ipc_open_reg_base + IPCOPEN_IPCERR_OFFSET );

   BCM_DBG_EXIT();
   return( BCM_SUCCESS );
}

#if defined( __KERNEL__ )

#include <linux/module.h>

/* Export the following symbols so that the videocore driver can use them.*/

EXPORT_SYMBOL( chal_ipc_config );
EXPORT_SYMBOL( chal_ipc_query_wakeup_vc );
EXPORT_SYMBOL( chal_ipc_wakeup_vc );
EXPORT_SYMBOL( chal_ipc_sleep_vc );

EXPORT_SYMBOL( chal_ipc_write_mailbox );
EXPORT_SYMBOL( chal_ipc_read_mailbox );

EXPORT_SYMBOL( chal_ipc_int_secmode );
EXPORT_SYMBOL( chal_ipc_int_clr );
EXPORT_SYMBOL( chal_ipc_int_vcset );
EXPORT_SYMBOL( chal_ipc_get_int_status );
EXPORT_SYMBOL( chal_ipc_get_int_source );
#endif



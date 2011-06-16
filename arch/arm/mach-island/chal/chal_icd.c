/*****************************************************************************
* Copyright (c) 2003-2008 Broadcom Corporation.  All rights reserved.
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
#include <plat/chal/chal_types.h>
#include <chal/chal_icd.h>
#include <chal/chal_defs.h>
#include <chal/bcmregmem.h>
#include <mach/rdb/brcm_rdb_gicdist.h>
//#include <mach/csp/bcmdbg.h>
#include <mach/io_map.h>
#include <mach/irqs.h>

#define BCM_DBG_ENTER()
#define BCM_DBG_EXIT()

#define MM_IO_BASE_GICDIST KONA_GICDIST_VA

/* ---- External Function Prototypes ------------------------------------- */
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Function Prototypes -------------------------------------- */
static BCM_ERR_CODE chal_icd_trigger_software_interrupt
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_filter_t        eFilterType,   /* [in] target filter type */
    int_cpu_interface_t eCpuIfMask,    /* [in] target cpu bit mask */
    int_secure_state_t  eSecureState   /* [in] secure state */
);

static BCM_ERR_CODE chal_icd_set_bit ( chal_icd_handle     hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId, /* [in] Interrupt ID */
    uint32_t             ulReg    /* [in] register offset */
);

/* ---- Private Variables ------------------------------------------------ */
/* ==== Public Functions ================================================= */

/*******************************************************************************
*
*   Name: chal_icd_get_info()
*
*   Purpose:
*       Returns some basic information about the ICD, such as the number of
*       lockable SPIs, security domains, CPUs and interrupts that this ICD
*       supports.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       None.
*
*******************************************************************************/
#if 0
BCM_ERR_CODE chal_icd_get_info
(
    chal_icd_handle     hIcd,         /* [in] Interrupt Controller Dist Handle */
    uint32_t            *num_lspi,    /* [out] Number of Lockable SPI */
    uint32_t            *num_sd,      /* [out] Number of security domains */
    uint32_t            *num_cpus,    /* [out] Number of Cortex-A9 processors */
    uint32_t            *num_it       /* [out] Number of IT (interrupt lines) */
)
{
    uint32_t    ulAddr;
    uint32_t    ulData;

    CHAL_UNUSED( hIcd );

    ulAddr = MM_IO_BASE_GICDIST + GICDIST_IC_TYPE_REG_OFFSET;
    ulData = BCM_REG_READ32(NULL, ulAddr);

    *num_lspi = (ulData & GICDIST_IC_TYPE_REG_LSPI_MASK) >> GICDIST_IC_TYPE_REG_LSPI_SHIFT;
    // Number of security domains is equal to the value plus 1
    *num_sd   = ((ulData & GICDIST_IC_TYPE_REG_TZ_MASK) >> GICDIST_IC_TYPE_REG_TZ_SHIFT) + 1;
    // Number of CPUs is equal to the entereed value plus 1
    *num_cpus = ((ulData & GICDIST_IC_TYPE_REG_CPU_NUMBER_MASK) >> GICDIST_IC_TYPE_REG_CPU_NUMBER_SHIFT) + 1;
    // Number of interrupts is (value + 1) * 32. The +1 is for the 32 built-in interrupts (value is number of externals)
    *num_it   = (((ulData & GICDIST_IC_TYPE_REG_IT_LINES_NUMBER_MASK) >> GICDIST_IC_TYPE_REG_IT_LINES_NUMBER_SHIFT) + 1) * 32;

    return (BCM_SUCCESS);
}
#endif

/*******************************************************************************
*
*   Name: chal_icd_disable()
*
*   Purpose:
*       Calling this function will disable the GIC.  Basially,
*       ignores any external stimulus change that occurs on the corresponding SPI
*       or PPI_C signals.  Does not forward STIs, PPIs, or SPIs to the CPU 
*       Interfaces that are in the Pending state.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_enable()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_disable
(
    chal_icd_handle hIcd   /* [in] Interrupt Controller Dist Handle */
)
{
    CHAL_UNUSED( hIcd );
    
    BCM_DBG_ENTER();

    /*-----------------------------------------------------------------------
     * Does not forward pending interrupt to CPU interfaces.
     * The GICDIST1_enable_s register is BANKED.  One for secure and one for
     * open world.
     *-----------------------------------------------------------------------*/
    BCM_REG_WRITE32( NULL, MM_IO_BASE_GICDIST + GICDIST_ENABLE_S_OFFSET, 0 );
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_disable */


/*******************************************************************************
*
*   Name: chal_icd_enable()
*
*   Purpose:
*       Forward STIs, PPIs, or SPIs to the CPU Interfaces that are in the Pending state.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_disable()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_enable
(
    chal_icd_handle hIcd   /* [in] Interrupt Controller Dist Handle */
)
{
    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    /*-----------------------------------------------------------------------
     * Forward pending interrupt to CPU interfaces.
     * The GICDIST1_enable_s register is BANKED.  One for secure and one for
     * open world.
     *-----------------------------------------------------------------------*/
    BCM_REG_WRITE32( NULL, MM_IO_BASE_GICDIST + GICDIST_ENABLE_S_OFFSET, 1 );    
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_enable */


/*******************************************************************************
*
*   Name: chal_icd_get_security()
*
*   Purpose:
*       Gets the security state of an interrupt,either secure or non-secure.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_set_security()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_get_security
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_secure_state_t  *peSecureState /* [out] secure state */
)
{
    uint32_t   ulRegNum = INT_ID_GET_REG_NUM( ulIntId );
    uint32_t   ulBitNum = INT_ID_GET_BIT_NUM( ulIntId );
    uint32_t   ulAddr;        

    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    ulAddr   = MM_IO_BASE_GICDIST + GICDIST_INT_SECURITY0_OFFSET + (ulRegNum * 4);     
    *peSecureState = (BCM_REG_READ32( NULL, ulAddr) & (1 << ulBitNum)) 
                        ? eINT_STATE_NONSECURE 
                        : eINT_STATE_SECURE;
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_get_security */


/*******************************************************************************
*
*   Name: chal_icd_set_enable()
*
*   Purpose:
*       Allows a pending interrupt to be forwarded to the CPU interface.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_clear_enable()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_set_enable
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
)
{    
    return( chal_icd_set_bit( hIcd, ulIntId, MM_IO_BASE_GICDIST + GICDIST_ENABLE_SET0_OFFSET ) );
} /* chal_icd_set_enable */


/*******************************************************************************
*
*   Name: chal_icd_clear_enable()
*
*   Purpose:
*       Disallows a pending interrupt to be forwarded to the CPU interface.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_set_enable()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_clear_enable
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
)
{
    return( chal_icd_set_bit( hIcd, ulIntId, MM_IO_BASE_GICDIST + GICDIST_ENABLE_CLR0_OFFSET ) );
} /* chal_icd_clear_enable */


/*******************************************************************************
*
*   Name: chal_icd_get_enable()
*
*   Purpose:
*       Get the status of the enable interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_set_enable()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_get_enable
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulMask /* [out] 1 == on, 0 == off */
)
{
    uint32_t   ulRegNum = INT_ID_GET_REG_NUM( ulIntId );
    uint32_t   ulBitNum = INT_ID_GET_BIT_NUM( ulIntId );
    uint32_t   ulAddr;        

    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    ulAddr   = MM_IO_BASE_GICDIST + GICDIST_ENABLE_SET0_OFFSET + (ulRegNum * 4);    
    *pulMask = (BCM_REG_READ32( NULL, ulAddr) & (1 << ulBitNum)) ? 1 : 0;
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_get_enable */


/*******************************************************************************
*
*   Name: chal_icd_set_pending()
*
*   Purpose:
*       should be used for testing only.  use this to trigger interrupt
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_get_pending()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_set_pending
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
)
{  
    return( chal_icd_set_bit( hIcd, ulIntId, MM_IO_BASE_GICDIST + GICDIST_PENDING_SET0_OFFSET ) );
} /* chal_icd_set_pending */


/*******************************************************************************
*
*   Name: chal_icd_get_pending()
*
*   Purpose:
*       Gets the pending status of an interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_set_pending()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_get_pending
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulStat /* [out] 1 == on, 0 == off */
)
{
    uint32_t    ulRegNum = INT_ID_GET_REG_NUM( ulIntId );
    uint32_t    ulBitNum = INT_ID_GET_BIT_NUM( ulIntId );
    uint32_t    ulAddr;        

    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    ulAddr   = MM_IO_BASE_GICDIST + GICDIST_PENDING_SET0_OFFSET + (ulRegNum * 4);    
    *pulStat = (BCM_REG_READ32( NULL, ulAddr) & (1 << ulBitNum)) ? 1 : 0;
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_get_pending */


/*******************************************************************************
*
*   Name: chal_icd_get_pending()
*
*   Purpose:
*       Clears the pending interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_set_pending()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_clear_pending
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId /* [in] Interrupt ID */
)
{
    return( chal_icd_set_bit( hIcd, ulIntId, MM_IO_BASE_GICDIST + GICDIST_PENDING_CLR0_OFFSET ) );
} /* chal_icd_clear_pending */


/*******************************************************************************
*
*   Name: chal_icd_get_active_status()
*
*   Purpose:
*       Gets the active status of an interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_get_active_status
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulStat /* [out] 1 == on, 0 == off */
)
{
    uint32_t   ulRegNum = INT_ID_GET_REG_NUM( ulIntId );
    uint32_t   ulBitNum = INT_ID_GET_BIT_NUM( ulIntId );
    uint32_t   ulAddr;        

    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    ulAddr   = MM_IO_BASE_GICDIST + GICDIST_ACTIVE_STATUS0_OFFSET + (ulRegNum * 4);    
    *pulStat = (BCM_REG_READ32( NULL, ulAddr) & (1 << ulBitNum)) ? 1 : 0;
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_get_active_status */


/*******************************************************************************
*
*   Name: chal_icd_set_priority_level()
*
*   Purpose:
*       Sets the priority level of an interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_get_priority_level()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_set_priority_level
(
    chal_icd_handle hIcd,      /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,   /* [in] Interrupt ID */
    int_priority_t  ePriority  /* [in] priority level */
)
{
    uint32_t     ulRegNum = ulIntId >> 2; /* divide by 4 */
    uint32_t     ulBitNum = ((ulIntId & 3) * 8);  /* get the bit location */
    uint32_t     ulAddr;
    uint32_t     ulVal;
    uint32_t     ulPriority;
    
    CHAL_UNUSED( hIcd );    
    BCM_DBG_ENTER();
    ulPriority = (uint32_t) ePriority;

    if( (ulPriority >= BCM_INT_PRIORITY_MAX) || (ulIntId >= BCM_INT_ID_MAX) )
        return( BCM_ERROR );

    ulAddr = MM_IO_BASE_GICDIST + GICDIST_PRIORITY_LEVEL0_OFFSET + (ulRegNum * 4);

    /*-----------------------------------------------------------------------
     * WARNING:
     * the priority is from 0 - 31; however, GIC ignores the lower
     * 3-bits and used the upper 5-bits to represent the priority
     * level.  Need to shift the priority level 3 bits.
     *-----------------------------------------------------------------------*/
    ulPriority <<= 3;
    
    /* WARNING: the read/modify/write might need to be atomic */
    /*
     *  Don't need CS this cause it is done in init state
     * BCM_ENTER_CRITICAL_SECTION();
     */
    ulVal  = BCM_REG_READ32( NULL, ulAddr );
    ulVal &= ~(0xFF << ulBitNum);
    ulVal |= ulPriority << ulBitNum;
    BCM_REG_WRITE32( NULL, ulAddr, ulVal );
    /*
     *  Don't need CS this cause it is done in init state
     * BCM_EXIT_CRITICAL_SECTION();
     */
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_set_priority_level */


/*******************************************************************************
*
*   Name: chal_icd_get_priority_level()
*
*   Purpose:
*       Gets the priority level of an interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_set_priority_level()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_get_priority_level
(
    chal_icd_handle hIcd,       /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,    /* [in] Interrupt ID */
    int_priority_t  *pePriority /* [out] Priority Level */
)
{
    uint32_t     ulRegNum = ulIntId >> 2; /* divide by 4 */
    uint32_t     ulBitNum = ((ulIntId & 3) * 8);  /* get the bit location */
    uint32_t     ulAddr;
    uint32_t     ulPriority;
    
    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( (ulIntId >= BCM_INT_ID_MAX) )
        return( BCM_ERROR );

    ulAddr = MM_IO_BASE_GICDIST + GICDIST_PRIORITY_LEVEL0_OFFSET + (ulRegNum * 4);

    ulPriority = (BCM_REG_READ32( NULL, ulAddr) >> ulBitNum) & ~(BCM_INT_PRIORITY_MAX-1);
    *pePriority = (int_priority_t) ulPriority;
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_get_priority_level */


/*******************************************************************************
*
*   Name: chal_icd_set_trigger_type()
*
*   Purpose:
*       Sets the tigger type of an interrupt
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_get_trigger_type()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_set_trigger_type
(
    chal_icd_handle hIcd,        /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,     /* [in] Interrupt ID */
    int_trigger_t   eTriggerType /* [in] trigger type */
)
{
    uint32_t     ulRegNum = ulIntId >> 4; /* divided by 16 */
    uint32_t     ulBitNum = ((ulIntId & 0xF) * 2);  /* get the bit location */
    uint32_t     ulAddr;
    uint32_t     ulVal;
    
    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    /*-----------------------------------------------------------------------
     *  The Distributor does not provide a register for:
     *  INTID [15:0] because SGIs are always pulse-sensitive and use the N-N 
     *  software model. The Distributor returns b10, for each bit-pair, when 
     *  address 0xC00 is read.
     *  INTID [31:16] because PPIs are configured during the GIC configuration
     *  process. The Distributor returns either b01 or b11, for each bit-pair, 
     *  when address 0xC04 is read.
     *-----------------------------------------------------------------------*/
    if( ulIntId < BCM_INT_ID_PPI_MAX )
    {
        printk( "setting trigger type for INTID == %d has no affect\n", ulIntId );
    }
    
    ulAddr = MM_IO_BASE_GICDIST + GICDIST_INT_CONFIG0_OFFSET + (ulRegNum * 4);
    
    /* WARNING: the read/modify/write might need to be atomic */
    /*
     *  Don't need CS this cause it is done in init state
     * BCM_ENTER_CRITICAL_SECTION();
     */
    
    ulVal  = BCM_REG_READ32( NULL, ulAddr );
    if( eINT_TRIGGER_LEVEL == eTriggerType )
    {
        ulVal &= ~(1 << (ulBitNum + 1));
    }
    else
    {
        ulVal |= 1 << (ulBitNum + 1);
    }
    BCM_REG_WRITE32( NULL, ulAddr, ulVal );
    
    /*
     *  Don't need CS this cause it is done in init state
     * BCM_EXIT_CRITICAL_SECTION();
     */
    
    BCM_DBG_EXIT();    
    return( BCM_SUCCESS );
} /* chal_icd_set_trigger_type */


/*******************************************************************************
*
*   Name: chal_icd_get_trigger_type()
*
*   Purpose:
*       Gets the tigger type of an interrupt
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_set_trigger_type()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_get_trigger_type
(
    chal_icd_handle hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId,       /* [in] Interrupt ID */
    int_trigger_t   *peTriggerType /* [out] trigger type */
)
{
    uint32_t     ulRegNum = ulIntId >> 4; /* divided by 16 */
    uint32_t     ulBitNum = ((ulIntId & 0xF) * 2);  /* get the bit location */
    uint32_t     ulAddr;
    uint32_t     ulVal;
    
    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    /*-----------------------------------------------------------------------
     *  The Distributor does not provide a register for:
     *  INTID [15:0] because SGIs are always pulse-sensitive and use the N-N 
     *  software model. The Distributor returns b10, for each bit-pair, when 
     *  address 0xC00 is read.
     *  INTID [31:16] because PPIs are configured during the GIC configuration
     *  process. The Distributor returns either b01 or b11, for each bit-pair, 
     *  when address 0xC04 is read.
     *-----------------------------------------------------------------------*/
    if( ulIntId < BCM_INT_ID_PPI_MAX )
    {
        printk("trigger type does not change for INTID == %d\n", ulIntId);
    }

    ulAddr = MM_IO_BASE_GICDIST + GICDIST_INT_CONFIG0_OFFSET + (ulRegNum * 4);
    
    ulVal  = BCM_REG_READ32( NULL, ulAddr );
    ulVal &= 1 << (ulBitNum + 1); /* only want trigger bit */
    
    if( ulVal == 0 )
    {
        *peTriggerType = eINT_TRIGGER_LEVEL;
    }
    else
    {
        *peTriggerType = eINT_TRIGGER_PULSE;
    }
    
    BCM_DBG_EXIT();    
    return( BCM_SUCCESS );
} /* chal_icd_get_trigger_type */


/*******************************************************************************
*
*   Name: chal_icd_get_signal_status()
*
*   Purpose:
*       Gets the actual signal status of an interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_get_signal_status
(
    chal_icd_handle hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         *pulStat /* [out] 1 == on, 0 == off */
)
{
    uint32_t   ulRegNum;
    uint32_t   ulBitNum;
    uint32_t   ulAddr;        

    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );
   
    if( ulIntId < BCM_INT_ID_STI_MAX )
    {
        /* TODO: */
        /* software generated (triggered) interrupt */
        //BCM_DBG_OUT(( "TO BE IMPLEMENTED\n" ));
        *pulStat = 0;
    }
    else if( (ulIntId < BCM_INT_ID_PPI_MAX) )
    {
        /* WARNING: */
        //BCM_DBG_ERR(( "NOT SUPPORTED FOR MAUI\n" ));
        /* private peripheral interrupt */
        /* get into the correct range */
        ulIntId -= BCM_INT_ID_STI_MAX;
        /*-------------------------------------------------------------------
         * Usage constraints: A CPU can only read the status of its PPI_C signals, 
         * and therefore cannot read the status of PPI_C signals for other CPUs.
         * Configurations: This register is only available when the GIC is 
         * configured to provide two or more CPU Interfaces.
         * If the GIC is configured to provide a single CPU Interface then the 
         * Distributor returns 0x00000000. 
         *-------------------------------------------------------------------*/
        ulAddr   = MM_IO_BASE_GICDIST + GICDIST_PPI_STATUS_OFFSET;
        *pulStat = (BCM_REG_READ32( NULL, ulAddr ) & (1 << ulIntId)) ? 1 : 0;
    }
    else /*if( ulIntId < BCM_INT_ID_MAX )*/
    {
        /* shared peripheral interrupt */
        /* get into the correct range */
        ulIntId -= BCM_INT_ID_PPI_MAX; 
        ulRegNum = INT_ID_GET_REG_NUM( ulIntId );
        ulBitNum = INT_ID_GET_BIT_NUM( ulIntId );

        ulAddr   = MM_IO_BASE_GICDIST + GICDIST_SPI_STATUS0_OFFSET + (ulRegNum * 4);
        *pulStat = (BCM_REG_READ32( NULL, ulAddr ) & (1 << ulBitNum)) ? 1 : 0;
    }
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_get_status */


/*******************************************************************************
*
*   Name: chal_icd_trigger_secure_software_interrupt()
*
*   Purpose:
*       Triggers secure software interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_trigger_nonsecure_software_interrupt()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_trigger_secure_software_interrupt
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_filter_t        eFilterType,   /* [in] target filter type */
    int_cpu_interface_t eCpuIfMask     /* [in] target cpu bit mask */
)
{
    BCM_ERR_CODE    bcmErr;
    
    bcmErr = chal_icd_trigger_software_interrupt
                ( hIcd, ulIntId, eFilterType, eCpuIfMask, eINT_STATE_SECURE );
    return( bcmErr );
} /* chal_icd_trigger_secure_software_interrupt */


/*******************************************************************************
*
*   Name: chal_icd_trigger_nonsecure_software_interrupt()
*
*   Purpose:
*       Triggers a nonsecure software interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_trigger_secure_software_interrupt()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_trigger_nonsecure_software_interrupt
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_filter_t        eFilterType,   /* [in] target filter type */
    int_cpu_interface_t eCpuIfMask     /* [in] target cpu bit mask */
)
{
    BCM_ERR_CODE    bcmErr;
    
    bcmErr = chal_icd_trigger_software_interrupt
                ( hIcd, ulIntId, eFilterType, eCpuIfMask, eINT_STATE_NONSECURE );
    return( bcmErr );
} /* chal_icd_trigger_nonsecure_software_interrupt */



/* ==== Private Functions ================================================= */
/*******************************************************************************
*
*   Name: chal_icd_trigger_software_interrupt()
*
*   Purpose:
*       Triggers a software interrupt.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*
*******************************************************************************/
static BCM_ERR_CODE chal_icd_trigger_software_interrupt
(
    chal_icd_handle     hIcd,          /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,       /* [in] Interrupt ID */
    int_filter_t        eFilterType,   /* [in] target filter type */
    int_cpu_interface_t eCpuIfMask,    /* [in] target cpu bit mask */
    int_secure_state_t  eSecureState   /* [in] secure state */
)
{
    uint32_t     ulState;     /* secure state */
    uint32_t     ulCpuIfList; /* cpu masks */
    uint32_t     ulFilter;    /* fitler type, level or edge */
    uint32_t     ulRegVal;
    
    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_STI_MAX ) 
        return( BCM_ERROR );

    ulIntId = ulIntId << GICDIST_STI_CONTROL_STI_INTID_SHIFT; 
    
    ulState = (eINT_STATE_SECURE == eSecureState) 
                ? 0 : 1 << GICDIST_STI_CONTROL_SATT_SHIFT;    
    
    switch( eFilterType )
    {
        case eINT_FILTER_SPECIFIC:   ulFilter = 0; break;
        case eINT_FILTER_NO_REQUEST: ulFilter = 1; break;
        case eINT_FILTER_REQUEST:    ulFilter = 2; break;
        case eINT_FILTER_RESEVERED:  ulFilter = 3; break;
        default: ulFilter = 0; break;
    }
    ulFilter = ulFilter << GICDIST_STI_CONTROL_TARGET_LIST_FILTER_SHIFT;
        
    ulCpuIfList = CHAL_ETOUI(eCpuIfMask) << GICDIST_STI_CONTROL_CPU_TARGET_LIST_SHIFT;

    ulRegVal = ulIntId | ulState | ulCpuIfList | ulFilter;

    BCM_REG_WRITE32( NULL, MM_IO_BASE_GICDIST + GICDIST_STI_CONTROL_OFFSET, ulRegVal );

    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_trigger_software_interrupt */


/*******************************************************************************
*
*   Name: chal_icd_set_bit()
*
*   Purpose:
*       Sets and interrupt bit.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*
*******************************************************************************/
static BCM_ERR_CODE chal_icd_set_bit
(
    chal_icd_handle     hIcd,    /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId, /* [in] Interrupt ID */
    uint32_t             ulReg    /* [in] register offset */
)
{
    uint32_t     ulRegNum = INT_ID_GET_REG_NUM( ulIntId );
    uint32_t     ulBitNum = INT_ID_GET_BIT_NUM( ulIntId );
    uint32_t     ulAddr;
    uint32_t     ulVal;

    CHAL_UNUSED( hIcd );
    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    /*-----------------------------------------------------------------------
     * The Distributor does not provide registers for INTIDs < 16 because STIs  
     * are always enabled. If an STI is present then the Distributor returns 1, 
     * otherwise it returns 0.
     *-----------------------------------------------------------------------*/
    if( ulIntId < BCM_INT_ID_STI_MAX )
    {
        printk( "Can't set or clear STI: INTID == %d\n", ulIntId );
        return( BCM_SUCCESS );        
    }

    ulAddr = ulReg + (ulRegNum * 4);
    ulVal = (1 << ulBitNum); /* Write 1 to enable, 0 has not effect */
    BCM_REG_WRITE32( NULL, ulAddr, ulVal );
       
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
}

/*=========================================================================*/
/*                          SECURE ACCESS ONLY                             */
/*                          SECURE ACCESS ONLY                             */
/*                          SECURE ACCESS ONLY                             */
/*                                                                         */
/*        The function(s) below could only be accessed in secure world.    */
/*=========================================================================*/
/*******************************************************************************
*
*   Name: chal_icd_set_security()
*
*   Purpose:
*       Sets the security state of an interrupt, to be either secure or non-secure.
*       This function could only be called in secure world.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_get_security()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_set_security
(
    chal_icd_handle     hIcd,        /* [in] Interrupt Controller Dist Handle */
    uint32_t             ulIntId,     /* [in] Interrupt ID */
    int_secure_state_t  eSecureState /* [in] secure state */
)
{
    uint32_t   ulRegNum = INT_ID_GET_REG_NUM( ulIntId );
    uint32_t   ulBitNum = INT_ID_GET_BIT_NUM( ulIntId );
    uint32_t   ulAddr;
    uint32_t   ulVal;

    CHAL_UNUSED( hIcd );

    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    ulAddr = MM_IO_BASE_GICDIST + GICDIST_INT_SECURITY0_OFFSET + (ulRegNum * 4);
    
    /* WARNING: the read/modify/write might need to be atomic */
    /*
     *  Don't need CS this cause it is done in init state
     *  BCM_ENTER_CRITICAL_SECTION();
     */
    ulVal  = BCM_REG_READ32( NULL, ulAddr );
    
    if( eSecureState == eINT_STATE_NONSECURE )
    {
        ulVal |= (1 << ulBitNum);
    }
    else
    {
        ulVal &= ~(1 << ulBitNum);
    }

    //BCM_DBG_MSG(( "0x%08x == 0x%08x\n", ulAddr, ulVal ));
    BCM_REG_WRITE32( NULL, ulAddr, ulVal );
    /*
     *  Don't need CS this cause it is done in init state
     * BCM_EXIT_CRITICAL_SECTION();
     */
    
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_set_security */

/*******************************************************************************
*
*   Name: chal_icd_set_spi_target()
*
*   Purpose:
*       set spi target associated with an INTID.
*
*   Returns:
*       BCM_SUCCESS if successful,
*       otherwise errors code.
*
*   See Also:
*       chal_icd_clear_spi_target()
*
*******************************************************************************/
BCM_ERR_CODE chal_icd_set_spi_target
(
    chal_icd_handle hIcd,   /* [in] Interrupt Controller Dist Handle */
    uint32_t         ulIntId, /* [in] Interrupt ID */
    uint32_t         ulTargets /* cpu target selections */
)
{    
    uint32_t     ulRegNum = ADDR_ICDIPTR( ulIntId );
    uint32_t     ulBitNum = ICDIPTR_SHIFT( ulIntId );
    uint32_t     ulAddr;
    uint32_t     ulVal;

    CHAL_UNUSED( hIcd );
    BCM_DBG_ENTER();

    if( ulIntId >= BCM_INT_ID_MAX )
        return( BCM_ERROR );

    /*-----------------------------------------------------------------------
     * The Distributor does not provide registers for INTIDs < 16 because STIs  
     * are always enabled. If an STI is present then the Distributor returns 1, 
     * otherwise it returns 0.
     *-----------------------------------------------------------------------*/
    if( ulIntId < BCM_INT_ID_STI_MAX )
    {
        //BCM_DBG_WRN(( "Can't set or clear STI: INTID == %d\n", ulIntId ));
        return( BCM_SUCCESS );        
    }

    ulAddr = ICDIPTR_REG + (ulRegNum * 4);
    ulVal  = BCM_REG_READ32( NULL, ulAddr );
    ulVal &= ~(ICDIPTR_MASK << ulBitNum);    /* clear the field */
    ulVal |= (ulTargets << ulBitNum);        /* set new value  */
    BCM_REG_WRITE32( NULL, ulAddr, ulVal );
       
    BCM_DBG_EXIT();
    return( BCM_SUCCESS );
} /* chal_icd_set_spi_target */

#if defined( __KERNEL__ )

#include <linux/module.h>

/* Export the following symbols so that the videocore and other drivers can use them.*/

EXPORT_SYMBOL( chal_icd_get_signal_status );
EXPORT_SYMBOL( chal_icd_set_security );
#endif

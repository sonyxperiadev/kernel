/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef CHAL_INT_H__
#define CHAL_INT_H__

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
 *  Use these macros to convert interrup id to register and bit offsets.
 **************************************************************************/
#define INT_ID_GET_REG_NUM(id)	((id) >> 5)
#define INT_ID_GET_BIT_NUM(id)	((id)&0x1F)

typedef enum
{
    eINT_STATE_SECURE    = 0,
    eINT_STATE_NONSECURE = 1
} int_secure_state_t;

typedef enum
{
    eCPU_INTERFACE_0 = 0x01,
    eCPU_INTERFACE_1 = 0x02,
    eCPU_INTERFACE_2 = 0x04,
    eCPU_INTERFACE_3 = 0x08,
    eCPU_INTERFACE_4 = 0x10,
    eCPU_INTERFACE_5 = 0x20,
    eCPU_INTERFACE_6 = 0x40,
    eCPU_INTERFACE_7 = 0x80
} int_cpu_interface_t;

typedef enum
{
    eINT_TRIGGER_LEVEL = 0,
    eINT_TRIGGER_PULSE = 1
} int_trigger_t;

typedef enum
{
    /* sends the interrupt to CPUs that the CPU Target List field specifies.
     * If the CPU Target Filter List is 0x00 then the Distributor
     * does not permit the interrupt that STI_INTID specifies, to become
     * Pending on any CPU.
     */
    eINT_FILTER_SPECIFIC   = 0,

    /* sends the interrupt to all CPUs except the CPU
     * that requested the interrupt
     */
    eINT_FILTER_NO_REQUEST = 1,

    /* sends the interrupt to the CPU that requested the interrupt */
    eINT_FILTER_REQUEST    = 2,

    /* Not used - Reserved bits must be written with 0 */
    eINT_FILTER_RESEVERED  = 3
} int_filter_t;

typedef enum
{
    eINT_PRIORITY_0 = 0,
    eINT_PRIORITY_1,
    eINT_PRIORITY_2,
    eINT_PRIORITY_3,
    eINT_PRIORITY_4,
    eINT_PRIORITY_5,
    eINT_PRIORITY_6,
    eINT_PRIORITY_7,
    eINT_PRIORITY_8,
    eINT_PRIORITY_9,
    eINT_PRIORITY_10,
    eINT_PRIORITY_11,
    eINT_PRIORITY_12,
    eINT_PRIORITY_13,
    eINT_PRIORITY_14,
    eINT_PRIORITY_15,
    eINT_PRIORITY_16,
    eINT_PRIORITY_17,
    eINT_PRIORITY_18,
    eINT_PRIORITY_19,
    eINT_PRIORITY_20,
    eINT_PRIORITY_21,
    eINT_PRIORITY_22,
    eINT_PRIORITY_23,
    eINT_PRIORITY_24,
    eINT_PRIORITY_25,
    eINT_PRIORITY_26,
    eINT_PRIORITY_27,
    eINT_PRIORITY_28,
    eINT_PRIORITY_29,
    eINT_PRIORITY_30,
    eINT_PRIORITY_31
} int_priority_t;



#ifdef __cplusplus
}
#endif

#endif /* CHAL_INT_H__ */


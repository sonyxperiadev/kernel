/***************************************************************************
 * *
 * * Copyright 2004 - 2009 Broadcom Corporation.  All rights reserved.
 * *
 * * Unless you and Broadcom execute a separate written software license
 * * agreement governing use of this software, this software is licensed to you
 * * under the terms of the GNU General Public License version 2, available at
 * * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 * *
 * * Notwithstanding the above, under no circumstances may you combine this
 * * software in any way with any other Broadcom software provided under a
 * * license other than the GPL, without Broadcom's express prior written
 * * consent.
 * *
 * ****************************************************************************/
/**
 * *  @file   chal_keypad.c
 * *
 * *  @brief  RHEA Keypad Controller cHAL source code file.
 * *
 * *  @note
 * *
 * ****************************************************************************/

#ifdef tempINTERFACE_OSDAL_KEYPAD

#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <mach/rdb/brcm_rdb_keypad.h>
#include <mach/rdb/brcm_rdb_util.h>
#include <plat/chal/chal_keypad.h>

/******************************************************************************/
/*                            T Y P E D E F S                                */
/******************************************************************************/

typedef struct {
	void __iomem *baseAddr;
	Boolean pullUpMode;
	cUInt32 imr0Val;
	cUInt32 imr1Val;
} CHAL_KEYPAD_DEVICE_t;

/******************************************************************************/
/*                            G L O B A L S                                  */
/******************************************************************************/

/******************************************************************************/
/*                            LOCALS                                         */
/******************************************************************************/
static CHAL_KEYPAD_DEVICE_t KeypadDev;

/******************************************************************************/
/*                            P R O T O T Y P E S                            */
/******************************************************************************/
static cUInt8 chal_keypad_calculate_pin_mask(cUInt8 pin);

/******************************************************************************/
/*                            F U N C T I O N S                              */
/******************************************************************************/

/******************************************************************************
 *
 * Function Name: bitCount()
 *
 * Description: takes in a 32-bit value and returns the number of bits set
 *     to '1' in that value. This method of counting ones was chosen because it
 *     is optimised for inputs that have a sparse number of ones.
 *
 * Parameters:          n               (in)    The 32-bit value to examine
 * Return:              cUInt8                  The numner of '1' bits in n.
 *
 *****************************************************************************/
static cUInt8 bitCount(cUInt32 n)
{
	cUInt8 count = 0;

	while (n) {		/* loop until we have cleared all '1' bits */
		count++;
		n &= (n - 1);	/* this sets the rightmost '1' bit to 0 */
	}

	return count;

}				/* bitCount() */

/******************************************************************************
 *
 * Function Name: bitNumber()
 *
 * Description: takes in a 32-bit value and returns the index of the
 * rightmost '1' bit. It uses a DeBruijn sequence to calculate the index
 * faster than iterating through the value to find it.
 *
 * Parameters:     val         (in)   The 32-bit value to examine.
 * Return:         cUInt8             The index of the rightmost '1' bit in n.
 *
 ******************************************************************************/
static cUInt8 bitNumber(cUInt32 val)
{
	static const cUInt8 MultiplyDeBruijnBitPosition[32] = {
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};

	/* The constant 0x077CB531 is a DeBruijn sequence that produces a
	 * unique pattern of bits in the high 5 bits for each possible bit
	 * position that it is multiplied against.
	 * The usage of (val & -val) isolates only the rightmost '1' bit from
	 * val. This ensures that the multiplier only has 1 bit set to '1'. */

	return MultiplyDeBruijnBitPosition[(cUInt8)
					   (((val & -val) *
					     0x077CB531U) >> 27)];

}	/* bitNumber() */

/******************************************************************************
 *
 * Function Name: chal_keypad_calculate_pin_mask()
 *
 * Description: creates a mask that has the first n bits set.
 *
 * Parameters:          n            (in)   The number of bits to set
 * Return:              cUInt8              A mask with the first n bits set.
 *
 *****************************************************************************/
cUInt8 chal_keypad_calculate_pin_mask(cUInt8 n)
{
	cUInt16 tempVal;

	if ((n > 8) || (n < 1))	/* n should never be bigger than 8
				as the hardware supports 8x8  */
		/* matrix maximum. */
	{
		return 0;
	}

	tempVal = (1 << n) - 1;

	return (cUInt8)tempVal;
}				/* chal_keypad_calculate_pin_maks() */

/******************************************************************************
 *
 * Function Name: chal_keypad_init.
 *
 * Description: initialize the keypad csl driver
 *
 * Parameters: baseAddr    (in)    hardware cfg. of keypad to initialize
 *
 * Note: When this function completes, the keypad hardware is configured
 *        as requested and key interrupts are enabled.
 *
*******************************************************************************/
CHAL_HANDLE chal_keypad_init(void __iomem *baseAddr)
{

	KeypadDev.baseAddr = baseAddr;

	return (CHAL_HANDLE)&KeypadDev;

}

/******************************************************************************
 *
 * Function Name: chal_keypad_shutdown
 *
 * Description: turn off the keypad csl driver.
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_shutdown(CHAL_HANDLE handle)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	chal_keypad_disable_interrupts(handle);
	/*disable the hardware block */

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, ENABLE, 0);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_enable
 *
 * Description: Enable or Disable the keypad ASIC block.
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_set_enable(CHAL_HANDLE handle, Boolean enable)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, ENABLE,
			     ((enable == TRUE) ? 1 : 0));
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_pullup_mode
 *
 * Description: Set Pull-up or pull-down mode for the ASIC block.
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_set_pullup_mode(CHAL_HANDLE handle, Boolean pullUp)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	pKpdDev->pullUpMode = pullUp;

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, MODE,
			     ((pullUp == TRUE) ? 1 : 0));
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_column_filter
 *
 * Description: Set column filter mode for the ASIC block.
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_set_column_filter(CHAL_HANDLE handle, Boolean enable,
				   CHAL_KEYPAD_DEBOUNCE_TIME_t debounce)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, COLFILTERENABLE,
			     ((enable == TRUE) ? 1 : 0));
	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, COLUMNFILTERTYPE,
			     debounce);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_status_filter
 *
 * Description: Set status filter mode for the ASIC block.
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_set_status_filter(CHAL_HANDLE handle, Boolean enable,
				   CHAL_KEYPAD_DEBOUNCE_TIME_t debounce)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, STATUSFILTERENABLE,
			     ((enable == TRUE) ? 1 : 0));
	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, STATUSFILTERTYPE,
			     debounce);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_column_width
 *
 * Description: Set column width for the ASIC block.
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_set_column_width(CHAL_HANDLE handle, cUInt32 columns)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, COLUMNWIDTH,
			     (columns - 1));
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_row_width
 *
 * Description: Set row width for the ASIC block.
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_set_row_width(CHAL_HANDLE handle, cUInt32 rows)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, ROWWIDTH,
			     (rows - 1));
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_row_output_control
 *
 * Description: Set row output control for the ASIC block.
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_set_row_output_control(CHAL_HANDLE handle, cUInt32 rows)
{
	cUInt32 rowMask;
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	rowMask = chal_keypad_calculate_pin_mask(rows);

	/* use rows as output */
	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPIOR, ROWOCONTRL,
			     rowMask);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_column_output_control
 *
 * Description: Set column output control for the ASIC block.
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_set_column_output_control(CHAL_HANDLE handle, cUInt32 columns)
{
	cUInt32 columnMask;
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	columnMask = chal_keypad_calculate_pin_mask(columns);

	/* use columns as output */
	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPIOR, COLUMNOCONTRL,
			     columnMask);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_interrupt_edge
 *
 * Description: Set interrupt edge control for the ASIC block.
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_set_interrupt_edge(CHAL_HANDLE handle,
				    CHAL_KEYPAD_INTERRUPT_EDGE_t edge)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;
	cUInt32 tempReg, i;

	/* configure the individual key interrupt controls */
	tempReg = 0;
	for (i = 0; i <= 30; i = i + 2)
		tempReg |= (edge << i);

	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPEMR0, tempReg);
	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPEMR1, tempReg);
	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPEMR2, tempReg);
	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPEMR3, tempReg);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_set_interrupt_mask
 *
 * Description: Set interrupt mask control for the ASIC block.
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_set_interrupt_mask(CHAL_HANDLE handle, cUInt32 rows,
				    cUInt32 columns)
{
	cUInt32 tempReg, columnMask;
	cUInt32 imr0RowCount, imr1RowCount, i;
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	columnMask = chal_keypad_calculate_pin_mask(columns);

	/* enable the appropriate interrupts. */
	if (rows >= 4) {
		imr0RowCount = 4;
		imr1RowCount = rows - 4;
	} else {
		imr0RowCount = rows;
		imr1RowCount = 0;
	}

	tempReg = 0;
	for (i = 0; i < imr0RowCount; i++)
		tempReg |= (columnMask << (i * 8));

	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPIMR0, tempReg);
	pKpdDev->imr0Val = tempReg;	/* save the imr0 value as we need it
					when enabling/disabling interrupts */

	tempReg = 0;
	for (i = 0; i < imr1RowCount; i++)
		tempReg |= (columnMask << (i * 8));

	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPIMR1, tempReg);
	pKpdDev->imr1Val = tempReg;	/* save the imr1 value as we need it
					when enabling/disabling interrupts */

}

/******************************************************************************
 *
 * Function Name: chal_keypad_get_pullup_status
 *
 * Description: Return the value of the Keypad Pullup mode.
 *
 * Parameters:  return    (out)   TRUE = Pullup mode
 *                                FALSE = Pull down mode
 *
 ******************************************************************************/
Boolean chal_keypad_get_pullup_status(CHAL_HANDLE handle)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	return pKpdDev->pullUpMode;
}

/******************************************************************************
 *
 * Function Name: chal_keypad_swap_row_and_column
 *
 * Description: Set the Swap row and column feature
 *
 * Parameters:  swap     (in)     TRUE = turn swap row and column ON
 *                                FALSE = set swap row and column OFF
 *
 ******************************************************************************/
void chal_keypad_swap_row_and_column(CHAL_HANDLE handle, Boolean swap)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG_FIELD(pKpdDev->baseAddr, KEYPAD_KPCR, SWAPROWCOLUMN,
			     ((swap == TRUE) ? 1 : 0));
}

/******************************************************************************
 *
 * Function Name: chal_keypad_handle_interrupt
 *
 * Description: A keypad interrupt has occurred. Save off all information
 * relating to the event for later processing
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_retrieve_key_event_registers(CHAL_HANDLE handle,
					      CHAL_KEYPAD_REGISTER_SET_t *
					      regState)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	regState->ssr0 = BRCM_READ_REG(pKpdDev->baseAddr, KEYPAD_KPSSR0);
	regState->ssr1 = BRCM_READ_REG(pKpdDev->baseAddr, KEYPAD_KPSSR1);
	regState->isr0 = BRCM_READ_REG(pKpdDev->baseAddr, KEYPAD_KPISR0);
	regState->isr1 = BRCM_READ_REG(pKpdDev->baseAddr, KEYPAD_KPISR1);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_clear_interrupts
 *
 * Description: Clear all outstanding interrupts
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_clear_interrupts(CHAL_HANDLE handle)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPICR0, 0xFFFFFFFF);
	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPICR1, 0xFFFFFFFF);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_disable_interrupts
 *
 * Description: Disable key event interrupts
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_disable_interrupts(CHAL_HANDLE handle)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPIMR0, 0);
	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPIMR1, 0);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_enable_interrupts
 *
 * Description: Enable key event interrupts
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_enable_interrupts(CHAL_HANDLE handle)
{
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPIMR0, pKpdDev->imr0Val);
	BRCM_WRITE_REG(pKpdDev->baseAddr, KEYPAD_KPIMR1, pKpdDev->imr1Val);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_decode_interrupt_event_registers
 *
 * Description: Take a set of keypad registers recorded when an
 *    interrupt occurred and decode the key event contained in them.
 *
 * Parameters:
 *          regSet       (in)       Is this the first or second register set?
 *                                  This determines if the events are for
 *                                  columns 0-3 or 4-7
 *                                    0 = ISR0 and SSR0 (col 0-3)
 *                                    1 = ISR1 and SSR1 (col 4-7)
 *          bitCount     (in)       The number of bits set in ISR
 *          ssr          (in)       The value of SSR
 *          isr          (in)       The value of ISR
 *
 ******************************************************************************/
void chal_keypad_decode_key_event_registers(CHAL_HANDLE *handle,
					    cUInt32 regSet,
					    cUInt32 bitCount,
					    cUInt32 ssr,
					    cUInt32 isr,
					    cUInt32 *numKeyEvents,
					    CHAL_KEYPAD_KEY_EVENT_LIST_t
					    keyEvents)
{
	cUInt32 mask, i, firstbit;
	CHAL_KEYPAD_EVENT_t *keyEvent;
	CHAL_KEYPAD_DEVICE_t *pKpdDev = (CHAL_KEYPAD_DEVICE_t *) handle;

	mask = isr;
	if (!pKpdDev->pullUpMode)	/* if pull-down mode, the
					SSR bits logic is negated */
	{
		ssr = ~ssr;
	}

	for (i = 0; i < bitCount; i++)	/* loop through all set bits, as
					 each indicates a key event */
	{
		keyEvent = &keyEvents[*numKeyEvents];

		firstbit = mask & (-mask);	/* this isolates the
					rightmost '1' bit from mask */

		if ((ssr & firstbit) == 0)
			keyEvent->keyAction = CHAL_KEYPAD_KEY_PRESS;
		else
			keyEvent->keyAction = CHAL_KEYPAD_KEY_RELEASE;

		keyEvent->keyId = bitNumber(firstbit) + (regSet * 32);
				/* raw KeyId is of the form (C*8)+R: */
		/*    C = column number  */
		/*    R = row number */

		mask &= (~firstbit);	/* mask out the bit
					 we just processed the event for */

		(*numKeyEvents)++;
	}

}

/******************************************************************************
 *
 * Function Name: chal_keypad_process_interrupt_events
 *
 * Description: Take one data set off the interrupt event FIFO and decode all
 *              key events contained in it and add them to the Key Event FIFO
 *
 * Parameters: none
 *
 ******************************************************************************/
cUInt32 chal_keypad_process_key_event_registers(CHAL_HANDLE *handle,
						CHAL_KEYPAD_REGISTER_SET_t *
						regState,
						CHAL_KEYPAD_KEY_EVENT_LIST_t
						keyEvents)
{
	cUInt32 bc1, bc2;
	cUInt32 numKeyEvents = 0;

	/* chal_dprintf( CDBG_INFO, "Key Event Regs: %lx %lx %lx %lx",
		regState->ssr0,regState->ssr1, regState->isr0,regState->isr1);*/

	bc1 = bitCount(regState->isr0);
	bc2 = bitCount(regState->isr1);

	if ((bc1 + bc2) <= MAX_SIMULTANEOUS_KEY_EVENTS) {
		if (bc1) {
			chal_keypad_decode_key_event_registers(handle,
							       0, bc1,
							       regState->ssr0,
							       regState->isr0,
							       &numKeyEvents,
							       keyEvents);
		}

		if (bc2) {
			chal_keypad_decode_key_event_registers(handle,
							       1, bc2,
							       regState->ssr1,
							       regState->isr1,
							       &numKeyEvents,
							       keyEvents);
		}

	}

	return numKeyEvents;

}	/* chal_keypad_process_key_event_registers() */

/******************************************************************************
 *
 * Function Name: chal_keypad_config_read_status1
 *
 * Description: Return the register value of the Keypad Status 1 register.
 *
 * Parameters:  return    (out)   SSR1 value
 *
 ******************************************************************************/
cUInt32 chal_keypad_config_read_status1(void)
{
	return BRCM_READ_REG(KeypadDev.baseAddr, KEYPAD_KPSSR0);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_config_read_status2
 *
 * Description: Return the register value of the Keypad Status 2 register.
 *
 * Parameters:  return    (out)   SSR2 value
 *
 ******************************************************************************/
cUInt32 chal_keypad_config_read_status2(void)
{
	return BRCM_READ_REG(KeypadDev.baseAddr, KEYPAD_KPSSR1);
}

#else /* tempINTERFACE_OSDAL_KEYPAD */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*#define "memmap.h" */
#include "chal_common.h"
#include "dbg.h"

#include "brcm_rdb_sysmap.h"
/*#include "brcm_rdb_syscfg.h" */
#include "brcm_rdb_keypad.h"

#include "chal_keypad.h"

#if (defined(_HERA_) || defined(_RHEA_))
#define SWAP_ROW_COL
#endif
/******************************************************************************/
/*                            T Y P E D E F S                                 */
/******************************************************************************/
#define	IOCR1_REG			0x08880004
#define KEYPAD_KPCR_Enable		KEYPAD_KPCR_ENABLE_MASK
#define KEYPAD_KPCR_Mode		KEYPAD_KPCR_MODE_MASK
#define KEYPAD_KPCR_ColFilterEnable	KEYPAD_KPCR_COLFILTERENABLE_MASK
#define KEYPAD_KPCR_StatusFilterEnable	KEYPAD_KPCR_STATUSFILTERENABLE_MASK
#define KEYPAD_KPCR_SwapRowColumn	KEYPAD_KPCR_SWAPROWCOLUMN_MASK

#define CHAL_INTERRUPT_EVENT_FIFO_LENGTH	4 /* max number of interrupt
						 event that can be stored */
						/* must be a power of 2 for
						 fifo operations to work */

#define CHAL_KEYPAD_KEY_ACTION_FIFO_LENGTH	4 /* max number of key event
						 that can be stored */
						/* since each interrupt
						  register set can have a
						  max of 4 simultaneous events
						  it must be at least 4 */
						/* must be a power of 2 for
						 fifo operations to work */

#define MAX_KEY_EVENT_PER_INTERRUPT		4 /* max valid key events in
						one interrupt-hardware defined*/

typedef struct {
	cUInt32 ssr0;
	cUInt32 ssr1;
	cUInt32 isr0;
	cUInt32 isr1;
} CHAL_KEYPAD_INTERRUPT_EVENT_t;	/* register set data
					 for 1 interrupt event. */

typedef struct {
	cUInt8 head;
	cUInt8 tail;
	cUInt8 length;
	CHAL_KEYPAD_INTERRUPT_EVENT_t eventQ[CHAL_INTERRUPT_EVENT_FIFO_LENGTH];
} CHAL_KEYPAD_INTERRUPT_FIFO_t;	/* interrupt event Q - register data
				 from an interrupt to be processed later. */

typedef struct {
	cUInt8 head;
	cUInt8 tail;
	cUInt8 length;
	CHAL_KEYPAD_EVENT_t actionQ[CHAL_KEYPAD_KEY_ACTION_FIFO_LENGTH];
} CHAL_KEYPAD_KEY_ACTION_FIFO_t;	/* key event Q - key events
					 decoded from interrupt events. */

#define FIFO_FULL(fifo)		(((fifo.head - fifo.tail) & \
				 (fifo.length-1)) >= fifo.length)
#define FIFO_EMPTY(fifo)	(fifo.head == fifo.tail)
#define FIFO_INCREMENT_HEAD(fifo)	(fifo.head = ((fifo.head+1) & \
						 (fifo.length-1)))
#define FIFO_INCREMENT_TAIL(fifo)	(fifo.tail = ((fifo.tail+1) & \
						 (fifo.length-1)))

/******************************************************************************/
/*                            G L O B A L S                                   */
/******************************************************************************/

/******************************************************************************/
/*                            LOCALS                                          */
/******************************************************************************/
CHAL_KEYPAD_CONFIG_t keypadConfig;	/* keypad hardware configuration */
cUInt8 keypadRowMask;		/* mask of pins used for rows */
cUInt8 keypadColumnMask;	/* mask of pins used for columns */
CHAL_KEYPAD_INTERRUPT_FIFO_t interruptEventQ;	/* interrupt Event Q */
CHAL_KEYPAD_KEY_ACTION_FIFO_t keyActionQ;	/* key Event Q */
cUInt32 imr0Val, imr1Val;	/* interrupt mask values. */

/******************************************************************************/
/*                            P R O T O T Y P E S                             */
/******************************************************************************/
static void chal_keypad_store_event(void);
static cUInt8 chal_keypad_calculate_pin_mask(cUInt8 pin);

/******************************************************************************/
/*                            F U N C T I O N S                               */
/******************************************************************************/

/******************************************************************************
 *
 * Function Name: bitCount()
 *
 * Description:  takes in a 32-bit value and returns the number of bits set
 *     to '1' in that value. This method of counting ones was chosen because
 *     it is optimised for inputs that have a sparse number of ones.
 *
 * Parameters:         n               (in)    The 32-bit value to examine
 * Return:              cUInt8                  The numner of '1' bits in n.
 *
 ******************************************************************************/
static cUInt8 bitCount(cUInt32 n)
{
	cUInt8 count = 0;

	while (n) {		/* loop until we have cleared all '1' bits */
		count++;
		n &= (n - 1);	/* this sets the rightmost '1' bit to 0 */
	}

	return count;

}	/* bitCount() */

/******************************************************************************
 *
 * Function Name: bitNumber()
 *
 * Description:takes in a 32-bit value and returns the index of the rightmost
 *    '1' bit. It uses a DeBruijn sequence to calculate the index faster than
 *    iterating through the value to find it.
 *
 * Parameters:    n          (in)    The 32-bit value to examine
 * Return:         cUInt8             The index of the rightmost '1' bit in n
 *
 ******************************************************************************/
static cUInt8 bitNumber(cUInt32 val)
{
	static const cUInt8 MultiplyDeBruijnBitPosition[32] = {
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};

	/* The constant 0x077CB531 is a DeBruijn sequence that produces a
	 * unique pattern of bits in the high 5 bits for each possible bit
	 * position that it is multiplied against
	 * The usage of (val & -val) isolates only the rightmost '1' bit from
	 * val. This ensures that the multiplier only has 1 bit set to '1'. */

	return MultiplyDeBruijnBitPosition[(cUInt8)
					   (((val & -val) *
					     0x077CB531U) >> 27)];

}				/* bitNumber() */

/******************************************************************************
 *
 * Function Name: chal_keypad_calculate_pin_mask()
 *
 * Description:  creates a mask that has the first n bits set.
 *
 * Parameters:        n            (in)    The number of bits to set
 * Return:             cUInt8               A mask with the first n bits set.
 *
 ******************************************************************************/
cUInt8 chal_keypad_calculate_pin_mask(cUInt8 n)
{

	if ((n > 8) || (n < 1)) {	/* n should never be bigger than 8 as
					   the hardware supports 8x8
					   matrix maximum. */
		return 0;
	}

	return (1 << n) - 1;
}				/* chal_keypad_calculate_pin_maks() */

/******************************************************************************
 *
 * Function Name: chal_keypad_init
 *
 * Description: initialize the keypad csl driver
 *
 * Parameters:    config     (in)    hardware cfg. of keypad to initialize
 *
 * Note: When this function completes, the keypad hardware is configured as
 *        requested and key interrupts are enabled.
 *
 ******************************************************************************/
void chal_keypad_init(CHAL_KEYPAD_CONFIG_t config)
{
	cUInt32 tempReg, kpcrReg;
	cUInt8 imr0RowCount, imr1RowCount, i;

	/* Initialize the 2 interrupt event FIFOs */
	interruptEventQ.head = 0;
	interruptEventQ.tail = 0;
	interruptEventQ.length = CHAL_INTERRUPT_EVENT_FIFO_LENGTH;

	keyActionQ.head = 0;
	keyActionQ.tail = 0;
	keyActionQ.length = CHAL_KEYPAD_KEY_ACTION_FIFO_LENGTH;

	/* store the keypad config locally */
	keypadConfig = config;

	/* create the row and col masks needed to set various register values */
	keypadRowMask = chal_keypad_calculate_pin_mask(keypadConfig.rows);
	keypadColumnMask = chal_keypad_calculate_pin_mask(keypadConfig.columns);

	/* disable all key interrupts */
	CHAL_REG_WRITE32(KEYPAD_KPIMR0, 0);
	CHAL_REG_WRITE32(KEYPAD_KPIMR1, 0);

	/* clear any old interrupts */
	CHAL_REG_WRITE32(KEYPAD_KPICR0, 0xFFFFFFFF);
	CHAL_REG_WRITE32(KEYPAD_KPICR1, 0xFFFFFFFF);

	/* disable the keypad hardware block */
	kpcrReg = CHAL_REG_READ32(KEYPAD_KPCR);
	CHAL_REG_WRITE32(KEYPAD_KPCR, (kpcrReg & ~(KEYPAD_KPCR_Enable)));

	/* set the pin configuration for the row and column signals */
/*#if !(defined(_HERA_) || defined(_RHEA_)) */
	CHAL_REG_WRITE32(IOCR1_REG, ((keypadColumnMask << 8) | keypadRowMask));
/*#endif */
#ifdef SWAP_ROW_COL

#else

#endif
	/* setup the hardware configuration register */
	kpcrReg = 0;
	if (keypadConfig.pullUpMode)
		kpcrReg |= KEYPAD_KPCR_Mode;

	kpcrReg |= KEYPAD_KPCR_ColFilterEnable;
	kpcrReg |= (keypadConfig.debounceTime << 8);
	kpcrReg |= KEYPAD_KPCR_StatusFilterEnable;
	kpcrReg |= (keypadConfig.debounceTime << 12);
	kpcrReg |= ((keypadConfig.columns - 1) << 16);
	kpcrReg |= ((keypadConfig.rows - 1) << 20);

#ifdef SWAP_ROW_COL
	kpcrReg |= KEYPAD_KPCR_SwapRowColumn;
#endif
	CHAL_REG_WRITE32(KEYPAD_KPCR, kpcrReg);

/*#ifdef SWAP_ROW_COL */
/*      // use cols as output */
/*      CHAL_REG_WRITE32( KEYPAD_KPIOR, (keypadColumnMask << 16) ); */
/*#else */
	/* use rows as output */
	CHAL_REG_WRITE32(KEYPAD_KPIOR, (keypadColumnMask << 24));
/*#endif */
	/* configure the individual key interrupt controls */
	tempReg = 0;
	for (i = 0; i <= 30; i = i + 2)
		tempReg |= (keypadConfig.interruptEdge << i);

	CHAL_REG_WRITE32(KEYPAD_KPEMR0, tempReg);
	CHAL_REG_WRITE32(KEYPAD_KPEMR1, tempReg);
	CHAL_REG_WRITE32(KEYPAD_KPEMR2, tempReg);
	CHAL_REG_WRITE32(KEYPAD_KPEMR3, tempReg);

	/* enable the appropriate interrupts. */
	if (keypadConfig.rows >= 4) {
		imr0RowCount = 4;
		imr1RowCount = keypadConfig.rows - 4;
	} else {
		imr0RowCount = keypadConfig.rows;
		imr1RowCount = 0;
	}

	CHAL_REG_WRITE32(KEYPAD_KPICR0, 0xFFFFFFFF);
	CHAL_REG_WRITE32(KEYPAD_KPICR1, 0xFFFFFFFF);

	tempReg = 0;
	for (i = 0; i < imr0RowCount; i++)
		tempReg |= (keypadColumnMask << (i * 8));

	CHAL_REG_WRITE32(KEYPAD_KPIMR0, tempReg);
	imr0Val = tempReg;	/* save the imr0 value as we need it when
					 enabling/disabling interrupts */

	tempReg = 0;
	for (i = 0; i < imr1RowCount; i++)
		tempReg |= (keypadColumnMask << (i * 8));

	CHAL_REG_WRITE32(KEYPAD_KPIMR1, tempReg);
	imr1Val = tempReg;	/* save the imr1 value as we need it when
					 enabling/disabling interrupts */

	/* clear any outstanding interrupts */
	CHAL_REG_WRITE32(KEYPAD_KPICR0, 0xFFFFFFFF);
	CHAL_REG_WRITE32(KEYPAD_KPICR1, 0xFFFFFFFF);

	/* enable the keypad hardware block */
	CHAL_REG_WRITE32(KEYPAD_KPCR, (kpcrReg | KEYPAD_KPCR_Enable));

	return;
}				/* chal_keypad_init() */

/******************************************************************************
 *
 * Function Name: chal_keypad_shutdown
 *
 * Description: turn off the keypad csl driver
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_shutdown()
{
	cUInt32 kpcrReg;

	chal_keypad_disable_interrupts();

	/*disable the hardware block */
	kpcrReg = CHAL_REG_READ32(KEYPAD_KPCR);
	CHAL_REG_WRITE32(KEYPAD_KPCR, (kpcrReg & ~(KEYPAD_KPCR_Enable)));
}

/******************************************************************************
 *
 * Function Name: chal_keypad_handle_interrupt
 *
 * Description: A keypad interrupt has occurred. Save off all information
 * relating to the event for later processing
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_handle_interrupt()
{
	CHAL_KEYPAD_INTERRUPT_EVENT_t *event;

	if (!FIFO_FULL(interruptEventQ)) {
		event = &interruptEventQ.eventQ[interruptEventQ.head];
		event->ssr0 = *(volatile cUInt32 *)KEYPAD_KPSSR0;
		event->ssr1 = *(volatile cUInt32 *)KEYPAD_KPSSR1;
		event->isr0 = *(volatile cUInt32 *)KEYPAD_KPISR0;
		event->isr1 = *(volatile cUInt32 *)KEYPAD_KPISR1;
		FIFO_INCREMENT_HEAD(interruptEventQ);
	}
	/* event information is saved, clear the interrupt */
	CHAL_REG_WRITE32(KEYPAD_KPICR0, 0xFFFFFFFF);
	CHAL_REG_WRITE32(KEYPAD_KPICR1, 0xFFFFFFFF);

}

/******************************************************************************
 *
 * Function Name: chal_keypad_clear_interrupts
 *
 * Description: Clear all outstanding interrupts
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_clear_interrupts()
{
	CHAL_REG_WRITE32(KEYPAD_KPICR0, 0xFFFFFFFF);
	CHAL_REG_WRITE32(KEYPAD_KPICR1, 0xFFFFFFFF);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_disable_interrupts
 *
 * Description: Disable key event interrupts
 *
 * Parameters:  none
 *
 ******************************************************************************/
void chal_keypad_disable_interrupts()
{
	CHAL_REG_WRITE32(KEYPAD_KPIMR0, 0);
	CHAL_REG_WRITE32(KEYPAD_KPIMR1, 0);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_enable_interrupts
 *
 * Description: Enable key event interrupts
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_enable_interrupts()
{
	CHAL_REG_WRITE32(KEYPAD_KPIMR0, imr0Val);
	CHAL_REG_WRITE32(KEYPAD_KPIMR1, imr1Val);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_decode_interrupt_event_registers
 *
 * Description: Take a set of keypad registers recorded when an interrupt
 *    occurred and decode the key event contained in them.
 *
 * Parameters:
 *         regSet       (in)       Is this the first or second register set?
 *                                 This determines if the events are for
 *                                 columns 0-3 or 4-7
 *                                 0 = ISR0 and SSR0 (col 0-3)
 *                                 1 = ISR1 and SSR1 (col 4-7)
 *         bitCount     (in)       The number of bits set in ISR
 *         ssr          (in)       The value of SSR
 *         isr          (in)       The value of ISR
 *
 ******************************************************************************/
void chal_keypad_decode_interrupt_event_registers(cUInt32 regSet,
						  cUInt32 bitCount, cUInt32 ssr,
						  cUInt32 isr)
{
	cUInt32 mask, i, firstbit;
	CHAL_KEYPAD_EVENT_t *keyEvent;

	mask = isr;
	if (!keypadConfig.pullUpMode)	/* if pull-down mode, the SSR bits
							 logic is negated */
	{
		ssr = ~ssr;
	}

	for (i = 0; i < bitCount; i++)	/* loop through all set bits, as each
						 indicates a key event */
	{
		keyEvent = &keyActionQ.actionQ[keyActionQ.tail];

		firstbit = mask & (-mask);	/* this isolates the rightmost
							 '1' bit from mask */

		if ((ssr & firstbit) == 0)
			keyEvent->keyAction = CHAL_KEYPAD_KEY_PRESS;
		else
			keyEvent->keyAction = CHAL_KEYPAD_KEY_RELEASE;

		keyEvent->keyId = bitNumber(firstbit) + (regSet * 32);
				/* raw KeyId is of the form 0xCR where: */
		/*    C = column number  */
		/*    R = row number */

		mask &= (~firstbit);	/* mask out the bit we just processed
							 the event for */

		FIFO_INCREMENT_TAIL(keyActionQ);
	}

}

/******************************************************************************
 *
 * Function Name: chal_keypad_process_interrupt_events
 *
 * Description: Take one data set off the interrupt event FIFO and decode all
 *              of the key events contained in it and add them to the
 *              Key Event FIFO.
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_process_interrupt_events()
{
	cUInt32 bc1, bc2;
	CHAL_KEYPAD_INTERRUPT_EVENT_t *interruptEvent;

	if ((!FIFO_EMPTY(interruptEventQ)) && (!FIFO_FULL(keyActionQ))) {

		interruptEvent = &interruptEventQ.eventQ[interruptEventQ.tail];

		bc1 = bitCount(interruptEvent->isr0);
		bc2 = bitCount(interruptEvent->isr1);

		if ((bc1 + bc2) <= MAX_KEY_EVENT_PER_INTERRUPT) {
			if (bc1) {
				chal_keypad_decode_interrupt_event_registers(0,
							bc1,
							interruptEvent->ssr0,
							interruptEvent->isr0);
			}

			if (bc2) {
				chal_keypad_decode_interrupt_event_registers(1,
							bc2,
							interruptEvent->ssr1,
							interruptEvent->isr1);
			}

		} else {
			/* appears to be an error (too many bits set in ISR)
			 clear the interrupts to reset and hopefully continue */
			chal_keypad_clear_interrupts();
		}

		FIFO_INCREMENT_TAIL(interruptEventQ);
	}

}				/* chal_keypad_process_interrupt_events() */

/******************************************************************************
 *
 * Function Name: chal_keypad_get_action_from_Q
 *
 * Description: Pull the first set of hardware register off of the interrupt
 *      event Q.
 *
 * Parameters:  event   (out)   A key event
 *
 * Return:   Boolean   TRUE = key event found and info set in event parameter
 *                     FALSE = no key event exist in queue.
 *
 ******************************************************************************/
Boolean chal_keypad_get_action_from_Q(CHAL_KEYPAD_EVENT_t *event)
{
	Boolean retVal = FALSE;

	if (!FIFO_EMPTY(keyActionQ)) {
		event->keyId = keyActionQ.actionQ[keyActionQ.head].keyId;
		event->keyAction =
		    keyActionQ.actionQ[keyActionQ.head].keyAction;
		FIFO_INCREMENT_HEAD(keyActionQ);
		retVal = TRUE;
	}

	return retVal;
}	/* chal_keypad_get_action_from_Q() */

/******************************************************************************
 *
 * Function Name: chal_keypad_retrieve_event
 *
 * Description: Find a saved key event. First check the key event Q. This Q
 *      contains key event from previously processed interrupt events. If
 *      nothing is in the key event Q, then check to see if interrupt events
 *      exist in that Q. If so, process the first one. and then take the
 *      resulting first key event and return it.
 *
 * Parameters:  event    (out)   A key event
 *
 ******************************************************************************/
void chal_keypad_retrieve_event(CHAL_KEYPAD_EVENT_t *event)
{

	if (!chal_keypad_get_action_from_Q(event)) {
		chal_keypad_process_interrupt_events();

		if (!chal_keypad_get_action_from_Q(event))
			event->keyAction = CHAL_KEYPAD_KEY_NO_ACTION;


	}
}	/* chal_keypad_retrieve_event() */

/******************************************************************************
 *
 * Function Name: chal_keypad_config_reset
 *
 * Description: Reset the keypad ASIC block
 *
 * Parameters: none
 *
 ******************************************************************************/
void chal_keypad_config_reset()
{

	CHAL_REG_WRITE32(KEYPAD_KPIMR0, 0);

}

/******************************************************************************
 *
 * Function Name: chal_keypad_config_read_status1
 *
 * Description: Return the register value of the Keypad Status 1 register.
 *
 * Parameters:  return    (out)   SSR1 value
 *
 ******************************************************************************/
cUInt32 chal_keypad_config_read_status1()
{
	return CHAL_REG_READ32(KEYPAD_KPSSR0);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_config_read_status2
 *
 * Description: Return the register value of the Keypad Status 2 register.
 *
 * Parameters:  return    (out)   SSR2 value
 *
 ******************************************************************************/
cUInt32 chal_keypad_config_read_status2()
{
	return CHAL_REG_READ32(KEYPAD_KPSSR1);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_update_interrupt_clear_register0
 *
 * Description: Update the Keypad Interrupt Clear register 0.
 *
 * Parameters:  value    (in)    new value for ICR0
 *
 ******************************************************************************/
void chal_keypad_update_interrupt_clear_register0(cUInt32 value)
{
	CHAL_REG_WRITE32(KEYPAD_KPICR0, value);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_update_interrupt_clear_register1
 *
 * Description: Update the Keypad Interrupt Clear register 1.
 *
 * Parameters:  value    (in)    new value for ICR1
 *
 ******************************************************************************/
void chal_keypad_update_interrupt_clear_register2(cUInt32 value)
{
	CHAL_REG_WRITE32(KEYPAD_KPICR1, value);
}

/******************************************************************************
 *
 * Function Name: chal_keypad_read_pullup_status
 *
 * Description: Return the value of the Keypad Pullup mode.
 *
 * Parameters:  return    (out)   Pullup Mode bit
 *
 ******************************************************************************/
cUInt32 chal_keypad_read_pullup_status()
{
	return (CHAL_REG_READ32(KEYPAD_KPCR)) & KEYPAD_KPCR_Mode;
}

/******************************************************************************
 *
 * Function Name: chal_keypad_swap_row_and_column
 *
 * Description: Set the Swap row and column feature
 *
 * Parameters:  swap     (in)     TRUE = turn swap row and column ON
 *                                FALSE = set swap row and column OFF
 *
 ******************************************************************************/
void chal_keypad_swap_row_and_column(Boolean swap)
{
	cUInt32 regVal;

	regVal = CHAL_REG_READ32(KEYPAD_KPCR);

	if (swap)
		regVal |= KEYPAD_KPCR_SWAPROWCOLUMN_MASK;
	else
		regVal &= ~KEYPAD_KPCR_SWAPROWCOLUMN_MASK;


	CHAL_REG_WRITE32(KEYPAD_KPCR, regVal);

}

#endif /* tempINTERFACE_OSDAL_KEYPAD */

/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
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

/****************************************************************************/
/**
*  @file    chal_keypad.c
*
*  @brief   Low level keypad driver routines
*
*  @note
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <chal/chal_keypad.h>

/* ---- External Function Prototypes ------------------------------------- */
/* ---- External Variable Declarations ----------------------------------- */
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */

/* ==== Public Functions ================================================= */

/****************************************************************************
* See chal_keypad.h for API documentation.
****************************************************************************/

int chal_keypad_init(CHAL_KEYPAD_HANDLE_t *handle, const CHAL_KEYPAD_CONFIG_t *config)
{
   uint32_t temp;
   int      i;


   if ( !handle || !handle->regBaseAddr )
   {
      return( -1 );
   }

   if ( !config ||
         (config->columns > CHAL_KEYPAD_SCAN_COL_CNT) ||
         (config->rows > CHAL_KEYPAD_SCAN_ROW_CNT) ||
         (config->interruptEdge > CHAL_KEYPAD_INTERRUPT_EDGE_MAX) ||
         (config->debounceTime > CHAL_KEYPAD_DEBOUNCE_MAX)
      )
   {
      return( -2 );
   }

   /* Ensure things are inactive and in a known default state. */

   chal_keypad_term(handle);

   /* Use rows as output for scan. Need to set a bit for each row. */

   temp = (1 << config->rows) - 1;
   temp = temp << KEYPAD_KPIOR_ROWOCONTRL_SHIFT;
   CHAL_REG_WRITE32(handle->regBaseAddr + KEYPAD_KPIOR_OFFSET, temp );

   /* Configure the individual key interrupt controls. There's 2-bits for each key
    * for this, spread over 4 32-bit registers. We will set all keys to the desired
    * value, even though all keys might not be used. Create a 32-bit value with
    * all the 2-bit fields set the same, and then write to the 4 registers.
    */
   temp = 0;
   for ( i = 0; i < 32; i += 2 )
   {
      temp |= (config->interruptEdge << i);
   }
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPEMR0_OFFSET, temp );
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPEMR1_OFFSET, temp );
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPEMR2_OFFSET, temp );
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPEMR3_OFFSET, temp );

   /* Setup the hardware configuration register, including enable of keypad operations */

   temp = KEYPAD_KPCR_ENABLE_MASK |
            KEYPAD_KPCR_COLFILTERENABLE_MASK |
            (config->debounceTime << KEYPAD_KPCR_COLUMNFILTERTYPE_SHIFT) |
            KEYPAD_KPCR_STATUSFILTERENABLE_MASK |
            (config->debounceTime << KEYPAD_KPCR_STATUSFILTERTYPE_SHIFT) |
            ((config->columns - 1) << KEYPAD_KPCR_COLUMNWIDTH_SHIFT) |
            ((config->rows - 1) << KEYPAD_KPCR_ROWWIDTH_SHIFT);

   if (config->activeLowMode)
   {
      temp |= KEYPAD_KPCR_MODE_MASK;
   }
   if (config->swapRowColumn)
   {
      temp |= KEYPAD_KPCR_SWAPROWCOLUMN_MASK;
   }
   CHAL_REG_WRITE32(handle->regBaseAddr + KEYPAD_KPCR_OFFSET, temp);

   return( 0 );
}

/* ==== Private Functions ================================================= */


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
*  @file    chal_keypad.h
*
*  @brief   Keypad driver interface header file.
*
*  @note
*
****************************************************************************/
#ifndef _CHAL_KEYPAD_H
#define _CHAL_KEYPAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>

#include <chal/chal_defs.h>
/* --- START: CHAL compatability shim --- */
static void CHAL_REG_SETBIT32(uint32_t addr, unsigned int bits)
{
    iowrite32(ioread32((void *)addr) | bits,  (void *)addr);
}

static void CHAL_REG_CLRBIT32(uint32_t addr, unsigned int bits)
{
    iowrite32(ioread32((void *)addr) & (~bits),  (void *)addr);
}

#define CHAL_REG_WRITE32(x,y) iowrite32(y,x)
#define CHAL_REG_READ32(x) ioread32(x)
#define chal_dbg_printf(x,...) pr_debug(__VA_ARGS__)
#include <mach/rdb/brcm_rdb_keypad.h>
/* --- END:  CHAL compatability shim --- */

/* ---- Public Constants and Types --------------------------------------- */

#define CHAL_KEYPAD_REG_ADDR_BASE_PHYSICAL      MM_ADDR_IO_KEYPAD
#define CHAL_KEYPAD_REG_ADDR_SPACE_BYTES        (KEYPAD_KPDBCTR_OFFSET + sizeof(uint32_t))

#define CHAL_KEYPAD_SCAN_COL_CNT                8
#define CHAL_KEYPAD_SCAN_ROW_CNT                8

#define CHAL_KEYPAD_REG32_CNT                   (CHAL_KEYPAD_SCAN_ROW_CNT / sizeof(uint32_t))

/*
 * Handle for KEYPAD controller operations.
 *
 * NOTE: The regBaseAddr must be set prior to using the handle, and contain
 * an appropriate base address for register accesses. This could be a static
 * (compile time) address such as MM_IO_BASE_KEYPAD, or a virtual address
 * acquired during runtime using CHAL_KEYPAD_REG_ADDR_BASE_PHYSICAL.
 */
struct chal_keypad_handle
{
   uint32_t    regBaseAddr;      /* Base address to use for register accesses */
};
typedef struct chal_keypad_handle CHAL_KEYPAD_HANDLE_t;

/*
 * Interrupt triggering edge. This enumeration maps directly to register
 * field values.
 */
enum chal_keypad_interrupt_edge
{
   CHAL_KEYPAD_INTERRUPT_EDGE_NONE    = 0,
   CHAL_KEYPAD_INTERRUPT_EDGE_RISING  = 1,
   CHAL_KEYPAD_INTERRUPT_EDGE_FALLING = 2,
   CHAL_KEYPAD_INTERRUPT_EDGE_BOTH    = 3,
   CHAL_KEYPAD_INTERRUPT_EDGE_CNT
};
typedef enum chal_keypad_interrupt_edge CHAL_KEYPAD_INTERRUPT_EDGE_t;

#define CHAL_KEYPAD_INTERRUPT_EDGE_MIN          CHAL_KEYPAD_INTERRUPT_EDGE_NONE
#define CHAL_KEYPAD_INTERRUPT_EDGE_MAX          CHAL_KEYPAD_INTERRUPT_EDGE_BOTH

/*
 * Debounce time for key state detection. This enumeration maps directly to
 * register field values.
 */
enum chal_keypad_debounce_time
{
   CHAL_KEYPAD_DEBOUNCE_1_ms   = 0,
   CHAL_KEYPAD_DEBOUNCE_2_ms   = 1,
   CHAL_KEYPAD_DEBOUNCE_4_ms   = 2,
   CHAL_KEYPAD_DEBOUNCE_8_ms   = 3,
   CHAL_KEYPAD_DEBOUNCE_16_ms  = 4,
   CHAL_KEYPAD_DEBOUNCE_32_ms  = 5,
   CHAL_KEYPAD_DEBOUNCE_64_ms  = 6,
   CHAL_KEYPAD_DEBOUNCE_128_ms = 7,
   CHAL_KEYPAD_DEBOUNCE_CNT
};
typedef enum chal_keypad_debounce_time CHAL_KEYPAD_DEBOUNCE_TIME_t;

#define CHAL_KEYPAD_DEBOUNCE_MIN                CHAL_KEYPAD_DEBOUNCE_1_ms
#define CHAL_KEYPAD_DEBOUNCE_MAX                CHAL_KEYPAD_DEBOUNCE_128_ms

/*
 * KEYPAD controller configuration parameters.
 */
struct chal_keypad_config
{
   /* Number of rows and columns in the physical keypad layout */
   unsigned  rows;
   unsigned  columns;

   /* Key press is active low. Output signals will be pulled-up. */
   unsigned  activeLowMode;

   /* Swap row and column physical scan lines */
   unsigned  swapRowColumn;

   CHAL_KEYPAD_INTERRUPT_EDGE_t  interruptEdge;
   CHAL_KEYPAD_DEBOUNCE_TIME_t   debounceTime;
};
typedef struct chal_keypad_config CHAL_KEYPAD_CONFIG_t;

/*
 * The keypad matrix is used for scan and interrupt status, and also
 * for interrupt enable mask. This matrix info is typically contained
 * in 2 registers, with the first register containing rows 0..3 and the
 * second registers rows 4..7. To simplify access and make things more
 * readable, use a union which can be used to convert between register
 * values and an array of scan lines reflecting the rows.
 *
 * NOTE: use the chal_keypad_matrix_xxx() routines to access variables
 * of this type. Direct access is not recommended.
 */
struct chal_keypad_matrix
{
   union
   {
      uint32_t    reg[CHAL_KEYPAD_REG32_CNT];
      uint8_t     line[CHAL_KEYPAD_SCAN_ROW_CNT];
   }
   scan;
};
typedef struct chal_keypad_matrix CHAL_KEYPAD_MATRIX_t;

/* ---- Public Variable Externs ------------------------------------------ */
/* ---- Public Function Prototypes --------------------------------------- */

static inline void chal_keypad_interrupt_disable_all(CHAL_KEYPAD_HANDLE_t *handle);
static inline void chal_keypad_interrupt_clear_all(CHAL_KEYPAD_HANDLE_t *handle);
static inline uint32_t chal_keypad_scan_mode_is_active_low(CHAL_KEYPAD_HANDLE_t *handle);

/*===========================================================================
* ===========================================================================
* Functions for KEYPAD controller operations.
* ===========================================================================
* ===========================================================================*/

/****************************************************************************/
/**
*  @brief   Initiate keypad hardware operations
*
*  @param   handle      (out) keypad handle
*  @param   config      (in)  keypad hardware configuration
*
*  @return  < 0 if an error, >=0 otherwise
*
*  @note    The handle regBaseAddr must be set prior to calling this routine.
*           Interrupts are disabled upon exit from this routine. To enable,
*           see chal_keypad_interrupt_enable().
****************************************************************************/
int chal_keypad_init(CHAL_KEYPAD_HANDLE_t *handle, const CHAL_KEYPAD_CONFIG_t *config);

/****************************************************************************/
/**
*  @brief   Terminate keypad hardware operations
*
*  @param   handle      (in)  keypad handle
*
*  @return  < 0 if an error, >=0 otherwise
*
****************************************************************************/
static inline int chal_keypad_term(CHAL_KEYPAD_HANDLE_t *handle)
{
   chal_keypad_interrupt_disable_all(handle);
   chal_keypad_interrupt_clear_all(handle);
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPCR_OFFSET, 0 );

   return( 0 );
}
#define chal_keypad_exit      chal_keypad_term
#define chal_keypad_shutdown  chal_keypad_term

/****************************************************************************/
/**
*  @brief   Clear keypad event interrupt status
*
*  @param   handle      (in)  keypad handle
*  @param   matrix      (in)  matrix from chal_keypad_interrupt_get_status().
*
*  @return
****************************************************************************/
static inline void chal_keypad_interrupt_clear(CHAL_KEYPAD_HANDLE_t *handle, CHAL_KEYPAD_MATRIX_t *status)
{
    /** @todo Handle any endianess issues. This is OK right now because everything is little-endian */

   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPICR0_OFFSET, status->scan.reg[0] );
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPICR1_OFFSET, status->scan.reg[1] );
}

/****************************************************************************/
/**
*  @brief   Clear all keypad event interrupts
*
*  @param   handle      (in)  keypad handle
*
*  @return  none
*
****************************************************************************/
static inline void chal_keypad_interrupt_clear_all(CHAL_KEYPAD_HANDLE_t *handle)
{
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPICR0_OFFSET, 0xFFFFFFFF );
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPICR1_OFFSET, 0xFFFFFFFF );
}

/****************************************************************************/
/**
*  @brief   Disable all keypad event interrupts
*
*  @param   handle      (in)  keypad handle
*
*  @return  none
*
****************************************************************************/
static inline void chal_keypad_interrupt_disable_all(CHAL_KEYPAD_HANDLE_t *handle)
{
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPIMR0_OFFSET, 0 );
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPIMR1_OFFSET, 0 );
}

/****************************************************************************/
/**
*  @brief   Enable keypad event interrupts
*
*  @param   handle      (in)  keypad handle
*  @param   mask        (in)  matrix specifying which keys to enable for interrupt
*
*  @return  none
*
*  @note    If interrupts are to be enabled, then a row / column interrupt mask
*           needs to be created before calling this routine. If interrupts
*           are to be enabled / disabled / re-enabled, it is recommended
*           that the mask be stored for subsequent use instead of re-creating
*           the mask each time interrupts are to be enabled. The registers that
*           contain the mask are also used to disable interrupts, and the
*           registers are also write-only. To create the desired mask, see
*              chal_keypad_matrix_clear_all()
*              chal_keypad_matrix_set()
*              chal_keypad_matrix_set_all()
*
****************************************************************************/
static inline void chal_keypad_interrupt_enable(CHAL_KEYPAD_HANDLE_t *handle, CHAL_KEYPAD_MATRIX_t *mask)
{
   /** @todo Handle any endianess issues. This is OK right now because everything is little-endian */

   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPIMR0_OFFSET, mask->scan.reg[0] );
   CHAL_REG_WRITE32( handle->regBaseAddr + KEYPAD_KPIMR1_OFFSET, mask->scan.reg[1] );
}

/****************************************************************************/
/**
*  @brief   Get keypad event interrupt status
*
*  @param   handle      (in)  keypad handle
*  @param   status      (out) keypad row/column matrix.
*
*  @return  !0 if any status is set.
*
*  @note    Use chal_keypad_matrix_is_set() to inspect status afterwards.
*
****************************************************************************/
static inline uint32_t chal_keypad_interrupt_get_status(CHAL_KEYPAD_HANDLE_t *handle, CHAL_KEYPAD_MATRIX_t *status)
{
   /** @todo Handle any endianess issues. This is OK right now because everything is little-endian */

   status->scan.reg[0] = CHAL_REG_READ32( handle->regBaseAddr + KEYPAD_KPISR0_OFFSET );
   status->scan.reg[1] = CHAL_REG_READ32( handle->regBaseAddr + KEYPAD_KPISR1_OFFSET );

   return( status->scan.reg[0] | status->scan.reg[1] );
}

/****************************************************************************/
/**
*  @brief   Get keypad scan line status
*
*  @param   handle      (in)  keypad handle
*  @param   status      (out) keypad row/column matrix.
*
*  @return  !0 if any scan status is set (any key pressed).
*
*  @note    Use chal_keypad_matrix_is_set() to inspect status afterwards.
*
****************************************************************************/
static inline uint32_t chal_keypad_scan_get_status(CHAL_KEYPAD_HANDLE_t *handle, CHAL_KEYPAD_MATRIX_t *status)
{
   /** @todo Handle any endianess issues. This is OK right now because everything is little-endian */

   status->scan.reg[0] = CHAL_REG_READ32( handle->regBaseAddr + KEYPAD_KPSSR0_OFFSET );
   status->scan.reg[1] = CHAL_REG_READ32( handle->regBaseAddr + KEYPAD_KPSSR1_OFFSET );

   if ( chal_keypad_scan_mode_is_active_low(handle) )
   {
      /* Register status reflects physical scan line state. Flip the bits to get the
       * logical state.
       */
      status->scan.reg[0] = ~status->scan.reg[0];
      status->scan.reg[1] = ~status->scan.reg[1];
   }

   return( status->scan.reg[0] | status->scan.reg[1] );
}

/****************************************************************************/
/**
*  @brief   Get scan rows / columns
*
*  @param   handle      (in)  keypad handle
*
*  @return  Current configured number of rows / columns.
****************************************************************************/
static inline uint32_t chal_keypad_scan_get_cols(CHAL_KEYPAD_HANDLE_t *handle)
{
   return( 1 + ((CHAL_REG_READ32(handle->regBaseAddr + KEYPAD_KPCR_OFFSET) & KEYPAD_KPCR_COLUMNWIDTH_MASK) >> KEYPAD_KPCR_COLUMNWIDTH_SHIFT) );
}
static inline uint32_t chal_keypad_scan_get_rows(CHAL_KEYPAD_HANDLE_t *handle)
{
   return( 1 + ((CHAL_REG_READ32(handle->regBaseAddr + KEYPAD_KPCR_OFFSET) & KEYPAD_KPCR_ROWWIDTH_MASK) >> KEYPAD_KPCR_ROWWIDTH_SHIFT) );
}

/****************************************************************************/
/**
*  @brief   Get scan active low mode state
*
*  @param   handle      (in)  keypad handle
*
*  @return  Current configured state. 0 implies active low mode disabled.
****************************************************************************/
static inline uint32_t chal_keypad_scan_mode_is_active_low(CHAL_KEYPAD_HANDLE_t *handle)
{
   return( CHAL_REG_READ32(handle->regBaseAddr + KEYPAD_KPCR_OFFSET) & KEYPAD_KPCR_MODE_MASK );
}

/****************************************************************************/
/**
*  @brief   Enable / disable the swapping of row / column scan lines
*
*  @param   handle   (in)    keypad handle
*
*  @return
*
*  @note    If enabled, swaps the physical scan lines used for row / column.
*           I.e. the scan lines normally used for rows will be used for columns
*           and vice-versa.
****************************************************************************/
static inline void chal_keypad_scan_swap_row_column(CHAL_KEYPAD_HANDLE_t *handle, int swap)
{
   if (swap)
   {
      CHAL_REG_SETBIT32(handle->regBaseAddr + KEYPAD_KPCR_OFFSET, KEYPAD_KPCR_SWAPROWCOLUMN_MASK);
   }
   else
   {
      CHAL_REG_CLRBIT32(handle->regBaseAddr + KEYPAD_KPCR_OFFSET, KEYPAD_KPCR_SWAPROWCOLUMN_MASK);
   }
}


/*===========================================================================
* ===========================================================================
* Functions for accessing a KEYPAD matrix variable. Should use these instead
* of interpreting the variable contents directly.
* ===========================================================================
* ===========================================================================*/

/****************************************************************************/
/**
*  @brief   Clear keypad matrix in a row / column location
*
*  @param   matrix      (out) keypad row/column matrix.
*  @param   row         (in)  row (zero based enumeration)
*  @param   col         (in)  column (zero based enumeration)
*
*  @return  none
****************************************************************************/
static inline void chal_keypad_matrix_clear(CHAL_KEYPAD_MATRIX_t *matrix, unsigned row, unsigned col)
{
   matrix->scan.line[row] &= ~(1 << col);
}

/****************************************************************************/
/**
*  @brief   Clear all keypad matrix entries.
*
*  @param   matrix      (out) keypad row/column matrix.
*
*  @return  none
****************************************************************************/
static inline void chal_keypad_matrix_clear_all(CHAL_KEYPAD_MATRIX_t *matrix)
{
   matrix->scan.reg[0] = 0;
   matrix->scan.reg[1] = 0;
}

/****************************************************************************/
/**
*  @brief   Indicate if keypad matrix is set in a row / column location
*
*  @param   matrix      (in)  keypad row/column matrix.
*  @param   row         (in)  row (zero based enumeration)
*  @param   col         (in)  column (zero based enumeration)
*
*  @return  none
****************************************************************************/
static inline unsigned chal_keypad_matrix_is_set(CHAL_KEYPAD_MATRIX_t *matrix, unsigned row, unsigned col)
{
   return( matrix->scan.line[row] & (1 << col) );
}

/****************************************************************************/
/**
*  @brief   Set keypad matrix in a row / column location
*
*  @param   matrix      (out) keypad row/column matrix.
*  @param   row         (in)  row (zero based enumeration)
*  @param   col         (in)  column (zero based enumeration)
*
*  @return  none
****************************************************************************/
static inline void chal_keypad_matrix_set(CHAL_KEYPAD_MATRIX_t *matrix, unsigned row, unsigned col)
{
   matrix->scan.line[row] |= (1 << col);
}

/****************************************************************************/
/**
*  @brief   Set all keypad matrix entries in row / column dimensions.
*
*  @param   matrix      (out) keypad row/column matrix.
*  @param   rowCnt      (in)  row count
*  @param   colCnt      (in)  column count
*
*  @return  none
****************************************************************************/
static inline void chal_keypad_matrix_set_all(CHAL_KEYPAD_MATRIX_t *matrix, unsigned rowCnt, unsigned colCnt)
{
   uint32_t    colMask;
   unsigned    rowIdx;


   /* Convert the column count into a mask reflecting the active columns in a row.
    * Use this mask for the specified row count. Clear the remaining unsed rows.
    */
   colMask = (1 << colCnt) - 1;

   for ( rowIdx = 0; rowIdx < rowCnt; rowIdx++ )
   {
      matrix->scan.line[rowIdx] = colMask;
   }
   for ( ; rowIdx < CHAL_KEYPAD_SCAN_ROW_CNT; rowIdx++ )
   {
      matrix->scan.line[rowIdx] = 0;
   }
}


#ifdef __cplusplus
}
#endif

#endif /* _CHAL_KEYPAD_H */


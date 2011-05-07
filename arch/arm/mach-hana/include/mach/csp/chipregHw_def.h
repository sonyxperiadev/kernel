/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
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
#ifndef CHIPREGHW_DEF_H
#define CHIPREGHW_DEF_H

/* ---- Include Files ----------------------------------------------------- */

#include <mach/csp/chipregHw_reg.h>

/* ---- Public Constants and Types ---------------------------------------- */

typedef enum {
  chipregHw_STRAP_ASIC_MODE_BIGISLAND    = 0x0,
  chipregHw_STRAP_ASIC_MODE_HANA         = 0x1,
  chipregHw_STRAP_ASIC_MODE_LITTLEISLAND = 0x2
 } chipregHw_STRAP_ASIC_MODE_e;

typedef enum {
  chipregHw_STRAP_XTAL_TYPE_13MHZ    = 0x0,
  chipregHw_STRAP_XTAL_TYPE_26MHZ    = 0x1,
  chipregHw_STRAP_XTAL_TYPE_19p2MHZ  = 0x2,
  chipregHw_STRAP_XTAL_TYPE_38p4MHZ  = 0x3
 } chipregHw_STRAP_XTAL_TYPE_e;

typedef enum {
  chipregHw_STRAP_BOOT_OPTIONS_NOR_FLASH    = 0x0,
  chipregHw_STRAP_BOOT_OPTIONS_SD_MMC       = 0x1,
  chipregHw_STRAP_BOOT_OPTIONS_NAND_FLASH   = 0x2,
  chipregHw_STRAP_BOOT_OPTIONS_SERIAL_FLASH = 0x3,
  chipregHw_STRAP_BOOT_OPTIONS_UART         = 0x4,
  chipregHw_STRAP_BOOT_OPTIONS_USB_OTG      = 0x5
 } chipregHw_STRAP_BOOT_OPTIONS_e;

/* Pin function - refer to pinmux spreadsheet */
typedef enum {
  chipregHw_PIN_FUNCTION_ALT01 = chipregHw_REG_PIN_MUX_ALT01,
  chipregHw_PIN_FUNCTION_ALT02 = chipregHw_REG_PIN_MUX_ALT02,
  chipregHw_PIN_FUNCTION_ALT03 = chipregHw_REG_PIN_MUX_ALT03,
  chipregHw_PIN_FUNCTION_ALT04 = chipregHw_REG_PIN_MUX_ALT04,
  chipregHw_PIN_FUNCTION_ALT05 = chipregHw_REG_PIN_MUX_ALT05,
  chipregHw_PIN_FUNCTION_ALT06 = chipregHw_REG_PIN_MUX_ALT06,
} chipregHw_PIN_FUNCTION_e;

/* PIN Output slew rate */
typedef enum {
  chipregHw_PIN_SLEW_RATE_HIGH          =    chipregHw_REG_SLEW_RATE_HIGH,
  chipregHw_PIN_SLEW_RATE_LOW           =    chipregHw_REG_SLEW_RATE_LOW
} chipregHw_PIN_SLEW_RATE_e;

/* PIN Current drive strength */
typedef enum {
  chipregHw_PIN_CURRENT_STRENGTH_02mA   =    chipregHw_REG_CURRENT_STRENGTH_02mA,
  chipregHw_PIN_CURRENT_STRENGTH_04mA   =    chipregHw_REG_CURRENT_STRENGTH_04mA,
  chipregHw_PIN_CURRENT_STRENGTH_06mA   =    chipregHw_REG_CURRENT_STRENGTH_06mA,
  chipregHw_PIN_CURRENT_STRENGTH_08mA   =    chipregHw_REG_CURRENT_STRENGTH_08mA,
  chipregHw_PIN_CURRENT_STRENGTH_10mA   =    chipregHw_REG_CURRENT_STRENGTH_10mA,
  chipregHw_PIN_CURRENT_STRENGTH_12mA   =    chipregHw_REG_CURRENT_STRENGTH_12mA,
  chipregHw_PIN_CURRENT_STRENGTH_14mA   =    chipregHw_REG_CURRENT_STRENGTH_14mA,
  chipregHw_PIN_CURRENT_STRENGTH_16mA   =    chipregHw_REG_CURRENT_STRENGTH_16mA
} chipregHw_PIN_CURRENT_STRENGTH_e;


/* PIN Pull up register settings */
typedef enum {
  chipregHw_PIN_PULL_NONE               =    chipregHw_REG_PULL_NONE,
  chipregHw_PIN_PULL_UP                 =    chipregHw_REG_PULL_UP,
  chipregHw_PIN_PULL_DOWN               =    chipregHw_REG_PULL_DOWN
} chipregHw_PIN_PULL_e;


/* PIN input type settings */
typedef enum {
  chipregHw_PIN_INPUTTYPE_TTL           =    chipregHw_REG_INPUTTYPE_TTL,
  chipregHw_PIN_INPUTTYPE_ST            =    chipregHw_REG_INPUTTYPE_ST
} chipregHw_PIN_INPUTTYPE_e;

/* ---- Public Variable Externs ------------------------------------------ */
/* ---- Public Function Prototypes --------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#endif /* CHIPREGHW_DEF_H */

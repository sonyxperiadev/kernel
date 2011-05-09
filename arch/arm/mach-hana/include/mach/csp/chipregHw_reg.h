/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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
*  @file    chipregHw_reg.h
*
*  @brief   Definitions for low level chip control registers
*
*/
/****************************************************************************/
#ifndef chipregHw_REG_H
#define chipregHw_REG_H

#include <mach/csp/chal_types.h>
#include <mach/csp/reg.h>
#include <mach/csp/mm_io.h>

#define chipregHw_BASE_ADDRESS    MM_IO_BASE_CHIPREG

#define chipregHw_BOOT_2ND_ADDR_OFFSET		0x0000017C

#define CHIPREG_CHIPID_REG_OFFSET                                         0x00000000
#define CHIPREG_ISLAND_ID_OFFSET                                          0x00000008
#define CHIPREG_ISLAND_STRAP_OFFSET                                       0x0000000C
#define CHIPREG_CHIP_SW_STRAP_OFFSET                                      0x00000100
#define    CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_SHIFT                        31
#define    CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK                         (1<<CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_SHIFT)

#define chipregHw_REG_STRAP_XTAL_OSC_BYPASS_MASK         (1 << chipregHw_REG_STRAP_XTAL_OSC_BYPASS_SHIFT)
#define chipregHw_REG_STRAP_XTAL_OSC_BYPASS_SHIFT        10
#define chipregHw_REG_STRAP_ASIC_MODE_MASK               (0x3 << chipregHw_REG_STRAP_ASIC_MODE_SHIFT)
#define chipregHw_REG_STRAP_ASIC_MODE_SHIFT              8
#define chipregHw_REG_STRAP_XTAL_TYPE_MASK               (0x3 << chipregHw_REG_STRAP_XTAL_TYPE_SHIFT)
#define chipregHw_REG_STRAP_XTAL_TYPE_SHIFT              6

#define chipregHw_REG_STRAP_BOOT_OPTIONS_MASK            (0x7 << chipregHw_REG_STRAP_BOOT_OPTIONS_SHIFT)
#define chipregHw_REG_STRAP_BOOT_OPTIONS_SHIFT           3
#define chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_MASK     (1 << chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_SHIFT)
#define chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_SHIFT    2
#define chipregHw_REG_STRAP_TMACT_MASK                   (1 << chipregHw_REG_STRAP_TMACT_SHIFT)
#define chipregHw_REG_STRAP_TMACT_SHIFT                  1
#define chipregHw_REG_STRAP_FLASHBOOT_MASK               1
#define chipregHw_REG_STRAP_FLASHBOOT_SHIFT              0

#define chipregHw_REG_SLEW_RATE_HIGH                     0x00000000	/* High speed slew rate */
#define chipregHw_REG_SLEW_RATE_LOW                      0x00000001	/* Slow slew rate */
#define chipregHw_REG_PIN_SLEW_RATE_POSITION             4
#define chipregHw_REG_PIN_SLEW_RATE_MASK                 (1 << chipregHw_REG_PIN_SLEW_RATE_POSITION)

#define chipregHw_REG_CURRENT_STRENGTH_02mA              0x00000000
#define chipregHw_REG_CURRENT_STRENGTH_04mA              0x00000001
#define chipregHw_REG_CURRENT_STRENGTH_06mA              0x00000002
#define chipregHw_REG_CURRENT_STRENGTH_08mA              0x00000003
#define chipregHw_REG_CURRENT_STRENGTH_10mA              0x00000004
#define chipregHw_REG_CURRENT_STRENGTH_12mA              0x00000005
#define chipregHw_REG_CURRENT_STRENGTH_14mA              0x00000006
#define chipregHw_REG_CURRENT_STRENGTH_16mA              0x00000007
#define chipregHw_REG_PIN_CURRENT_STRENGTH_POSITION      0
#define chipregHw_REG_PIN_CURRENT_STRENGTH_MASK          0x7

#define chipregHw_REG_PULL_NONE                          0x00000000	/* No pull up register */
#define chipregHw_REG_PULL_UP                            0x00000001	/* Pull up register enable */
#define chipregHw_REG_PULL_DOWN                          0x00000002	/* Pull down register enable */
#define chipregHw_REG_PULL_POSITION                      5
#define chipregHw_REG_PULL_MASK                          (0x3 << chipregHw_REG_PULL_POSITION)

#define chipregHw_REG_INPUTTYPE_TTL                      0x00000000	/* Normal CMOS logic */
#define chipregHw_REG_INPUTTYPE_ST                       0x00000001	/* High speed Schmitt Trigger */
#define chipregHw_REG_PIN_INPUTTYPE_POSITION             7
#define chipregHw_REG_PIN_INPUTTYPE_MASK                 (1 << chipregHw_REG_PIN_INPUTTYPE_POSITION)

#define chipregHw_REG_PIN_MUX_ALT01                      0x00000000
#define chipregHw_REG_PIN_MUX_ALT02                      0x00000001
#define chipregHw_REG_PIN_MUX_ALT03                      0x00000002
#define chipregHw_REG_PIN_MUX_ALT04                      0x00000003	/* GPIO for pins 0-191 */
#define chipregHw_REG_PIN_MUX_ALT05                      0x00000004
#define chipregHw_REG_PIN_MUX_ALT06                      0x00000005
#define chipregHw_REG_PIN_MUX_POSITION                   8
#define chipregHw_REG_PIN_MUX_MASK                       (0x7 << chipregHw_REG_PIN_MUX_POSITION)

typedef struct
{
    uint32_t pin_padctrl_addr;
    uint32_t pin_function;
    uint32_t pin_input_type;
    uint32_t pin_pull;
    uint32_t pin_curr_strength;
    uint32_t pin_slew_rate;
} chipregHw_pin_info_t;

extern chipregHw_pin_info_t PIN_DEFAULT_INFO[];

#define chipregHw_REG_PIN_CONTROL(pin)                   ( (volatile uint32_t*)(chipregHw_BASE_ADDRESS + PIN_DEFAULT_INFO[pin].pin_padctrl_addr) )
#define CHIPREGHW_PIN_FUNCTION_DEFAULT(pin)              ( PIN_DEFAULT_INFO[pin].pin_function )
#define CHIPREGHW_PIN_SLEW_RATE_DEFAULT(pin)             ( PIN_DEFAULT_INFO[pin].pin_slew_rate )
#define CHIPREGHW_PIN_CURRENT_STRENGTH_DEFAULT(pin)      ( PIN_DEFAULT_INFO[pin].pin_curr_strength )
#define CHIPREGHW_PIN_PULL_DEFAULT(pin)                  ( PIN_DEFAULT_INFO[pin].pin_pull )
#define CHIPREGHW_PIN_INPUTTYPE_DEFAULT(pin)             ( PIN_DEFAULT_INFO[pin].pin_input_type )

#endif /* chipregHw_REG_H */

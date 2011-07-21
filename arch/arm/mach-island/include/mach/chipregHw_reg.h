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

#include <plat/chal/chal_types.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_map.h>
#include <mach/io_map.h>

#define chipregHw_BASE_ADDRESS    KONA_CHIPREG_VA

#define chipregHw_REG_STRAP_XTAL_OSC_BYPASS_MASK         (CHIPREG_ISLAND_STRAP_STRAP_IN_10_MASK)
#define chipregHw_REG_STRAP_XTAL_OSC_BYPASS_SHIFT        (CHIPREG_ISLAND_STRAP_STRAP_IN_10_SHIFT)
#define chipregHw_REG_STRAP_ASIC_MODE_MASK               (CHIPREG_ISLAND_STRAP_STRAP_IN_9_MASK | CHIPREG_ISLAND_STRAP_STRAP_IN_8_MASK)
#define chipregHw_REG_STRAP_ASIC_MODE_SHIFT              (CHIPREG_ISLAND_STRAP_STRAP_IN_8_SHIFT)
#define chipregHw_REG_STRAP_XTAL_TYPE_MASK               (CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_MASK)
#define chipregHw_REG_STRAP_XTAL_TYPE_SHIFT              (CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_SHIFT)
#define chipregHw_REG_STRAP_BOOT_OPTIONS_MASK            (CHIPREG_ISLAND_STRAP_STRAP_IN_5TO3_MASK)
#define chipregHw_REG_STRAP_BOOT_OPTIONS_SHIFT           (CHIPREG_ISLAND_STRAP_STRAP_IN_5TO3_SHIFT)
#define chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_MASK     (CHIPREG_ISLAND_STRAP_STRAP_IN_2_MASK)
#define chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_SHIFT    (CHIPREG_ISLAND_STRAP_STRAP_IN_2_SHIFT)
#define chipregHw_REG_STRAP_TMACT_MASK                   (CHIPREG_ISLAND_STRAP_STRAP_IN_1_MASK)
#define chipregHw_REG_STRAP_TMACT_SHIFT                  (CHIPREG_ISLAND_STRAP_STRAP_IN_1_SHIFT)
#define chipregHw_REG_STRAP_FLASHBOOT_MASK               (CHIPREG_ISLAND_STRAP_STRAP_IN_0_MASK)
#define chipregHw_REG_STRAP_FLASHBOOT_SHIFT              (CHIPREG_ISLAND_STRAP_STRAP_IN_0_SHIFT)

#define chipregHw_REG_SLEW_RATE_HIGH                     0x00000000	/* High speed slew rate */
#define chipregHw_REG_SLEW_RATE_LOW                      0x00000001	/* Slow slew rate */
#define chipregHw_REG_PIN_SLEW_RATE_POSITION             CHIPREG_GPIO_0_SRC_SHIFT
#define chipregHw_REG_PIN_SLEW_RATE_MASK                 CHIPREG_GPIO_0_SRC_MASK

#define chipregHw_REG_CURRENT_STRENGTH_02mA              0x00000000	/* Current driving strength 2 milli ampere */
#define chipregHw_REG_CURRENT_STRENGTH_04mA              0x00000001	/* Current driving strength 4 milli ampere */
#define chipregHw_REG_CURRENT_STRENGTH_06mA              0x00000002	/* Current driving strength 6 milli ampere */
#define chipregHw_REG_CURRENT_STRENGTH_08mA              0x00000003	/* Current driving strength 8 milli ampere */
#define chipregHw_REG_CURRENT_STRENGTH_10mA              0x00000004	/* Current driving strength 10 milli ampere */
#define chipregHw_REG_CURRENT_STRENGTH_12mA              0x00000005	/* Current driving strength 12 milli ampere */
#define chipregHw_REG_CURRENT_STRENGTH_14mA              0x00000006	/* Current driving strength 14 milli ampere */
#define chipregHw_REG_CURRENT_STRENGTH_16mA              0x00000007	/* Current driving strength 16 milli ampere */
#define chipregHw_REG_PIN_CURRENT_STRENGTH_POSITION      CHIPREG_GPIO_0_SEL_SHIFT
#define chipregHw_REG_PIN_CURRENT_STRENGTH_MASK          CHIPREG_GPIO_0_SEL_MASK

#define chipregHw_REG_PULL_NONE                          0x00000000	/* No pull up register */
#define chipregHw_REG_PULL_UP                            0x00000001	/* Pull up register enable */
#define chipregHw_REG_PULL_DOWN                          0x00000002	/* Pull down register enable */
#define chipregHw_REG_PULL_POSITION                      CHIPREG_GPIO_0_PUP_SHIFT
#define chipregHw_REG_PULL_MASK                          (CHIPREG_GPIO_0_PUP_MASK | CHIPREG_GPIO_0_PDN_MASK)

#define chipregHw_REG_INPUTTYPE_TTL                      0x00000000	/* Normal CMOS logic */
#define chipregHw_REG_INPUTTYPE_ST                       0x00000001	/* High speed Schmitt Trigger */
#define chipregHw_REG_PIN_INPUTTYPE_POSITION             CHIPREG_GPIO_0_HYS_EN_SHIFT
#define chipregHw_REG_PIN_INPUTTYPE_MASK                 CHIPREG_GPIO_0_HYS_EN_MASK

#define chipregHw_REG_PIN_MUX_ALT01                      0x00000000
#define chipregHw_REG_PIN_MUX_ALT02                      0x00000001
#define chipregHw_REG_PIN_MUX_ALT03                      0x00000002
#define chipregHw_REG_PIN_MUX_ALT04                      0x00000003	/* GPIO for pins 0-191 */
#define chipregHw_REG_PIN_MUX_ALT05                      0x00000004
#define chipregHw_REG_PIN_MUX_ALT06                      0x00000005
#define chipregHw_REG_PIN_MUX_POSITION                   CHIPREG_GPIO_0_PINSEL_2_0_SHIFT
#define chipregHw_REG_PIN_MUX_MASK                       CHIPREG_GPIO_0_PINSEL_2_0_MASK

/* The following defines are used for HANA only */
#define chipregHw_REG_I2C_PULL_NONE                      0x00000000	/* No pull up register */
#define chipregHw_REG_I2C_PULL_UP                        0x00000001	/* Pull up register enable */
#define chipregHw_REG_I2C_PULL_POSITION                  CHIPREG_VC_CAM1_SCL_PUP_SHIFT
#define chipregHw_REG_I2C_PULL_MASK                      CHIPREG_VC_CAM1_SCL_PUP_MASK

#define chipregHw_REG_I2C_SRC_RATE_HIGHSPEED             0x00000000
#define chipregHw_REG_I2C_SRC_RATE_SLOWSPEED             0x00000001
#define chipregHw_REG_I2C_SRC_RATE_POSITION              CHIPREG_VC_CAM1_SCL_SRC_SHIFT
#define chipregHw_REG_I2C_SRC_RATE_MASK                  CHIPREG_VC_CAM1_SCL_SRC_MASK

#define chipregHw_REG_I2C_MODE_NONHIGHSPEED              0x00000000
#define chipregHw_REG_I2C_MODE_HIGHSPEED                 0x00000001
#define chipregHw_REG_I2C_MODE_POSITION                  CHIPREG_VC_CAM1_SCL_SEL_SHIFT
#define chipregHw_REG_I2C_MODE_MASK                      CHIPREG_VC_CAM1_SCL_SEL_MASK

#define chipregHw_REG_I2C_INPUT_CTRL_ENABLE              0x00000000
#define chipregHw_REG_I2C_INPUT_CTRL_DISABLE             0x00000001
#define chipregHw_REG_I2C_INPUT_CTRL_POSITION            CHIPREG_VC_CAM1_SCL_IND_SHIFT
#define chipregHw_REG_I2C_INPUT_CTRL_MASK                CHIPREG_VC_CAM1_SCL_IND_MASK

#define chipregHw_REG_SLV_64BIT_MODE_POSITION            CHIPREG_ARM_PERI_CONTROL_SLV_TIMER_64BITMODE_SHIFT        
#define chipregHw_REG_SLV_64BIT_MODE_MASK                CHIPREG_ARM_PERI_CONTROL_SLV_TIMER_64BITMODE_MASK
#define chipregHw_REG_HUB_64BIT_MODE_POSITION            CHIPREG_ARM_PERI_CONTROL_HUB_TIMER_64BITMODE_SHIFT
#define chipregHw_REG_HUB_64BIT_MODE_MASK                CHIPREG_ARM_PERI_CONTROL_HUB_TIMER_64BITMODE_MASK
#define chipregHw_REG_ARM_PERI_CONTROL_OFFSET            CHIPREG_ARM_PERI_CONTROL_OFFSET

#endif /* chipregHw_REG_H */

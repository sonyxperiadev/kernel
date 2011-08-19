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

#ifndef CHIPREG_INLINE_H
#define CHIPREG_INLINE_H

/* ---- Include Files ----------------------------------------------------- */

#include <plat/chal/chal_types.h>
#include <mach/reg_utils.h>
#include <mach/chipregHw_reg.h>

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


/****************************************************************************/
/**
*  @brief   Get Numeric Chip ID
*
*  This function returns Chip ID
*
*  @return  Complete numeric Chip ID
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getChipId(void)
{
    return reg32_read( (volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIPID_REG_OFFSET ));
}

/****************************************************************************/
/**
*  @brief   Get Island ID
*
*  This function returns Island ID
*
*  @return  Complete Island ID
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getIslandId(void)
{
    return reg32_read( (volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_ISLAND_ID_OFFSET ));
}

static inline uint32_t chipregHw_getChipIdNum(void)
{
    return reg32_read((volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIPID_REG_OFFSET)) >> 12;
}

static inline uint32_t chipregHw_getChipIdRev(void)
{
    return reg32_read((volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIPID_REG_OFFSET)) & 0xff;
}

/****************************************************************************/
/**
*  @brief   Get Hardware Straps
*
*  This function returns Hardware Straps
*
*  @return  Hardware Straps
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getHardwareStraps(void)
{
    return reg32_read( (volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_ISLAND_STRAP_OFFSET ));
}

/****************************************************************************/
/**
*  @brief   Get Software Straps
*
*  This function returns Software Straps
*
*  @return  Software Straps
*
*/
/****************************************************************************/
static inline uint32_t chipregHw_getSoftwareStraps(void)
{
    return reg32_read( (volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ));
}

/****************************************************************************/
/**
*  @brief   Set Software Straps
*
*  This function sets Software Straps
*
*/
/****************************************************************************/
static inline void chipregHw_setSoftwareStraps(uint32_t strapOptions)
{
    reg32_write( (volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ), strapOptions);
}

/****************************************************************************/
/**  @brief Checks if software strap is enabled
 *
 *   @return 1 : When enable
        0 : When disable
 */
/****************************************************************************/
static inline _Bool chipregHw_isSoftwareStrapsEnable(void)
{
    return (chipregHw_getSoftwareStraps() & CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK) >> CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_SHIFT;
}


/****************************************************************************/
/**  @brief Enable software strap
 */
/****************************************************************************/
static inline void chipregHw_softwareStrapsEnable(void)
{
    reg32_set_bits((volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ), CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK);
}


/****************************************************************************/
/**  @brief Disable software strap
 */
/****************************************************************************/
static inline void chipregHw_softwareStrapsDisable(void)
{
    reg32_clear_bits((volatile uint32_t*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ), CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK);
}

/****************************************************************************/
/**  @brief Get Crytal Oscillator Bypass strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapXtalOscBypass(void)
{
    if ( chipregHw_isSoftwareStrapsEnable())
    {
        return ( (chipregHw_getSoftwareStraps() & chipregHw_REG_STRAP_XTAL_OSC_BYPASS_MASK)
                      >> chipregHw_REG_STRAP_XTAL_OSC_BYPASS_SHIFT);
    }
    else
    {
        return ( (chipregHw_getHardwareStraps() & chipregHw_REG_STRAP_XTAL_OSC_BYPASS_MASK)
                      >> chipregHw_REG_STRAP_XTAL_OSC_BYPASS_SHIFT);
    }
}

/****************************************************************************/
/**  @brief Get ASIC mode strap
 */
/****************************************************************************/
static inline chipregHw_STRAP_ASIC_MODE_e chipregHw_getStrapAsicMode(void)
{
    if ( chipregHw_isSoftwareStrapsEnable())
    {
        return ( (chipregHw_getSoftwareStraps() & chipregHw_REG_STRAP_ASIC_MODE_MASK)
                      >> chipregHw_REG_STRAP_ASIC_MODE_SHIFT);
    }
    else
    {
        return ( (chipregHw_getHardwareStraps() & chipregHw_REG_STRAP_ASIC_MODE_MASK)
                      >> chipregHw_REG_STRAP_ASIC_MODE_SHIFT);
    }
}

/****************************************************************************/
/**  @brief Get crystal type strap
 */
/****************************************************************************/
static inline chipregHw_STRAP_XTAL_TYPE_e chipregHw_getStrapXtalType(void)
{
    if ( chipregHw_isSoftwareStrapsEnable())
    {
        return ( (chipregHw_getSoftwareStraps() & chipregHw_REG_STRAP_XTAL_TYPE_MASK)
                      >> chipregHw_REG_STRAP_XTAL_TYPE_SHIFT);
    }
    else
    {
        return ( (chipregHw_getHardwareStraps() & chipregHw_REG_STRAP_XTAL_TYPE_MASK)
                      >> chipregHw_REG_STRAP_XTAL_TYPE_SHIFT);
    }
}

/****************************************************************************/
/**  @brief Get boot mode strap
 */
/****************************************************************************/
static inline chipregHw_STRAP_BOOT_OPTIONS_e chipregHw_getStrapBootMode(void)
{
    if ( chipregHw_isSoftwareStrapsEnable())
    {
        return ( (chipregHw_getSoftwareStraps() & chipregHw_REG_STRAP_BOOT_OPTIONS_MASK)
                      >> chipregHw_REG_STRAP_BOOT_OPTIONS_SHIFT);
    }
    else
    {
        return ( (chipregHw_getHardwareStraps() & chipregHw_REG_STRAP_BOOT_OPTIONS_MASK)
                      >> chipregHw_REG_STRAP_BOOT_OPTIONS_SHIFT);
    }
}

/****************************************************************************/
/**  @brief Get OTP Program and Status strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapOtpProgMode(void)
{
    if ( chipregHw_isSoftwareStrapsEnable())
    {
        return ( (chipregHw_getSoftwareStraps() & chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_MASK)
                      >> chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_SHIFT);
    }
    else
    {
        return ( (chipregHw_getHardwareStraps() & chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_MASK)
                      >> chipregHw_REG_STRAP_OTP_PROG_AND_STATUS_SHIFT);
    }
}

/****************************************************************************/
/**  @brief Get TMACT strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapTmact(void)
{
    if ( chipregHw_isSoftwareStrapsEnable())
    {
        return ( (chipregHw_getSoftwareStraps() & chipregHw_REG_STRAP_TMACT_MASK)
                      >> chipregHw_REG_STRAP_TMACT_SHIFT);
    }
    else
    {
        return ( (chipregHw_getHardwareStraps() & chipregHw_REG_STRAP_TMACT_MASK)
                      >> chipregHw_REG_STRAP_TMACT_SHIFT);
    }
}

/****************************************************************************/
/**  @brief Get flashboot strap
 */
/****************************************************************************/
static inline _Bool chipregHw_getStrapFlashboot(void)
{
    if ( chipregHw_isSoftwareStrapsEnable())
    {
        return ( (chipregHw_getSoftwareStraps() & chipregHw_REG_STRAP_FLASHBOOT_MASK)
                      >> chipregHw_REG_STRAP_FLASHBOOT_SHIFT);
    }
    else
    {
        return ( (chipregHw_getHardwareStraps() & chipregHw_REG_STRAP_FLASHBOOT_MASK)
                      >> chipregHw_REG_STRAP_FLASHBOOT_SHIFT);
    }
}

#endif /* CHIPREG_INLINE_H */

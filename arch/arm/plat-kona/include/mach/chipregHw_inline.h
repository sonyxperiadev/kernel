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

// SARU #include <mach/csp/reg.h>
// SARU #include <mach/csp/chal_types.h>
#include <mach/chipregHw_reg.h>
#include <mach/chipregHw_def.h>
#include <mach/gpio_defs.h>
/* ---- Private Constants and Types --------------------------------------- */
/* ---- Public Constants and Types ---------------------------------------- */
/* ---- Public Variable Externs ------------------------------------------- */
/* ---- Public Function Prototypes ---------------------------------------- */
/* ---- Private Function Prototypes --------------------------------------- */

static inline void reg32_clear_bits(volatile unsigned int *reg, unsigned int value)
{
    *reg &= ~(value);
}

static inline void reg32_set_bits(volatile unsigned int *reg, unsigned int value)
{
    *reg |= value;
}

static inline void reg32_toggle_bits(volatile unsigned int *reg, unsigned int value)
{
    *reg ^= value;
}

static inline void reg32_write_masked(volatile unsigned int *reg, unsigned int mask, unsigned int value)
{
    *reg = (*reg & (~mask)) | (value & mask);
}

static inline void reg32_write(volatile unsigned int *reg, unsigned int value)
{
    *reg = value;
}

static inline unsigned int reg32_read(volatile unsigned int *reg)
{
    return *reg;
}

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
static inline unsigned int chipregHw_getChipId(void)
{
    return reg32_read( (volatile unsigned int*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIPID_REG_OFFSET ));
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
static inline unsigned int chipregHw_getIslandId(void)
{
    return reg32_read( (volatile unsigned int*)(chipregHw_BASE_ADDRESS + CHIPREG_ISLAND_ID_OFFSET ));
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
static inline unsigned int chipregHw_getHardwareStraps(void)
{
    return reg32_read( (volatile unsigned int*)(chipregHw_BASE_ADDRESS + CHIPREG_ISLAND_STRAP_OFFSET ));
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
static inline unsigned int chipregHw_getSoftwareStraps(void)
{
    return reg32_read( (volatile unsigned int*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ));
}

/****************************************************************************/
/**
*  @brief   Set Software Straps
*
*  This function sets Software Straps
*
*/
/****************************************************************************/
static inline void chipregHw_setSoftwareStraps(unsigned int strapOptions)
{
    reg32_write( (volatile unsigned int*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ), strapOptions);
}

/****************************************************************************/
/**  @brief Checks if software strap is enabled
 *
 *   @return 1 : When enable
        0 : When disable
 */
/****************************************************************************/
static inline unsigned char chipregHw_isSoftwareStrapsEnable(void)
{
    return (chipregHw_getSoftwareStraps() & CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK) >> CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_SHIFT;
}


/****************************************************************************/
/**  @brief Enable software strap
 */
/****************************************************************************/
static inline void chipregHw_softwareStrapsEnable(void)
{
    reg32_set_bits((volatile unsigned int*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ), CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK);
}


/****************************************************************************/
/**  @brief Disable software strap
 */
/****************************************************************************/
static inline void chipregHw_softwareStrapsDisable(void)
{
    reg32_clear_bits((volatile unsigned int*)(chipregHw_BASE_ADDRESS + CHIPREG_CHIP_SW_STRAP_OFFSET ), CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK);
}

/****************************************************************************/
/**  @brief Get Crytal Oscillator Bypass strap
 */
/****************************************************************************/
static inline unsigned char chipregHw_getStrapXtalOscBypass(void)
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
static inline unsigned char chipregHw_getStrapOtpProgMode(void)
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
static inline unsigned char chipregHw_getStrapTmact(void)
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
static inline unsigned char chipregHw_getStrapFlashboot(void)
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

/****************************************************************************/
/**
*  @brief   Get to know the configuration of pin
*
*/
/****************************************************************************/
static inline chipregHw_PIN_FUNCTION_e chipregHw_getPinFunction(int pin)
{
   volatile unsigned int* reg;
   chipregHw_PIN_FUNCTION_e rc = -1;

   if ( pin >= 0 && pin <= gpio_defs_MAX_PINS )
   {
      reg = chipregHw_REG_PIN_CONTROL(pin);
   	rc = ( (reg32_read(reg) & chipregHw_REG_PIN_MUX_MASK) >> chipregHw_REG_PIN_MUX_POSITION );
   }

   return rc;
}


/****************************************************************************/
/**
*  @brief   Configure pin function
*
*/
/****************************************************************************/
static inline void chipregHw_setPinFunction(int pin, chipregHw_PIN_FUNCTION_e func)
{
   volatile unsigned int* reg;

   if ( pin >= 0 && pin <= gpio_defs_MAX_PINS )
   {
      reg = chipregHw_REG_PIN_CONTROL(pin);

      reg32_clear_bits(reg, chipregHw_REG_PIN_MUX_MASK);
      reg32_write_masked(reg, chipregHw_REG_PIN_MUX_MASK, func << chipregHw_REG_PIN_MUX_POSITION );
   }
}

/****************************************************************************/
/**
*  @brief   Set Pin slew rate
*
*  This function sets the slew of individual pin
*
*/
/****************************************************************************/
static inline void chipregHw_setPinSlewRate(int pin, chipregHw_PIN_SLEW_RATE_e slewRate)
{
   volatile unsigned int* reg;

   if ( pin >= 0 && pin <= gpio_defs_MAX_PINS )
   {
      reg = chipregHw_REG_PIN_CONTROL(pin);

      reg32_clear_bits(reg, chipregHw_REG_PIN_SLEW_RATE_MASK);
      reg32_write_masked(reg, chipregHw_REG_PIN_SLEW_RATE_MASK, slewRate << chipregHw_REG_PIN_SLEW_RATE_POSITION );
   }
}

/****************************************************************************/
/**
*  @brief   Set Pin output drive current
*
*  This function sets output drive current of individual pin
*
*  Note: Avoid the use of the word 'current' since linux headers define this
*        to be the current task.
*/
/****************************************************************************/
static inline void chipregHw_setPinOutputCurrent(int pin, chipregHw_PIN_CURRENT_STRENGTH_e curr)
{
   volatile unsigned int* reg;

   if ( pin >= 0 && pin <= gpio_defs_MAX_PINS )
   {
      reg = chipregHw_REG_PIN_CONTROL(pin);

      reg32_clear_bits(reg, chipregHw_REG_PIN_CURRENT_STRENGTH_MASK);
      reg32_write_masked(reg, chipregHw_REG_PIN_CURRENT_STRENGTH_MASK, curr << chipregHw_REG_PIN_CURRENT_STRENGTH_POSITION );
   }
}


/****************************************************************************/
/**
*  @brief   Set Pin pullup register
*
*  This function sets pullup register of individual pin
*
*  @note: For "pin" use chipregHw_REG_PULL_PIN_XXXXX macros
*/  
/****************************************************************************/
static inline void chipregHw_setPinPullup(int pin, chipregHw_PIN_PULL_e pullup)
{
   volatile unsigned int* reg;

   if ( pin >= 0 && pin <= gpio_defs_MAX_PINS )
   {
      reg = chipregHw_REG_PIN_CONTROL(pin);

      reg32_clear_bits(reg, chipregHw_REG_PULL_MASK);
      reg32_write_masked(reg, chipregHw_REG_PULL_MASK, pullup << chipregHw_REG_PULL_POSITION );
   }
}

/****************************************************************************/
/**
*  @brief   Set Pin input type
*
*  This function sets input type of individual pin
*
*
*  @note: For "pin" use chipregHw_REG_INPUTTYPE_PIN_XXXXX macros
*/
/****************************************************************************/
static inline void chipregHw_setPinInputType(int pin, chipregHw_PIN_INPUTTYPE_e inputType)
{
   volatile unsigned int* reg;

   if ( pin >= 0 && pin <= gpio_defs_MAX_PINS )
   {
      reg = chipregHw_REG_PIN_CONTROL(pin);

      reg32_clear_bits(reg, chipregHw_REG_PIN_INPUTTYPE_MASK);
      reg32_write_masked(reg, chipregHw_REG_PIN_INPUTTYPE_MASK, inputType << chipregHw_REG_PIN_INPUTTYPE_POSITION );
   }
}


#endif /* CHIPREG_INLINE_H */

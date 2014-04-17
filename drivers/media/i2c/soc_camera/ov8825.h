/*
 * OmniVision OV5648 sensor driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __OV8825_H__
#define __OV8825_H__

#undef UINT8P8
#define UINT8P8(x)	((int)((x)*256))
#undef UINT24P8
#define UINT24P8(x)	((int)((x)*256))
#define F24p8(x)	((u32)((x)*256))
#define F4p7(x)		((u16)((x)*128))

#define INPUT_CLOCK_24MHZ 1

#define OV8825_REG_CHIP_ID_HIGH			0x300a
#define OV8825_REG_CHIP_ID_LOW			0x300b
/* Gain */
#define OV8825_GAIN_MIN					0
#define OV8825_GAIN_MAX					0x3FFF
#define OV8825_GAIN_STEP				1
#define OV8825_REG_AGC_HI				0x350A
#define OV8825_REG_AGC_LO				0x350B

/* Exp */
#define OV8825_EXP_MIN					0
#define OV8825_EXP_MAX					1000000
#define OV8825_EXP_STEP					1
#define OV8825_REG_EXP_HI				0x3500
#define OV8825_REG_EXP_MI				0x3501
#define OV8825_REG_EXP_LO				0x3502

/* VTS */
#define OV8825_REG_TIMING_VTS_HI                        0x380E
#define OV8825_REG_TIMING_VTS_LO                        0x380F

/* Lens */
#define OV8825_LENS_MIN					0
#define OV8825_LENS_MAX					255
#define OV8825_LENS_STEP				1
#define DEFAULT_LENS_POS				215

#define OV8825_APERTURE_MAX				255
#define OV8825_APERTURE_MIN				0
#define OV8825_APERTURE_STEP				1
#define DEFAULT_APERTURE				215
#define DEFAULT_EXPO			                32
#define DEFAULT_GAIN			                UINT8P8(1.0)

/* Flash */
#define OV8825_FLASH_INTENSITY_MIN 0
#define OV8825_FLASH_INTENSITY_MAX 255
#define OV8825_FLASH_INTENSITY_DEFAULT 255
#define OV8825_FLASH_INTENSITY_STEP 1

#define OV8825_FLASH_TIMEOUT_MIN 1
#define OV8825_FLASH_TIMEOUT_MAX 2
#define OV8825_FLASH_TIMEOUT_DEFAULT 1
#define OV8825_FLASH_TIMEOUT_STEP 1



#endif /* __OV5648_H__ */

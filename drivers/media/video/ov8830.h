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

#ifndef __OV8830_H__
#define __OV8830_H__

#undef UINT8P8
#define UINT8P8(x)	((int)((x)*256))
#undef UINT24P8
#define UINT24P8(x)	((int)((x)*256))
#define F24p8(x)	((u32)((x)*256))
#define F4p7(x)		((u16)((x)*128))

#define INPUT_CLOCK_24MHZ 1

#define OV8830_REG_CHIP_ID_HIGH			0x300a
#define OV8830_REG_CHIP_ID_LOW			0x300b
#define OV8830_CHIP_ID						0x8830

/* Frame */
#define OV8830_MCLK							13000000
#define OV8830_PCLK \
		151670000
#define OV8830_PREVIEW_LINE_LENGTH		0x0E18
#define OV8830_PREVIEW_FRAME_LENGTH		0x0579

#define OV8830_FULL_LINE_LENGTH \
	OV8830_PREVIEW_LINE_LENGTH
#define OV8830_FULL_FRAME_LENGTH			0x0AF1


/* Gain */
#define OV8830_GAIN_MIN					0
#define OV8830_GAIN_MAX					0x3FFF
#define OV8830_GAIN_STEP				1
#define OV8830_REG_AGC_HI				0x350A
#define OV8830_REG_AGC_LO				0x350B

/* Exp */
#define OV8830_EXP_MIN					0
#define OV8830_EXP_MAX					1000000
#define OV8830_EXP_STEP					1
#define OV8830_REG_EXP_HI				0x3500
#define OV8830_REG_EXP_MI				0x3501
#define OV8830_REG_EXP_LO				0x3502

/* VTS */
#define OV8830_REG_TIMING_VTS_HI                        0x380E
#define OV8830_REG_TIMING_VTS_LO                        0x380F

/* Lens */
#define OV8830_LENS_MIN					0
#define OV8830_LENS_MAX					255
#define OV8830_LENS_STEP				1
#define DEFAULT_LENS_POS				215

#define OV8830_APERTURE_MAX				255
#define OV8830_APERTURE_MIN				0
#define OV8830_APERTURE_STEP				1
#define DEFAULT_APERTURE				215
#define DEFAULT_EXPO			                50
#define DEFAULT_GAIN			                UINT8P8(1.0)

/* Flash */
#define OV8830_FLASH_INTENSITY_MIN 0
#define OV8830_FLASH_INTENSITY_MAX 1000
#define OV8830_FLASH_INTENSITY_DEFAULT 795
#define OV8830_FLASH_INTENSITY_STEP 1

#define OV8830_FLASH_TIMEOUT_MIN 2
#define OV8830_FLASH_TIMEOUT_MAX 1280
#define OV8830_FLASH_TIMEOUT_DEFAULT 72
#define OV8830_FLASH_TIMEOUT_STEP 2



#endif /* __OV5648_H__ */

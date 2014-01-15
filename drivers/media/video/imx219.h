/*
 * Sony IMX219 sensor driver
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

#ifndef __IMX219_H__
#define __IMX219_H__

#undef UINT8P8
#define UINT8P8(x)	((int)((x)*256))
#undef UINT24P8
#define UINT24P8(x)	((int)((x)*256))
#define F24p8(x)	((u32)((x)*256))
#define F4p7(x)		((u16)((x)*128))

#define INPUT_CLOCK_24MHZ 1
#define IMX219_REG_MODEL_ID_HIGH		0x0000
#define IMX219_REG_MODEL_ID_LOW			0x0001

/* Lens */
/* infinity, 0 current */
#define IMX219_LENS_MIN					0
/* macro, max current */
#define IMX219_LENS_MAX					255
#define IMX219_LENS_STEP				1

/* AEC */
#define IMX219_DEFAULT_EXP		                10000
#define IMX219_DEFAULT_GAIN		                UINT8P8(1.0)
#define IMX219_GAIN_MIN					UINT8P8(1.0)
#define IMX219_GAIN_MAX					UINT8P8(8.0)
#define IMX219_EXP_MIN					1
#define IMX219_ANA_GAIN_GLOBAL                          0x0157
#define IMX219_COARSE_INT_TIME_HI                       0x015A
#define IMX219_COARSE_INT_TIME_LO                       0x015B
#define IMX219_FRM_LENGTH_HI                            0x0160
#define IMX219_FRM_LENGTH_LO                            0x0161

#endif /* __IMX219_H_ */

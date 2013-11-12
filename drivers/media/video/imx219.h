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
/* macro max current */
#define IMX219_LENS_MAX					255
#define IMX219_LENS_STEP				1
#define IMX219_LENS_POS_DEFAULT				0

#endif /* __IMX219_H_ */

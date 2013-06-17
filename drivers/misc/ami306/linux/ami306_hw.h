/**
 * @file	ami306_hw.h
 * @brief	AMI306 hardware definition
 */
#ifndef AMI306_HW_H
#define AMI306_HW_H

#if 0 /* defined in the platform data */
/* I2C */
#define	AMI_I2C_BUS_NUM			4	/* pandaboard */
#define AMI_I2C_ADDRESS			0x0E	/* pandaboard */

/* GPIO */
#define AMI_GPIO_INT			0
#define AMI_GPIO_DRDY			0

/* sensor position */
#define AMI_DIR				6
#define AMI_POLARITY			7
#endif
/*
 * AMI-Sensor Internal Register Address
 * (Please refer to AMI-Sensor Specifications)
 */
#define AMI_REG_INFO			0x0d
#define AMI_REG_WIA			0x0f
#define AMI_REG_DATAX			0x10
#define AMI_REG_DATAY			0x12
#define AMI_REG_DATAZ			0x14
#define AMI_REG_STA1			0x18
#define AMI_REG_CTRL1			0x1b
#define AMI_REG_CTRL2			0x1c
#define AMI_REG_CTRL3			0x1d
#define AMI_REG_CTRL4			0x5c
#define AMI_REG_TEMP			0x60
#define AMI_REG_OFFX			0x6c
#define AMI_REG_OFFY			0x72
#define AMI_REG_OFFZ			0x78
#define AMI_REG_FINEOUTPUT_X		0x90
#define AMI_REG_FINEOUTPUT_Y		0x92
#define AMI_REG_FINEOUTPUT_Z		0x94
#define AMI_REG_SENX			0x96
#define AMI_REG_SENY			0x98
#define AMI_REG_SENZ			0x9a
#define AMI_REG_GAINX			0x9c
#define AMI_REG_GAINY			0x9e
#define AMI_REG_GAINZ			0xa0
#define AMI_REG_VER			0xe8
#define AMI_REG_SN			0xea
#define AMI_REG_OFFOTPX			0xf8
#define AMI_REG_OFFOTPY			0xfa
#define AMI_REG_OFFOTPZ			0xfc

/* AMI-Sensor Control Bit  (Please refer to AMI-Sensor Specifications) */
#define AMI_CTRL1_PC1			0x80
#define AMI_CTRL1_FS1_FORCE		0x02
#define AMI_CTRL1_ODR1			0x10
#define AMI_CTRL2_DREN			0x08
#define AMI_CTRL2_DRP			0x04
#define AMI_CTRL3_FORCE_BIT		0x40
#define AMI_CTRL3_B0_LO_BIT		0x10
#define AMI_CTRL3_SRST_BIT		0x80
#ifndef AMI306L
#define AMI_CTRL4_HS			0xa07e
#else
#define AMI_CTRL4_HS			0x0802
#endif
#define AMI_CTRL4_AB			0x0001
#define AMI_STA1_DRDY_BIT		0x40
#define AMI_STA1_DOR_BIT		0x20

#endif

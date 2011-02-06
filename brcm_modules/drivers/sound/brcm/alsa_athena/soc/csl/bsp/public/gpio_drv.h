/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
*  @file   gpio_drv.h
*  @brief  gpio driver head file
*
*****************************************************************************/
/**
*
* @defgroup CSLGPIODrvGroup GPIO
* @ingroup CSLGroup
* @brief This group defines the APIs for GPIO driver

Click here to navigate back to the Chip Support Library Overview page: \ref CSLOverview. \n
*****************************************************************************/
#ifndef GPIO_H__
#define GPIO_H__

#include <plat/osdal_os_service.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* GPIO PIN id 
*****************************************************************************/
#define	GPIO0	0
#define	GPIO1	1
#define	GPIO2	2
#define	GPIO3	3
#define	GPIO4	4
#define	GPIO5	5
#define	GPIO6	6
#define	GPIO7	7
#define	GPIO8	8
#define	GPIO9	9
#define	GPIO10	10
#define	GPIO11	11
#define	GPIO12	12
#define	GPIO13	13
#define	GPIO14	14
#define	GPIO15	15
#define	GPIO16	16
#define	GPIO17	17
#define	GPIO18	18
#define	GPIO19	19
#define	GPIO20	20
#define	GPIO21	21
#define	GPIO22	22
#define	GPIO23	23
#define	GPIO24	24
#define	GPIO25	25
#define	GPIO26	26
#define	GPIO27	27
#define	GPIO28	28
#define	GPIO29	29
#define	GPIO30	30
#define	GPIO31	31
#define	GPIO32	32
#define	GPIO33	33
#define	GPIO34	34
#define	GPIO35	35
#define	GPIO36	36
#define	GPIO37	37
#define	GPIO38	38
#define	GPIO39	39
#define	GPIO40	40
#define	GPIO41	41
#define	GPIO42	42
#define	GPIO43	43
#define	GPIO44	44
#define	GPIO45	45
#define	GPIO46	46
#define	GPIO47	47
#define	GPIO48	48
#define	GPIO49	49
#define	GPIO50	50
#define	GPIO51	51
#define	GPIO52	52
#define	GPIO53	53
#define	GPIO54	54
#define	GPIO55	55
#define	GPIO56	56
#define	GPIO57	57
#define	GPIO58	58
#define	GPIO59	59
#define	GPIO60	60
#define	GPIO61	61
#define	GPIO62	62
#define	GPIO63	63


#define GPIO_INVALID 0xFF

#define NR_GPIO	(GPIO63 + 1)

/**
*  Define:  GPIO_DIR_xxx/GPIO_INT_xxx
*
*  Following defines config values for GPIO pins.
******************************************************************************/
#define GPIO_DIR_INPUT          OSDAL_GPIO_DIR_INPUT
#define GPIO_DIR_OUTPUT         OSDAL_GPIO_DIR_OUTPUT


/**
*  Define:  GPIO_DB_xxx
*
*  Following defines debounce values for GPIO pins.
*****************************************************************************/
#define GPIO_DB_DISABLE         OSDAL_GPIO_DB_DISABLE
#define GPIO_DB_ENABLE          OSDAL_GPIO_DB_ENABLE

/**
*  Define:  GPIO_PULL_xxx
*
*  Following defines pull values for GPIO pins.
*****************************************************************************/
#define GPIO_PULL_DISABLE        0x00
#define GPIO_PULL_ENABLE         0x01
#define GPIO_PULL_DOWN        	 (0 << 1)
#define GPIO_PULL_UP         	 (1 << 1)

#define GPIO_IRQ_ENABLE          0x01
#define GPIO_IRQ_DISABLE         0x00


#define GPIODRV_Set_Mode(pin_id,mode)   OSDAL_GPIO_Set_Config(pin_id,OSDAL_GPIO_CFG_MODE,mode)
#define GPIODRV_Set_Bit(pin_id,high)    OSDAL_GPIO_Set(pin_id,high)

#ifdef __cplusplus
}
#endif

#endif // CHAL_GPIO_H__


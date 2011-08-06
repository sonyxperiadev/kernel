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
#if defined (_HERA_) || defined (_RHEA_)
#define	GPIO64	64
#define	GPIO65	65
#define	GPIO66	66
#define	GPIO67	67
#define	GPIO68	68
#define	GPIO69	69
#define	GPIO70	70
#define	GPIO71	71
#define	GPIO72	72
#define	GPIO73	73
#define	GPIO74	74
#define	GPIO75	75
#define	GPIO76	76
#define	GPIO77	77
#define	GPIO78	78
#define	GPIO79	79
#define	GPIO80	80
#define	GPIO81	81
#define	GPIO82	82
#define	GPIO83	83
#define	GPIO84	84
#define	GPIO85	85
#define	GPIO86	86
#define	GPIO87	87
#define	GPIO88	88
#define	GPIO89	89
#define	GPIO90	90
#define	GPIO91	91
#define	GPIO92	92
#define	GPIO93	93
#define	GPIO94	94
#define	GPIO95	95
#define	GPIO96	96
#define	GPIO97	97
#define	GPIO98	98
#define	GPIO99	99
#define	GPIO100	100
#define	GPIO101	101
#define	GPIO102	102
#define	GPIO103	103
#define	GPIO104	104
#define	GPIO105	105
#define	GPIO106	106
#define	GPIO107	107
#define	GPIO108	108
#define	GPIO109	109
#define	GPIO110	110
#define	GPIO111	111
#define	GPIO112	112
#define	GPIO113	113
#define	GPIO114	114
#define	GPIO115	115
#define	GPIO116	116
#define	GPIO117	117
#define	GPIO118	118
#define	GPIO119	119
#define	GPIO120	120
#define	GPIO121	121
#define	GPIO122	122
#define	GPIO123	123
#define	GPIO124	124
#endif

#define GPIO_INVALID 0xFF

#if defined (_HERA_) || defined(_RHEA_) || defined(_SAMOA_)
#define NR_GPIO	(GPIO124 + 1)
#else
#define NR_GPIO	(GPIO63 + 1)
#endif

/**
*  Define:  GPIO_DIR_xxx/GPIO_INT_xxx
*
*  Following defines config values for GPIO pins.
******************************************************************************/
#define GPIO_DIR_INPUT          0x00
#define GPIO_DIR_OUTPUT         0x01
#define GPIO_INT_RISING         (1 << 1)
#define GPIO_INT_FALLING        (1 << 2)
#define GPIO_INT_BOTH           (1 << 3)

/**
*  Define:  GPIO_DB_xxx
*
*  Following defines debounce values for GPIO pins.
*****************************************************************************/
#define GPIO_DB_DISABLE         0x00
#define GPIO_DB_ENABLE          0x01
#define GPIO_DB_1MS        	 	(0 << 1)
#define GPIO_DB_2MS        	 	(1 << 1)
#define GPIO_DB_4MS        	 	(2 << 1)
#define GPIO_DB_8MS        	 	(3 << 1)
#define GPIO_DB_16MS       	 	(4 << 1)
#define GPIO_DB_32MS       	 	(5 << 1)
#define GPIO_DB_64MS       	 	(6 << 1)
#define GPIO_DB_128MS      	 	(7 << 1)

/**
*  Define:  GPIO_PULL_xxx
*
*  Following defines pull values for GPIO pins.
*****************************************************************************/
#define GPIO_STATUS_LOW          0x0
#define GPIO_STATUS_HIGH         0x1
#define GPIO_PULL_DISABLE        0x00
#define GPIO_PULL_ENABLE         0x01
#define GPIO_PULL_DOWN        	 (0 << 1)
#define GPIO_PULL_UP         	 (1 << 1)

#define GPIO_IRQ_ENABLE          0x01
#define GPIO_IRQ_DISABLE         0x00

#if defined(CNEON_COMMON) || defined(CNEON_MODEM)
typedef void (*GPIO_CB)(UInt32);
#else /* CNEON_COMMON */
typedef void (*GPIO_CB)(void);
#endif /* CNEON_COMMON */

/** @addtogroup CSLGPIODrvGroup
	@{
*/

/**
*
*  @brief  gpio initialization
*  @return none
******************************************************************************/
void GPIODRV_Init(void);

/**
*
*  @brief  gpio de-initialization 
*  @return none 
******************************************************************************/
void GPIODRV_Deinit(void);

/**
*
*  @brief  gpio set bit 
* 
*  @param  pin_id (in) GPIO ID 
*  @param  value  (in) value to set 
*
*  @return none 
******************************************************************************/
void GPIODRV_Set_Bit(UInt32 pin_id, UInt32 value);

/**
*
*  @brief  gpio get bit 
* 
*  @param  pin_id     (in) GPIO ID
*
*  @return GPIO bit 
******************************************************************************/
UInt32 GPIODRV_Get_Bit(UInt32 pin_id);

/**
*
*  @brief  gpio set mode 
* 
*  @param  pin_id (in) GPIO ID 
*  @param  mode   (in) mode info 
*
*  @return none 
******************************************************************************/
void GPIODRV_Set_Mode(UInt32 pin_id, UInt32 mode);

/**
*
*  @brief  gpio get mode 
*
*  @param  pin_id (in) GPIO ID 
*
*  @return mode info 
******************************************************************************/
UInt32 GPIODRV_Get_Mode(UInt32 pin_id);

/**
*
*  @brief  gpio set pull 
* 
*  @param  pin_id (in) GPIO ID 
*  @param  value  (in) pull value 
*
*  @return none 
******************************************************************************/
void GPIODRV_Set_Pull(UInt32 pin_id, UInt32 value);

/**
*
*  @brief  gpio get pull 
* 
*  @param  pin_id     (in) GPIO ID
*
*  @return pull info 
******************************************************************************/
UInt32 GPIODRV_Get_Pull(UInt32 pin_id);

/**
*
*  @brief  gpio set debounce 
* 
*  @param  pin_id (in) GPIO ID
*  @param  value  (in) debounce value 
*
*  @return none 
******************************************************************************/
void GPIODRV_Set_Db(UInt32 pin_id, UInt32 value);

/**
*
*  @brief  gpio get debounce 
* 
*  @param  pin_id     (in) GPIO ID 
*
*  @return debounce value 
******************************************************************************/
UInt32 GPIODRV_Get_Db(UInt32 pin_id);

/**
*
*  @brief  gpio set irq mask 
* 
*  @param  pin_id (in) GPIO ID 
*  @param  value  (in) IRQ mask value 
*
*  @return none 
******************************************************************************/
void GPIODRV_Set_Irqmask(UInt32 pin_id, UInt32 value);

/**
*
*  @brief  gpio get irq mask 
* 
*  @param  pin_id     (in) GPIO ID 
*
*  @return IRQ mask  
******************************************************************************/
UInt32 GPIODRV_Get_Irqmask(UInt32 pin_id);

/**
*
*  @brief  gpio clear int 
* 
*  @param  pin_id     (in) GPIO ID 
*
*  @return none 
******************************************************************************/
void GPIODRV_Clear_Int(UInt32 pin_id);

/**
*
*  @brief  gpio irq 
* 
*  @param  pin_id     (in) GPIO ID
*
*  @return none 
******************************************************************************/
void GPIODRV_Enable_Int(UInt32 pin_id);

/**
*
*  @brief  gpio disable irq 
*
*  @param  pin_id     (in) GPIO ID 
*
*  @return none 
******************************************************************************/
void GPIODRV_Disable_Int(UInt32 pin_id);

/**
*
*  @brief  Register a GPIO interrupt callback function by
*          interrupt pin number.
*  
*  @param 	callback   function to be called when an interrupt
*         occurs on line 'id'.
*  @param  pin_id     (in) GPIO ID 
*
*  @return none 
******************************************************************************/
void GPIODRV_Register(GPIO_CB callback, UInt32 pin_id);

#if defined(CNEON_COMMON)
GPIO_CB GPIODRV_Get_CallBack(UInt32 input_pin);
#endif /* CNEON_COMMON */

/**
*
*  @brief  Deregister a GPIO interrupt callback function by
*          interrupt pin number.
*  
*  @param  pin_id     (in) GPIO ID 
*
*  @return none 
******************************************************************************/
void GPIODRV_Deregister(UInt32 pin_id);

/**
*
*  @brief  Return GPIO driver version information.
*
*  @return driver version 
******************************************************************************/

UInt32 GPIODRV_Get_Version(void);

/** @} */


#ifdef __cplusplus
}
#endif

#endif // CHAL_GPIO_H__


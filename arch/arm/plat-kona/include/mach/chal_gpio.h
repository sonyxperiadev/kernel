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
*  @file   isl_gpio.h
*  @brief  gpio cHAL interface
*  @note  
*
*****************************************************************************/
#ifndef ISL_GPIO_H__
#define ISL_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/csp/isl_types.h>

/**
 * @addtogroup cHAL_Interface 
 * @{
 */

/**
* ISL_GPIO PIN id 
*****************************************************************************/
#define	ISL_GPIO0	0
#define	ISL_GPIO1	1
#define	ISL_GPIO2	2
#define	ISL_GPIO3	3
#define	ISL_GPIO4	4
#define	ISL_GPIO5	5
#define	ISL_GPIO6	6
#define	ISL_GPIO7	7
#define	ISL_GPIO8	8
#define	ISL_GPIO9	9
#define	ISL_GPIO10	10
#define	ISL_GPIO11	11
#define	ISL_GPIO12	12
#define	ISL_GPIO13	13
#define	ISL_GPIO14	14
#define	ISL_GPIO15	15
#define	ISL_GPIO16	16
#define	ISL_GPIO17	17
#define	ISL_GPIO18	18
#define	ISL_GPIO19	19
#define	ISL_GPIO20	20
#define	ISL_GPIO21	21
#define	ISL_GPIO22	22
#define	ISL_GPIO23	23
#define	ISL_GPIO24	24
#define	ISL_GPIO25	25
#define	ISL_GPIO26	26
#define	ISL_GPIO27	27
#define	ISL_GPIO28	28
#define	ISL_GPIO29	29
#define	ISL_GPIO30	30
#define	ISL_GPIO31	31
#define	ISL_GPIO32	32
#define	ISL_GPIO33	33
#define	ISL_GPIO34	34
#define	ISL_GPIO35	35
#define	ISL_GPIO36	36
#define	ISL_GPIO37	37
#define	ISL_GPIO38	38
#define	ISL_GPIO39	39
#define	ISL_GPIO40	40
#define	ISL_GPIO41	41
#define	ISL_GPIO42	42
#define	ISL_GPIO43	43
#define	ISL_GPIO44	44
#define	ISL_GPIO45	45
#define	ISL_GPIO46	46
#define	ISL_GPIO47	47
#define	ISL_GPIO48	48
#define	ISL_GPIO49	49
#define	ISL_GPIO50	50
#define	ISL_GPIO51	51
#define	ISL_GPIO52	52
#define	ISL_GPIO53	53
#define	ISL_GPIO54	54
#define	ISL_GPIO55	55
#define	ISL_GPIO56	56
#define	ISL_GPIO57	57
#define	ISL_GPIO58	58
#define	ISL_GPIO59	59
#define	ISL_GPIO60	60
#define	ISL_GPIO61	61
#define	ISL_GPIO62	62
#define	ISL_GPIO63	63
#define	ISL_GPIO64	64
#define	ISL_GPIO65	65
#define	ISL_GPIO66	66
#define	ISL_GPIO67	67
#define	ISL_GPIO68	68
#define	ISL_GPIO69	69
#define	ISL_GPIO70	70
#define	ISL_GPIO71	71
#define	ISL_GPIO72	72
#define	ISL_GPIO73	73
#define	ISL_GPIO74	74
#define	ISL_GPIO75	75
#define	ISL_GPIO76	76
#define	ISL_GPIO77	77
#define	ISL_GPIO78	78
#define	ISL_GPIO79	79
#define	ISL_GPIO80	80
#define	ISL_GPIO81	81
#define	ISL_GPIO82	82
#define	ISL_GPIO83	83
#define	ISL_GPIO84	84
#define	ISL_GPIO85	85
#define	ISL_GPIO86	86
#define	ISL_GPIO87	87
#define	ISL_GPIO88	88
#define	ISL_GPIO89	89
#define	ISL_GPIO90	90
#define	ISL_GPIO91	91
#define	ISL_GPIO92	92
#define	ISL_GPIO93	93
#define	ISL_GPIO94	94
#define	ISL_GPIO95	95
#define	ISL_GPIO96	96
#define	ISL_GPIO97	97
#define	ISL_GPIO98	98
#define	ISL_GPIO99	99
#define	ISL_GPIO100	100
#define	ISL_GPIO101	101
#define	ISL_GPIO102	102
#define	ISL_GPIO103	103
#define	ISL_GPIO104	104
#define	ISL_GPIO105	105
#define	ISL_GPIO106	106
#define	ISL_GPIO107	107
#define	ISL_GPIO108	108
#define	ISL_GPIO109	109
#define	ISL_GPIO110	110
#define	ISL_GPIO111	111
#define	ISL_GPIO112	112
#define	ISL_GPIO113	113
#define	ISL_GPIO114	114
#define	ISL_GPIO115	115
#define	ISL_GPIO116	116
#define	ISL_GPIO117	117
#define	ISL_GPIO118	118
#define	ISL_GPIO119	119
#define	ISL_GPIO120	120
#define	ISL_GPIO121	121
#define	ISL_GPIO122	122
#define	ISL_GPIO123	123
#define	ISL_GPIO124	124
#define	ISL_GPIO125	125
#define	ISL_GPIO126	126
#define	ISL_GPIO127	127
#define	ISL_GPIO128	128
#define	ISL_GPIO129	129
#define	ISL_GPIO130	130
#define	ISL_GPIO131	131
#define	ISL_GPIO132	132
#define	ISL_GPIO133	133
#define	ISL_GPIO134	134
#define	ISL_GPIO135	135
#define	ISL_GPIO136	136
#define	ISL_GPIO137	137
#define	ISL_GPIO138	138
#define	ISL_GPIO139	139
#define	ISL_GPIO140	140
#define	ISL_GPIO141	141
#define	ISL_GPIO142	142
#define	ISL_GPIO143	143
#define	ISL_GPIO144	144
#define	ISL_GPIO145	145
#define	ISL_GPIO146	146
#define	ISL_GPIO147	147
#define	ISL_GPIO148	148
#define	ISL_GPIO149	149
#define	ISL_GPIO150	150
#define	ISL_GPIO151	151
#define	ISL_GPIO152	152
#define	ISL_GPIO153	153
#define	ISL_GPIO154	154
#define	ISL_GPIO155	155
#define	ISL_GPIO156	156
#define	ISL_GPIO157	157
#define	ISL_GPIO158	158
#define	ISL_GPIO159	159
#define	ISL_GPIO160	160
 
#define NR_ISL_GPIO	(ISL_GPIO160 + 1)

/**
*  Define:  ISL_GPIO Control Register
*  
******************************************************************************/
#define ISL_GPIO_GPCTR_IOTR        0x0001
#define ISL_GPIO_GPCTR_IOTR_SHIFT  0
#define ISL_GPIO_GPCTR_PED         0x0002
#define ISL_GPIO_GPCTR_PED_SHIFT   1
#define ISL_GPIO_GPCTR_PUD         0x0004
#define ISL_GPIO_GPCTR_PUD_SHIFT   2
#define ISL_GPIO_GPCTR_ITR         0x0018
#define ISL_GPIO_GPCTR_ITR_SHIFT   3
#define ISL_GPIO_GPCTR_DBR         0x01E0
#define ISL_GPIO_GPCTR_DBR_SHIFT   5

/**
*  Define:  ISL_GPIO_VALUE_xxx
*
*  Following defines values for ISL_GPIO pins.
*****************************************************************************/

#define   ISL_GPIO_VALUE_ZERO		0
#define   ISL_GPIO_VALUE_ONE		1

/**
*  Define:  ISL_GPIO_DIR_xxx/ISL_GPIO_INT_xxx
*
*  Following defines config values for ISL_GPIO pins.
******************************************************************************/
#define ISL_GPIO_DIR_INPUT          0x01
#define ISL_GPIO_DIR_OUTPUT         0x00
#define ISL_GPIO_INT_DISABLE		  0
#define ISL_GPIO_INT_RISING         1
#define ISL_GPIO_INT_FALLING        2
#define ISL_GPIO_INT_BOTH           3

/**
*  Define:  ISL_GPIO_DB_xxx
*
*  Following defines debounce values for ISL_GPIO pins.
*****************************************************************************/
#define ISL_GPIO_DB_DISABLE         0x00
#define ISL_GPIO_DB_ENABLE          0x08
#define ISL_GPIO_DB_1MS        	 	0
#define ISL_GPIO_DB_2MS        	 	1
#define ISL_GPIO_DB_4MS        	 	2
#define ISL_GPIO_DB_8MS        	 	3
#define ISL_GPIO_DB_16MS       	 	4
#define ISL_GPIO_DB_32MS       	 	5
#define ISL_GPIO_DB_64MS       	 	6
#define ISL_GPIO_DB_128MS      	 	7
#define ISL_GPIO_DB_NONE    			8
/**
*  Define:  ISL_GPIO_PULL_xxx
*
*  Following defines pull values for ISL_GPIO pins.
*****************************************************************************/
#define ISL_GPIO_PULL_DISABLE        0x00
#define ISL_GPIO_PULL_ENABLE         0x01
#define ISL_GPIO_PULL_DOWN        	 (0 << 1)
#define ISL_GPIO_PULL_UP         	 (1 << 1)

#define ISL_GPIO_IRQ_ENABLE          0x01
#define ISL_GPIO_IRQ_DISABLE         0x00

/**
*
*  @brief  gpio initilization
*
*  @param  base  (in) Base address of ISL_GPIO block
*
*  @return handle of this ISL_GPIO instance
******************************************************************************/
ISL_HANDLE isl_gpio_init(cUInt32 base);

/**
*
*  @brief  gpio de-initialization 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_deinit(ISL_HANDLE hContext);

/**
*
*  @brief  gpio set bit 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio_id  (in) ISL_GPIO ID 
*  @param  value  	(in) value to set 
*
*  @return none 
*
*  @note   For any pins that toggles between input & output, make sure the pin
*          is in output mode before calling this function.
******************************************************************************/
cVoid isl_gpio_set_bit(ISL_HANDLE hContext, cUInt32 gpio_id, cUInt32 value);

/**
*
*  @brief  gpio get bit 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init()
*  @param  gpio_id  (in) ISL_GPIO ID
*
*  @return ISL_GPIO pin status 
******************************************************************************/
cUInt32 isl_gpio_get_bit(ISL_HANDLE hContext, cUInt32 gpio_id);

/**
*
*  @brief  gpio get mode 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID 
*
*  @return mode info 
******************************************************************************/
cUInt32 isl_gpio_get_mode(ISL_HANDLE hContext, cUInt32 pin_id);

/**
*
*  @brief  gpio set pull 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID 
*  @param  value  (in) pull value 
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_set_pull(ISL_HANDLE hContext, cUInt32 pin_id, cUInt32 value);

/**
*
*  @brief  gpio get pull 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID
*
*  @return pull info 
******************************************************************************/
cUInt32 isl_gpio_get_pull(ISL_HANDLE hContext, cUInt32 pin_id);

/**
*
*  @brief  gpio set debounce 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID
*  @param  value  (in) debounce value 
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_set_db(ISL_HANDLE hContext, cUInt32 pin_id, cUInt32 value);

/**
*
*  @brief  gpio get debounce 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID 
*
*  @return debounce value 
******************************************************************************/
cUInt32 isl_gpio_get_db(ISL_HANDLE hContext, cUInt32 pin_id);

/**
*
*  @brief  gpio set interrupt mask 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio_id  (in) ISL_GPIO ID 
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_set_irqmask(ISL_HANDLE hContext, cUInt32 gpio_id, cUInt32 value);

/**
*
*  @brief  gpio get interrupt mask 
*
*  @param  handle  	(in) Handle returned in isl_gpio_init() 
*  @param  pin_id   (in) ISL_GPIO ID 
*
*  @return interrupt mask  
******************************************************************************/
cUInt32 isl_gpio_get_irqmask(ISL_HANDLE hContext, cUInt32 gpio_id);

/**
*
*  @brief  gpio clear interrupt mask 
*
*  @param  handle  	(in) Handle returned in isl_gpio_init() 
*  @param  pin_id   (in) ISL_GPIO ID 
*
*  @return none  
******************************************************************************/
cVoid isl_gpio_clear_irqmask(ISL_HANDLE hContext, cUInt32 gpio_id);

/**
*
*  @brief  gpio get intrrupt status 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio_id   (in) ISL_GPIO ID 
*
*  @return status 
******************************************************************************/
cUInt32 isl_gpio_get_irqstatus(ISL_HANDLE hContext, cUInt32 gpio_id);

/**
*
*  @brief  gpio clear int status 
*
*  @param  handle 	(in) Handle returned in isl_gpio_init() 
*  @param  gpio_id  (in) ISL_GPIO ID 
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_clear_int(ISL_HANDLE hContext, cUInt32 gpio_id);

/**
*
*  @brief  gpio irq 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_enable_int(ISL_HANDLE hContext, cUInt32 pin_id);

/**
*
*  @brief  gpio disable irq 
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID 
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_disable_int(ISL_HANDLE hContext, cUInt32 pin_id);

/**
*
*  @brief  isl_gpio_set_mode
*
*  @param  handle (in) Handle returned in isl_gpio_init() 
*  @param  pin_id     (in) ISL_GPIO ID
*
*  @return none 
******************************************************************************/
cVoid isl_gpio_set_mode(ISL_HANDLE hContext, cUInt32 gpio_id, cUInt32 mode);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ISL_GPIO_H__ */

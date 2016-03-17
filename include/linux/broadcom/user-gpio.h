/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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


/****************************************************************************/
/**
*   @file   user-gpio.h
*
*   @brief  Reflects the gpiolib API into user space through a driver.
*/
/****************************************************************************/

#if !defined( USER_GPIO_H )
#define USER_GPIO_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/ioctl.h>

/* ---- Constants and Types ---------------------------------------------- */

#define GPIO_MAGIC  'G'

#define GPIO_CMD_REQUEST                0x80
#define GPIO_CMD_FREE                   0x81
#define GPIO_CMD_DIRECTION_INPUT        0x82
#define GPIO_CMD_DIRECTION_OUTPUT       0x83
#define GPIO_CMD_DIRECTION_IS_OUTPUT    0x84
#define GPIO_CMD_GET_VALUE              0x85
#define GPIO_CMD_SET_VALUE              0x86

typedef struct
{
    unsigned    gpio;
    char        label[ 32 ];

} GPIO_Request_t;

typedef struct
{
    unsigned    gpio;
    int         value;

} GPIO_Value_t;

#define GPIO_IOCTL_REQUEST              _IOW(   GPIO_MAGIC, GPIO_CMD_REQUEST, GPIO_Request_t )              /* arg is GPIO_Request_t * */
#define GPIO_IOCTL_FREE                 _IO(    GPIO_MAGIC, GPIO_CMD_FREE )                                 /* arg is int */
#define GPIO_IOCTL_DIRECTION_INPUT      _IO(    GPIO_MAGIC, GPIO_CMD_DIRECTION_INPUT )                      /* arg is int */
#define GPIO_IOCTL_DIRECTION_OUTPUT     _IOW(   GPIO_MAGIC, GPIO_CMD_DIRECTION_OUTPUT, GPIO_Value_t )       /* arg is GPIO_Value_t * */
#define GPIO_IOCTL_DIRECTION_IS_OUTPUT  _IOWR(  GPIO_MAGIC, GPIO_CMD_DIRECTION_IS_OUTPUT, GPIO_Value_t )    /* arg is GPIO_Value_t * */
#define GPIO_IOCTL_GET_VALUE            _IOWR(  GPIO_MAGIC, GPIO_CMD_GET_VALUE, GPIO_Value_t )              /* arg is GPIO_Value_t * */
#define GPIO_IOCTL_SET_VALUE            _IOW(   GPIO_MAGIC, GPIO_CMD_SET_VALUE, GPIO_Value_t )              /* arg is GPIO_Value_t * */

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

#if !defined( __KERNEL__ )

int gpio_init( void );
void gpio_term( void );

/* The following APIs are available in user-space - using the same API as the kernel side functions */

int  gpio_request( unsigned gpio, const char *label );
void gpio_free( unsigned gpio );
int  gpio_direction_input( unsigned gpio );
int  gpio_direction_output( unsigned gpio, int initialValue );
int  gpio_direction_is_output( unsigned gpio );
int  gpio_get_value( unsigned gpio );
void gpio_set_value( unsigned gpio, int value );

#endif  /* !defined( __KERNEL__ ) */

#endif  /* USER_GPIO_H */


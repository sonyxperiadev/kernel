/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef _BQ27541_I2C_H_
#define _BQ27541_I2C_H_

#define I2C_BQ27541_DRIVER_NAME           "bq27541_i2c_drvr"
#define I2C_BQ27541_DRIVER_SLAVE_NUMBER   0x55

#define BQ27541_CMD_TEMP         0x06	/* Temperature */
#define BQ27541_CMD_VOLT         0x08	/* Voltage */
#define BQ27541_CMD_FLAGS        0x0A	/* Flags */
#define BQ27541_CMD_FAC          0x0E	/* Full Available Capacity */
#define BQ27541_CMD_RM           0x10	/* Remaining Capacity */
#define BQ27541_CMD_FCC          0x12	/* Full Charge Capacity */
#define BQ27541_CMD_AI           0x14	/* Current */
#define BQ27541_CMD_TTE          0x16	/* Time To Empty */
#define BQ27541_CMD_TTF          0x18	/* Time To Full */
#define BQ27541_CMD_AE           0x22	/* Available Energy */
#define BQ27541_CMD_AP           0x24	/* Average Power */
#define BQ27541_CMD_CC           0x2A	/* Cycle Count */
#define BQ27541_CMD_SOC          0x2C	/* State of Charge */

/* Function for sending i2c command to BQ27541 */
int i2c_bq27541_cmd(int cmd, int *value);

#endif /* _BQ27541_I2C_H_ */

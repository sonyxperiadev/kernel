/*
 *  Copyright (C) 2010, Imagis Technology Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef __IST30XX_UPDATE_H__
#define __IST30XX_UPDATE_H__


#define IST30XX_INTERNAL_BIN    (1)

#define IST30XX_EEPROM_SIZE     (0x8000)
#define EEPROM_PAGE_SIZE        (64)
#define EEPROM_BASE_ADDR        (0)

#define IST30XX_FW_START_ADDR   (0x0600)
#define IST30XX_FW_END_ADDR     (0x7A00)	/* F/W ver 3.0*/
#define IST30XX_CONFIG_SIZE     (160 * IST30XX_DATA_LEN)	/* F/W v3.0*/
#define IST30XX_SENSOR1_SIZE    (64 * IST30XX_DATA_LEN)	/* F/W v3.0*/
#define IST30XX_SENSOR2_SIZE    (32 * IST30XX_DATA_LEN)	/* F/W v3.0*/

#define IST30XX_FLAG_SIZE       (0x40)
#define IST30XX_FW_SIZE         (IST30XX_FW_END_ADDR - IST30XX_FW_START_ADDR)
#define IST30XX_SENSOR_SIZE     (IST30XX_SENSOR1_SIZE + IST30XX_SENSOR2_SIZE)

#define IST30XX_FW_NAME         "ist30xx.fw"
#define IST30XX_PARAM_NAME      "ist30xx.param"


#define PARSE_FLAG_FW           (1)
#define PARSE_FLAG_PARAM        (2)

#define MASK_FW_VER             (0xFFFF0000)
#define IST30XX_FW_VER1         (0x00010000)
#define IST30XX_FW_VER2         (0x00020000)
#define IST30XX_FW_VER3         (0x00030000)

#define IST30XX_FW_UPDATE_RETRY (5)

#define WAIT_CALIB_CNT          (50)

#define CALIB_TO_GAP(n)         ((n >> 16) & 0xFFF)
#define CALIB_TO_STATUS(n)      ((n >> 12) & 0xF)

#define MASK_UPDATE_BIN         (1)
#define MASK_UPDATE_FW          (1)
#define MASK_UPDATE_ISP         (2)

#define TAGS_PARSE_OK           (0)

/* I2C Transaction size */
#define I2C_MAX_WRITE_SIZE      (64)            /* bytes */
#define I2C_MAX_READ_SIZE       (8)             /* bytes */



int ist30xx_read_buf(struct i2c_client *client, u32 cmd, u32 *buf, u16 len);
int ist30xx_write_buf(struct i2c_client *client, u32 cmd, u32 *buf, u16 len);

void ist30xx_get_update_info(struct ist30xx_data *data,
			const u8 *buf, const u32 size);

int ist30xx_fw_update(struct i2c_client *client,
			const u8 *buf, int size, bool mode);
int ist30xx_param_update(struct i2c_client *client, const u8 *buf, int size);
int ist30xx_auto_fw_update(struct ist30xx_data *data);
int ist30xx_auto_param_update(struct ist30xx_data *data);

int ist30xx_calib_wait(void);
int ist30xx_get_ic_fw_ver(void);
int ist30xx_init_update_sysfs(void);
int ist30xx_force_fw_update(struct ist30xx_data *data);
int ist30xx_force_param_update(struct ist30xx_data *data);
int ist30xx_get_ic_fw_ver(void);
#endif  /* __IST30XX_UPDATE_H__*/

/*
 *  Copyright (C) 2010,Imagis Technology Co. Ltd. All Rights Reserved.
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

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <asm/unaligned.h>
#include <linux/module.h>

#include "ist30xx.h"
#include "ist30xx_update.h"

/*#include "ist30xx_fw.h"*/
#include "ist30xx_fw3_2_3.h"

u32 ist30xx_fw_ver;

extern struct ist30xx_data *ts_data;

extern void ist30xx_disable_irq(struct ist30xx_data *data);
extern void ist30xx_enable_irq(struct ist30xx_data *data);


int ist30xx_i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
			 int msg_num, u8 *cmd_buf)
{
	int ret = 0;
	int idx = msg_num - 1;
	int size = msgs[idx].len;
	u8 *msg_buf = NULL;
	u8 *pbuf = NULL;
	int trans_size, max_size = 0;

	if (msg_num == WRITE_CMD_MSG_LEN)
		max_size = I2C_MAX_WRITE_SIZE;
	else if (msg_num == READ_CMD_MSG_LEN)
		max_size = I2C_MAX_READ_SIZE;

	if (max_size == 0) {
		pr_err("[ TSP ] %s() : transaction size(%d)\n",
		       __func__, max_size);
		return -EINVAL;
	}

	if (msg_num == WRITE_CMD_MSG_LEN) {
		msg_buf = kmalloc(max_size + IST30XX_ADDR_LEN, GFP_KERNEL);
		if (!msg_buf)
			return -ENOMEM;
		memcpy(msg_buf, cmd_buf, IST30XX_ADDR_LEN);
		pbuf = msgs[idx].buf;
	}

	while (size > 0) {
		trans_size = (size >= max_size ? max_size : size);

		msgs[idx].len = trans_size;
		if (msg_num == WRITE_CMD_MSG_LEN) {
			memcpy(&msg_buf[IST30XX_ADDR_LEN], pbuf, trans_size);
			msgs[idx].buf = msg_buf;
			msgs[idx].len += IST30XX_ADDR_LEN;
		}
		ret = i2c_transfer(adap, msgs, msg_num);
		if (ret != msg_num) {
			pr_err("[ TSP ] %s() : i2c_transfer failed(%d), num=%d\n",
			       __func__, ret, msg_num);
			break;
		}

		if (msg_num == WRITE_CMD_MSG_LEN)
			pbuf += trans_size;
		else
			msgs[idx].buf += trans_size;

		size -= trans_size;
	}

	if (msg_num == WRITE_CMD_MSG_LEN)
		kfree(msg_buf);

	return ret;
}

int ist30xx_read_buf(struct i2c_client *client, u32 cmd, u32 *buf, u16 len)
{
	int ret, i;
	u32 le_reg = cpu_to_be32(cmd);

	struct i2c_msg msg[READ_CMD_MSG_LEN] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = IST30XX_ADDR_LEN,
			.buf = (u8 *)&le_reg,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = len * IST30XX_DATA_LEN,
			.buf = (u8 *)buf,
		},
	};

	ret = ist30xx_i2c_transfer(client->adapter,
		msg, READ_CMD_MSG_LEN, NULL);
	if (ret != READ_CMD_MSG_LEN)
		return -EIO;

	for (i = 0; i < len; i++)
		buf[i] = cpu_to_be32(buf[i]);

	return 0;
}

int ist30xx_write_buf(struct i2c_client *client, u32 cmd, u32 *buf, u16 len)
{
	int i;
	int ret;
	struct i2c_msg msg;
	u8 cmd_buf[IST30XX_ADDR_LEN];
	u8 msg_buf[IST30XX_DATA_LEN * (len + 1)];

	put_unaligned_be32(cmd, cmd_buf);

	if (len > 0) {
		for (i = 0; i < len; i++)
			put_unaligned_be32(buf[i],
			msg_buf + (i * IST30XX_DATA_LEN));
	} else {
		/* then add dummy data(4byte) */
		put_unaligned_be32(0, msg_buf);
		len = 1;
	}

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = IST30XX_DATA_LEN * len;
	msg.buf = msg_buf;

	ret = ist30xx_i2c_transfer(client->adapter,
		&msg, WRITE_CMD_MSG_LEN, cmd_buf);
	if (ret != WRITE_CMD_MSG_LEN)
		return -EIO;

	return 0;
}


static void prepare_msg_buffer(u8 *buf, u8 cmd, u16 addr)
{
	memset(buf, 0, sizeof(buf));

	buf[0] = cmd;
	buf[1] = (u8)((addr >> 8) & 0xFF);
	buf[2] = (u8)(addr & 0xFF);
}


/* ist30xx isp function */
#define CMD_ENTER_ISP_MODE          (0xE1)
#define CMD_EXIT_ISP_MODE           (0xE0)

#define CMD_ISP_BEGIN_READ          (0x11)
#define CMD_ISP_END_READ            (0x10)
#define CMD_ISP_ERASE_ALL_PAGE      (0x8F)
#define CMD_ISP_PROGRAM_PAGE        (0x91)

int ist30xx_isp_cmd(struct i2c_client *client, const u8 cmd)
{
	int ret;
	struct i2c_msg msg;

	msg.addr = IST30XX_FW_DEV_ID;
	msg.flags = I2C_M_IGNORE_NAK;
	msg.len = 1;
	msg.buf = (u8 *)&cmd;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1)
		return -EIO;

	/* need 2.5msec after this cmd */
	msleep(3);

	return 0;
}


int ist30xx_isp_write(struct i2c_client *client, const u8 *buf)
{
	int ret, i;
	struct i2c_msg msg;
	u32 page_cnt;
	u16 addr;
	u8 msg_buf[EEPROM_PAGE_SIZE + 3];

	addr = EEPROM_BASE_ADDR;
	page_cnt = IST30XX_EEPROM_SIZE / EEPROM_PAGE_SIZE;

	for (i = 0; i < page_cnt; i++) {
		prepare_msg_buffer(msg_buf, CMD_ISP_PROGRAM_PAGE, addr);

		memcpy(msg_buf + 3, buf, EEPROM_PAGE_SIZE);

		msg.addr = IST30XX_FW_DEV_ID;
		msg.flags = I2C_M_IGNORE_NAK;
		msg.len = EEPROM_PAGE_SIZE + 3;
		msg.buf = msg_buf;

		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret != 1)
			return -EIO;

		addr += EEPROM_PAGE_SIZE;
		buf += EEPROM_PAGE_SIZE;

		msleep(2);
	}

	return 0;
}


int ist30xx_isp_read(struct i2c_client *client, u32 len, u8 *buf)
{
	int ret, i;
	u16 addr = EEPROM_BASE_ADDR;
	u8 msg_buf[3]; /* Command + Dummy 16bit addr */
	struct i2c_msg msg;
	u32 fw_len, page_cnt;

	prepare_msg_buffer(msg_buf, CMD_ISP_BEGIN_READ, addr);
	msg.addr = IST30XX_FW_DEV_ID;
	msg.flags = I2C_M_IGNORE_NAK;
	msg.len = 3;
	msg.buf = msg_buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1)
		return -EIO;

	if (len % EEPROM_PAGE_SIZE)
		fw_len = (len / EEPROM_PAGE_SIZE + 1) * EEPROM_PAGE_SIZE;
	else
		fw_len = len;

	page_cnt = fw_len / EEPROM_PAGE_SIZE;

	for (i = 0; i < page_cnt; i++) {
		msg.addr = IST30XX_FW_DEV_ID;
		msg.flags = I2C_M_RD | I2C_M_IGNORE_NAK;
		msg.len = EEPROM_PAGE_SIZE;
		msg.buf = buf;

		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret != 1)
			return -EIO;

		addr += EEPROM_PAGE_SIZE;
		buf += EEPROM_PAGE_SIZE;
	}

	prepare_msg_buffer(msg_buf, CMD_ISP_END_READ, addr);
	msg.addr = IST30XX_FW_DEV_ID;
	msg.flags = I2C_M_IGNORE_NAK;
	msg.len = 3;
	msg.buf = msg_buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1)
		return -EIO;

	return 0;
}


int ist30xx_isp_erase_all(struct i2c_client *client)
{
	int ret;
	struct i2c_msg msg;

	/* EEPROM pase size + Command + Dummy 16bit addr */
	u8 msg_buf[EEPROM_PAGE_SIZE + 3];

	prepare_msg_buffer(msg_buf, CMD_ISP_ERASE_ALL_PAGE, EEPROM_BASE_ADDR);

	msg.addr = IST30XX_FW_DEV_ID;
	msg.flags = I2C_M_IGNORE_NAK;
	msg.len = EEPROM_PAGE_SIZE + 3;
	msg.buf = (u8 *)msg_buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1)
		return -EIO;

	/* need 2.5msec after erasing eeprom */
	msleep(3);

	return 0;
}


int ist30xx_isp_update(struct i2c_client *client, const u8 *buf)
{
	int ret = 0;

	ret = ist30xx_isp_cmd(client, CMD_ENTER_ISP_MODE);
	if (ret)
		return -EIO;

	ret = ist30xx_isp_erase_all(client);
	if (ret)
		return -EIO;

	ret = ist30xx_isp_write(client, buf);
	if (ret < 0)
		return -EIO;

	ret = ist30xx_isp_cmd(client, CMD_EXIT_ISP_MODE);
	if (ret)
		return -EIO;

	return 0;
}


int ist30xx_fw_write(struct i2c_client *client, const u8 *buf)
{
	int ret;
	int len;
	u32 *buf32 = (u32 *)(buf + ts_data->fw.index);
	u32 size = ts_data->fw.size;

	if (size < 0 || size > IST30XX_EEPROM_SIZE)
		return -ENOEXEC;

	while (size > 0) {
		len = (size >= EEPROM_PAGE_SIZE ? EEPROM_PAGE_SIZE : size);

		ret = ist30xx_write_buf(client,
			CMD_ENTER_FW_UPDATE, buf32, (len >> 2));
		if (ret)
			return ret;

		buf32 += (len >> 2);
		size -= len;

		msleep(5);
	}
	return 0;
}


int ist30xx_extra_write(struct i2c_client *client, const u8 *buf, u32 size)
{
	int i;
	int ret;
	u32 len;
	struct i2c_msg msg;
	u8 cmd_buf[IST30XX_ADDR_LEN];
	u8 *msg_buf;
	u32 *buf32 = (u32 *)buf;

	msg_buf = kmalloc(size, GFP_KERNEL);
	if (msg_buf == NULL)
		return -ENOMEM;

	len = (size >> 2); /*word unit : 4bytes;*/
	put_unaligned_be32(len, cmd_buf);

	for (i = 0; i < len; i++)
		put_unaligned_be32(*buf32++, &msg_buf[i * IST30XX_DATA_LEN]);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = size;
	msg.buf = &msg_buf[0];

	ret = ist30xx_i2c_transfer(client->adapter,
		&msg, WRITE_CMD_MSG_LEN, cmd_buf);
	if (ret != WRITE_CMD_MSG_LEN) {
		pr_err("[ TSP ] %s i2c transfer failed (ret=%d)\n",
		__func__, ret);
		kfree(msg_buf);
		return -EIO;
	}

	kfree(msg_buf);

	return 0;
}


u32 ist30xx_parse_fw_ver(int flag, const u8 *buf)
{
	u32 ver;
	u32 *buf32 = (u32 *)buf;

	if (flag == PARSE_FLAG_FW)
		ver = (u32)buf32[(ts_data->tags.flag_addr + 4) >> 2];
	else if (flag == PARSE_FLAG_PARAM)
		ver = (u32)buf32[0];
	else
		ver = 0;

	return ver;
}


u32 ist30xx_parse_param_ver(int flag, u8 *buf)
{
	u32 ver;
	u32 *buf32 = (u32 *)buf;

	if (flag == PARSE_FLAG_FW)
		ver = (u32)buf32[ts_data->tags.cfg_addr >> 2];
	else if (flag == PARSE_FLAG_PARAM)
		ver = (u32)buf32[4 >> 2];
	else
		ver = 0;

	return ver;
}


int calib_ms_delay = WAIT_CALIB_CNT;
int ist30xx_calib_wait(void)
{
	int cnt = calib_ms_delay;

	ts_data->status.calib = 0;
	while (cnt-- > 0) {
		msleep(100);

		if (ts_data->status.calib) {
			DMSG(
				"[ TSP ] Calibration Status : %d, Max raw gap : %d - (raw: %08x)\n",
			    CALIB_TO_STATUS(ts_data->status.calib),
			    CALIB_TO_GAP(ts_data->status.calib),
				ts_data->status.calib);

			return 0;
		}
	}
	pr_err("[ TSP ] Calibration time out\n");

	return -EPERM;
}


u32 ist30xx_make_checksum(const u8 *buf)
{
	int size = 0;
	u32 chksum = 0;
	u32 *buf32 = (u32 *)(buf + ts_data->fw.index);

	size = ts_data->fw.size - ts_data->tags.flag_size;
	if (size < 0 || size > IST30XX_EEPROM_SIZE)
		return 0;

	size >>= 2;
	for (; size > 0; size--)
		chksum += *buf32++;

	return chksum;
}


struct ist30xx_tags *ts_tags;
int ist30xx_parse_tags(struct ist30xx_data *data,
		const u8 *buf, const u32 size)
{
	int ret = -1;

	ts_tags = (struct ist30xx_tags *)
		(&buf[size - sizeof(struct ist30xx_tags)]);

	if (!strncmp(ts_tags->magic1, IST30XX_TAG_MAGIC,
		sizeof(ts_tags->magic1))
	    && !strncmp(ts_tags->magic2, IST30XX_TAG_MAGIC,
		sizeof(ts_tags->magic2))) {
		data->fw.index = ts_tags->fw_addr;
		data->fw.size = ts_tags->flag_addr - ts_tags->fw_addr +
				ts_tags->flag_size;
		data->fw.chksum = ts_tags->chksum;
		data->tags = *ts_tags;

		ret = 0;
	}

	return ret;
}

void ist30xx_get_update_info(struct ist30xx_data *data,
	const u8 *buf, const u32 size)
{
	int ret;

	ret = ist30xx_parse_tags(data, buf, size);
	if (ret != TAGS_PARSE_OK) {
		pr_err("[ TSP ] Cannot find tags of F/W, make a tags by 'tagts.exe'\n");
		if (size >= IST30XX_EEPROM_SIZE) {
			/*Firmware update*/
			data->fw.index = IST30XX_FW_START_ADDR;
			data->fw.size = IST30XX_FW_SIZE;
			data->tags.flag_addr =
				IST30XX_FW_END_ADDR - IST30XX_FLAG_SIZE;
			data->tags.flag_size = IST30XX_FLAG_SIZE;
			data->fw.chksum = ist30xx_make_checksum(buf);
		} else {                    /* Parameters update*/
			data->tags.cfg_size = IST30XX_CONFIG_SIZE;
			data->tags.sensor1_size = IST30XX_SENSOR1_SIZE;
			data->tags.sensor2_size = IST30XX_SENSOR2_SIZE;
		}
	}
}


#define update_next_step(ret)   { if (ret) goto end; }
int ist30xx_fw_update(struct i2c_client *client,
	const u8 *buf, int size, bool mode)
{
	int ret = 0;
	u32 val = 0;

	printk(KERN_INFO "[ TSP ] *** F/W update ***\n");
	printk(KERN_INFO "[ TSP ] F/W ver: %08x, addr: 0x%04x ~ 0x%04x\n",
	     ist30xx_fw_ver, ts_data->fw.index,
	     ts_data->fw.index + ts_data->fw.size);

	ts_data->status.fw_update = 1;

	ist30xx_disable_irq(ts_data);

	ist30xx_reset();

	if (mode) {     /* ISP Mode */
			printk(KERN_INFO "[ TSP ] ist30xx_fw_update ISP mode\n");
		ret = ist30xx_isp_update(client, buf);
		update_next_step(ret);
	} else {        /* I2C Mode */
		DMSG("[ TSP ] by ISP Mode\n");
		ret = ist30xx_cmd_update(client, CMD_ENTER_FW_UPDATE);
		update_next_step(ret);

		ret = ist30xx_fw_write(client, buf);
		update_next_step(ret);
	}
	msleep(50);

	buf += IST30XX_EEPROM_SIZE;
	size -= IST30XX_EEPROM_SIZE;

	ret = ist30xx_cmd_run_device(client);
	update_next_step(ret);

	ret = ist30xx_read_cmd(client, CMD_GET_CHECKSUM, &val);
	if ((ret) || (val != ts_data->fw.chksum))
		goto end;

	ret = ist30xx_cmd_calibrate(client);
	update_next_step(ret);

	if (ist30xx_fw_ver < IST30XX_FW_VER3) {
		size -= sizeof(struct ist30xx_tags);
		ret = ist30xx_extra_write(client, buf, size);
		update_next_step(ret);
	}

end:
	if (ret) {
		pr_err("[ TSP ] F/W update Fail!, ret=%d", ret);
	} else if (val != ts_data->fw.chksum) {
		pr_err("[ TSP ] Error CheckSum: 0x%08x, 0x%08x\n",
		       ts_data->fw.chksum, val);
		ret = -ENOEXEC;
	}

	ist30xx_enable_irq(ts_data);

	ret = ist30xx_calib_wait();

	ts_data->status.fw_update = 2;

	return ret;
}


int ist30xx_param_update(struct i2c_client *client, const u8 *buf, int size)
{
	int ret;
	u32 val, ver;
	u32 len;
	u32 *param = (u32 *)buf;

	ts_data->status.fw_update = 1;

	DMSG("[ TSP ] *** Parameters update ***\n");

	ist30xx_disable_irq(ts_data);

	ret = ist30xx_cmd_run_device(client);
	update_next_step(ret);

	/* Compare F/W version */
	ret = ist30xx_read_cmd(client, CMD_GET_FW_VER, &ver);
	update_next_step(ret);

	val = ist30xx_parse_fw_ver(PARSE_FLAG_PARAM, buf);
	if ((val & MASK_FW_VER) != (ver & MASK_FW_VER)) {
		pr_err("[ TSP ] Fail, F/W ver check. 0x%04x -> 0x%04x\n",
		       ver, val);
		goto end;
	}
	ist30xx_fw_ver = val;

	ret = ist30xx_cmd_update(client, CMD_ENTER_UPDATE);
	update_next_step(ret);

	/* update config */
	len = (ts_data->tags.cfg_size >> 2);
	ret = ist30xx_write_buf(client, CMD_UPDATE_CONFIG, param, len);
	update_next_step(ret);
	msleep(10);

	param += len;
	size -= (len << 2);

	/* update sensor */
	len = ((ts_data->tags.sensor1_size + ts_data->tags.sensor2_size) >> 2);
	ret = ist30xx_write_buf(client, CMD_UPDATE_SENSOR, param, len);
	update_next_step(ret);
	msleep(10);

	param += len;
	size -= (len << 2);

	ret = ist30xx_cmd_update(client, CMD_EXIT_UPDATE);
	update_next_step(ret);
	msleep(120);

	ret = ist30xx_cmd_run_device(client);
	update_next_step(ret);

	/* update auto calibration code */
	ret = ist30xx_cmd_calibrate(client);
	update_next_step(ret);

	if (ist30xx_fw_ver < IST30XX_FW_VER3) {
		buf = (u8 *)param;
		size -= sizeof(struct ist30xx_tags);
		ret = ist30xx_extra_write(client, buf, size);
	}

end:
	if (size < 0)
		pr_err("[ TSP ] Param update_fail, size=%d\n", size);
	else if (ret)
		pr_err("[ TSP ] Param update_fail, ret=%d\n", ret);

	ist30xx_enable_irq(ts_data);

	ret = ist30xx_calib_wait();

	ts_data->status.fw_update = 2;

	return ret;
}


int ist30xx_get_ic_fw_ver(void)
{
	return ist30xx_parse_param_ver(PARSE_FLAG_PARAM, ist30xx_param);

}

#if IST30XX_INTERNAL_BIN
int ist30xx_auto_fw_update(struct ist30xx_data *data)
{
	int ret = 0;
	int retry = IST30XX_FW_UPDATE_RETRY;
	bool isp_mode = false;
	printk(KERN_INFO "[TSP]ist30xx_auto_fw_update\n");
	ist30xx_get_update_info(data, ist30xx_fw, sizeof(ist30xx_fw));
	if ((data->fw.ver != 0) && (data->fw.ver != 0xFFFFFFFF)) {
		ist30xx_fw_ver =
			ist30xx_parse_fw_ver(PARSE_FLAG_FW, ist30xx_fw);
		if (data->fw.ver >= ist30xx_fw_ver)
			return 0;
	}

	DMSG("[ TSP ] F/W version is new. 0x%08x -> 0x%08x\n",
	     data->fw.ver, ist30xx_fw_ver);

	mutex_lock(&ist30xx_mutex);
	while (retry--) {
		ret = ist30xx_fw_update(data->client,
			ist30xx_fw, sizeof(ist30xx_fw),
			isp_mode);
		if (!ret)
			break;
	}
	mutex_unlock(&ist30xx_mutex);

	return ret;
}


int ist30xx_auto_param_update(struct ist30xx_data *data)
{
	int ret = 0;
	u32 val = 0;

	ist30xx_get_update_info(data, ist30xx_fw, sizeof(ist30xx_fw));
	if ((data->param_ver > 0) && (data->param_ver < 0xFFFFFFFF)) {
		val = ist30xx_parse_param_ver(PARSE_FLAG_PARAM, ist30xx_param);
		if (data->param_ver >= val)
			return 0;
	}

	DMSG("[ TSP ] Param version is new. 0x%04x -> 0x%04x\n",
	     data->param_ver, val);

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_param_update(data->client,
		ist30xx_param, sizeof(ist30xx_param));
	mutex_unlock(&ist30xx_mutex);

	return ret;
}

int ist30xx_force_fw_update(struct ist30xx_data *data)
{
	int ret = 0;
	int retry = IST30XX_FW_UPDATE_RETRY;
	bool isp_mode = false;

	mutex_lock(&ist30xx_mutex);
	while (retry--) {
		ret = ist30xx_fw_update(data->client,
			ist30xx_fw, sizeof(ist30xx_fw),
			isp_mode);
		if (!ret)
			break;
	}
	mutex_unlock(&ist30xx_mutex);

	return ret;
}
#endif

int ist30xx_force_param_update(struct ist30xx_data *data)
{
	int ret = 0;
	u32 val = 0;

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_param_update(data->client,
		ist30xx_param, sizeof(ist30xx_param));
	mutex_unlock(&ist30xx_mutex);

	return ret;
}

/* sysfs: /sys/class/touch/firmware/firmware */
ssize_t ist30xx_fw_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int retry = IST30XX_FW_UPDATE_RETRY;
	bool isp_mode = false;
	int fw_size = 0;
	u8 *fw = NULL;
	char *tmp;
	const struct firmware *request_fw = NULL;

	unsigned long mode = simple_strtoul(buf, &tmp, 10);

	if (mode == MASK_UPDATE_ISP || mode == MASK_UPDATE_FW) {
		ret = request_firmware(&request_fw,
			IST30XX_FW_NAME, &ts_data->client->dev);
		if (ret) {
			pr_err("[ TSP ] File not found, %s\n", IST30XX_FW_NAME);
			return size;
		}
		fw = (u8 *)request_fw->data;
		fw_size = (u32)request_fw->size;
		isp_mode = (mode == MASK_UPDATE_ISP ? true : false);
	} else {
#if IST30XX_INTERNAL_BIN
		fw = (u8 *)ist30xx_fw;
		fw_size = sizeof(ist30xx_fw);
#else
		return size;
#endif
	}

	ist30xx_get_update_info(ts_data, fw, fw_size);
	ist30xx_fw_ver = ist30xx_parse_fw_ver(PARSE_FLAG_FW, fw);

	mutex_lock(&ist30xx_mutex);
	while (retry--) {
		ret = ist30xx_fw_update(ts_data->client, fw, fw_size, isp_mode);
		if (!ret)
			break;
	}
	ist30xx_init_touch_driver(ts_data);
	mutex_unlock(&ist30xx_mutex);

	if (mode == MASK_UPDATE_ISP || mode == MASK_UPDATE_FW)
		release_firmware(request_fw);

	return size;
}

ssize_t ist30xx_fw_status(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int count;

	switch (ts_data->status.fw_update) {
	case 1:
		count = sprintf(buf, "Downloading\n");
		break;
	case 2:
		count = sprintf(buf,
				"Update success, ver %x -> %x, status : %d, gap : %d\n",
				ts_data->fw.pre_ver, ts_data->fw.ver,
				CALIB_TO_STATUS(ts_data->status.calib),
				CALIB_TO_GAP(ts_data->status.calib));
		break;
	default:
		count = sprintf(buf, "Pass\n");
	}

	return count;
}


/* sysfs: /sys/class/touch/firmware/viersion */
ssize_t ist30xx_fw_version(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	int count;

	count = sprintf(buf,
		"IST30XX id: 0x%x, f/w ver: 0x%x, param ver: 0x%x\n",
		ts_data->chip_id, ts_data->fw.ver, ts_data->param_ver);
	return count;
}


/* sysfs: /sys/class/touch/parameters/param */
ssize_t ist30xx_param_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t size)
{
	int ret = 0;
	u32 mode;
	int param_size = 0;
	u8 *param = NULL;
	char *tmp;
	const struct firmware *request_fw = NULL;

	mode = simple_strtoul(buf, &tmp, 10);
	if (mode == MASK_UPDATE_BIN) {
		ret = request_firmware(&request_fw,
			IST30XX_PARAM_NAME, &ts_data->client->dev);
		if (ret) {
			pr_err("[ TSP ] File not found, %s\n",
				IST30XX_PARAM_NAME);
			return size;
		}
		param = (u8 *)request_fw->data;
		param_size = (u32)request_fw->size;
	} else {
#if IST30XX_INTERNAL_BIN
		param = (u8 *)ist30xx_param;
		param_size = (u32)sizeof(ist30xx_param);
#else
		return size;
#endif
	}

	ist30xx_get_update_info(ts_data, param, param_size);

	mutex_lock(&ist30xx_mutex);
	ist30xx_param_update(ts_data->client, param, param_size);
	ist30xx_init_touch_driver(ts_data);
	mutex_unlock(&ist30xx_mutex);

	if (mode == MASK_UPDATE_BIN)
		release_firmware(request_fw);

	return size;
}

/* sysfs  */
static DEVICE_ATTR(firmware, 0666, ist30xx_fw_status, ist30xx_fw_store);
static DEVICE_ATTR(version, 0666, ist30xx_fw_version, NULL);
static DEVICE_ATTR(param, 0666, NULL, ist30xx_param_store);

struct class *ist30xx_class;
struct device *ist30xx_fw_dev;
struct device *ist30xx_param_dev;


int ist30xx_init_update_sysfs(void)
{
	/* /sys/class/touch */
	ist30xx_class = class_create(THIS_MODULE, "touch");

	/* /sys/class/touch/firmware */
	ist30xx_fw_dev = device_create(ist30xx_class,
		NULL, 0, NULL, "firmware");

	/* /sys/class/touch/firmwware/firmware */
	if (device_create_file(ist30xx_fw_dev, &dev_attr_firmware) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_firmware.attr.name);

	/* /sys/class/touch/firmware/version */
	if (device_create_file(ist30xx_fw_dev, &dev_attr_version) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_version.attr.name);

	/* /sys/class/touch/parameters */
	ist30xx_param_dev = device_create(ist30xx_class,
		NULL, 0, NULL, "parameters");

	/* /sys/class/touch/parameters/param */
	if (device_create_file(ist30xx_param_dev, &dev_attr_param) < 0)
		pr_err("[ TSP ] Failed to create device file(%s)!\n",
		       dev_attr_param.attr.name);

	return 0;
}

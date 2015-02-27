/*
 * Copyright (c) 2013-2014 Yamaha Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sysfs.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "buffer.h"
#include "iio.h"
#include "ring_sw.h"
#include "sysfs.h"
#include "trigger.h"
#include "trigger_consumer.h"
#include "yas.h"
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS532 \
	|| YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS533

#define YAS532_REG_DEVID		(0x80)
#define YAS532_REG_RCOILR		(0x81)
#define YAS532_REG_CMDR			(0x82)
#define YAS532_REG_CONFR		(0x83)
#define YAS532_REG_DLYR			(0x84)
#define YAS532_REG_OXR			(0x85)
#define YAS532_REG_OY1R			(0x86)
#define YAS532_REG_OY2R			(0x87)
#define YAS532_REG_TEST1R		(0x88)
#define YAS532_REG_TEST2R		(0x89)
#define YAS532_REG_CALR			(0x90)
#define YAS532_REG_DATAR		(0xB0)

#define YAS532_VERSION_AC_COEF_X	(850)
#define YAS532_VERSION_AC_COEF_Y1	(750)
#define YAS532_VERSION_AC_COEF_Y2	(750)
#define YAS532_DATA_CENTER		(4096)
#define YAS532_DATA_UNDERFLOW		(0)
#define YAS532_DATA_OVERFLOW		(8190)
#define YAS532_DEVICE_ID		(0x02)	/* YAS532 (MS-3R/3F) */
#define YAS532_TEMP20DEGREE_TYPICAL	(390)

#define YAS_X_OVERFLOW			(0x01)
#define YAS_X_UNDERFLOW			(0x02)
#define YAS_Y1_OVERFLOW			(0x04)
#define YAS_Y1_UNDERFLOW		(0x08)
#define YAS_Y2_OVERFLOW			(0x10)
#define YAS_Y2_UNDERFLOW		(0x20)
#define YAS_OVERFLOW	(YAS_X_OVERFLOW|YAS_Y1_OVERFLOW|YAS_Y2_OVERFLOW)
#define YAS_UNDERFLOW	(YAS_X_UNDERFLOW|YAS_Y1_UNDERFLOW|YAS_Y2_UNDERFLOW)

#define YAS532_MAG_STATE_NORMAL		(0)
#define YAS532_MAG_STATE_INIT_COIL	(1)
#define YAS532_MAG_STATE_MEASURE_OFFSET	(2)
#define YAS532_MAG_INITCOIL_TIMEOUT	(1000)	/* msec */
#define YAS532_MAG_TEMPERATURE_LOG	(10)
#define YAS532_MAG_NOTRANS_POSITION	(3)
#if YAS532_DRIVER_NO_SLEEP
#define YAS_MAG_MAX_BUSY_LOOP		(1000)
#endif
#define GEOMAGNETIC_RSTN_GPIO 64 
#define CHECK_RANGE(X, MIN, MAX)    (X>=MIN && X<=MAX)?1:0
#define CHECK_GREATER(X, MIN)    (X>=MIN)?1:0

#define set_vector(to, from) \
	{int _l; for (_l = 0; _l < 3; _l++) (to)[_l] = (from)[_l]; }
#define is_valid_offset(a) \
	(((a)[0] <= 31) && ((a)[1] <= 31) && ((a)[2] <= 31) \
		&& (-31 <= (a)[0]) && (-31 <= (a)[1]) && (-31 <= (a)[2]))

struct yas_cal_data {
	int8_t rxy1y2[3];
	uint8_t fxy1y2[3];
	int32_t Cx, Cy1, Cy2;
	int32_t a2, a3, a4, a5, a6, a7, a8, a9, k;
};
#if 1 < YAS532_MAG_TEMPERATURE_LOG
struct yas_temperature_filter {
	uint16_t log[YAS532_MAG_TEMPERATURE_LOG];
	int num;
	int idx;
};
#endif
struct yas_cdriver {
	int initialized;
	struct yas_cal_data cal;
	struct yas_driver_callback cbk;
	int measure_state;
	int8_t hard_offset[3];
	int32_t coef[3];
	int overflow;
	uint32_t overflow_time;
	int position;
	int delay;
	int enable;
	uint8_t dev_id;
	const int8_t *transform;
#if 1 < YAS532_MAG_TEMPERATURE_LOG
	struct yas_temperature_filter t;
#endif
	uint32_t current_time;
	uint16_t last_raw[4];
#if YAS532_DRIVER_NO_SLEEP
	int start_flag;
	int wait_flag;
#endif
};

static const int yas532_version_ac_coef[] = {YAS532_VERSION_AC_COEF_X,
	YAS532_VERSION_AC_COEF_Y1, YAS532_VERSION_AC_COEF_Y2};
static const int8_t INVALID_OFFSET[] = {0x7f, 0x7f, 0x7f};
static const int8_t YAS532_TRANSFORMATION[][9] = {
	{ 0,  1,  0, -1,  0,  0,  0,  0,  1 },
	{-1,  0,  0,  0, -1,  0,  0,  0,  1 },
	{ 0, -1,  0,  1,  0,  0,  0,  0,  1 },
	{ 1,  0,  0,  0,  1,  0,  0,  0,  1 },
	{ 0, -1,  0, -1,  0,  0,  0,  0, -1 },
	{ 1,  0,  0,  0, -1,  0,  0,  0, -1 },
	{ 0,  1,  0,  1,  0,  0,  0,  0, -1 },
	{-1,  0,  0,  0,  1,  0,  0,  0, -1 },
};
static struct yas_cdriver driver;

#define yas_read(a, b, c) \
	(driver.cbk.device_read(YAS_TYPE_MAG, (a), (b), (c)))
static int yas_single_write(uint8_t addr, uint8_t data)
{
	return driver.cbk.device_write(YAS_TYPE_MAG, addr, &data, 1);
}

static uint32_t curtime(void)
{
	if (driver.cbk.current_time)
		return driver.cbk.current_time();
	else
		return driver.current_time;
}

static void xy1y2_to_linear(uint16_t *xy1y2, int32_t *xy1y2_linear)
{
	static const uint16_t cval[] = {3721, 3971, 4221, 4471};
	int i;
	for (i = 0; i < 3; i++)
		xy1y2_linear[i] = xy1y2[i] - cval[driver.cal.fxy1y2[i]]
			+ (driver.hard_offset[i] - driver.cal.rxy1y2[i])
			* driver.coef[i];
}

static int get_cal_data_yas532(struct yas_cal_data *c)
{
	uint8_t data[14]; int i;
	if (yas_read(YAS532_REG_CALR, data, 14) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	if (yas_read(YAS532_REG_CALR, data, 14) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	c->fxy1y2[0] = (uint8_t)(((data[10]&0x01)<<1) | ((data[11]>>7)&0x01));
	c->rxy1y2[0] = ((int8_t)(((data[10]>>1) & 0x3f)<<2))>>2;
	c->fxy1y2[1] = (uint8_t)(((data[11]&0x01)<<1) | ((data[12]>>7)&0x01));
	c->rxy1y2[1] = ((int8_t)(((data[11]>>1) & 0x3f)<<2))>>2;
	c->fxy1y2[2] = (uint8_t)(((data[12]&0x01)<<1) | ((data[13]>>7)&0x01));
	c->rxy1y2[2] = ((int8_t)(((data[12]>>1) & 0x3f)<<2))>>2;
	c->Cx = data[0] * 10 - 1280;
	c->Cy1 = data[1] * 10 - 1280;
	c->Cy2 = data[2] * 10 - 1280;
	c->a2 = ((data[3]>>2)&0x03f) - 32;
	c->a3 = (uint8_t)(((data[3]<<2) & 0x0c) | ((data[4]>>6) & 0x03)) - 8;
	c->a4 = (uint8_t)(data[4] & 0x3f) - 32;
	c->a5 = ((data[5]>>2) & 0x3f) + 38;
	c->a6 = (uint8_t)(((data[5]<<4) & 0x30) | ((data[6]>>4) & 0x0f)) - 32;
	c->a7 = (uint8_t)(((data[6]<<3) & 0x78) | ((data[7]>>5) & 0x07)) - 64;
	c->a8 = (uint8_t)(((data[7]<<1) & 0x3e) | ((data[8]>>7) & 0x01)) - 32;
	c->a9 = (uint8_t)(((data[8]<<1) & 0xfe) | ((data[9]>>7) & 0x01));
	c->k = (uint8_t)((data[9]>>2) & 0x1f);
	for (i = 0; i < 13; i++)
		if (data[i] != 0)
			return YAS_NO_ERROR;
	if (data[13] & 0x80)
		return YAS_NO_ERROR;
	return YAS_ERROR_CALREG;
}

#if YAS532_DRIVER_NO_SLEEP
static int busy_wait(void)
{
	int i;
	uint8_t busy;
	for (i = 0; i < YAS_MAG_MAX_BUSY_LOOP; i++) {
		if (yas_read(YAS532_REG_DATAR, &busy, 1) < 0)
			return YAS_ERROR_DEVICE_COMMUNICATION;
		if (!(busy & 0x80))
			return YAS_NO_ERROR;
	}
	return YAS_ERROR_BUSY;
}

static int wait_if_busy(void)
{
	int rt;
	if (driver.start_flag && driver.wait_flag) {
		rt = busy_wait();
		if (rt < 0)
			return rt;
		driver.wait_flag = 0;
	}
	return YAS_NO_ERROR;
}
#endif

static int measure_start_yas532(int ldtc, int fors, int wait)
{
	uint8_t data = 0x01;
	data = (uint8_t)(data | (((!!ldtc)<<1) & 0x02));
	data = (uint8_t)(data | (((!!fors)<<2) & 0x04));
	if (yas_single_write(YAS532_REG_CMDR, data) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
#if YAS532_DRIVER_NO_SLEEP
	if (wait) {
		int rt;
		rt = busy_wait();
		if (rt < 0)
			return rt;
		driver.wait_flag = 0;
	} else
		driver.wait_flag = 1;
	driver.start_flag = 1;
#else
	(void) wait;
	driver.cbk.usleep(1500);
#endif
	return YAS_NO_ERROR;
}

static int measure_normal_yas532(int ldtc, int fors, int *busy, uint16_t *t,
		uint16_t *xy1y2, int *ouflow)
{
	uint8_t data[8];
	int i, rt;
#if YAS532_DRIVER_NO_SLEEP
	if (!driver.start_flag) {
#endif
		rt = measure_start_yas532(ldtc, fors, 1);
		if (rt < 0)
			return rt;
#if YAS532_DRIVER_NO_SLEEP
	}
#endif
	if (yas_read(YAS532_REG_DATAR, data, 8) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
#if YAS532_DRIVER_NO_SLEEP
	driver.start_flag = 0;
#endif
	*busy = (data[0]>>7) & 0x01;
	*t = (uint16_t)((((int32_t)data[0]<<3) & 0x3f8)|((data[1]>>5) & 0x07));
	xy1y2[0] = (uint16_t)((((int32_t)data[2]<<6) & 0x1fc0)
			| ((data[3]>>2) & 0x3f));
	xy1y2[1] = (uint16_t)((((int32_t)data[4]<<6) & 0x1fc0)
			| ((data[5]>>2) & 0x3f));
	xy1y2[2] = (uint16_t)((((int32_t)data[6]<<6) & 0x1fc0)
			| ((data[7]>>2) & 0x3f));
	*ouflow = 0;
	for (i = 0; i < 3; i++) {
		if (xy1y2[i] == YAS532_DATA_OVERFLOW)
			*ouflow |= (1<<(i*2));
		if (xy1y2[i] == YAS532_DATA_UNDERFLOW)
			*ouflow |= (1<<(i*2+1));
	}
	return YAS_NO_ERROR;
}

static int yas_cdrv_set_offset(const int8_t *offset)
{
	if (yas_single_write(YAS532_REG_OXR, (uint8_t)offset[0]) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	if (yas_single_write(YAS532_REG_OY1R, (uint8_t)offset[1]) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	if (yas_single_write(YAS532_REG_OY2R, (uint8_t)offset[2]) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	set_vector(driver.hard_offset, offset);
	return YAS_NO_ERROR;
}

static int yas_cdrv_measure_and_set_offset(void)
{
	static const int correct[5] = {16, 8, 4, 2, 1};
	int8_t hard_offset[3] = {0, 0, 0};
	uint16_t t, xy1y2[3];
	int32_t flag[3];
	int i, j, busy, ouflow, rt;
#if YAS532_DRIVER_NO_SLEEP
	driver.start_flag = 0;
#endif
	for (i = 0; i < 5; i++) {
		rt = yas_cdrv_set_offset(hard_offset);
		if (rt < 0)
			return rt;
		rt = measure_normal_yas532(0, 0, &busy, &t, xy1y2, &ouflow);
		if (rt < 0)
			return rt;
		if (busy)
			return YAS_ERROR_BUSY;
		for (j = 0; j < 3; j++) {
			if (YAS532_DATA_CENTER == xy1y2[j])
				flag[j] = 0;
			if (YAS532_DATA_CENTER < xy1y2[j])
				flag[j] = 1;
			if (xy1y2[j] < YAS532_DATA_CENTER)
				flag[j] = -1;
		}
		for (j = 0; j < 3; j++)
			if (flag[j])
				hard_offset[j] = (int8_t)(hard_offset[j]
						+ flag[j] * correct[i]);
	}
	return yas_cdrv_set_offset(hard_offset);
}

static int yas_cdrv_sensitivity_measuremnet(int32_t *sx, int32_t *sy)
{
	struct yas_cal_data *c = &driver.cal;
	uint16_t xy1y2_on[3], xy1y2_off[3], t;
	int busy, flowon = 0, flowoff = 0;
	if (measure_normal_yas532(1, 0, &busy, &t, xy1y2_on, &flowon) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	if (busy)
		return YAS_ERROR_BUSY;
	if (measure_normal_yas532(1, 1, &busy, &t, xy1y2_off, &flowoff) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	if (busy)
		return YAS_ERROR_BUSY;
	*sx = c->k * (xy1y2_on[0] - xy1y2_off[0]) * 10 / YAS_MAG_VCORE;
	*sy = c->k * c->a5 * ((xy1y2_on[1] - xy1y2_off[1])
			- (xy1y2_on[2] - xy1y2_off[2])) / 10 / YAS_MAG_VCORE;
	return flowon | flowoff;
}

static int yas_get_position(void)
{
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	return driver.position;
}

static int yas_set_position(int position)
{
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	if (position < 0 || 7 < position)
		return YAS_ERROR_ARG;
	if (position == YAS532_MAG_NOTRANS_POSITION)
		driver.transform = NULL;
	else
		driver.transform = YAS532_TRANSFORMATION[position];
	driver.position = position;
	return YAS_NO_ERROR;
}

static int yas_set_offset(const int8_t *hard_offset)
{
	if (!driver.enable) {
		set_vector(driver.hard_offset, hard_offset);
		return YAS_NO_ERROR;
	}
	if (is_valid_offset(hard_offset)) {
#if YAS532_DRIVER_NO_SLEEP
		int rt;
		rt = wait_if_busy();
		if (rt < 0)
			return rt;
#endif
		if (yas_cdrv_set_offset(hard_offset) < 0)
			return YAS_ERROR_DEVICE_COMMUNICATION;
		driver.measure_state = YAS532_MAG_STATE_NORMAL;
	} else {
		set_vector(driver.hard_offset, INVALID_OFFSET);
		driver.measure_state = YAS532_MAG_STATE_MEASURE_OFFSET;
	}
	return YAS_NO_ERROR;
}

static int yas_measure(struct yas_data *data, int num, int temp_correction,
		int *ouflow)
{
	struct yas_cal_data *c = &driver.cal;
	int32_t xy1y2_linear[3];
	int32_t xyz_tmp[3], tmp;
	int32_t sx, sy1, sy2, sy, sz;
	int i, busy;
	uint16_t t, xy1y2[3];
	uint32_t tm;
	int rt;
#if 1 < YAS532_MAG_TEMPERATURE_LOG
	int32_t sum = 0;
#endif
	*ouflow = 0;
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	if (data == NULL || num < 0)
		return YAS_ERROR_ARG;
	if (driver.cbk.current_time == NULL)
		driver.current_time += (uint32_t)driver.delay;
	if (num == 0)
		return 0;
	if (!driver.enable)
		return 0;
	switch (driver.measure_state) {
	case YAS532_MAG_STATE_INIT_COIL:
		tm = curtime();
		if (tm - driver.overflow_time < YAS532_MAG_INITCOIL_TIMEOUT)
			break;
		driver.overflow_time = tm;
		if (yas_single_write(YAS532_REG_RCOILR, 0x00) < 0)
			return YAS_ERROR_DEVICE_COMMUNICATION;
		if (!driver.overflow && is_valid_offset(driver.hard_offset)) {
			driver.measure_state = YAS532_MAG_STATE_NORMAL;
			break;
		}
		/* FALLTHRU */
	case YAS532_MAG_STATE_MEASURE_OFFSET:
		rt = yas_cdrv_measure_and_set_offset();
		if (rt < 0)
			return rt;
		driver.measure_state = YAS532_MAG_STATE_NORMAL;
		break;
	}

	if (measure_normal_yas532(0, 0, &busy, &t, xy1y2, ouflow) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	xy1y2_to_linear(xy1y2, xy1y2_linear);
#if 1 < YAS532_MAG_TEMPERATURE_LOG
	driver.t.log[driver.t.idx++] = t;
	if (YAS532_MAG_TEMPERATURE_LOG <= driver.t.idx)
		driver.t.idx = 0;
	driver.t.num++;
	if (YAS532_MAG_TEMPERATURE_LOG <= driver.t.num)
		driver.t.num = YAS532_MAG_TEMPERATURE_LOG;
	for (i = 0; i < driver.t.num; i++)
		sum += driver.t.log[i];
	tmp = sum * 10 / driver.t.num - YAS532_TEMP20DEGREE_TYPICAL * 10;
#else
	tmp = (t - YAS532_TEMP20DEGREE_TYPICAL) * 10;
#endif
	sx  = xy1y2_linear[0];
	sy1 = xy1y2_linear[1];
	sy2 = xy1y2_linear[2];
	if (temp_correction) {
		sx  -= (c->Cx  * tmp) / 1000;
		sy1 -= (c->Cy1 * tmp) / 1000;
		sy2 -= (c->Cy2 * tmp) / 1000;
	}
	sy = sy1 - sy2;
	sz = -sy1 - sy2;
	data->xyz.v[0] = c->k * ((100   * sx + c->a2 * sy + c->a3 * sz) / 10);
	data->xyz.v[1] = c->k * ((c->a4 * sx + c->a5 * sy + c->a6 * sz) / 10);
	data->xyz.v[2] = c->k * ((c->a7 * sx + c->a8 * sy + c->a9 * sz) / 10);
	if (driver.transform != NULL) {
		for (i = 0; i < 3; i++) {
			xyz_tmp[i] = driver.transform[i*3] * data->xyz.v[0]
				+ driver.transform[i*3+1] * data->xyz.v[1]
				+ driver.transform[i*3+2] * data->xyz.v[2];
		}
		set_vector(data->xyz.v, xyz_tmp);
	}
	for (i = 0; i < 3; i++) {
		data->xyz.v[i] -= data->xyz.v[i] % 10;
		if (*ouflow & (1<<(i*2)))
			data->xyz.v[i] += 1; /* set overflow */
		if (*ouflow & (1<<(i*2+1)))
			data->xyz.v[i] += 2; /* set underflow */
	}
	tm = curtime();
	data->type = YAS_TYPE_MAG;
	if (driver.cbk.current_time)
		data->timestamp = tm;
	else
		data->timestamp = 0;
	data->accuracy = 0;
	if (busy)
		return YAS_ERROR_BUSY;
	if (0 < *ouflow) {
		if (!driver.overflow)
			driver.overflow_time = tm;
		driver.overflow = 1;
		driver.measure_state = YAS532_MAG_STATE_INIT_COIL;
	} else
		driver.overflow = 0;
	for (i = 0; i < 3; i++)
		driver.last_raw[i] = xy1y2[i];
	driver.last_raw[i] = t;
#if YAS532_DRIVER_NO_SLEEP
	rt = measure_start_yas532(0, 0, 0);
	if (rt < 0)
		return rt;
#endif
	return 1;
}

static int yas_measure_wrap(struct yas_data *data, int num)
{
	int ouflow;
	return yas_measure(data, num, 1, &ouflow);
}

static int yas_get_delay(void)
{
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	return driver.delay;
}

static int yas_set_delay(int delay)
{
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	if (delay < 0)
		return YAS_ERROR_ARG;
	driver.delay = delay;
	return YAS_NO_ERROR;
}

static int yas_get_enable(void)
{
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	return driver.enable;
}

static int yas_set_enable(int enable)
{
	int rt = YAS_NO_ERROR;
        int gpio = 0;

        gpio = gpio_get_value(GEOMAGNETIC_RSTN_GPIO);
	
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	enable = !!enable;
	if (driver.enable == enable)
		return YAS_NO_ERROR;
	if (enable) {
	    if(gpio == 0)
            {
	        if(gpio_request(GEOMAGNETIC_RSTN_GPIO,"geomagnetic-rstn") < 0)
	        {
		        printk(KERN_ERR "%s: gpio_request geomagnetic-rstn", __FUNCTION__);
	        }
	        if(gpio_direction_output(GEOMAGNETIC_RSTN_GPIO, 1) < 0)
	        {
		        printk(KERN_ERR "%s: gpio_direction_output geomagnetic-rstn", __FUNCTION__);
	        }
            }
		if (driver.cbk.device_open(YAS_TYPE_MAG) < 0)
			return YAS_ERROR_DEVICE_COMMUNICATION;
		if (yas_single_write(YAS532_REG_TEST1R, 0x00) < 0) {
			driver.cbk.device_close(YAS_TYPE_MAG);
			return YAS_ERROR_DEVICE_COMMUNICATION;
		}
		if (yas_single_write(YAS532_REG_TEST2R, 0x00) < 0) {
			driver.cbk.device_close(YAS_TYPE_MAG);
			return YAS_ERROR_DEVICE_COMMUNICATION;
		}
		if (yas_single_write(YAS532_REG_RCOILR, 0x00) < 0) {
			driver.cbk.device_close(YAS_TYPE_MAG);
			return YAS_ERROR_DEVICE_COMMUNICATION;
		}
		if (is_valid_offset(driver.hard_offset)) {
			if (yas_cdrv_set_offset(driver.hard_offset) < 0) {
				driver.cbk.device_close(YAS_TYPE_MAG);
				return YAS_ERROR_DEVICE_COMMUNICATION;
			}
			driver.measure_state = YAS532_MAG_STATE_NORMAL;
		} else {
			set_vector(driver.hard_offset, INVALID_OFFSET);
			driver.measure_state = YAS532_MAG_STATE_MEASURE_OFFSET;
		}
	} else {
#if YAS532_DRIVER_NO_SLEEP
		rt = wait_if_busy();
#endif
		driver.cbk.device_close(YAS_TYPE_MAG);
	}
	driver.enable = enable;
	return rt;
}

static int yas_ext(int32_t cmd, void *p)
{
	struct yas532_self_test_result *r;
	struct yas_data data;
	int32_t xy1y2_linear[3], *raw_xyz;
	int rt, i, enable, ouflow;
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	if (p == NULL)
		return YAS_ERROR_ARG;
	switch (cmd) {
	case YAS532_SELF_TEST:
		r = (struct yas532_self_test_result *) p;
		r->id = driver.dev_id;
		enable = driver.enable;
		if (!enable) {
			rt = yas_set_enable(1);
			if (rt < 0)
				return rt;
		}
#if YAS532_DRIVER_NO_SLEEP
		rt = wait_if_busy();
		if (rt < 0)
			return rt;
#endif
		if (yas_single_write(YAS532_REG_RCOILR, 0x00) < 0) {
			if (!enable)
				yas_set_enable(0);
			return YAS_ERROR_DEVICE_COMMUNICATION;
		}
		yas_set_offset(INVALID_OFFSET);
		rt = yas_measure(&data, 1, 0, &ouflow);
		set_vector(r->xy1y2, driver.hard_offset);
		if (rt < 0) {
			if (!enable)
				yas_set_enable(0);
			return rt;
		}
		if (ouflow & YAS_OVERFLOW) {
			if (!enable)
				yas_set_enable(0);
			return YAS_ERROR_OVERFLOW;
		}
		if (ouflow & YAS_UNDERFLOW) {
			if (!enable)
				yas_set_enable(0);
			return YAS_ERROR_UNDERFLOW;
		}
		if (data.xyz.v[0] == 0 && data.xyz.v[1] == 0
				&& data.xyz.v[2] == 0) {
			if (!enable)
				yas_set_enable(0);
			return YAS_ERROR_DIRCALC;
		}
		r->dir = 99;
		for (i = 0; i < 3; i++)
			r->xyz[i] = data.xyz.v[i] / 1000;
#if YAS532_DRIVER_NO_SLEEP
		rt = wait_if_busy();
		if (rt < 0) {
			if (!enable)
				yas_set_enable(0);
			return rt;
		}
		driver.start_flag = 0;
#endif
		rt = yas_cdrv_sensitivity_measuremnet(&r->sx, &r->sy);
		if (rt < 0) {
			if (!enable)
				yas_set_enable(0);
			return rt;
		}
		if (rt & YAS_OVERFLOW) {
			if (!enable)
				yas_set_enable(0);
			return YAS_ERROR_OVERFLOW;
		}
		if (rt & YAS_UNDERFLOW) {
			if (!enable)
				yas_set_enable(0);
			return YAS_ERROR_UNDERFLOW;
		}
		if (!enable)
			yas_set_enable(0);
		return YAS_NO_ERROR;
	case YAS532_SELF_TEST_NOISE:
		raw_xyz = (int32_t *) p;
		enable = driver.enable;
		if (!enable) {
			rt = yas_set_enable(1);
			if (rt < 0)
				return rt;
		}
#if YAS532_DRIVER_NO_SLEEP
		rt = wait_if_busy();
		if (rt < 0)
			return rt;
#endif
		rt = yas_measure(&data, 1, 0, &ouflow);
		if (rt < 0) {
			if (!enable)
				yas_set_enable(0);
			return rt;
		}
#if YAS532_DRIVER_NO_SLEEP
		rt = wait_if_busy();
		if (rt < 0) {
			if (!enable)
				yas_set_enable(0);
			return rt;
		}
#endif
		xy1y2_to_linear(driver.last_raw, xy1y2_linear);
		raw_xyz[0] = xy1y2_linear[0];
		raw_xyz[1] = xy1y2_linear[1] - xy1y2_linear[2];
		raw_xyz[2] = -xy1y2_linear[1] - xy1y2_linear[2];
		if (!enable)
			yas_set_enable(0);
		return YAS_NO_ERROR;
	case YAS532_GET_HW_OFFSET:
		set_vector((int8_t *) p, driver.hard_offset);
		return YAS_NO_ERROR;
	case YAS532_SET_HW_OFFSET:
		return yas_set_offset((int8_t *) p);
	case YAS532_GET_LAST_RAWDATA:
		for (i = 0; i < 4; i++)
			((uint16_t *) p)[i] = driver.last_raw[i];
		return YAS_NO_ERROR;
	default:
		break;
	}
	return YAS_ERROR_ARG;
}

uint8_t g_iio_compass_product_id=0;
static int yas_init(void)
{
	int i, rt;
	uint8_t data;
	if (driver.initialized)
		return YAS_ERROR_INITIALIZE;
	if (driver.cbk.device_open(YAS_TYPE_MAG) < 0)
		return YAS_ERROR_DEVICE_COMMUNICATION;
	if (yas_read(YAS532_REG_DEVID, &data, 1) < 0) {
		driver.cbk.device_close(YAS_TYPE_MAG);
		return YAS_ERROR_DEVICE_COMMUNICATION;
	}
	driver.dev_id = data;
	g_iio_compass_product_id = driver.dev_id;
	if (driver.dev_id != YAS532_DEVICE_ID) {
		driver.cbk.device_close(YAS_TYPE_MAG);
		return YAS_ERROR_CHIP_ID;
	}
	rt = get_cal_data_yas532(&driver.cal);
	if (rt < 0) {
		driver.cbk.device_close(YAS_TYPE_MAG);
		return rt;
	}
	driver.cbk.device_close(YAS_TYPE_MAG);

	driver.measure_state = YAS532_MAG_STATE_INIT_COIL;
	set_vector(driver.hard_offset, INVALID_OFFSET);
	driver.overflow = 0;
	driver.overflow_time = driver.current_time;
	driver.position = YAS532_MAG_NOTRANS_POSITION;
	driver.delay = YAS_DEFAULT_SENSOR_DELAY;
	driver.enable = 0;
	driver.transform = NULL;
#if YAS532_DRIVER_NO_SLEEP
	driver.start_flag = 0;
	driver.wait_flag = 0;
#endif
#if 1 < YAS532_MAG_TEMPERATURE_LOG
	driver.t.num = driver.t.idx = 0;
#endif
	driver.current_time = curtime();
	for (i = 0; i < 3; i++) {
		driver.coef[i] = yas532_version_ac_coef[i];
		driver.last_raw[i] = 0;
	}
	driver.last_raw[3] = 0;
	driver.initialized = 1;
	return YAS_NO_ERROR;
}

static int yas_term(void)
{
	int rt;
	if (!driver.initialized)
		return YAS_ERROR_INITIALIZE;
	rt = yas_set_enable(0);
	driver.initialized = 0;
	return rt;
}

int yas_mag_driver_init(struct yas_mag_driver *f)
{
	if (f == NULL || f->callback.device_open == NULL
			|| f->callback.device_close == NULL
			|| f->callback.device_read == NULL
			|| f->callback.device_write == NULL
#if !YAS532_DRIVER_NO_SLEEP
			|| f->callback.usleep == NULL
#endif
	   )
		return YAS_ERROR_ARG;
	f->init = yas_init;
	f->term = yas_term;
	f->get_delay = yas_get_delay;
	f->set_delay = yas_set_delay;
	f->get_enable = yas_get_enable;
	f->set_enable = yas_set_enable;
	f->get_position = yas_get_position;
	f->set_position = yas_set_position;
	f->measure = yas_measure_wrap;
	f->ext = yas_ext;
	driver.cbk = f->callback;
	yas_term();
	return YAS_NO_ERROR;
}
#endif

static struct i2c_client *this_client;

enum {
	YAS_SCAN_MAGN_X,
	YAS_SCAN_MAGN_Y,
	YAS_SCAN_MAGN_Z,
	YAS_SCAN_TIMESTAMP,
};
struct yas_state {
	struct mutex lock;
	struct yas_mag_driver mag;
	struct i2c_client *client;
	struct iio_trigger  *trig;
	struct delayed_work work;
	int16_t sampling_frequency;
	atomic_t pseudo_irq_enable;
	int32_t compass_data[3];
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend sus;
#endif
};

static int yas_device_open(int32_t type)
{
	return 0;
}

static int yas_device_close(int32_t type)
{
	return 0;
}

static int yas_device_write(int32_t type, uint8_t addr, const uint8_t *buf,
		int len)
{
	uint8_t tmp[2];
	if (sizeof(tmp) - 1 < len)
		return -1;
	tmp[0] = addr;
	memcpy(&tmp[1], buf, len);
	if (i2c_master_send(this_client, tmp, len + 1) < 0)
		return -1;
	return 0;
}

static int yas_device_read(int32_t type, uint8_t addr, uint8_t *buf, int len)
{
	struct i2c_msg msg[2];
	int err;
	msg[0].addr = this_client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &addr;
	msg[1].addr = this_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = buf;
	err = i2c_transfer(this_client->adapter, msg, 2);
	if (err != 2) {
		dev_err(&this_client->dev,
				"i2c_transfer() read error: "
				"slave_addr=%02x, reg_addr=%02x, err=%d\n",
				this_client->addr, addr, err);
		return err;
	}
	return 0;
}

static void yas_usleep(int us)
{
	usleep_range(us, us + 1000);
}

static uint32_t yas_current_time(void)
{
	return jiffies_to_msecs(jiffies);
}

static int yas_pseudo_irq_enable(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	if (!atomic_cmpxchg(&st->pseudo_irq_enable, 0, 1))
		schedule_delayed_work(&st->work, 0);
	return 0;
}

static int yas_pseudo_irq_disable(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	if (atomic_cmpxchg(&st->pseudo_irq_enable, 1, 0))
		cancel_delayed_work_sync(&st->work);
	return 0;
}

static int yas_set_pseudo_irq(struct iio_dev *indio_dev, int enable)
{
	if (enable)
		yas_pseudo_irq_enable(indio_dev);
	else
		yas_pseudo_irq_disable(indio_dev);
	return 0;
}

static int yas_data_rdy_trig_poll(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	iio_trigger_poll(st->trig, iio_get_time_ns());
	return 0;
}

static irqreturn_t yas_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct iio_buffer *buffer = indio_dev->buffer;
	struct yas_state *st = iio_priv(indio_dev);
	int len = 0, i, j;
	size_t datasize = buffer->access->get_bytes_per_datum(buffer);
	int32_t *mag;

	mag = (int32_t *) kmalloc(datasize, GFP_KERNEL);
	if (mag == NULL) {
		dev_err(indio_dev->dev.parent,
				"memory alloc failed in buffer bh");
		return -ENOMEM;
	}
	if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength)) {
		j = 0;
		for (i = 0; i < 3; i++) {
			if (test_bit(i, indio_dev->active_scan_mask)) {
				mag[j] = st->compass_data[i];
				j++;
			}
		}
		len = j * 4;
	}

	/* Guaranteed to be aligned with 8 byte boundary */
	if (buffer->scan_timestamp)
		*(s64 *)(((phys_addr_t)mag + len
					+ sizeof(s64) - 1) & ~(sizeof(s64) - 1))
			= pf->timestamp;
	buffer->access->store_to(buffer, (u8 *)mag, pf->timestamp);

	iio_trigger_notify_done(indio_dev->trig);
	kfree(mag);
	return IRQ_HANDLED;
}

static int yas_data_rdy_trigger_set_state(struct iio_trigger *trig,
		bool state)
{
	struct iio_dev *indio_dev = trig->private_data;
	yas_set_pseudo_irq(indio_dev, state);
	return 0;
}

static const struct iio_trigger_ops yas_trigger_ops = {
	.owner = THIS_MODULE,
	.set_trigger_state = &yas_data_rdy_trigger_set_state,
};

static int yas_probe_trigger(struct iio_dev *indio_dev)
{
	int ret;
	struct yas_state *st = iio_priv(indio_dev);
	indio_dev->pollfunc = iio_alloc_pollfunc(&iio_pollfunc_store_time,
			&yas_trigger_handler, IRQF_ONESHOT, indio_dev,
			"%s_consumer%d", indio_dev->name, indio_dev->id);
	if (indio_dev->pollfunc == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	st->trig = iio_allocate_trigger("%s-dev%d",
			indio_dev->name,
			indio_dev->id);
	if (!st->trig) {
		ret = -ENOMEM;
		goto error_dealloc_pollfunc;
	}
	st->trig->dev.parent = &st->client->dev;
	st->trig->ops = &yas_trigger_ops;
	st->trig->private_data = indio_dev;
	ret = iio_trigger_register(st->trig);
	if (ret)
		goto error_free_trig;
	return 0;

error_free_trig:
	iio_free_trigger(st->trig);
error_dealloc_pollfunc:
	iio_dealloc_pollfunc(indio_dev->pollfunc);
error_ret:
	return ret;
}

static void yas_remove_trigger(struct iio_dev *indio_dev)
{
	struct yas_state *st = iio_priv(indio_dev);
	iio_trigger_unregister(st->trig);
	iio_free_trigger(st->trig);
	iio_dealloc_pollfunc(indio_dev->pollfunc);
}

static const struct iio_buffer_setup_ops yas_buffer_setup_ops = {
	.preenable = &iio_sw_buffer_preenable,
	.postenable = &iio_triggered_buffer_postenable,
	.predisable = &iio_triggered_buffer_predisable,
};

static void yas_remove_buffer(struct iio_dev *indio_dev)
{
	iio_buffer_unregister(indio_dev);
	iio_sw_rb_free(indio_dev->buffer);
};

static int yas_probe_buffer(struct iio_dev *indio_dev)
{
	int ret;
	struct iio_buffer *buffer;

	buffer = iio_sw_rb_allocate(indio_dev);
	if (!buffer) {
		ret = -ENOMEM;
		goto error_ret;
	}
	buffer->scan_timestamp = true;
	indio_dev->buffer = buffer;
	indio_dev->setup_ops = &yas_buffer_setup_ops;
	indio_dev->modes |= INDIO_BUFFER_TRIGGERED;
	ret = iio_buffer_register(indio_dev, indio_dev->channels,
			indio_dev->num_channels);
	if (ret)
		goto error_free_buf;
	iio_scan_mask_set(indio_dev, indio_dev->buffer, YAS_SCAN_MAGN_X);
	iio_scan_mask_set(indio_dev, indio_dev->buffer, YAS_SCAN_MAGN_Y);
	iio_scan_mask_set(indio_dev, indio_dev->buffer, YAS_SCAN_MAGN_Z);
	return 0;

error_free_buf:
	iio_sw_rb_free(indio_dev->buffer);
error_ret:
	return ret;
}

static ssize_t yas_position_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int ret;
	mutex_lock(&st->lock);
	ret = st->mag.get_position();
	mutex_unlock(&st->lock);
	if (ret < 0)
		return -EFAULT;
	return sprintf(buf, "%d\n", ret);
}

static ssize_t yas_position_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int ret, position;
	sscanf(buf, "%d\n", &position);
	mutex_lock(&st->lock);
	ret = st->mag.set_position(position);
	mutex_unlock(&st->lock);
	if (ret < 0)
		return -EFAULT;
	return count;
}

#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS532 \
	|| YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS533
static ssize_t yas_hard_offset_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int8_t hard_offset[3];
	int ret;
	mutex_lock(&st->lock);
	ret = st->mag.ext(YAS532_GET_HW_OFFSET, hard_offset);
	mutex_unlock(&st->lock);
	if (ret < 0)
		return -EFAULT;
	return sprintf(buf, "%d %d %d\n", hard_offset[0], hard_offset[1],
			hard_offset[2]);
}

static ssize_t yas_hard_offset_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int32_t tmp[3];
	int8_t hard_offset[3];
	int ret, i;
	sscanf(buf, "%d %d %d\n", &tmp[0], &tmp[1], &tmp[2]);
	for (i = 0; i < 3; i++)
		hard_offset[i] = (int8_t)tmp[i];
	mutex_lock(&st->lock);
	ret = st->mag.ext(YAS532_SET_HW_OFFSET, hard_offset);
	mutex_unlock(&st->lock);
	if (ret < 0)
		return -EFAULT;
	return count;
}
#endif

static ssize_t yas_sampling_frequency_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	return sprintf(buf, "%d\n", st->sampling_frequency);
}

static ssize_t yas_sampling_frequency_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int ret, data;
	ret = kstrtoint(buf, 10, &data);
	if (ret)
		return ret;
	if (data <= 0)
		return -EINVAL;
	st->sampling_frequency = data;
	return count;
}

#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS532 \
	|| YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS533

int checkresult(int ret, int id, int x, int y1, int y2, int dir, int sx,
			int sy, int ohx, int ohy, int ohz)
{
	
	if(ret)
	    return 0;
	if(id != 0x02)
	    return 0;
	if(!CHECK_RANGE(x, -30, 30))
	    return 0;
	if(!CHECK_RANGE(y1, -30, 30))
	    return 0;
	if(!CHECK_RANGE(y2, -30, 30))
	    return 0;
	if(!CHECK_RANGE(dir, 0, 359))
	    return 0;
	if(!CHECK_GREATER(sx,17))
	    return 0;
	if(!CHECK_GREATER(sy,22))
	    return 0;
	    
	return 1;
}

static ssize_t yas_self_test_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	struct yas532_self_test_result r;
	int ret;
	int res = 0;

	mutex_lock(&st->lock);
	ret = st->mag.ext(YAS532_SELF_TEST, &r);
	mutex_unlock(&st->lock);

	res = checkresult(ret, r.id, r.xy1y2[0], r.xy1y2[1], r.xy1y2[2],
			r.dir, r.sx, r.sy, r.xyz[0], r.xyz[1], r.xyz[2]);
	return sprintf(buf, "ret=%d, id=%d, xy1y2=%d %d %d, dir=%d, sx=%d, sy=%d, ohxyz=%d %d %d\nself_test: %s\n",
			ret, r.id, r.xy1y2[0], r.xy1y2[1], r.xy1y2[2],
			r.dir, r.sx, r.sy, r.xyz[0], r.xyz[1], r.xyz[2], (res)?"Pass":"Fail");
}

static ssize_t yas_self_test_noise_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	int32_t xyz_raw[3];
	int ret;
	mutex_lock(&st->lock);
	ret = st->mag.ext(YAS532_SELF_TEST_NOISE, xyz_raw);
	mutex_unlock(&st->lock);
	if (ret < 0)
		return -EFAULT;
	return sprintf(buf, "%d %d %d\n", xyz_raw[0], xyz_raw[1], xyz_raw[2]);
}

static ssize_t yas_ping(struct device *dev,
		struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "0x2e:0x%02x\n", driver.dev_id);
}
#endif

static int yas_read_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan,
		int *val,
		int *val2,
		long mask) {
	struct yas_state  *st = iio_priv(indio_dev);
	int ret = -EINVAL;

	if (chan->type != IIO_MAGN)
		return -EINVAL;

	mutex_lock(&st->lock);

	switch (mask) {
	case 0:
		*val = st->compass_data[chan->channel2 - IIO_MOD_X];
		ret = IIO_VAL_INT;
		break;
	case IIO_CHAN_INFO_SCALE:
		/* Gain : counts / uT = 1000 [nT] */
		/* Scaling factor : 1000000 / Gain = 1000 */
		*val = 0;
		*val2 = 1000;
		ret = IIO_VAL_INT_PLUS_MICRO;
		break;
	}

	mutex_unlock(&st->lock);

	return ret;
}

static void yas_work_func(struct work_struct *work)
{
	struct yas_data mag[1];
	struct yas_state *st =
		container_of((struct delayed_work *)work,
				struct yas_state, work);
	struct iio_dev *indio_dev = iio_priv_to_dev(st);
	uint32_t time_before, time_after;
	int32_t delay;
	int ret, i;

	time_before = jiffies_to_msecs(jiffies);
	mutex_lock(&st->lock);
	ret = st->mag.measure(mag, 1);
	if (ret == 1) {
		for (i = 0; i < 3; i++)
			st->compass_data[i] = mag[0].xyz.v[i];
	}
	mutex_unlock(&st->lock);
	if (ret == 1)
		yas_data_rdy_trig_poll(indio_dev);
	time_after = jiffies_to_msecs(jiffies);
	delay = MSEC_PER_SEC / st->sampling_frequency
		- (time_after - time_before);
	if (delay <= 0)
		delay = 1;
	schedule_delayed_work(&st->work, msecs_to_jiffies(delay));
}

#define YAS_MAGNETOMETER_CHANNEL(axis)		\
{							\
	.type = IIO_MAGN,				\
	.modified = 1,					\
	.channel2 = IIO_MOD_##axis,			\
	.info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT,	\
	.scan_index = YAS_SCAN_MAGN_##axis,		\
	.scan_type = IIO_ST('s', 32, 32, 0)		\
}

static const struct iio_chan_spec yas_channels[] = {
	YAS_MAGNETOMETER_CHANNEL(X),
	YAS_MAGNETOMETER_CHANNEL(Y),
	YAS_MAGNETOMETER_CHANNEL(Z),
	IIO_CHAN_SOFT_TIMESTAMP(YAS_SCAN_TIMESTAMP)
};

static IIO_DEVICE_ATTR(sampling_frequency, S_IRUSR|S_IWUSR,
		yas_sampling_frequency_show,
		yas_sampling_frequency_store, 0);
static IIO_DEVICE_ATTR(position, S_IRUSR|S_IWUSR,
		yas_position_show, yas_position_store, 0);
#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS532 \
	|| YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS533
static IIO_DEVICE_ATTR(hard_offset, S_IRUSR|S_IWUSR,
		yas_hard_offset_show, yas_hard_offset_store, 0);
static IIO_DEVICE_ATTR(self_test, S_IRUGO|S_IWUSR|S_IWGRP, yas_self_test_show, NULL, 0);
static IIO_DEVICE_ATTR(self_test_noise, S_IRUSR, yas_self_test_noise_show,
		NULL, 0);
static IIO_DEVICE_ATTR(ping, S_IRUGO|S_IWUSR|S_IWGRP, yas_ping, NULL, 0);
#endif

static struct attribute *yas_attributes[] = {
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	&iio_dev_attr_position.dev_attr.attr,
#if YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS532 \
	|| YAS_MAG_DRIVER == YAS_MAG_DRIVER_YAS533
	&iio_dev_attr_hard_offset.dev_attr.attr,
	&iio_dev_attr_self_test.dev_attr.attr,
	&iio_dev_attr_self_test_noise.dev_attr.attr,
	&iio_dev_attr_ping.dev_attr.attr, 
#endif
	NULL
};
static const struct attribute_group yas_attribute_group = {
	.attrs = yas_attributes,
};

static const struct iio_info yas_info = {
	.read_raw = &yas_read_raw,
	.attrs = &yas_attribute_group,
	.driver_module = THIS_MODULE,
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void yas_early_suspend(struct early_suspend *h)
{
	struct yas_state *st = container_of(h,
			struct yas_state, sus);
	if (atomic_read(&st->pseudo_irq_enable))
		cancel_delayed_work_sync(&st->work);
}


static void yas_late_resume(struct early_suspend *h)
{
	struct yas_state *st = container_of(h,
			struct yas_state, sus);
	if (atomic_read(&st->pseudo_irq_enable))
		schedule_delayed_work(&st->work, 0);
}
#endif

static void mag_sensor_power_on(struct i2c_client *client)
{
	static struct regulator *reg_l19;
	static struct regulator *reg_lvs1;
	int error;

printk(KERN_INFO "%s: mag power on start\n", __func__);

	//get power and set voltage level
	reg_l19 = regulator_get(&client->dev, "vdd");
	if (IS_ERR(reg_l19)) {
		printk("%s: Regulator get failed vdd rc=%ld\n", __FUNCTION__, PTR_ERR(reg_l19));
	}
	if (regulator_count_voltages(reg_l19) > 0) {
		error = regulator_set_voltage(reg_l19,  2850000, 2850000);
		if (error) {
			printk("%s: regulator set_vtg vdd failed rc=%d\n", __FUNCTION__, error);
		}
	}

	reg_lvs1 = regulator_get(&client->dev,"vddio");
	if (IS_ERR(reg_lvs1)){
		printk("could not get vddio lvs1, rc = %ld\n", PTR_ERR(reg_lvs1));
		}

	//enable power

	error = regulator_set_optimum_mode(reg_l19, 100000);
	if (error < 0) {
		printk("%s: Regulator vdd set_opt failed rc=%d\n", __FUNCTION__, error);
		regulator_put(reg_l19);
	}

	error = regulator_enable(reg_l19);
	if (error) {
		printk("%s: Regulator vdd enable failed rc=%d\n", __FUNCTION__, error);
		regulator_put(reg_l19);
	}

	error = regulator_enable(reg_lvs1);
	if (error) {
		printk("%s: enable vddio lvs1 failed, rc=%d\n", __FUNCTION__, error);
		regulator_put(reg_lvs1);
	}

printk(KERN_INFO "%s: mag power on end\n", __func__);

	error = gpio_tlmm_config(GPIO_CFG(GEOMAGNETIC_RSTN_GPIO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if(error < 0)
	{
		printk(KERN_ERR "%s: gpio_tlmm_config geomagnetic-rstn, err=%d", __FUNCTION__, error);
	}
	error = gpio_request(GEOMAGNETIC_RSTN_GPIO,"geomagnetic-rstn");
	if(error < 0)
	{
		printk(KERN_ERR "%s: gpio_request geomagnetic-rstn, err=%d", __FUNCTION__, error);
	}
	error = gpio_direction_output(GEOMAGNETIC_RSTN_GPIO, 1);
	if(error < 0)
	{
		printk(KERN_ERR "%s: gpio_direction_output geomagnetic-rstn, err=%d", __FUNCTION__, error);
	}

	mdelay(3);// delay 3 ms for power ready when issue first I2C command
	
}

static int yas_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	struct yas_state *st;
	struct iio_dev *indio_dev;
	int ret;

	this_client = i2c;
	mag_sensor_power_on(i2c);
	
	indio_dev = iio_allocate_device(sizeof(*st));
	if (!indio_dev) {
		ret = -ENOMEM;
		goto error_ret;
	}
	i2c_set_clientdata(i2c, indio_dev);

	indio_dev->name = id->name;
	indio_dev->dev.parent = &i2c->dev;
	indio_dev->info = &yas_info;
	indio_dev->channels = yas_channels;
	indio_dev->num_channels = ARRAY_SIZE(yas_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	st = iio_priv(indio_dev);
	st->client = i2c;
	st->sampling_frequency = 20;
	st->mag.callback.device_open = yas_device_open;
	st->mag.callback.device_close = yas_device_close;
	st->mag.callback.device_read = yas_device_read;
	st->mag.callback.device_write = yas_device_write;
	st->mag.callback.usleep = yas_usleep;
	st->mag.callback.current_time = yas_current_time;
	INIT_DELAYED_WORK(&st->work, yas_work_func);
	mutex_init(&st->lock);
#ifdef CONFIG_HAS_EARLYSUSPEND
	st->sus.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	st->sus.suspend = yas_early_suspend;
	st->sus.resume = yas_late_resume;
	register_early_suspend(&st->sus);
#endif

	ret = yas_probe_buffer(indio_dev);
	if (ret)
		goto error_free_dev;
	ret = yas_probe_trigger(indio_dev);
	if (ret)
		goto error_remove_buffer;
	ret = iio_device_register(indio_dev);
	if (ret)
		goto error_remove_trigger;
	ret = yas_mag_driver_init(&st->mag);
	if (ret < 0) {
		ret = -EFAULT;
		goto error_unregister_iio;
	}
	ret = st->mag.init();
	if (ret < 0) {
		ret = -EFAULT;
		goto error_unregister_iio;
	}
	ret = st->mag.set_enable(1);
	if (ret < 0) {
		ret = -EFAULT;
		goto error_driver_term;
	}
	
	return 0;

error_driver_term:
	st->mag.term();
error_unregister_iio:
	iio_device_unregister(indio_dev);
error_remove_trigger:
	yas_remove_trigger(indio_dev);
error_remove_buffer:
	yas_remove_buffer(indio_dev);
error_free_dev:
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&st->sus);
#endif
	iio_free_device(indio_dev);
error_ret:
	i2c_set_clientdata(i2c, NULL);
	this_client = NULL;
	return ret;
}

static int yas_remove(struct i2c_client *i2c)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(i2c);
	struct yas_state *st;
	if (indio_dev) {
		st = iio_priv(indio_dev);
#ifdef CONFIG_HAS_EARLYSUSPEND
		unregister_early_suspend(&st->sus);
#endif
		yas_pseudo_irq_disable(indio_dev);
		st->mag.term();
		iio_device_unregister(indio_dev);
		yas_remove_trigger(indio_dev);
		yas_remove_buffer(indio_dev);
		iio_free_device(indio_dev);
		this_client = NULL;
	}
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int yas_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	if (atomic_read(&st->pseudo_irq_enable))
		cancel_delayed_work_sync(&st->work);
	st->mag.set_enable(0);
	return 0;
}

static int yas_resume(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct yas_state *st = iio_priv(indio_dev);
	st->mag.set_enable(1);
	if (atomic_read(&st->pseudo_irq_enable))
		schedule_delayed_work(&st->work, 0);
	return 0;
}

static SIMPLE_DEV_PM_OPS(yas_pm_ops, yas_suspend, yas_resume);
#define YAS_PM_OPS (&yas_pm_ops)
#else
#define YAS_PM_OPS NULL
#endif

static const struct i2c_device_id yas_id[] = {
	{"yas532", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, yas_id);

static struct of_device_id yas533_of_match[] = {
		{ .compatible  = "qcom,yas532",},
		{ .compatible  = "yas532",},
		{ },
};
MODULE_DEVICE_TABLE(of, yas533_of_match);

static struct i2c_driver yas_driver = {
	.driver = {
		.name	= "yas532",
		.owner	= THIS_MODULE,
		.pm	= YAS_PM_OPS,
              .of_match_table = yas533_of_match,
	},
	.probe		= yas_probe,
	.remove		= __devexit_p(yas_remove),
	.id_table	= yas_id,
};
static int __init yas_initialize(void)
{
	return i2c_add_driver(&yas_driver);
}

static void __exit yas_terminate(void)
{
	i2c_del_driver(&yas_driver);
}

module_init(yas_initialize);
module_exit(yas_terminate);

MODULE_DESCRIPTION("Yamaha YAS532 I2C driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("5.1.1000c.c2");

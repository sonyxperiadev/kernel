/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html
 *
 * (C) Copyright 2014 Sony Mobile Communications
 * (C) Copyright 2015 Sony Mobile Communications Inc
 *
 * All Rights Reserved
 */

#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/rtc.h>

struct fw_header_flags {
	u16   no_exec:1;
	u16   reserved1:7;
	u16   i2c_clk_rate:3;
	u16   rom_v_exp:4;
	u16   reserved2:1;
};

struct fw_header {
	u16 magic;
	union {
		u16 flags_value;
		struct fw_header_flags flags;
	};
	u32 text_crc;
	u32 reserved1;
	u16 text_length;
	u16 reserved2;
} __attribute__((__packed__));

struct fw_cds {
	u16 signature;
	u8 res[6];
	u16 ram_ver;
	u8 cds_ver;
	u8 pin_sel[8];
	u8 pull_sel[8];
	u8 dev_name[16];
};

#define EEPROM_MAGIC_VALUE 0x652A
#define RAM_BUF_LEN 64
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define FIFO_SIZE_MAX 184

enum em718x_reg {
	R8_PRODUCT_ID = 0x90,
	R8_REV_ID = 0x91,
	R16_ROM_VERS = 0x70,
	R16_RAM_VERS = 0x72,
	R8_FEATURE_FLAGS = 0x39,
	R16_FW_UPLOAD_ADDR = 0x94,
	R8_FW_UPLOAD_DATA = 0x96,
	R32_HOST_CRC = 0x97,
	R8_HOST_CTL = 0x34,
	R8_EVENTS_ENABLE = 0x33,
	R8_ALGO_CTL = 0x54,
	R8_CENTRAL_STATUS = 0x37,
	R8_EV_STATUS = 0x35,
	RX_SNS_DATA = 0x12,
	RX_FEAT_SNS_DATA = 0x2a,
	RX_QUAT_DATA = 0,
	R8_RESET = 0x9b,
	R8_ERR_REGISTER = 0x50,
	R16_DATA_EXTENTION_REG = 0x4e,
	R32_PARAMETER_READ_REG = 0x3b,
	R32_PARAMETER_LOAD_REG = 0x60,
	R8_PARAMETER_REQ_REG = 0x64,
	R8_PARAMETER_ACK_REG = 0x3a,
	R8_APP_CPU_SATE_REG = 0x69,
	RX_TILT_ACC_REG = 0x3f,
	R8_FIFO_ACK_REG = 0x5b,
	R8_FIFO_SIZE_REG = 0x5c,
	R8_FIFO_SIZE_ACK_REG = 0x4b,
	RX_ACC_DATA = 0x1a,
	R16_CAL_STATUS_REG = 0x4d,
};

enum app_cpu_state {
	APP_CPU_SUSPENDING = 1 << 0,
};

enum em718x_features {
	F_BAROMETER = 1 << 0,
	F_HUMIDITY = 1 << 1,
	F_TEMPERATURE = 1 << 2,
	F_CUSTOM_0 = 1 << 3,
	F_CUSTOM_1 = 1 << 4,
	F_CUSTOM_2 = 1 << 5,
	MAX_FEATURES_NUM = 6,
	MAX_FEATURES_ACTIVE = 3,
};

enum em718x_sensors {
	SNS_QUATERNION,
	SNS_MAG,
	SNS_ACC,
	SNS_GYRO,
	SNS_PHYS_MAX = SNS_GYRO,

	SNS_F0,
	SNS_F1,
	SNS_F2,
	SNS_CNT
};

enum em718x_algo_ctl {
	ALGO_STANDBY = 1 << 0,
	ALGO_RAW_DATA = 1 << 1,
	ALGO_HPR = 1 << 2,
	ALGO_6D_OUT = 1 << 3,
	ALGO_ENU_OUT = 1 << 5,
	ALGO_GYRO_DIS = 1 << 6,
	ALGO_P_TRANSFER = 1 << 7,
};

enum em718x_host_ctl {
	RUN_ENABLE = 1 << 0,
	UPLOAD_ENABLE = 1 << 1,
};

enum em718x_ev_enable {
	EV_RESET = 1 << 0,
	EV_ERROR = 1 << 1,
	EV_QUATERNION = 1 << 2,
	EV_MAG = 1 << 3,
	EV_ACC = 1 << 4,
	EV_GYRO = 1 << 5,
	EV_FEATURE = 1 << 6,
};

enum em718x_sentral_status {
	STATUS_INITIALIZED = 1 << 3,
};

struct em718x_sns_ctl {
	int rate_set_reg;
	int rate_get_reg;
	u8 bit_mask;
	u8 divisor;
};

static const struct em718x_sns_ctl em718x_sns_ctl[] = {
	[SNS_QUATERNION] = { 0x32, 0x32, EV_QUATERNION, 1, },
	[SNS_MAG] = { 0x55, 0x45, EV_MAG, 1, },
	[SNS_ACC] = { 0x56, 0x46, EV_ACC, 10, },
	[SNS_GYRO] = { 0x57, 0x47, EV_GYRO, 10, },
	[SNS_F0] = { 0x58, 0x48, EV_FEATURE, 1, },
	[SNS_F1] = { 0x59, 0x49, EV_FEATURE, 1, },
	[SNS_F2] = { 0x5a, 0x4a, EV_FEATURE, 1, },
};

static const char * const sns_id[] = {
	[SNS_QUATERNION] = "quat",
	[SNS_MAG] = "mag",
	[SNS_ACC] = "acc",
	[SNS_GYRO] = "gyro",
	[SNS_F0] = "feat0",
	[SNS_F1] = "feat1",
	[SNS_F2] = "feat2",
};

enum feature_type {
	F_ANY_MOTION,
	F_TILT,
	F_STEP_CNTR,
};

static const char * const feature_type_id[] = {
	[F_ANY_MOTION] = "amd",
	[F_TILT] = "tilt",
	[F_STEP_CNTR] = "step",
};

struct data_3d {
	union {
		s16 data[3];
		struct {
			s16 x, y, z;
		};
	};
	u16 t;
};

struct data_quat {
	union {
		u32 d[4];
		struct {
			u32 x, y, z, w;
		};
		struct {
			u32 h, p, r, reserved;
		};
	};
	u16 t;
};

struct feature_data {
	u16 d[4];
};

struct em718x_amgf_result {
	struct data_3d mag;
	struct data_3d acc;
	struct data_3d gyro;
	struct feature_data fd;
};

struct em718x_status {
	union {
		u8 s[4];
		union {
			u8 event_status;
			struct {
				u8 reset:1;
				u8 error:1;
				u8 quaternion:1;
				u8 mag:1;
				u8 acc:1;
				u8 gyro:1;
				u8 feature:1;
			} ev_st;
		};
		union {
			u8 sensor_status;
			struct {
				u8 mag_nak:1;
				u8 acc_nak:1;
				u8 gyro_nak:1;
				u8 reserved:1;
				u8 mag_id:1;
				u8 acc_id:1;
				u8 gyro_id:1;
			} sens_st;
		};
		u8 sentral_status;
		union {
			u8 algo_status;
			struct {
				u8 stdby:1;
				u8 slow:1;
				u8 still:1;
				u8 mag_cal_complete:1;
				u8 mag_trans:1;
				u8 data_unrel:1;
			} algo_st;
		};
	};
};

struct feature_desc {
	bool present;
	u8 data_idx;
	enum feature_type type;
	u16 value_last;
	bool last_valid;
};

struct em718x_parameters {
	u32 addr;
	u32 val;
};

enum sensor_type {
	SENSOR_TYPE_ACC,
	SENSOR_TYPE_MAG,
	SENSOR_TYPE_GYRO,
	SENSOR_TYPE_QUAT,
	SENSOR_TYPE_STEP,
	SENSOR_TYPE_AMD,
	SENSOR_TYPE_TILT_WRIST,
	SENSOR_TYPE_STATUS,
	SENSOR_TYPE_CALSCORE,
	SENSOR_TYPE_WAKE_UP = 1 << 4,
	SENSOR_FLUSH_COMPLETE = 1 << 5,
};

struct sensor_event {
	enum sensor_type type;
	int d[4];
	int64_t time;
};

#define ACC_EV_BUF_SUZE (FIFO_SIZE_MAX + FIFO_SIZE_MAX / 10)
#define GYRO_EV_BUF_SUZE 16
#define MAG_EV_BUF_SUZE 16
#define QUAT_EV_BUF_SUZE 16
#define STEP_EV_BUF_SUZE 1
#define TILT_EV_BUF_SUZE 2
#define AMD_EV_BUF_SUZE 2

#define EVENT_Q_SIZE (ACC_EV_BUF_SUZE + GYRO_EV_BUF_SUZE +\
			MAG_EV_BUF_SUZE + QUAT_EV_BUF_SUZE +\
			STEP_EV_BUF_SUZE)

#define WAKE_EVENT_Q_SIZE (TILT_EV_BUF_SUZE + AMD_EV_BUF_SUZE)

struct sensor_event_queue {
	int ridx;
	int widx;
	struct sensor_event *ev;
	int queue_size;
	struct mutex lock;
};

struct event_device {
	char name[32];
	struct miscdevice cdev;
	wait_queue_head_t wq;
	struct sensor_event_queue ev_queue;
	struct em718x *em718x;
};

struct sensor_request {
	u32 rate;
	bool report;
	int fifo_size;
};

struct parameter_transfer {
	u32 buf[127];
	int num;
	int pos;
	bool pending;
};

struct em718x {
	struct i2c_client *client;
	int irq_gpio;
	int irq;
	struct delayed_work reset_work;
	struct mutex lock;
	const char *fw_image_name;
	u8 num_features;
	u8 algo_ctl;
	u8 enabled_sns;
	u8 suspend_enabled_sns;
	struct feature_desc f_desc[MAX_FEATURES_ACTIVE];
	struct firmware fw;
	struct wake_lock w_lock;
	struct notifier_block nb;
	bool suspend_prepared;
	struct em718x_parameters *dts_parameters;
	int dts_parameters_num;
	struct event_device ev_device;
	struct event_device wake_ev_device;
	u8 status_reported;
	s64 irq_time;
	s64 last_samle_t;
	u32 acc_rate_ns;
	u8 mcal_status;
	struct sensor_request sns_req[SNS_CNT];
	struct sensor_request sns_state[SNS_CNT];
	struct mutex ctl_lock;
	struct work_struct sns_ctl_work;
	struct parameter_transfer parameter_r;
	struct parameter_transfer parameter_w;
	wait_queue_head_t control_wq;
	bool control_pending;

};

static int smbus_read_byte(struct i2c_client *client,
				  unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, dummy);
		return dummy;
	}
	*data = dummy & 0x000000ff;
	return 0;
}

static int smbus_write_byte(struct i2c_client *client,
				   unsigned char reg_addr, unsigned char data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, data);
	if (dummy < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, dummy);
		return dummy;
	}
	dev_vdbg(&client->dev, "%s: 0x%02x -> [0x%02x] = %d\n",
			__func__, reg_addr, data, dummy);
	return 0;
}

static int smbus_read_byte_block(struct i2c_client *client,
					unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, dummy);
		return dummy;
	}
	dev_vdbg(&client->dev, "%s: read %u bytes from 0x%02x\n", __func__,
			len, reg_addr);
	return 0;
}

static int smbus_write_byte_block(struct i2c_client *client,
					unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_write_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, dummy);
		return dummy;
	}
	return 0;
}

static int big_block_read(struct i2c_client *cl, u8 addr,
	int len, char *rec_buf)
{
	int rc;
	struct i2c_msg msg[2] = {
		[0] =  {
			.addr = cl->addr,
			.flags = cl->flags & I2C_M_TEN,
			.len = 1,
			.buf = &addr,
		},
		[1] =  {
			.addr = cl->addr,
			.flags = (cl->flags & I2C_M_TEN) | I2C_M_RD,
			.len = len,
			.buf = rec_buf,
		},
	};
	rc = i2c_transfer(cl->adapter, msg, ARRAY_SIZE(msg));
	if (rc == ARRAY_SIZE(msg))
		return 0;
	dev_err(&cl->dev, "%s: rc %d\n", __func__, rc);
	return rc < 0 ? rc : -EIO;
}

static int notify_app_cpu_suspended(struct em718x *em718x, bool suspended)
{
	return smbus_write_byte(em718x->client, R8_APP_CPU_SATE_REG,
			suspended ? APP_CPU_SUSPENDING : 0);
}

static int em718x_standby(struct em718x *em718x, bool standby)
{
	int rc;
	dev_dbg(&em718x->client->dev, "%s: stanby %d\n", __func__, standby);
	if (standby)
		em718x->algo_ctl |= ALGO_STANDBY;
	else
		em718x->algo_ctl &= ~ALGO_STANDBY;

	rc = smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);
	if (rc)
		dev_err(&em718x->client->dev, "failed to set stanby %d\n",
				standby);
	return rc;
}

static int em718x_host_ctl(struct em718x *em718x, u8 ctl)
{
	int rc;
	dev_dbg(&em718x->client->dev, "%s: ctl 0x%02x\n", __func__, ctl);

	rc = smbus_write_byte(em718x->client, R8_HOST_CTL, ctl);
	if (rc)
		dev_err(&em718x->client->dev, "failed to set host ctl 0x%02x\n",
				ctl);
	return rc;
}

static inline bool enabled(struct em718x *em718x, enum em718x_sensors sns)
{
	return !!(em718x->enabled_sns & (1 << sns));
}

static int em718x_update_run_mode(struct em718x *em718x)
{
	int i;
	for (i = SNS_QUATERNION + 1; i < SNS_CNT; i++) {
		if (em718x->sns_state[i].rate)
			return em718x_standby(em718x, false);
	}
	return em718x_standby(em718x, true);
}

static int em718x_update_event_ena_reg(struct em718x *em718x,
	u8 enabled_sensors)
{
	int i;
	u8 rpt_ena_reg = EV_RESET | EV_ERROR;
	const struct em718x_sns_ctl *ctl = em718x_sns_ctl;

	for (i = 0; i < SNS_CNT; i++) {
		if (enabled_sensors & (1 << i)) {
			rpt_ena_reg |= em718x_sns_ctl[i].bit_mask;
			if (i > SNS_GYRO && em718x->sns_state[i].rate) {
				u8 reg = em718x->sns_state[i].rate | (1 << 7);
				smbus_write_byte(em718x->client,
						ctl[i].rate_set_reg, reg);
			}
		} else {
			if (i > SNS_GYRO && em718x->sns_state[i].rate) {
				u8 reg = em718x->sns_state[i].rate & ~(1 << 7);
				smbus_write_byte(em718x->client,
						ctl[i].rate_set_reg, reg);
			}
		}
	}
	return smbus_write_byte(em718x->client, R8_EVENTS_ENABLE, rpt_ena_reg);
}

static int em718x_set_sensor_report(struct em718x *em718x,
		enum em718x_sensors sns, bool enable)
{
	int rc;
	struct device *dev = &em718x->client->dev;
	if (enable) {
		em718x->enabled_sns |= 1 << sns;
		if (sns > SNS_GYRO)
			em718x->f_desc[sns - SNS_GYRO - 1].last_valid = false;
		dev_dbg(dev, "%s: '%s' enable report\n", __func__, sns_id[sns]);
	} else {
		em718x->enabled_sns &= ~(1 << sns);
		dev_dbg(dev, "%s: '%s' disable report\n",
				__func__, sns_id[sns]);
	}
	if (em718x->suspend_prepared)
		return 0;
	rc = em718x_update_event_ena_reg(em718x, em718x->enabled_sns);
	if (!rc)
		em718x->sns_state[sns].report = enable;
	return rc;
}

static unsigned long acc_actual_rate_ns(struct em718x *em718x)
{
	static const struct {
		u32 bw_hz;
		u32 update_t_ns;
	} supported_filter_bw[] = {
		{  8, 64000000},
		{ 16, 32000000},
		{ 32, 16000000},
		{ 63, 8000000},
		{125, 4000000},
		{250, 2000000},
		{500, 1000000},
		{1000, 500000},
	};
	u32 bw = em718x->sns_state[SNS_ACC].rate / 2;
	size_t i;
	for (i = 0; i < ARRAY_SIZE(supported_filter_bw) - 1; i++) {
		if (bw <= supported_filter_bw[i].bw_hz)
			break;
	}
	dev_dbg(&em718x->client->dev, "acc update time %u ns\n",
			supported_filter_bw[i].update_t_ns);
	return supported_filter_bw[i].update_t_ns;
}

static int em718x_set_sensor_rate(struct em718x *em718x,
		enum em718x_sensors sns, u32 rate)
{
	struct device *dev = &em718x->client->dev;
	const struct em718x_sns_ctl *ctl = &em718x_sns_ctl[sns];
	int rc;
	u8 rate_to_appy;

	dev_dbg(dev, "%s: %s, rate %u\n", __func__, sns_id[sns], rate);

	if (rate) {
		rate_to_appy = rate / ctl->divisor;
		if (!rate_to_appy)
			rate_to_appy = 1;
	} else {
		rate_to_appy = 0;
	}
	dev_dbg(dev, "%s: '%s' set rate register: %u\n", __func__, sns_id[sns],
			rate_to_appy);
	rc = smbus_write_byte(em718x->client, ctl->rate_set_reg, rate_to_appy);
	if (!rc) {
		em718x->sns_state[sns].rate = rate;
		if (sns == SNS_ACC)
			em718x->acc_rate_ns = acc_actual_rate_ns(em718x);
		rc = em718x_update_run_mode(em718x);
	}
	return rc;
}

#define PARAM_READ_RETRY_NUM 3
static int em718x_parameters_read(struct em718x *em718x, u32 *buf, int from,
	size_t num)
{
	int rc;
	u8 ack;

	rc = smbus_write_byte(em718x->client, R8_PARAMETER_REQ_REG, from);
	if (rc)
		return rc;
	em718x->algo_ctl |= 1 << 7;
	rc = smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);
	if (rc)
		return rc;
	do {
		int i;
		for (i = 0; i < PARAM_READ_RETRY_NUM; i++) {
			usleep_range(5000, 6000);
			rc = smbus_read_byte(em718x->client,
					R8_PARAMETER_ACK_REG, &ack);
			if (rc)
				goto exit;
			if (ack == from) {
				dev_dbg(&em718x->client->dev, "%s: ack %d\n",
						__func__, i);
				goto acked;
			}
		}
		dev_err(&em718x->client->dev, "%s: no ack\n", __func__);
		goto exit;
acked:
		rc = smbus_read_byte_block(em718x->client,
				R32_PARAMETER_READ_REG, (u8 *)buf,
				sizeof(*buf));
		if (rc)
			goto exit;
		buf++;
		rc = smbus_write_byte(em718x->client,
				R8_PARAMETER_REQ_REG, ++from);
		if (rc)
			goto exit;
	} while (--num);
exit:
	em718x->algo_ctl &= ~(1 << 7);
	(void)smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);
	(void)smbus_write_byte(em718x->client, R8_PARAMETER_REQ_REG, 0);
	return rc;
}

#define PARAM_LOAD_RETRY_NUM 10
static int em718x_parameter_load(struct em718x *em718x, u32 val, int addr)
{
	int rc;
	u8 ack;
	int i;

	rc = smbus_write_byte_block(em718x->client, R32_PARAMETER_LOAD_REG,
			(u8 *)&val, sizeof(val));
	if (rc)
		return rc;

	addr |= 1 << 7;
	rc = smbus_write_byte(em718x->client, R8_PARAMETER_REQ_REG, addr);
	if (rc)
		return rc;

	em718x->algo_ctl |= 1 << 7;
	rc = smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);
	if (rc)
		return rc;

	for (i = 0; i < PARAM_LOAD_RETRY_NUM; i++) {
		usleep_range(10000, 11000);
		rc = smbus_read_byte(em718x->client,
				R8_PARAMETER_ACK_REG, &ack);
		if (rc)
			goto exit;

		if (ack == addr) {
			dev_dbg(&em718x->client->dev,
				"%s: 0x%08x -> 0x%02x ack 0x%02x\n", __func__,
				val, addr & 0x7f, ack);
			goto exit;
		}
	}
	dev_err(&em718x->client->dev, "%s: 0x%08x -> 0x%02x NACK 0x%02x\n",
			__func__, val, addr & 0x7f, ack);
	rc = -EIO;
exit:
	em718x->algo_ctl &= ~(1 << 7);
	(void)smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);
	(void)smbus_write_byte(em718x->client, R8_PARAMETER_REQ_REG, 0);
	return rc;
}

static void exec_parameters_transfer(struct em718x *em718x)
{
	if (em718x->parameter_r.pending) {
		em718x->parameter_r.num = em718x_parameters_read(em718x,
				em718x->parameter_r.buf,
				em718x->parameter_r.pos,
				em718x->parameter_r.num);
		em718x->parameter_r.pending = false;
	}
	if (em718x->parameter_w.pending) {
		int rc;
		int i;
		for (i = 0; i < em718x->parameter_w.num; i++) {
			rc = em718x_parameter_load(em718x,
					em718x->parameter_w.buf[i],
					em718x->parameter_w.pos + i);
			if (rc)
				break;
		}
		em718x->parameter_w.num = rc;
		em718x->parameter_w.pending = false;
	}
}

static void post_control_work_locked(struct em718x *em718x)
{
	em718x->control_pending = true;
	schedule_work(&em718x->sns_ctl_work);
}

static ssize_t parameters_bin_read(struct file *data_file,
		struct kobject *kobj, struct bin_attribute *attributes,
		char *buf, loff_t pos, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	struct em718x *em718x = dev_get_drvdata(&client->dev);

	if (pos + count >= sizeof(em718x->parameter_r.buf))
		return -EINVAL;

	mutex_lock(&em718x->ctl_lock);
	em718x->parameter_r.pos = pos;
	em718x->parameter_r.num = count / sizeof(u32);
	em718x->parameter_r.pending = true;
	post_control_work_locked(em718x);
	mutex_unlock(&em718x->ctl_lock);

	wait_event_interruptible(em718x->control_wq,
			!em718x->control_pending);

	if (em718x->parameter_r.num > 0)
		memcpy(buf, em718x->parameter_r.buf, count);
	return em718x->parameter_r.num < 0 ? em718x->parameter_r.num : count;
}

static ssize_t parameters_bin_write(struct file *data_file,
		struct kobject *kobj, struct bin_attribute *attributes,
		char *buf, loff_t pos, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	struct em718x *em718x = dev_get_drvdata(&client->dev);

	if (pos + count >= sizeof(em718x->parameter_w.buf))
		return -EINVAL;

	mutex_lock(&em718x->ctl_lock);
	em718x->parameter_w.pos = pos;
	em718x->parameter_w.num = count / sizeof(u32);
	em718x->parameter_w.pending = true;
	memcpy(em718x->parameter_w.buf, buf, count);
	post_control_work_locked(em718x);
	mutex_unlock(&em718x->ctl_lock);

	wait_event_interruptible(em718x->control_wq,
			!em718x->control_pending);

	return em718x->parameter_w.num < 0 ? em718x->parameter_w.num : count;
}

static struct bin_attribute parameter_bin_data = {
	.attr = {
		.name = "parameters",
		.mode = S_IRUGO | S_IWUSR,
	},
	.size = 127,
	.read = parameters_bin_read,
	.write = parameters_bin_write,
};

static ssize_t em718x_set_register(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	char op;
	s32 addr;
	s32 value;
	int rc;
	if (3 != sscanf(buf, "%c,%i,%i", &op, &addr, &value))
		return -EINVAL;
	if (op == 'r' && value && value <= 32) {
		u8 buf[32];
		char str[sizeof(buf) * 3 + 1];
		char *p;
		int i;
		rc = smbus_read_byte_block(em718x->client,
				addr, buf, value);
		if (rc)
			return rc;
		p += scnprintf(p, sizeof(str) - (unsigned)(p - str),
				"read %d bytes from %02x: ", value, addr);
		for (i = 0, p = str; i < value; i++)
			p += scnprintf(p, sizeof(str) - (unsigned)(p - str),
				"%02x ", buf[i]);
		dev_info(dev, "%s\n", str);
	} else if (op == 'w') {
		rc = smbus_write_byte(em718x->client, addr, value);
	} else {
		rc = -EINVAL;
	}
	return rc ? rc : count;
}

static DEVICE_ATTR(reg, S_IWUSR, NULL, em718x_set_register);

static ssize_t em718x_get_rate(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	int rate[SNS_CNT];
	int rate_req[SNS_CNT];
	size_t i;
	int rc;
	u8 rate_reg;
	char *p = buf;

	for (i = 0; i < SNS_CNT; i++) {
		rc = smbus_read_byte_block(em718x->client,
				em718x_sns_ctl[i].rate_get_reg,
				&rate_reg, sizeof(rate_reg));
		if (rc)
			rate[i] = -1;
		else
			rate[i] = (unsigned)rate_reg *
					em718x_sns_ctl[i].divisor;

		rc = smbus_read_byte_block(em718x->client,
				em718x_sns_ctl[i].rate_set_reg,
				&rate_reg, sizeof(rate_reg));
		if (rc)
			rate_req[i] = -1;
		else
			rate_req[i] = (unsigned)rate_reg *
					em718x_sns_ctl[i].divisor;
	}
	for (i = 0; i < SNS_CNT; i++)
		p += scnprintf(p, PAGE_SIZE - (p - buf), "%s %d (%d), ",
				sns_id[i], rate[i], rate_req[i]);

	p += scnprintf(p, PAGE_SIZE - (p - buf), "\n");
	return (ssize_t)(p - buf);
}

static ssize_t em718x_set_rate(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	char id[32];
	u32 rate;
	size_t i;

	if (2 != sscanf(buf, "%32s %u", id, &rate))
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(sns_id); i++) {
		if (!strcmp(id, sns_id[i])) {
			mutex_lock(&em718x->ctl_lock);
			if (em718x->sns_state[i].rate != rate) {
				em718x->sns_req[i].rate = rate;
				post_control_work_locked(em718x);
				mutex_unlock(&em718x->ctl_lock);
				wait_event_interruptible(em718x->control_wq,
						!em718x->control_pending);
				goto exit;
			}
			mutex_unlock(&em718x->ctl_lock);
exit:
			return count;
		}
	}
	return -EINVAL;
}

static DEVICE_ATTR(rate, S_IRUGO | S_IWUSR,
		em718x_get_rate, em718x_set_rate);


static ssize_t em718x_get_report_enable(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	size_t i;
	char *p = buf;

	for (i = 0; i < SNS_CNT; i++) {
		p += scnprintf(p, PAGE_SIZE - (p - buf), "%s %d, ",
				sns_id[i], enabled(em718x, i));
	}
	p += scnprintf(p, PAGE_SIZE - (p - buf), "\n");
	return (ssize_t)(p - buf);
}

static ssize_t em718x_set_report_enable(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	char id[32];
	u32 enable;
	size_t i;

	if (2 != sscanf(buf, "%32s %u", id, &enable))
		return -EINVAL;

	enable = !!enable;
	for (i = 0; i < ARRAY_SIZE(sns_id); i++) {
		if (!strcmp(id, sns_id[i])) {
			mutex_lock(&em718x->ctl_lock);
			if (em718x->sns_state[i].report != enable) {
				em718x->sns_req[i].report = enable;
				post_control_work_locked(em718x);
				mutex_unlock(&em718x->ctl_lock);
				wait_event_interruptible(em718x->control_wq,
						!em718x->control_pending);
				goto exit;
			}
			mutex_unlock(&em718x->ctl_lock);
exit:
			return count;
		}
	}
	return -EINVAL;
}

static DEVICE_ATTR(report, S_IRUGO | S_IWUSR,
		em718x_get_report_enable, em718x_set_report_enable);

static ssize_t em718x_set_reset(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	int rc;
	unsigned long reset;
	rc = kstrtoul(buf, 0, &reset);
	if (!rc)
		rc = smbus_write_byte(em718x->client, R8_RESET, 0x01);
	return rc ? rc : count;
}

static DEVICE_ATTR(reset, S_IWUSR, NULL, em718x_set_reset);

static int em718x_fifo_size_set(struct em718x *em718x, int fifo_size)
{
	int i;
	unsigned char ack = ~fifo_size;
	dev_info(&em718x->client->dev, "%s: %d\n", __func__, fifo_size);

	em718x->algo_ctl |= 1 << 7;
	(void)smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);
	msleep(20);
	for (i = 0; i < 5; i++) {
		int rc;

		usleep_range(2000, 3000);
		(void)smbus_write_byte(em718x->client,
				R8_FIFO_SIZE_REG, fifo_size);
		rc = smbus_read_byte(em718x->client,
				R8_FIFO_SIZE_ACK_REG, &ack);
		if (!rc && ack == fifo_size)
			break;
	}
	em718x->algo_ctl &= ~(1 << 7);
	(void)smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);

	if (ack != fifo_size) {
		dev_err(&em718x->client->dev, "%s: failed: no ack\n", __func__);
		return -EIO;
	}
	em718x->sns_state[SNS_ACC].fifo_size = fifo_size;
	return 0;
}

static void sensor_control_work(struct work_struct *work)
{
	struct em718x *em718x = container_of(work, struct em718x, sns_ctl_work);
	size_t i;

	mutex_lock(&em718x->lock);
	mutex_lock(&em718x->ctl_lock);
	for (i = 0; i < ARRAY_SIZE(em718x->sns_req); i++) {
		struct sensor_request *rq = &em718x->sns_req[i];
		struct sensor_request *state = &em718x->sns_state[i];

		if (state->rate != rq->rate)
			(void)em718x_set_sensor_rate(em718x, i, rq->rate);

		if (i == SNS_ACC && (rq->fifo_size || state->fifo_size))
			(void)em718x_fifo_size_set(em718x, rq->fifo_size);

		if (state->report != rq->report)
			(void)em718x_set_sensor_report(em718x, i, rq->report);
	}
	exec_parameters_transfer(em718x);
	em718x->control_pending = false;
	wake_up_interruptible(&em718x->control_wq);
	mutex_unlock(&em718x->ctl_lock);
	mutex_unlock(&em718x->lock);
}

static ssize_t em718x_get_fifo_size(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d",
			em718x->sns_state[SNS_ACC].fifo_size);
}

static ssize_t em718x_set_fifo_size(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	int rc;
	unsigned long fifo_size;
	rc = kstrtoul(buf, 0, &fifo_size);

	if (rc)
		return rc;
	if (fifo_size > FIFO_SIZE_MAX)
		return -EINVAL;

	mutex_lock(&em718x->ctl_lock);
	if (em718x->sns_state[SNS_ACC].fifo_size || fifo_size) {
		em718x->sns_req[SNS_ACC].fifo_size = fifo_size;
		post_control_work_locked(em718x);
		mutex_unlock(&em718x->ctl_lock);
		wait_event_interruptible(em718x->control_wq,
				!em718x->control_pending);
		goto exit;
	}
	mutex_unlock(&em718x->ctl_lock);
exit:
	return count;
}

static DEVICE_ATTR(fifo_size, S_IRUGO | S_IWUSR,
		em718x_get_fifo_size, em718x_set_fifo_size);

static ssize_t em718x_get_fifo_max_size(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d", FIFO_SIZE_MAX);
}


static DEVICE_ATTR(fifo_size_max, S_IRUGO, em718x_get_fifo_max_size, NULL);

static ssize_t em718x_get_wuff(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d",
			!!(em718x->suspend_enabled_sns & (1 << SNS_ACC)));
}

static ssize_t em718x_set_wuff(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	int rc;
	unsigned long wuff;
	rc = kstrtoul(buf, 0, &wuff);

	if (rc)
		return rc;
	if (wuff > 1)
		return -EINVAL;
	mutex_lock(&em718x->lock);
	if (wuff)
		em718x->suspend_enabled_sns |= 1 << SNS_ACC;
	else
		em718x->suspend_enabled_sns &= ~(1 << SNS_ACC);
	mutex_unlock(&em718x->lock);
	return count;
}

static DEVICE_ATTR(wuff, S_IRUGO | S_IWUSR,
		em718x_get_wuff, em718x_set_wuff);


static ssize_t em718x_get_acc_rate_ns(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u", em718x->acc_rate_ns);
}

static DEVICE_ATTR(acc_rate_ns, S_IRUGO, em718x_get_acc_rate_ns, NULL);

static struct attribute *em718x_attributes[] = {
	&dev_attr_reg.attr,
	&dev_attr_rate.attr,
	&dev_attr_report.attr,
	&dev_attr_reset.attr,
	&dev_attr_fifo_size.attr,
	&dev_attr_fifo_size_max.attr,
	&dev_attr_wuff.attr,
	&dev_attr_acc_rate_ns.attr,
	NULL
};

static const struct attribute_group em718x_attribute_group = {
	.attrs = em718x_attributes
};

static int step_cntr_get_32(struct em718x *em718x, int *ext_steps, int idx)
{
	int i;
	for (i = 0; i < 3; i++) {
		u16 ext, temp;
		struct feature_data fd;
		int rc = smbus_read_byte_block(em718x->client,
				R16_DATA_EXTENTION_REG,
				(void *)&ext, sizeof(ext));
		if (rc)
			break;
		rc = smbus_read_byte_block(em718x->client,
				RX_FEAT_SNS_DATA, (void *)&fd, sizeof(fd));
		if (rc)
			break;
		rc = smbus_read_byte_block(em718x->client,
				R16_DATA_EXTENTION_REG,
				(void *)&temp, sizeof(temp));
		if (rc)
			break;
		if (temp == ext) {
			*ext_steps = (ext << 16) + fd.d[idx];
			return 0;
		}
	}
	dev_err(&em718x->client->dev, "unable to read steps extension\n");
	return -EINVAL;
}

static void em718x_queue_event(struct em718x *em718x, struct sensor_event *ev)
{
	struct event_device *ev_dev = &em718x->ev_device;
	struct sensor_event_queue *p = &ev_dev->ev_queue;
	int widx;

	mutex_lock(&p->lock);
	widx = (p->widx + 1) % p->queue_size;

	if (p->ridx == widx) {
		dev_warn(&em718x->client->dev, "fifo overrun\n");
		p->ridx = (p->ridx + 1) % p->queue_size;
	}
	p->ev[p->widx] = *ev;
	p->widx = widx;
	mutex_unlock(&p->lock);
	wake_up_interruptible(&ev_dev->wq);
}

static void em718x_queue_event_wake(struct em718x *em718x,
	struct sensor_event *ev)
{
	struct event_device *ev_dev = &em718x->wake_ev_device;
	struct sensor_event_queue *p = &ev_dev->ev_queue;
	int widx;

	mutex_lock(&p->lock);
	widx = (p->widx + 1) % p->queue_size;

	if (p->ridx == widx) {
		dev_warn(&em718x->client->dev, "fifo overrun\n");
		p->ridx = (p->ridx + 1) % p->queue_size;
	}
	p->ev[p->widx] = *ev;
	p->widx = widx;
	if (!wake_lock_active(&em718x->w_lock))
		wake_lock(&em718x->w_lock);
	mutex_unlock(&p->lock);
	wake_up_interruptible(&ev_dev->wq);
}

static void drain_fifo(struct em718x *em718x)
{
	struct device *dev = &em718x->client->dev;
	bool first;
	int samples;
	s64 irq_t;
	u16 samples_left;
	u16 sample_seq_no;
	struct sensor_event ev;
	s64 ns = em718x->acc_rate_ns;
	int ev_type;

	dev_dbg(dev, "%s: enter\n", __func__);

	irq_t = em718x->irq_time;
	em718x->algo_ctl |= 1 << 7;
	ev_type = SENSOR_TYPE_ACC |
		(!!(em718x->suspend_enabled_sns & (1 << SNS_ACC)) ?
		SENSOR_TYPE_WAKE_UP : 0);
	(void)smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);

	for (first = true, samples = 0;;) {
		struct data_3d acc;

		if (big_block_read(em718x->client, RX_ACC_DATA,
					sizeof(acc), (u8 *)&acc))
			break;

		if (!acc.t || !enabled(em718x, SNS_ACC) ||
					!em718x->sns_state[SNS_ACC].fifo_size) {
			if (samples) {
				ev.type = SENSOR_TYPE_ACC |
						SENSOR_FLUSH_COMPLETE;
				em718x_queue_event(em718x, &ev);
				dev_dbg(dev,
					"acc flush complete, %d samples\n",
					samples);
			}
			break;
		}

		if (first || (sample_seq_no != (acc.t & 0xff))) {

			sample_seq_no = acc.t & 0xff;
			samples_left = acc.t >> 8;

			if (!samples_left) {
				dev_warn(dev, "%s: samples_left=0", __func__);
				goto skip;
			}

			if (first) {
				first = false;
				irq_t = irq_t - (samples_left - 1) * ns;
				if (irq_t < em718x->last_samle_t + ns) {

					long ns8 = (em718x->irq_time -
						em718x->last_samle_t) >> 8;
					ns8 = ns8 / samples_left;
					ns = (s64)ns8 << 8;
					irq_t = ns + em718x->last_samle_t;
				}
			}

			em718x->last_samle_t = irq_t;

			dev_vdbg(dev, "acc[%u, %u, %lld] %d, %d, %d\n",
					sample_seq_no, samples_left, irq_t,
					acc.x, acc.y, -acc.z);
			irq_t += ns;
			samples++;

			ev.type = ev_type;
			ev.time = irq_t;
			ev.d[0] = acc.x;
			ev.d[1] = acc.y;
			ev.d[2] = -acc.z;
			em718x_queue_event(em718x, &ev);
		}
skip:
		smbus_write_byte(em718x->client, R8_FIFO_ACK_REG, 0);
	}
	em718x->algo_ctl &= ~(1 << 7);
	(void)smbus_write_byte(em718x->client, R8_ALGO_CTL, em718x->algo_ctl);
}

static void em718x_process_amgf(struct em718x *em718x,
	struct em718x_amgf_result *amgf, u8 ev_status)
{
	struct device *dev = &em718x->client->dev;
	struct sensor_event ev;

	ev.time = em718x->irq_time;

	if ((ev_status & EV_ACC) && enabled(em718x, SNS_ACC)) {
		if (em718x->sns_state[SNS_ACC].fifo_size) {
			drain_fifo(em718x);
		} else {
			dev_vdbg(dev, "acc[%u] %d, %d, %d\n", amgf->acc.t,
					amgf->acc.x, amgf->acc.y, -amgf->acc.z);
			ev.type = SENSOR_TYPE_ACC;
			ev.d[0] = amgf->acc.x;
			ev.d[1] = amgf->acc.y;
			ev.d[2] = -amgf->acc.z;
			em718x_queue_event(em718x, &ev);
		}
	}
	if (ev_status & EV_MAG) {
		int rc;
		u8 cal_status;

		rc = smbus_read_byte_block(em718x->client, R16_CAL_STATUS_REG,
				(void *)&cal_status, sizeof(cal_status));

		if (!rc && (cal_status != em718x->mcal_status)) {
			dev_vdbg(dev, "Calibration status/score %u / %u\n",
					cal_status >> 5, cal_status & 0x1f);
			em718x->mcal_status = cal_status;
			ev.type = SENSOR_TYPE_CALSCORE;
			ev.d[0] = cal_status >> 5;
			ev.d[1] = cal_status & 0x1f;
			em718x_queue_event(em718x, &ev);
		}

		if (enabled(em718x, SNS_MAG)) {

			dev_vdbg(dev, "mag[%u] %d, %d, %d\n",
					amgf->mag.t, amgf->mag.x,
					amgf->mag.y, -amgf->mag.z);

			ev.type = SENSOR_TYPE_MAG;
			ev.d[0] = amgf->mag.x;
			ev.d[1] = amgf->mag.y;
			ev.d[2] = -amgf->mag.z;
			em718x_queue_event(em718x, &ev);
		}

	}
	if ((ev_status & EV_GYRO) && enabled(em718x, SNS_GYRO)) {
		dev_vdbg(dev, "gyro[%u] %d, %d, %d\n", amgf->gyro.t,
				-amgf->gyro.x, -amgf->gyro.y, amgf->gyro.z);
		ev.type = SENSOR_TYPE_GYRO;
		ev.d[0] = -amgf->gyro.x;
		ev.d[1] = -amgf->gyro.y;
		ev.d[2] = amgf->gyro.z;
		em718x_queue_event(em718x, &ev);
	}
	if (ev_status & EV_FEATURE) {
		int i;
		int idx;
		int val;

		for (i = 0; i < ARRAY_SIZE(em718x->f_desc); i++) {
			if (!enabled(em718x, SNS_F0 + i))
				continue;
			if (!em718x->f_desc[i].present)
				continue;

			idx = em718x->f_desc[i].data_idx;
			val = amgf->fd.d[idx];

			if (em718x->f_desc[i].value_last == val &&
					em718x->f_desc[i].last_valid)
				continue;

			em718x->f_desc[i].value_last = val;
			em718x->f_desc[i].last_valid = true;

			switch (em718x->f_desc[i].type) {
			case F_STEP_CNTR:
				if (step_cntr_get_32(em718x, &val, idx))
					break;
				dev_dbg(dev, "feature-%d: step %d\n", i, val);
				if (val == 0) {
					dev_dbg(dev, "Skip it\n");
					break;
				}
				ev.type = SENSOR_TYPE_STEP;
				ev.d[0] = val;
				em718x_queue_event(em718x, &ev);
				break;
			case F_TILT:
				dev_dbg(dev, "feature-%d: tilt (%d)\n",
						i, val);
				if (val == 0) {
					dev_dbg(dev, "Skip it\n");
					break;
				}
				ev.type = SENSOR_TYPE_TILT_WRIST |
						SENSOR_TYPE_WAKE_UP;
				ev.d[0] = 1;
				em718x_queue_event_wake(em718x, &ev);
				break;
			case F_ANY_MOTION:
				if (!(val & ~1)) {
					dev_info(dev, "Skip amd %d\n", val);
					break;
				}
				dev_info(dev, "feature-%d: amd %d (%d)\n",
					i, !(val & 1), val >> 1);
				val &= 1;
				ev.type = SENSOR_TYPE_AMD |
						SENSOR_TYPE_WAKE_UP;
				ev.d[0] = !val;
				em718x_queue_event_wake(em718x, &ev);
				break;
			default:
				dev_dbg(dev, "feature-%d: ?? value %d\n", i,
						val);
				break;
			}
		}
	}
}

static void em718x_step_counter_sync_locked(struct em718x *em718x)
{
	int rc;
	int i;
	struct em718x_amgf_result amgf;
	struct device *dev = &em718x->client->dev;
	struct sensor_event ev;

	rc = big_block_read(em718x->client, RX_SNS_DATA,
				sizeof(amgf), (u8 *)&amgf);
	if (rc)
		return;
	for (i = 0; i < ARRAY_SIZE(em718x->f_desc); i++) {
		int idx;
		int val;

		if (em718x->f_desc[i].type != F_STEP_CNTR ||
				!enabled(em718x, SNS_F0 + i) ||
				!em718x->f_desc[i].present)
			continue;

		idx = em718x->f_desc[i].data_idx;
		val = amgf.fd.d[idx];

		dev_dbg(dev, "feature-%d on resume: step %d\n", i, val);

		if (em718x->f_desc[i].value_last == val &&
				em718x->f_desc[i].last_valid)
			break;

		em718x->f_desc[i].value_last = val;
		em718x->f_desc[i].last_valid = true;

		if (step_cntr_get_32(em718x, &val, idx))
			break;

		dev_dbg(dev, "report steps on resume %d steps\n", val);

		if (val == 0) {
			dev_dbg(dev, "Skip it\n");
			break;
		}
		ev.time =  ktime_to_ns(ktime_get_boottime());
		ev.type = SENSOR_TYPE_STEP;
		ev.d[0] = val;
		em718x_queue_event(em718x, &ev);
	}
}

static void em718x_process_q(struct em718x *em718x,
	struct data_quat *quat)
{
	struct device *dev = &em718x->client->dev;

	if (enabled(em718x, SNS_QUATERNION)) {
		struct sensor_event ev;

		ev.time =  em718x->irq_time;

		dev_vdbg(dev, "quaternion[%u]: %u, %u, %u, %u\n",
				quat->t, quat->d[0],
				quat->d[1], quat->d[2], quat->d[3]);
		ev.type = SENSOR_TYPE_QUAT;
		ev.d[0] = quat->d[0];
		ev.d[1] = quat->d[1];
		ev.d[2] = quat->d[2];
		ev.d[3] = quat->d[3];
		em718x_queue_event(em718x, &ev);
	}
}

static irqreturn_t em718x_irq_handler(int irq, void *handle)
{
	struct em718x *em718x = handle;
	struct device *dev = &em718x->client->dev;
	int rc;
	struct em718x_status status;

	mutex_lock(&em718x->lock);

	em718x->irq_time = ktime_to_ns(ktime_get_boottime());

	rc = smbus_read_byte_block(em718x->client, R8_EV_STATUS,
			(u8 *)&status, sizeof(status));
	if (rc)
		goto exit;
	dev_vdbg(dev, "status 0x%02x%02x%02x%02x\n", status.s[0],
			status.s[1], status.s[2], status.s[3]);

	if (status.ev_st.reset) {
		dev_info(dev, "reset occured\n");
		schedule_delayed_work(&em718x->reset_work,
				msecs_to_jiffies(50));
		goto exit;
	}
	if (status.ev_st.error) {
		u8 err;

		dev_err(dev, "FW error occured:\n");
		smbus_read_byte(em718x->client, R8_ERR_REGISTER, &err);
		dev_info(dev, "sensor status 0x%02x, err 0x%02x\n",
				status.s[1], err);
		(void)smbus_write_byte(em718x->client, R8_RESET, 0x01);
		goto exit;
	}

	if (em718x->status_reported != status.s[3]) {
		struct sensor_event ev;

		ev.time =  em718x->irq_time;
		ev.type = SENSOR_TYPE_STATUS;
		ev.d[0] = status.s[3];
		em718x_queue_event(em718x, &ev);
		dev_dbg(dev, "sensor status updated 0x%02x\n", status.s[3]);
		em718x->status_reported = status.s[3];
	}

	if (status.ev_st.quaternion) {
		struct data_quat quat;

		rc = big_block_read(em718x->client, RX_QUAT_DATA,
				sizeof(quat), (u8 *)&quat);
		if (!rc)
			em718x_process_q(em718x, &quat);
	}
	if (status.event_status & (EV_ACC | EV_MAG | EV_GYRO | EV_FEATURE)) {
		struct em718x_amgf_result amgf;

		rc = big_block_read(em718x->client, RX_SNS_DATA,
				sizeof(amgf), (u8 *)&amgf);
		if (!rc)
			em718x_process_amgf(em718x, &amgf, status.event_status);
	}
exit:
	mutex_unlock(&em718x->lock);
	flush_work(&em718x->sns_ctl_work);
	return IRQ_HANDLED;
}

static int get_feature_type(const char *id)
{
	size_t i;
	for (i = 0; i < ARRAY_SIZE(feature_type_id); i++) {
		if (!strcmp(id , feature_type_id[i]))
			return i;
	}
	return -1;
}

static int feature_data_index(int num_f, int f_idx)
{
	if (num_f > MAX_FEATURES_ACTIVE || f_idx >= num_f)
		return -EINVAL;
	switch (num_f) {
	case 0:
	case 1:
	default:
		return 0;
	case 2:
		return f_idx == 0 ? 0 : 2;
	case 3:
		return f_idx;
	}
}

static int em718x_parse_features(struct em718x *em718x)
{
	u8 features;
	int i;
	struct device *dev = &em718x->client->dev;
	struct device_node *np = dev->of_node;
	int rc = smbus_read_byte(em718x->client, R8_FEATURE_FLAGS, &features);

	dev_dbg(dev, "feature flags 0x%02x\n", features);
	if (rc)
		return rc;
	for (em718x->num_features = 0, i = 0; i < MAX_FEATURES_NUM &&
			em718x->num_features <= MAX_FEATURES_ACTIVE; i++) {
		if (features & (1 << i))
			em718x->num_features++;
	}
	dev_dbg(&em718x->client->dev, "%d features supported\n",
			em718x->num_features);

	if (!np)
		return -EINVAL;

	em718x->suspend_enabled_sns &= ~((1 << SNS_F0) | (1 << SNS_F1) |
			(1 << SNS_F2));

	for (i = 0; i < ARRAY_SIZE(em718x->f_desc); i++) {
		char name[32];
		const char *s;
		int ft;
		u32 flag;

		em718x->f_desc[i].present = false;

		snprintf(name, sizeof(name), "feat%d-type", i);
		if (of_property_read_string(np, name, &s))
			continue;

		snprintf(name, sizeof(name), "feat%d-flag", i);
		if (of_property_read_u32(np, name, &flag))
			continue;

		if (!(features & (1 << flag))) {
			dev_info(&em718x->client->dev,
				"feature %d '%s' not supported in FW\n", i, s);
			continue;
		}

		em718x->f_desc[i].present = true;

		ft = get_feature_type(s);
		if (ft < 0)
			continue;
		em718x->f_desc[i].type = ft;
		em718x->f_desc[i].data_idx =
			feature_data_index(em718x->num_features, i);
		em718x->f_desc[i].present =
				em718x->f_desc[i].data_idx >= 0;

		if (em718x->f_desc[i].present) {
			switch (ft) {
			case F_ANY_MOTION:
			case F_TILT:
				em718x->suspend_enabled_sns |=
						1 << (i + SNS_F0);
				break;
			default:
				break;
			}
		}
		dev_info(dev, "feature %d '%s' present %d\n",
			i, feature_type_id[ft], em718x->f_desc[i].present);
	}
	return 0;
}

static int em718x_check_id(struct em718x *em718x)
{
	u8 pr_id, rev_id;
	int rc = smbus_read_byte(em718x->client, R8_PRODUCT_ID, &pr_id);
	if (rc)
		goto err;
	rc = smbus_read_byte(em718x->client, R8_REV_ID, &rev_id);
	if (rc)
		goto err;
	dev_info(&em718x->client->dev, "Product ID %02x, revision ID 0x%02x\n",
			pr_id, rev_id);
	return 0;
err:
	dev_err(&em718x->client->dev, "%s: rc %d\n", __func__, rc);
	return rc;
}

static ssize_t em718x_upload_firmware(struct file *file,
		struct kobject *kobj,
		struct bin_attribute *bin_attr,
		char *buf, loff_t pos, size_t size)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct em718x *em718x = dev_get_drvdata(dev);

	dev_dbg(dev, "%s: pos %lld, size %d\n", __func__,  pos, size);
	if (!pos) {
		struct fw_header *header = (struct fw_header *)buf;

		kfree(em718x->fw.data);
		em718x->fw.size = sizeof(*header) + header->text_length;
		dev_info(dev, "%s: fw size %d\n", __func__, em718x->fw.size);
		em718x->fw.data = kmalloc(em718x->fw.size, GFP_KERNEL);
		if (!em718x->fw.data) {
			dev_err(dev, "%s: no memory (size %d)\n", __func__,
					em718x->fw.size);
			return -ENOMEM;
		}
	}
	if (pos + size > em718x->fw.size) {
		dev_err(dev, "%s: bad range: pos %lld, size %d, fw size %d\n",
				__func__, pos, size, em718x->fw.size);
		kfree(em718x->fw.data);
		return -ENOMEM;
	}
	memcpy((void *)em718x->fw.data + pos, buf, size);
	if (pos + size == em718x->fw.size)
		dev_info(dev, "%s: upload finished\n", __func__);
	return size;
}

static struct bin_attribute em718x_firmware = {
	.attr = {
		.name = "firmware",
		.mode = S_IWUSR,
	},
	.size = 30 * 1024,
	.write = em718x_upload_firmware,
};

static int em718x_load_firmware(struct em718x *em718x)
{
	const struct firmware *fw;
	int rc;
	struct fw_header *header;
	struct fw_cds *cds;
	u32 *data;
	u32 size;
	struct device *dev = &em718x->client->dev;
	u16 addr;
	u32 crc;

	if (!em718x->fw.data) {
		rc = request_firmware(&fw, em718x->fw_image_name, dev);
		if (rc) {
			dev_err(dev, "failed to find firmware file %s: %d\n",
					em718x->fw_image_name, rc);
			goto exit;
		}
		dev_info(dev, "found firmware image '%s', size: %d\n",
				em718x->fw_image_name, fw->size);
	} else {
		fw = &em718x->fw;
		dev_info(dev, "firmware image size: %d\n", fw->size);
	};


	if (fw->size < sizeof(*header)) {
		dev_err(dev, "not large enough for firmware\n");
		rc = -ENODEV;
		goto exit_release;
	}
	header = (struct fw_header *)fw->data;
	if (header->magic != EEPROM_MAGIC_VALUE) {
		dev_err(dev, "invalid firmware magic: %x\n", header->magic);
		rc = -ENODEV;
		goto exit_release;
	}
	if ((sizeof(*header) + header->text_length) > fw->size) {
		dev_err(dev, "firmware too large (%u)\n", header->text_length);
		rc = -ENODEV;
		goto exit_release;
	}
	cds = (struct fw_cds *)(fw->data + sizeof(*header) +
			header->text_length - sizeof(*cds));

	dev_info(dev, "ram 0x%04x, cds 0x%02x\n", cds->ram_ver, cds->cds_ver);

	addr = 0;
	rc = smbus_write_byte_block(em718x->client, R16_FW_UPLOAD_ADDR,
			(u8 *)&addr, sizeof(addr));
	if (rc)
		goto exit_release;

	rc = em718x_host_ctl(em718x, UPLOAD_ENABLE);
	if (rc)
		goto exit_release;

	data = (u32 *)(((u8 *)fw->data) + sizeof(*header));
	size = fw->size - sizeof(*header);
	dev_info(dev, "writing firmware data, size: %d", size);
	while (size) {
		u32 buf[MIN(RAM_BUF_LEN, I2C_SMBUS_BLOCK_MAX) / 4];
		size_t wsize;
		int i;

		wsize = MIN(size, sizeof(buf));
		for (i = 0; i < wsize / sizeof(u32); i++)
			buf[i] = swab32(*data++);
		rc = smbus_write_byte_block(em718x->client, R8_FW_UPLOAD_DATA,
				(u8 *)buf, wsize);
		if (rc)
			goto exit_release;
		size -= wsize;
		dev_vdbg(dev, "%u bytes sent, %u remaining\n", wsize, size);
	}
	dev_dbg(dev, "fw data uploaded\n");

	(void)em718x_host_ctl(em718x, 0);
	rc = smbus_read_byte_block(em718x->client, R32_HOST_CRC, (u8 *)&crc,
			sizeof(crc));
	if (!rc) {
		rc = crc == header->text_crc ? 0 : -EINVAL;
		dev_info(dev, "host CRC 0x%08x, image CRC 0x%08x: %satch\n",
				crc,
				header->text_crc, rc ? "does not m" : "m");
	} else {
		dev_err(dev, "could not read crc\n");
	}

exit_release:
	if (!em718x->fw.data)
		release_firmware(fw);
exit:
	return rc;
}

static int em718x_parameters_apply(struct em718x *em718x)
{
	int i, n;
	int rc;

	for (i = 0; i < em718x->dts_parameters_num; i++) {
		for (n = 0; n < 2; n++) {
			rc = em718x_parameter_load(em718x,
					em718x->dts_parameters[i].val,
					em718x->dts_parameters[i].addr);
			if (!rc)
				break;
		}
		if (rc)
			return rc;
	}
	return 0;
}

static void em718x_reset_work_func(struct work_struct *work)
{
	struct delayed_work *dw = container_of(work, struct delayed_work,
			work);
	struct em718x *em718x = container_of(dw, struct em718x, reset_work);
	struct device *dev = &em718x->client->dev;
	int rc;
	u8 status;

	dev_dbg(&em718x->client->dev, "%s\n", __func__);

	mutex_lock(&em718x->lock);
	(void)smbus_read_byte(em718x->client, R8_CENTRAL_STATUS, &status);
	if (!(STATUS_INITIALIZED & status)) {
		dev_warn(dev, "not in initialized state yet\n");
		schedule_delayed_work(&em718x->reset_work,
				msecs_to_jiffies(10));
		goto exit_rescheduled;
	}

	rc = em718x_load_firmware(em718x);
	if (rc) {
		dev_warn(dev, "triggering device reset\n");
		rc = smbus_write_byte(em718x->client, R8_RESET, 0x01);
		goto exit;
	}
	rc = em718x_standby(em718x, true);
	if (rc)
		goto exit;
	rc = em718x_host_ctl(em718x, RUN_ENABLE);
	if (rc)
		goto exit;

	em718x_parameters_apply(em718x);
	em718x_parse_features(em718x);

	mutex_lock(&em718x->ctl_lock);
	memset(em718x->sns_state, 0, sizeof(em718x->sns_state));
	post_control_work_locked(em718x);
	mutex_unlock(&em718x->ctl_lock);
	mutex_unlock(&em718x->lock);
	wait_event_interruptible(em718x->control_wq,
			!em718x->control_pending);
	dev_info(dev, "reset finished with no errors\n");
	return;

exit_rescheduled:
	mutex_unlock(&em718x->lock);
	return;
exit:
	(void)em718x_host_ctl(em718x, 0);
	dev_info(dev, "reset finished with errors, device in shutdown state\n");
	mutex_unlock(&em718x->lock);
	return;
}

static int em718x_suspend_notifier(struct notifier_block *nb,
		unsigned long event, void *dummy)
{
	struct em718x *em718x = container_of(nb, struct em718x, nb);
	(void)dummy;

	switch (event) {
	case PM_SUSPEND_PREPARE:
		mutex_lock(&em718x->lock);

		dev_dbg(&em718x->client->dev,
			"PM_SUSPEND_PREPARE: report enable mask 0x%02x\n",
			em718x->enabled_sns & em718x->suspend_enabled_sns);

		em718x_update_event_ena_reg(em718x, em718x->enabled_sns &
				em718x->suspend_enabled_sns);
		(void)notify_app_cpu_suspended(em718x, true);
		em718x->suspend_prepared = true;

		mutex_unlock(&em718x->lock);
		synchronize_irq(em718x->irq);

		dev_dbg(&em718x->client->dev, "no non-wakeup IRQs from now\n");
		if (wake_lock_active(&em718x->w_lock))
			dev_warn(&em718x->client->dev, "wake_lock_active!\n");

		break;
	case PM_POST_SUSPEND:
		mutex_lock(&em718x->lock);

		dev_dbg(&em718x->client->dev,
			"PM_POST_SUSPEND: report enable mask 0x%02x\n",
			em718x->enabled_sns);
		em718x_update_event_ena_reg(em718x, em718x->enabled_sns);
		em718x_step_counter_sync_locked(em718x);
		(void)notify_app_cpu_suspended(em718x, false);

		em718x->suspend_prepared = false;
		mutex_unlock(&em718x->lock);
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static ssize_t em718x_cdev_read(struct file *filp, char __user *buf,
		size_t size, loff_t *offs)
{
	ssize_t len;
	struct miscdevice *c = filp->private_data;
	struct event_device *ev_dev =
			container_of(c, struct event_device, cdev);
	struct em718x *em718x = ev_dev->em718x;
	struct sensor_event_queue *p = &ev_dev->ev_queue;

	if (!c)
		return -ENODEV;

	mutex_lock(&p->lock);

	while (p->ridx == p->widx) {

		mutex_unlock(&p->lock);
		if (filp->f_flags & O_NONBLOCK) {
			len = -EAGAIN;
			goto exit;
		}
		if (wait_event_interruptible(ev_dev->wq, p->ridx != p->widx)) {
			len = -ERESTARTSYS;
			goto exit;
		}
		mutex_lock(&p->lock);
	}

	for (len = 0; size >= sizeof(p->ev[0]) && p->ridx != p->widx;
			len += sizeof(p->ev[0])) {
		struct sensor_event ev = p->ev[p->ridx];
		size -= sizeof(p->ev[0]);
		p->ridx = (p->ridx + 1) % p->queue_size;

		if (copy_to_user(buf + len, &ev, sizeof(ev))) {
			len = -EFAULT;
			goto exit;
		}
	}
	if ((ev_dev == &em718x->wake_ev_device) &&
			(ev_dev->ev_queue.ridx == ev_dev->ev_queue.widx) &&
			wake_lock_active(&em718x->w_lock)) {
		wake_unlock(&em718x->w_lock);
		dev_dbg(&em718x->client->dev, "removing wake lock\n");
	}
exit:
	mutex_unlock(&p->lock);
	return len;
}

static unsigned int em718x_cdev_poll(struct file *filp,
	struct poll_table_struct *pt)
{
	unsigned int mask;
	struct miscdevice *c = filp->private_data;
	struct event_device *ev_dev =
			container_of(c, struct event_device, cdev);
	struct sensor_event_queue *p = &ev_dev->ev_queue;

	if (!c)
		return 0;

	poll_wait(filp, &ev_dev->wq, pt);
	mutex_lock(&p->lock);
	if (p->ridx != p->widx)
		mask = POLLIN | POLLRDNORM;
	else
		mask = 0;
	mutex_unlock(&p->lock);
	return mask;
}

static int em718x_cdev_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int em718x_cdev_close(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static const struct file_operations em718x_fops = {
	.owner = THIS_MODULE,
	.open = em718x_cdev_open,
	.release = em718x_cdev_close,
	.read = em718x_cdev_read,
	.poll = em718x_cdev_poll,
};

static int em718x_setup_cdev(struct em718x *em718x)
{
	static int dev_index;
	int rc;
	struct sensor_event *ev = devm_kzalloc(&em718x->client->dev,
			(WAKE_EVENT_Q_SIZE + EVENT_Q_SIZE) * sizeof(*ev),
			GFP_KERNEL);

	if (!ev)
		return -ENOMEM;

	scnprintf(em718x->ev_device.name, sizeof(em718x->ev_device.name),
			"em718x-%d", dev_index);

	em718x->ev_device.ev_queue.ev = ev;
	em718x->ev_device.ev_queue.queue_size = EVENT_Q_SIZE;
	init_waitqueue_head(&em718x->ev_device.wq);
	em718x->ev_device.cdev.minor = MISC_DYNAMIC_MINOR;
	em718x->ev_device.cdev.name = em718x->ev_device.name;
	em718x->ev_device.cdev.fops = &em718x_fops;
	em718x->ev_device.em718x = em718x;
	mutex_init(&em718x->ev_device.ev_queue.lock);
	rc = misc_register(&em718x->ev_device.cdev);
	if (rc) {
		dev_err(&em718x->client->dev, "%s: misc_register error %d\n",
				__func__, rc);
		return rc;
	}
	dev_info(&em718x->client->dev, "%s: misc_device %s added\n",
				__func__, em718x->ev_device.cdev.name);

	scnprintf(em718x->wake_ev_device.name,
			sizeof(em718x->wake_ev_device.name),
			"em718x-wake-%d", dev_index);

	em718x->wake_ev_device.ev_queue.ev = ev + EVENT_Q_SIZE;
	em718x->wake_ev_device.ev_queue.queue_size = WAKE_EVENT_Q_SIZE;
	init_waitqueue_head(&em718x->wake_ev_device.wq);
	em718x->wake_ev_device.cdev.minor = MISC_DYNAMIC_MINOR;
	em718x->wake_ev_device.cdev.name = em718x->wake_ev_device.name;
	em718x->wake_ev_device.cdev.fops = &em718x_fops;
	em718x->wake_ev_device.em718x = em718x;
	mutex_init(&em718x->wake_ev_device.ev_queue.lock);
	rc = misc_register(&em718x->wake_ev_device.cdev);
	if (rc) {
		dev_err(&em718x->client->dev, "%s: misc_register error %d\n",
				__func__, rc);
		misc_deregister(&em718x->ev_device.cdev);
	}
	dev_info(&em718x->client->dev, "%s: misc_device %s added\n",
				__func__, em718x->wake_ev_device.cdev.name);
	dev_index++;
	return rc;
}

static int em718x_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int rc;
	struct em718x *em718x;
	struct device *dev = &client->dev;
	int i;

	rc = i2c_check_functionality(client->adapter, I2C_FUNC_I2C);
	if (!rc) {
		dev_err(dev, "i2c_check_functionality error\n");
		return -ENODEV;
	}
	em718x = devm_kzalloc(dev, sizeof(*em718x), GFP_KERNEL);
	if (!em718x)
		return -ENOMEM;

	i2c_set_clientdata(client, em718x);
	em718x->client = client;
	INIT_DELAYED_WORK(&em718x->reset_work, em718x_reset_work_func);
	INIT_WORK(&em718x->sns_ctl_work, sensor_control_work);
	mutex_init(&em718x->lock);
	mutex_init(&em718x->ctl_lock);
	wake_lock_init(&em718x->w_lock, WAKE_LOCK_SUSPEND, dev_name(dev));
	init_waitqueue_head(&em718x->control_wq);
	rc = em718x_check_id(em718x);
	if (rc)
		goto exit;

	em718x->algo_ctl = ALGO_ENU_OUT;

	if (client->dev.of_node) {
		struct device_node *np = dev->of_node;
		u32 val;

		if (of_property_read_string(np, "firmware",
					&em718x->fw_image_name)) {
			dev_err(dev, "no FW image name given\n");
			rc = -EINVAL;
			goto exit;
		}

		if (!of_property_read_u32(np, "gpio-irq-pin", &val)) {
			em718x->irq_gpio = val;
		} else {
			dev_err(dev, "no IRQ GPIO given\n");
			rc = -EINVAL;
			goto exit;
		}
		rc = devm_gpio_request(dev, em718x->irq_gpio, dev_name(dev));
		if (rc) {
			dev_err(dev, "could not request gpio %d\n",
					em718x->irq_gpio);
			rc = -EINVAL;
			goto exit;
		}
		em718x->irq = gpio_to_irq(em718x->irq_gpio);

		rc = of_property_count_strings(np, "parameters");
		if (rc > 0) {
			int n;
			struct em718x_parameters *par;
			const char *name;

			par = devm_kzalloc(dev, sizeof(*par) * rc, GFP_KERNEL);
			if (!par)
				return -ENOMEM;
			for (i = 0, n = 0; i < rc; i++) {
				if (of_property_read_string_index(np,
						"parameters", i, &name))
					continue;
				if (2 == sscanf(name, "%i %i", &par[n].addr,
						&par[n].val)) {
					dev_info(dev, "parameter %d = 0x%08x",
						par[n].addr, par[n].val);
					n++;
				}
			}
			em718x->dts_parameters_num = n;
			em718x->dts_parameters = par;
		} else {
			em718x->dts_parameters_num = 0;
		}

	} else {
		dev_err(dev, "could not find dts record\n");
		rc = -EINVAL;
		goto exit;
	}


	rc = em718x_setup_cdev(em718x);
	if (rc)
		goto exit;

	rc = sysfs_create_bin_file(&dev->kobj, &em718x_firmware);
	if (rc < 0)
		dev_err(dev, "could not create bin sysfs\n");

	rc = sysfs_create_bin_file(&dev->kobj, &parameter_bin_data);
	if (rc < 0)
		dev_err(dev, "could not create parameter bin sysfs\n");

	rc = sysfs_create_group(&dev->kobj, &em718x_attribute_group);
	if (rc < 0)
		dev_err(dev, "could not create sysfs\n");

	rc = devm_request_threaded_irq(dev, em718x->irq, NULL,
			em718x_irq_handler, IRQF_TRIGGER_RISING | IRQF_ONESHOT |
			IRQF_NO_SUSPEND, dev_name(dev), em718x);
	if (rc) {
		dev_err(dev, "could not request irq %d\n",
				em718x->irq);
		goto exit_misc;
	}
	device_init_wakeup(dev, 1);
	dev_info(dev, "init finished with no errors, make reset to load FW\n");

	em718x->nb.notifier_call = em718x_suspend_notifier;
	register_pm_notifier(&em718x->nb);

	return 0;
exit_misc:
	misc_deregister(&em718x->ev_device.cdev);
	misc_deregister(&em718x->wake_ev_device.cdev);
exit:
	wake_lock_destroy(&em718x->w_lock);
	return rc;
}

static int em718x_remove(struct i2c_client *client)
{
	struct em718x *em718x = dev_get_drvdata(&client->dev);
	unregister_pm_notifier(&em718x->nb);
	sysfs_remove_group(&client->dev.kobj, &em718x_attribute_group);
	sysfs_remove_bin_file(&client->dev.kobj, &parameter_bin_data);
	misc_deregister(&em718x->ev_device.cdev);
	misc_deregister(&em718x->wake_ev_device.cdev);
	wake_lock_destroy(&em718x->w_lock);
	return 0;
}

static int em718x_suspend_prepare(struct device *dev)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	mutex_lock(&em718x->lock);
	dev_dbg(dev, "%s\n", __func__);
	return 0;
}

static void em718x_resume_complete(struct device *dev)
{
	struct em718x *em718x = dev_get_drvdata(dev);
	dev_dbg(dev, "%s\n", __func__);
	mutex_unlock(&em718x->lock);
}

static const struct dev_pm_ops em718x_pm_ops = {
	.prepare = em718x_suspend_prepare,
	.complete = em718x_resume_complete,
};

static const struct i2c_device_id em718x_id[] = {
	{"em718x-hub", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, em718x_id);

static const struct of_device_id em718x_of_match[] = {
	{.compatible = "em718x-hub",},
	{},
}
MODULE_DEVICE_TABLE(of, em718x_of_match);

static struct i2c_driver em718x_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "em718x-hub",
		   .of_match_table = em718x_of_match,
		   .pm = &em718x_pm_ops,
		   },
	.id_table = em718x_id,
	.probe = em718x_probe,
	.remove = em718x_remove,
};

static int __init em718x_init(void)
{
	return i2c_add_driver(&em718x_driver);
}

static void __exit em718x_exit(void)
{
	i2c_del_driver(&em718x_driver);
}

MODULE_AUTHOR("Aleksej Makarov <aleksej.makarov@sonymobile.com>");
MODULE_DESCRIPTION("em718x sensor hub  driver");
MODULE_LICENSE("GPL");

module_init(em718x_init);
module_exit(em718x_exit);

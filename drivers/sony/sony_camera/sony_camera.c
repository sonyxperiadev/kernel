/*
 * Copyright 2021 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/kthread.h>
#include <linux/thermal.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/pm_runtime.h>
#include <uapi/linux/sched/types.h>
#include <linux/i2c.h>
#include <cam_cci_dev.h>
#include "sony_camera.h"

#define SONY_CAMERA_LOGE		(1 << 0)
#define SONY_CAMERA_LOGD		(1 << 1)
#define SONY_CAMERA_LOGI		(1 << 2)
#define SONY_CAMERA_HEX_DUMP	(1 << 8)

#define LOGE(fmt, args...)                                                 \
	({                                                                     \
		sony_camera_log(SONY_CAMERA_LOGE, __func__, __LINE__, fmt, ##args);\
	})

#define LOGD(fmt, args...)                                                 \
	({                                                                     \
		sony_camera_log(SONY_CAMERA_LOGD, __func__, __LINE__, fmt, ##args);\
	})

#define LOGI(fmt, args...)                                                 \
	({                                                                     \
		sony_camera_log(SONY_CAMERA_LOGI, __func__, __LINE__, fmt, ##args);\
	})

#define SONY_CAMERA_MCLK_DEFAULT				8000000
#define SONY_CAMERA_MAX_RETRY_COUNT				5
#define SONY_CAMERA_MAX_IDLE_RETRY_COUNT		0
#define SONY_CAMERA_FRONT_SENSOR_POWER_UP_WAIT	10
#define SONY_CAMERA_I2C_MAX_LEN					(256)
#define SONY_CAMERA_I2C_MAX_SIZE				(sizeof(struct cam_sensor_i2c_reg_array) * SONY_CAMERA_I2C_MAX_LEN)
#define SONY_CAMERA_QUP_I2C_MAX_LEN				(SONY_CAMERA_I2C_MAX_LEN + SONY_CAMERA_I2C_TYPE_MAX)
#define SONY_CAMERA_HEX_DUMP_LINE_BYTES			16
#define SONY_CAMERA_HEX_DUMP_LINE_LENGTH		(10 + SONY_CAMERA_HEX_DUMP_LINE_BYTES * 3 + SONY_CAMERA_HEX_DUMP_LINE_BYTES / 4)

#define SONY_CAMERA_SPI_FLASH_PP_MAX_LEN	256					// 256byte
#define SONY_CAMERA_SPI_FLASH_MAX_SIZE		(16 * 1024 * 1024)	// Max size 16MB
#define SONY_CAMERA_SPI_FLASH_SECTOR_SIZE	(64 * 1024)			// Sector size 64KB
#define SONY_CAMERA_SPI_FLASH_PAGE_PROGRAM	0x02				// Page program
#define SONY_CAMERA_SPI_FLASH_READ_DATA		0x03				// Read data bytes
#define SONY_CAMERA_SPI_FLASH_READ_STATUS	0x05				// Read status
#define SONY_CAMERA_SPI_FLASH_WRITE_ENABLE	0x06				// Write enable
#define SONY_CAMERA_SPI_FLASH_READ_ID		0x9F				// Read identification
#define SONY_CAMERA_SPI_FLASH_BULK_ERASE	0xC7				// Bulk erase
#define SONY_CAMERA_SPI_FLASH_SECTOR_ERASE	0xD8				// Sector erase

#define SONY_CAMERA_SPI_COMPLETION_TIMEOUT	msecs_to_jiffies(10000)
#define SONY_CAMERA_IDLE_COMPLETION_TIMEOUT	msecs_to_jiffies(10)
#define SONY_CAMERA_TASKQ_SIZE				10

#define SONY_CAMERA_DEV_NAME				"sony_camera"
#define SONY_CAMERA_PINCTRL_STATE_SLEEP		"cam_suspend"
#define SONY_CAMERA_PINCTRL_STATE_DEFAULT	"cam_default"
#define SONY_CAMERA_GPIO_RESET				"SONY_CAMERA_RESET"
#define SONY_CAMERA_GPIO_RESET2				"SONY_CAMERA_RESET2"
#define SONY_CAMERA_GPIO_IRQ_SPI_READY		"SONY_CAMERA_SPI_READY"
#define SONY_CAMERA_GPIO_IRQ_SOF			"SONY_CAMERA_SOF"
#define SONY_CAMERA_GPIO_VANA				"SONY_CAMERA_VANA"
#define SONY_CAMERA_GPIO_IRQ_EXTERNAL		"SONY_CAMERA_EXTERNAL"
#define SONY_CAMERA_GPIO_IRQ_IDLE			"SONY_CAMERA_IDLE"
#define SONY_CAMERA_MIPI_SWITCH				"SONY_CAMERA_SWITCH"

enum sony_camera_irq_type {
	SONY_CAMERA_IRQ_SPI_READY_ASSERT_EVENT		= 1,
	SONY_CAMERA_IRQ_SPI_READY_DEASSERT_EVENT	= 2,
	SONY_CAMERA_IRQ_SOF_EVENT					= 3,
	SONY_CAMERA_IRQ_EXTERNAL_EVENT				= 4,
	SONY_CAMERA_IRQ_IDLE_ASSERT_EVENT			= 5,
	SONY_CAMERA_IRQ_IDLE_DEASSERT_EVENT			= 6,
	SONY_CAMERA_IRQ_EVENT_MAX					= 7,
};

enum sony_camera_state {
	SONY_CAMERA_STATE_POWER_UP,
	SONY_CAMERA_STATE_POWER_DOWN,
	SONY_CAMERA_STATE_MAX
};

enum sony_camera_irq_status {
	SONY_CAMERA_IRQ_STATUS_DEASSERT,
	SONY_CAMERA_IRQ_STATUS_ASSERT,
	SONY_CAMERA_IRQ_STATUS_MAX,
};

struct sony_camera_task_queue_cmd {
	struct list_head			list;
	const char					*irq_name;
	enum sony_camera_irq_type	irq_type;
	bool						cmd_used;
};

struct sony_camera_seq {
	enum sony_camera_cmd	cmd;
	int						val1;
	int						val2;
	int						wait;
};

struct sony_camera_module {
	const char		*name;
	struct sony_camera_seq	*seq_on;
	struct sony_camera_seq	*seq_off;
	uint32_t				i2c_freq_mode;
};

struct sony_camera_match_id {
	uint16_t	slave_addr;
	uint16_t	addr;
	uint8_t		len;
	uint16_t	expect_value;
};

struct sony_camera_event_list_data {
	struct list_head				list;
	struct sony_camera_event_data	event_data;
};

struct sony_camera_cci_device {
	struct cam_sensor_cci_client	cci_info;
	char							*cache;
	struct mutex					lock;
};

struct sony_camera_qup_i2c_device {
	uint8_t				enabled;
	struct i2c_client	*client;
	uint8_t				*cache;
	struct mutex		lock;
};

struct sony_camera_qup_spi_device {
	uint8_t				enabled;
	struct spi_device	*client;
	uint8_t				*cache;
	uint32_t			cache_size;
	uint32_t			max_rx_size;
	uint32_t			max_tx_size;
	struct mutex		lock;
};

struct sony_camera_regulator_t {
	struct regulator	*vreg;
	bool				enabled_by_user;
};

struct sony_camera_data {
	// platform device
	uint32_t							id;
	struct sony_camera_match_id			match_id;
	struct platform_device				*p_dev;
	struct sony_camera_module			*module;
	bool								probe_done;
	// camera sub device on qup
	struct sony_camera_qup_i2c_device	qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_MAX];
	struct sony_camera_qup_spi_device	qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_MAX];
	// CCI
	struct sony_camera_cci_device		cci_dev;
	// pin controll
	struct pinctrl						*pinctrl;
	struct pinctrl_state				*gpio_state_active;
	struct pinctrl_state				*gpio_state_suspend;
	struct gpio							*gpio_req_tbl;
	uint8_t								gpio_req_tbl_size;
	bool								gpio_requested;
	bool								has_hw_sof;
	bool								has_hw_ext_irq;
	bool								has_idle;
	uint8_t								sof_irq_trigger_type;
	// power
	struct sony_camera_regulator_t		cam_vdig;
	struct sony_camera_regulator_t		cam_vio;
	struct sony_camera_regulator_t		cam_vana;
	struct sony_camera_regulator_t		cam_vaf;
	struct sony_camera_regulator_t		cam_vaf2;
	struct sony_camera_regulator_t		cam_ois;
	struct sony_camera_regulator_t		cam_ois2;
	struct sony_camera_regulator_t		cam_clk;
	struct sony_camera_regulator_t		cam_clk2;
	struct sony_camera_regulator_t		cam_clk3;
	struct sony_camera_regulator_t		cam_vmdr;
	struct sony_camera_regulator_t		cam_vdig2;
	struct sony_camera_regulator_t		cam_vdig3;
	struct sony_camera_regulator_t		cam_vdig4;
	struct sony_camera_regulator_t		cam_vana2;
	struct sony_camera_regulator_t		pmic_vin;
	struct sony_camera_regulator_t		cam_pmic_cs0;
	struct sony_camera_regulator_t		cam_pmic_cs1;
	struct sony_camera_regulator_t		cam_pmic_vdd0;
	struct sony_camera_regulator_t		cam_pmic_vdd1;
	struct sony_camera_regulator_t		cam_gyro;
	struct sony_camera_regulator_t		cam_isp1;
	struct sony_camera_regulator_t		cam_isp2;
	struct sony_camera_regulator_t		cam_isp3;
	struct sony_camera_regulator_t		cam_isp4;
	struct sony_camera_regulator_t		cam_isp5;
	struct sony_camera_regulator_t		cam_extio;
	struct clk							*clk_handle;
	struct mutex						state_lock;
	enum sony_camera_state				state;
	// thermal
	uint16_t							thremal_enable;
	struct mutex						thermal_lock;
	struct thermal_zone_device			*thermal_zone_dev[SONY_CAMERA_MAX_TEMPERATURE_NUM_PER_CAMERA];
	int32_t								thermal_sensor_temperature[SONY_CAMERA_MAX_TEMPERATURE_NUM_PER_CAMERA];
	int									thermal_ret_val;
	// Sensor SOF count
	spinlock_t							sof_lock;
	uint32_t							sof_count;
	// User event
	spinlock_t							event_list_lock;
	wait_queue_head_t					event_wait_q;
	struct list_head					event_available;
	// Kernel event
	struct completion					spi_assert_complete;
	struct completion					spi_deassert_complete;
	struct completion					idle_assert_complete;
	struct completion					idle_deassert_complete;
	spinlock_t							task_lock;
	struct task_struct					*sensor_task;
	wait_queue_head_t					wait_q;
	struct list_head					task_q;
	atomic_t							irq_cnt;
	uint8_t								taskq_idx;
	struct sony_camera_task_queue_cmd	task_queue_cmd[SONY_CAMERA_TASKQ_SIZE];
	struct mutex						driver_lock;
	uint32_t							open_count;
};

struct sony_camera_driver_combination_info {
	uint32_t id;
	uint32_t thermal_zone;
	char thermal_name[32];
};

static int g_dev_id;
static uint16_t g_sensor_num;
static struct platform_device *g_camera_device;
static struct class *g_camera_device_class;
static uint64_t g_sony_camera_debug_flag = SONY_CAMERA_LOGE;
static struct dentry *g_dbg_file_folder_ptr = NULL;
static struct sony_camera_data camera_data[] =
{
	{
		.id = 0,
		.thermal_sensor_temperature = { 0, 0, 0 },
		.thermal_ret_val = -ENODEV,
		.has_hw_sof = 0,
	},
	{
		.id = 1,
		.thermal_sensor_temperature = { 0, 0, 0 },
		.thermal_ret_val = -ENODEV,
		.has_hw_sof = 0,
	},
	{
		.id = 2,
		.thermal_sensor_temperature = { 0, 0, 0 },
		.thermal_ret_val = -ENODEV,
		.has_hw_sof = 0,
	},
	{
		.id = 3,
		.thermal_sensor_temperature = { 0, 0, 0 },
		.thermal_ret_val = -ENODEV,
		.has_hw_sof = 0,
	},
	{
		.id = 4,
		.thermal_sensor_temperature = { 0, 0, 0 },
		.thermal_ret_val = -ENODEV,
		.has_hw_sof = 0,
	},
	{
		.id = 5,
		.thermal_sensor_temperature = { 0, 0, 0 },
		.thermal_ret_val = -ENODEV,
		.has_hw_sof = 0,
	},
};
// TODO: Add thermal name on dtsi-file.
static struct sony_camera_driver_combination_info driver_comb_tbl[] =
{
	{
		.id = 0,
		.thermal_zone = 0,
		.thermal_name = "sony_cam_back0",
	},
	{
		.id = 1,
		.thermal_zone = 0,
		.thermal_name = "sony_cam_back1",
	},
	{
		.id = 1,
		.thermal_zone = 1,
		.thermal_name = "sony_cam_back1_ext",
	},
	{
		.id = 2,
		.thermal_zone = 0,
		.thermal_name = "sony_cam_front0",
	},
	{
		.id = 3,
		.thermal_zone = 0,
		.thermal_name = "sony_cam_back2",
	},
	{
		.id = 4,
		.thermal_zone = 0,
		.thermal_name = "sony_cam_tof",
	},
	{
		.id = 4,
		.thermal_zone = 1,
		.thermal_name = "sony_cam_tof_laser",
	},
};

static int sony_camera_set_power(struct sony_camera_data*, int, bool);

static void sony_camera_log(uint8_t log_type, const char *func,
	const int line, const char *fmt, ...) {
	char str_buffer[512];
	va_list args;

	va_start(args, fmt);
	vsnprintf(str_buffer, 512, fmt, args);

	switch (log_type) {
	case SONY_CAMERA_LOGE:
		if (g_sony_camera_debug_flag & SONY_CAMERA_LOGE) {
			pr_err("SONY_CAM_ERR: %s: %d: %s",
				func, line, str_buffer);
		}
		break;
	case SONY_CAMERA_LOGD:
		if (g_sony_camera_debug_flag & SONY_CAMERA_LOGD) {
			pr_err("SONY_CAM_DBG: %s: %d: %s",
				func, line, str_buffer);
		}
		break;
	case SONY_CAMERA_LOGI:
		if (g_sony_camera_debug_flag & SONY_CAMERA_LOGI) {
			pr_err("SONY_CAM_INFO: %s: %d: %s",
				func, line, str_buffer);
		}
		break;
	default:
		break;
	}
	va_end(args);
}

static void sony_camera_hex_dump(const char *buf, uint32_t len)
{
	char linebuf[SONY_CAMERA_HEX_DUMP_LINE_LENGTH] = { 0 };
	uint32_t i = 0;
	uint32_t j = 0;

	for (i = 0; i < (len - 1) / SONY_CAMERA_HEX_DUMP_LINE_BYTES + 1; i++) {
		char *p = linebuf;

		p += scnprintf(p, 12, "%08X: ", i * SONY_CAMERA_HEX_DUMP_LINE_BYTES);
		for (j = 0; j < SONY_CAMERA_HEX_DUMP_LINE_BYTES &&
			(i * SONY_CAMERA_HEX_DUMP_LINE_BYTES + j < len); j++) {
			p += scnprintf(p, 4, "%02X ",
				(uint8_t)buf[i * SONY_CAMERA_HEX_DUMP_LINE_BYTES + j]);
			if ((j + 1) % 4 == 0)
				p += scnprintf(p, 2, " ");
		}
		LOGI("%s", linebuf);
	}
}

static int sony_camera_get_dt_gpio_req_tbl(struct device_node *of_node,
	struct sony_camera_data *camera_data)
{
	int32_t rc = 0, i = 0;
	uint32_t count = 0;
	uint16_t *gpio_array = NULL;
	int16_t gpio_array_size = 0;
	uint32_t *val_array = NULL;

	gpio_array_size = of_gpio_count(of_node);

	if (gpio_array_size <= 0)
		return 0;

	LOGI("gpio count %d", gpio_array_size);

	gpio_array = vzalloc(gpio_array_size * sizeof(uint16_t));
	if (!gpio_array)
		goto exit;

	for (i = 0; i < gpio_array_size; i++)
		gpio_array[i] = of_get_gpio(of_node, i);

	if (!of_get_property(of_node, "gpio-req-tbl-num", &count))
		return 0;

	count /= sizeof(uint32_t);
	if (!count) {
		LOGE("gpio-req-tbl-num 0");
		return 0;
	}

	val_array = vzalloc(count * sizeof(uint32_t));
	if (!val_array) {
		vfree(gpio_array);
		return -ENOMEM;
	}

	camera_data->gpio_req_tbl = vzalloc(count * sizeof(struct gpio));
	if (!camera_data->gpio_req_tbl) {
		rc = -ENOMEM;
		goto free_val_array;
	}
	camera_data->gpio_req_tbl_size = count;

	rc = of_property_read_u32_array(of_node, "gpio-req-tbl-num",
		val_array, count);
	if (rc) {
		LOGE("failed in reading gpio-req-tbl-num, rc = %d", rc);
		goto free_gpio_req_tbl;
	}

	for (i = 0; i < count; i++) {
		if (val_array[i] >= gpio_array_size) {
			LOGE("gpio req tbl index %d invalid", val_array[i]);
			goto free_gpio_req_tbl;
		}
		camera_data->gpio_req_tbl[i].gpio = gpio_array[val_array[i]];
	}

	rc = of_property_read_u32_array(of_node, "gpio-req-tbl-flags",
		val_array, count);
	if (rc) {
		LOGE("Failed in gpio-req-tbl-flags, rc %d", rc);
		goto free_gpio_req_tbl;
	}

	for (i = 0; i < count; i++)
		camera_data->gpio_req_tbl[i].flags = val_array[i];

	for (i = 0; i < count; i++) {
		rc = of_property_read_string_index(of_node,
			"gpio-req-tbl-label", i,
			&camera_data->gpio_req_tbl[i].label);
		if (rc) {
			LOGE("Failed rc %d", rc);
			goto free_gpio_req_tbl;
		}
	}

	vfree(val_array);
	vfree(gpio_array);

	return rc;

free_gpio_req_tbl:
	vfree(camera_data->gpio_req_tbl);
free_val_array:
	vfree(val_array);
	camera_data->gpio_req_tbl_size = 0;
	vfree(gpio_array);
exit:

	return rc;
}

static struct gpio *sony_camera_get_gpio_pin(
	struct sony_camera_data *camera_data, const char *label)
{
	uint32_t i = 0;
	uint32_t size = 0;
	struct gpio *gpio_tbl = NULL;

	if (camera_data == NULL || label == NULL) {
		LOGE("invalid args:%p, %p", camera_data, label);
		return NULL;
	}
	gpio_tbl = camera_data->gpio_req_tbl;
	size = camera_data->gpio_req_tbl_size;
	for (i = 0; i < size; i++) {
		if (!strcmp(label, gpio_tbl->label))
			return gpio_tbl;
		gpio_tbl++;
	}
	return NULL;
}

static int sony_camera_parse_dt(struct platform_device *p_dev,
	uint32_t id)
{
	int rc = 0;
	int count = 0;
	uint16_t i = 0;
	uint32_t val_u32[4] = {0};
	struct device_node *of_node = p_dev->dev.of_node;
	struct device_node *of_node_module = NULL;
	struct device_node *of_node_module_power_off = NULL;
	struct device_node *of_node_module_power_on = NULL;
	const int8_t *power_order_name = NULL;

	rc = of_property_read_u32_array(of_node, "match_id", &val_u32[0], 4);
	if (rc < 0) {
		LOGE("read property failed rc %d", rc);
		goto fail;
	}
	camera_data[id].match_id.slave_addr = val_u32[0];
	camera_data[id].match_id.addr = val_u32[1];
	camera_data[id].match_id.len = val_u32[2];
	camera_data[id].match_id.expect_value = val_u32[3];

	rc = of_property_read_u32(of_node, "cci-device", &val_u32[0]);
	if (rc < 0) {
		camera_data[id].cci_dev.cci_info.cci_device = CCI_DEVICE_0;
	} else {
		camera_data[id].cci_dev.cci_info.cci_device = val_u32[0];
	}

	rc = of_property_read_u32(of_node, "cci-master", &val_u32[0]);
	if (rc < 0) {
		camera_data[id].cci_dev.cci_info.cci_i2c_master = MASTER_0;
	} else {
		camera_data[id].cci_dev.cci_info.cci_i2c_master = val_u32[0];
	}

	rc = of_property_read_u32(of_node, "thremal_enable", &val_u32[0]);
	if (rc < 0) {
		LOGE("read property failed rc %d", rc);
		goto fail;
	}
	camera_data[id].thremal_enable = val_u32[0];

	rc = sony_camera_get_dt_gpio_req_tbl(of_node, &camera_data[id]);
	if (rc < 0) {
		LOGE("read gpio table failed rc %d", rc);
		goto fail;
	}

	if (sony_camera_get_gpio_pin(&camera_data[id],
		SONY_CAMERA_GPIO_IRQ_SOF) == NULL) {
		camera_data[id].has_hw_sof = 0;
	} else {
		camera_data[id].has_hw_sof = 1;
	}

	if (camera_data[id].has_hw_sof) {
		rc = of_property_read_u32(of_node, "sof_irq_trigger_type", &val_u32[0]);
		if (rc < 0) {
			LOGE("read sof irq trigger type property failed rc %d", rc);
			goto fail;
		}
		camera_data[id].sof_irq_trigger_type = val_u32[0] & IRQF_TRIGGER_MASK;
	}

	if (sony_camera_get_gpio_pin(&camera_data[id],
		SONY_CAMERA_GPIO_IRQ_EXTERNAL) == NULL) {
		camera_data[id].has_hw_ext_irq = 0;
	} else {
		camera_data[id].has_hw_ext_irq = 1;
	}

	if (sony_camera_get_gpio_pin(&camera_data[id],
		SONY_CAMERA_GPIO_IRQ_IDLE) == NULL) {
		camera_data[id].has_idle = 0;
	} else {
		camera_data[id].has_idle = 1;
	}

	camera_data[id].module = vzalloc(sizeof(struct sony_camera_module));
	if (!camera_data[id].module) {
		LOGE("no memory");
		rc = -ENOMEM;
		goto fail;
	}

	rc = of_property_read_string(of_node,
		"module_name",
		(const char **)(&camera_data[id].module->name));
	LOGD("name[%d] = %s", id, camera_data[id].module->name);
	if (rc < 0) {
		LOGE("read property failed rc %d", rc);
		goto fail;
	}

	of_node_module = of_find_node_by_name(of_node,
		camera_data[id].module->name);
	if (!of_node_module) {
		LOGE("read property failed rc %d", rc);
		rc = -EFAULT;
		goto fail;
	}

	rc = of_property_read_u32(of_node_module,
		"i2c_freq_mode",
		&camera_data[id].module->i2c_freq_mode);

	if (rc < 0) {
		LOGE("read property failed rc %d", rc);
		goto fail;
	}

	of_node_module_power_off = of_find_node_by_name(
		of_node_module, "power_off");
	if (!of_node_module_power_off) {
		LOGE("read property failed rc %d", rc);
		rc = -EFAULT;
		goto fail;
	}

	count = of_property_count_strings(of_node_module_power_off,
		"commands");
	if (count < 0) {
		LOGE("read property failed rc %d", rc);
		rc = -EFAULT;
		goto fail;
	}
	camera_data[id].module->seq_off = vzalloc(sizeof(struct sony_camera_seq) * count);
	if (!camera_data[id].module->seq_off) {
		LOGE("read property failed rc %d", rc);
		rc = -ENOMEM;
		goto fail;
	}

	for (i = 0; i < count; i++) {
		rc = of_property_read_string_index(
			of_node_module_power_off,
			"commands", i,
			(const char **)(&power_order_name));
		if (rc < 0) {
			LOGE("read property failed rc %d", rc);
			goto fail;
		}

		rc = of_property_read_u32_array(
			of_node_module_power_off, power_order_name,
			&val_u32[0], 4);
		if (rc < 0) {
			LOGE("read property failed rc %d", rc);
			goto fail;
		}
		camera_data[id].module->seq_off[i].cmd = val_u32[0];
		camera_data[id].module->seq_off[i].val1 = val_u32[1];
		camera_data[id].module->seq_off[i].val2 = val_u32[2];
		camera_data[id].module->seq_off[i].wait = val_u32[3];
	}

	of_node_module_power_on = of_find_node_by_name(of_node_module,
		"power_on");
	if (!of_node_module_power_on) {
		LOGE("read property failed rc %d", rc);
		rc = -EFAULT;
		goto fail;
	}

	count = of_property_count_strings(of_node_module_power_on,
		"commands");
	if (count < 0) {
		LOGE("read property failed rc %d", rc);
		rc = -EFAULT;
		goto fail;
	}

	camera_data[id].module->seq_on = vzalloc(sizeof(struct sony_camera_seq) * count);
	if (!camera_data[id].module->seq_on) {
		LOGE("no memory");
		rc = -ENOMEM;
		goto fail;
	}

	for (i = 0; i < count; i++) {
		rc = of_property_read_string_index(
			of_node_module_power_on,
			"commands", i,
			(const char **)(&power_order_name));
		if (rc < 0) {
			LOGE("read property failed i=%d count=%d", i, count);
			goto fail;
		}

		rc = of_property_read_u32_array(
			of_node_module_power_on, power_order_name,
			&val_u32[0], 4);
		if (rc < 0) {
			LOGE("read property failed j=%d count=%d", i, count);
			goto fail;
		}
		camera_data[id].module->seq_on[i].cmd = val_u32[0];
		camera_data[id].module->seq_on[i].val1 = val_u32[1];
		camera_data[id].module->seq_on[i].val2 = val_u32[2];
		camera_data[id].module->seq_on[i].wait = val_u32[3];
	}

	return 0;
fail:
	if (camera_data[id].module) {
		if (camera_data[id].module->seq_on)
			vfree(camera_data[id].module->seq_on);
		if (camera_data[id].module->seq_off)
			vfree(camera_data[id].module->seq_off);
		if (camera_data[id].module)
			vfree(camera_data[id].module);
	}

	return rc;
}

static int sony_camera_gpio_init(struct sony_camera_data *data)
{
	int rc = 0;
	uint16_t i = 0;
	struct gpio *gpio_tbl = data->gpio_req_tbl;
	uint8_t size = data->gpio_req_tbl_size;

	if (!data->gpio_requested) {
		for (i = 0; i < size; i++) {
			rc = gpio_request_one(gpio_tbl[i].gpio,
				gpio_tbl[i].flags, gpio_tbl[i].label);
			if (rc) {
				// TODO: After GPIO request fails, contine to apply new gpios,
				// outout a error message for driver bringup debug
				LOGE("gpio %d:%s request failed",
					gpio_tbl[i].gpio, gpio_tbl[i].label);
			}
		}
		if (rc == 0)
			data->gpio_requested = true;
	}
	return rc;
}

static int sony_camera_gpio_deinit(struct sony_camera_data *data)
{
	int rc = 0;
	struct gpio *gpio_tbl = data->gpio_req_tbl;
	uint8_t size = data->gpio_req_tbl_size;

	if (data->gpio_requested) {
		gpio_free_array(gpio_tbl, size);
		data->gpio_requested = false;
	}
	return rc;
}

static int sony_camera_gpio_set(struct sony_camera_data *data,
	int gpio_pin, int value)
{
	int rc = 0;

	if (data->gpio_requested)
		gpio_set_value_cansleep(gpio_pin, value);
	else
		rc = -EPERM;

	return rc;
}

static int sony_camera_regist_gpio_irq(
	struct sony_camera_data *camera_data, irq_handler_t handler,
	unsigned int flags, const char *label)
{
	int irq;
	int rc = 0;
	struct gpio *irq_gpio = NULL;

	irq_gpio = sony_camera_get_gpio_pin(camera_data, label);
	if (!irq_gpio) {
		LOGE("Sony sensor: can't find gpio irq:%s", label);
		return -EINVAL;
	}
	irq = gpio_to_irq(irq_gpio->gpio);
	rc = request_irq(irq, handler, flags, irq_gpio->label, camera_data);
	if (rc) {
		LOGE("Sony sensor: can't regist gpio irq:%s", label);
		return -EINVAL;
	}
	LOGD("regist gpio %d for %s trigger type %x", irq_gpio->gpio, label, flags);
	return 0;
}

static int sony_camera_unregist_gpio_irq(
	struct sony_camera_data *camera_data, const char *label)
{
	int irq;
	struct gpio *irq_gpio = NULL;

	irq_gpio = sony_camera_get_gpio_pin(camera_data, label);
	if (!irq_gpio) {
		LOGE("Sony sensor: can't find gpio irq:%s", label);
		return -EINVAL;
	}
	irq = gpio_to_irq(irq_gpio->gpio);
	disable_irq(irq);
	free_irq(irq, camera_data);
	LOGD("unregist gpio %d for %s", irq_gpio->gpio, label);
	return 0;
}

static void sony_camera_send_event(struct sony_camera_data *data,
	const struct sony_camera_event_data *sensor_event)
{
	unsigned long flags;

	struct sony_camera_event_list_data *event = kzalloc(
		sizeof(struct sony_camera_event_list_data), GFP_ATOMIC);
	if (!event) {
		LOGE("no memory");
		return;
	}
	LOGI("event type 0x%x", sensor_event->type);
	memcpy(&event->event_data, sensor_event,
		sizeof(struct sony_camera_event_data));
	spin_lock_irqsave(&camera_data->event_list_lock, flags);
	list_add_tail(&event->list, &data->event_available);
	spin_unlock_irqrestore(&camera_data->event_list_lock, flags);
	wake_up_interruptible(&data->event_wait_q);
}

static irqreturn_t sony_camera_irq_handler(int irq, void *info)
{
	struct irq_desc *desc = NULL;
	struct sony_camera_data *camera_data = NULL;
	struct sony_camera_task_queue_cmd *queue_cmd = NULL;
	unsigned long flags;
	int irq_type = SONY_CAMERA_IRQ_EVENT_MAX;

	camera_data = (struct sony_camera_data *)info;
	if (!camera_data) {
		LOGE("err camera_data is NULL");
		goto exit;
	}
	spin_lock_irqsave(&camera_data->task_lock, flags);
	desc = irq_to_desc(irq);
	if (desc != NULL) {
		if (!strcmp(desc->action->name, SONY_CAMERA_GPIO_IRQ_SOF)) {
			struct sony_camera_event_data camera_event;
			struct timespec64 ts;
			unsigned long sof_lock_flags;

			ktime_get_boottime_ts64(&ts);
			memset(&camera_event, 0, sizeof(camera_event));
			spin_lock_irqsave(&camera_data->sof_lock, sof_lock_flags);
			camera_data->sof_count++;
			if (camera_data->sof_count > 0xFFFFFFF0)
				camera_data->sof_count = 1;
			camera_event.sof_data.sof_count = camera_data->sof_count;
			spin_unlock_irqrestore(&camera_data->sof_lock, sof_lock_flags);
			camera_event.type = SONY_CAMERA_EVT_SOF;
			camera_event.sof_data.mono_timestamp.tv_sec = ts.tv_sec;
			camera_event.sof_data.mono_timestamp.tv_nsec = ts.tv_nsec;
			LOGI("sof_count = %d", camera_data->sof_count);
			sony_camera_send_event(camera_data, &camera_event);
			spin_unlock_irqrestore(&camera_data->task_lock, flags);
			return IRQ_HANDLED;
		} else if (!strcmp(desc->action->name, SONY_CAMERA_GPIO_IRQ_EXTERNAL)) {
			struct sony_camera_event_data camera_event;
			memset(&camera_event, 0, sizeof(camera_event));
			camera_event.type = SONY_CAMERA_EVT_EXTERNAL;
			sony_camera_send_event(camera_data, &camera_event);
			spin_unlock_irqrestore(&camera_data->task_lock, flags);
			return IRQ_HANDLED;
		} else if (!strcmp(desc->action->name, SONY_CAMERA_GPIO_IRQ_SPI_READY) ||
			!strcmp(desc->action->name, SONY_CAMERA_GPIO_IRQ_IDLE)) {
			struct gpio *irq_gpio = NULL;
			irq_gpio = sony_camera_get_gpio_pin(
				camera_data,
				desc->action->name);
			if (!strcmp(desc->action->name, SONY_CAMERA_GPIO_IRQ_SPI_READY)) {
				irq_type = gpio_get_value(irq_gpio->gpio) ?
					SONY_CAMERA_IRQ_SPI_READY_ASSERT_EVENT :
					SONY_CAMERA_IRQ_SPI_READY_DEASSERT_EVENT;
			} else {
				irq_type = gpio_get_value(irq_gpio->gpio) ?
					SONY_CAMERA_IRQ_IDLE_ASSERT_EVENT :
					SONY_CAMERA_IRQ_IDLE_DEASSERT_EVENT;
			}
			queue_cmd = &camera_data->task_queue_cmd[camera_data->taskq_idx];
			if (!queue_cmd) {
				spin_unlock_irqrestore(&camera_data->task_lock, flags);
				LOGE("err queue_cmd is NULL");
				goto exit;
			}
			if (queue_cmd->cmd_used) {
				LOGE("err task queue overflow");
				list_del(&queue_cmd->list);
			} else {
				atomic_add(1, &camera_data->irq_cnt);
			}
			queue_cmd->cmd_used = true;
			camera_data->taskq_idx = (camera_data->taskq_idx + 1) %
				SONY_CAMERA_TASKQ_SIZE;
			if (!desc || !(desc->action) || !(desc->action->name)) {
				LOGE("err, can't find irq's name");
				queue_cmd->irq_name = "Stray irq";
			} else {
				queue_cmd->irq_name = desc->action->name;
			}
			queue_cmd->irq_type = irq_type;
			LOGD("irq_name = %s irq_type = %d taskq_idx = %d", desc->action->name,
				irq_type, camera_data->taskq_idx);
			list_add_tail(&queue_cmd->list, &camera_data->task_q);
			spin_unlock_irqrestore(&camera_data->task_lock, flags);
			wake_up_interruptible(&camera_data->wait_q);
		}
	}

exit:
	return IRQ_HANDLED;
}

static int sony_camera_irq_init(
	struct sony_camera_data *camera_data)
{
	int rc = 0;

	if (camera_data->has_hw_ext_irq) {
		rc = sony_camera_regist_gpio_irq(camera_data,
			sony_camera_irq_handler,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT,
			SONY_CAMERA_GPIO_IRQ_EXTERNAL);
		if (rc < 0)
			LOGE("regist gpio irq: %s failed",
				SONY_CAMERA_GPIO_IRQ_EXTERNAL);
	}
	if (camera_data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP].enabled) {
		rc = sony_camera_regist_gpio_irq(camera_data,
			sony_camera_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			SONY_CAMERA_GPIO_IRQ_SPI_READY);
		if (rc < 0)
			LOGE("regist gpio irq: %s failed",
				 SONY_CAMERA_GPIO_IRQ_SPI_READY);
	}
	if (camera_data->has_idle) {
		rc = sony_camera_regist_gpio_irq(camera_data,
			sony_camera_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			SONY_CAMERA_GPIO_IRQ_IDLE);
		if (rc < 0)
			LOGE("regist gpio irq: %s failed",
				 SONY_CAMERA_GPIO_IRQ_IDLE);
	}
	return rc;
}

static int sony_camera_irq_deinit(struct sony_camera_data *camera_data)
{
	int rc = 0;

	if (camera_data->has_hw_ext_irq) {
		rc = sony_camera_unregist_gpio_irq(camera_data,
			SONY_CAMERA_GPIO_IRQ_EXTERNAL);
		if (rc < 0)
			LOGE("unregist gpio irq: %s failed",
				SONY_CAMERA_GPIO_IRQ_EXTERNAL);
	}
	if (camera_data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP].enabled) {
		rc = sony_camera_unregist_gpio_irq(camera_data,
			SONY_CAMERA_GPIO_IRQ_SPI_READY);
		if (rc < 0)
			LOGE("unregist gpio irq: %s failed",
				SONY_CAMERA_GPIO_IRQ_SPI_READY);
	}
	if (camera_data->has_idle) {
		rc = sony_camera_unregist_gpio_irq(camera_data,
			SONY_CAMERA_GPIO_IRQ_IDLE);
		if (rc < 0)
			LOGE("unregist gpio irq: %s failed",
				SONY_CAMERA_GPIO_IRQ_IDLE);
	}
	return rc;
}

static int sony_camera_check_irq(struct sony_camera_data *camera_data)
{
	unsigned long flags;
	int irq_cnt;

	spin_lock_irqsave(&camera_data->task_lock, flags);
	irq_cnt = atomic_read(&camera_data->irq_cnt);
	spin_unlock_irqrestore(&camera_data->task_lock, flags);
	return irq_cnt;
}

struct sony_camera_regulator_t* sony_camera_vreg_get(
	struct sony_camera_data *data,
	enum sony_camera_cmd cmd, int level)
{
	const char *vreg_name;
	struct device *dev = &data->p_dev->dev;
	struct sony_camera_regulator_t *regulator_ptr = NULL;
	switch (cmd) {
		case SONY_CAM_VDIG:
			regulator_ptr = &data->cam_vdig;
			vreg_name = "cam_vdig";
			break;
		case SONY_CAM_VDIG2:
			regulator_ptr = &data->cam_vdig2;
			vreg_name = "cam_vdig2";
			break;
		case SONY_CAM_VDIG3:
			regulator_ptr = &data->cam_vdig3;
			vreg_name = "cam_vdig3";
			break;
		case SONY_CAM_VDIG4:
			regulator_ptr = &data->cam_vdig4;
			vreg_name = "cam_vdig4";
			break;
		case SONY_CAM_VIO:
			regulator_ptr = &data->cam_vio;
			vreg_name = "cam_vio";
			break;
		case SONY_CAM_EXTIO:
			regulator_ptr = &data->cam_extio;
			vreg_name = "cam_extio";
			break;
		case SONY_CAM_VANA:
			regulator_ptr = &data->cam_vana;
			vreg_name = "cam_vana";
			break;
		case SONY_CAM_VANA2:
			regulator_ptr = &data->cam_vana2;
			vreg_name = "cam_vana2";
			break;
		case SONY_CAM_VMDR:
			regulator_ptr = &data->cam_vmdr;
			vreg_name = "cam_vmdr";
			break;
		case SONY_CAM_VAF:
			regulator_ptr = &data->cam_vaf;
			vreg_name = "cam_vaf";
			break;
		case SONY_CAM_VAF2:
			regulator_ptr = &data->cam_vaf2;
			vreg_name = "cam_vaf2";
			break;
		case SONY_CAM_OIS:
			regulator_ptr = &data->cam_ois;
			vreg_name = "cam_ois";
			break;
		case SONY_CAM_OIS2:
			regulator_ptr = &data->cam_ois2;
			vreg_name = "cam_ois2";
			break;
		case SONY_CAM_GYRO:
			regulator_ptr = &data->cam_gyro;
			vreg_name = "cam_gyro";
			break;
		case SONY_CAM_CLK:
			regulator_ptr = &data->cam_clk;
			vreg_name = "cam_clk";
			break;
		case SONY_CAM_CLK2:
			regulator_ptr = &data->cam_clk2;
			vreg_name = "cam_clk2";
			break;
		case SONY_CAM_CLK3:
			regulator_ptr = &data->cam_clk3;
			vreg_name = "cam_clk3";
			break;
		case SONY_CAM_ISP1:
			regulator_ptr = &data->cam_isp1;
			vreg_name = "cam_isp1";
			break;
		case SONY_CAM_ISP2:
			regulator_ptr = &data->cam_isp2;
			vreg_name = "cam_isp2";
			break;
		case SONY_CAM_ISP3:
			regulator_ptr = &data->cam_isp3;
			vreg_name = "cam_isp3";
			break;
		case SONY_CAM_ISP4:
			regulator_ptr = &data->cam_isp4;
			vreg_name = "cam_isp4";
			break;
		case SONY_CAM_ISP5:
			regulator_ptr = &data->cam_isp5;
			vreg_name = "cam_isp5";
			break;
		case SONY_PMIC_CS0:
			regulator_ptr = &data->cam_pmic_cs0;
			vreg_name = "cam_pmic_cs0";
			break;
		case SONY_PMIC_CS1:
			regulator_ptr = &data->cam_pmic_cs1;
			vreg_name = "cam_pmic_cs1";
			break;
		case SONY_PMIC_VDD0:
			regulator_ptr = &data->cam_pmic_vdd0;
			vreg_name = "cam_pmic_vdd0";
			break;
		case SONY_PMIC_VDD1:
			regulator_ptr = &data->cam_pmic_vdd1;
			vreg_name = "cam_pmic_vdd1";
			break;
		case SONY_PMIC_VIN:
			regulator_ptr = &data->pmic_vin;
			vreg_name = "pmic_vin";
			break;
		default:
			goto exit;
	}
	if (!regulator_ptr->vreg && level >= 0) {
		regulator_ptr->vreg = regulator_get(dev, vreg_name);
		if (IS_ERR(regulator_ptr->vreg)) {
			LOGE("could not get %s, vreg = %ld",
				vreg_name, PTR_ERR(regulator_ptr->vreg));
			regulator_ptr = NULL;
		}
	}
exit:
	return regulator_ptr;
}

static int sony_camera_vreg_set(struct sony_camera_regulator_t *regulator_ptr,
	int level, int op_mode)
{
	int rc = 0;

	level *= 1000;
	if (regulator_ptr->vreg) {
		if (level >= 0) {
			if (level > 0) {
				rc = regulator_set_voltage(regulator_ptr->vreg, level, level);
				if (rc < 0)
					goto set_voltage_fail;
			}
			if (op_mode > 0) {
				rc = regulator_set_load(regulator_ptr->vreg, op_mode);
				if (rc < 0)
					goto set_voltage_fail;
			}
			rc = regulator_enable(regulator_ptr->vreg);
			if (rc < 0)
				goto enable_fail;
		} else {
			if (op_mode == 0)
				(void)regulator_set_load(regulator_ptr->vreg, 0);
			(void)regulator_disable(regulator_ptr->vreg);
			regulator_put(regulator_ptr->vreg);
		}
	}
	goto exit;

enable_fail:
	(void)regulator_set_load(regulator_ptr->vreg, 0);
set_voltage_fail:
	regulator_put(regulator_ptr->vreg);
exit:
	if (regulator_ptr && (rc < 0 || level < 0)) {
		if (regulator_ptr->vreg)
			regulator_ptr->vreg = NULL;
	}

	if (rc < 0)
		LOGE("error happened (%d)", rc);
	return rc;
}

static int sony_camera_mclk_set(struct sony_camera_data *data, int value)
{
	int rc = 0;
	int clk_rate = 0;
	struct device *dev = &data->p_dev->dev;
	struct clk *clk_handle = data->clk_handle;

	if (value >= 0) {
		clk_handle = clk_get(dev, "cam_clk");
		if (clk_handle == NULL) {
			LOGE("get cam_clk failed");
			rc = -EINVAL;
			goto fail_get;
		}
		clk_rate = clk_round_rate(clk_handle,
			value ? value : SONY_CAMERA_MCLK_DEFAULT);
		if (clk_rate < 0) {
			LOGE("cam_clk round failed");
			goto fail_set;
		}
		rc = clk_set_rate(clk_handle, clk_rate);
		if (rc < 0) {
			LOGE("%s set failed", "cam_clk");
			goto fail_set;
		}
		rc = clk_prepare_enable(clk_handle);
		if (rc < 0) {
			LOGE("enable failed", "cam_clk");
			goto fail_prepare_enable;
		}
		data->clk_handle = clk_handle;
	} else {
		if (clk_handle != NULL) {
			clk_disable_unprepare(clk_handle);
			clk_put(clk_handle);
			data->clk_handle = NULL;
		}
	}

	if (rc < 0)
		LOGE("error happened (%d)", rc);
	return rc;

fail_prepare_enable:
	clk_disable_unprepare(clk_handle);
fail_set:
	clk_put(clk_handle);
fail_get:
	return rc;
}

static int sony_camera_cci_init(struct sony_camera_data *data)
{
	int rc = 0;
	struct cam_cci_ctrl cci_ctrl;

	memset(&cci_ctrl, 0, sizeof(cci_ctrl));
	cci_ctrl.cmd = MSM_CCI_INIT;
	cci_ctrl.cci_info = &data->cci_dev.cci_info;
	rc = v4l2_subdev_call(data->cci_dev.cci_info.cci_subdev,
		core, ioctl, VIDIOC_MSM_CCI_CFG, &cci_ctrl);
	if (rc < 0) {
		LOGE("cci init failed rc = %d", rc);
		return rc;
	}
	rc = cci_ctrl.status;
	return rc;
}

static int sony_camera_cci_deinit(struct sony_camera_data *data)
{
	int rc = 0;
	struct cam_cci_ctrl cci_ctrl;

	memset(&cci_ctrl, 0, sizeof(cci_ctrl));
	cci_ctrl.cmd = MSM_CCI_RELEASE;
	cci_ctrl.cci_info = &data->cci_dev.cci_info;
	rc = v4l2_subdev_call(data->cci_dev.cci_info.cci_subdev,
		core, ioctl, VIDIOC_MSM_CCI_CFG, &cci_ctrl);
	if (rc < 0) {
		LOGE("cci deinit rc = %d", rc);
		return rc;
	}
	rc = cci_ctrl.status;
	return rc;
}

static int sony_camera_qup_i2c_read(struct sony_camera_data *data,
	uint8_t slave_addr, uint32_t addr, uint8_t addr_type,
	int size, uint8_t *buf, enum sony_camera_sub_device_type device_type)
{
	int rc;
	struct i2c_client *client = data->qup_i2c_dev[device_type].client;
	uint8_t w_buf[4];
	struct i2c_msg msg[2];

	if (!data->qup_i2c_dev[device_type].enabled) {
		LOGE("camera %d qup i2c device is not enabled", data->id);
		rc = -EINVAL;
		goto end;
	}
	if (client->addr != slave_addr) {
		LOGE("no selected device found expected 0x%02x actual 0x%02x",
			client->addr, slave_addr);
		rc = -ENODEV;
		goto end;
	}
	if (!buf) {
		LOGE("no memory");
		rc = -ENOMEM;
		goto end;
	}
	if (addr_type == SONY_CAMERA_I2C_TYPE_BYTE) {
		w_buf[0] = (uint8_t)addr;
	} else if (addr_type == SONY_CAMERA_I2C_TYPE_WORD) {
		w_buf[0] = (uint8_t)(addr >> 8);
		w_buf[1] = (uint8_t)(addr & 0xFF);
	} else {
		w_buf[0] = (uint8_t)(addr >> 24);
		w_buf[1] = (uint8_t)(addr >> 16);
		w_buf[2] = (uint8_t)(addr >> 8);
		w_buf[3] = (uint8_t)(addr & 0xFF);
	}

	msg[0].addr = (client->addr) >> 1;
	msg[0].flags = 0;
	msg[0].buf = w_buf;
	msg[0].len = addr_type;

	msg[1].addr = (client->addr) >> 1;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = size;
	rc = i2c_transfer(client->adapter, msg, 2);
	if (rc != 2) {
		LOGE("i2c_transfer fail");
		rc = -EFAULT;
	}

end:
	return rc;
}

static int sony_camera_qup_i2c_write(struct sony_camera_data *data,
	uint8_t slave_addr, uint32_t addr, uint8_t addr_type,
	int size, uint8_t *buf, enum sony_camera_sub_device_type device_type)
{
	int rc;
	struct i2c_client *client = data->qup_i2c_dev[device_type].client;
	struct i2c_msg msg[2];
	uint8_t *w_buf = NULL;

	if (!data->qup_i2c_dev[device_type].enabled) {
		LOGE("camera %d qup i2c device is not enabled", data->id);
		rc = -EINVAL;
		goto end;
	}
	if (client->addr != slave_addr) {
		LOGE("no selected device found expected 0x%02x actual 0x%02x",
			client->addr, slave_addr);
		rc = -ENODEV;
		goto end;
	}
	if (!buf) {
		LOGE("no memory");
		rc = -ENOMEM;
		goto end;
	}
	memset(data->qup_i2c_dev[device_type].cache, 0, SONY_CAMERA_QUP_I2C_MAX_LEN);
	w_buf = data->qup_i2c_dev[device_type].cache;

	if (!w_buf) {
		LOGE("no memory");
		rc = -ENOMEM;
		goto end;
	}
	if (addr_type == SONY_CAMERA_I2C_TYPE_BYTE) {
		w_buf[0] = (uint8_t)addr;
	} else if (addr_type == SONY_CAMERA_I2C_TYPE_WORD) {
		w_buf[0] = (uint8_t)(addr >> 8);
		w_buf[1] = (uint8_t)(addr & 0xFF);
	} else {
		w_buf[0] = (uint8_t)(addr >> 24);
		w_buf[1] = (uint8_t)(addr >> 16);
		w_buf[2] = (uint8_t)(addr >> 8);
		w_buf[3] = (uint8_t)(addr & 0xFF);
	}
	memcpy(&w_buf[addr_type], buf, size);

	msg[0].addr = (client->addr) >> 1;
	msg[0].flags = 0;
	msg[0].buf = w_buf;
	msg[0].len = size + addr_type;
	rc = i2c_transfer(client->adapter, msg, 1);
	if (rc != 1) {
		LOGE("i2c_transfer fail");
		rc = -EFAULT;
	}

end:
	return rc;
}

static int sony_camera_cci_i2c_read(struct sony_camera_data *data,
			uint8_t slave_addr, uint32_t addr,
			uint8_t type, uint16_t len, uint8_t *buf)
{
	int rc = 0;
	int i = 0;
	int retry_cnt = SONY_CAMERA_MAX_RETRY_COUNT;
	uint8_t read_data[SONY_CAMERA_MAX_I2C_DATA];
	struct cam_cci_ctrl cci_ctrl;

	memset(&cci_ctrl, 0, sizeof(cci_ctrl));

	if (type > CAMERA_SENSOR_I2C_TYPE_MAX || len > SONY_CAMERA_MAX_I2C_DATA) {
		rc = -EINVAL;
	} else {
		cci_ctrl.cmd = MSM_CCI_I2C_READ;
		cci_ctrl.cci_info = &data->cci_dev.cci_info;
		cci_ctrl.cci_info->sid = slave_addr >> 1;
		cci_ctrl.cfg.cci_i2c_read_cfg.addr = addr;
		cci_ctrl.cfg.cci_i2c_read_cfg.addr_type = type;
		cci_ctrl.cfg.cci_i2c_read_cfg.data = read_data;
		cci_ctrl.cfg.cci_i2c_read_cfg.num_byte = len;
		while ((rc = v4l2_subdev_call(data->cci_dev.cci_info.cci_subdev,
			core, ioctl, VIDIOC_MSM_CCI_CFG, &cci_ctrl)) && retry_cnt) {
			retry_cnt--;
			usleep_range(1000, 3000);
			LOGE("Retry slave0x%04x,addr0x%04x,type0x%02x,len0x%02x",
				slave_addr, addr, type, len);
		}
		if (rc < 0) {
			LOGE("slave0x%04x,addr0x%04x,type0x%02x,len0x%02x",
				slave_addr, addr, type, len);
			LOGE("i2c read failed(%d)", rc);
			rc = -EIO;
		} else {
			rc = cci_ctrl.status;
			for (i = 0; i < len; i++)
				buf[i] = read_data[i];
		}
	}
	return rc;
}

static int sony_camera_cci_i2c_write(struct sony_camera_data *data,
	uint8_t slave_addr, uint32_t addr, uint8_t type,
	uint16_t len, uint8_t *buf)
{
	int32_t rc = -EINVAL;
	int i = 0;
	struct cam_cci_ctrl cci_ctrl;
	struct cam_sensor_i2c_reg_array* reg_settings;
	bool need_free = false;
	int retry_cnt = SONY_CAMERA_MAX_RETRY_COUNT;
	if (len > SONY_CAMERA_I2C_MAX_LEN) {
		reg_settings = vzalloc(sizeof(struct cam_sensor_i2c_reg_array) * len);
		need_free = true;
	} else {
		memset(data->cci_dev.cache, 0, SONY_CAMERA_I2C_MAX_SIZE);
		reg_settings = (struct cam_sensor_i2c_reg_array*)data->cci_dev.cache;
	}

	for (i = 0; i < len; i++) {
		reg_settings[i].reg_addr = addr + i;
		reg_settings[i].reg_data = buf[i];
		reg_settings[i].delay = 0;
		reg_settings[i].data_mask = 0;
	}
	memset(&cci_ctrl, 0, sizeof(cci_ctrl));
	cci_ctrl.cmd = MSM_CCI_I2C_WRITE_BURST;
	cci_ctrl.cci_info = &data->cci_dev.cci_info;
	cci_ctrl.cci_info->sid = slave_addr >> 1;
	cci_ctrl.cfg.cci_i2c_write_cfg.reg_setting = reg_settings;
	cci_ctrl.cfg.cci_i2c_write_cfg.data_type = 1; // TODO: CAMERA_SENSOR_I2C_TYPE_BYTE
	cci_ctrl.cfg.cci_i2c_write_cfg.addr_type = type;
	cci_ctrl.cfg.cci_i2c_write_cfg.size = len;
	while ((rc = v4l2_subdev_call(data->cci_dev.cci_info.cci_subdev,
			core, ioctl, VIDIOC_MSM_CCI_CFG, &cci_ctrl)) && retry_cnt) {
		retry_cnt--;
		usleep_range(1000, 3000);
		LOGE("Retry slave0x%04x,addr0x%04x,type0x%02x,len0x%02x",
			slave_addr, addr, type, len);
	}
	if (rc < 0)
		LOGE("i2c write failed(%d) slave0x%04x,addr0x%04x,type0x%02x,len0x%02x",
			rc, slave_addr, addr, type, len);
	if (need_free)
		vfree(reg_settings);
	return rc;
}

static int sony_camera_spi_read(
	struct sony_camera_data *camera_data, uint32_t len,
	struct sony_camera_qup_spi_device *spi_dev)
{
	struct spi_transfer rx = {
		.rx_buf		 = spi_dev->cache,
		.len		 = len,
	};
	struct spi_message m;
	int rc = 0;

	if (!spi_dev->enabled) {
		LOGE("Device is not enabled");
		rc = -EINVAL;
		goto end;
	}
	spi_message_init(&m);
	spi_message_add_tail(&rx, &m);

	rc = spi_sync(spi_dev->client, &m);
	LOGD("client %p spi_sync rx.len=%d",
		spi_dev->client, rx.len);
end:
	return rc;
}

static int sony_camera_spi_write(
	struct sony_camera_data *camera_data, uint32_t len,
	struct sony_camera_qup_spi_device *spi_dev)
{
	struct spi_transfer tx = {
		.tx_buf  = spi_dev->cache,
		.len	 = len,
	};
	struct spi_message m;
	int rc = 0;

	if (!spi_dev->enabled) {
		LOGE("Device is not enabled");
		rc = -EINVAL;
		goto end;
	}

	spi_message_init(&m);
	spi_message_add_tail(&tx, &m);

	rc = spi_sync(spi_dev->client, &m);
	LOGD("client %p spi_sync tx.len=%d",
		spi_dev->client, tx.len);
end:
	return rc;
}

static int sony_camera_spi_write_and_read(
	struct sony_camera_data *camera_data,
	uint32_t tx_len, uint32_t rx_len,
	struct sony_camera_qup_spi_device *spi_dev)
{
	struct spi_transfer rx = {
		.tx_buf		= NULL,
		.rx_buf		= NULL,
		.len		= 0,
	};
	struct spi_transfer tx = {
		.tx_buf		= NULL,
		.rx_buf		= NULL,
		.len		= 0,
	};
	struct spi_message m;
	int rc = 0;

	if (!spi_dev->enabled) {
		LOGE("Device is not enabled");
		rc = -EINVAL;
		goto end;
	}
	if ((tx_len + rx_len > spi_dev->cache_size) ||
		(tx_len > spi_dev->max_tx_size) ||
		(rx_len > spi_dev->max_rx_size)) {
		LOGE("Invalid size set for write and read");
		rc = -EINVAL;
		goto end;
	}
	spi_message_init(&m);
	if (tx_len > 0) {
		tx.tx_buf = spi_dev->cache;
		tx.len = tx_len;
		spi_message_add_tail(&tx, &m);
	}
	rx.rx_buf = &spi_dev->cache[tx_len];
	rx.len = rx_len;
	spi_message_add_tail(&rx, &m);
	rc = spi_sync(spi_dev->client, &m);
	if (rc) {
		LOGE("spi_sync failed");
		goto end;
	}
	LOGD("client %p spi_sync tx.len=%d rx.len %d",
		spi_dev->client, tx.len, rx.len);

end:
	return rc;
}

static int sony_camera_spi_read_with_status(
	struct sony_camera_data *camera_data, uint8_t *user_buf,
	uint32_t len, enum sony_camera_irq_status status,
	struct sony_camera_qup_spi_device *spi_device)
{
	int rc = 0;
	int val = 0;
	int retry = 0;
	uint32_t i = 0;
	uint32_t copy_len = 0;
	uint32_t read_len = 0;
	uint32_t spi_rx_width = 0;
	struct gpio *irq_gpio = NULL;
	int retry_cnt = SONY_CAMERA_MAX_RETRY_COUNT;

	if (spi_device && spi_device->client) {
		spi_rx_width = spi_device->client->bits_per_word / 8;
	} else {
		LOGE("spi_device is not enabled");
		rc = -ENODEV;
		goto exit;
	}
	if (status == SONY_CAMERA_IRQ_STATUS_ASSERT) {
		while (!wait_for_completion_timeout(
			&camera_data->spi_assert_complete,
			SONY_CAMERA_SPI_COMPLETION_TIMEOUT) && retry_cnt) {
			retry_cnt--;
			LOGE("spi status assert wait timeout retry_cnt %d", retry_cnt);
		}
		if (!retry_cnt) {
			LOGE("spi status assert wait failed");
			rc = -ETIMEDOUT;
			goto exit;
		}
	} else {
		while (!wait_for_completion_timeout(
			&camera_data->spi_deassert_complete,
			SONY_CAMERA_SPI_COMPLETION_TIMEOUT) && retry_cnt) {
			retry_cnt--;
			LOGE("spi status deassert wait timeout retry_cnt %d", retry_cnt);
		}
		if (!retry_cnt) {
			LOGE("spi status deassert wait failed");
			rc = -ETIMEDOUT;
			goto exit;
		}
	}

	irq_gpio = sony_camera_get_gpio_pin(camera_data,
		SONY_CAMERA_GPIO_IRQ_SPI_READY);
	val = gpio_get_value(irq_gpio->gpio);
	LOGD("spi status: expect=%d, current=%d", status, val);
	while (val != status) {
		usleep_range(200, 300);
		val = gpio_get_value(irq_gpio->gpio);
		if (++retry > 10000) {
			LOGE("wait spi status(%d) timeout", status);
			rc = -ETIMEDOUT;
			goto exit;
		}
	}
	while (i < len && !rc) {
		copy_len = ((len - i) < spi_device->max_rx_size) ?
			len % spi_device->max_rx_size : spi_device->max_rx_size;
		read_len = ((len - i) < spi_device->max_rx_size) ?
			(len % spi_device->max_rx_size + spi_rx_width - 1) & ~(spi_rx_width - 1) :
			spi_device->max_rx_size;
		rc = sony_camera_spi_read(camera_data, read_len, spi_device);
		if (rc < 0) {
			LOGE("sony_camera_spi_read failed rc:%d", rc);
			goto exit;
		}
		rc = copy_to_user((void __user *)user_buf + i, spi_device->cache,
			copy_len);
		if (rc < 0) {
			LOGE("copy to user failed rc:%d", rc);
			goto exit;
		}
		i += copy_len;
	}
exit:
	LOGI("X rc = %d", rc);
	return rc;
}

static int sony_camera_spi_write_with_status(
	struct sony_camera_data *camera_data, const uint8_t *user_buf,
	uint32_t len, enum sony_camera_irq_status status,
	struct sony_camera_qup_spi_device *spi_device)
{
	int rc = 0;
	int val = 0;
	int retry = 0;
	uint32_t i = 0;
	uint32_t copy_len = 0;
	uint32_t write_len = 0;
	uint32_t spi_tx_width = 0;
	struct gpio *irq_gpio = NULL;
	int retry_cnt = SONY_CAMERA_MAX_RETRY_COUNT;

	if (spi_device && spi_device->client) {
		spi_tx_width = spi_device->client->bits_per_word / 8;
	} else {
		LOGE("spi_device is not enabled");
		rc = -ENODEV;
		goto exit;
	}
	if (status == SONY_CAMERA_IRQ_STATUS_ASSERT) {
		while (!wait_for_completion_timeout(
			&camera_data->spi_assert_complete,
			SONY_CAMERA_SPI_COMPLETION_TIMEOUT) && retry_cnt) {
			retry_cnt--;
			LOGE("spi status assert wait timeout retry_cnt %d", retry_cnt);
		}
		if (!retry_cnt) {
			LOGE("spi status assert wait failed");
			rc = -ETIMEDOUT;
			goto exit;
		}
	} else {
		while (!wait_for_completion_timeout(
			&camera_data->spi_deassert_complete,
			SONY_CAMERA_SPI_COMPLETION_TIMEOUT) && retry_cnt) {
			retry_cnt--;
			LOGE("spi status deassert wait timeout retry_cnt %d", retry_cnt);
		}
		if (!retry_cnt) {
			LOGE("spi status deassert wait failed");
			rc = -ETIMEDOUT;
			goto exit;
		}
	}
	irq_gpio = sony_camera_get_gpio_pin(camera_data,
		SONY_CAMERA_GPIO_IRQ_SPI_READY);
	val = gpio_get_value(irq_gpio->gpio);
	LOGD("spi status: expect=%d, current=%d", status, val);
	while (val != status) {
		usleep_range(200, 300);
		val = gpio_get_value(irq_gpio->gpio);
		if (++retry > 10000) {
			LOGE("wait spi status(%d) timeout", status);
			rc = -ETIMEDOUT;
			goto exit;
		}
	}

	while (i < len && !rc) {
		copy_len = ((len - i) < spi_device->max_tx_size) ?
			len % spi_device->max_tx_size : spi_device->max_tx_size;
		write_len = ((len - i) < spi_device->max_tx_size) ?
			(len % spi_device->max_tx_size + spi_tx_width - 1) & ~(spi_tx_width - 1) :
			spi_device->max_tx_size;
		if (copy_len != write_len) {
			memset(spi_device->cache, 0, write_len);
		}
		rc = copy_from_user(spi_device->cache, (void __user *)user_buf + i,
			copy_len);
		if (rc < 0) {
			LOGE("copy from user failed rc:%d", rc);
			goto exit;
		}
		rc = sony_camera_spi_write(camera_data, write_len, spi_device);
		if (rc < 0) {
			LOGE("sony_camera_spi_write failed rc:%d", rc);
			goto exit;
		}
		i += copy_len;
	}
exit:
	return rc;
}

static int sony_camera_check_flash_status(struct sony_camera_data *camera_data)
{
	int rc = 0;
	struct sony_camera_qup_spi_device *flash_device =
		&camera_data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_FLASH];

	while (!rc){
		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_READ_STATUS;
		rc = sony_camera_spi_write_and_read(camera_data, 1, 1,
			flash_device);
		if (rc != 0) {
			LOGE("%s: flash read status failed\n", __func__);
			goto exit;
		}
		if ((0x1 & flash_device->cache[1]) == 0) {
			LOGI("Check flash status OK, rc %d\n", rc);
			break;
		}
	}
exit:
	return rc;
}

static int sony_camera_download_fw(struct sony_camera_data *data,
	const uint8_t *buf, uint32_t addr, uint32_t size)
{
	int rc = 0;
	uint32_t offset = 0;
	uint32_t write_size = 0;
	struct sony_camera_qup_spi_device *flash_device =
		&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_FLASH];

	if (!flash_device->enabled) {
		LOGE("Device is not enabled");
		rc = -ENODEV;
		goto exit;
	}
	if (addr % SONY_CAMERA_SPI_FLASH_SECTOR_SIZE) {
		LOGE("invalid flash sector start address");
		goto exit;
	}

	if (addr != 0xFFFFFFFF) {
		uint32_t i = 0;
		uint32_t sector_count = (size + SONY_CAMERA_SPI_FLASH_SECTOR_SIZE - 1) /
			SONY_CAMERA_SPI_FLASH_SECTOR_SIZE;
		for (i = 0; i < sector_count; i++) {
			flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_WRITE_ENABLE;
			rc = sony_camera_spi_write(data, 1, flash_device);
			if (rc != 0) {
				LOGE("%s: flash write enable failed\n", __func__);
				goto exit;
			}
			LOGD("WRITE ENABLE rc %d\n", rc);

			flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_SECTOR_ERASE;
			flash_device->cache[1] = 0xff & ((addr + i * SONY_CAMERA_SPI_FLASH_SECTOR_SIZE) >> 16);
			flash_device->cache[2] = 0xff & ((addr + i * SONY_CAMERA_SPI_FLASH_SECTOR_SIZE) >> 8);
			flash_device->cache[3] = 0xff & (addr + i * SONY_CAMERA_SPI_FLASH_SECTOR_SIZE);
			rc = sony_camera_spi_write(data, 4, flash_device);
			if (rc != 0) {
				LOGE("flash sector erase failed");
				goto exit;
			}
			rc = sony_camera_check_flash_status(data);
			if (rc != 0)
				goto exit;
		}
		LOGD("SECTOR ERASE rc %d", rc);
	} else {
		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_WRITE_ENABLE;
		rc = sony_camera_spi_write(data, 1, flash_device);
		if (rc != 0) {
			LOGE("%s: flash write enable failed\n", __func__);
			goto exit;
		}
		LOGD("WRITE ENABLE rc %d\n", rc);

		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_BULK_ERASE;
		rc = sony_camera_spi_write(data, 1, flash_device);
		if (rc != 0) {
			LOGE("flash bulk erase failed");
			goto exit;
		}
		rc = sony_camera_check_flash_status(data);
		if (rc != 0)
			goto exit;
		LOGD("BULK ERASE rc %d\n", rc);
	}

	while (write_size < size) {
		uint32_t len = 0;
		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_WRITE_ENABLE;
		rc = sony_camera_spi_write(data, 1, flash_device);
		if (rc != 0) {
			LOGE("flash write enable failed");
			goto exit;
		}
		LOGD("WRITE ENABLE rc %d",rc);

		len = ((size - write_size) < SONY_CAMERA_SPI_FLASH_PP_MAX_LEN) ?
			size % SONY_CAMERA_SPI_FLASH_PP_MAX_LEN : SONY_CAMERA_SPI_FLASH_PP_MAX_LEN;
		offset = addr + write_size;

		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_PAGE_PROGRAM;
		flash_device->cache[1] = 0xff & (offset >> 16);
		flash_device->cache[2] = 0xff & (offset >> 8);
		flash_device->cache[3] = 0xff & offset;
		memcpy(&flash_device->cache[4], &buf[write_size], len);
		rc = sony_camera_spi_write(data, len + 4, flash_device);
		if (rc != 0) {
			LOGE("flash page program failed");
			break;
		}
		LOGD("PAGE PROGRAM addr 0x%06X size 0x%x rc %d", (0xFFFFFF & offset), len, rc);

		write_size += len;
		rc = sony_camera_check_flash_status(data);
		if (rc != 0)
			goto exit;
	}

	if (g_sony_camera_debug_flag & SONY_CAMERA_HEX_DUMP) {
		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_READ_ID;
		rc = sony_camera_spi_write_and_read(data, 1,
			3, flash_device);
		if (rc != 0) {
			LOGE("flash read id failed");
			goto exit;
		}
		LOGD("Read nor flash ID %0X%0X%0X rc %d", flash_device->cache[1],
			flash_device->cache[2], flash_device->cache[3], rc);
		msleep(10);

		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_READ_DATA;
		flash_device->cache[1] = 0xff & (addr >> 16);
		flash_device->cache[2] = 0xff & (addr >> 8);
		flash_device->cache[3] = 0xff & addr;
		rc = sony_camera_spi_write_and_read(data, 4,
			SONY_CAMERA_HEX_DUMP_LINE_BYTES * 2,
			flash_device);
		if (rc != 0) {
			LOGE("flash read data failed");
			goto exit;
		}
		sony_camera_hex_dump(&flash_device->cache[4],
			SONY_CAMERA_HEX_DUMP_LINE_BYTES * 2);
		msleep(10);

		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_READ_DATA;
		flash_device->cache[1] = 0xff & ((addr + size - SONY_CAMERA_HEX_DUMP_LINE_BYTES * 4) >> 16);
		flash_device->cache[2] = 0xff & ((addr + size - SONY_CAMERA_HEX_DUMP_LINE_BYTES * 4) >> 8);
		flash_device->cache[3] = 0xff & (addr + size - SONY_CAMERA_HEX_DUMP_LINE_BYTES * 4);
		rc = sony_camera_spi_write_and_read(data, 4,
			SONY_CAMERA_HEX_DUMP_LINE_BYTES * 4,
			flash_device);
		if (rc != 0) {
			LOGE("flash read data failed");
			goto exit;
		}
		sony_camera_hex_dump(&flash_device->cache[4],
			SONY_CAMERA_HEX_DUMP_LINE_BYTES * 4);
	}

exit:
	return rc;
}

static int sony_camera_read_flash_memory(struct sony_camera_data *data,
	uint8_t *user_buf, uint32_t addr, uint32_t size)
{
	int rc = 0;
	uint32_t offset = 0;
	uint32_t read_size = 0;
	struct sony_camera_qup_spi_device *flash_device =
		&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_FLASH];

	if (!flash_device->enabled) {
		LOGE("Device is not enabled");
		rc = -ENODEV;
		goto exit;
	}
	if ((addr + size) > SONY_CAMERA_SPI_FLASH_MAX_SIZE) {
		LOGE("%s: invalid flash address 0x%08X or size %d.\n", __func__, addr, size);
		goto exit;
	}
	rc = sony_camera_check_flash_status(data);
	if (rc != 0)
		goto exit;
	while (read_size < size) {
		uint32_t len = 0;
		len = ((size - read_size) < SONY_CAMERA_SPI_FLASH_PP_MAX_LEN) ?
			size % SONY_CAMERA_SPI_FLASH_PP_MAX_LEN : SONY_CAMERA_SPI_FLASH_PP_MAX_LEN;
		offset = addr + read_size;

		flash_device->cache[0] = SONY_CAMERA_SPI_FLASH_READ_DATA;
		flash_device->cache[1] = 0xff & (offset >> 16);
		flash_device->cache[2] = 0xff & (offset >> 8);
		flash_device->cache[3] = 0xff & offset;
		rc = sony_camera_spi_write_and_read(data, 4,
			len, flash_device);
		if (rc != 0) {
			LOGE("%s: flash read data failed\n", __func__);
			break;
		}
		rc = copy_to_user((void __user *)user_buf + read_size, &flash_device->cache[4], len);
		if (rc < 0) {
			LOGE("copy to user failed rc:%d", rc);
			goto exit;
		}
		LOGD("READ addr 0x%06X size 0x%x rc %d\n", (0xFFFFFF & offset), len, rc);
		read_size += len;
		rc = sony_camera_check_flash_status(data);
		if (rc != 0)
			goto exit;
	}

exit:
	return rc;
}

static int sony_camera_spi_set(struct sony_camera_data *data,
	int value)
{
	int rc = 0;
	uint32_t i = 0;

	if (data->probe_done) {
		for (i = 0; i < SONY_CAMERA_SUB_DEVICE_TYPE_MAX; i++) {
			if (data->qup_spi_dev[i].enabled) {
				if (value >= 0) {
					rc = pm_runtime_get_sync(data->qup_spi_dev[i].client->master->dev.parent);
					if (rc < 0)
						LOGE("device_type %d spi_active failed rc %d", i, rc);
						break;
				} else {
					rc = pm_runtime_put_sync(data->qup_spi_dev[i].client->master->dev.parent);
					if (rc < 0)
						LOGE("device_type %d spi_suspend failed rc %d", i, rc);
					break;
				}
			}
		}
	}

	return rc;
}

static int sony_camera_wait_idle(
	struct sony_camera_data *camera_data,
	enum sony_camera_irq_status status)
{
	int rc = 0;
	int val = 0;
	int retry = 0;
	struct gpio *irq_gpio = NULL;
	int retry_cnt = SONY_CAMERA_MAX_IDLE_RETRY_COUNT;

	if (!camera_data->has_idle) {
		LOGI("idle port is not implemented. no need to wait");
		goto exit;
	}
	irq_gpio = sony_camera_get_gpio_pin(camera_data,
		SONY_CAMERA_GPIO_IRQ_IDLE);
	if (status == SONY_CAMERA_IRQ_STATUS_ASSERT) {
		while (status != gpio_get_value(irq_gpio->gpio) &&
			!wait_for_completion_timeout(
				&camera_data->idle_assert_complete,
				SONY_CAMERA_IDLE_COMPLETION_TIMEOUT) && retry_cnt) {
			retry_cnt--;
			val = gpio_get_value(irq_gpio->gpio);
			LOGI("idle status assert wait timeout retry_cnt %d val %d", retry_cnt, val);
		}
		if (!retry_cnt) {
			LOGE("idle status assert wait failed");
			rc = -ETIMEDOUT;
			goto exit;
		}
	} else {
		while (status != gpio_get_value(irq_gpio->gpio) &&
			!wait_for_completion_timeout(
				&camera_data->idle_deassert_complete,
				SONY_CAMERA_IDLE_COMPLETION_TIMEOUT) && retry_cnt) {
			retry_cnt--;
			LOGI("idle status deassert wait timeout retry_cnt %d", retry_cnt);
		}
		if (!retry_cnt) {
			LOGE("idle status deassert wait failed");
			rc = -ETIMEDOUT;
			goto exit;
		}
	}

	val = gpio_get_value(irq_gpio->gpio);
	LOGD("idle status: expect=%d, current=%d", status, val);
	while (val != status) {
		usleep_range(200, 300);
		val = gpio_get_value(irq_gpio->gpio);
		if (++retry > 10000) {
			LOGE("wait idle status(%d) timeout", status);
			rc = -ETIMEDOUT;
			goto exit;
		}
	}
exit:
	return rc;
}

static int sony_camera_sof_event_ctrl(struct sony_camera_data *data,
	uint8_t ctrl)
{
	int rc = 0;

	if (ctrl == 1) {
		if (data->has_hw_sof) {
			rc = sony_camera_regist_gpio_irq(data,
				sony_camera_irq_handler,
				data->sof_irq_trigger_type | IRQF_ONESHOT,
				SONY_CAMERA_GPIO_IRQ_SOF);
			if (rc < 0)
				LOGE("regist gpio irq: %s failed",
					SONY_CAMERA_GPIO_IRQ_SOF);
		}
	} else if (ctrl == 0) {
		if (data->has_hw_sof) {
			rc = sony_camera_unregist_gpio_irq(data,
				SONY_CAMERA_GPIO_IRQ_SOF);
			if (rc < 0)
				LOGE("unregist gpio irq: %s failed",
					SONY_CAMERA_GPIO_IRQ_SOF);
		}
	}
	return rc;
}

static int sony_camera_power_ctrl(struct sony_camera_data *data,
	uint8_t on)
{
	int rc = 0;
	const struct sony_camera_module *mod = data->module;
	const struct sony_camera_seq *seq = on ? mod->seq_on : mod->seq_off;
	struct gpio *gpio_reset = NULL;

	while (seq->cmd != EXIT) {
		struct sony_camera_regulator_t *regulator_ptr = NULL;
		uint8_t iodt = 0x00;
		LOGI("cmd %d val1 %d val2 %d wait %d",
			seq->cmd, seq->val1, seq->val2, seq->wait);
		switch (seq->cmd) {
		case SONY_GPIO_RESET:
			gpio_reset = sony_camera_get_gpio_pin(data,
				SONY_CAMERA_GPIO_RESET);
			if (gpio_reset->gpio <= 0) {
				rc = -EPERM;
				break;
			}
			LOGD("reset gpio %d->%d", gpio_reset->gpio, seq->val1);
			rc = sony_camera_gpio_set(data, gpio_reset->gpio, seq->val1);
			break;
		case SONY_GPIO_RESET2:
			gpio_reset = sony_camera_get_gpio_pin(data,
				SONY_CAMERA_GPIO_RESET2);
			if (gpio_reset->gpio <= 0) {
				rc = -EPERM;
				break;
			}
			LOGD("reset2 gpio %d->%d", gpio_reset->gpio, seq->val1);
			rc = sony_camera_gpio_set(data, gpio_reset->gpio, seq->val1);
			break;
		case SONY_GPIO_VANA:
		{
			struct gpio *gpio_vana = sony_camera_get_gpio_pin(data,
				SONY_CAMERA_GPIO_VANA);
			if (!gpio_vana) {
				rc = -EPERM;
				break;
			}
			LOGD("vana gpio %d->%d", gpio_vana->gpio, seq->val1);
			rc = sony_camera_gpio_set(data, gpio_vana->gpio, seq->val1);
			break;
		}
		case SONY_CAM_VDIG:
		case SONY_CAM_VIO:
		case SONY_CAM_VANA:
		case SONY_CAM_VAF:
		case SONY_CAM_VAF2:
		case SONY_CAM_OIS:
		case SONY_CAM_OIS2:
		case SONY_CAM_VMDR:
		case SONY_CAM_GYRO:
		case SONY_CAM_VDIG2:
		case SONY_CAM_VDIG3:
		case SONY_CAM_VDIG4:
		case SONY_CAM_VANA2:
		case SONY_CAM_CLK2:
		case SONY_CAM_CLK3:
		case SONY_PMIC_CS0:
		case SONY_PMIC_CS1:
		case SONY_PMIC_VDD0:
		case SONY_PMIC_VDD1:
		case SONY_CAM_ISP1:
		case SONY_CAM_ISP2:
		case SONY_CAM_ISP3:
		case SONY_CAM_ISP4:
		case SONY_CAM_ISP5:
		case SONY_CAM_EXTIO:
			regulator_ptr = sony_camera_vreg_get(data, seq->cmd, seq->val1);
			if (!regulator_ptr) {
				LOGE("sony_camera_vreg_get failed cmd %d val %d", seq->cmd, seq->val1);
				rc = -ENODEV;
				goto exit;
			}
			rc = sony_camera_vreg_set(regulator_ptr, seq->val1, seq->val2);
			if (rc < 0) {
				LOGE("sony_camera_vreg_set failed cmd %d val1 %d val2 %d",
					seq->cmd, seq->val1, seq->val2);
				goto exit;
			}
			break;
		case SONY_SPI_SET:
			rc = sony_camera_spi_set(data,seq->val1);
			break;
		case SONY_CAM_CLK:
			regulator_ptr = sony_camera_vreg_get(data, seq->cmd, on ? 0 : -1);
			if (!regulator_ptr) {
				LOGE("mclk vreg get failed %d", rc);
				rc = -ENODEV;
				goto exit;
			}
			rc = sony_camera_vreg_set(regulator_ptr, on ? 0 : -1, seq->val2);
			if (rc < 0) {
				LOGE("mclk vreg set failed %d", rc);
			} else {
				rc = sony_camera_mclk_set(data, seq->val1);
			}
			break;
		case SONY_PMIC_VIN:
			regulator_ptr = sony_camera_vreg_get(data, seq->cmd, 0);
			if (!regulator_ptr) {
				LOGE("sony_camera_vreg_get failed cmd %d", seq->cmd);
				rc = -ENODEV;
				goto exit;
			}
			rc = regulator_set_voltage(regulator_ptr->vreg, seq->val1 * 1000, seq->val2 * 1000);
			if (rc < 0) {
				LOGE("pmic vin voltage set failed");
				goto exit;
			}
			if (on) {
				rc = regulator_enable(regulator_ptr->vreg);
				if (rc < 0) {
					LOGE("pmic vin enable failed %d");
					goto exit;
				}
			} else {
				(void)regulator_disable(regulator_ptr->vreg);
			}
			break;
		case SONY_MIPI_SWITCH:
			gpio_reset = sony_camera_get_gpio_pin(data,
				SONY_CAMERA_MIPI_SWITCH);
			if (gpio_reset->gpio <= 0) {
				rc = -EPERM;
				break;
			}
			LOGD("reset gpio %d->%d", gpio_reset->gpio, seq->val1);
			rc = sony_camera_gpio_set(data, gpio_reset->gpio, seq->val1);
			break;
		case SONY_I2C_WRITE:
			rc = sony_camera_cci_i2c_write(data,
				data->match_id.slave_addr, seq->val1, 2, 1, &iodt);
			break;
		case SONY_PROBE_WAIT:
		case SONY_PROBE_WAIT2:
			if (!data->probe_done)
				usleep_range(seq->val1, seq->val1);
			break;
		default:
			goto exit;
		}
		usleep_range(seq->wait, seq->wait);
		if (rc < 0 && on)
			goto exit;
		seq++;
	}
exit:
	return rc;
}

static int sony_camera_gpio_reset(struct sony_camera_data *data,
	uint8_t on)
{
	int rc = 0;
	const struct sony_camera_module *mod = data->module;
	const struct sony_camera_seq *seq = on ? mod->seq_on : mod->seq_off;
	struct gpio *gpio_reset = NULL;

	while (seq->cmd != EXIT) {
		LOGD("cmd %d val1 %d val2 %d wait %d",
			seq->cmd, seq->val1, seq->val2, seq->wait);
		switch (seq->cmd) {
		case SONY_GPIO_RESET:
			gpio_reset = sony_camera_get_gpio_pin(data,
				SONY_CAMERA_GPIO_RESET);
			if (gpio_reset->gpio <= 0) {
				rc = -EPERM;
				goto exit;
			}
			LOGD("reset gpio %d->%d", gpio_reset->gpio, seq->val1);
			rc = sony_camera_gpio_set(data, gpio_reset->gpio, seq->val1);
			usleep_range(seq->wait, seq->wait);
			goto exit;
		default:
			break;
		}
		seq++;
	}
exit:
	return rc;
}

static int sony_camera_task_handler(void *data)
{
	int rc = 0;
	int irq_type = SONY_CAMERA_IRQ_EVENT_MAX;
	struct sony_camera_task_queue_cmd *queue_cmd = NULL;
	unsigned long flags;
	struct sony_camera_data *camera_data = NULL;
	struct task_struct *tsk;
	struct sched_param PARAM = { .sched_priority = MAX_RT_PRIO };

	tsk = current;
	sched_setscheduler(tsk, SCHED_FIFO, &PARAM);

	if (data == NULL)
		return -EINVAL;

	camera_data = (struct sony_camera_data *)data;
	while (!kthread_should_stop()) {
		wait_event_interruptible(camera_data->wait_q,
			sony_camera_check_irq(camera_data) ||
			kthread_should_stop());
		while (atomic_read(&camera_data->irq_cnt)) {
			spin_lock_irqsave(&camera_data->task_lock, flags);
			queue_cmd = list_first_entry(&camera_data->task_q,
				struct sony_camera_task_queue_cmd, list);
			if (!queue_cmd) {
				atomic_set(&camera_data->irq_cnt, 0);
				spin_unlock_irqrestore(&camera_data->task_lock,
					flags);
				break;
			}
			irq_type = queue_cmd->irq_type;
			atomic_sub(1, &camera_data->irq_cnt);
			list_del(&queue_cmd->list);
			queue_cmd->cmd_used = false;
			spin_unlock_irqrestore(&camera_data->task_lock, flags);
			LOGI("Handle %d in Task Handler", irq_type);
			switch (irq_type) {
			case SONY_CAMERA_IRQ_SPI_READY_ASSERT_EVENT:
				complete(&camera_data->spi_assert_complete);
				reinit_completion(&camera_data->spi_deassert_complete);
				break;
			case SONY_CAMERA_IRQ_SPI_READY_DEASSERT_EVENT:
				complete(&camera_data->spi_deassert_complete);
				reinit_completion(&camera_data->spi_assert_complete);
				break;
			case SONY_CAMERA_IRQ_IDLE_ASSERT_EVENT:
				complete(&camera_data->idle_assert_complete);
				reinit_completion(&camera_data->idle_deassert_complete);
				break;
			case SONY_CAMERA_IRQ_IDLE_DEASSERT_EVENT:
				complete(&camera_data->idle_deassert_complete);
				reinit_completion(&camera_data->idle_assert_complete);
				break;
			default:
				break;
			}
		}
	}
	return rc;
}

void sony_camera_flush_task_queue(struct sony_camera_data *camera_data)
{
	unsigned long flags;
	struct sony_camera_task_queue_cmd *queue_cmd = NULL;

	spin_lock_irqsave(&camera_data->task_lock, flags);
	while (atomic_read(&camera_data->irq_cnt)) {
		queue_cmd = list_first_entry(&camera_data->task_q,
			struct sony_camera_task_queue_cmd, list);
		if (!queue_cmd) {
			atomic_set(&camera_data->irq_cnt, 0);
			break;
		}
		atomic_sub(1, &camera_data->irq_cnt);
		list_del(&queue_cmd->list);
		queue_cmd->cmd_used = false;
	}
	spin_unlock_irqrestore(&camera_data->task_lock, flags);
}

static int sony_camera_thermal_get_temp(
	struct thermal_zone_device *thermal, int *temp)
{
	int rc = 0;
	int id = 0;
	int thermal_id = 0;
	uint8_t i = 0;
	uint8_t max_cmb_tbl_num = 0;

	if (!temp) {
		LOGE("invalid arg");
		rc = -EPERM;
		goto error;
	}

	max_cmb_tbl_num = sizeof(driver_comb_tbl) / sizeof(driver_comb_tbl[0]);
	for (i = 0; i < max_cmb_tbl_num; i++) {
		if (!strncmp(thermal->type, driver_comb_tbl[i].thermal_name, sizeof(thermal->type))) {
			id = driver_comb_tbl[i].id;
			thermal_id = driver_comb_tbl[i].thermal_zone;
			break;
		}
	}
	if (i == max_cmb_tbl_num) {
		rc = -EPERM;
		goto error;
	}
	mutex_lock(&camera_data[id].thermal_lock);
	if (camera_data[id].thermal_ret_val < 0)
		rc = camera_data[id].thermal_ret_val;
	else
		*temp = (int)camera_data[id].thermal_sensor_temperature[thermal_id];
	LOGD("id = %d rc = %d *temp = %d", id, rc, *temp);
	mutex_unlock(&camera_data[id].thermal_lock);

error:
	return rc;
}

static struct thermal_zone_device_ops sony_camera_thermal_ops =
{
	.get_temp = sony_camera_thermal_get_temp,
};

static int sony_camera_clear(struct sony_camera_data *data)
{
	int rc = 0;
	unsigned long flags;
	spin_lock_irqsave(&camera_data->event_list_lock, flags);
	while (!list_empty(&data->event_available)) {
		struct sony_camera_event_list_data *event = list_entry(
			data->event_available.next,
			struct sony_camera_event_list_data, list);

		list_del(&event->list);
		kfree(event);
	}
	spin_unlock_irqrestore(&camera_data->event_list_lock, flags);
	spin_lock_irqsave(&camera_data->sof_lock, flags);
	data->sof_count = 0;
	spin_unlock_irqrestore(&camera_data->sof_lock, flags);

	return rc;
}

static int sony_camera_power_down(struct sony_camera_data *data)
{
	int rc = 0;
	uint16_t cmd_idx = 0;

	LOGD("power down camera %d state %d", data->id, data->state);
	if (data->state == SONY_CAMERA_STATE_POWER_DOWN) {
	    goto exit;
	}
	mutex_lock(&data->state_lock);
	data->state = SONY_CAMERA_STATE_POWER_DOWN;
	mutex_unlock(&data->state_lock);

	for (cmd_idx = 0; cmd_idx < EXIT; cmd_idx++) {
		struct sony_camera_regulator_t *regulator_ptr = NULL;
		regulator_ptr = sony_camera_vreg_get(data, cmd_idx, -1);
		if (!regulator_ptr) {
			continue;
		}
		if (regulator_ptr->enabled_by_user) {
			rc = sony_camera_vreg_set(regulator_ptr, -1, 0);
			regulator_ptr->enabled_by_user = false;
		}
	}

	rc = sony_camera_power_ctrl(data, false);
	if (rc < 0)
		LOGE("power_down fail");

	if (data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP].enabled ||
		data->has_hw_sof || data->has_hw_ext_irq || data->has_idle) {
		rc = sony_camera_irq_deinit(data);
		if (rc < 0)
			LOGE("irq_deinit failed");
	}

	if (data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP].enabled && data->sensor_task) {
		rc = kthread_stop(data->sensor_task);
		if (rc < 0)
			LOGE("kthread_stop failed %d", rc);
		sony_camera_flush_task_queue(data);
	}
	if (data->cci_dev.cache) {
		kfree(data->cci_dev.cache);
		if (!data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].enabled) {
			rc = sony_camera_cci_deinit(data);
		}
	}
	if (rc < 0)
		LOGE("cci_deinit failed");

	rc = pinctrl_select_state(data->pinctrl,
		data->gpio_state_suspend);
	if (rc)
		pr_err("%s:%d cannot set pin to suspend state",
			__func__, __LINE__);

	rc = sony_camera_gpio_deinit(data);
	if (rc < 0)
		LOGE("gpio_deinit failed");
	mutex_lock(&data->thermal_lock);
	data->thermal_ret_val = -ENODEV;
	mutex_unlock(&data->thermal_lock);
	sony_camera_clear(data);

exit:

	return rc;
}

static int sony_camera_power_up(struct sony_camera_data *data)
{
	int rc = 0;
	uint8_t retry_cnt = SONY_CAMERA_MAX_RETRY_COUNT;

	LOGD("power up camera %d state %d", data->id, data->state);
	if (data->state == SONY_CAMERA_STATE_POWER_UP) {
	    goto exit;
	}

	rc = sony_camera_gpio_init(data);
	if (rc < 0) {
		LOGE("gpio_init failed");
		sony_camera_gpio_deinit(data);
		goto exit;
	}

	rc = pinctrl_select_state(data->pinctrl,
		data->gpio_state_active);
	if (rc) {
		LOGE("cannot set pin to active state");
		sony_camera_gpio_deinit(data);
		pinctrl_select_state(data->pinctrl,
			data->gpio_state_suspend);
		goto exit;
	}

	LOGI("id = %d, has_hw_sof %d use_spi %d %s",
		data->id, data->has_hw_sof, data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP].enabled,
		dev_name(&data->p_dev->dev));
	if (data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP].enabled) {
		data->sensor_task = kthread_run(sony_camera_task_handler,
			data, "sony_camera_task");
		if (IS_ERR(data->sensor_task)) {
			LOGE("failed to run kernel thread, rc = %d", rc);
			rc = PTR_ERR(data->sensor_task);
			goto exit;
		}
		reinit_completion(&data->spi_assert_complete);
		reinit_completion(&data->spi_deassert_complete);
		reinit_completion(&data->idle_assert_complete);
		reinit_completion(&data->idle_deassert_complete);
	}
	if (data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP].enabled || data->has_hw_sof ||
		data->has_hw_ext_irq || data->has_idle) {
		rc = sony_camera_irq_init(data);
		if (rc < 0) {
			LOGE("irq_init failed");
			goto exit;
		}
	}
	if (!data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].enabled) {
		rc = sony_camera_cci_init(data);
		if (rc < 0) {
			LOGE("cci_init failed");
			sony_camera_gpio_deinit(data);
			pinctrl_select_state(data->pinctrl,
				data->gpio_state_suspend);
			sony_camera_cci_deinit(data);
			goto exit;
		}
	}

	retry_cnt = SONY_CAMERA_MAX_RETRY_COUNT;
	while (!(data->cci_dev.cache = kzalloc(SONY_CAMERA_I2C_MAX_SIZE, GFP_KERNEL)) &&
			retry_cnt) {
		retry_cnt--;
		usleep_range(1000, 3000);
		LOGE("Retry kmalloc cci cache");
	}
	if (!data->cci_dev.cache) {
		LOGE("cci cache kmalloc failed");
		return -ENOMEM;
	}
	rc = sony_camera_power_ctrl(data, true);
	if (rc < 0) {
		LOGE("power_up fail");
		sony_camera_power_down(data);
		goto exit;
	}
	mutex_lock(&data->state_lock);
	data->state = SONY_CAMERA_STATE_POWER_UP;
	mutex_unlock(&data->state_lock);
	mutex_lock(&data->thermal_lock);
	data->thermal_ret_val = -EINVAL;
	mutex_unlock(&data->thermal_lock);

exit:

	return rc;
}

static int sony_camera_open(struct inode* inode, struct file* file)
{
	int rc = 0;
	unsigned int id = iminor(inode);
	file->private_data = &camera_data[id];

	mutex_lock(&camera_data[id].driver_lock);

	if (camera_data[id].open_count == 0) {
		device_init_wakeup(&camera_data[id].p_dev->dev, 1);
		pm_stay_awake(&camera_data[id].p_dev->dev);
	}
	camera_data[id].open_count++;

	mutex_unlock(&camera_data[id].driver_lock);

	LOGI("sensor opened %d", id);
	return rc;
}

static int sony_camera_close(struct inode* inode, struct file* file)
{
	int rc = 0;
	struct sony_camera_data* data = (struct sony_camera_data*)file->private_data;

	mutex_lock(&data->driver_lock);

	if (0 < data->open_count) {
		data->open_count--;
	}
	if (data->open_count == 0) {
		sony_camera_power_down(data);
		pm_relax(&data->p_dev->dev);
		device_init_wakeup(&data->p_dev->dev, 0);
	}

	mutex_unlock(&data->driver_lock);

	LOGI("sensor closed");
	return rc;
}

static unsigned int sony_camera_poll(struct file *file,
	struct poll_table_struct *wait)
{
	unsigned int rc = 0;
	unsigned long flags;
	int empty = 0;
	struct sony_camera_data* data = (struct sony_camera_data *)file->private_data;

	spin_lock_irqsave(&data->event_list_lock, flags);
	empty = list_empty(&data->event_available);
	spin_unlock_irqrestore(&data->event_list_lock, flags);
	if (empty) {
		poll_wait(file, &data->event_wait_q, wait);
	}
	spin_lock_irqsave(&data->event_list_lock, flags);
	empty = list_empty(&data->event_available);
	spin_unlock_irqrestore(&data->event_list_lock, flags);
	if (!empty) {
		rc = POLLPRI;
	}
	return rc;
}

static long sony_camera_ioctl_common(struct file *file,
	unsigned int cmd, unsigned long parm, bool compat)
{
	int rc = 0;
	unsigned long flags;
	struct sony_camera_data *data = (struct sony_camera_data *)file->private_data;
	struct sony_camera_i2c_data setting;
	uint16_t i = 0;

	if (!data->probe_done) {
		LOGE("camera %d probe not done", data->id);
		rc = -ENODEV;
		goto exit;
	}

	switch (cmd) {
	case SONY_CAMERA_CMD_POWER_UP:
		mutex_lock(&data->driver_lock);
		if (!data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].enabled) {
			mutex_lock(&data->cci_dev.lock);
		}
		for (i = 0; i < SONY_CAMERA_SUB_DEVICE_TYPE_MAX; i++) {
			if (data->qup_i2c_dev[i].enabled)
				mutex_lock(&data->qup_i2c_dev[i].lock);
			if (data->qup_spi_dev[i].enabled)
				mutex_lock(&data->qup_spi_dev[i].lock);
		}
		LOGI("sensor power up %d", data->id);
		rc = sony_camera_power_up(data);
		for (i = 0; i < SONY_CAMERA_SUB_DEVICE_TYPE_MAX; i++) {
			if (data->qup_i2c_dev[i].enabled)
				mutex_unlock(&data->qup_i2c_dev[i].lock);
			if (data->qup_spi_dev[i].enabled)
				mutex_unlock(&data->qup_spi_dev[i].lock);
		}
		if (!data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].enabled) {
			mutex_unlock(&data->cci_dev.lock);
		}
		mutex_unlock(&data->driver_lock);
		break;
	case SONY_CAMERA_CMD_POWER_DOWN:
		mutex_lock(&data->driver_lock);
		if (!data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].enabled) {
			mutex_lock(&data->cci_dev.lock);
		}
		for (i = 0; i < SONY_CAMERA_SUB_DEVICE_TYPE_MAX; i++) {
			if (data->qup_i2c_dev[i].enabled)
				mutex_lock(&data->qup_i2c_dev[i].lock);
			if (data->qup_spi_dev[i].enabled)
				mutex_lock(&data->qup_spi_dev[i].lock);
		}
		LOGI("sensor power down %d", data->id);
		rc = sony_camera_power_down(data);
		for (i = 0; i < SONY_CAMERA_SUB_DEVICE_TYPE_MAX; i++) {
			if (data->qup_i2c_dev[i].enabled)
				mutex_unlock(&data->qup_i2c_dev[i].lock);
			if (data->qup_spi_dev[i].enabled)
				mutex_unlock(&data->qup_spi_dev[i].lock);
		}
		if (!data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].enabled) {
			mutex_unlock(&data->cci_dev.lock);
		}
		mutex_unlock(&data->driver_lock);
		break;
	case SONY_CAMERA_CMD_I2C_READ:
		mutex_lock(&data->cci_dev.lock);
		LOGD("sensor I2C read id=%d", data->id);
		rc = copy_from_user(&setting,
			(void __user *)parm, sizeof(setting));
		if (rc < 0) {
			LOGE("copy_from_user failed");
			mutex_unlock(&data->cci_dev.lock);
			goto exit;
		}
		if (data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].client) {
			rc = sony_camera_qup_i2c_read(data,
				setting.slave_addr, setting.addr, setting.addr_type,
				setting.len, setting.data,
				SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR);
		} else {
			rc = sony_camera_cci_i2c_read(data,
				setting.slave_addr, setting.addr, setting.addr_type,
				setting.len, setting.data);
		}
		if (rc < 0) {
			LOGE("sony_camera_cci_i2c_read failed");
			mutex_unlock(&data->cci_dev.lock);
			goto exit;
		}
		rc = copy_to_user((void __user *)parm,
			&setting, sizeof(setting));
		if (rc < 0)
			LOGE("copy_to_user failed");
		mutex_unlock(&data->cci_dev.lock);
		break;
	case SONY_CAMERA_CMD_I2C_WRITE:
		mutex_lock(&data->cci_dev.lock);
		LOGD("sensor I2C write id=%d", data->id);
		rc = copy_from_user(&setting,
			(void __user *)parm, sizeof(setting));
		if (rc < 0) {
			LOGE("copy_from_user failed");
			mutex_unlock(&data->cci_dev.lock);
			goto exit;
		}
		LOGD("sensor I2C write slave_addr=%x addr=%x len=%d",
			setting.slave_addr, setting.addr, setting.len);
		if (data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].client) {
			rc = sony_camera_qup_i2c_write(data,
				setting.slave_addr, setting.addr, setting.addr_type,
				setting.len, setting.data,
				SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR);
		} else {
			rc = sony_camera_cci_i2c_write(data,
				setting.slave_addr, setting.addr, setting.addr_type,
				setting.len, setting.data);
		}
		mutex_unlock(&data->cci_dev.lock);
		break;
	case SONY_CAMERA_CMD_I2C_READ_2:
	{
		struct sony_camera_qup_i2c_device *i2c_device = &data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_OIS];
		mutex_lock(&i2c_device->lock);
		LOGD("sensor qup I2C read id=%d", data->id);
		rc = copy_from_user(&setting,
			(void __user *)parm, sizeof(setting));
		if (rc < 0) {
			LOGE("%s: copy_from_user failed", __func__);
			mutex_unlock(&i2c_device->lock);
			goto exit;
		}
		rc = sony_camera_qup_i2c_read(data,
			setting.slave_addr, setting.addr, setting.addr_type,
			setting.len, setting.data,
			SONY_CAMERA_SUB_DEVICE_TYPE_OIS);
		if (rc < 0) {
			LOGE("sony_camera_cci_i2c_read failed");
			mutex_unlock(&i2c_device->lock);
			goto exit;
		}
		rc = copy_to_user((void __user *)parm,
			&setting, sizeof(setting));
		if (rc < 0)
			LOGE("copy_to_user failed");
		mutex_unlock(&i2c_device->lock);
		break;
	}
	case SONY_CAMERA_CMD_I2C_WRITE_2:
	{
		struct sony_camera_qup_i2c_device *i2c_device = &data->qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_OIS];
		mutex_lock(&i2c_device->lock);
		LOGD("sensor qup I2C write id=%d", data->id);
		rc = copy_from_user(&setting,
			(void __user *)parm, sizeof(setting));
		if (rc < 0) {
			LOGE("copy_from_user failed");
			mutex_unlock(&i2c_device->lock);
			goto exit;
		}
		LOGD("sensor I2C write slave_addr=%x addr=%x len=%d",
			setting.slave_addr, setting.addr, setting.len);
		rc = sony_camera_qup_i2c_write(data,
			setting.slave_addr, setting.addr, setting.addr_type,
			setting.len, setting.data,
			SONY_CAMERA_SUB_DEVICE_TYPE_OIS);
		mutex_unlock(&i2c_device->lock);
		break;
	}
	case SONY_CAMERA_CMD_SPI_READ:
	{
		struct sony_camera_qup_spi_device *spi_dev =
			&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP];
		struct sony_camera_spi_data camera_spi_data;

		mutex_lock(&spi_dev->lock);
		if (!spi_dev->enabled) {
			LOGE("Device is not enabled");
			rc = -ENODEV;
			mutex_unlock(&spi_dev->lock);
			break;
		}
		rc = copy_from_user(&camera_spi_data,
			(void __user *)parm, sizeof(camera_spi_data));
		if (rc < 0) {
			mutex_unlock(&spi_dev->lock);
			break;
		}
		LOGI("Begin write header size = %d", camera_spi_data.header_size);
		rc = sony_camera_spi_write_with_status(data,
			(const uint8_t *)camera_spi_data.header,
			camera_spi_data.header_size,
			SONY_CAMERA_IRQ_STATUS_ASSERT,
			spi_dev);
		LOGI("End write header rc = %d", rc);
		if (rc < 0) {
			LOGE("sony_camera_spi_write_with_status failed");
			mutex_unlock(&spi_dev->lock);
			break;
		}

		LOGI("Begin read payload size = %d", camera_spi_data.payload_size);
		rc = sony_camera_spi_read_with_status(data,
			camera_spi_data.payload,
			camera_spi_data.payload_size,
			SONY_CAMERA_IRQ_STATUS_DEASSERT,
			spi_dev);
		LOGI("End read payload rc = %d", rc);
		if (rc < 0) {
			LOGE("sony_camera_spi_read_with_status failed");
			mutex_unlock(&spi_dev->lock);
			break;
		}
		mutex_unlock(&spi_dev->lock);
		break;
	}
	case SONY_CAMERA_CMD_SPI_WRITE:
	{
		struct sony_camera_qup_spi_device *spi_dev =
			&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP];
		struct sony_camera_spi_data camera_spi_data;

		mutex_lock(&spi_dev->lock);
		if (!spi_dev->enabled) {
			LOGE("Device is not enabled");
			rc = -ENODEV;
			mutex_unlock(&spi_dev->lock);
			break;
		}
		rc = copy_from_user(&camera_spi_data,
			(void __user *)parm, sizeof(camera_spi_data));
		if (rc < 0)
			break;
		LOGD("Begin write header size = %d", camera_spi_data.header_size);
		rc = sony_camera_spi_write_with_status(data,
			(uint8_t *)camera_spi_data.header,
			camera_spi_data.header_size,
			SONY_CAMERA_IRQ_STATUS_ASSERT,
			spi_dev);
		LOGD("End write header rc = %d", rc);
		if (rc < 0) {
			LOGE("sony_camera_spi_write_with_status failed");
			mutex_unlock(&spi_dev->lock);
			break;
		}

		LOGD("Begin write payload size = %d", camera_spi_data.payload_size);
		rc = sony_camera_spi_write_with_status(data,
			(const uint8_t *)camera_spi_data.payload,
			camera_spi_data.payload_size,
			SONY_CAMERA_IRQ_STATUS_DEASSERT,
			spi_dev);
		LOGD("End write payload rc = %d", rc);
		mutex_unlock(&spi_dev->lock);
		break;
	}
	case SONY_CAMERA_CMD_SPI_READ_2:
	{
		struct sony_camera_spi_data camera_spi_data;
		struct sony_camera_qup_spi_device *spi_dev =
			&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_OIS];

		mutex_lock(&spi_dev->lock);
		LOGD("sensor qup SPI read id=%d", data->id);
		if (!spi_dev->enabled) {
			LOGE("Device is not enabled");
			rc = -ENODEV;
			mutex_unlock(&spi_dev->lock);
			break;
		}
		rc = copy_from_user(&camera_spi_data,
			(void __user *)parm, sizeof(camera_spi_data));
		if (rc < 0) {
			LOGE("Copy camera_spi_data fail from user");
			mutex_unlock(&spi_dev->lock);
			break;
		}
		if ((camera_spi_data.header_size + camera_spi_data.payload_size >
			spi_dev->cache_size) ||
			(camera_spi_data.header_size > spi_dev->max_tx_size) ||
			(camera_spi_data.payload_size > spi_dev->max_rx_size)) {
			LOGE("Invalid size set for write and read");
			rc = -EINVAL;
			mutex_unlock(&spi_dev->lock);
			break;
		}
		if (camera_spi_data.header_size > 0) {
			rc = copy_from_user(spi_dev->cache,
				(void __user *)camera_spi_data.header,
				camera_spi_data.header_size);
			if (rc < 0) {
				LOGE("Copy header fail from user");
				mutex_unlock(&spi_dev->lock);
				break;
			}
		}
		LOGD("Begin read header size = %d payload size = %d",
			camera_spi_data.header_size, camera_spi_data.payload_size);
		rc = sony_camera_spi_write_and_read(data,
			camera_spi_data.header_size,
			camera_spi_data.payload_size,
			spi_dev);
		if (rc < 0) {
			LOGE("sony_camera_spi_write_and_read failed");
			mutex_unlock(&spi_dev->lock);
			break;
		}
		if (copy_to_user((void __user *)camera_spi_data.payload,
			&spi_dev->cache[camera_spi_data.header_size],
			camera_spi_data.payload_size)) {
			rc = -EFAULT;
			mutex_unlock(&spi_dev->lock);
			break;
		}
		mutex_unlock(&spi_dev->lock);
		break;
	}
	case SONY_CAMERA_CMD_SPI_WRITE_2:
		break;
	case SONY_CAMERA_CMD_DOWNLOAD_FW:
	{
		struct sony_camera_write_flash_data firmware;
		char *flash_data = NULL;
		struct sony_camera_qup_spi_device *spi_dev =
			&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_FLASH];

		mutex_lock(&spi_dev->lock);
		rc = copy_from_user(&firmware, (void __user *)parm,
			sizeof(struct sony_camera_write_flash_data));
		if (rc < 0) {
			break;
			mutex_unlock(&spi_dev->lock);
		}
		flash_data = vzalloc(firmware.size);
		if (flash_data == NULL) {
			LOGE("vzalloc(%d) failed", firmware.size);
			rc = -ENOMEM;
			mutex_unlock(&spi_dev->lock);
			break;
		}
		rc = copy_from_user(flash_data, (void __user *)firmware.data, firmware.size);
		if (rc < 0) {
			vfree(flash_data);
			mutex_unlock(&spi_dev->lock);
			break;
		}
		LOGI("Update firmware: %d", firmware.size);
		rc = sony_camera_gpio_reset(data, false);
		if (rc < 0) {
			LOGE("gpio reset failed");
			vfree(flash_data);
			mutex_unlock(&spi_dev->lock);
			break;
		}
		sony_camera_spi_set(data, 0);
		msleep(10);
		rc = sony_camera_download_fw(data, flash_data, firmware.addr, firmware.size);
		if (rc < 0) {
			LOGE("spi flash failed");
			vfree(flash_data);
			mutex_unlock(&spi_dev->lock);
			break;
		}
		sony_camera_spi_set(data, -1);
		rc = sony_camera_gpio_reset(data, true);
		if (rc < 0) {
			LOGE("gpio reset failed");
			vfree(flash_data);
			mutex_unlock(&spi_dev->lock);
			break;
		}
		msleep(1000);
		vfree(flash_data);
		mutex_unlock(&spi_dev->lock);
		break;
	}
	case SONY_CAMERA_CMD_READ_FLASH:
	{
		struct sony_camera_nor_flash_data flash_data;
		int rc2 = 0;
		struct sony_camera_qup_spi_device *spi_dev =
			&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_FLASH];

		mutex_lock(&spi_dev->lock);
		rc = copy_from_user(&flash_data,
			(void __user *)parm, sizeof(flash_data));
		if (rc < 0) {
			mutex_unlock(&spi_dev->lock);
			break;
		}
		LOGI("Read flash: address:0x%08X, size %d.\n",
			flash_data.addr, flash_data.size);
		LOGI("Flash size:(0x%08X).\n", flash_data.size);
		rc = sony_camera_gpio_reset(data, false);
		if (rc < 0) {
			LOGE("%s:%d gpio reset failed\n", __func__, __LINE__);
			mutex_unlock(&spi_dev->lock);
			break;
		}
		msleep(10);
		rc = sony_camera_read_flash_memory(data, flash_data.buf, flash_data.addr, flash_data.size);
		if (rc < 0) {
			LOGE("%s:%d read flash memory failed\n", __func__, __LINE__);
		}
		rc2 = sony_camera_gpio_reset(data, true);
		if (rc2 < 0)
			LOGE("%s:%d gpio reset failed\n", __func__, __LINE__);
		msleep(1000);
		mutex_unlock(&spi_dev->lock);
		break;
	}
	case SONY_CAMERA_CMD_WAIT_IDLE:
	{
		struct sony_camera_qup_spi_device *spi_dev =
			&data->qup_spi_dev[SONY_CAMERA_SUB_DEVICE_TYPE_EXTERNAL_ISP];
		mutex_lock(&spi_dev->lock);
		rc = sony_camera_wait_idle(data, SONY_CAMERA_IRQ_STATUS_ASSERT);
		if (rc < 0)
			LOGE("%s:%d fe idle failed\n", __func__, __LINE__);
		mutex_unlock(&spi_dev->lock);
		break;
	}
	case SONY_CAMERA_CMD_GET_EVENT:
		spin_lock_irqsave(&data->event_list_lock, flags);
		if (list_empty(&data->event_available)) {
			rc = -ENODATA;
			spin_unlock_irqrestore(&data->event_list_lock, flags);
		} else {
			struct sony_camera_event_list_data *event = list_entry(
				data->event_available.next,
				struct sony_camera_event_list_data, list);
			list_del(&event->list);
			spin_unlock_irqrestore(&data->event_list_lock, flags);
			rc = copy_to_user((void __user *)parm,
				&event->event_data , sizeof(event->event_data));
			kfree(event);
		}
		break;
	case SONY_CAMERA_CMD_SET_THERMAL:
		mutex_lock(&data->thermal_lock);
		data->thermal_ret_val = 0;
		rc = copy_from_user(&data->thermal_sensor_temperature,
			(void __user *)parm, sizeof(data->thermal_sensor_temperature));
		if (rc < 0) {
			LOGE("copy_from_user failed");
			rc = -EINVAL;
		}
		mutex_unlock(&data->thermal_lock);
		break;
	case SONY_CAMERA_CMD_CLEAR:
		mutex_lock(&data->driver_lock);
		rc = sony_camera_clear(data);
		mutex_unlock(&data->driver_lock);
		break;
	case SONY_CAMERA_CMD_BUG_ON:
		mutex_lock(&data->driver_lock);
		BUG_ON(1);
		mutex_unlock(&data->driver_lock);
		break;
	case SONY_CAMERA_CMD_GET_POWER_STATE:
		mutex_lock(&data->driver_lock);
		rc = data->state;
		mutex_unlock(&data->driver_lock);
		break;
	case SONY_CAMERA_CMD_SET_POWER:
		mutex_lock(&data->driver_lock);
		{
			struct sony_camera_power_cmd cmd;
			rc = copy_from_user(&cmd,
				(void __user *)parm, sizeof(cmd));
			if (rc < 0) {
				LOGE("copy_from_user failed");
				mutex_unlock(&data->driver_lock);
				goto exit;
			}
			rc = sony_camera_set_power(data, cmd.cmd, (cmd.arg != 0));
			if (rc < 0) {
				LOGE("set_power failed");
				mutex_unlock(&data->driver_lock);
				goto exit;
			}
		}
		mutex_unlock(&data->driver_lock);
		break;
	case SONY_CAMERA_CMD_START_SOF_EVENT:
		mutex_lock(&data->driver_lock);
		rc = sony_camera_sof_event_ctrl(data, 1);
		mutex_unlock(&data->driver_lock);
		break;
	case SONY_CAMERA_CMD_STOP_SOF_EVENT:
		mutex_lock(&data->driver_lock);
		rc = sony_camera_sof_event_ctrl(data, 0);
		mutex_unlock(&data->driver_lock);
		break;
	default:
		rc = -EINVAL;
		break;
	}
exit:
	return rc;
}

static long sony_camera_ioctl(struct file* file,
	unsigned int cmd, unsigned long parm)
{
	unsigned int rc = 0;

	rc = sony_camera_ioctl_common(file, cmd, parm, false);

	return rc;
}

static long sony_camera_ioctl32(struct file* file,
	unsigned int cmd, unsigned long parm)
{
	unsigned int rc = 0;

	rc = sony_camera_ioctl_common(file, cmd, parm, true);

	return rc;
}

static struct file_operations sony_camera_fops =
{
	.owner   = THIS_MODULE,
	.poll    = sony_camera_poll,
	.open    = sony_camera_open,
	.release = sony_camera_close,
#ifdef CONFIG_COMPAT
	.compat_ioctl = sony_camera_ioctl32,
#endif	/* CONFIG_COMPAT */
	.unlocked_ioctl = sony_camera_ioctl,
};

static const struct of_device_id sony_camera_0_dt_match[] =
{
	{
		.compatible = "sony_camera_0",
		.data = &camera_data[0]
	},
	{
	},
};

static const struct of_device_id sony_camera_1_dt_match[] =
{
	{
		.compatible = "sony_camera_1",
		.data = &camera_data[1]
	},
	{
	},
};

static const struct of_device_id sony_camera_2_dt_match[] =
{
	{
		.compatible = "sony_camera_2",
		.data = &camera_data[2]
	},
	{
	},
};

static const struct of_device_id sony_camera_3_dt_match[] =
{
	{
		.compatible = "sony_camera_3",
		.data = &camera_data[3]
	},
	{
	},
};

static const struct of_device_id sony_camera_4_dt_match[] =
{
	{
		.compatible = "sony_camera_4",
		.data = &camera_data[4]
	},
	{
	},

};

static const struct of_device_id sony_camera_5_dt_match[] =
{
	{
		.compatible = "sony_camera_5",
		.data = &camera_data[5]
	},
	{
	},
};

static const struct of_device_id sony_camera_subdev_dt_match[] =
{
	{
		.compatible = "sony,camera_subdev"
	},
	{ },
};

MODULE_DEVICE_TABLE(of, sony_camera_0_dt_match      );
MODULE_DEVICE_TABLE(of, sony_camera_1_dt_match      );
MODULE_DEVICE_TABLE(of, sony_camera_2_dt_match      );
MODULE_DEVICE_TABLE(of, sony_camera_3_dt_match      );
MODULE_DEVICE_TABLE(of, sony_camera_4_dt_match      );
MODULE_DEVICE_TABLE(of, sony_camera_5_dt_match      );
MODULE_DEVICE_TABLE(of, sony_camera_subdev_dt_match );

static const struct i2c_device_id sony_camera_qup_i2c_subdev_id[] =
{
	{ "camera_subdev", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, sony_camera_qup_i2c_subdev_id       );

static int sony_camera_qup_i2c_subdev_probe(struct i2c_client *qup_client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	uint32_t camera_id = 0;
	uint32_t device_type = 0;
	struct sony_camera_qup_i2c_device *device = NULL;
	uint8_t retry_count = SONY_CAMERA_MAX_RETRY_COUNT;

	if (!i2c_check_functionality(qup_client->adapter, I2C_FUNC_I2C)) {
		LOGE("check functionality failed");
		rc = -EIO;
		goto fail;
	}

	rc = of_property_read_u32(qup_client->dev.of_node, "camera_id", &camera_id);
	if (rc < 0) {
		LOGE("find camera_id failed");
		rc = -EINVAL;
		goto fail;
	}
	rc = of_property_read_u32(qup_client->dev.of_node, "device_type", &device_type);
	if (rc < 0) {
		LOGE("find device_type failed");
		rc = -EINVAL;
		goto fail;
	}

	device = &camera_data[camera_id].qup_i2c_dev[device_type];
	device->client = qup_client;
	device->enabled = true;

	while (!(device->cache = kzalloc(SONY_CAMERA_QUP_I2C_MAX_LEN, GFP_KERNEL)) &&
			retry_count) {
		retry_count--;
		usleep_range(1000, 3000);
		LOGE("Retry kmalloc qup i2c cache");
	}
	if (!device->cache) {
		LOGE("qup cache kmalloc failed");
		return -ENOMEM;
	}

	LOGI("QUP I2C driver probe success attached to camera %d", camera_id);
	return 0;
fail:
	if (device->cache)
		kfree(device->cache);
	return rc;
}

static int sony_camera_qup_i2c_subdev_remove(struct i2c_client *qup_client)
{
	i2c_set_clientdata(qup_client, NULL);
	return 0;
}

static struct i2c_driver sony_camera_qup_i2c_subdev_driver =
{
	.driver = {
		.name = "sony_camera_i2c_subdev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sony_camera_subdev_dt_match),
	},
	.probe = sony_camera_qup_i2c_subdev_probe,
	.id_table = sony_camera_qup_i2c_subdev_id,
	.remove = sony_camera_qup_i2c_subdev_remove,
};
static bool i2c_qup_driver_added = false;

static int sony_camera_qup_spi_subdev_probe(struct spi_device *spi)
{
	int rc = 0;
	uint32_t camera_id = 0;
	uint32_t device_type = 0;
	uint32_t bits_per_word = 0;
	struct sony_camera_qup_spi_device *device = NULL;
	uint8_t retry_cnt = SONY_CAMERA_MAX_RETRY_COUNT;

	rc = of_property_read_u32(spi->dev.of_node, "camera_id", &camera_id);
	if (rc < 0) {
		LOGE("find camera_id failed");
		rc = -EINVAL;
		goto fail;
	}
	rc = of_property_read_u32(spi->dev.of_node, "device_type", &device_type);
	if (rc < 0 || device_type >= SONY_CAMERA_SUB_DEVICE_TYPE_MAX) {
		LOGE("find device_type failed");
		rc = -EINVAL;
		goto fail;
	}
	rc = of_property_read_u32(spi->dev.of_node, "bits_per_word", &bits_per_word);
	if (rc < 0) {
		LOGE("find bits_per_word failed");
		rc = -EINVAL;
		goto fail;
	}
	device = &camera_data[camera_id].qup_spi_dev[device_type];
	rc = of_property_read_u32(spi->dev.of_node, "max_rx_size", &device->max_rx_size);
	if (rc < 0) {
		LOGE("find max_rx_size failed");
		rc = -EINVAL;
		goto fail;
	}
	rc = of_property_read_u32(spi->dev.of_node, "max_tx_size", &device->max_tx_size);
	if (rc < 0) {
		LOGE("find max_tx_size failed");
		rc = -EINVAL;
		goto fail;
	}

	spi->bits_per_word = bits_per_word;
	spi_setup(spi);
	device->client = spi;
	device->enabled = true;

	device->cache_size = device->max_tx_size > device->max_rx_size ?
		device->max_tx_size : device->max_rx_size;
	while (!(device->cache =
		kzalloc(device->cache_size, GFP_KERNEL | GFP_DMA)) && retry_cnt) {
		retry_cnt--;
		usleep_range(1000, 3000);
		LOGE("Retry kmalloc qup spi payload");
	}
	if (!device->cache) {
		LOGE("qup spi payload data kmalloc failed");
		return -ENOMEM;
	}

	LOGI("spi %p device_type %d probe success attatched to camera %d", spi, device_type, camera_id);
	return 0;
fail:
	if (device->cache)
		kfree(device->cache);
	return rc;
}

static int sony_camera_qup_spi_subdev_remove(struct spi_device *spi)
{
	return 0;
}

static struct spi_driver sony_camera_qup_spi_subdev_driver =
{
	.driver = {
		.name = "sony_camera_spi_subdev",
		.owner = THIS_MODULE,
		.of_match_table = sony_camera_subdev_dt_match,
	},
	.probe = sony_camera_qup_spi_subdev_probe,
	.remove = sony_camera_qup_spi_subdev_remove,
};
static bool spi_qup_driver_added = false;

static struct platform_driver sony_camera_platform_driver[] =
{
	{
		.driver = {
			.name = "sony_camera_0",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_0_dt_match,
		},
	},
	{
		.driver = {
			.name = "sony_camera_1",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_1_dt_match,
		},
	},
	{
		.driver = {
			.name = "sony_camera_2",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_2_dt_match,
		},
	},
	{
		.driver = {
			.name = "sony_camera_3",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_3_dt_match,
		},
	},
#ifdef USE_CAMERA_4
	{
		.driver = {
			.name = "sony_camera_4",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_4_dt_match,
		},
	},
#endif
#ifdef USE_CAMERA_5
	{
		.driver = {
			.name = "sony_camera_5",
			.owner = THIS_MODULE,
			.of_match_table = sony_camera_5_dt_match,
		},
	},
#endif
};

static void sony_camera_platform_cleanup(void)
{
	uint16_t i;
	uint16_t j;

	for (i = 0; i < g_sensor_num; i++) {
		platform_driver_unregister(&sony_camera_platform_driver[i]);
		if (camera_data[i].module) {
			if (camera_data[i].module->seq_on)
				vfree(camera_data[i].module->seq_on);
			if (camera_data[i].module->seq_off)
				vfree(camera_data[i].module->seq_off);
			if (camera_data[i].module)
				vfree(camera_data[i].module);
		}
		for (j = 0; j < SONY_CAMERA_MAX_TEMPERATURE_NUM_PER_CAMERA; j++) {
			if (camera_data[i].thermal_zone_dev[j]) {
				thermal_zone_device_unregister(camera_data[i].thermal_zone_dev[j]);
			}
		}
	}
}

static int sony_camera_platform_probe(struct platform_device *p_dev)
{
	int rc = 0;
	uint32_t id = 0;
	const struct of_device_id *match;
	match = of_match_device(sony_camera_0_dt_match, &p_dev->dev);
	if (!match && g_sensor_num > 1) {
		match = of_match_device(sony_camera_1_dt_match, &p_dev->dev);
		id = 1;
	}
	if (!match && g_sensor_num > 2) {
		match = of_match_device(sony_camera_2_dt_match, &p_dev->dev);
		id = 2;
	}
	if (!match && g_sensor_num > 3) {
		match = of_match_device(sony_camera_3_dt_match, &p_dev->dev);
		id = 3;
	}
	if (!match && g_sensor_num > 4) {
		match = of_match_device(sony_camera_4_dt_match, &p_dev->dev);
		id = 4;
	}
	if (!match && g_sensor_num > 5) {
		match = of_match_device(sony_camera_5_dt_match, &p_dev->dev);
		id = 5;
	}
	if (!match) {
		LOGE("of_match_device fail");
		rc = -EFAULT;
		goto fail;
	}

	camera_data[id].id = id;
	camera_data[id].p_dev = p_dev;
	camera_data[id].probe_done = false;

	rc = sony_camera_parse_dt(p_dev, id);
	if (rc < 0) {
		LOGE("sony_camera_parse_dt failed");
		goto fail;
	}
	if (!camera_data[id].qup_i2c_dev[SONY_CAMERA_SUB_DEVICE_TYPE_SENSOR].enabled) {
		// CCI initialize
		camera_data[id].cci_dev.cci_info.cci_subdev = cam_cci_get_subdev(camera_data[id].cci_dev.cci_info.cci_device);
		camera_data[id].cci_dev.cci_info.i2c_freq_mode = camera_data[id].module->i2c_freq_mode;
		camera_data[id].cci_dev.cci_info.sid = 0;
		camera_data[id].cci_dev.cci_info.cid = 0;
		camera_data[id].cci_dev.cci_info.timeout = 0;
		camera_data[id].cci_dev.cci_info.retries = 3;
		camera_data[id].cci_dev.cci_info.id_map = 0;
	}
	camera_data[id].pinctrl = devm_pinctrl_get(&p_dev->dev);
	if (IS_ERR_OR_NULL(camera_data[id].pinctrl)) {
		LOGE("Getting pinctrl handle failed");
		rc = -EINVAL;
		goto fail;
	}
	camera_data[id].gpio_state_active =
		pinctrl_lookup_state(camera_data[id].pinctrl,
			SONY_CAMERA_PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(camera_data[id].gpio_state_active)) {
		LOGE("Failed to get the active state pinctrl handle");
		rc = -EINVAL;
		goto fail;
	}
	camera_data[id].gpio_state_suspend =
		pinctrl_lookup_state(camera_data[id].pinctrl,
			SONY_CAMERA_PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(camera_data[id].gpio_state_suspend)) {
		LOGE("Failed to get the suspend state pinctrl handle");
		rc = -EINVAL;
		goto fail;
	}

	if (camera_data[id].thremal_enable) {
		uint8_t n = 0;
		uint8_t max_cmb_tbl_num = sizeof(driver_comb_tbl) / sizeof(driver_comb_tbl[0]);
		for (n = 0; n < max_cmb_tbl_num; n++) {
			if (id == driver_comb_tbl[n].id) {
				camera_data[id].thermal_zone_dev[driver_comb_tbl[n].thermal_zone] =
					thermal_zone_device_register(driver_comb_tbl[n].thermal_name,
					0, 0, 0, &sony_camera_thermal_ops, 0, 0, 0);
				if (IS_ERR(camera_data[id].thermal_zone_dev[driver_comb_tbl[n].thermal_zone])) {
					LOGE("thermal_zone_device_register (%u)", id);
					rc = PTR_ERR(camera_data[id].thermal_zone_dev[driver_comb_tbl[n].thermal_zone]);
					goto fail;
				}
			}
		}
	}

	camera_data[id].probe_done = true;
	LOGI("camera %d probe ok", id);

	return 0;

fail:
	return rc;
}

static int sony_camera_set_debug(void *data, u64 val)
{
	g_sony_camera_debug_flag = val;
	LOGD("Set sony camera debug value: 0x%08x", val);
	g_sony_camera_debug_flag |= 0x01;
	return 0;
}

static int sony_camera_get_debug(void *data, u64 *val)
{
	*val = g_sony_camera_debug_flag;
	LOGD("Get sony camera debug value: 0x%08x",
		g_sony_camera_debug_flag);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(sony_camera_debug,
	sony_camera_get_debug,
	sony_camera_set_debug, "%llx\n");

static int sony_camera_debug_register(void)
{
	int rc = 0;
	struct dentry *dbg_file_ptr = NULL;

	dbg_file_ptr = debugfs_create_dir("sony_camera", NULL);
	if (!dbg_file_ptr) {
		LOGE("DebugFS could not create directory!");
		rc = -ENOENT;
		goto end;
	}
	/* Store parent inode for cleanup in caller */
	g_dbg_file_folder_ptr = dbg_file_ptr;

	dbg_file_ptr = debugfs_create_file("log_mask", 0644,
		g_dbg_file_folder_ptr, NULL, &sony_camera_debug);

	if (IS_ERR(dbg_file_ptr)) {
		if (PTR_ERR(dbg_file_ptr) == -ENODEV)
			LOGE("DebugFS not enabled in kernel!");
		else
			rc = PTR_ERR(dbg_file_ptr);
		goto end;
	}

	return rc;
end:
	debugfs_remove_recursive(g_dbg_file_folder_ptr);
	g_dbg_file_folder_ptr = NULL;
	return rc;
}

static int __init sony_camera_init_module(void)
{
	int rc = 0;
	uint16_t i, j;
	uint16_t probe_count = 0;
	uint32_t retryCnt = 0;

	g_sensor_num = ARRAY_SIZE(sony_camera_platform_driver);
	i2c_add_driver(&sony_camera_qup_i2c_subdev_driver);
	i2c_qup_driver_added = true;
	spi_register_driver(&sony_camera_qup_spi_subdev_driver);
	spi_qup_driver_added = true;

	for (i = 0; i < g_sensor_num; i++) {
		camera_data[i].state = SONY_CAMERA_STATE_POWER_DOWN;
		camera_data[i].taskq_idx = 0;
		camera_data[i].open_count = 0;
		mutex_init(&camera_data[i].driver_lock);
		mutex_init(&camera_data[i].cci_dev.lock);
		for (j = 0; j < SONY_CAMERA_SUB_DEVICE_TYPE_MAX; j++) {
			mutex_init(&camera_data[i].qup_i2c_dev[j].lock);
			mutex_init(&camera_data[i].qup_spi_dev[j].lock);
		}
		spin_lock_init(&camera_data[i].task_lock);
		spin_lock_init(&camera_data[i].event_list_lock);
		mutex_init(&camera_data[i].thermal_lock);
		spin_lock_init(&camera_data[i].sof_lock);
		mutex_init(&camera_data[i].state_lock);
		init_waitqueue_head(&camera_data[i].event_wait_q);
		init_waitqueue_head(&camera_data[i].wait_q);
		INIT_LIST_HEAD(&camera_data[i].event_available);
		INIT_LIST_HEAD(&camera_data[i].task_q);
		init_completion(&camera_data[i].spi_assert_complete);
		init_completion(&camera_data[i].spi_deassert_complete);
		init_completion(&camera_data[i].idle_assert_complete);
		init_completion(&camera_data[i].idle_deassert_complete);
		retryCnt = 0;
		do {
			rc = platform_driver_probe(
				&sony_camera_platform_driver[i],
				sony_camera_platform_probe);
			if (rc < 0) {
				usleep_range(10000,10000);
				retryCnt++;
				LOGE("platform_driver_probe(%u), retry. rc=%d", i, rc);
			}
		} while (rc < 0 && retryCnt < 100);
		if (rc < 0) {
			LOGE("sony_camera%u platform_driver_probe fail rc=%d", i,rc);
			continue;
		} else {
			LOGI("sony_camera%u platform_driver_probe success", i);
		}
		probe_count++;
		msleep(SONY_CAMERA_FRONT_SENSOR_POWER_UP_WAIT);
	}

	if (!probe_count) {
		LOGE("platform_driver_probe (%u)", probe_count);
		goto fail_probe;
	}

	/* register the driver */
	g_dev_id = register_chrdev(0, SONY_CAMERA_DEV_NAME, &sony_camera_fops);
	LOGD("register_chrdev() (Major:%d).", g_dev_id);
	if (g_dev_id < 0) {
		LOGE("register_chrdev() failed (Major:%d).",
			g_dev_id);
		rc = -EINVAL;
		goto fail_register_chrdev;
	}

	/* memory allocation */
	g_camera_device = platform_device_alloc(SONY_CAMERA_DEV_NAME, -1);
	if (!g_camera_device) {
		LOGE("platform_device_alloc() failed.");
		rc = -ENOMEM;
		goto fail_device_alloc;
	}

	/* add device */
	rc = platform_device_add(g_camera_device);
	if (rc) {
		LOGE("platform_device_add() failed.");
		goto fail_platform_dev_add;
	}

	/* create the node of device */
	g_camera_device_class = class_create(THIS_MODULE, SONY_CAMERA_DEV_NAME);
	if (IS_ERR(g_camera_device_class)) {
		LOGE("class_create() failed.");
		rc = PTR_ERR(g_camera_device_class);
		goto fail_class_create;
	}

	/* create the logical device */
	for (i = 0; i < g_sensor_num; i++) {
		if (camera_data[i].probe_done) {
			struct device *dev = NULL;
			dev = device_create(g_camera_device_class, NULL, MKDEV(g_dev_id, i), NULL, SONY_CAMERA_DEV_NAME"%d", i);
			if (IS_ERR(dev)) {
				LOGE("device_create() failed.");
				rc = PTR_ERR(dev);
				goto fail_device_create;
			}
		} else {
			LOGE("camera %d probe failed do not create device", i);
		}
	}

	(void)sony_camera_debug_register();

	return 0;

fail_device_create:
	for (i = 0; i < g_sensor_num; i++) {
		device_destroy(g_camera_device_class, MKDEV(g_dev_id, i));
	}
	class_destroy(g_camera_device_class);
fail_class_create:
	platform_device_del(g_camera_device);
fail_platform_dev_add:
	platform_device_put(g_camera_device);
fail_device_alloc:
	unregister_chrdev_region(g_dev_id, g_sensor_num);
fail_register_chrdev:
	sony_camera_platform_cleanup();
fail_probe:
	if (spi_qup_driver_added) {
		spi_unregister_driver(&sony_camera_qup_spi_subdev_driver);
		spi_qup_driver_added = false;
	}
	if (i2c_qup_driver_added) {
		i2c_del_driver(&sony_camera_qup_i2c_subdev_driver);
		i2c_qup_driver_added = false;
	}
	return 0;
}

static void __exit sony_camera_exit_module(void)
{
	uint16_t i, j;

	if (i2c_qup_driver_added)
		i2c_del_driver(&sony_camera_qup_i2c_subdev_driver);
	if (spi_qup_driver_added)
		spi_unregister_driver(&sony_camera_qup_spi_subdev_driver);

	sony_camera_platform_cleanup();
	for (i = 0; i < g_sensor_num; i++) {
		mutex_destroy(&camera_data[i].driver_lock);
		mutex_destroy(&camera_data[i].cci_dev.lock);
		for (j = 0; j < SONY_CAMERA_SUB_DEVICE_TYPE_MAX; j++) {
			mutex_destroy(&camera_data[i].qup_i2c_dev[j].lock);
			mutex_destroy(&camera_data[i].qup_spi_dev[j].lock);
		}
		mutex_destroy(&camera_data[i].thermal_lock);
		mutex_destroy(&camera_data[i].state_lock);
	}
	unregister_chrdev_region(g_dev_id, g_sensor_num);
	debugfs_remove_recursive(g_dbg_file_folder_ptr);
	g_dbg_file_folder_ptr = NULL;
}

static int sony_camera_set_power(struct sony_camera_data *data,
	int cmd, bool enable)
{
	unsigned int rc;
	struct sony_camera_regulator_t *regulator_ptr = NULL;

	regulator_ptr = sony_camera_vreg_get(data, cmd, enable ? 0 : -1);
	if (!regulator_ptr) {
		LOGE("sony_camera_vreg_get failed cmd %d enable %d", cmd, enable);
		rc = -ENODEV;
		goto exit;
	}
	if (enable) {
		rc = sony_camera_vreg_set(regulator_ptr, 0, 0);
	} else {
		rc = sony_camera_vreg_set(regulator_ptr, -1, 0);
	}

	if (rc >= 0) {
		regulator_ptr->enabled_by_user = enable;
	}
exit:
	return rc;
}

module_init(sony_camera_init_module);
module_exit(sony_camera_exit_module);

MODULE_SOFTDEP("pre: slg51000-regulator");
MODULE_DESCRIPTION("SONY camera sensor driver");
MODULE_LICENSE("GPL v2");

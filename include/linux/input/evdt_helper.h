/* include/linux/input/evdt_helper.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * Author: Kenji Suzuki <Kenji.X.Suzuki@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LINUX_INPUT_EVDT_HELPER_H__
#define __LINUX_INPUT_EVDT_HELPER_H__

#include <linux/input.h>
#include <linux/delay.h>
#include <linux/of.h>

enum {
	EVDT_MSLEEP = 0,
	EVDT_KEY,
	EVDT_LOG,
	EVDT_END = 99,
};

static inline int evdt_get_msleep(struct device_node *node, u32 *ms)
{
	return of_property_read_u32(node, "ms", ms);
}

static inline int evdt_get_key(struct device_node *node, u32 *code, u32 *down)
{
	int rc;
	rc = of_property_read_u32(node, "code", code);
	if (!rc)
		rc = of_property_read_u32(node, "down", down);
	return rc;
}

static inline int evdt_get_log(struct device_node *node, const char **message)
{
	return of_property_read_string(node, "message", message);
}

static inline struct device_node *evdt_initialize(struct device *dev,
				struct input_dev *input, const char *node)
{
	struct device_node *wg_node = NULL;
	struct device_node *block = NULL;
	struct device_node *record = NULL;
	int index = 0;
	u32 type;
	u32 ms;
	u32 code;
	u32 down;
	const char *message;
	int rc;

	if (!dev->of_node || !input)
		return NULL;

	wg_node = of_find_node_by_name(dev->of_node, node);
	if (!wg_node) {
		pr_err("%s: No %s node found\n", __func__, node);
		return NULL;
	}

	for_each_child_of_node(wg_node, block) {
		index = 0;
		for_each_child_of_node(block, record) {
			rc = of_property_read_u32(record, "type", &type);
			if (rc)
				goto error;

			switch (type) {
			case EVDT_MSLEEP:
				rc = evdt_get_msleep(record, &ms);
				if (rc)
					goto error;
				pr_info("%s: MSLEEP [%u]\n", __func__, ms);
				break;
			case EVDT_KEY:
				rc = evdt_get_key(record, &code, &down);
				if (rc)
					goto error;
				input_set_capability(input, EV_KEY, code);
				pr_info("%s: KEY [%u, %s]\n", __func__, code,
					down ? "DOWN" : "UP");
				break;
			case EVDT_LOG:
				rc = evdt_get_log(record, &message);
				if (rc)
					goto error;
				pr_info("%s: LOG [%s]\n", __func__, message);
				break;
			case EVDT_END:
			default:
				break;
			}
			index++;
		}
	}
	return wg_node;

error:
	pr_err("%s: invalid value at index %d for %s rc=%d\n",
	       __func__, index, block->name, rc);
	return NULL;
}

static inline int evdt_execute(struct device_node *block,
				struct input_dev *input, u16 gesture_code)
{
	struct device_node *gesture = NULL;
	struct device_node *record;
	u32 type;
	u32 ms;
	u32 code;
	u32 down;
	const char *message;
	int rc;

	if (!block || !input)
		return 0;

	for_each_child_of_node(block, gesture) {
		rc = of_property_read_u32(gesture, "gesture_code", &type);
		if (rc) {
			pr_err("%s: Error reading gesture_code for %s rc=%d\n",
					__func__, gesture->name, rc);
			return rc;
		}

		if (gesture_code != type)
			continue;

		for_each_child_of_node(gesture, record) {
			of_property_read_u32(record, "type", &type);

			switch (type) {
			case EVDT_MSLEEP:
				evdt_get_msleep(record, &ms);
				pr_debug("%s: MSLEEP(%d)\n", __func__, ms);
				msleep(ms);
				break;
			case EVDT_KEY:
				evdt_get_key(record, &code, &down);
				pr_debug("%s: KEY(%d) %s\n", __func__, code,
					 down ? "DOWN" : "UP");
				input_report_key(input, code, down);
				input_sync(input);
				break;
			case EVDT_LOG:
				evdt_get_log(record, &message);
				pr_info("%s: %s\n", __func__, message);
				break;
			case EVDT_END:
			default:
				goto loop_end;
			}
		}
	}
	pr_warn("%s: Gesture code 0x%X not defined\n", __func__, gesture_code);

loop_end:
	return 0;
}

#endif /* __LINUX_INPUT_EVDT_HELPER_H__ */

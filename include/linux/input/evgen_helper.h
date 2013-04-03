/* include/linux/input/evgen_helper.h
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Kenji Suzuki <Kenji.X.Suzuki@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __LINUX_INPUT_EVGEN_HELPER_H__
#define __LINUX_INPUT_EVGEN_HELPER_H__

#include <linux/input.h>
#include <linux/delay.h>

#define EVGEN_STR_LENGTH	255

enum {
	EVGEN_MSLEEP,
	EVGEN_KEY,
	EVGEN_LOG,
	EVGEN_END,
};

struct evgen_data_msleep {
	unsigned int ms;
};

struct evgen_data_key {
	int code;
	bool down;
};

struct evgen_data_log {
	const char *message;
};

struct evgen_record {
	int type;
	union {
		struct evgen_data_key key;
		struct evgen_data_msleep msleep;
		struct evgen_data_log log;
	} data;
};

struct evgen_block {
	const char *name; /* NULL = end of blocks */
	struct evgen_record *records;
};

static inline void evgen_initialize_records(struct input_dev *input,
					   struct evgen_record *records)
{
	struct evgen_record *r = records;

	while (r) {
		switch (r->type) {
		case EVGEN_MSLEEP:
			dev_dbg(&input->dev, "%s: MSLEEP(%d)\n", __func__,
				r->data.msleep.ms);
			break;
		case EVGEN_KEY:
			dev_dbg(&input->dev, "%s: KEY(%d)\n", __func__,
				r->data.key.code);
			input_set_capability(input, EV_KEY, r->data.key.code);
			break;
		case EVGEN_LOG:
			dev_dbg(&input->dev, "%s: LOG(%s)\n", __func__,
				r->data.log.message);
			break;
		case EVGEN_END:
		default:
			goto loop_end;
		}
		r++;
	}
loop_end:
	dev_dbg(&input->dev, "%s: end of records\n", __func__);
}

static inline void evgen_initialize(struct input_dev *input,
				    struct evgen_block *blocks)
{
	struct evgen_block *b = blocks;

	while (b && b->name && b->records) {
		evgen_initialize_records(input, b->records);
		b++;
	}
}

static inline int evgen_execute_records(struct input_dev *input,
					struct evgen_record *records)
{
	struct evgen_record *r = records;
	int rc = 0;

	while (r) {
		switch (r->type) {
		case EVGEN_MSLEEP:
			msleep(r->data.msleep.ms);
			break;
		case EVGEN_KEY:
			input_report_key(input, r->data.key.code,
					 r->data.key.down);
			input_sync(input);
			break;
		case EVGEN_LOG:
			dev_info(&input->dev, "%s: %s\n", __func__,
				r->data.log.message);
			break;
		case EVGEN_END:
			goto loop_end;
		default:
			dev_err(&input->dev, "%s: *** unknown type (%d)\n",
			       __func__, r->type);
			rc = -EINVAL;
			goto loop_end;
		}
		r++;
	}
loop_end:
	return rc;
}

static inline int evgen_execute(struct input_dev *input,
				struct evgen_block *blocks, const char *name)
{
	struct evgen_block *b = blocks;
	int rc = 0;

	while (b && b->name && b->records) {
		if (strncmp(b->name, name, EVGEN_STR_LENGTH) == 0) {
			rc = evgen_execute_records(input, b->records);
			break;
		}
		b++;
	}

	return rc;
}

#endif /* __LINUX_INPUT_EVGEN_HELPER_H__ */

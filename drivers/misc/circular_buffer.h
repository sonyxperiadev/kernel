#ifndef _CIRCULAR_BUFFER_H
#define _CIRCULAR_BUFFER_H

/*
 * Circular buffer for Idd kernel probes
 *
 * Copyright (C) 2014 Sony Mobile Communications AB.
 *
 * Author: Krzysztof Drobinski <krzysztof.drobinski@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/string.h>

#define CB_SIZE PAGE_SIZE
#define MAX_LINE_SIZE 80
#define MAX_LINES     (CB_SIZE / MAX_LINE_SIZE)

/* Circular buffer */
static char **cbuf;
static atomic_t cb_idx;

static DEFINE_SPINLOCK(cbuf_lock);

static void probe_cb_insert_data(char *data)
{
	unsigned long flags;
	size_t l_idx;

	spin_lock_irqsave(&cbuf_lock, flags);
	l_idx = atomic_add_return(1, &cb_idx);

	/* Fill next line, format your data */
	scnprintf(cbuf[l_idx % MAX_LINES], MAX_LINE_SIZE, "%X %s\n",
		  l_idx, data);
	spin_unlock_irqrestore(&cbuf_lock, flags);
};

static int probe_cb_init(void)
{
	int i;

	cbuf = kmalloc(MAX_LINES * sizeof(char *), GFP_KERNEL);
	if (!cbuf)
		return -ENOMEM;

	for (i = 0; i < MAX_LINES; i++) {
		cbuf[i] = kzalloc(MAX_LINE_SIZE, GFP_KERNEL);
		if (!cbuf[i]) {
			while (i)
				kfree(cbuf[--i]);
			kfree(cbuf);
			return -ENOMEM;
		}
	}
	atomic_set(&cb_idx, 0);

	return 0;
}

static void probe_cb_free(void)
{
	int i;

	for (i = 0; i < MAX_LINES; i++)
		kfree(cbuf[i]);
	kfree(cbuf);
}

#endif /* #ifndef _CIRCULAR_BUFFER_H */

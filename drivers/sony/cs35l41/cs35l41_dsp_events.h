/* SPDX-License-Identifier: GPL-2.0 */

/*
 * cs35l41_dsp_events.h -- DSP-generated event definitions for CS35L41
 *
 * Copyright (c) 2018-2020 Cirrus Logic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __CS35L41_DSP_EVENTS_H
#define __CS35L41_DSP_EVENTS_H

// Event definitions
#define EVENT_PORT_BLOCKED 0x65
#define EVENT_SPEAKER_OPEN_SHORT_STATUS 0x66

// Data definitions
#define SPK_STATUS_ALL_CLEAR     (1 << 0)
#define SPK_STATUS_OPEN_CIRCUIT  (1 << 1)
#define SPK_STATUS_SHORT_CIRCUIT (1 << 2)

#endif /* __CS35L41_DSP_EVENTS_H */

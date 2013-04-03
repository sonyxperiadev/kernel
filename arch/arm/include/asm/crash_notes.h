/* arch/arm/include/asm/crash_notes.h
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __CRASH_NOTES_H
#define __CRASH_NOTES_H

enum crash_note_save_type {
	CRASH_NOTE_INIT,
	CRASH_NOTE_STOPPING,
	CRASH_NOTE_CRASHING
};

extern void crash_notes_save_this_cpu(enum crash_note_save_type type,
				      unsigned int cpu);

#endif

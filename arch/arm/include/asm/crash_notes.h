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

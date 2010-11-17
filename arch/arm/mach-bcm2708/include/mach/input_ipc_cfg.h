/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  VMCS
Module   :  Input Service
File     :  $File: //software/projects/sv-vc4-dev-git/applications/vmcs/inputserv/input_server_cfg.h $
Revision :  $Revision: #1 $

FILE DESCRIPTION
Input service, server side API 
Dependency: HDMI/HDCP VEC middleware
=============================================================================*/
#ifndef _INPUT_IPC_CFG_H_
#define _INPUT_IPC_CFG_H_

// Select what sources of events the input service should pass to the ARM
#define INPUT_WANT_REMOTE
//#define INPUT_WANT_KEYBOARD
//#define INPUT_WANT_MOUSE
#define INPUT_WANT_RAW_REMOTE

// Marker for upper word of our magic values bounding shared data
#ifndef INPUT_MAGIC_HEADER
#define INPUT_MAGIC_HEADER (0xF1F0)
#endif

// Value for lower word of magic value bounding our directory area.
#ifndef INPUT_MAGIC_BASE
#define INPUT_MAGIC_BASE (0xBA5E)
#endif

#ifndef INPUT_SHARED_SIZE
#define INPUT_SHARED_SIZE (4096)
#endif

// Number of events to queue for an input source
#ifndef INPUT_FIFO_SIZE
#define INPUT_FIFO_SIZE (16)
#endif

// IR Remote control defaults
#ifndef INPUT_REMOTE_ID
#define INPUT_REMOTE_ID (0x01)
#endif

// Keyboard defaults
#ifndef INPUT_KEYBOARD_ID
#define INPUT_KEYBOARD_ID (0x02)
#endif

// Mouse event defaults
#ifndef INPUT_MOUSE_ID
#define INPUT_MOUSE_ID (0x02)
#endif

// Raw IR remote control defaults
#ifndef INPUT_RAW_REMOTE_ID
#define INPUT_RAW_REMOTE_ID (0x03)
#endif

typedef struct {
   uint32_t    ident;            // The ID of the event source (e.g. INPUT_KEYBOARD_ID)
   uint32_t    offset;           // The offset in shared memory, in bytes, where the
                                 // FIFO is located.
} input_dir_entry_t;

#define INPUT_DIR_ENTRY_SIZE   (8)     // bytes
#define INPUT_DIR_ENTRY_IDENT_OFFSET (0x00)
#define INPUT_DIR_ENTRY_OFFSET_OFFSET (0x04)

// There is one input_directory_t structure located at byte offset 0 in the shared memory. It
// contains information about the devices that may be sending input events and the location
// of the FIFO for each source.
typedef struct {
   uint32_t    magic;            // Marker will be (INPUT_MAGIC_HEADER << 16) + INPUT_MAGIC_BASE
   uint32_t    entry_count;      // Number of entries in entry[]
   input_dir_entry_t entry[2];   // Variable length array of directory entries, one per event source
                                 // After the last entry will be a single uint32_t with the same
                                 // value as the magic field at the beginning of this structure
} input_directory_t;

#define INPUT_DIR_MAGIC_OFFSET (0x00)
#define INPUT_DIR_COUNT_OFFSET (0x04)
#define INPUT_DIR_ENTRY_OFFSET (0x08)

typedef struct {
   uint32_t    type;             // EV_REP or EV_KEY
   uint32_t    code;             // One of the KEY_nnnn values from input_linux_events.h (VC)
                                 // or 2835-kernel-2.6.32/include/linux/input.h
} input_event_t;

#define INPUT_EVENT_TYPE_OFFSET  (0x00)
#define INPUT_EVENT_CODE_OFFSET  (0x04)
#define INPUT_EVENT_SIZE         (0x08)

typedef struct {
   uint32_t       magic;                  // Marker for FIFO. Upper bytes are INPUT_MAGIC_HEADER
                                          // lower bytes are ID of device (e.g. INPUT_KEYBOARD_ID)
   uint32_t       element_count;          // Number of elements in data[]
   uint32_t       index_in;               // Index into data[] for next event to be written
   uint32_t       index_out;              // Index into data[] for next event to be read
   input_event_t  data[INPUT_FIFO_SIZE];  // The event queue
} input_fifo_t;

#define INPUT_FIFO_MAGIC_OFFSET     (0x00)
#define INPUT_FIFO_COUNT_OFFSET     (0x04)
#define INPUT_FIFO_INDEX_IN_OFFSET  (0x08)
#define INPUT_FIFO_INDEX_OUT_OFFSET (0x0C)
#define INPUT_FIFO_DATA_OFFSET      (0x10)
#endif   // _INPUT_IPC_CFG_H_

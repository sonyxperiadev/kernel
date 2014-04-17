/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/*
*
*****************************************************************************
*
*  gist.h
*
*  PURPOSE:
*
*     This file contains the interface to the Generic I/O Streaming utility
*
*  NOTES:
*
*****************************************************************************/

#if !defined( GIST_H )
#define GIST_H

/* ---- Include Files ---------------------------------------- */
/* ---- Constants and Types ---------------------------------- */

#define GIST_FILENAME_MAXLEN 256
#define GIST_INVALID_HANDLE  0

#define GIST_READ_MODE_ONCE    0
#define GIST_READ_MODE_REPEAT  1
#define GIST_READ_MODE_RESERVED_ONCE    0x80
#define GIST_READ_MODE_RESERVED_REPEAT  0x81

#define GIST_WRITE_MODE_TRUNCATE  0
#define GIST_WRITE_MODE_APPEND    1

#define GIST_BUFFER_SIZE_USE_DEFAULT 0

/* reader parameters */
typedef struct {
	char filename[GIST_FILENAME_MAXLEN];	/* Name of file to read from */
	int mode;		/* Flag indicating if file should be
				 * repated from the beginning
				 * if the end is reached. */
	int buffer_size;	/* Size of buffer to use.  Specify
				 * GIST_BUFFER_SIZE_USE_DEFAULT to
				 * let GIST use the default size */
} GIST_READER_PARAMS;

/* writer parameters */
typedef struct {
	char filename[GIST_FILENAME_MAXLEN];	/* Name of the file to write to */
	int mode;		/* Flag indicating if existing files
				 * should be overwritten or appended to. */
	int buffer_size;	/* Size of buffer to use.  Specify
				 * GIST_BUFFER_SIZE_USE_DEFAULT to
				 * let GIST use the default size */
} GIST_WRITER_PARAMS;

#if defined( __KERNEL__ )
/* ---- Variable Externs ------------------------------------- */
/* ---- Function Prototypes ---------------------------------- */
extern int gist_alloc_reader(const GIST_READER_PARAMS *params);
extern int gist_alloc_writer(const GIST_WRITER_PARAMS *params);

extern int gist_free_reader(int handle);
extern int gist_free_writer(int handle);

extern int gist_read(int handle, int length, void *bufp);
extern int gist_write(int handle, int length, const void *bufp);

extern int gist_get_reader_params(int handle, GIST_READER_PARAMS *params);
extern int gist_get_writer_params(int handle, GIST_WRITER_PARAMS *params);

#endif /* __KERNEL__ */
#endif /* GIST_H */

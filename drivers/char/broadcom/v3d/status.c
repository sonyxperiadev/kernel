/*******************************************************************************
Copyright 2012 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/math64.h>

#include "driver.h"
#include "status.h"

#define STATUS_OUTPUT_BYTES (1 << 16)


/* ================================================================ */

typedef struct {
	char          *buffer;
	unsigned int   allocated;
	unsigned int   bytes;
	unsigned int (*get_output)(char *buffer, unsigned int bytes);
} proc_entry_t;

static unsigned int version_read(char *buffer, unsigned int bytes);
static unsigned int status_read(char  *buffer, unsigned int bytes);
static unsigned int session_read(char  *buffer, unsigned int bytes);

proc_entry_t session = {
	NULL,
	0,
	0,
	&session_read
};
proc_entry_t status = {
	NULL,
	0,
	0,
	&status_read
};
proc_entry_t version = {
	V3D_VERSION_STR,
	sizeof(V3D_VERSION_STR) - 1,
	sizeof(V3D_VERSION_STR) - 1,
	&version_read
};


/* ================================================================ */

static int proc_entry_verbose_read(
	char  *buffer,
	char **start,
	off_t  offset,
	int    bytes,
	int   *eo_f,
	void  *context)
{
	proc_entry_t *instance = (proc_entry_t *) context;
	unsigned int copy;
	if (offset == 0 || instance == NULL) {
		if (instance == NULL || instance->buffer == NULL) {
			*eo_f = 1;
			return 0;
		}

		/* Read the full output into our buffer and set the length */
		instance->bytes = (*instance->get_output)(instance->buffer, instance->allocated);
	}

	if (offset >= instance->allocated) {
		*eo_f = 1;
		return 0; /* No bytes written */
	}

	copy = min(instance->bytes - (unsigned int) offset, (unsigned int) bytes);
	*eo_f = copy == instance->bytes - offset ? 1 : 0;
	memcpy(*start = buffer, instance->buffer + offset, copy);
	return copy;
}

int dummy_read(
	char  *buffer,
	char **start,
	off_t  offset,
	int    bytes,
	int   *eo_f,
	void  *data)
{
	*eo_f = 1;
	return 0;
}

int dummy_write(
	struct file  *file,
	const char   *buffer,
	unsigned long bytes,
	void         *data)
{
	return 0;
}

struct proc_dir_entry *proc_entry_create(
	const char            *name,
	int                    permission,
	struct proc_dir_entry *directory,
	void                  *context)
{
	struct proc_dir_entry *entry = create_proc_entry(name, permission, directory);
	if (entry == NULL)
		return NULL;

	entry->read_proc  = (permission & (S_IRUSR | S_IRGRP)) != 0 ? proc_entry_verbose_read : &dummy_read;
	entry->write_proc = &dummy_write;
	entry->data       = context;
	return entry;
}

void proc_entry_delete(const char *name, struct proc_dir_entry *directory)
{
	remove_proc_entry(name, directory);
}


/* ================================================================ */

static unsigned int my_snprintf(
	char        *buffer,
	unsigned int count,
	const char  *format,
	...)
{
	va_list args;
	int     bytes;
	va_start(args, format);
	bytes = vsnprintf(buffer, count, format, args);
	va_end(args);
	if (bytes < 0 || bytes >= count /* Not enough space */)
		return 0;
	return (unsigned int) bytes;
}


/* ================================================================ */

static unsigned int statistics_output(
	char         *buffer,
	unsigned int  bytes,
	const char   *indent,
	unsigned int  samples,
	unsigned int  mean,
	unsigned int  standard_deviation,
	unsigned int  minimum,
	unsigned int  maximum)
{
	if (samples == 0)
		return my_snprintf(buffer, bytes, "%sNo samples\n", indent);

	if (minimum == maximum)
		return my_snprintf(
			buffer, bytes,
			"%s     %8u (%u sample%s)\n",
			indent,
			mean,
			samples, samples == 1 ? "" : "s");

	return my_snprintf(
		buffer, bytes,
		"%sMean %8u SD %8u Minimum %8u Maximum %8u (%u sample%s)\n",
		indent,
		mean,
		standard_deviation,
		minimum,
		maximum,
		samples, samples == 1 ? "" : "s");
}


/* ================================================================ */

static unsigned int version_read(char *buffer, unsigned int bytes)
{
	return bytes; /* Already there */
}

extern v3d_driver_t *v3d_driver;

typedef struct {
	uint64_t     mean;
	uint64_t     standard_deviation;
	unsigned int minimum;
	unsigned int maximum;
	unsigned int samples;
} calculated_statistics_t;

static void calculate_job_statistics(
	calculated_statistics_t *instance,
	statistics_t           *statistics)
{
	statistics_calculate(
		statistics,
		1, &instance->mean,
		1, &instance->standard_deviation,
		&instance->minimum, &instance->maximum, &instance->samples);
}

static uint32_t rounding_division64(uint64_t numerator, uint64_t denominator)
{
	return div64_u64(numerator + denominator / 2, denominator);
}

static int output_job_statistics(
	const char                     *description,
	const calculated_statistics_t *statistics,
	char                           *buffer,
	unsigned int                    bytes,
	unsigned int                   *offset,
	unsigned int                    elapsed)
{
	unsigned int job_rate;
	if (statistics[0].samples == 0 && statistics[1].samples == 0)
		return 0;

	job_rate = elapsed / 1000 != 0 ? rounding_division64(10000ULL * (uint64_t) statistics[1].samples, elapsed / 1000) : 0;
	*offset += my_snprintf(
		buffer + *offset, bytes - *offset,
		"  %-16s(%3u.%01u/s)\n",
		description,
		job_rate / 10, job_rate % 10);

	*offset += statistics_output(
		buffer + *offset, bytes - *offset, "   Queue to run (us): ",
		statistics[0].samples,
		(unsigned int) statistics[0].mean,
		(unsigned int) statistics[0].standard_deviation,
		statistics[0].minimum, statistics[0].maximum);
	*offset += statistics_output(
		buffer + *offset, bytes - *offset, "   Run time     (us): ",
		statistics[1].samples,
		(unsigned int) statistics[1].mean,
		(unsigned int) statistics[1].standard_deviation,
		statistics[1].minimum, statistics[1].maximum);
	return 1;
}

static unsigned int status_read(char *buffer, unsigned int bytes)
{
	/* Overall stats */
	ktime_t       now     = ktime_get();
	unsigned int  elapsed = ktime_us_delta(now, v3d_driver->start);
	unsigned int  run     = v3d_driver->total_run;
	unsigned int  load    = elapsed == 0 ? 0 : rounding_division64(1000ULL * (uint64_t) run, (uint64_t) elapsed);
	unsigned int  offset;
	unsigned long flags;
	calculated_statistics_t calculated_statistics[5];
	statistics_t           statistics[5];

	/* Copy-out the raw statistics data to minimise the time we're locked */
	spin_lock_irqsave(&v3d_driver->job.posted.lock, flags);
	statistics[0] = v3d_driver->bin_render.queue;
	statistics[1] = v3d_driver->bin_render.run;
	statistics[2] = v3d_driver->user.queue;
	statistics[3] = v3d_driver->user.run;
	statistics[4] = v3d_driver->bin_render.binning_bytes;
	v3d_driver_reset_statistics(v3d_driver);
	spin_unlock_irqrestore(&v3d_driver->job.posted.lock, flags);

	/* Now calculate the statistics */
	calculate_job_statistics(&calculated_statistics[0], &statistics[0]);
	calculate_job_statistics(&calculated_statistics[1], &statistics[1]);
	calculate_job_statistics(&calculated_statistics[2], &statistics[2]);
	calculate_job_statistics(&calculated_statistics[3], &statistics[3]);
	calculate_job_statistics(&calculated_statistics[4], &statistics[4]);

	offset = my_snprintf(
		buffer, bytes,
		" Overall                      load %3u.%01u%%\n", load / 10, load % 10);
	output_job_statistics("Bin/Render jobs", &calculated_statistics[0], buffer, bytes, &offset, elapsed);
	if (calculated_statistics[4].samples != 0)
		offset += statistics_output(
			buffer + offset, bytes - offset, "   Binning bytes    : ",
			calculated_statistics[4].samples,
			(unsigned int) calculated_statistics[4].mean,
			(unsigned int) calculated_statistics[4].standard_deviation,
			calculated_statistics[4].minimum, calculated_statistics[4].maximum);
	output_job_statistics("User jobs",       &calculated_statistics[2], buffer, bytes, &offset, elapsed);
	return offset;
}

static unsigned int session_read(char *buffer, unsigned int bytes)
{
	unsigned int offset;
	unsigned int i, load, elapsed;
	ktime_t      now;

	if (v3d_driver == NULL)
		return my_snprintf(buffer, bytes, "No V3D driver\n");

	offset = my_snprintf(buffer, bytes, "V3D sessions\n");

	/* Session stats */
	for (i = 0 ; i < sizeof(v3d_driver->sessions) / sizeof(v3d_driver->sessions[0]) ; ++i)
		if (v3d_driver->sessions[i] != NULL) {
			v3d_session_t *session = v3d_driver->sessions[i];
			calculated_statistics_t calculated_statistics[5];
			statistics_t           statistics[5];
			unsigned int    run     = session->total_run;
			unsigned long   flags;
			now     = ktime_get();
			elapsed = ktime_us_delta(now, session->start);
			load    = elapsed == 0 ? 0 : rounding_division64(1000ULL * (uint64_t) run, (uint64_t) elapsed);

			/* Copy-out the raw statistics data to minimise the time we're locked */
			spin_lock_irqsave(&v3d_driver->job.posted.lock, flags);
			statistics[0] = session->bin_render.queue;
			statistics[1] = session->bin_render.run;
			statistics[2] = session->user.queue;
			statistics[3] = session->user.run;
			statistics[4] = session->binning_bytes;
			v3d_session_reset_statistics(session);
			spin_unlock_irqrestore(&v3d_driver->job.posted.lock, flags);

			/* Now calculate the statistics */
			calculate_job_statistics(&calculated_statistics[0], &statistics[0]);
			calculate_job_statistics(&calculated_statistics[1], &statistics[1]);
			calculate_job_statistics(&calculated_statistics[2], &statistics[2]);
			calculate_job_statistics(&calculated_statistics[3], &statistics[3]);
			calculate_job_statistics(&calculated_statistics[4], &statistics[4]);

			offset += my_snprintf(
				buffer + offset, bytes - offset,
				" Session %-20s load %3u.%01u%%\n",
				session->name != NULL ? session->name : "unknown",
				load / 10, load % 10);
			output_job_statistics("Bin/Render jobs", &calculated_statistics[0], buffer, bytes, &offset, elapsed);
			if (calculated_statistics[4].samples != 0)
				offset += statistics_output(
					buffer + offset, bytes - offset, "   Binning bytes    : ",
					calculated_statistics[4].samples,
					(unsigned int) calculated_statistics[4].mean,
					(unsigned int) calculated_statistics[4].standard_deviation,
					calculated_statistics[4].minimum, calculated_statistics[4].maximum);
			output_job_statistics("User jobs",       &calculated_statistics[2], buffer, bytes, &offset, elapsed);
		}

	offset += status_read(buffer + offset, bytes - offset);
	return offset;
}


/* ================================================================ */

int v3d_driver_create_proc_entries(v3d_driver_t *instance)
{
	instance->proc.initialised = 0;

	status.buffer = kmalloc(status.allocated = STATUS_OUTPUT_BYTES, GFP_KERNEL);
	if (status.buffer == NULL)
		return v3d_driver_delete_proc_entries(instance), -1;
	++instance->proc.initialised;

	session.buffer = kmalloc(session.allocated = STATUS_OUTPUT_BYTES, GFP_KERNEL);
	if (session.buffer == NULL)
		return v3d_driver_delete_proc_entries(instance), -2;
	++instance->proc.initialised;

	instance->proc.directory = proc_mkdir(V3D_DEV_NAME, NULL);
	if (instance->proc.directory == NULL)
		return v3d_driver_delete_proc_entries(instance), -3;
	++instance->proc.initialised;

	instance->proc.status = proc_entry_create("version", S_IRUSR | S_IRGRP | S_IROTH, instance->proc.directory, &version);
	if (instance->proc.status == NULL)
		return v3d_driver_delete_proc_entries(instance), -4;
	++instance->proc.initialised;

	instance->proc.status = proc_entry_create("status", S_IRUSR | S_IRGRP | S_IROTH, instance->proc.directory, &status);
	if (instance->proc.status == NULL)
		return v3d_driver_delete_proc_entries(instance), -5;
	++instance->proc.initialised;

	instance->proc.session = proc_entry_create("session", S_IRUSR | S_IRGRP | S_IROTH, instance->proc.directory, &session);
	if (instance->proc.session == NULL)
		return v3d_driver_delete_proc_entries(instance), -6;
	++instance->proc.initialised;

	return 0;
}

void v3d_driver_delete_proc_entries(v3d_driver_t *instance)
{
	switch (instance->proc.initialised) {
	case 6:
		proc_entry_delete("session", instance->proc.directory);

	case 5:
		proc_entry_delete("status", instance->proc.directory);

	case 4:
		proc_entry_delete("version", instance->proc.directory);

	case 3:
		remove_proc_entry(V3D_DEV_NAME, NULL);

	case 2:
		kfree(session.buffer);

	case 1:
		kfree(status.buffer);

	case 0:
		break;
	}
}

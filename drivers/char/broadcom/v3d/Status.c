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

#include "Driver.h"
#include "Status.h"

#define STATUS_OUTPUT_BYTES (1 << 16)


/* ================================================================ */

typedef struct {
	char          *Buffer;
	unsigned int   Allocated;
	unsigned int   Bytes;
	unsigned int (*GetOutput)(char *Buffer, unsigned int Bytes);
} ProcEntryType;

static unsigned int VersionRead(char *Buffer, unsigned int Bytes);
static unsigned int StatusRead(char  *Buffer, unsigned int Bytes);
static unsigned int SessionRead(char  *Buffer, unsigned int Bytes);

ProcEntryType Session = {
	NULL,
	0,
	0,
	&SessionRead
};
ProcEntryType Status = {
	NULL,
	0,
	0,
	&StatusRead
};
ProcEntryType Version = {
	V3D_VERSION_STR,
	sizeof(V3D_VERSION_STR) - 1,
	sizeof(V3D_VERSION_STR) - 1,
	&VersionRead
};


/* ================================================================ */

static int ProcEntry_VerboseRead(
	char  *Buffer,
	char **Start,
	off_t  Offset,
	int    Bytes,
	int   *EoF,
	void  *Context)
{
	ProcEntryType *Instance = (ProcEntryType *) Context;
	unsigned int Copy;
	if (Offset == 0 || Instance == NULL) {
		if (Instance == NULL || Instance->Buffer == NULL) {
			*EoF = 1;
			return 0;
		}

		/* Read the full output into our buffer and set the length */
		Instance->Bytes = (*Instance->GetOutput)(Instance->Buffer, Instance->Allocated);
	}

	if (Offset >= Instance->Allocated) {
		*EoF = 1;
		return 0; /* No bytes written */
	}

	Copy = min(Instance->Bytes - (unsigned int) Offset, (unsigned int) Bytes);
	*EoF = Copy == Instance->Bytes - Offset ? 1 : 0;
	memcpy(*Start = Buffer, Instance->Buffer + Offset, Copy);
	return Copy;
}

int DummyRead(
	char  *Buffer,
	char **Start,
	off_t  Offset,
	int    Bytes,
	int   *EoF,
	void  *Data)
{
	*EoF = 1;
	return 0;
}

int DummyWrite(
	struct file  *File,
	const char   *Buffer,
	unsigned long Bytes,
	void         *Data)
{
	return 0;
}

struct proc_dir_entry *ProcEntry_Create(
	const char            *Name,
	int                    Permission,
	struct proc_dir_entry *Directory,
	void                  *Context)
{
	struct proc_dir_entry *Entry = create_proc_entry(Name, Permission, Directory);
	if (Entry == NULL)
		return NULL;

	Entry->read_proc  = (Permission & (S_IRUSR | S_IRGRP)) != 0 ? ProcEntry_VerboseRead : &DummyRead;
	Entry->write_proc = &DummyWrite;
	Entry->data       = Context;
	return Entry;
}

void ProcEntry_Delete(const char *Name, struct proc_dir_entry *Directory)
{
	remove_proc_entry(Name, Directory);
}


/* ================================================================ */

static unsigned int MySNPrintf(
	char        *Buffer,
	unsigned int Count,
	const char  *Format,
	...)
{
	va_list Args;
	int     Bytes;
	va_start(Args, Format);
	Bytes = vsnprintf(Buffer, Count, Format, Args);
	va_end(Args);
	if (Bytes < 0 || Bytes >= Count /* Not enough space */)
		return 0;
	return (unsigned int) Bytes;
}


/* ================================================================ */

static unsigned int Statistics_Output(
	char         *Buffer,
	unsigned int  Bytes,
	const char   *Indent,
	unsigned int  Samples,
	unsigned int  Mean,
	unsigned int  StandardDeviation,
	unsigned int  Minimum,
	unsigned int  Maximum)
{
	if (Samples == 0)
		return MySNPrintf(Buffer, Bytes, "%sNo samples\n", Indent);

	if (Minimum == Maximum)
		return MySNPrintf(
			Buffer, Bytes,
			"%s     %6u (%u sample%s)\n",
			Indent,
			Mean,
			Samples, Samples == 1 ? "" : "s");

	return MySNPrintf(
		Buffer, Bytes,
		"%sMean %6u SD %6u Minimum %6u Maximum %6u (%u sample%s)\n",
		Indent,
		Mean,
		StandardDeviation,
		Minimum,
		Maximum,
		Samples, Samples == 1 ? "" : "s");
}


/* ================================================================ */

static unsigned int VersionRead(char *Buffer, unsigned int Bytes)
{
	return Bytes; /* Already there */
}

extern V3dDriverType *v3d_driver;

typedef struct {
	uint64_t     Mean;
	uint64_t     StandardDeviation;
	unsigned int Minimum;
	unsigned int Maximum;
	unsigned int Samples;
} CalculatedStatisticsType;

static void CalculateJobStatistics(
	CalculatedStatisticsType *Instance,
	StatisticsType           *Statistics)
{
	Statistics_Calculate(
		Statistics,
		1, &Instance->Mean,
		1, &Instance->StandardDeviation,
		&Instance->Minimum, &Instance->Maximum, &Instance->Samples);
}

static uint32_t RoundingDivision64(uint64_t Numerator, uint64_t Denominator)
{
	return div64_u64(Numerator + Denominator / 2, Denominator);
}

static int OutputJobStatistics(
	const char                     *Description,
	const CalculatedStatisticsType *Statistics,
	char                           *Buffer,
	unsigned int                    Bytes,
	unsigned int                   *Offset,
	unsigned int                    Elapsed)
{
	unsigned int JobRate;
	if (Statistics[0].Samples == 0 && Statistics[1].Samples == 0)
		return 0;

	JobRate = Elapsed / 1000 != 0 ? RoundingDivision64(10000ULL * (uint64_t) Statistics[1].Samples, Elapsed / 1000) : 0;
	*Offset += MySNPrintf(
		Buffer + *Offset, Bytes - *Offset,
		"  %-16s(%3u.%01u/s)\n",
		Description,
		JobRate / 10, JobRate % 10);

	*Offset += Statistics_Output(
		Buffer + *Offset, Bytes - *Offset, "   Queue to run (us): ",
		Statistics[0].Samples,
		(unsigned int) Statistics[0].Mean,
		(unsigned int) Statistics[0].StandardDeviation,
		Statistics[0].Minimum, Statistics[0].Maximum);
	*Offset += Statistics_Output(
		Buffer + *Offset, Bytes - *Offset, "   Run Time     (us): ",
		Statistics[1].Samples,
		(unsigned int) Statistics[1].Mean,
		(unsigned int) Statistics[1].StandardDeviation,
		Statistics[1].Minimum, Statistics[1].Maximum);
	return 1;
}

static unsigned int StatusRead(char *Buffer, unsigned int Bytes)
{
	/* Overall stats */
	ktime_t       Now     = ktime_get();
	unsigned int  Elapsed = ktime_us_delta(Now, v3d_driver->Start);
	unsigned int  Run     = v3d_driver->TotalRun;
	unsigned int  Load    = Elapsed == 0 ? 0 : RoundingDivision64(1000ULL * (uint64_t) Run, (uint64_t) Elapsed);
	unsigned int  Offset;
	unsigned long Flags;
	CalculatedStatisticsType CalculatedStatistics[4];
	StatisticsType           Statistics[4];

	/* Copy-out the raw statistics data to minimise the time we're locked */
	spin_lock_irqsave(&v3d_driver->Job.Posted.Lock, Flags);
	Statistics[0] = v3d_driver->BinRender.Queue;
	Statistics[1] = v3d_driver->BinRender.Run;
	Statistics[2] = v3d_driver->User.Queue;
	Statistics[3] = v3d_driver->User.Run;
	V3dDriver_ResetStatistics(v3d_driver);
	spin_unlock_irqrestore(&v3d_driver->Job.Posted.Lock, Flags);

	/* Now calculate the statistics */
	CalculateJobStatistics(&CalculatedStatistics[0], &Statistics[0]);
	CalculateJobStatistics(&CalculatedStatistics[1], &Statistics[1]);
	CalculateJobStatistics(&CalculatedStatistics[2], &Statistics[2]);
	CalculateJobStatistics(&CalculatedStatistics[3], &Statistics[3]);

	Offset = MySNPrintf(
		Buffer, Bytes,
		" Overall                      load %3u.%01u%%\n", Load / 10, Load % 10);
	OutputJobStatistics("Bin/Render Jobs", &CalculatedStatistics[0], Buffer, Bytes, &Offset, Elapsed);
	OutputJobStatistics("User Jobs",       &CalculatedStatistics[2], Buffer, Bytes, &Offset, Elapsed);
	return Offset;
}

static unsigned int SessionRead(char *Buffer, unsigned int Bytes)
{
	unsigned int Offset;
	unsigned int i, Load, Elapsed;
	ktime_t      Now;

	if (v3d_driver == NULL)
		return MySNPrintf(Buffer, Bytes, "No V3D driver\n");

	Offset = MySNPrintf(Buffer, Bytes, "V3D Sessions\n");

	/* Session stats */
	for (i = 0 ; i < sizeof(v3d_driver->Sessions) / sizeof(v3d_driver->Sessions[0]) ; ++i)
		if (v3d_driver->Sessions[i] != NULL) {
			V3dSessionType *Session = v3d_driver->Sessions[i];
			CalculatedStatisticsType CalculatedStatistics[4];
			StatisticsType           Statistics[4];
			unsigned int    Run     = Session->TotalRun;
			unsigned long   Flags;
			Now     = ktime_get();
			Elapsed = ktime_us_delta(Now, Session->Start);
			Load    = Elapsed == 0 ? 0 : RoundingDivision64(1000ULL * (uint64_t) Run, (uint64_t) Elapsed);

			/* Copy-out the raw statistics data to minimise the time we're locked */
			spin_lock_irqsave(&v3d_driver->Job.Posted.Lock, Flags);
			Statistics[0] = Session->BinRender.Queue;
			Statistics[1] = Session->BinRender.Run;
			Statistics[2] = Session->User.Queue;
			Statistics[3] = Session->User.Run;
			V3dSession_ResetStatistics(Session);
			spin_unlock_irqrestore(&v3d_driver->Job.Posted.Lock, Flags);

			/* Now calculate the statistics */
			CalculateJobStatistics(&CalculatedStatistics[0], &Statistics[0]);
			CalculateJobStatistics(&CalculatedStatistics[1], &Statistics[1]);
			CalculateJobStatistics(&CalculatedStatistics[2], &Statistics[2]);
			CalculateJobStatistics(&CalculatedStatistics[3], &Statistics[3]);

			Offset += MySNPrintf(
				Buffer + Offset, Bytes - Offset,
				" Session %-20s load %3u.%01u%%\n",
				Session->Name != NULL ? Session->Name : "unknown",
				Load / 10, Load % 10);
			OutputJobStatistics("Bin/Render Jobs", &CalculatedStatistics[0], Buffer, Bytes, &Offset, Elapsed);
			OutputJobStatistics("User Jobs",       &CalculatedStatistics[2], Buffer, Bytes, &Offset, Elapsed);
		}

	Offset += StatusRead(Buffer + Offset, Bytes - Offset);
	return Offset;
}


/* ================================================================ */

int V3dDriver_CreateProcEntries(V3dDriverType *Instance)
{
	Instance->Proc.Initialised = 0;

	Status.Buffer = kmalloc(Status.Allocated = STATUS_OUTPUT_BYTES, GFP_KERNEL);
	if (Status.Buffer == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -1;
	++Instance->Proc.Initialised;

	Session.Buffer = kmalloc(Session.Allocated = STATUS_OUTPUT_BYTES, GFP_KERNEL);
	if (Session.Buffer == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -2;
	++Instance->Proc.Initialised;

	Instance->Proc.Directory = proc_mkdir(V3D_DEV_NAME, NULL);
	if (Instance->Proc.Directory == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -3;
	++Instance->Proc.Initialised;

	Instance->Proc.Status = ProcEntry_Create("version", S_IRUSR | S_IRGRP | S_IROTH, Instance->Proc.Directory, &Version);
	if (Instance->Proc.Status == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -4;
	++Instance->Proc.Initialised;

	Instance->Proc.Status = ProcEntry_Create("status", S_IRUSR | S_IRGRP | S_IROTH, Instance->Proc.Directory, &Status);
	if (Instance->Proc.Status == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -5;
	++Instance->Proc.Initialised;

	Instance->Proc.Session = ProcEntry_Create("session", S_IRUSR | S_IRGRP | S_IROTH, Instance->Proc.Directory, &Session);
	if (Instance->Proc.Session == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -6;
	++Instance->Proc.Initialised;

	return 0;
}

void V3dDriver_DeleteProcEntries(V3dDriverType *Instance)
{
	switch (Instance->Proc.Initialised) {
	case 6:
		ProcEntry_Delete("session", Instance->Proc.Directory);

	case 5:
		ProcEntry_Delete("status", Instance->Proc.Directory);

	case 4:
		ProcEntry_Delete("version", Instance->Proc.Directory);

	case 3:
		remove_proc_entry(V3D_DEV_NAME, NULL);

	case 2:
		kfree(Session.Buffer);

	case 1:
		kfree(Status.Buffer);

	case 0:
		break;
	}
}

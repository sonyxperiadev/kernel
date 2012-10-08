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

static int ProcEntry_VerboseRead(char *Buffer, char **Start, off_t Offset, int Bytes, int *EoF, void *Context)
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

int DummyRead(char *Buffer, char **Start, off_t Offset, int Bytes, int *EoF, void *Data)
{
	*EoF = 1;
	return 0;
}

int DummyWrite(struct file *File, const char *Buffer, unsigned long Bytes, void *Data)
{
	return 0;
}

struct proc_dir_entry *ProcEntry_Create(const char *Name, int Permission, struct proc_dir_entry *Directory, void *Context)
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

static unsigned int MySNPrintf(char *Buffer, unsigned int Count, const char *Format, ...)
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
} MyStatisticsType;

static void CalculateJobStatistics(MyStatisticsType *Instance, StatisticsType *Statistics)
{
	Statistics_Calculate(
		Statistics,
		1, &Instance->Mean,
		1, &Instance->StandardDeviation,
		&Instance->Minimum, &Instance->Maximum, &Instance->Samples);
}

static void OutputJobStatistics(const MyStatisticsType *Statistics, char *Buffer, unsigned int Bytes, unsigned int *Offset)
{
	if (Statistics[0].Samples == 0 && Statistics[1].Samples == 0) {
		*Offset += MySNPrintf(Buffer + *Offset, Bytes - *Offset, "   Idle\n");
		return;
	}

	*Offset += Statistics_Output(
		Buffer + *Offset, Bytes - *Offset, "   Queue to Start (us): ",
		Statistics[0].Samples,
		(unsigned int) Statistics[0].Mean, (unsigned int) Statistics[0].StandardDeviation,
		Statistics[0].Minimum, Statistics[0].Maximum);
	*Offset += Statistics_Output(
		Buffer + *Offset, Bytes - *Offset, "   Run Time       (us): ",
		Statistics[1].Samples,
		(unsigned int) Statistics[1].Mean, (unsigned int) Statistics[1].StandardDeviation,
		Statistics[1].Minimum, Statistics[1].Maximum);
}

static unsigned int StatusRead(char *Buffer, unsigned int Bytes)
{
	MyStatisticsType Statistics[2];
	unsigned int Offset;
	unsigned int i, Load, Elapsed;
	ktime_t      Now;

	if (v3d_driver == NULL)
		return MySNPrintf(Buffer, Bytes, "No V3D driver\n");

	Offset = MySNPrintf(Buffer, Bytes, "V3D Status\n");

	/* Locking intentionally not done */

	/* Session stats */
	for (i = 0 ; i < sizeof(v3d_driver->Sessions) / sizeof(v3d_driver->Sessions[0]) ; ++i)
		if (v3d_driver->Sessions[i] != NULL) {
			V3dSessionType *Session = v3d_driver->Sessions[i];
			Now     = ktime_get();
			Elapsed = ktime_us_delta(Now, Session->Start);
			Load    = Elapsed == 0 ? 0 : 100 * Session->TotalRun / Elapsed;

			Offset += MySNPrintf(
				Buffer + Offset, Bytes - Offset,
				" Session %-20s load %3u%%\n",
				Session->Name != NULL ? Session->Name : "unknown",
				Load);

			CalculateJobStatistics(&Statistics[0], &Session->BinRender.Queue);
			CalculateJobStatistics(&Statistics[1], &Session->BinRender.Run);
			Offset += MySNPrintf(Buffer + Offset, Bytes - Offset, "  Bin/Render Jobs\n");
			OutputJobStatistics(Statistics, Buffer, Bytes, &Offset);

			CalculateJobStatistics(&Statistics[0], &Session->User.Queue);
			CalculateJobStatistics(&Statistics[1], &Session->User.Run);
			Offset += MySNPrintf(Buffer + Offset, Bytes - Offset, "  User Jobs\n");
			OutputJobStatistics(Statistics, Buffer, Bytes, &Offset);

			V3dSession_ResetStatistics(Session);
		}

	/* Overall stats */
	Now     = ktime_get();
	Elapsed = ktime_us_delta(Now, v3d_driver->Start);
	Load    = Elapsed == 0 ? 0 : 100 * v3d_driver->TotalRun / Elapsed;

	Offset += MySNPrintf(
		Buffer + Offset, Bytes - Offset,
		"\n Overall load %3u%%\n",
		Load);
	CalculateJobStatistics(&Statistics[0], &v3d_driver->BinRender.Queue);
	CalculateJobStatistics(&Statistics[1], &v3d_driver->BinRender.Run);
	Statistics_Initialise(&v3d_driver->BinRender.Queue);
	Statistics_Initialise(&v3d_driver->BinRender.Run);

	Offset += MySNPrintf(Buffer + Offset, Bytes - Offset, "  Bin/Render Jobs\n");
	OutputJobStatistics(Statistics, Buffer, Bytes, &Offset);

	CalculateJobStatistics(&Statistics[0], &v3d_driver->User.Queue);
	CalculateJobStatistics(&Statistics[1], &v3d_driver->User.Run);
	Statistics_Initialise(&v3d_driver->User.Queue);
	Statistics_Initialise(&v3d_driver->User.Run);

	Offset += MySNPrintf(Buffer + Offset, Bytes - Offset, "  User Jobs\n");
	OutputJobStatistics(Statistics, Buffer, Bytes, &Offset);

	V3dDriver_ResetStatistics(v3d_driver);

	return Offset;
}


/* ================================================================ */

int V3dDriver_CreateProcEntries(V3dDriverType *Instance)
{
	Instance->Proc.Initialised = 0;

	Status.Buffer = kmalloc(Status.Allocated = STATUS_OUTPUT_BYTES, GFP_KERNEL);
	if (Status.Buffer == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -3;
	++Instance->Proc.Initialised;

	Instance->Proc.Directory = proc_mkdir(V3D_DEV_NAME, NULL);
	if (Instance->Proc.Directory == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -1;
	++Instance->Proc.Initialised;

	Instance->Proc.Status = ProcEntry_Create("version", S_IRUSR | S_IRGRP | S_IROTH, Instance->Proc.Directory, &Version);
	if (Instance->Proc.Status == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -2;
	++Instance->Proc.Initialised;

	Instance->Proc.Status = ProcEntry_Create("status", S_IRUSR | S_IRGRP | S_IROTH, Instance->Proc.Directory, &Status);
	if (Instance->Proc.Status == NULL)
		return V3dDriver_DeleteProcEntries(Instance), -4;
	++Instance->Proc.Initialised;

	return 0;
}

void V3dDriver_DeleteProcEntries(V3dDriverType *Instance)
{
	switch (Instance->Proc.Initialised) {
	case 4:
		ProcEntry_Delete("status", Instance->Proc.Directory);

	case 3:
		ProcEntry_Delete("version", Instance->Proc.Directory);

	case 2:
		remove_proc_entry(V3D_DEV_NAME, NULL);

	case 1:
		kfree(Status.Buffer);

	case 0:
		break;
	}
}

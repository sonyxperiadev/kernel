/* **FIXME** stubs for header files not integrated from CP (not required for AP build) */
#ifndef _DEV_SYSPARM_H_
#define _DEV_SYSPARM_H_

#include "ostypes.h"

#define SYS_IMEI_LEN            8

typedef enum
{
	IMEI_1,	// Default IMEI
	IMEI_2	// IMEI associated with second SIM for dual-SIM
} IMEI_TYPE_t;

#endif //_DEV_SYSPARM_H_
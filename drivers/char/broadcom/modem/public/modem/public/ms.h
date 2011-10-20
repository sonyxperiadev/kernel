/****************************************************************************
*
*     Copyright (c) 2005 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/

#ifndef __MS_H__
#define __MS_H__


/* GNATS TR3147 yaxin use external definition in external_synonym.h to have flexibility in constant definitions */
#include "external_synonym.h"

#if defined SIMULATION_dSH /* simulation build */
// mcheng 11/03/01: Combine the codebase for target and simulation build

#ifndef USIM_UNIT_TESTING
#include "ms-sdtenv.h"
#endif /* USIM_UNIT_TESTING */

#define	GET_INSIGNALPTR(p)		(p->yInSignalPtr) // Needed for simulation also.

#elif defined UHT_HOST_BUILD /* UHT Stack host test */

/*Dung*/
//5.1 doesnt see the types definition
#include "mobcom_types.h"
//5.1 doesnt see the PRIO definition
#include "ms_host_prio.h"

#define	GET_INSIGNALPTR(p)		(p->Signal)
#include "scttypes.h"
#include "ms_host.h"

#define T_TIMESLOT_MASK T_TIMESLOT_ALLOCATION

#else /* target build */
// THIS FILE DEFINES SOME SYMBOLS THAT GOT "LOST" IN GOING FROM THE
// OLD STACK TO THE NEW: THESE WILL HAVE TO BE RESOLBED

#define	GET_INSIGNALPTR(p)		(p->yInSignalPtr)

#include "scttypes.h"

// #define xSignalNode  SDL_Integer

// Undefine MACROS that are used by both WIN32 and TARGET
// environments.  This removes many warnings that occur
// when compiling the L1 simulation.

#ifdef TRANSPARENT
#undef TRANSPARENT
#endif
#ifdef SERVICE_DISABLED
#undef SERVICE_DISABLED
#endif

#define T_TIMESLOT_MASK T_TIMESLOT_ALLOCATION


#include "msnu.h"

typedef SDL_Integer  T_ACCESS_BURST_TYPE ;

typedef SDL_PId	PId_t;

#include "ctypes.h"
#include "nas.h"
#include "l3.h"
#include "common_sim.h"

extern SDL_Duration SDL_Duration_Lit (xint32 secs, xint32 nsecs);
extern long SDL_Duration_seconds( SDL_Duration duration );

extern UInt8 MS_GetProductionTestMode(void);
extern Boolean SIM_IsTestSIM(void);
extern SIMType_t  SIM_GetSIMType( void );

extern Boolean usimap_is_test_sim(void);
extern Boolean ue_get_production_test_mode(void);



/* callback function. Returns TRUE if CAPI is able to read the stored data successfully*/
typedef SDL_Boolean (*MMStoredDataCb_t)(                                                  // Callback to retrieve/store USIM related EFs
                T_STORED_DATA_OP_TYPE        oper_type, T_STORED_PS_PARAMS *stored_ps_params_ptr
                );

void MM_Register(									// Register call-back functions
	MMStoredDataCb_t MMStoreData_cb					// callback function to operate on the USIM data
	);


#endif /* SDTENV */

#endif


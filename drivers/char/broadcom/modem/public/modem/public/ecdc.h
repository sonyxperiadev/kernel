//****************** Copyright 2000 Mobilink Telecom, Inc. *********************
//
// Description:  This include file contains the functions associated . . .
//
// Reference:  N/A
//
// $RCSfile: ecdc.h $
// $Revision: 1.7 $
// $Date: 2001/03/13 10:59:14 $
// $Author: reichler $
//
//******************************** History *************************************
//
// $Log: ecdc.h $
// Revision 1.7  2001/03/13 10:59:14  reichler
// Added missing prototype for retrieving compression mode (direction)
// Revision 1.6  2001/01/17 22:30:40Z  reichler
// Added 2nd arg to MakeLinkReq for connection element - ECDC
// needs to know about non-transparent calls that require data
// compression.  
// Revision 1.5  2000/12/08 19:40:21Z  reichler
// Added function for ATC to query compression engine maximums
// Revision 1.4  2000/05/11 23:49:08Z  reichler
// Sense of 'initiator' versus 'acceptor' is opposite that of bit-field 
// used to track this mode.  Changed enum so that 'initiator' is a 1 as
// is bit 7 of byte 7 of Option Information Structure (OIS).  
// Revision 1.3  2000/04/28 16:55:08Z  dlwin
// Removed the used of 'ecdccmd.h', so that stuff is moved to here.
// Revision 1.2  2000/03/10 21:35:18Z  dlwin
// Group check-in so that Marc can start the PC software from a
// common baseline.
// Revision 1.1  2000/01/21 17:01:47Z  dlwin
// Initial revision
//
//******************************************************************************

#ifndef _ECDC_ECDC_H_
#define _ECDC_ECDC_H_

#include "dlc.h"
#include "common_ecdc.h"

//******************************************************************************
//		Definition
//******************************************************************************

//******************************************************************************
//		Typedefs
//******************************************************************************
//******************************************************************************
//	Error Correction Stuff
//******************************************************************************

typedef enum
{
	ECDCSTATUS_SUCCESS,
	ECDCSTATUS_FAIL
} ECDCStatus_t;


typedef enum
{
	ECDCLINKMODE_ACCEPTOR,			// OIS_7(BIT_7) == 0, then acceptor
	ECDCLINKMODE_INITIATOR			// OIS_7(BIT_7) == 1, then initiator
} ECDCLinkMode_t;


typedef enum
{
	ECBREAKTYPE_IGNORE = 0,			// Break=Ignored
	ECBREAKTYPE_NON_EXP_NON_DEST = 1,// Break=Non-expedited/Non-destructive
	ECBREAKTYPE_EXP_NON_DEST = 2,  	// Break=Expedited/Non-destructive
	ECBREAKTYPE_EXP_DEST = 3	   	// Break=Expedited/Destructive
} ECBreakType_t;					// Break Type, see AT+EB

typedef enum
{
	ECBREAKTIMED_NO_LENGTH = 0,		// No Break Length enforced
	ECBREAKTIMED_LENGTH = 1			// Break Length enforced
} ECBreakTimed_t;					// Break Timed, see AT+EB

typedef UInt8 ECBreakLength_t;		// Break Length, see AT+EB

typedef enum
{
	ECSELREPEAT_NOT_USE_SREJ = 0,	// Do not use SREJ
	ECSELREPEAT_USE_SREJ_CONT = 1,	// Use SREJ if avail., if not avail. continue
	ECSELREPEAT_USE_SREJ_DISC = 2	// Use SREJ if avail., if not avail. disconnect
} ECSelRepeat_t;					// Selective Repeat, see AT+ESR

typedef enum
{
	ECFCSTYPE_16BIT = 0,	 		// Use 16 Bit FCS
	ECFCSTYPE_32BIT_CONT = 1,		// Use 32 Bit FCS, if not avail. use 16 Bit
	ECFCSTYPE_32BIT_DISC = 2 		// Use 32 Bit FCS, if not avail. disconnect
} ECFCSType_t;						// FCS Type, see AT+EFCS

typedef UInt8 ECWindowSize_t;		// Window Size, see AT+EWIND
typedef UInt16 ECFrameLength_t;		// Frame Length, see AT+EFRAM

typedef enum {
	ECDCLINK_TRANSPARENT	= 0,	// Connection element is transparent
	ECDCLINK_NONTRANSPARENT	= 1		// Connection element is non-transparent
} ECDCLinkType_t;

//******************************************************************************
//	Data Compression Stuff
//******************************************************************************

typedef UInt16 DCMxDictionarySize_t;	// Mx Dictionary Size (P1), see AT+DS
typedef UInt8 DCMxStringSize_t;		// Mx String Size (P2), see AT+DS


//******************************************************************************
//		Define CallBack Functions
//******************************************************************************

typedef void (*CB_ReportMakeLinkRst_t)(	// Make Link Request Result
	ECDCStatus_t result,				// Result of Make Link Request
	ECMode_t ec_est_mode,			// Error Correction mode
	DCMode_t dc_mode				// Data Compression mode
	);
	
typedef void (*CB_ReportStopLinkRst_t)( // Stop Link Request Result
	ECDCStatus_t result				// Result of Stop Link Request
	);

typedef void (*CB_ReportHaltLinkRst_t)( // Halt Link Request Result
	ECDCStatus_t result				// Result of Halt Link Request
	);

typedef void (*CB_ReportStartDataRst_t)( // Start Data Request Result
	ECDCStatus_t result				// Result of Start Data Request
	);

typedef void (*CB_ReportStopDataRst_t)( // Stop Data Request Result
	ECDCStatus_t result				// Result of Stop Data Request
	);

typedef void (*CB_ReportConnectInd_t)( // Connect Indication
	void
	);

//******************************************************************************
//		Prototypes
//******************************************************************************

void ECDC_Init(
	void
	);

void ECDC_Run(
	void
	);

void ECDC_Shutdown(
	void
	);

void ECDC_Register(
	CB_ReportConnectInd_t ReportConnectInd_cb,	// Connect Indication
	CB_ReportMakeLinkRst_t ReportMakeLinkRst_cb, // Make Link Request Result
	CB_ReportStopLinkRst_t ReportStopLinkRst_cb, // Stop Link Request Result
	CB_ReportHaltLinkRst_t ReportHaltLinkRst_cb, // Halt Link Request Result
	CB_ReportStartDataRst_t ReportStartDataRst_cb, // Start Data Request Result
	CB_ReportStopDataRst_t ReportStopDataRst_cb // Stop Data Request Result
	);

void ECDC_GetNextTick(
	void
	);

void ECDC_StartTick(
	void
	);

void ECDC_StopTick(
	void
	);

void ECDC_SetAttachedDLC(
	DLC_t dlc
	);


ECDCStatus_t ECDC_SetATES(
	ECOrigReq_t orig_req, 				// Originator Request
	ECOrigFB_t orig_fb,					// Originator Fallback
	ECAnsFB_t ans_fb					// Answer Fallback
	);

void ECDC_GetATES(
	ECOrigReq_t *orig_req, 				// Originator Request
	ECOrigFB_t *orig_fb,				// Originator Fallback
	ECAnsFB_t *ans_fb					// Answer Fallback
	);


ECDCStatus_t ECDC_SetATEB(
	ECBreakType_t break_type,			// Break Type
	ECBreakTimed_t break_timed,			// Break Timed
	ECBreakLength_t break_length		// Break Length
	);

void ECDC_GetATEB(
	ECBreakType_t *break_type,			// Break Type
	ECBreakTimed_t *break_timed,		// Break Timed
	ECBreakLength_t *break_length		// Break Length
	);

ECDCStatus_t ECDC_SetATESR(
	ECSelRepeat_t sel_repeat			// Selective Repeat
	);

void ECDC_GetATESR(
	ECSelRepeat_t *sel_repeat			// Selective Repeat
	);

ECDCStatus_t ECDC_SetATEFCS(
	ECFCSType_t fcs_type				// FCS Type
	);

void ECDC_GetATEFCS(
	ECFCSType_t *fcs_type				// FCS Type
	);

ECDCStatus_t ECDC_SetATETBM(
	void								// ????
	);

void ECDC_GetATETBM(
	void								// ????
	);	

ECDCStatus_t ECDC_SetATEWIND(
	ECWindowSize_t window_size			// Window Size
	);

void ECDC_GetATEWIND(
	ECWindowSize_t *window_size			// Window Size
	);

ECDCStatus_t ECDC_SetATEFRAM(
	ECFrameLength_t frame_length		// Frame Length
	);

void ECDC_GetATEFRAM(
	ECFrameLength_t *frame_length		// Frame Length
	);

ECDCStatus_t ECDC_SetATDS(
	DCMode_t dc_mode,					// Data Compression Mode (P0)
	DCMxDictionarySize_t mx_dict_sz,	// Mx Dictionary Size (P1)
	DCMxStringSize_t mx_string_sz		// Mx String Size (P2)
	);

void ECDC_GetATDS(
	DCMode_t *dc_mode,					// Data Compression Mode (P0)
	DCMxDictionarySize_t *mx_dict_sz,	// Mx Dictionary Size (P1)
	DCMxStringSize_t *mx_string_sz		// Mx String Size (P2)
	);

void ECDC_GetHwATDS(
	DCMode_t *dc_mode,					// Supported Data Compression Mode (P0)
	DCMxDictionarySize_t *mx_dict_sz,	// Supported Mx Dictionary Size (P1)
	DCMxStringSize_t *mx_string_sz		// Supported Mx String Size (P2)
	);

void ECDC_MakeLinkReq(
	ECDCLinkMode_t mode,				// mode of link mode; initiator/acceptor
	ECDCLinkType_t	connection_element	// type of link; non-/transparent
	);

void ECDC_StopLinkReq(
	void
	);

void ECDC_HaltLinkReq(
	void
	);

void ECDC_StartDataReq(
	void
	);

void ECDC_StopDataReq(
	void
	);

ECMode_t ECDC_GetECMode(
	void
	);

DCMode_t ECDC_GetDCMode(
	void
	);

void TCI_tx_response(
	void
	);

void ECDCDCE_FlushRX(void);

void ECDC_NoneEcdcLinkEst(void);

Boolean ECDC_IsBypassMode(void	);

#endif


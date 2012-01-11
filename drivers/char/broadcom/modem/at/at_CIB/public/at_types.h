//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//***************************************************************************
/**
*
*   @file   at_types.h
*
*   @brief  This file contains definitions for AT types.
*
****************************************************************************/

#ifndef _AT_TYPES_H_
#define _AT_TYPES_H_			/// Include Guard

///  @addtogroup ATCommandTypeGroup AT Command Types
///  @ingroup ATCommand
///  @{ 

/** \file at_types.h &copy; 2004 Broadcom Corporation */
#include "mobcom_types.h"

/** Maximum length of an AT command input line */
#define AT_PARM_BUFFER_LEN 10240

//-----------------------------------------------------------------------------
//  Type definitions
//-----------------------------------------------------------------------------

//
//  AT function completion status
//
/** AT command handler status is DONE */
#define AT_STATUS_DONE      1
/** AT command handler status is PENDING */
#define AT_STATUS_PENDING   2
#define AT_STATUS_ERROR     3

/** AT command handler status.   AT_STATUS_DONE or AT_STATUS_PENDING or  AT_STATUS_ERROR*/
typedef UInt32 AT_Status_t ;

/** AT command ID.  A unique ID is assigned to each AT command using the AT Command
  * table generator utility.
  */
typedef UInt16 AT_CmdId_t ;

#define AT_INVALID_CMD_ID   ((AT_CmdId_t)(~0))  /// invalid AT command ID

/** Command access types */
typedef enum {
    AT_ACCESS_UNKNOWN,  /**< unknown */
    AT_ACCESS_REGULAR,  /**< invoke or set; examples: at+cops=1,2; ate; ate0 */
    AT_ACCESS_READ,     /**< status or query: examples: at+cops?; ate? */
    AT_ACCESS_TEST      /**< get help; examples: at+cops=?; ate=? */
} AT_CmdAccessTypes_t;

/** Command types */
typedef enum {
    AT_CMDTYPE_UNKNOWN, /**< unknown */
    AT_CMDTYPE_BASIC,   /**< basic command; examples: ATE, ATE0 */
    AT_CMDTYPE_EXTENDED, /**< extended command; examples: AT+COPS=1,2,3 */
    AT_CMDTYPE_SPECIAL  /**< special command accepts everything as arg; example: ATD12345; */
} AT_CmdTypes_t;

/** Command options, may be or'ed together */
typedef enum {
    AT_OPTION_NONE                  = 0x00, /**< no options */
    AT_OPTION_NO_PIN                = 0x01, /**< PIN not required */
    AT_OPTION_NO_PWR                = 0x02, /**< +CFUN=1 not required */
    AT_OPTION_USER_DEFINED_TEST_RSP = 0x04, /**< user-defined TEST response implemented in handler */
    AT_OPTION_WAIT_PIN_INIT         = 0x08, /**< wait for PIN initialization */
    AT_OPTION_HIDE_NAME             = 0x10, /**< hide name in AT* and similar cmds */
    AT_OPTION_BYPASS_TO_CP          = 0x20, /**< Send AT command from AP to CP */
    AT_OPTION_BYPASS_TO_AP          = 0x40  /**< Send AT command from CP to AP */
} AT_CmdOptions_t;

// parameter range types - describes elements in generated range-spec array

#define AT_RANGE_NOCHECK        1       /// do not check range 
#define AT_RANGE_SCALAR_UINT8   2       /// value is a scalar UInt8
#define AT_RANGE_SCALAR_UINT16  3       /// value is a scalar UInt16
#define AT_RANGE_SCALAR_UINT32  4       /// value is a scalar UInt32
#define AT_RANGE_MINMAX_UINT8   5       /// two consecutive values are UInt8  min and max
#define AT_RANGE_MINMAX_UINT16  6       /// two consecutive values are UInt16 min and max
#define AT_RANGE_MINMAX_UINT32  7       /// two consecutive values are UInt32 min and max
#define AT_RANGE_STRING         8       /// value is an index in string table
#define AT_RANGE_ENDRANGE       9       /// end of range list
#define AT_RANGE_SPECIAL		10 
#define AT_RANGE_NOVALIDPARMS (-1)      /// 'validParms' index when no range specified

/**
	AT command handler ( generic number of parameters )
	@param [in]	cmdId Command ID
	@param [in]	chan  Channel
	@param [in]	accessType - See #AT_CmdAccessTypes_t
	@return AT_Status_t		Returns AT command handler status.
*/
typedef AT_Status_t ( *AT_CmdHndlr_t) ( 
    AT_CmdId_t          cmdId,
    UInt8               chan,
    UInt8               accessType, ... ) ;

///
/// AT Command Table entry
///
typedef struct {                    
    const char*         cmdName ;           ///<    Command Name
    const char*         validParms ;        ///<    Valid Parameters list
    UInt8               cmdType ;           ///<    Command Type - See #AT_CmdTypes_t
    AT_CmdHndlr_t       cmdHndlr ;          ///<    Command Handler
    Int32               rangeIdx ;          ///<    range index; -1 = no range spec
    UInt8               minParms ;          ///<    Minimum number of parameters used for this command
    UInt8               maxParms ;          ///<    Maximum number of parameters used for this command
    UInt8               options ;           ///<    Command Options - See #AT_CmdOptions_t
}   AT_Cmd_t ;                      

///
///	AT Command Info 
///
typedef struct 
{
    UInt8           channel;        ///< Channel Info
    Int16           len;            ///< At Response string len
    UInt8           *buffer;        ///< Response Buffer
    UInt8           simID;          ///< SIM ID
}AtCmdInfo_t;

///  @} 

#endif // _AT_TYPES_H_



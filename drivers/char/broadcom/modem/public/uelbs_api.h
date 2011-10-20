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

#ifndef UELBS_API_H
#define UELBS_API_H

/*
** =================================================================================================
**
**                 INCLUDE FILES
**
** =================================================================================================
*/


/*
** =================================================================================================
**
**                 DEFINITIONS
**
** =================================================================================================
*/


/* =========================================================================
**
**             TYPES
**
** =========================================================================
*/

#define LBS_INVALID_UARFCN			0xFFFF
#define LBS_MAX_NUM_UMTS_FREQ		3
#define LBS_MAX_NUM_CELL_PER_FREQ   32
#define	LBS_MAX_NUM_GSM_NCELL		32

typedef struct
{
	/** Mobile country code that identifies uniquely the country of domicile of the
		mobile subscriber. Valid values: 0x0000 to 0xFFFF; Invalid/default value:
		0xFFFF. See 3GPP TS 03.03, sub-clause 4.1. Also see 3GPP TS 24.008, subclause
		10.5.1.3 for MCC contained in LAI (Location Area Identification). */
	UInt16			mcc; 			

	/** Mobile network code that identifies the home GSM PLMN of the mobile
		subscriber. Valid values: 0x0000 to 0xFFFF; Invalid/default value: 0x00FF. See
		3GPP TS 03.03, sub-clause 4.1. Also see 3GPP TS 24.008, sub-clause 10.5.1.3
		for mnc contained in LAI (Location Area Identification). */
	UInt16			mnc; 		

	/** Location area code that identifies a location area within a GSM PLMN. Valid
		values: 0x0000 to 0xFFFF; Invalid/default value: 0xFFFF. Value of 0xFFFE
		represents deleted LAI. See 3GPP TS 03.03, sub-clause 4.1. Also see 3GPP TS
		24.008, sub-clause 10.5.1.3 for LAC contained in LAI (Location Area
		Identification). */
	UInt16			lac; 

} T_LBS_MM_Params;

typedef struct
{
	/** Cell identification that identifies the cell within a location area. Valid value:
		0x0000 to 0xFFFF; Invalid/default: 0x0000. See 3GPP TS 03.03, sub-clause
		4.3.1. */
	UInt16			cell_id;	 

	/** Absolute radio frequency channel number in idle mode. Valid values: 0 to
		1023; Invalid/default value: 0xFFFF. See 3GPP TS 05.05, sub-clause 2. 
		@note the most significant bit is changed from 1 to 0 for PCS band. So it contains the
		actual arfcn number in any band. */
	UInt16			arfcn;			

	/** contains the value of the RXLEV (receive power level) parameter for the
		serving cell calculated by the mobile station. Valid values: 0 to 63; Invalid/default
		value: 0xFF. See 3GPP TS 05.08, sub-clause 8.1.3, 8.1.4. */
	UInt8           rxlev;				

	/** The base station identity code that allows a MS to distinguish between
		different neighbouring base stations. Valid values: 0 to 63; Invalid/default value:
		0xFF. See 3GPP TS 05.08, sub-clause 9. */
	UInt8			bsic;	
} T_LBS_GSM_Cell;

#if defined(UMTS) || defined(CPLANE_RRC_API_INCLUDED)
typedef struct 
{
   /* Cell Identity identifies a cell unambiguously within a PLMN.
    * Format: 28 bits starting from LSB. 
    * Invalid value: INVALID_CELL_IDENTITY (0xFFFFFFFF)
    * See 3GPP 25.331 section 10.3.2.2 */
   UInt32 cell_identity;

   /* Primary scrambling code
    * Range: 0-511
    * Invalid value: 512
    * See 3GPP 25.331 section 10.3.6.60 */
   UInt16 psc;

   /* Received Signal Code Power
    * Range: -121 to -25
    * Invalid value: INVALID_FDD_RSCP_DBM (-122)*/
   Int8  rscp;

   /* Pathloss
    * Range: 46 to 158
    * Invalid value: 0 */
   UInt16  pathloss;
   
   /* Received energy per chip divided by the power density in the band 
    * Range: -24 to 0
    * Invalid value: MIN_FDD_ECNO_DB - 1  (-25)*/
   Int8  ecno;
} T_LBS_UMTS_MeasCell;

typedef struct
{
   /* Downlink and Uplink UARFCN
    * Range: 0-16383
    * Invalid value: INVALID_UARFCN
    * See 3GPP 25.331 section 10.3.6.36 */
   UInt16 uarfcn_DL;
   UInt16 uarfcn_UL;

   /* RSSI 
    * Range: -121 to -25
    * Invalid value: INVALID_FDD_RSCP_DBM (-122)*/
   Int8  rssi;

   /* measurement for each cells on this frequency */
   UInt8 num_cell;
   T_LBS_UMTS_MeasCell cells[LBS_MAX_NUM_CELL_PER_FREQ];
} T_LBS_UMTS_MeasFreq;

typedef struct
{
   
   /* used frequency. Contains serving cell in non-DCH state, and active set cells
    * in DCH state */
   T_LBS_UMTS_MeasFreq  active;
   
   /* non-used frequencies, up to 2 frequencies, up to 32 cells each frequency, but
    * total is no greater than 32 cells */
   UInt8 num_umts_freq;
   T_LBS_UMTS_MeasFreq umts_freqs[LBS_MAX_NUM_UMTS_FREQ];

   /* gsm cells */
   UInt8 num_gsm_ncell;
   T_LBS_GSM_Cell gsm_ncells[LBS_MAX_NUM_GSM_NCELL];

#if 0 /* just for compiling purpose, once CAPI change is done, it can be removed */
   UInt32 cell_identity;
   UInt16 psc;
   UInt16 uarfcn_DL;
   UInt16 uarfcn_UL;

   Int8  rscp;
   UInt16  pathloss;
   Int8  ecno;   
#endif
} T_LBS_UMTS_Params;
#endif // UMTS

typedef struct
{
    /* there will be only 1 serving cell in GSM */
    T_LBS_GSM_Cell scell;

#if defined(UMTS) || defined(CPLANE_RRC_API_INCLUDED)
   /* UMTS neighbor cells. up to 2 frequencies, up to 32 cells each frequency, but
    * total is no greater than 32 cells */
   UInt8 num_umts_freq;
   T_LBS_UMTS_MeasFreq umts_freqs[LBS_MAX_NUM_UMTS_FREQ];
#endif

   /* gsm neighbor cells */
   UInt8 num_gsm_ncells;
   T_LBS_GSM_Cell gsm_ncells[LBS_MAX_NUM_GSM_NCELL];

#if 0   /* once CAPI change is made, they will be removed */
	UInt16			cell_id;	 
	UInt16			arfcn;			
	UInt8           rxlev;				
	UInt8			bsic;			
#endif
} T_LBS_GSM_Params;

typedef struct
{
	T_LBS_MM_Params		lbs_mm_params;
   	T_LBS_GSM_Params	lbs_gsm_params;
#if defined(UMTS) || defined(CPLANE_RRC_API_INCLUDED)
    T_LBS_UMTS_Params	lbs_umts_params;
#endif // UMTS
} T_UE_LBS_PARAMS;

///The serving cell information.
typedef struct
{
	UInt8			mRAT; ///< Current Radio Access Technology: RAT_NOT_AVAILABLE(0),RAT_GSM(1),RAT_UMTS(2)
	T_UE_LBS_PARAMS mLbsParams; ///< The LBS parameters
} ServingCellInfo_t;

/* Define index to maintain read buffer for SCELL Params variable */
extern UInt8 ue_lbs_index;

/* Define global variable of all parameters required and 
   initialize them to Invalid-Value */
extern T_UE_LBS_PARAMS ue_lbs_params[2];

/* this Macro is used by RRMPRIM to read SCELL Params from the data-structure */
#define READ_UE_LBS_PARAMS_PTR   (&ue_lbs_params[ue_lbs_index % 2])

/* this Macro is used by URRC to write SCELL Params into the data-structure */
#define WRITE_UE_LBS_PARAMS_PTR  (&ue_lbs_params[ue_lbs_index++ % 2])

#endif   /* UELBS_API_H */


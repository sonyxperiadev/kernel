/****************************************************************************
*																			
*     Copyright (c) 2007-2008 Broadcom Corporation								
*																			
*   Unless you and Broadcom execute a separate written software license		
*   agreement governing use of this software, this software is licensed to you	
*   under the terms of the GNU General Public License version 2, available	
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").	
*																			
*   Notwithstanding the above, under no circumstances may you combine this	
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.	
*																			
****************************************************************************/

#define xdr_Boolean		xdr_u_char
#define _xdr_Boolean	_xdr_u_char

#define xdr_u_int8_t	xdr_u_char	
#define _xdr_u_int8_t	_xdr_u_char	

#define xdr_UInt32	xdr_u_long
#define _xdr_UInt32	_xdr_u_long

#define xdr_Int32	xdr_long

#define xdr_UInt8	xdr_u_char
#define _xdr_UInt8	_xdr_u_char

#define xdr_Int8	xdr_char
#define _xdr_Int8(a,b,c)	xdr_char(a,b)

#define xdr_UInt16	xdr_u_int16_t
#define _xdr_UInt16	_xdr_u_int16_t

#define xdr_Ticks_t	xdr_UInt32

bool_t xdr_void_ptr_t(XDR *xdrs, void** ptr);

bool_t xdr_USIM_EST_SERVICE_t(XDR* xdrs, USIM_EST_SERVICE_t* data);
bool_t xdr_SIMServiceStatus_t(XDR* xdrs, SIMServiceStatus_t *data);
bool_t xdr_CAPI2_Patch_Revision_Ptr_t(XDR* xdrs, CAPI2_Patch_Revision_Ptr_t* data);


#define CAPI2_CcApi_SetElement_inCompareObjPtr_union_dscrm	CAPI2_CcApi_CompareObjPtr_union_dscrm
#define CAPI2_CcApi_SetElement_outElementPtr_union_dscrm	CAPI2_CcApi_ElementPtr_union_dscrm

#define CAPI2_CcApi_GetElement_inCompareObjPtr_union_dscrm	CAPI2_CcApi_CompareObjPtr_union_dscrm
#define CAPI2_CcApi_GetElement_outElementPtr_union_dscrm	CAPI2_CcApi_ElementPtr_union_dscrm

#ifndef _T
#define _T(a) a
#endif

bool_t xdr_ClientInfo_t(XDR *xdrs, ClientInfo_t *args);


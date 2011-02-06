#include "xdr.h"

struct CAPI2_ReqRep_t;

#include "capi2_phonectrl_api.h"

#include "capi2_ds_api.h"

#include "capi2_sim_api.h"

#include "capi2_sim_msg.h"
#include "capi2_phonectrl_msg.h"
#include "capi2_pch_api.h"
#include "capi2_pch_msg.h"
#include "capi2_cp_msg.h"
#include "capi2_cp_socket.h"
#include "capi2_stk_api.h"
#include "capi2_phonebk_api.h"
#include "capi2_sms_api.h"
#include "capi2_lcs_msg.h"
#include "lcs_cplane_shared_def.h"
#include "lcs_cplane_rrc_api.h"
#include "isim_def.h"
#include "pch_def.h"
#include "pch_api.h"
#include "pchex_def.h"
#include "pchex_api.h"
#include "capi2_pch_ds.h"

#include "capi2_stk_msg.h"

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

// Common xdr functions
/*typedef char* char_ptr_t;
typedef unsigned char* uchar_ptr_t;
bool_t xdr_uchar_ptr_t(XDR *xdrs, unsigned char** ptr);
bool_t xdr_char_ptr_t(XDR *xdrs, char** ptr);
*/
bool_t xdr_void_ptr_t(XDR *xdrs, void** ptr);

bool_t xdr_USIM_EST_SERVICE_t(XDR* xdrs, USIM_EST_SERVICE_t* data);
bool_t xdr_SIMServiceStatus_t(XDR* xdrs, SIMServiceStatus_t *data);
bool_t xdr_CAPI2_Patch_Revision_Ptr_t(XDR* xdrs, CAPI2_Patch_Revision_Ptr_t* data);

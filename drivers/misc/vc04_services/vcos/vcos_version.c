#include "vcos_build_info.h"
#include "vcinclude/vc_debug_sym.h"

VC_DEBUG_DECLARE_STRING_VAR( vcos_build_hostname, "LBRMN-COREVIDEO" );
VC_DEBUG_DECLARE_STRING_VAR( vcos_build_version, "unknown" );
VC_DEBUG_DECLARE_STRING_VAR( vcos_build_time,    __TIME__ );
VC_DEBUG_DECLARE_STRING_VAR( vcos_build_date,    __DATE__ );

const char *vcos_get_build_hostname( void )
{
   return vcos_build_hostname;
}

const char *vcos_get_build_version( void )
{
   return vcos_build_version;
}

const char *vcos_get_build_date( void )
{
   return vcos_build_date;
}

const char *vcos_get_build_time( void )
{
   return vcos_build_time;
}

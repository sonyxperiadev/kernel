/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  VCHI
Module   :  Endian-aware routines to read/write data

FILE DESCRIPTION

=============================================================================*/

#ifndef _VCHI_ENDIAN_H_
#define _VCHI_ENDIAN_H_

#include "interface/vcos/vcos.h"

int16_t  vchi_readbuf_int16 ( const void *ptr );
uint16_t vchi_readbuf_uint16( const void *ptr );
uint32_t vchi_readbuf_uint32( const void *ptr );
vcos_fourcc_t vchi_readbuf_fourcc( const void *ptr );

void vchi_writebuf_uint16( void *ptr, uint16_t value );
void vchi_writebuf_uint32( void *ptr, uint32_t value );
void vchi_writebuf_fourcc( void *ptr, vcos_fourcc_t value );

#endif /* _VCHI_ENDIAN_H_ */

/********************************** End of file ******************************************/

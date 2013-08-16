/*=============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 VC1 VLC tables
===============================================================================*/

#ifndef VC1_PARSE_TABLES_H
#define VC1_PARSE_TABLES_H

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif

/*
   Many fields in VC1 headers are specified using variable-length codes.
   eg
   0  --> X
   10 --> Y
   11 --> Z
   The table to use depends on which field is being decoded, and sometimes on the value
   of other fields.
   We don't have hardware to do this (apart from norm6 bitplane chunks), so instead we peek at the maximum
   number of bits for the field, and look them up in a modified table, which provides the value and the number of
   bits to actually advance. So the example above would have a table like:
   00 --> X, 1
   01 --> X, 1
   10 --> Y, 2
   11 --> Y, 2

   This could, in theory, peek past the end of the data. The hardware allows this (TODO: check software stream implementations).

   The implementation of this is all macro-ed up. DECLARE_VLC_TABLE(name, size) declares a table and, alongside it, a constant
   containing the maximum size of a codeword.
   In vc1_parse_tables.c, START_DEFINE_VLC_TABLE and END_DEFINE_VLC_TABLE are used to define the table, and to check that the
   number of table entries is correct for the maximum codeword length.

   The VC1_VLC_TABLE(tablename) macro expands to the name,max_size pair, so they can be used together without accidentally getting
   mismatched.

*/


typedef struct vc1_parse_table_entry_t
{
   uint8_t value;
   uint8_t size;
} VC1_VLC_TABLE_ENTRY_T;


#define DECLARE_VLC_TABLE(tablename, max_size) \
   extern const VC1_VLC_TABLE_ENTRY_T    vc1_vlc_ ## tablename ## _table[]; \
   enum {                                vc1_vlc_ ## tablename ## _max_size = max_size };


DECLARE_VLC_TABLE(PTYPE, 4)
DECLARE_VLC_TABLE(PTYPE_SPMP, 2)
DECLARE_VLC_TABLE(FCM, 2)
DECLARE_VLC_TABLE(IMODE, 4)
DECLARE_VLC_TABLE(NORM2_DIFF2, 3)
DECLARE_VLC_TABLE(CONDOVER, 2)
DECLARE_VLC_TABLE(TRANSACFRM, 2)
DECLARE_VLC_TABLE(MVRANGE, 3)

DECLARE_VLC_TABLE(MVMODE_PQUANT_LE12, 4)
DECLARE_VLC_TABLE(MVMODE_PQUANT_GT12, 4)
DECLARE_VLC_TABLE(MVMODE2_PQUANT_LE12, 3)
DECLARE_VLC_TABLE(MVMODE2_PQUANT_GT12, 3)
DECLARE_VLC_TABLE(INTCOMPFIELD, 2)


#undef DECLARE_VLC_TABLE


#define VC1_VLC_TABLE(tablename) vc1_vlc_ ## tablename ## _table, vc1_vlc_ ## tablename ## _max_size

#endif //VC1_PARSE_TABLES_H


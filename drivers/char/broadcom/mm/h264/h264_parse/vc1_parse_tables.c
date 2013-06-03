/*=============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 VC1 VLC tables
===============================================================================*/

#include "vc1_parse_tables.h"
#include "vc1_parse_types.h"
#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#define vcos_static_assert(x)
#else
#include "interface/vcos/vcos_assert.h"
#endif


#define START_DEFINE_VLC_TABLE(tablename) \
   const VC1_VLC_TABLE_ENTRY_T vc1_vlc_ ## tablename ## _table[] = {


#define END_DEFINE_VLC_TABLE(tablename) \
   }; \
   vcos_static_assert((1 << vc1_vlc_ ## tablename ## _max_size) == countof(vc1_vlc_ ## tablename ## _table));

START_DEFINE_VLC_TABLE(PTYPE)
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_B, 2},
   { VC1_PTYPE_B, 2},
   { VC1_PTYPE_B, 2},
   { VC1_PTYPE_B, 2},
   { VC1_PTYPE_I, 3},
   { VC1_PTYPE_I, 3},
   { VC1_PTYPE_BI, 4},
   { VC1_PTYPE_SKIP, 4}
END_DEFINE_VLC_TABLE(PTYPE)

START_DEFINE_VLC_TABLE(PTYPE_SPMP)
   { VC1_PTYPE_B, 2},
   { VC1_PTYPE_I, 2},
   { VC1_PTYPE_P, 1},
   { VC1_PTYPE_P, 1}
END_DEFINE_VLC_TABLE(PTYPE_SPMP)


START_DEFINE_VLC_TABLE(FCM)
   { VC1_FCM_Progressive, 1},
   { VC1_FCM_Progressive, 1},
   { VC1_FCM_Frame_Interlace, 2},
   { VC1_FCM_Field_Interlace, 2},
END_DEFINE_VLC_TABLE(FCM)


START_DEFINE_VLC_TABLE(IMODE)
   { VC1_IMODE_Raw     , 4},
   { VC1_IMODE_Diff6   , 4},
   { VC1_IMODE_Diff2   , 3},
   { VC1_IMODE_Diff2   , 3},
   { VC1_IMODE_RowSkip , 3},
   { VC1_IMODE_RowSkip , 3},
   { VC1_IMODE_ColSkip , 3},
   { VC1_IMODE_ColSkip , 3},
   { VC1_IMODE_Norm2   , 2},
   { VC1_IMODE_Norm2   , 2},
   { VC1_IMODE_Norm2   , 2},
   { VC1_IMODE_Norm2   , 2},
   { VC1_IMODE_Norm6   , 2},
   { VC1_IMODE_Norm6   , 2},
   { VC1_IMODE_Norm6   , 2},
   { VC1_IMODE_Norm6   , 2}
END_DEFINE_VLC_TABLE(IMODE)

START_DEFINE_VLC_TABLE(NORM2_DIFF2)
   {0, 1},
   {0, 1},
   {0, 1},
   {0, 1},
   {2, 3},
   {1, 3},
   {3, 2},
   {3, 2}
END_DEFINE_VLC_TABLE(NORM2_DIFF2)

START_DEFINE_VLC_TABLE(CONDOVER)
   {VC1_CONDOVERLAP_DISABLE, 1},
   {VC1_CONDOVERLAP_DISABLE, 1},
   {VC1_CONDOVERLAP_ENABLE, 2},
   {VC1_CONDOVERLAP_MB_LEVEL, 2}
END_DEFINE_VLC_TABLE(CONDOVER)

START_DEFINE_VLC_TABLE(TRANSACFRM)
   {0, 1},
   {0, 1},
   {1, 2},
   {2, 2}
END_DEFINE_VLC_TABLE(TRANSACFRM)

START_DEFINE_VLC_TABLE(MVRANGE)
   {VC1_MVRANGE_64x32y,       1},
   {VC1_MVRANGE_64x32y,       1},
   {VC1_MVRANGE_64x32y,       1},
   {VC1_MVRANGE_64x32y,       1},
   {VC1_MVRANGE_128x64y,      2},
   {VC1_MVRANGE_128x64y,      2},
   {VC1_MVRANGE_512x128y,     3},
   {VC1_MVRANGE_1024x256y,    3}
END_DEFINE_VLC_TABLE(MVRANGE)

START_DEFINE_VLC_TABLE(MVMODE_PQUANT_LE12)
   {VC1_MVHalfPelBilinear, 4},
   {VC1_MVIntensityComp,   4},
   {VC1_MVHalfPelBicubic,  3},
   {VC1_MVHalfPelBicubic,  3},
   {VC1_MVMixed,           2},
   {VC1_MVMixed,           2},
   {VC1_MVMixed,           2},
   {VC1_MVMixed,           2},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1}
END_DEFINE_VLC_TABLE(MVMODE_PQUANT_LE12)

START_DEFINE_VLC_TABLE(MVMODE_PQUANT_GT12)
   {VC1_MVMixed,            4},
   {VC1_MVIntensityComp,    4},
   {VC1_MVHalfPelBicubic,   3},
   {VC1_MVHalfPelBicubic,   3},
   {VC1_MVQuarterPel,       2},
   {VC1_MVQuarterPel,       2},
   {VC1_MVQuarterPel,       2},
   {VC1_MVQuarterPel,       2},
   {VC1_MVHalfPelBilinear,  1},
   {VC1_MVHalfPelBilinear,  1},
   {VC1_MVHalfPelBilinear,  1},
   {VC1_MVHalfPelBilinear,  1},
   {VC1_MVHalfPelBilinear,  1},
   {VC1_MVHalfPelBilinear,  1},
   {VC1_MVHalfPelBilinear,  1},
   {VC1_MVHalfPelBilinear,  1}
END_DEFINE_VLC_TABLE(MVMODE_PQUANT_GT12)

START_DEFINE_VLC_TABLE(MVMODE2_PQUANT_LE12)
   {VC1_MVHalfPelBilinear, 3},
   {VC1_MVHalfPelBicubic,  3},
   {VC1_MVMixed,           2},
   {VC1_MVMixed,           2},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1},
   {VC1_MVQuarterPel,      1}
END_DEFINE_VLC_TABLE(MVMODE2_PQUANT_LE12)

START_DEFINE_VLC_TABLE(MVMODE2_PQUANT_GT12)
   {VC1_MVMixed,           3},
   {VC1_MVHalfPelBicubic,  3},
   {VC1_MVQuarterPel,      2},
   {VC1_MVQuarterPel,      2},
   {VC1_MVHalfPelBilinear, 1},
   {VC1_MVHalfPelBilinear, 1},
   {VC1_MVHalfPelBilinear, 1},
   {VC1_MVHalfPelBilinear, 1}
END_DEFINE_VLC_TABLE(MVMODE2_PQUANT_GT12)

START_DEFINE_VLC_TABLE(INTCOMPFIELD)
   { VC1_INTCOMPFIELD_Top, 2},
   { VC1_INTCOMPFIELD_Bottom, 2},
   { VC1_INTCOMPFIELD_Both, 1},
   { VC1_INTCOMPFIELD_Both, 1},
END_DEFINE_VLC_TABLE(INTCOMPFIELD)




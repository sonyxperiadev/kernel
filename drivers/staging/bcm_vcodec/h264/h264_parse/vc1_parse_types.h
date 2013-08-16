/*=============================================================================
 Copyright (c) 2011 Broadcom Europe Limited.
 All rights reserved.

 Module   :  Hardware video decoder

 $Id: $

 FILE DESCRIPTION
 VC1 parsing structures
===============================================================================*/

#ifndef VC1_PARSE_TYPES_H
#define VC1_PARSE_TYPES_H

/***************************************************************************//**
\file
Define types and structures to hold the output of VC1 bitstream parsing, and
the routines to process the bitstream.

Packed structures in the bitstream are broken out into conventional fixed-width
structures for easy manipulation.
********************************************************************************/

#ifdef VC4_LINUX_PORT
#include"vd3_utils.h"
#else
#include "vcinclude/common.h"
#endif


typedef enum
{
   VC1_NAL_SEQ_START    = 0x0F,
   VC1_NAL_SEQ_END      = 0x0A,
   VC1_NAL_ENTRYPOINT   = 0x0E,
   VC1_NAL_FRAME        = 0x0D,
   VC1_NAL_FIELD        = 0x0C,
   VC1_NAL_SLICE        = 0x0B,

   VC1_NAL_UD1          = 0x1B,
   VC1_NAL_UD2          = 0x1C,
   VC1_NAL_UD3          = 0x1D,
   VC1_NAL_UD4          = 0x1E,
   VC1_NAL_UD5          = 0x1F

} vc1_startcode_t;

#if defined(__HIGHC__)
typedef vc1_startcode_t VC1_STARTCODE_T;
#else
typedef uint8_t         VC1_STARTCODE_T;
#endif


typedef enum
{
   VC1_PROFILE_SIMPLE        =      1,
   VC1_PROFILE_MAIN          =      2,
   VC1_PROFILE_ADVANCED      =      0,
} vc1_profile_t;

#if defined(__HIGHC__)
typedef vc1_profile_t VC1_PROFILE_T;
#else
typedef uint8_t       VC1_PROFILE_T;
#endif




typedef enum
{
   VC1_PTYPE_I = 0,
   VC1_PTYPE_P = 1,
   VC1_PTYPE_B = 2,
   VC1_PTYPE_BI = 3,
   VC1_PTYPE_SKIP = 4
} vc1_ptype_t;

#if defined(__HIGHC__)
typedef vc1_ptype_t VC1_PTYPE_T;
#else
typedef uint8_t     VC1_PTYPE_T;
#endif


typedef enum
{
   VC1_FPTYPE_I_I = 0,
   VC1_FPTYPE_I_P,
   VC1_FPTYPE_P_I,
   VC1_FPTYPE_P_P,
   VC1_FPTYPE_B_B,
   VC1_FPTYPE_B_BI,
   VC1_FPTYPE_BI_B,
   VC1_FPTYPE_BI_BI
} vc1_fptype_t;

#if defined(__HIGHC__)
typedef vc1_fptype_t VC1_FPTYPE_T;
#else
typedef uint8_t      VC1_FPTYPE_T;
#endif


typedef enum
{
   VC1_FCM_Progressive,
   VC1_FCM_Frame_Interlace,
   VC1_FCM_Field_Interlace
} vc1_fcm_t;

#if defined(__HIGHC__)
typedef vc1_fcm_t VC1_FCM_T;
#else
typedef uint8_t   VC1_FCM_T;
#endif




typedef enum
{
   VC1_IMODE_Raw = 0 ,
   VC1_IMODE_Diff6   ,
   VC1_IMODE_Diff2   ,
   VC1_IMODE_RowSkip ,
   VC1_IMODE_ColSkip ,
   VC1_IMODE_Norm2   ,
   VC1_IMODE_Norm6
} vc1_imode_t;

#if defined(__HIGHC__)
typedef vc1_imode_t VC1_IMODE_T;
#else
typedef uint8_t     VC1_IMODE_T;
#endif



typedef enum
{
   VC1_MVRANGE_64x32y = 0,
   VC1_MVRANGE_128x64y,
   VC1_MVRANGE_512x128y,
   VC1_MVRANGE_1024x256y
} vc1_mvrange_t;

#if defined(__HIGHC__)
typedef vc1_mvrange_t VC1_MVRANGE_T;
#else
typedef uint8_t       VC1_MVRANGE_T;
#endif


typedef enum
{
   VC1_MVHalfPelBilinear = 0,
   VC1_MVQuarterPel,
   VC1_MVHalfPelBicubic,
   VC1_MVMixed,
   VC1_MVIntensityComp
} vc1_mvmode_t;

#if defined(__HIGHC__)
typedef vc1_mvmode_t VC1_MVMODE_T;
#else
typedef uint8_t      VC1_MVMODE_T;
#endif



// MQUANT Type defines:
typedef enum
{
   // uses PQUANT as MQUANT cross all macroblocks
   // this is used when DQUANT == 0
   VC1_MB_FRM_QUANT,

   //  uses ALTPQUANT as MQUANT for the selected boundary edge macroblocks
   //  uses PQUANT as MQUANT on rest of macroblocks
   //  this is used when
   //    (DQUANT == 2)                          or
   //    ((DQUANT == 1) && (DQPROFILE  == 00b)) or
   //    ((DQUANT == 1) && (DQPROFILE  == 01b)) or
   //    ((DQUQNT == 1) && (DQPROFILE  == 10b))
   //  MQQuantEdgeFlag is used to
   //  defined which edges should be
   //  used ALTPQUANT as MQANT
   VC1_MB_EDGE_QUANT,

   // uses ALTPQUANT as MQUANT if MQDIFF is set to 1
   // used PQUANT as MQUANT    if MQDIFF is set to 0
   // MQDIFF is decoded on  macroblock layer syntax
   // this is used when
   // ((DQUANT  == 1) && (DQPROFILE == 11b) && (DQBILEVEL = 1))
   VC1_MB_BILEVEL_QUANT,

   // used when MQUANT are vary within the frame
   // MQUANT is construct based on MQDIFF, ABSMQ
   // this is used when
   // ((DQUANT == 1) && (DQPROFILE == 11b) && (DQBILEVEL = 0))
   VC1_MB_QUANT
} vc1_mquant_type_t;

#if defined(__HIGHC__)
typedef vc1_mquant_type_t VC1_MQUANT_TYPE_T;
#else
typedef uint8_t           VC1_MQUANT_TYPE_T;
#endif



// DQPROFILE, 7.1.1.31.2
typedef enum
{
   VC1_AllEdges = 0,
   VC1_DoubleEdge,
   VC1_SingleEdge,
   VC1_AllMacroblocks
} vc1_dqprofile_t;

#if defined(__HIGHC__)
typedef vc1_dqprofile_t VC1_DQPROFILE_T;
#else
typedef uint8_t         VC1_DQPROFILE_T;
#endif


// DQSBEDGE, 7.1.1.31.3
typedef enum
{
   VC1_LeftEdge = 1,
   VC1_TopEdge = 2,
   VC1_RightEdge = 4,
   VC1_BottomEdge = 8,
   VC1_All_Four_Edges = 15
} vc1_dqedge_t;

#if defined(__HIGHC__)
typedef vc1_dqedge_t VC1_DQEDGE_T;
#else
typedef uint8_t      VC1_DQEDGE_T;
#endif


typedef enum
{
   VC1_INTCOMPFIELD_Top = 1,
   VC1_INTCOMPFIELD_Bottom,
   VC1_INTCOMPFIELD_Both
} vc1_intcompfield_t;

#if defined(__HIGHC__)
typedef vc1_intcompfield_t VC1_INTCOMPFIELD_T;
#else
typedef uint8_t            VC1_INTCOMPFIELD_T;
#endif


// SI TTTYPE Table Index
typedef enum
{
   VC1_PQUANT_LT5         =              0,
   VC1_PQUANT_GE5_LT13    =              1,
   VC1_PQUANT_GE13        =              2,
} vc1_ttype_index_t;

#if defined(__HIGHC__)
typedef vc1_ttype_index_t VC1_TTYPE_INDEX_T;
#else
typedef uint8_t           VC1_TTYPE_INDEX_T;
#endif


// IL SI COEF Table Index
typedef enum
{
   VC1_HIGH_RATE_INTRA         =          0,
   VC1_HIGH_MOTION_INTRA       =          1,
   VC1_MID_RATE_INTRA          =          2,
   VC1_LOW_MOTION_INTRA        =          3,
   VC1_HIGH_RATE_INTER         =          0,
   VC1_HIGH_MOTION_INTER       =          1,
   VC1_MID_RATE_INTER          =          2,
   VC1_LOW_MOTION_INTER        =          3,
   VC1_ESCLVLSZ_PQUANT_LE_7    =          0,
   VC1_ESCLVLSZ_PQUANT_GT_7    =          1,
} vc1_si_coeff_index_t;

#if defined(__HIGHC__)
typedef vc1_si_coeff_index_t VC1_SI_COEFF_INDEX_T;
#else
typedef uint8_t              VC1_SI_COEFF_INDEX_T;
#endif



typedef enum
{
   VC1_CONDOVERLAP_DISABLE = 0,
   VC1_CONDOVERLAP_ENABLE = 1, // 10b in standard
   VC1_CONDOVERLAP_MB_LEVEL = 2, // 11b in standard
} vc1_condover_t;


#if defined(__HIGHC__)
typedef vc1_condover_t VC1_CONDOVER_T;
#else
typedef uint8_t        VC1_CONDOVER_T;
#endif


typedef struct vc1_sequence_display_ext_t
{
   uint16_t          DISP_HORIZ_SIZE;  // This is (size - 1)
   uint16_t          DISP_VERT_SIZE;   // This is (size - 1)

   uint8_t           ASPECT_RATIO_FLAG;
   uint8_t           ASPECT_RATIO;
   uint8_t           ASPECT_HORIZ_SIZE; // This is probably (size - 1) XXX standard is unclear
   uint8_t           ASPECT_VERT_SIZE;  // This is probably (size - 1)

   uint8_t           FRAMERATE_FLAG;
   uint8_t           FRAMERATEIND;
   uint8_t           FRAMERATENR;
   uint8_t           FRAMERATEDR;
   uint16_t          FRAMERATEEXP;

   uint8_t           COLOR_FORMAT_FLAG;
   uint8_t           COLOR_PRIM;
   uint8_t           TRANSFER_CHAR;
   uint8_t           MATRIX_COEFF;


} VC1_SEQUENCE_DISPLAY_EXT_T;

#define VC1_MAX_LEAKY_BUCKETS 31

typedef struct vc1_hrd_param_t
{
   uint8_t           HRD_NUM_LEAKY_BUCKETS;
   uint8_t           BIT_RATE_EXPONENT;
   uint8_t           BUFFER_SIZE_EXPONENT;
   uint16_t          HRD_RATE[VC1_MAX_LEAKY_BUCKETS];
   uint16_t          HRD_BUFFER[VC1_MAX_LEAKY_BUCKETS];
} VC1_HRD_PARAM_T;



typedef struct vc1_sequence_t
{
   VC1_PROFILE_T     PROFILE;
   uint8_t           LEVEL;
   uint8_t           COLORDIFF_FORMAT;
   uint8_t           FRMRTQ_POSTPROC;
   uint8_t           BITRTQ_POSTPROC;
   uint8_t           POSTPROCFLAG;
   uint16_t          MAX_CODED_WIDTH;
   uint16_t          MAX_CODED_HEIGHT;
   uint8_t           PULLDOWN;
   uint8_t           INTERLACE;
   uint8_t           TFCNTRFLAG;
   uint8_t           FINTERPFLAG;
   uint8_t           RESERVED;
   uint8_t           PSF;
   uint8_t           DISPLAY_EXT_PRESENT; // Called DISPLAY_EXT in standard
   VC1_SEQUENCE_DISPLAY_EXT_T DISPLAY_EXT;

   uint8_t           HRD_PARAM_FLAG;
   VC1_HRD_PARAM_T   HRD_PARAM;

   // SPMP only
   uint8_t           RANGERED;
   uint8_t           MAXBFRAMES;
   uint8_t           MULTIRES;
   uint8_t           SYNCMARKER;

} VC1_SEQUENCE_T;


typedef struct vc1_hrd_fullness_t
{
   uint8_t           HRD_FULL[VC1_MAX_LEAKY_BUCKETS];
} VC1_HRD_FULLNESS_T;



typedef struct vc1_entrypoint_t
{
   uint8_t           BROKEN_LINK;
   uint8_t           CLOSED_ENTRY;
   uint8_t           PANSCAN_FLAG;
   uint8_t           REFDIST_FLAG;
   uint8_t           LOOPFILTER;
   uint8_t           FASTUVMC;
   uint8_t           EXTENDED_MV;
   uint8_t           DQUANT;
   uint8_t           VSTRANSFORM;
   uint8_t           OVERLAP;
   uint8_t           QUANTIZER;

   VC1_HRD_FULLNESS_T HRD_FULLNESS;

   uint8_t           CODED_SIZE_FLAG;
   uint16_t          CODED_WIDTH;
   uint16_t          CODED_HEIGHT;

   uint16_t          display_width;
   uint16_t          display_height;

   uint16_t          actual_coded_width;
   uint16_t          actual_coded_height;

   uint8_t           EXTENDED_DMV;

   uint8_t           RANGE_MAPY_FLAG;
   uint8_t           RANGE_MAPY;
   uint8_t           RANGE_MAPUV_FLAG;
   uint8_t           RANGE_MAPUV;

} VC1_ENTRYPOINT_T;

#define VC1_MAX_PANSCAN_WINDOWS 4
#define VC1_MAX_BITPLANES_PER_PICTURE 3
#define VC1_BITPLANE_ROW_BYTES 16
#define VC1_BITPLANE_WIDTH (VC1_BITPLANE_ROW_BYTES*8)
#define VC1_MAX_BITPLANE_HEIGHT 128

typedef struct vc1_panscan_window_t
{
   uint32_t          PS_HOFFSET;
   uint32_t          PS_VOFFSET;
   uint16_t          PS_WIDTH;
   uint16_t          PS_HEIGHT;
} VC1_PANSCAN_WINDOW_T;




// For advanced profile
typedef struct vc1_picture_header_t
{
   VC1_FCM_T         FCM;
   VC1_PTYPE_T       PTYPE;

   uint8_t           TFCNTR;

   uint8_t           RPTFRM;
   uint8_t           TFF;
   uint8_t           RFF;

   uint8_t           PS_PRESENT;
   uint8_t           NumberOfPanScanWindows; // If PS_PRESENT, between 1 and 4. Calculation in 8.9.1
   VC1_PANSCAN_WINDOW_T pan_scan_windows[VC1_MAX_PANSCAN_WINDOWS];

   uint8_t           RNDCTRL;
   uint8_t           UVSAMP;
   uint8_t           INTERPFRM;

   uint8_t           PQINDEX;
   uint8_t           HALFQP;

   uint8_t           PQUANTIZER;
   uint8_t           POSTPROC;


   uint8_t           TRANSACFRM;

   uint8_t           TRANSDCTAB;

   // Dequant
   uint8_t           PQDIFF;
   uint8_t           ABSPQ;
   uint8_t           DQUANTFRM;
   VC1_DQPROFILE_T   DQPROFILE;
   VC1_DQEDGE_T      DQSBEDGE;
   VC1_DQEDGE_T      DQDBEDGE;
   uint8_t           DQBILEVEL;



   // For I/BI
   VC1_IMODE_T       ACPRED_imode;
   VC1_CONDOVER_T    CONDOVER;
   VC1_IMODE_T       OVERFLAGS_imode;
   uint8_t           TRANSACFRM2;


   // For P/B
   VC1_MVRANGE_T     MVRANGE;
   uint8_t           DMVRANGE;
   VC1_MVMODE_T      MVMODE;
   VC1_MVMODE_T      MVMODE2;
   uint8_t           LUMSCALE;
   uint8_t           LUMSHIFT;
   VC1_IMODE_T       MVTYPEMB_imode;
   VC1_IMODE_T       SKIPMB_imode;
   uint8_t           MVTAB;
   uint8_t           CBPTAB;
   uint8_t           TTMBF;
   uint8_t           TTFRM;

   uint8_t           BFRACTION;
   VC1_IMODE_T       DIRECTMB_imode;

   // For interlace
   VC1_FPTYPE_T      FPTYPE;
   uint8_t           REFDIST;
   VC1_IMODE_T       FIELDTX_imode;
   uint8_t           FourMVSWITCH; // 4MVSWITCH in the standard
   uint8_t           INTCOMP;
   uint8_t           MBMODETAB;
   uint8_t           TwoMVBPTAB;  // 2MVBPTAB in the standard
   uint8_t           FourMVBPTAB;  // 4MVBPTAB in the standard
   uint8_t           NUMREF;
   uint8_t           REFFIELD;
   VC1_INTCOMPFIELD_T INTCOMPFIELD;

   uint8_t           LUMSCALE2;
   uint8_t           LUMSHIFT2;
   VC1_IMODE_T       FORWARDMB_imode;

   // SPMP only
   uint8_t           RANGEREDFRM;
   uint8_t           BF;
   uint8_t           RESPIC;
   uint8_t           FRMCNT;


   // Derived quantities
   uint8_t           PQUANT; // Derived in section 7.1.1.6
   uint8_t           ALTPQUANT; // Derived in section 7.1.1.31.6
   VC1_SI_COEFF_INDEX_T ac_intra_luma;
   VC1_SI_COEFF_INDEX_T ac_chroma_and_intra_luma;
   VC1_SI_COEFF_INDEX_T esc_lvl_sz;
   VC1_TTYPE_INDEX_T ttype;
   VC1_MQUANT_TYPE_T mquant_type;
   VC1_DQEDGE_T      dqedges;


   uint8_t            bitplanes_used; // We have three bitplane slots (0,1,2). A bit in bitplanes_used set means the corresponding slot is in use
   // Bitplane format: Each macroblock row gets a 16-byte space in bitplane_data, padded if necessary. There are three bitplane 'slots', corresponding
   // to the order the bitplanes are expected by the inner loop.
   // There is no additional padding between slots. If the image is less tall than the maximum, there will be free space at the end of this array,
   // which the categoriser can slice off to save on messagebuffer space.
   uint32_t           bitplane_data_align_dummy; // Force 32-bit alignment of bitplane data
   uint8_t            bitplane_data[VC1_MAX_BITPLANES_PER_PICTURE * VC1_BITPLANE_ROW_BYTES * VC1_MAX_BITPLANE_HEIGHT];
} VC1_PICTURE_HEADER_T;

typedef struct vc1_slice_header_t
{
   uint16_t SLICE_ADDR;
   uint8_t PIC_HEADER_FLAG;
} VC1_SLICE_HEADER_T;




#endif // VC1_PARSE_TYPES


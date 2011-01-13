/*
 *  mdec_codecs.h
 *
 *  A specification of the audio and video codec enums, as well as the structs for specifying additional
 *  configuration data for each codec if required.
 *
 *  Note: to avoid too much transcoding, the enums for the various configs are copied from OMX_Audio.h,
 *  so if that file updates, this one has to as well!
 */

#ifndef _mdec_codecs_h
#define _mdec_codecs_h

/** Codec type enumerations  *********************************************************************/ 

typedef enum
{
   MEDIA_DEC_VIDEO_CodingUnused,     /**< Value when coding is N/A */
   MEDIA_DEC_VIDEO_CodingMPEG4,      /**< MPEG-4 */
   MEDIA_DEC_VIDEO_CodingAVC,        /**< H.264/AVC */
} MEDIA_DEC_VIDEO_T;

typedef enum
{
   MEDIA_DEC_AUDIO_CodingUnused = 0,  /**< Placeholder value when coding is N/A  */
   MEDIA_DEC_AUDIO_CodingPCM,         /**< Any variant of PCM coding */
   MEDIA_DEC_AUDIO_CodingAAC,         /**< Any variant of AAC encoded data */
   MEDIA_DEC_AUDIO_CodingMP3,         /**< Any variant of MP3 encoded data */
   MEDIA_DEC_AUDIO_CodingDDP,         /**< Any variant of Dolby Digital Plus */
} MEDIA_DEC_AUDIO_T;


/** PCM mode type  ******************************************************************************/ 

typedef enum MDEC_PCM_MODETYPE 
{ 
    MEDIA_DEC_PCMModeLinear = 0,  /**< Linear PCM encoded data */ 
    MEDIA_DEC_PCMModeALaw,        /**< A law PCM encoded data (G.711) */ 
    MEDIA_DEC_PCMModeMULaw,       /**< Mu law PCM encoded data (G.711)  */ 
} MDEC_PCM_MODETYPE; 

typedef struct MDEC_PCM_CONFIG 
{ 
   uint32_t nChannels;                /**< Number of channels (e.g. 2 for stereo) */ 
   int8_t   bSigned;                  /**< indicates PCM data as signed (non-zero) or unsigned (0) */ 
   int8_t   bBigEndian;               /**< indicates PCM data as little (0) or big endian */ 
   int8_t   bInterleaved;             /**< indicates interleaved data (non-0) or block/non-interleaved (0) */
   uint32_t nBitPerSample;            /**< Bit per sample */ 
   uint32_t nSamplingRate;            /**< Sampling rate of the source data.  Use 0 for 
                                           variable or unknown sampling rate. */ 
   MDEC_PCM_MODETYPE ePCMMode;        /**< PCM mode enumeration */ 
} MDEC_PCM_CONFIG; 


/** DDP params *****************************************************************************************/

typedef enum MDEC_DDP_BITSTREAMID {
   MEDIA_DEC_DDPBitStreamIdAC3 = 8,
   MEDIA_DEC_DDPBitStreamIdEAC3 = 16,
   MEDIA_DEC_DDPBitStreamIdKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
   MEDIA_DEC_DDPBitStreamIdVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
   MEDIA_DEC_DDPBitStreamIdMax = 0x7FFFFFFF
} MDEC_DDP_BITSTREAMID;

typedef enum MDEC_DDP_BITSTREAMMODE {
   MEDIA_DEC_DDPBitStreamModeCM = 0,   /**< DDP any main audio service: complete main (CM) */
   MEDIA_DEC_DDPBitStreamModeME,       /**< DDP any main audio service: music and effects (ME) */
   MEDIA_DEC_DDPBitStreamModeVI,       /**< DDP any associated service: visually impaired (VI) */
   MEDIA_DEC_DDPBitStreamModeHI,       /**< DDP any associated service: hearing impaired (HI)  */
   MEDIA_DEC_DDPBitStreamModeD,        /**< DDP any associated service: dialogue (D)           */
   MEDIA_DEC_DDPBitStreamModeC,        /**< DDP any associated service: commentary (C)         */
   MEDIA_DEC_DDPBitStreamModeE,        /**< DDP any associated service: emergency (E)          */
   MEDIA_DEC_DDPBitStreamModeVO,       /**< DDP associated service: voice over (VO)            */
   MEDIA_DEC_DDPBitStreamModeK,        /**< DDP main audio service: karaoke                    */
   MEDIA_DEC_DDPBitStreamModeKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
   MEDIA_DEC_DDPBitStreamModeVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
   MEDIA_DEC_DDPBitStreamModeMax = 0x7FFFFFFF
} MDEC_DDP_BITSTREAMMODE;


typedef struct MEDIA_DEC_PARAM_DDPTYPE 
{
   uint32_t nChannels;            /**< Number of channels */
   uint32_t nBitRate;             /**< Bit rate of the input data.  Use 0 for variable
                                     rate or unknown bit rates */
   uint32_t nSampleRate;          /**< Sampling rate of the source data. Use 0 for
                                     variable or unknown sampling rate. */
   MDEC_DDP_BITSTREAMID   eBitStreamId;
   MDEC_DDP_BITSTREAMMODE eBitStreamMode;
} MDEC_DDP_CONFIG;

#endif // _mdec_codecs_h

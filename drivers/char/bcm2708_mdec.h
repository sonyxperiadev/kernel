#ifndef	_BCM2708_MDEC_H_
#define	_BCM2708_MDEC_H_

#include "mdec_codecs.h"

#define MEDIA_DEC_DEBUG_FILENAME_LENGTH 256

typedef struct {
   int          audio_type;
   int          video_type;
   unsigned int filename_size;
   char         filename[MEDIA_DEC_DEBUG_FILENAME_LENGTH];
} bcm2708_mdec_play_t;

typedef struct {
   int audio_type;
   int video_type;

   union
   {
      MDEC_PCM_CONFIG mPCM;
      MDEC_DDP_CONFIG mDDP;
   } mAudioConfig;
} bcm2708_mdec_setup_t;

typedef struct {
   unsigned int data_size;
   void        *data_buf;
   int          flags;
   uint32_t     callback_context;
   uint64_t     timestamp;
} bcm2708_mdec_send_data_t;

typedef struct {
   int volumeInmB;
} bcm2708_mdec_set_volume_t;

typedef struct {
   int      property_id;
   union {
      int          volume;
      unsigned int level;
   } out_value;
} bcm2708_mdec_get_property_t;

typedef struct {
   int x;
   int y;
   int width;
   int height;
} bcm2708_mdec_set_source_region_t;

typedef struct {
   int display;
   int fullscreen;
   int layer;
   int x;
   int y;
   int width;
   int height;
   int transform; // MEDIA_DEC_TRANSFORM_T
   int mode; // MEDIA_DEC_ASPECT_FLAGS_T
} bcm2708_mdec_set_dest_region_t;

typedef struct {
   int muted;
} bcm2708_mdec_set_muted_t;

typedef struct {
   int alpha;
} bcm2708_mdec_set_transparency_t;

typedef struct {
   MDEC_CALLBACK_REASON mReason;
   uint64_t             mData;
   uint32_t             mContext;
} bcm2708_mdec_wait_for_callback_t;

#define MAX_BCM2708_MDEC_IOCTL_CMD_SIZE (MEDIA_DEC_DEBUG_FILENAME_LENGTH + 256)

enum mdec_property_id {
   MDEC_PROPERTY_VOLUME = 0x1,         // int
   MDEC_PROPERTY_VIDEO_BUFFER_LEVEL,   // unsigned int
   MDEC_PROPERTY_AUDIO_BUFFER_LEVEL,   // unsigned int
};

enum mdec_ioctl_id {
        MDEC_IOCTL_PLAYER_LOCAL_DBG_ID  = 0x1,
        MDEC_IOCTL_PLAYER_SETUP_ID,
        MDEC_IOCTL_PLAYER_START_ID,
        MDEC_IOCTL_PLAYER_SEND_VIDEO_DATA_ID,
        MDEC_IOCTL_PLAYER_SEND_AUDIO_DATA_ID,
        MDEC_IOCTL_PLAYER_STOP_ID,
        MDEC_IOCTL_PLAYER_PAUSE_ID,
        MDEC_IOCTL_PLAYER_RESUME_ID,
        MDEC_IOCTL_PLAYER_SET_VOLUME_ID,
        MDEC_IOCTL_PLAYER_GET_PROPERTY_ID,
        MDEC_IOCTL_PLAYER_SET_SOURCE_REGION_ID,
        MDEC_IOCTL_PLAYER_SET_DEST_REGION_ID,
        MDEC_IOCTL_PLAYER_SET_MUTED_ID,
        MDEC_IOCTL_PLAYER_SET_TRANSPARENCY_ID,
        MDEC_IOCTL_PLAYER_WAIT_FOR_CALLBACK_ID,
        MDEC_IOCTL_PLAYER_STOP_CALLBACK_DISPATCH_ID,
        
};

#define MDEC_IOCTL_PLAYER_LOCAL_DBG               _IOW( 'S', MDEC_IOCTL_PLAYER_LOCAL_DBG_ID, bcm2708_mdec_play_t )
#define MDEC_IOCTL_PLAYER_SETUP                   _IOW( 'S', MDEC_IOCTL_PLAYER_SETUP_ID, bcm2708_mdec_setup_t )
#define MDEC_IOCTL_PLAYER_START                   _IO(  'S', MDEC_IOCTL_PLAYER_START_ID )
#define MDEC_IOCTL_PLAYER_SEND_AUDIO_DATA         _IOW(  'S', MDEC_IOCTL_PLAYER_SEND_AUDIO_DATA_ID, bcm2708_mdec_send_data_t )
#define MDEC_IOCTL_PLAYER_SEND_VIDEO_DATA         _IOW(  'S', MDEC_IOCTL_PLAYER_SEND_VIDEO_DATA_ID, bcm2708_mdec_send_data_t )
#define MDEC_IOCTL_PLAYER_STOP                    _IO(   'S', MDEC_IOCTL_PLAYER_STOP_ID )
#define MDEC_IOCTL_PLAYER_PAUSE                   _IO(   'S', MDEC_IOCTL_PLAYER_PAUSE_ID )
#define MDEC_IOCTL_PLAYER_RESUME                  _IO(   'S', MDEC_IOCTL_PLAYER_RESUME_ID )
#define MDEC_IOCTL_PLAYER_SET_VOLUME              _IOW(  'S', MDEC_IOCTL_PLAYER_SET_VOLUME_ID, bcm2708_mdec_set_volume_t )
#define MDEC_IOCTL_PLAYER_GET_PROPERTY            _IOWR( 'S', MDEC_IOCTL_PLAYER_GET_PROPERTY_ID, bcm2708_mdec_get_property_t )
#define MDEC_IOCTL_PLAYER_SET_SOURCE_REGION       _IOW(  'S', MDEC_IOCTL_PLAYER_SET_SOURCE_REGION_ID, bcm2708_mdec_set_source_region_t )
#define MDEC_IOCTL_PLAYER_SET_DEST_REGION         _IOW(  'S', MDEC_IOCTL_PLAYER_SET_DEST_REGION_ID, bcm2708_mdec_set_dest_region_t )
#define MDEC_IOCTL_PLAYER_SET_MUTED               _IOW(  'S', MDEC_IOCTL_PLAYER_SET_MUTED_ID, bcm2708_mdec_set_muted_t )
#define MDEC_IOCTL_PLAYER_SET_TRANSPARENCY        _IOW(  'S', MDEC_IOCTL_PLAYER_SET_TRANSPARENCY_ID, bcm2708_mdec_set_transparency_t )
#define MDEC_IOCTL_PLAYER_WAIT_FOR_CALLBACK       _IOR(  'S', MDEC_IOCTL_PLAYER_WAIT_FOR_CALLBACK_ID, bcm2708_mdec_wait_for_callback_t )
#define MDEC_IOCTL_PLAYER_STOP_CALLBACK_DISPATCH  _IO('S', MDEC_IOCTL_PLAYER_STOP_CALLBACK_DISPATCH_ID )


#endif

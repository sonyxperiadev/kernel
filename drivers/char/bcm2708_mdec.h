#ifndef	_BCM2708_MDEC_H_
#define	_BCM2708_MDEC_H_

#define BCM_STATUS_T int
#define BCM_HANDLE_T int

typedef struct {
	unsigned int	audio_type;
	unsigned int	video_type;
        unsigned int    filename_size;
	char		filename[MEDIA_DEC_DEBUG_FILENAME_LENGTH];
} bcm2708_mdec_play_t;

typedef struct {
        unsigned int    audio_type;
        unsigned int    video_type;
} bcm2708_mdec_setup_t;


typedef struct {
	unsigned int	data_size;
	void		*data_buf;
} bcm2708_mdec_send_data_t;

#define MAX_BCM2708_MDEC_IOCTL_CMD_SIZE	(MEDIA_DEC_DEBUG_FILENAME_LENGTH + 256)

enum mdec_ioctl_id {
        MDEC_IOCTL_PLAYER_LOCAL_DBG_ID  = 0x1,
	MDEC_IOCTL_PLAYER_SETUP_ID,
        MDEC_IOCTL_PLAYER_START_ID,
        MDEC_IOCTL_PLAYER_SEND_DATA_ID,
        MDEC_IOCTL_PLAYER_STOP_ID,
};

#define MDEC_IOCTL_PLAYER_LOCAL_DBG  _IOW('S', MDEC_IOCTL_PLAYER_LOCAL_DBG_ID, bcm2708_mdec_play_t)

#define MDEC_IOCTL_PLAYER_SETUP  _IOW('S', MDEC_IOCTL_PLAYER_SETUP_ID, bcm2708_mdec_setup_t)

#define MDEC_IOCTL_PLAYER_START  _IO('S', MDEC_IOCTL_PLAYER_START_ID)

#define MDEC_IOCTL_PLAYER_SEND_DATA  _IOW('S', MDEC_IOCTL_PLAYER_SEND_DATA_ID, bcm2708_mdec_send_data_t)

#define MDEC_IOCTL_PLAYER_STOP  _IO('S', MDEC_IOCTL_PLAYER_STOP_ID)

#endif	

#ifndef	_BCM2708_MDEC_H_
#define	_BCM2708_MDEC_H_

typedef struct {
	unsigned int	audio_type;
	unsigned int	video_type;
        unsigned int    filename_size;
	char		filename[MEDIA_DEC_DEBUG_FILENAME_LENGTH];
} bcm2708_mdec_play_t;

#define MAX_BCM2708_MDEC_IOCTL_CMD_SIZE	(MEDIA_DEC_DEBUG_FILENAME_LENGTH + 256)

enum mdec_ioctl_id {
	MDEC_IOCTL_PLAY_ID = 0x1,
};

#define MDEC_IOCTL_PLAY  _IOW('S', MDEC_IOCTL_PLAY_ID, bcm2708_mdec_play_t)


#endif	

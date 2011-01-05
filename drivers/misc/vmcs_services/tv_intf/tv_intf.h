#ifndef _TV_INTF_H
#define _TV_INTF_H

//#define TV_INTF_DEBUG

#ifdef TV_INTF_DEBUG

#ifdef __KERNEL__
#define refmt(fmt) "[%s]: " fmt, __func__
#define tv_intf_print(fmt, ...) \
	printk(KERN_ERR refmt(fmt), ##__VA_ARGS__)
#else
#define refmt(fmt) "[%s]: " fmt, __func__
#define tv_intf_print(fmt, ...) \
    printf(refmt(fmt), ##__VA_ARGS__)
#endif

#else
#define tv_intf_print(fmt, ...)
#endif

#define TV_INTF_CMD_SIZE		512
#define TV_INTF_RESP_SIZE	512

typedef enum {
    TV_INTF_CTRL_INVALID = 0,      ///< Disallow cleared memory
    TV_INTF_CTRL_OFF,              ///< No output at all (device appears to be powered down)
    TV_INTF_CTRL_HDMI,             ///< Output to HDMI
    TV_INTF_CTRL_SDTV,             ///< Output to composite
    TV_INTF_CTRL_AUTO,             ///< Output to HDMI if plugged in otherwise composite
} TV_INTF_CTRL_T;

// Status bit(s) in same register as control report back for TV_INTF_CTRL_T
typedef enum {
   TV_INTF_STATUS_HDMI = 0x80000000,         // Set if HDMI is plugged in
} TV_INTF_STATUS_T;

//Two main types of displays (CEA - HDMI device, DMT - computer monitors)
typedef enum {
    HDMI_RES_GROUP_INVALID = 0,
    HDMI_RES_GROUP_CEA,
    HDMI_RES_GROUP_DMT,
    HDMI_RES_GROUP_CUSTOM //Usused at the moment
} HDMI_RES_GROUP_T;

//These are CEA mode numbers (sent in AVI infoframe) for different resolutions
//1080i at 100/120Hz (40,46) are supported by HDMI H/W but note we cannot 
//display the debug overlay under these modes
typedef enum {
   HDMI_CEA_VGA             =  1,
   HDMI_CEA_480p60          =  2,
   HDMI_CEA_480p60H         =  3,
   HDMI_CEA_720p60          =  4,
   HDMI_CEA_1080i60         =  5,
   HDMI_CEA_480i60          =  6,
   HDMI_CEA_480i60H         =  7,
   HDMI_CEA_240p60          =  8,
   HDMI_CEA_240p60H         =  9,
   HDMI_CEA_480i60_4x       = 10,
   HDMI_CEA_480i60_4xH      = 11,
   HDMI_CEA_240p60_4x       = 12,
   HDMI_CEA_240p60_4xH      = 13,
   HDMI_CEA_480p60_2x       = 14,
   HDMI_CEA_480p60_2xH      = 15,
   HDMI_CEA_1080p60         = 16,
   HDMI_CEA_576p50          = 17,
   HDMI_CEA_576p50H         = 18,
   HDMI_CEA_720p50          = 19,
   HDMI_CEA_1080i50         = 20,
   HDMI_CEA_576i50          = 21,
   HDMI_CEA_576i50H         = 22,
   HDMI_CEA_288p50          = 23,
   HDMI_CEA_288p50H         = 24,
   HDMI_CEA_576i50_4x       = 25,
   HDMI_CEA_576i50_4xH      = 26,
   HDMI_CEA_288p50_4x       = 27,
   HDMI_CEA_288p50_4xH      = 28,
   HDMI_CEA_576p50_2x       = 29,
   HDMI_CEA_576p50_2xH      = 30,
   HDMI_CEA_1080p50         = 31,
   HDMI_CEA_1080p24         = 32,
   HDMI_CEA_1080p25         = 33,
   HDMI_CEA_1080p30         = 34,
   HDMI_CEA_480p60_4x       = 35,
   HDMI_CEA_480p60_4xH      = 36,
   HDMI_CEA_576p50_4x       = 37,
   HDMI_CEA_576p50_4xH      = 38,
   HDMI_CEA_1080i50_rb      = 39,
   HDMI_CEA_1080i100        = 40, 
   HDMI_CEA_720p100         = 41,
   HDMI_CEA_576p100         = 42,
   HDMI_CEA_576p100H        = 43,
   HDMI_CEA_576i100         = 44,
   HDMI_CEA_576i100H        = 45,
   HDMI_CEA_1080i120        = 46,
   HDMI_CEA_720p120         = 47,
   HDMI_CEA_480p120         = 48,
   HDMI_CEA_480p120H        = 49,
   HDMI_CEA_480i120         = 50,
   HDMI_CEA_480i120H        = 51,
   HDMI_CEA_576p200         = 52,
   HDMI_CEA_576p200H        = 53,
   HDMI_CEA_576i200         = 54,
   HDMI_CEA_576i200H        = 55,
   HDMI_CEA_480p240         = 56,
   HDMI_CEA_480p240H        = 57,
   HDMI_CEA_480i240         = 58,
   HDMI_CEA_480i240H        = 59,
   //Put more CEA codes here if we support in the future

   HDMI_CEA_OFF = 0xff //Special code to shutdown HDMI

} HDMI_CEA_RES_CODE_T;

//Enum is ordered as ascending order of DMT id
//All reduced blanking formats are 60Hz unless stated otherwise
typedef enum {
   HDMI_DMT_640x350_85      = 0x1,  //640x350
   HDMI_DMT_640x400_85      = 0x2,  //640x400
   HDMI_DMT_IBM_VGA_85      = 0x3,  //720x400
   HDMI_DMT_VGA_60          = 0x4,  //640x480 (60Hz is same as VGA above)
   HDMI_DMT_VGA_72          = 0x5,
   HDMI_DMT_VGA_75          = 0x6,
   HDMI_DMT_VGA_85          = 0x7,
   HDMI_DMT_SVGA_56         = 0x8,  //800x600
   HDMI_DMT_SVGA_60         = 0x9,
   HDMI_DMT_SVGA_72         = 0xA,
   HDMI_DMT_SVGA_75         = 0xB,
   HDMI_DMT_SVGA_85         = 0xC,
   HDMI_DMT_SVGA_120        = 0xD,
   HDMI_DMT_848x480_60      = 0xE,  //848x480
   HDMI_DMT_XGA_60          = 0x10, //1024x768
   HDMI_DMT_XGA_70          = 0x11,
   HDMI_DMT_XGA_75          = 0x12,
   HDMI_DMT_XGA_85          = 0x13,
   HDMI_DMT_XGA_120         = 0x14,
   HDMI_DMT_XGAP_75         = 0x15, //1152x864
   HDMI_DMT_WXGA_RB         = 0x16, //1280x768 reduced blanking
   HDMI_DMT_WXGA_60         = 0x17, 
   HDMI_DMT_WXGA_75         = 0x18, 
   HDMI_DMT_WXGA_85         = 0x19, 
   HDMI_DMT_WXGA_120        = 0x1A, //120Hz with reduced blanking
   HDMI_DMT_1280x800_RB     = 0x1B, //1280x800 reduced blanking
   HDMI_DMT_1280x800_60     = 0x1C, 
   HDMI_DMT_1280x960_60     = 0x20, //1280x960
   HDMI_DMT_1280x960_85     = 0x21,
   HDMI_DMT_SXGA_60         = 0x23, //1280x1024
   HDMI_DMT_SXGA_75         = 0x24, 
   HDMI_DMT_SXGA_85         = 0x25,
   HDMI_DMT_1360x768_60     = 0x27, //1360x768
   HDMI_DMT_1360x768_120    = 0x28, //120 Hz with reduced blanking
   HDMI_DMT_SXGAP_RB        = 0x29, //1400x1050 reduced blanking
   HDMI_DMT_SXGAP_60        = 0x2A, 
   HDMI_DMT_SXGAP_75        = 0x2B,
   HDMI_DMT_1440x900_RB     = 0x2E, //1440x900 reduced blanking
   HDMI_DMT_1440x900_60     = 0x2F,
   HDMI_DMT_1440x900_75     = 0x30,  
   HDMI_DMT_1440x900_85     = 0x31, 
   HDMI_DMT_UXGA_60         = 0x33, //1600x1200 60Hz
   HDMI_DMT_SWXGAP_RB       = 0x39, //1680x1050 reduced blanking
   HDMI_DMT_SWXGAP_60       = 0x3A, //1680x1050 60Hz
   HDMI_DMT_WUXGA_RB        = 0x44, //1920x1200 reduced blanking
   HDMI_DMT_1366x768_60     = 0x51, //1366x768 60Hz
   HDMI_DMT_1080p_60        = 0x52, //Same as 1080p60 above
   HDMI_DMT_1600x900_RB     = 0x53, //1600x900 reduced blanking
   HDMI_DMT_720p_60         = 0x55, //Same as 720p60 above
   HDMI_DMT_1366x768_RB     = 0x56, //1366x768 reduced blanking
   //Put more DMT codes here if we support in the future

   HDMI_DMT_OFF = 0xff
} HDMI_DMT_RES_CODE_T;

typedef struct {
    TV_INTF_CTRL_T      output_ctrl;
    HDMI_RES_GROUP_T    hdmi_res_group;
    uint32_t            hdmi_res_code;       // of type HDMI_CEA_RES_CODE_T or type HDMI_DMT_RES_CODE_T
} TV_INTF_IOCTL_CTRLS_T;

typedef struct {
   uint8_t edid[128];
} TV_INTF_IOCTL_EDID_T;

typedef enum
{
    TV_INTF_IOCTL_CTRLS_ID = 0x1,
    TV_INTF_IOCTL_EDID_ID,

}TV_INTF_COMMAND_ID_T;

#define TV_INTF_IOCTL_GET_CTRLS  _IOR('T', TV_INTF_IOCTL_CTRLS_ID, TV_INTF_IOCTL_CTRLS_T )
#define TV_INTF_IOCTL_SET_CTRLS  _IOW('T', TV_INTF_IOCTL_CTRLS_ID, TV_INTF_IOCTL_CTRLS_T )
#define TV_INTF_IOCTL_GET_EDID   _IOR('T', TV_INTF_IOCTL_EDID_ID, TV_INTF_IOCTL_EDID_T )

#define TV_INTF_MAX_IOCTL_CMD_SIZE 128

int bcm2835_tv_intf(char *response, int maxlen, const char *format, ...);
int bcm2835_tv_ioctl_get(TV_INTF_IOCTL_CTRLS_T *ctl);
int bcm2835_tv_ioctl_set(TV_INTF_IOCTL_CTRLS_T *ctl);
int bcm2835_tv_ioctl_edid_get(TV_INTF_IOCTL_EDID_T *edid);

#endif


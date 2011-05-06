/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/**
*
*  @file    tv_interface.h
*
****************************************************************************/

/***************************************************************************
 *
 * The TV Interface provides the mechanism for the ARM processor to monitor and
 * control the output of the audio/visual stream to a TV through either a HDMI
 * link or a composite video link (SDTV).
 *
 * Control and status is passed by shared memory control and status registers
 * that are in adjacent locations. Exceptions to this are made where the
 * size of the data is asymetrical.
 *
 * Controls are set by writing the desired value, then setting a notification
 * bit in the master TV control register to indicate which register(s) are
 * changed. Finally the ARM to VC doorbell interrupt is issued to the VC.
 * The VC will indicate that it has processed the request by setting new
 * values as required in to the associated status registers and by clearing
 * the change bits in the TV_INTF_CTRL_CHANGE_OFFSET register.
 *
 * Control registers are written to by the ARM an read by the VC.
 *
 * Status is returned in a similar manner except the values are set in
 * the status register, the bit(s) associated with the status register(s)
 * are set into the master TV status register and the VC to ARM doorbell
 * interrupt is set. The ARM will indicate that it has received the status
 * changes and notifcations by clearing the change bits in the
 * TV_INTF_STATUS_CHANGE_OFFSET register.
 *
 * Status and notify registers are written to by the VC and read by the ARM.
 *
 * The registers at TV_INTF_CTRL_CHANGE_OFFSET and TV_INTF_STATUS_CHANGE_OFFSET
 * are read and written to by both the ARM and VC and thus require that the
 * processor hold the mutex defined by TV_INTF_VC_ARM_FLAG0_OFFSET,
 * TV_INTF_VC_ARM_FLAG1_OFFSET and TV_INTF_VC_ARM_TURN_OFFSET. The mutex uses
 * Peterson's algorithm.
 *
 ***************************************************************************/

#ifndef TV_INTF_REGS_H
#define TV_INTF_REGS_H

#if defined( __cplusplus )
extern "C"
{
#endif
//access macro
#define TV_INTF_REGISTER_RW(base, offset) (*(volatile unsigned long *)(base + offset))
#define TV_INTF_REGISTER_RW_BYTE(base, offset) (*(volatile unsigned char *)(base + offset))

typedef enum {
   TV_INTF_OUTPUT_CHANGE               = (1 << 0),       // TV_INTF_OUTPUT_xxx_OFFSET value changed
   TV_INTF_SDTV_COLOR_CHANGE           = (1 << 1),       // TV_INTF_SDTV_COLOR_xxx_OFFSET value changed
   TV_INTF_SDTV_MODE_CHANGE            = (1 << 2),       // TV_INTF_SDTV_MODE_xxx_OFFSET value changed
   TV_INTF_SDTV_ASPECT_CHANGE          = (1 << 3),       // TV_INTF_SDTV_ASPECT_xxx_OFFSET value changed
   TV_INTF_SDTV_MACROVISION_CHANGE     = (1 << 4),       // TV_INTF_SDTV_MACROVISION_xxx_OFFSET value changed
   TV_INTF_SDTV_NOTIFY_CHANGE          = (1 << 5),       // TV_INTF_SDTV_NOTIFY_OFFSET value changed
   TV_INTF_HDMI_RES_GROUP_CHANGE       = (1 << 6),       // TV_INTF_HDMI_RES_GROUP_xxx_OFFSET value changed
   TV_INTF_HDMI_DISPLAY_OPTION_CHANGE  = (1 << 7),       // TV_INTF_HDMI_DISPLAY_OPTIONS_xxx_OFFSET value changed
   TV_INTF_HDMI_RES_CODE_CHANGE        = (1 << 8),       // TV_INTF_HDMI_RES_CODE_xxx_OFFSET value changed
   TV_INTF_HDMI_STATUS_CHANGE          = (1 << 9),       // TV_INTF_HDMI_STATUS_xxx_OFFSET value changed
   TV_INTF_HDMI_HDCP_CHANGE            = (1 << 10),      // TV_INTF_HDMI_HDCP_xxx_OFFSET value changed
   TV_INTF_HDMI_SDP_CHANGE             = (1 << 11),      // TV_INTF_HDMI_SDP_xxx_OFFSET value changed
   TV_INTF_HDMI_NOTIFY_CHANGE          = (1 << 12),      // TV_INTF_HDMI_NOTIFY_OFFSET value changed
   TV_INTF_HDMI_OPTIONS_CHANGE         = (1 << 13),      // TV_INTF_HDMI_OPTIONS_xxx_OFFSET value changed
   TV_INTF_HDMI_EDID_CHANGE            = (1 << 14),      // TV_INTF_HDMI_EDID_OFFSET value changed
   TV_INTF_HDMI_CEC_CHANGE             = (1 << 15),      // TV_INTF_HDMI_CEC_xxx_OFFSET value changed
   TV_INTF_HDMI_EDID_BLOCK_CHANGE      = (1 << 16),      // TV_INTF_HDMI_EDID_BLOCK_xxx_OFFSET value changed
} TV_INTF_CHANGE_T;

// Registers for Peterson's algorithm for locking control and status change registers.
#define TV_INTF_VC_ARM_FLAG0_OFFSET          0x00
#define TV_INTF_VC_ARM_FLAG1_OFFSET          0x04
#define TV_INTF_VC_ARM_TURN_OFFSET           0x08

#define TV_INTF_CTRL_CHANGE_OFFSET           0x10     // ARM->VC control changed bits (TV_INTF_CHANGE_T)
#define TV_INTF_STATUS_CHANGE_OFFSET         0x14     // VC->ARM status changed bits (TV_INTF_CHANGE_T)

#define TV_INTF_OUTPUT_CTRL_OFFSET              0x18     // Values of type TV_INTF_CTRL_T
#define TV_INTF_OUTPUT_STATUS_OFFSET            0x1C     // Values of type TV_INTF_CTRL_T or'd with TV_INTF_STATUS_T

//#define TV_INTF_SDTV_COLOR_CTRL_OFFSET          0x20     // Value of type SDTV_COLOUR_T
//#define TV_INTF_SDTV_COLOR_STATUS_OFFSET        0x24     // Value of type SDTV_COLOUR_T

#define TV_INTF_SDTV_MODE_CTRL_OFFSET           0x28     // Value of type SDTV_MODE_T
#define TV_INTF_SDTV_MODE_STATUS_OFFSET         0x2C     // Value of type SDTV_MODE_T

#define TV_INTF_SDTV_ASPECT_CTRL_OFFSET         0x30     // Value of type SDTV_ASPECT_T
#define TV_INTF_SDTV_ASPECT_STATUS_OFFSET       0x34     // Value of type SDTV_ASPECT_T

#define TV_INTF_SDTV_MACROVISION_CTRL_OFFSET    0x38     // Value of type SDTV_CP_MODE_T
#define TV_INTF_SDTV_MACROVISION_STATUS_OFFSET  0x3C     // Value of type SDTV_CP_MODE_T

#define TV_INTF_SDTV_NOTIFY                     0x40     // Value of type VC_SDTV_NOTIFY_T

#define TV_INTF_HDMI_RES_GROUP_CTRL_OFFSET      0x44     // Value of type HDMI_RES_GROUP_T
#define TV_INTF_HDMI_RES_GROUP_STATUS_OFFSET    0x48     // Value of type HDMI_RES_GROUP_T

//#define TV_INTF_HDMI_ASPECT_CTRL_OFFSET         0x4C     // Value of type HDMI_ASPECT_T
//#define TV_INTF_HDMI_ASPECT_STATUS_OFFSET       0x50     // Value of type HDMI_ASPECT_T

#define TV_INTF_HDMI_RES_CODE_CTRL_OFFSET       0x54     // Value of type HDMI_CEA_RES_CODE_T
#define TV_INTF_HDMI_RES_CODE_STATUS_OFFSET     0x58     // Value of type HDMI_CEA_RES_CODE_T

#define TV_INTF_HDMI_STATUS_OFFSET              0x5C     // Value of type HDMI_STATUS_T
   
#define TV_INTF_HDMI_HDCP_CTRL_OFFSET           0x60     // Value of type HDMI_CP_MODE_T
#define TV_INTF_HDMI_HDCP_STATUS_OFFSET         0x64     // Value of type HDMI_CP_MODE_T

#define TV_INTF_HDMI_SPD_CTRL_OFFSET            0x68     // Value of type HDMI_SPD_TYPE_CODE_T
#define TV_INTF_HDMI_SPD_STATUS_OFFSET          0x6C     // Value of type HDMI_SPD_TYPE_CODE_T
   
#define TV_INTF_HDMI_NOTIFY_OFFSET              0x70     // Value of type VC_HDMI_NOTIFY_T

//#define TV_INTF_HDMI_OPTIONS_CTRL_OFFSET        0x80     // 12 bytes of HDMI_DISPLAY_OPTIONS_T
//#define TV_INTF_HDMI_OPTIONS_STATUS_OFFSET      0x90     // 12 bytes of HDMI_DISPLAY_OPTIONS_T

#define TV_INTF_EDID_OFFSET                     0xA0     // 128 bytes of EDID data

#define TV_INTF_CEC_OUTPUT_OFFSET               0x1A0    // 16 bytes of CEC data
#define TV_INTF_CEC_INPUT_OFFSET                0x1B0    // 16 bytes of CEC data

#define TV_INTF_HDMI_EDID_BLOCK_CTRL_OFFSET     0x1C0    // Select which block of HDMI EDID info
                                                         // is presented at the TV_INTF_EDID_OFFSET
                                                         // address,
#define TV_INTF_HDMI_EDID_BLOCK_STATUS_OFFSET   0x1C4    // Actual block number of HDMI EDID info

// Display options set the bounding box (only used in CEA mode)
typedef struct {
   uint32_t       aspect;                    //  0 - value of type HDMI_ASPECT_T
   // Active area information - meanings as in CEA-861.
   uint32_t       vertical_bar_present;      //  4
   uint32_t       left_bar_width;            //  8
   uint32_t       right_bar_width;           //  C
   uint32_t       horizontal_bar_present;    // 10
   uint32_t       top_bar_height;            // 14
   uint32_t       bottom_bar_height;         // 18
   // S0/S1 flags as defined in CEA-861.
   uint32_t       overscan_flags;            // 1C
} TV_INTF_HDMI_DISPLAY_OPTIONS_T;

#define TV_INTF_HDMI_DISPLAY_OPTIONS_CTRL_OFFSET   0x1D0    // 32 Bytes of data of type TV_INTF_HDMI_DISPLAY_OPTIONS_T
#define TV_INTF_HDMI_DISPLAY_OPTIONS_STATUS_OFFSET 0x1F0    // 32 bytes of data of type TV_INTF_HDMI_DISPLAY_OPTIONS_T

#if defined( __cplusplus )
}
#endif
#endif // TV_INTF_REGS_H

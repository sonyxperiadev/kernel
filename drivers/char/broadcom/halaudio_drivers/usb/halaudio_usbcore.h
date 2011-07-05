/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
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

#ifndef HALAUDIO_USBCORE_H
#define HALAUDIO_USBCORE_H

/* ---- Include Files ----------------------------------------------------- */

/* ---- Constants and Types ----------------------------------------------- */ 

/* USB Audio sub-class definitions
 */
#define USB_SUBCLASS_AUDIO_CONTROL     0x01
#define USB_SUBCLASS_AUDIO_STREAMING   0x02
#define USB_SUBCLASS_MIDI_STREAMING	   0x03
#define USB_SUBCLASS_VENDOR_SPEC	      0xff

/* AudioControl specific descriptors
 */
#define HEADER                         0x01
#define INPUT_TERMINAL                 0x02
#define OUTPUT_TERMINAL                0x03
#define MIXER_UNIT                     0x04
#define SELECTOR_UNIT                  0x05
#define FEATURE_UNIT                   0x06
#define PROCESSING_UNIT                0x07
#define EXTENSION_UNIT                 0x08

/* AudioStreaming specific descriptors
 */
#define AS_GENERAL                     0x01
#define FORMAT_TYPE                    0x02
#define FORMAT_SPECIFIC                0x03

/* AudioStreaming endpoint specific descriptors
 */
#define EP_GENERAL                     0x01

/* Set/Get commands to configure USB device
 */
#define SET_CUR                        0x01
#define GET_CUR                        0x81
#define SET_MIN                        0x02
#define GET_MIN                        0x82
#define SET_MAX                        0x03
#define GET_MAX                        0x83
#define SET_RES                        0x04
#define GET_RES                        0x84
#define SET_MEM                        0x05
#define GET_MEM                        0x85
#define GET_STAT                       0xff

/* Configuration parameters
 */
#define SAMPLING_FREQ_CONTROL          0x01
#define PITCH_CONTROL                  0x02

/* Format Types
 */
#define USB_FORMAT_TYPE_I              0x01
#define USB_FORMAT_TYPE_II             0x02
#define USB_FORMAT_TYPE_III            0x03

/* Type I Formats
 */
#define USB_AUDIO_FORMAT_PCM           0x01
#define USB_AUDIO_FORMAT_PCM8          0x02
#define USB_AUDIO_FORMAT_IEEE_FLOAT	   0x03
#define USB_AUDIO_FORMAT_ALAW	         0x04
#define USB_AUDIO_FORMAT_MU_LAW        0x05

/* AudioControl Header Descriptor
 */
struct csAcIntHeaderDesc
{
   int8_t  bLength;
   int8_t  bDescriptorType;
   int8_t  bDescriptorSubtype;
   int8_t  bcdADC[2];
   int8_t  wTotalLength[2];
   int8_t  bInCollection;
   int8_t  baInterfaceNr[1];
};

/* AudioStreaming Standard Descriptor
 */
struct csAsIntDesc
{
   int8_t  bLength;
   int8_t  bDescriptorType;
   int8_t  bDescriptorSubtype;
   int8_t  bTerminalLink;
   int8_t  bDelay;
   int8_t  wFormatTag[2];
};

/* Format Type I Descriptor
 */
struct formatTypeIDesc
{
   int8_t  bLength;
   int8_t  bDescriptorType;
   int8_t  bDescriptorSubtype;
   int8_t  bFormatType;
   int8_t  bNrChannels;
   int8_t  bSubframeSize;
   int8_t  bBitResolution;
   uint8_t bSamFreqType;

   /* The following is followed by either a lower/upper
    * sampling frequencies or a set of 24-bit discrete
    * sampling frequencies.  This is a variable size
    * array.
    */
   uint8_t tSamFreq[1];
};

/* Volume Information
 */
struct usb_core_volume_info
{
   int cur;    /* Current Setting */
   int min;    /* Minimum Settting */
   int max;    /* Maximum Settting */
   int res;    /* Resolution */
};

/***************************************************************************/
/**
*  Callback used to move samples
*/
typedef void (*USB_CORE_MEDIA_CB)( 
   int16_t *pData,              /*<< (i/o) Samples       */
   int      bytes               /*<< (i) Number of bytes */
);

/***************************************************************************/
/**
*  Callback used to report sampling frequency or frame size change
*/
typedef void (*USB_CORE_INFO_CB)( 
   int freqHz,              /*<< (i) New sample rate        */
   int numberChannels       /*<< (i) New number of channels */
);

/***************************************************************************/
/**
*  Callback used to indicate a USB device is disconnected
*/
typedef void (*USB_CORE_DISCONNECT_CB)( void );

/***************************************************************************/
/**
*  Callbacks to this USB core
*/
typedef struct usb_core_cbs
{
   USB_CORE_MEDIA_CB      mediaIn;    /* Ingress media samples           */
   USB_CORE_MEDIA_CB      mediaOut;   /* Egress media samples            */
   USB_CORE_INFO_CB       infoIn;     /* Report ingress changes          */
   USB_CORE_INFO_CB       infoOut;    /* Report egress changes           */
   USB_CORE_DISCONNECT_CB disconnect; /* Report disconnect               */

} USB_CORE_CBS;

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

int  usbCorePrepare( void );
int  usbCoreEnable( void );
int  usbCoreDisable( void );
int  usbCoreSetIngressFreq( int freqHz );
int  usbCoreSetEgressFreq( int freqHz );
int  usbCoreGetIngressFreq( void );
int  usbCoreGetEgressFreq( void );
int  usbCoreGetIngressChannels( void );
int  usbCoreGetEgressChannels( void );
int  usbCoreGetIngressFrameSize1ms( void );
int  usbCoreGetEgressFrameSize1ms( void );
int  usbCoreIngressIsConnected( void );
int  usbCoreEgressIsConnected( void );
int  usbCoreSetIngressVolume( int vol );
int  usbCoreGetIngressVolume( struct usb_core_volume_info *pVolInfo );
int  usbCoreSetEgressVolume( int vol );
int  usbCoreGetEgressVolume( struct usb_core_volume_info *pVolInfo );
int  usbCoreIngressIsSupported( int freqHz, int numChans );
int  usbCoreEgressIsSupported( int freqHz, int numChans );

void usbCoreRegisterCb( USB_CORE_CBS *pCbs );
void usbCoreDeregisterCb( void );

#endif /* HALAUDIO_USBCORE */


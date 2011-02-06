
#if !defined( _HAL_XMODEM_TEST_H__ )
#define _HAL_XMODEM_TEST_H__

//#include "consts.h"
//#include "mobcom_types.h"

//--- X/Y - MODEM SEND --------------------------------------------------------
// HOST starts with sending NAK ( checksum ) or C ( CRC )
// 1K - start of packet starts with STX instead SOH

typedef struct 
{
	char	file_name[100];
	char	header[100];
	Boolean has_header;
	UInt32  header_size;
	UInt32  payload_size;
	UInt32  file_size;
	void	*payload_addr;
} ymodem_send_rec_t;

Boolean ymodem_send( void* fileinfo, UInt32 file_count );
 					
//extern void 	MPX_HandleInputDeviceEvent(UInt8, UInt8 *);

#endif  // _HAL_XMODEM_TEST_H__





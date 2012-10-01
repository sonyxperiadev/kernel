//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------
#ifdef __MAIN__
#define	GLOBAL
#else
#define GLOBAL	extern
#endif

#define POLY 0x8408                 // CRC16-CCITT FCS (X^16+X^12+X^5+1)

// ---------------------------------
// Target BL Command Codes
// ---------------------------------

#define TGT_CMD_ENTER_BL_MODE       0x90
#define TGT_CMD_GET_INFO            0x20
#define TGT_CMD_ERASE_FLASH_PAGE    0x30
#define TGT_CMD_WRITE_SIGNATURE     0x40
#define TGT_CMD_RESET_MCU           0x50
#define TGT_CMD_GET_PAGE_CRC        0x60
#define TGT_CMD_WRITE_FLASH_BYTES   0x70 

// ---------------------------------
// Target BL Response Codes
// ---------------------------------

#define TGT_RSP_OK                  0x00 // RSP_OK should always be 0
#define TGT_RSP_APP_MODE            0x01
#define TGT_RSP_BL_MODE             0x02
#define TGT_RSP_SIG_NOT_ERASED      0x03
#define TGT_RSP_ADDR_INVALID        0x04
#define TGT_RSP_UNSUPPORTED_CMD     0x05
#define TGT_RSP_PARAMETER_INVALID   0x06
#define TGT_RSP_ERROR               0x80

#define TGT_ENTER_BL_MODE_REQ_RSP_CODE 0x52 // This will request a response

#define TGT_BL_FW_INFOBLOCK_LENGTH  16   // See documentation

#define TGT_BL_FW_VERSION_LOW    0x00
#define TGT_BL_FW_VERSION_HIGH   0x01

#define KEY_CODE0 0xA5
#define KEY_CODE1 0xF1

#define APP_START_ADDR 0x600
#define APP_END_ADDR_OLD 0x7DFF
#define APP_END_ADDR_NEW 0x7BFF

// Signature Bytes
#define SIG_BYTE0                0x3D
#define SIG_BYTE1                0xC2

#define YTE_MODULE_VER   0x02
#define SMAC_MODULE_VER   0x03
#define YTE_MODULE_VER_OLD   0x04
#define SMAC_MODULE_VER_OLD    0x05
#define SMAC_MODULE_VER_NEW   0x07
#define YTE_MODULE_VER_NEW   0x0A

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

int mcsdl_download_binary_data(void);			// with binary type .c   file.
int mcsdl_download_binary_file(void);			// with binary type .bin file.

//int ResetMCU ();
//int WriteSignature ();
void Update_CRC (uint8_t newbyte);
int GetPageCRC (unsigned int addr);
int WriteBytes (unsigned int addr, unsigned int num);
int EnterBootMode (int res);
//int GetInfo ();
int EraseFlashPage (unsigned int addr);
int Firmware_Download (void);


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------


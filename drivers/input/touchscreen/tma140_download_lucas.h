#if !defined _TMA140_DOWNLOAD_CORI_H_
#define _TMA140_DOWNLOAD_CORI_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_revision.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define PROJECT_REV_304



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_directives.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//#define CY8CTMA300E_36LQXI
//#define CY8CTMA301D_48LTXI
#define CY8CTMA340_36LQXI





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_defs.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Block-Verify Uses 128-Bytes of RAM
#define TARGET_DATABUFF_LEN    128  			// **** CY8C20x66 Device ****

// The number of Flash blocks in each part is defined here. This is used in
// main programming loop when programming and verifying the blocks.
#define NUM_BANKS                     1
#define BLOCKS_PER_BANK             256
//#define BLOCKS_PER_BANK             128
#define SECURITY_BYTES_PER_BANK      64




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_delays.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// DELAY_M is the slope of the Delay = Mx + B equation
#define DELAY_M    1
// DELAY_B is the offset of the delay in Delay = Mx + B.
#define DELAY_B    3

///////////////////////////////////////////////////////////////////////////////
// CAUTION:
// For the above parameters the minimum delay value is 3 (this would result in
// 0 being passed for a minimum delay. A value less than 3 would actually
// create a negative number, causing a very long delay
///////////////////////////////////////////////////////////////////////////////

// TRANSITION_TIMEOUT is a loop counter for a 100msec timeout when waiting for
// a high-to-low transition. This is used in the polling loop of
// fDetectHiLoTransition(). Each pass through the loop takes approximately 15
// usec. 100 msec is about 6740 loops. 13480
#define TRANSITION_TIMEOUT     (65535*10)
//#define TRANSITION_TIMEOUT     (100*1000)

// XRES_DELAY is the time duration for which XRES is asserted. This defines
// a 63 usec delay.
// The minimum Xres time (from the device datasheet) is 10 usec.
#define XRES_CLK_DELAY    ((63 - DELAY_B) / DELAY_M)

// POWER_CYCLE_DELAY is the time required when power is cycled to the target
// device to create a power reset after programming has been completed. The
// actual time of this delay will vary from system to system depending on the
// bypass capacitor size.  A delay of 150 usec is used here.
#define POWER_CYCLE_DELAY ((150 - DELAY_B) / DELAY_M)

// DELAY_100us delays 100 usec. This is used in fXRESInitializeTargetForISSP to
// time the wait for Vdd to become stable after a power up.  A loop runs 10 of
// these for a total delay of 1 msec.
#define DELAY100us        ((100 - DELAY_B) / DELAY_M)




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_errors.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// The following are defines for error messages from the ISSP program.
#define PASS           0
// PASS is used to indicate that a function completed successfully.
#define ERROR         -1
// ERROR is a generic failure used within lower level functions before the
// error is reported.  This should not be seen as an error that is reported
// from main.
#define INIT_ERROR     1
// INIT_ERROR means a step in chip initialization failed.
#define SiID_ERROR     2
// SiID_ERROR means that the Silicon ID check failed. This happens if the
// target part does not match the device type that the ISSP program is
// configured for.
#define ERASE_ERROR    3
// ERASE_ERROR means that the bulk erase step failed.
#define BLOCK_ERROR    4
// BLOCK_ERROR means that a step in programming a Flash block or the verify
// of the block failed.
#define VERIFY_ERROR   5
// VERIFY_ERROR means that the checksum verification failed.
#define SECURITY_ERROR 6
// SECURITY_ERROR means that the write of the security information failed.
#define STATUS_ERROR 7


#define CHECKSUM_ERROR 8

#define NO_FIRMWARE_ERROR	0x10
#define FILE_ACCESS_ERROR	0x11





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_vectors.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef CY8CTMA300D_36LQXI
    unsigned char target_id_v[] = {0x05, 0x79};     //ID for CY8CTMA300D_36LQXI
#endif
#ifdef CY8CTMA300E_36LQXI
    unsigned char target_id_v[] = {0x05, 0x82};     //ID for CY8CTMA300E_36LQXI
#endif

#ifdef CY8CTMA340_36LQXI
    unsigned char target_id_v[] = {0x05, 0x96};     //ID for CY8CTMA340_36LQXI
#endif

#ifdef CY8CTMA301D_48LTXI
    unsigned char target_id_v[] = {0x05, 0x77};     //ID for CY8CTMA301D_48LTXI
#endif

const unsigned int num_bits_checksum = 418;
const unsigned char checksum_v[] =
{
	0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
    0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81, 0xF9, 0xF4, 0x01, 0xF7,
    0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
    0xF7, 0xA0, 0x1F, 0xDE, 0xA0, 0x1F, 0x7B, 0x00, 0x7D, 0xE0,
    0x0F, 0xF7, 0xC0, 0x07, 0xDF, 0x28, 0x1F, 0x7D, 0x18, 0x7D,
    0xFE, 0x25, 0xC0
};

const unsigned char read_status[] =
{
	0xBF, 0x00, 0x80
};


const unsigned int num_bits_id_setup_1 = 616;
const unsigned char id_setup_1[] =
{
	0xCA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0xEE, 0x21, 0xF7,
    0xF0, 0x27, 0xDC, 0x40,	0x9F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
    0xE7, 0xC1,	0xD7, 0x9F, 0x20, 0x7E, 0x3F, 0x9D, 0x78, 0xF6,
	0x21, 0xF7, 0xB8, 0x87, 0xDF, 0xC0, 0x1F, 0x71,	0x00, 0x7D,
	0xC0, 0x07, 0xF7, 0xB8, 0x07, 0xDE,	0x80, 0x7F, 0x7A, 0x80,
	0x7D, 0xEC, 0x01, 0xF7,	0x80, 0x4F, 0xDF, 0x00, 0x1F, 0x7C,
	0xA0, 0x7D,	0xF4, 0x61, 0xF7, 0xF8, 0x97
};

const unsigned int num_bits_id_setup_2 = 418;
const unsigned char id_setup_2[] =
{
    0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
    0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81, 0xF9, 0xF4, 0x01, 0xF7,
    0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
    0xF7, 0xA0, 0x1F, 0xDE, 0xA0, 0x1F, 0x7B, 0x00, 0x7D, 0xE0,
    0x0D, 0xF7, 0xC0, 0x07, 0xDF, 0x28, 0x1F, 0x7D, 0x18, 0x7D,
    0xFE, 0x25, 0xC0
};

const unsigned int num_bits_tsync_enable = 110;
const unsigned char tsync_enable[] =
{
    0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
    0x1F, 0xDE, 0xE0, 0x1C
};
const unsigned int num_bits_tsync_disable = 110;
const unsigned char tsync_disable[] =
{
    0xDE, 0xE2, 0x1F, 0x71, 0x00, 0x7D, 0xFC, 0x01, 0xF7, 0x00,
    0x1F, 0xDE, 0xE0, 0x1C
};


#if 0
const unsigned int num_bits_set_block_num = 33;
const unsigned char set_block_num[] =
{
    0xDE, 0xE0, 0x1E, 0x7D, 0x00, 0x70
};
#else
const unsigned int num_bits_set_block_num = 11;
const unsigned char set_block_num[] =
{
    0x9F, 0x40
};
#endif

const unsigned int num_bits_set_block_num_end = 3;		//PTJ: this selects the first three bits of set_block_num_end
const unsigned char set_block_num_end = 0xE0;

const unsigned int num_bits_read_write_setup = 66;		//PTJ:
const unsigned char read_write_setup[] =
{
    0xDE, 0xF0, 0x1F, 0x78, 0x00, 0x7D, 0xA0, 0x03, 0xC0
};

const unsigned int num_bits_my_verify_setup = 440;
const unsigned char verify_setup[] =
{
    0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
    0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81, 0xF9, 0xF7, 0x01, 0xF7,
    0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
    0xF6, 0xA8, 0x0F, 0xDE, 0x80, 0x7F, 0x7A, 0x80, 0x7D, 0xEC,
    0x01, 0xF7, 0x80, 0x0F, 0xDF, 0x00, 0x1F, 0x7C, 0xA0, 0x7D,
    0xF4, 0x61, 0xF7, 0xF8, 0x97
};

const unsigned int num_bits_erase = 396;		//PTJ: erase with TSYNC Enable and Disable
const unsigned char erase[] =
{
    0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
    0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x85, 0xFD, 0xFC, 0x01, 0xF7,
    0x10, 0x07, 0xDC, 0x00, 0x7F, 0x7B, 0x80, 0x7D, 0xE0, 0x0B,
    0xF7, 0xA0, 0x1F, 0xDE, 0xA0, 0x1F, 0x7B, 0x04, 0x7D, 0xF0,
    0x01, 0xF7, 0xC9, 0x87, 0xDF, 0x48, 0x1F, 0x7F, 0x89, 0x70
};

const unsigned int num_bits_secure = 440;		//PTJ: secure with TSYNC Enable and Disable
const unsigned char secure[] =
{
    0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
    0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81, 0xF9, 0xF7, 0x01, 0xF7,
    0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
    0xF6, 0xA0, 0x0F, 0xDE, 0x80, 0x7F, 0x7A, 0x80, 0x7D, 0xEC,
    0x01, 0xF7, 0x80, 0x27, 0xDF, 0x00, 0x1F, 0x7C, 0xA0, 0x7D,
    0xF4, 0x61, 0xF7, 0xF8, 0x97
};

const unsigned int num_bits_program_and_verify = 440;		//PTJ: length of program_block[], not including zero padding at end
const unsigned char program_and_verify[] =
{
    0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
    0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81, 0xF9, 0xF7, 0x01, 0xF7,
    0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
    0xF6, 0xA0, 0x0F, 0xDE, 0x80, 0x7F, 0x7A, 0x80, 0x7D, 0xEC,
    0x01, 0xF7, 0x80, 0x57, 0xDF, 0x00, 0x1F, 0x7C, 0xA0, 0x7D,
    0xF4, 0x61, 0xF7, 0xF8, 0x97
};

const unsigned char read_id_v[] =
{
    0xBF, 0x00, 0xDF, 0x90, 0x00, 0xFE, 0x60, 0xFF, 0x00
};

const unsigned char    write_byte_start = 0x90;			//PTJ: this is set to SRAM 0x80
const unsigned char    write_byte_end = 0xE0;

const unsigned char    num_bits_wait_and_poll_end = 40;
const unsigned char    wait_and_poll_end[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char read_checksum_v[] =
{
    0xBF, 0x20, 0xDF, 0x80, 0x80
};


const unsigned char read_byte_v[] =
{
    0xB0, 0x80
};




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_extern.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern signed char fXRESInitializeTargetForISSP(void);
extern signed char fPowerCycleInitializeTargetForISSP(void);
extern signed char fEraseTarget(void);
extern unsigned int iLoadTarget(void);
extern void ReStartTarget(void);
extern signed char fVerifySiliconID(void);
extern signed char fAccTargetBankChecksum(unsigned int*);
extern signed char fProgramTargetBlock(unsigned char, unsigned char);
extern signed char fVerifyTargetBlock(unsigned char, unsigned char);
extern signed char fVerifySetup(unsigned char, unsigned char);	//PTJ: VERIFY-SETUP
extern signed char fReadByteLoop(void);							//PTJ: read bytes after VERIFY-SETUP
extern signed char fSecureTargetFlash(void);

extern signed char fReadStatus(void);									//PTJ: READ-STATUS
extern signed char fReadWriteSetup(void);								//PTJ: READ-WRITE-SETUP

//extern void InitTargetTestData(void);
extern void LoadArrayWithSecurityData(unsigned char, unsigned char, unsigned char);

extern void LoadProgramData(unsigned char, unsigned char);
extern signed char fLoadSecurityData(unsigned char);
extern void Delay(unsigned int);
extern unsigned char fSDATACheck(void);
extern void SCLKHigh(void);
extern void SCLKLow(void);
#ifndef RESET_MODE  //only needed when power cycle mode
  extern void SetSCLKHiZ(void);
#endif
extern void SetSCLKStrong(void);
extern void SetSDATAHigh(void);
extern void SetSDATALow(void);
extern void SetSDATAHiZ(void);
extern void SetSDATAStrong(void);
extern void AssertXRES(void);
extern void DeassertXRES(void);
extern void SetXRESStrong(void);
extern void ApplyTargetVDD(void);
extern void RemoveTargetVDD(void);
extern void SetTargetVDDStrong(void);



#ifdef TX_ON
extern void UART_PutHexHalf(char ch);
extern void UART_PutHexByte(unsigned char ch);
extern void UART_PutHexWord(unsigned int ch);
#endif


extern unsigned char   fIsError;




#endif //_TMA140_DOWNLOAD_CORI_H_


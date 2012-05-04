/* drivers/input/touchscreen/tma140_download_lucas.c
 *
 * Copyright (C) 2007 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <mach/gpio.h>
#include <linux/device.h>
#include <linux/uaccess.h> 
#include <linux/slab.h>
#include <linux/syscalls.h>

#include "tma140_download_lucas.h"

#include "./TMA140_FW/TMA140_H01S02.h"


#define TSP_SDA 85
#define TSP_SCL 87

/***********************
*
* Touchpad Tuning APIs
*
************************/

extern void touch_ctrl_regulator(int on_off);

void TchDrv_DownloadVddSetHigh(void)
{
	touch_ctrl_regulator(1); /* always on */
}

void TchDrv_DownloadVddSetLow(void)
{
	touch_ctrl_regulator(0); /* always off */
}




/*++ DELAY TEST ++*/
// provides delays in us
#define ONE_MICROSSEC_CNT 100 // 312MHz
static void delay_1us(void)
{
	volatile unsigned int i;
 
	for(i=0; i<ONE_MICROSSEC_CNT; i++)
	{
	}
}

static void delay_us(unsigned int us)
{
	volatile unsigned int i;
 
	for(i=0; i<us; i++)
	{
		delay_1us();
	}
}
/*-- DELAY TEST --*/



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						Issp_driver_routines.c
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define SECURITY_DATA	0xAA

unsigned char    bTargetDataPtr;
unsigned char    abTargetDataOUT[TARGET_DATABUFF_LEN];
unsigned char  abTargetDataOUT_secure[TARGET_DATABUFF_LEN] ={0xAA,};
unsigned char    firmData[514][64];


// ****************************** PORT BIT MASKS ******************************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
#define SDATA_PIN   0x80        // P1.7
#define SCLK_PIN    0x40        // P1.6
#define XRES_PIN    0x40        // P2.6
#define TARGET_VDD  0x08        // P2.3


// ((((((((((((((((((((((( DEMO ISSP SUBROUTINE SECTION )))))))))))))))))))))))
// ((((( Demo Routines can be deleted in final ISSP project if not used   )))))
// ((((((((((((((((((((((((((((((((((((()))))))))))))))))))))))))))))))))))))))

// ============================================================================
// InitTargetTestData()
// !!!!!!!!!!!!!!!!!!FOR TEST!!!!!!!!!!!!!!!!!!!!!!!!!!
// PROCESSOR_SPECIFIC
// Loads a 64-Byte array to use as test data to program target. Ultimately,
// this data should be fed to the Host by some other means, ie: I2C, RS232,
// etc. Data should be derived from hex file.
//  Global variables affected:
//    bTargetDataPtr
//    abTargetDataOUT
// ============================================================================
void InitTargetTestData(unsigned char bBlockNum, unsigned char bBankNum)
{
    // create unique data for each block
    for (bTargetDataPtr = 0; bTargetDataPtr < TARGET_DATABUFF_LEN; bTargetDataPtr++)
    {
        abTargetDataOUT[bTargetDataPtr] = 0x55;
    }
}


// ============================================================================
// LoadArrayWithSecurityData()
// !!!!!!!!!!!!!!!!!!FOR TEST!!!!!!!!!!!!!!!!!!!!!!!!!!
// PROCESSOR_SPECIFIC
// Most likely this data will be fed to the Host by some other means, ie: I2C,
// RS232, etc., or will be fixed in the host. The security data should come
// from the hex file.
//   bStart  - the starting byte in the array for loading data
//   bLength - the number of byte to write into the array
//   bType   - the security data to write over the range defined by bStart and
//             bLength
// ============================================================================
void LoadArrayWithSecurityData(unsigned char bStart, unsigned char bLength, unsigned char bType)
{
    // Now, write the desired security-bytes for the range specified
    for (bTargetDataPtr = bStart; bTargetDataPtr < bLength; bTargetDataPtr++) {
        abTargetDataOUT_secure[bTargetDataPtr] = bType;
    }
}


// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// Delay()
// This delay uses a simple "nop" loop. With the CPU running at 24MHz, each
// pass of the loop is about 1 usec plus an overhead of about 3 usec.
//      total delay = (n + 3) * 1 usec
// To adjust delays and to adapt delays when porting this application, see the
// ISSP_Delays.h file.
// ****************************************************************************
void Delay(unsigned int n)
{
    while(n)
    {
        //asm("nop");
        //_nop_();

        n -= 1;
    }
}


// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// LoadProgramData()
// The final application should load program data from HEX file generated by
// PSoC Designer into a 64 byte host ram buffer.
//    1. Read data from next line in hex file into ram buffer. One record
//      (line) is 64 bytes of data.
//    2. Check host ram buffer + record data (Address, # of bytes) against hex
//       record checksum at end of record line
//    3. If error reread data from file or abort
//    4. Exit this Function and Program block or verify the block.
// This demo program will, instead, load predetermined data into each block.
// The demo does it this way because there is no comm link to get data.
// ****************************************************************************
void LoadProgramData(unsigned char bBlockNum, unsigned char bBankNum)
{
    // >>> The following call is for demo use only. <<<
    // Function InitTargetTestData fills buffer for demo
    InitTargetTestData(bBlockNum, bBankNum);

    // Note:
    // Error checking should be added for the final version as noted above.
    // For demo use this function just returns VOID.
}


// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// fLoadSecurityData()
// Load security data from hex file into 64 byte host ram buffer. In a fully
// functional program (not a demo) this routine should do the following:
//    1. Read data from security record in hex file into ram buffer.
//    2. Check host ram buffer + record data (Address, # of bytes) against hex
//       record checksum at end of record line
//    3. If error reread security data from file or abort
//    4. Exit this Function and Program block
// In this demo routine, all of the security data is set to unprotected (0x00)
// and it returns.
// This function always returns PASS. The flag return is reserving
// functionality for non-demo versions.
// ****************************************************************************
signed char fLoadSecurityData(unsigned char bBankNum)
{
    // >>> The following call is for demo use only. <<<
    // Function LoadArrayWithSecurityData fills buffer for demo
//    LoadArrayWithSecurityData(0,SECURITY_BYTES_PER_BANK, 0x00);
    LoadArrayWithSecurityData(0,SECURITY_BYTES_PER_BANK, SECURITY_DATA);		//PTJ: 0x1B (00 01 10 11) is more interesting security data than 0x00 for testing purposes

    // Note:
    // Error checking should be added for the final version as noted above.
    // For demo use this function just returns PASS.
    return(PASS);
}


// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// fSDATACheck()
// Check SDATA pin for high or low logic level and return value to calling
// routine.
// Returns:
//     0 if the pin was low.
//     1 if the pin was high.
// ****************************************************************************
unsigned char fSDATACheck(void)
{
	if ( gpio_get_value ( TSP_SDA ) )
		return(1);
	else
		return(0);
}


// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SCLKHigh()
// Set the SCLK pin High
// ****************************************************************************
void SCLKHigh(void)
{
	gpio_direction_output(TSP_SCL, 1);//gpio output high
	gpio_set_value(TSP_SCL,1);

	delay_us(1);
}


// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SCLKLow()
// Make Clock pin Low
// ****************************************************************************
void SCLKLow(void)
{
	gpio_direction_output(TSP_SCL, 0);//gpio output low
	gpio_set_value(TSP_SCL,0);
	
	delay_us(1);
}


#ifndef RESET_MODE  // Only needed for power cycle mode
// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetSCLKHiZ()
// Set SCLK pin to HighZ drive mode.
// ****************************************************************************
void SetSCLKHiZ(void)
{
	gpio_direction_input(TSP_SCL);//gpio input
}
#endif

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetSCLKStrong()
// Set SCLK to an output (Strong drive mode)
// ****************************************************************************
void SetSCLKStrong(void)
{
	gpio_direction_output(TSP_SCL ,0);//gpio output	(low)
	gpio_set_value(TSP_SCL,0);	
}


// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetSDATAHigh()
// Make SDATA pin High
// ****************************************************************************
void SetSDATAHigh(void)
{
	gpio_direction_output(TSP_SDA ,1);//gpio output high
	gpio_set_value(TSP_SDA,1);	
	delay_us(2);
}

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetSDATALow()
// Make SDATA pin Low
// ****************************************************************************
void SetSDATALow(void)
{
	gpio_direction_output(TSP_SDA, 0);//gpio output low
	gpio_set_value(TSP_SDA,0);	
	delay_us(2);
}

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetSDATAHiZ()
// Set SDATA pin to an input (HighZ drive mode).
// ****************************************************************************
void SetSDATAHiZ(void)
{
	gpio_direction_input(TSP_SDA);//gpio input	
}

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetSDATAStrong()
// Set SDATA for transmission (Strong drive mode) -- as opposed to being set to
// High Z for receiving data.
// ****************************************************************************
void SetSDATAStrong(void)
{
	gpio_direction_output(TSP_SDA, 1);//gpio output	(high)
	gpio_set_value(TSP_SDA,1);	
}

#ifdef RESET_MODE
// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetXRESStrong()
// Set external reset (XRES) to an output (Strong drive mode).
// ****************************************************************************
void SetXRESStrong(void)
{
    PRT2DM0 |=  XRES_PIN;
    PRT2DM1 &= ~XRES_PIN;
    PRT2DM2 &= ~XRES_PIN;
}

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// AssertXRES()
// Set XRES pin High
// ****************************************************************************
void AssertXRES(void)
{
    PRT2DR |= XRES_PIN;
}

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// DeassertXRES()
// Set XRES pin low.
// ****************************************************************************
void DeassertXRES(void)
{
    PRT2DR &= ~XRES_PIN;
}
#else

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// SetTargetVDDStrong()
// Set VDD pin (PWR) to an output (Strong drive mode).
// ****************************************************************************
void SetTargetVDDStrong(void)
{
	TchDrv_DownloadVddSetLow();	
}

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// ApplyTargetVDD()
// Provide power to the target PSoC's Vdd pin through a GPIO.
// ****************************************************************************
void ApplyTargetVDD(void)
{
	TchDrv_DownloadVddSetHigh();
}

// ********************* LOW-LEVEL ISSP SUBROUTINE SECTION ********************
// ****************************************************************************
// ****                        PROCESSOR SPECIFIC                          ****
// ****************************************************************************
// ****                      USER ATTENTION REQUIRED                       ****
// ****************************************************************************
// RemoveTargetVDD()
// Remove power from the target PSoC's Vdd pin.
// ****************************************************************************
void RemoveTargetVDD(void)
{
	TchDrv_DownloadVddSetLow();
}
#endif
//end of file ISSP_Drive_Routines.c



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						Issp_routines.c
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define PROGRAM_DATA	0x11

unsigned char  bTargetDataIN;
//unsigned char  abTargetDataOUT_secure[TARGET_DATABUFF_LEN] ={0x00,};

unsigned char  bTargetAddress;
unsigned char  bTargetDataPtr = 0;
unsigned char  bTargetID[10];
unsigned char  bTargetStatus; // bTargetStatus[10];			//PTJ: created to support READ-STATUS in fReadStatus()

unsigned char  fIsError = 0;

/* ((((((((((((((((((((( LOW-LEVEL ISSP SUBROUTINE SECTION ))))))))))))))))))))
   (( The subroutines in this section use functions from the C file          ))
   (( ISSP_Drive_Routines.c. The functions in that file interface to the     ))
   (( processor specific hardware. So, these functions should work as is, if ))
   (( the routines in ISSP_Drive_Routines.c are correctly converted.         ))
   (((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))*/

// ============================================================================
// RunClock()
// Description:
// Run Clock without sending/receiving bits. Use this when transitioning from
// write to read and read to write "num_cycles" is number of SCLK cycles, not
// number of counter cycles.
//
// SCLK cannot run faster than the specified maximum frequency of 8MHz. Some
// processors may need to have delays added after setting SCLK low and setting
// SCLK high in order to not exceed this specification. The maximum frequency
// of SCLK should be measured as part of validation of the final program
//
// ============================================================================
void RunClock(unsigned int iNumCycles)
{
    int i;

    for(i=0; i < iNumCycles; i++)
    {
        SCLKLow();
        SCLKHigh();
    }
}

// ============================================================================
// bReceiveBit()
// Clocks the SCLK pin (high-low-high) and reads the status of the SDATA pin
// after the rising edge.
//
// SCLK cannot run faster than the specified maximum frequency of 8MHz. Some
// processors may need to have delays added after setting SCLK low and setting
// SCLK high in order to not exceed this specification. The maximum frequency
// of SCLK should be measured as part of validation of the final program
//
// Returns:
//     0 if SDATA was low
//     1 if SDATA was high
// ============================================================================
unsigned char bReceiveBit(void)
{
    SCLKLow();
    SCLKHigh();
    if (fSDATACheck()) {
        return(1);
    }
    else {
        return(0);
    }
}

// ============================================================================
// bReceiveByte()
// Calls ReceiveBit 8 times to receive one byte.
// Returns:
//     The 8-bit values recieved.
// ============================================================================
unsigned char bReceiveByte(void)
{
    unsigned char b;
    unsigned char bCurrByte = 0x00;

    for (b=0; b<8; b++) {
        bCurrByte = (bCurrByte<<1) + bReceiveBit();
    }
    return(bCurrByte);
}


// ============================================================================
// SendByte()
// This routine sends up to one byte of a vector, one bit at a time.
//    bCurrByte   the byte that contains the bits to be sent.
//    bSize       the number of bits to be sent. Valid values are 1 to 8.
//
// SCLK cannot run faster than the specified maximum frequency of 8MHz. Some
// processors may need to have delays added after setting SCLK low and setting
// SCLK high in order to not exceed this specification. The maximum frequency
// of SCLK should be measured as part of validation of the final program
//
// There is no returned value.
// ============================================================================
void SendByte(unsigned char bCurrByte, unsigned char bSize)
{
    unsigned char b = 0;

    for(b=0; b<bSize; b++)
    {
        if (bCurrByte & 0x80)
        {
            // Send a '1'
            SetSDATAHigh();
            SCLKHigh();
            SCLKLow();
        }
        else
        {
            // Send a '0'
            SetSDATALow();
            SCLKHigh();
            SCLKLow();
        }
        bCurrByte = bCurrByte << 1;
    }
}

// ============================================================================
// SendVector()
// This routine sends the vector specifed. All vectors constant strings found
// in ISSP_Vectors.h.  The data line is returned to HiZ after the vector is
// sent.
//    bVect      a pointer to the vector to be sent.
//    nNumBits   the number of bits to be sent.
//    bCurrByte  scratch var to keep the byte to be sent.
//
// There is no returned value.
// ============================================================================
void SendVector(const unsigned char* bVect, unsigned int iNumBits)
{
    SetSDATAStrong();
    while(iNumBits > 0)
    {
        if (iNumBits >= 8) {
            SendByte(*(bVect), 8);
            iNumBits -= 8;
            bVect++;
        }
        else {
            SendByte(*(bVect), iNumBits);
            iNumBits = 0;
        }
    }
    SetSDATAHiZ();
}


// ============================================================================
// fDetectHiLoTransition()
// Waits for transition from SDATA = 1 to SDATA = 0.  Has a 100 msec timeout.
// TRANSITION_TIMEOUT is a loop counter for a 100msec timeout when waiting for
// a high-to-low transition. This is used in the polling loop of
// fDetectHiLoTransition(). The timing of the while(1) loops can be calculated
// and the number of loops is counted, using iTimer, to determine when 100
// msec has passed.
//
// SCLK cannot run faster than the specified maximum frequency of 8MHz. Some
// processors may need to have delays added after setting SCLK low and setting
// SCLK high in order to not exceed this specification. The maximum frequency
// of SCLK should be measured as part of validation of the final program
//
// Returns:
//     0 if successful
//    -1 if timed out.
// ============================================================================
signed char fDetectHiLoTransition(void)
{
    // nTimer breaks out of the while loops if the wait in the two loops totals
    // more than 100 msec.  Making this static makes the loop run a faster.
    // This is really a processor/compiler dependency and it not needed.
    unsigned long int iTimer=0;

	/// printk("[TSP] %s, %d\n", __func__, __LINE__);

    // NOTE:
    // These loops look unconventional, but it is necessary to check SDATA_PIN
    // as shown because the transition can be missed otherwise, due to the
    // length of the SDATA Low-High-Low after certain commands.

    // Generate clocks for the target to pull SDATA High
    iTimer = TRANSITION_TIMEOUT;
    while(1)
    {
        SCLKLow();
        if (fSDATACheck())       // exit once SDATA goes HI
        {
        break;
        }
        SCLKHigh();
        // If the wait is too long then timeout
        if (iTimer-- == 0) {
			printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
            return (ERROR);
        }
    }
    // Generate Clocks and wait for Target to pull SDATA Low again
    iTimer = TRANSITION_TIMEOUT;              // reset the timeout counter
    while(1)
    {
        SCLKLow();
        if (!fSDATACheck()) // exit once SDATA returns LOW
        {   
            break;
        }
        SCLKHigh();
        // If the wait is too long then timeout
        if (iTimer-- == 0) {
			printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
            return (ERROR);
        }
    }
    return (PASS);
}

signed char fDetectHiLoTransition_2(void)
{
    // nTimer breaks out of the while loops if the wait in the two loops totals
    // more than 100 msec.  Making this static makes the loop run a faster.
    // This is really a processor/compiler dependency and it not needed.
    unsigned long int iTimer=0;

	///	printk("[TSP] %s, %d\n", __func__, __LINE__);

    // NOTE:
    // These loops look unconventional, but it is necessary to check SDATA_PIN
    // as shown because the transition can be missed otherwise, due to the
    // length of the SDATA Low-High-Low after certain commands.

    // Generate clocks for the target to pull SDATA High
    iTimer = TRANSITION_TIMEOUT;
    while(1)
    {
        //SCLKLow();
        if (fSDATACheck())       // exit once SDATA goes HI
        break;
        //SCLKHigh();
        // If the wait is too long then timeout
        if (iTimer-- == 0) {
			printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
            return (ERROR);
        }
    }
    // Generate Clocks and wait for Target to pull SDATA Low again
    iTimer = TRANSITION_TIMEOUT;              // reset the timeout counter
    while(1)
    {
        //SCLKLow();
        if (!fSDATACheck()) {   // exit once SDATA returns LOW
            break;
        }
        //SCLKHigh();
        // If the wait is too long then timeout
        if (iTimer-- == 0) {
			printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
            return (ERROR);
        }
    }
    return (PASS);
}


/* ((((((((((((((((((((( HIGH-LEVEL ISSP ROUTINE SECTION ))))))))))))))))))))))
   (( These functions are mostly made of calls to the low level routines     ))
   (( above.  This should isolate the processor-specific changes so that     ))
   (( these routines do not need to be modified.                             ))
   (((((((((((((((((((((((((((((((((((())))))))))))))))))))))))))))))))))))))))*/

#ifdef RESET_MODE
// ============================================================================
// fXRESInitializeTargetForISSP()
// Implements the intialization vectors for the device.
// Returns:
//     0 if successful
//     INIT_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fXRESInitializeTargetForISSP(void)
{
    // Configure the pins for initialization
    SetSDATAHiZ();
    SetSCLKStrong();
    SCLKLow();
    SetXRESStrong();

    // Cycle reset and put the device in programming mode when it exits reset
    AssertXRES();
    Delay(XRES_CLK_DELAY);
    DeassertXRES();

    // !!! NOTE:
    //  The timing spec that requires that the first Init-Vector happen within
    //  1 msec after the reset/power up. For this reason, it is not advisable
    //  to separate the above RESET_MODE or POWER_CYCLE_MODE code from the
    //  Init-Vector instructions below. Doing so could introduce excess delay
    //  and cause the target device to exit ISSP Mode.

    //PTJ: Send id_setup_1 instead of init1_v
    //PTJ: both send CA Test Key and do a Calibrate1 SROM function
    SendVector(id_setup_1, num_bits_id_setup_1);
    if (fIsError = fDetectHiLoTransition()) {
        return(INIT_ERROR);
    }
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);

    // NOTE: DO NOT not wait for HiLo on SDATA after vector Init-3
    //       it does not occur (per spec).
    return(PASS);
}

#else  //else = the part is power cycle programmed

// ============================================================================
// fPowerCycleInitializeTargetForISSP()
// Implements the intialization vectors for the device.
// The first time fDetectHiLoTransition is called the Clk pin is highZ because
// the clock is not needed during acquire.
// Returns:
//     0 if successful
//     INIT_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fPowerCycleInitializeTargetForISSP(void)
{

	/// printk("[TSP] %s, %d\n", __func__, __LINE__);

    // Set all pins to highZ to avoid back powering the PSoC through the GPIO
    // protection diodes.
//    SetSCLKHiZ();
//    SetSDATAHiZ();

    // Turn on power to the target device before other signals
    SetTargetVDDStrong();
	mdelay(200);//200ms
    ApplyTargetVDD();
    // wait 1msec for the power to stabilize

//    for (n=0; n<10; n++) {
//        Delay(DELAY100us);
//    }
	//mdelay(1);//1ms


    // Set SCLK to high Z so there is no clock and wait for a high to low
    // transition on SDAT. SCLK is not needed this time.
    SetSCLKHiZ();
	SetSDATAHiZ(); // CY: adding 2012. 02.06

/*	
    if (fIsError = fDetectHiLoTransition_2()) {
		printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);		
        return(INIT_ERROR);
    }
*/

//CY: adding 2012.02.06
	delay_us(100);
	if(fSDATACheck())
	{
	  if ((fIsError = fDetectHiLoTransition())) {
						return(INIT_ERROR);
		}
	}
	else
	{
		delay_us(10000);
	}
//CY: adding 2012.02.06


    // Configure the pins for initialization
    SetSDATAHiZ();
    SetSCLKStrong();
    SCLKLow();					//PTJ: DO NOT SET A BREAKPOINT HERE AND EXPECT SILICON ID TO PASS!

    // !!! NOTE:
    //  The timing spec that requires that the first Init-Vector happen within
    //  1 msec after the reset/power up. For this reason, it is not advisable
    //  to separate the above RESET_MODE or POWER_CYCLE_MODE code from the
    //  Init-Vector instructions below. Doing so could introduce excess delay
    //  and cause the target device to exit ISSP Mode.

    SendVector(id_setup_1, num_bits_id_setup_1);
    if ((fIsError = fDetectHiLoTransition())) {
		printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);		
        return(INIT_ERROR);
    }
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);

    // NOTE: DO NOT not wait for HiLo on SDATA after vector Init-3
    //       it does not occur (per spec).
    return(PASS);
}
#endif


// ============================================================================
// fVerifySiliconID()
// Returns:
//     0 if successful
//     Si_ID_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fVerifySiliconID(void)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    SendVector(id_setup_2, num_bits_id_setup_2);
    if ((fIsError = fDetectHiLoTransition()))
    {
        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("fDetectHiLoTransition Error");
        #endif
		printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
        return(SiID_ERROR);
    }
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);

    SendVector(tsync_enable, num_bits_tsync_enable);

    //Send Read ID vector and get Target ID
    SendVector(read_id_v, 11);      // Read-MSB Vector is the first 11-Bits
    RunClock(2);                    // Two SCLK cycles between write & read
    bTargetID[0] = bReceiveByte();
    RunClock(1);
    SendVector(read_id_v+2, 12);    // 1+11 bits starting from the 3rd byte

    RunClock(2);                    // Read-LSB Command
    bTargetID[1] = bReceiveByte();

    RunClock(1);
    SendVector(read_id_v+4, 1);     // 1 bit starting from the 5th byte

    //read Revision ID from Accumulator A and Accumulator X
    //SendVector(read_id_v+5, 11);	//11 bits starting from the 6th byte
    //RunClock(2);
    //bTargetID[2] = bReceiveByte();	//Read from Acc.X
    //RunClock(1);
    //SendVector(read_id_v+7, 12);    //1+11 bits starting from the 8th byte
    //
    //RunClock(2);
    //bTargetID[3] = bReceiveByte();	//Read from Acc.A
    //
    //RunClock(1);
    //SendVector(read_id_v+4, 1);     //1 bit starting from the 5th byte,

    SendVector(tsync_disable, num_bits_tsync_disable);


    #ifdef TX_ON
        // Print READ-ID
        UART_PutCRLF(0);
        UART_PutString("Silicon-ID : ");
        UART_PutChar(' ');
        UART_PutHexByte(bTargetID[0]);
        UART_PutChar(' ');
        UART_PutHexByte(bTargetID[1]);
        UART_PutChar(' ');
    #endif

    #ifdef LCD_ON
        LCD_Char_Position(1, 0);
        LCD_Char_PrintString("ID : ");
        LCD_Char_PrintInt8(bTargetID[0]);
        LCD_Char_PutChar(' ');
        LCD_Char_PrintInt8(bTargetID[1]);
        LCD_Char_PutChar(' ');
    #endif

    if (bTargetID[0] != target_id_v[0] /*|| bTargetID[1] != target_id_v[1]*/)
    {
    	printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
        return(SiID_ERROR);
    }
    else
    {
        return(PASS);
    }
}

// PTJ: =======================================================================
// fReadStatus()
// Returns:
//     0 if successful
//     _____ if timed out on handshake to the device.
// ============================================================================
signed char fReadStatus(void)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    SendVector(tsync_enable, num_bits_tsync_enable);

    //Send Read ID vector and get Target ID
    SendVector(read_status, 11);      // Read-MSB Vector is the first 11-Bits
    RunClock(2);                    // Two SCLK cycles between write & read
    bTargetStatus = bReceiveByte();
    RunClock(1);
    SendVector(read_status+2, 1);    // 12 bits starting from the 3rd character

    SendVector(tsync_disable, num_bits_tsync_disable);

    if (bTargetStatus == 0x00)  // if bTargetStatus is 0x00, result is pass.
    {
        return PASS;
    }
    else
    {
    	printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
        return BLOCK_ERROR;
    }

}

// PTJ: =======================================================================
// fReadWriteSetup()
// PTJ: The READ-WRITE-SETUP vector will enable TSYNC and switches the device
//		to SRAM bank1 for PROGRAM-AND-VERIFY, SECURE and VERIFY-SETUP.
// Returns:
//     0 if successful
//     _____ if timed out on handshake to the device.
// ============================================================================
signed char fReadWriteSetup(void)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

	SendVector(read_write_setup, num_bits_read_write_setup);
	return(PASS);					//PTJ: is there anything else that should be done?
}

// ============================================================================
// fEraseTarget()
// Perform a bulk erase of the target device.
// Returns:
//     0 if successful
//     ERASE_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fEraseTarget(void)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    SendVector(erase, num_bits_erase);
    if ((fIsError = fDetectHiLoTransition())) {
		printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
        return(ERASE_ERROR);
    }
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);
    return(PASS);
}


// ============================================================================
// LoadTarget()
// Transfers data from array in Host to RAM buffer in the target.
// Returns the checksum of the data.
// ============================================================================
unsigned int iLoadTarget(void)
{
unsigned char bTemp;
unsigned int  iChecksumData = 0;

	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    // Set SDATA to Strong Drive here because SendByte() does not
    SetSDATAStrong();

    // Transfer the temporary RAM array into the target.
    // In this section, a 128-Byte array was specified by #define, so the entire
    // 128-Bytes are written in this loop.
    bTargetAddress = 0x00;
    bTargetDataPtr = 0x00;

    while(bTargetDataPtr < TARGET_DATABUFF_LEN) {
        bTemp = abTargetDataOUT[bTargetDataPtr];
        iChecksumData += bTemp;

        SendByte(write_byte_start,4);    //PTJ: we need to be able to write 128 bytes from address 0x80 to 0xFF
        SendByte(bTargetAddress, 7);	 //PTJ: we need to be able to write 128 bytes from address 0x80 to 0xFF
        SendByte(bTemp, 8);
        SendByte(write_byte_end, 3);

        // !!!NOTE:
        // SendByte() uses MSbits, so inc by '2' to put the 0..128 address into
        // the seven MSBit locations.
        //
        // This can be confusing, but check the logic:
        //   The address is only 7-Bits long. The SendByte() subroutine will
        // send however-many bits, BUT...always reads them bits from left-to-
        // right. So in order to pass a value of 0..128 as the address using
        // SendByte(), we have to left justify the address by 1-Bit.
        //   This can be done easily by incrementing the address each time by
        // '2' rather than by '1'.

        bTargetAddress += 2;			//PTJ: inc by 2 in order to support a 128 byte address space
        bTargetDataPtr++;
    }

    return(iChecksumData);
}


// ============================================================================
// fProgramTargetBlock()
// Program one block with data that has been loaded into a RAM buffer in the
// target device.
// Returns:
//     0 if successful
//     BLOCK_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fProgramTargetBlock(unsigned char bBankNumber, unsigned char bBlockNumber)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    SendVector(tsync_enable, num_bits_tsync_enable);

    SendVector(set_block_num, num_bits_set_block_num);

	// Set the drive here because SendByte() does not.
    SetSDATAStrong();
    SendByte(bBlockNumber,8);
    SendByte(set_block_num_end, 3);

    SendVector(tsync_disable, num_bits_tsync_disable);	//PTJ:

    // Send the program-block vector.
    SendVector(program_and_verify, num_bits_program_and_verify);		//PTJ: PROGRAM-AND-VERIFY
    // wait for acknowledge from target.
    if ((fIsError = fDetectHiLoTransition()))
    {
    	printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
        return(BLOCK_ERROR);
    }
    // Send the Wait-For-Poll-End vector
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);
    return(PASS);

    //PTJ: Don't do READ-STATUS here because that will
    //PTJ: require that we return multiple error values, if error occurs
}


// ============================================================================
// fAddTargetBankChecksum()
// Reads and adds the target bank checksum to the referenced accumulator.
// Returns:
//     0 if successful
//     VERIFY_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fAccTargetBankChecksum(unsigned int* pAcc)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    unsigned int wCheckSumData=0;

    SendVector(checksum_v, num_bits_checksum);

    if ((fIsError = fDetectHiLoTransition()))
    {
        return(CHECKSUM_ERROR);
    }
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);

    //SendVector(tsync_enable, num_bits_tsync_enable);

    //Send Read Checksum vector and get Target Checksum
    SendVector(read_checksum_v, 11);     // first 11-bits is ReadCKSum-MSB
    RunClock(2);                         // Two SCLKs between write & read
    bTargetDataIN = bReceiveByte();
    wCheckSumData = ((unsigned int)(bTargetDataIN))<<8;

    RunClock(1);                         // See Fig. 6
    SendVector(read_checksum_v + 2, 12); // 12 bits starting from 3rd character
    RunClock(2);                         // Read-LSB Command
    bTargetDataIN = bReceiveByte();
    wCheckSumData |= (unsigned int) bTargetDataIN;
    RunClock(1);
    SendVector(read_checksum_v + 4, 1);  // Send the final bit of the command

    //SendVector(tsync_disable, num_bits_tsync_disable);

    *pAcc = wCheckSumData;

    return(PASS);
}


// ============================================================================
// ReStartTarget()
// After programming, the target PSoC must be reset to take it out of
// programming mode. This routine performs a reset.
// ============================================================================
void ReStartTarget(void)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

#ifdef RESET_MODE
    // Assert XRES, then release, then disable XRES-Enable
    AssertXRES();
    Delay(XRES_CLK_DELAY);
    DeassertXRES();
#else
    // Set all pins to highZ to avoid back powering the PSoC through the GPIO
    // protection diodes.
    SetSCLKHiZ();
    SetSDATAHiZ();
    // Cycle power on the target to cause a reset
    RemoveTargetVDD();
	mdelay(200);
    ApplyTargetVDD();
	mdelay(200);
#endif
}

// ============================================================================
// fVerifySetup()
// Verify the block just written to. This can be done byte-by-byte before the
// protection bits are set.
// Returns:
//     0 if successful
//     BLOCK_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fVerifySetup(unsigned char bBankNumber, unsigned char bBlockNumber)
{
	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    SendVector(tsync_enable, num_bits_tsync_enable);

    SendVector(set_block_num, num_bits_set_block_num);

	//Set the drive here because SendByte() does not
    SetSDATAStrong();
    SendByte(bBlockNumber,8);
    SendByte(set_block_num_end, 3);

    SendVector(tsync_disable, num_bits_tsync_disable);

    SendVector(verify_setup, num_bits_my_verify_setup);
    if ((fIsError = fDetectHiLoTransition()))
    {
    	printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
        return(VERIFY_ERROR);
    }
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);

    return(PASS);
}

// ============================================================================
// fReadByteLoop()
// Reads the data back from Target SRAM and compares it to expected data in
// Host SRAM
// Returns:
//     0 if successful
//     BLOCK_ERROR if timed out on handshake to the device.
// ============================================================================

signed char fReadByteLoop(void)
{
	bTargetAddress = 0;
    bTargetDataPtr = 0;

	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    while(bTargetDataPtr < TARGET_DATABUFF_LEN)
    {
        //Send Read Byte vector and then get a byte from Target
        SendVector(read_byte_v, 4);
        // Set the drive here because SendByte() does not
        SetSDATAStrong();
        SendByte(bTargetAddress,7);

        RunClock(2);       // Run two SCLK cycles between writing and reading
        SetSDATAHiZ();     // Set to HiZ so Target can drive SDATA
        bTargetDataIN = bReceiveByte();

        RunClock(1);
        SendVector(read_byte_v + 1, 1);     // Send the ReadByte Vector End

        // Test the Byte that was read from the Target against the original
        // value (already in the 128-Byte array "abTargetDataOUT[]"). If it
        // matches, then bump the address & pointer,loop-back and continue.
        // If it does NOT match abort the loop and return and error.
        if (bTargetDataIN != abTargetDataOUT[bTargetDataPtr])
        {
            #ifdef TX_ON
                UART_PutCRLF(0);
                UART_PutString("bTargetDataIN : ");
                UART_PutHexByte(bTargetDataIN);
                UART_PutString(" abTargetDataOUT : ");
                UART_PutHexByte(abTargetDataOUT[bTargetDataPtr]);
            #endif
			printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
            return(BLOCK_ERROR);
        }

        bTargetDataPtr++;
        // Increment the address by 2 to accomodate 7-Bit addressing
        // (puts the 7-bit address into MSBit locations for "SendByte()").
        bTargetAddress += 2;

    }

    return(PASS);
}

// ============================================================================
// fSecureTargetFlash()
// Before calling, load the array, abTargetDataOUT, with the desired security
// settings using LoadArrayWithSecurityData(StartAddress,Length,SecurityType).
// The can be called multiple times with different SecurityTypes as needed for
// particular Flash Blocks. Or set them all the same using the call below:
// LoadArrayWithSecurityData(0,SECURITY_BYTES_PER_BANK, 0);
// Returns:
//     0 if successful
//     SECURITY_ERROR if timed out on handshake to the device.
// ============================================================================
signed char fSecureTargetFlash(void)
{
    unsigned char bTemp;

	/// printk( "[TSP] %s, %d\n", __func__, __LINE__);

    // Transfer the temporary RAM array into the target
    bTargetAddress = 0x00;
    bTargetDataPtr = 0x00;

    SetSDATAStrong();
    while(bTargetDataPtr < SECURITY_BYTES_PER_BANK)
    {
        bTemp = abTargetDataOUT_secure[bTargetDataPtr];
        SendByte(write_byte_start,4);
        SendByte(bTargetAddress, 7);
        SendByte(bTemp, 8);
        SendByte(write_byte_end, 3);


        // SendBytes() uses MSBits, so increment the address by '2' to put
        // the 0..n address into the seven MSBit locations
        bTargetAddress += 2;				//PTJ: inc by 2 in order to support a 128 byte address space
        bTargetDataPtr++;
    }

    SendVector(secure, num_bits_secure);	//PTJ:
    if ((fIsError = fDetectHiLoTransition()))
    {
    	printk( "[TSP] %s, %d : Error\n", __func__, __LINE__);
        return(SECURITY_ERROR);
    }
    SendVector(wait_and_poll_end, num_bits_wait_and_poll_end);
    return(PASS);
}




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						Main.c
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

unsigned char bBankCounter;
unsigned int  iBlockCounter;
unsigned int  iChecksumData;
unsigned int  iChecksumTarget;

#ifdef TX_ON
void UART_PutHexHalf(char ch)
{
    if(ch >=10)
        UART_PutChar(ch + 'A'-10);
    else
        UART_PutChar(ch + '0');
}

void UART_PutHexByte(unsigned char ch)
{
    UART_PutHexHalf(ch >> 4);
    UART_PutHexHalf(ch & 0x0f);
}

void UART_PutHexWord(unsigned int ch)
{
    UART_PutHexByte(ch>>8);
    UART_PutHexByte(ch&0xff);
}

#endif


/* ========================================================================= */
// ErrorTrap()
// Return is not valid from main for PSOC, so this ErrorTrap routine is used.
// For some systems returning an error code will work best. For those, the
// calls to ErrorTrap() should be replaced with a return(bErrorNumber). For
// other systems another method of reporting an error could be added to this
// function -- such as reporting over a communcations port.
/* ========================================================================= */
void ErrorTrap(unsigned char bErrorNumber)
{
	printk( "[TSP] %s, %d : ErrorNumber = %d\n", __func__, __LINE__, bErrorNumber);

    #ifndef RESET_MODE
        // Set all pins to highZ to avoid back powering the PSoC through the GPIO
        // protection diodes.
        SetSCLKHiZ();
        SetSDATAHiZ();
        // If Power Cycle programming, turn off the target
        RemoveTargetVDD();
    #endif


    #ifdef TX_ON
        UART_PutCRLF(0);
        UART_PutString("ErrorTrap");
        UART_PutHexByte(bErrorNumber);
    #endif

    #ifdef LCD_ON
        LCD_Char_Position(1, 0);
        LCD_Char_PrintString("                ");
        LCD_Char_Position(1, 0);
        LCD_Char_PrintString("ErrorTrap");
        LCD_Char_PrintInt8(bErrorNumber);
    #endif
}

/* ========================================================================= */
/* MAIN LOOP                                                                 */
/* Based on the diagram in the AN2026                                        */
/* ========================================================================= */
unsigned char make2ChTo1(unsigned char hi, unsigned char lo)
{
    unsigned char ch;
    
    if(hi == 'A' || hi == 'a')
        hi = 0xa;
    else if(hi == 'B' || hi == 'b')
        hi = 0xb;
    else if(hi == 'C' || hi == 'c')
        hi = 0xc;
    else if(hi == 'D' || hi == 'd')
        hi = 0xd;
    else if(hi == 'E' || hi == 'e')
        hi = 0xe;
    else if(hi == 'F' || hi == 'f')
        hi = 0xf;
    else
        hi = hi;

    if(lo == 'A' || lo == 'a')
        lo = 0xa;
    else if(lo == 'B' || lo == 'b')
        lo = 0xb;
    else if(lo == 'C' || lo == 'c')
        lo = 0xc;
    else if(lo == 'D' || lo == 'd')
        lo = 0xd;
    else if(lo == 'E' || lo == 'e')
        lo = 0xe;
    else if(lo == 'F' || lo == 'f')
        lo = 0xf;
    else
        lo = lo;
    
    ch = ((hi&0x0f) << 4) | (lo & 0x0f);

    return ch;
}


unsigned int load_tma140_frimware_data(int HW_ver)
{
	int i,j;

	mm_segment_t oldfs;
	int fd;
	uint8_t* buffer;
	int result = -1;

	unsigned int firmwareline, onelinelength;
	unsigned char temp_onelinedata[128];
	extern unsigned char tsp_special_update;
	
	if(tsp_special_update == 0)//normal firmware update from phone-binary
	{
		if(HW_ver == 1)
		{

			printk("[TSP] firmware_down_in_bin\n");
		
			for(i=0; i<512; i++)
				for(j=0; j<64; j++)
					firmData[i][j] = BinaryData_HW01SW02[i*64 + j];

			return PASS;
		}
		else
		{
			return NO_FIRMWARE_ERROR;
		}
	}
	else if(tsp_special_update == 1)//special firmware update from t-flash
	{
		printk("[TSP] %s, %d\n", __func__, __LINE__ );

		oldfs = get_fs();
		set_fs (KERNEL_DS);   /* set KERNEL address space */

		/* - Get file Size */
		buffer = kmalloc(72192, GFP_KERNEL);/*141*512*/

		if(buffer == NULL) 
		{
			printk("[TSP] firmware_down_using_sdcard : alllocate mem fail\n");
			result = FILE_ACCESS_ERROR;
			goto error;
		}

		if((fd = sys_open("/mnt/sdcard/cori_tsp.hex", O_RDONLY | O_LARGEFILE, 0)) > 0)
		{
			if(sys_read(fd, buffer, 72192) > 0) 
			{
				sys_close(fd);
				printk("[TSP] firmware_down_using_sdcard : read file success\n");	
			}
			else
			{
				sys_close(fd);
				result = FILE_ACCESS_ERROR;
				printk("[TSP] firmware_down_using_sdcard : file read fail\n");				
				goto error; 			
			}
		}
		else
		{
			result = FILE_ACCESS_ERROR;
			printk("[TSP] firmware_down_using_sdcard : file open fail\n");
			goto error;
		}
		
		printk("\n[TSP] firmware_down_using_sdcard : firmware_data : START \n");
		
		for(firmwareline=0; firmwareline<512; firmwareline++)
		{
			i = 0;

			strncpy(temp_onelinedata, buffer + 141*firmwareline + 9, 128);
			for(onelinelength=0; onelinelength<64; onelinelength++)
			{
				firmData[firmwareline][onelinelength] = make2ChTo1(temp_onelinedata[i], temp_onelinedata[i+1]);
				i += 2;
			}
		}

		printk("\n[TSP] firmware_down_using_sdcard : firmware_data : END \n");
		
	
		result = PASS;

		error:
			if(buffer)
				kfree(buffer);

			set_fs(oldfs);


		return result;
	}
	else
	{
		return NO_FIRMWARE_ERROR;
	}
	
	
	return PASS;

}


int cypress_update(int HW_ver)
{
    // -- This example section of commands show the high-level calls to -------
    // -- perform Target Initialization, SilcionID Test, Bulk-Erase, Target ---
    // -- RAM Load, FLASH-Block Program, and Target Checksum Verification. ----
	unsigned int i;
	unsigned int aIndex;

	if ((fIsError = load_tma140_frimware_data(HW_ver)))
    {
        ErrorTrap(fIsError);
		return fIsError;
    }


    // >>>> ISSP Programming Starts Here <<<<

    // Acquire the device through reset or power cycle
    #ifdef RESET_MODE
        // Initialize the Host & Target for ISSP operations
        if (fIsError = fXRESInitializeTargetForISSP())
        {
            ErrorTrap(fIsError);
			return fIsError;
        }
    #else
        // Initialize the Host & Target for ISSP operations
        if ((fIsError = fPowerCycleInitializeTargetForISSP()))
        {
            ErrorTrap(fIsError);
			return fIsError;
        }
    #endif


    // Run the SiliconID Verification, and proceed according to result.
	if ((fIsError = fVerifySiliconID()))
    {
        ErrorTrap(fIsError);
		return fIsError;
    }
    #ifdef TX_ON
        UART_PutCRLF(0);
        UART_PutString("End VerifySiliconID");
    #endif

	/* Disable watchdog and interrupt */
	//TchDrv_DownloadDisableIRQ();	// Disable Baseband touch interrupt ISR.
	//TchDrv_DownloadDisableWD();		// Disable Baseband watchdog timer

        // Bulk-Erase the Device.
        if ((fIsError = fEraseTarget()))
        {
            ErrorTrap(fIsError);
			//return fIsError;
			goto MCSDL_DOWNLOAD_FINISH;
        }

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("End EraseTarget");
            UART_PutCRLF(0);
            UART_PutString("Program Flash Blocks Start");
            UART_PutCRLF(0);
        #endif


    #if 1   // program flash block
        //LCD_Char_Position(1, 0);
        //LCD_Char_PrintString("Program Flash Blocks Start");

        //==============================================================//
        // Program Flash blocks with predetermined data. In the final application
        // this data should come from the HEX output of PSoC Designer.

        iChecksumData = 0;     // Calculte the device checksum as you go
        for (iBlockCounter=0; iBlockCounter<BLOCKS_PER_BANK; iBlockCounter++)
        {
	  if ((fIsError = fReadWriteSetup()))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }

			aIndex = iBlockCounter*2;
			
			for(i=0;i<TARGET_DATABUFF_LEN;i++)
			{
				if(i<64)
				{
					abTargetDataOUT[i] = firmData[aIndex][i];
				}
				else
				{
					abTargetDataOUT[i] = firmData[aIndex+1][i-64];
				}
			}

            //LoadProgramData(bBankCounter, (unsigned char)iBlockCounter);
            iChecksumData += iLoadTarget();

            if ((fIsError = fProgramTargetBlock(bBankCounter,(unsigned char)iBlockCounter)))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }

            if ((fIsError = fReadStatus()))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }

            #ifdef TX_ON
                UART_PutChar('#');
            #endif

        }

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("Program Flash Blocks End");
        #endif

    #endif


    #if 1  // verify
        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("Verify Start");
            UART_PutCRLF(0);
        #endif

        //=======================================================//
        //PTJ: Doing Verify
        //PTJ: this code isnt needed in the program flow because we use PROGRAM-AND-VERIFY (ProgramAndVerify SROM Func)
        //PTJ: which has Verify built into it.
        // Verify included for completeness in case host desires to do a stand-alone verify at a later date.

        for (iBlockCounter=0; iBlockCounter<BLOCKS_PER_BANK; iBlockCounter++)
        {
        	//LoadProgramData(bBankCounter, (unsigned char) iBlockCounter);
			aIndex = iBlockCounter*2;
						
			for(i=0;i<TARGET_DATABUFF_LEN;i++)
			{
				if(i<64)
				{
					abTargetDataOUT[i] = firmData[aIndex][i];
				}
				else
				{
					abTargetDataOUT[i] = firmData[aIndex+1][i-64];
				}
			}

			if ((fIsError = fReadWriteSetup()))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }

			if ((fIsError = fVerifySetup(bBankCounter,(unsigned char)iBlockCounter)))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }


			if ((fIsError = fReadStatus())) {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }


			if ((fIsError = fReadWriteSetup())) {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }


			if ((fIsError = fReadByteLoop())) {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }

            #ifdef TX_ON
                UART_PutChar('.');
            #endif

        }

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("Verify End");
        #endif

    #endif // end verify


    #if 1

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("Security Start");
        #endif


        //=======================================================//
        // Program security data into target PSoC. In the final application this
        // data should come from the HEX output of PSoC Designer.
        for (bBankCounter=0; bBankCounter<NUM_BANKS; bBankCounter++)
        {
            //PTJ: READ-WRITE-SETUP used here to select SRAM Bank 1

	  if ((fIsError = fReadWriteSetup()))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }
            // Load one bank of security data from hex file into buffer
			#if 0//please check
			if (fIsError = fLoadSecurityData(bBankCounter))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }
			#endif
            // Secure one bank of the target flash
            if ((fIsError = fSecureTargetFlash()))
            {
                ErrorTrap(fIsError);
				//return fIsError;
				goto MCSDL_DOWNLOAD_FINISH;
            }
        }

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("End Security data");
        #endif

    #endif


    #if 0   // checksum

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("CheckSum Start");
        #endif

        //PTJ: Doing Checksum
        iChecksumTarget = 0;
        if (fIsError = fAccTargetBankChecksum(&iChecksumTarget))
        {
            ErrorTrap(fIsError);
			return fIsError;
        }

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("Checksum : iChecksumTarget (0x");
            UART_PutHexWord(iChecksumTarget);
            UART_PutString("), iChecksumData (0x");
            UART_PutHexWord(iChecksumData);
            UART_PutString(")");
        #endif

        #ifdef LCD_ON
            LCD_Char_Position(0, 0);
            LCD_Char_PrintString("TarChk : ");
            LCD_Char_PrintInt16(iChecksumTarget);
            LCD_Char_Position(1, 0);
            LCD_Char_PrintString("CalChk : ");
            LCD_Char_PrintInt16(iChecksumData);
        #endif


        if (iChecksumTarget != iChecksumData)
        {
            ErrorTrap(CHECKSUM_ERROR);
			return fIsError;
        }

        #ifdef TX_ON
            UART_PutCRLF(0);
            UART_PutString("End Checksum");
        #endif

    #endif

    // *** SUCCESS ***
    // At this point, the Target has been successfully Initialize, ID-Checked,
    // Bulk-Erased, Block-Loaded, Block-Programmed, Block-Verified, and Device-
    // Checksum Verified.


MCSDL_DOWNLOAD_FINISH :


    // You may want to restart Your Target PSoC Here.
    ReStartTarget();

	return fIsError;

}
// end of main()



//--------------------------------------------------------
//
//
//	Melfas MMS100 Series Download base v1.0 2010.04.05
//
//
//--------------------------------------------------------

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/irq.h>

#include <asm/gpio.h>
#include <asm/io.h>

#include <mach/gpio.h>

//#include "mms100_download.h"
#include "mcs8000_download.h"


//============================================================
//
//	Include MELFAS Binary code File ( ex> MELFAS_FIRM_bin.c)
//
//	Warning!!!!
//		Please, don't add binary.c file into project
//		Just #include here !!
//
//============================================================


//extern const UINT16 binary_nLength;
//extern const  UINT8 binary[];
extern UINT16 binary_nLength;
extern UINT8 *binary;

extern void mcsdl_select_binary_data(int hw_ver);

extern void mcsdl_delay(UINT32 nCount);


//---------------------------------
//	Downloading functions
//---------------------------------

//static int  mms100_ISC_download(const UINT8 *pData, const UINT16 nLength,INT8 IdxNum );
static int  mms100_ISC_download(UINT8 *pData, UINT16 nLength,INT8 IdxNum );

static void mms100_ISC_set_ready(void);
static void mms100_ISC_reboot_mcs(void);

static UINT8 mcsdl_read_ack(void);
static void mcsdl_ISC_read_32bits( UINT8 *pData );
static void mcsdl_ISC_write_bits(UINT32 wordData, int nBits);
static UINT8 mms100_ISC_read_data(UINT8 addr);

static void mms100_ISC_enter_download_mode();
static void mms100_ISC_firmware_update_mode_enter();
static UINT8 mms100_ISC_firmware_update(UINT8 *_pBinary_reordered, UINT16 _unDownload_size, UINT8 flash_start, UINT8 flash_end);
static UINT8 mms100_ISC_read_firmware_status();
static void mms100_ISC_send_crc_ok();
//static UINT8 mms100_ISC_slave_crc_ok();

static void mms100_ISC_leave_firmware_update_mode();
static void mcsdl_i2c_start(void);
static void mcsdl_i2c_stop(void);
static UINT8 mcsdl_read_byte(void);


//---------------------------------
//	For debugging display
//---------------------------------
#if MELFAS_ENABLE_DBG_PRINT
static void mcsdl_ISC_print_result(int nRet);
#endif


//----------------------------------
// Download enable command
//----------------------------------
#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
void melfas_send_download_enable_command(void)
{
	// TO DO : Fill this up
}
#endif


//============================================================
//
//	Main Download furnction
//
//   1. Run mcsdl_download( pBinary[IdxNum], nBinary_length[IdxNum], IdxNum);
//       IdxNum : 0 (Master Chip Download)
//       IdxNum : 1 (2Chip Download)
//
//
//============================================================
int mms100_ISC_download_binary_data(int hw_ver)
{
	int nRet;
#ifdef FW_FROM_FILE
	struct file *filp;
	spinlock_t			 lock;
	loff_t	pos;
	int 	ret = 0;
	long fw1_size = 0;
	long fw2_size = 0;
	mm_segment_t oldfs;
	unsigned char *fw_data1;
	unsigned char *fw_data2;

	oldfs = get_fs();
	set_fs(get_ds());

	filp = filp_open(MELFAS_FW1, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		pr_err("file open error:%d\n", (s32)filp);
		return -1;
	}

	fw1_size = filp->f_path.dentry->d_inode->i_size;
	pr_info("Size of the file : %ld(bytes)\n", fw1_size);

	fw_data1 = kmalloc(fw1_size, GFP_KERNEL);
	memset(fw_data1, 0, fw1_size);

	pos = 0;
	memset(fw_data1, 0, fw1_size);
	ret = vfs_read(filp, (char __user *)fw_data1, fw1_size, &pos);

	if(ret != fw1_size) {
		pr_err("Failed to read file %s (ret = %d)\n", MELFAS_FW1, ret);
		kfree(fw_data1);
		filp_close(filp, current->files);
		return -1;
	}

	filp_close(filp, current->files);

	filp = filp_open(MELFAS_FW2, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		pr_err("file open error:%d\n", (s32)filp);
		return -1;
	}

	fw2_size = filp->f_path.dentry->d_inode->i_size;
	pr_info("Size of the file : %ld(bytes)\n", fw2_size);

	fw_data2 = kmalloc(fw2_size, GFP_KERNEL);
	memset(fw_data2, 0, fw2_size);

	pos = 0;
	memset(fw_data2, 0, fw2_size);
	ret = vfs_read(filp, (char __user *)fw_data2, fw2_size, &pos);

	if(ret != fw2_size) {
		pr_err("Failed to read file %s (ret = %d)\n", MELFAS_FW2, ret);
		kfree(fw_data2);
		filp_close(filp, current->files);
		return -1;
	}

	filp_close(filp, current->files);

	set_fs(oldfs);
	spin_lock_init(&lock);
	spin_lock(&lock);
#endif

#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
	melfas_send_download_enable_command();
	mcsdl_delay(MCSDL_DELAY_100US);
#endif

	MELFAS_DISABLE_BASEBAND_ISR();					// Disable Baseband touch interrupt ISR.
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET();			// Disable Baseband watchdog timer

	//------------------------
	// Run Download
	//------------------------
#ifdef FW_FROM_FILE
	nRet = mms100_ISC_download( (const UINT8*) fw_data1, (const UINT16)fw1_size, 0);
	if (nRet)
		goto fw_error;
#if MELFAS_ISC_2CHIP_DOWNLOAD_ENABLE
	nRet = mms100_ISC_download( (const UINT8*) fw_data2, (const UINT16)fw2_size, 1);
	if (nRet)
		goto fw_error;
	spin_unlock(&lock);
#endif
#else
#if !MELFAS_ISP_DOWNLOAD
	mcsdl_select_binary_data(hw_ver);

	nRet = mms100_ISC_download( (const UINT8*) binary, (const UINT16)binary_nLength , 0);
	if (nRet)
		goto fw_error;
#endif
#if MELFAS_ISC_2CHIP_DOWNLOAD_ENABLE
	nRet = mms100_ISC_download( (const UINT8*) binary_2, (const UINT16)binary_nLength_2, 1); // Slave Binary data download
	if (nRet)
		goto fw_error;
#endif
#endif
	MELFAS_ROLLBACK_BASEBAND_ISR(); 				// Roll-back Baseband touch interrupt ISR.
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET(); 		// Roll-back Baseband watchdog timer
	return 0;
fw_error:
	//mcsdl_erase_flash(0);
	//mcsdl_erase_flash(1);
#ifdef FW_FROM_FILE
	spin_unlock(&lock);
#endif
	return nRet;
}

int mms100_ISC_download_binary_file(int hw_ver)
{
	int nRet;
	int i;

	mcsdl_select_binary_data(hw_ver);

	UINT8  *pBinary[2] = {binary, NULL};
	UINT16 nBinary_length[2] ={binary_nLength, 0};
	UINT8 IdxNum = 0 // MELFAS_2CHIP_DOWNLOAD_ENABLE; // ISC mode is used for master download.
	//==================================================
	//
	//	1. Read '.bin file'
	//	2. *pBinary[0]       : Binary data(Master)
	//	    *pBinary[1]       : Binary data(Slave)
	//	    nBinary_length[0] : Firmware size(Master)
	//	    nBinary_length[1] : Firmware size(Slave)
	//	3. Run mcsdl_download( pBinary[IdxNum], nBinary_length[IdxNum], IdxNum);
	//	    IdxNum : 0 (Master Chip Download)
	//	    IdxNum : 1 (2Chip Download)
	//
	//==================================================

#if 0
	// TO DO : File Process & Get file Size(== Binary size)
	//			This is just a simple sample
	FILE *fp;
	INT  nRead;

	//------------------------------
	// Open a file
	//------------------------------
	if (fopen(fp, "MELFAS_FIRMWARE.bin", "rb") == NULL)
	{
		return MCSDL_RET_FILE_ACCESS_FAILED;
	}

	//------------------------------
	// Get Binary Size
	//------------------------------
	fseek(fp, 0, SEEK_END);

	nBinary_length = (UINT16)ftell(fp);

	//------------------------------
	// Memory allocation
	//------------------------------
	pBinary = (UINT8*)malloc((INT)nBinary_length);

	if (pBinary == NULL)
	{
		return MCSDL_RET_FILE_ACCESS_FAILED;
	}

	//------------------------------
	// Read binary file
	//------------------------------
	fseek(fp, 0, SEEK_SET);

	nRead = fread(pBinary, 1, (INT)nBinary_length, fp);		// Read binary file

	if (nRead != (INT)nBinary_length)
	{
		fclose(fp);												// Close file

		if (pBinary != NULL)										// free memory alloced.
			free(pBinary);

		return MCSDL_RET_FILE_ACCESS_FAILED;
	}

	//------------------------------
	// Close file
	//------------------------------
	fclose(fp);
#endif

#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
	melfas_send_download_enable_command();
	mcsdl_delay(MCSDL_DELAY_100US);
#endif

	MELFAS_DISABLE_BASEBAND_ISR();                  // Disable Baseband touch interrupt ISR.
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET();          // Disable Baseband watchdog timer

	//for (i = 0;i <= IdxNum;i++)
	{
		//if (pBinary[i] != NULL && nBinary_length[i] > 0 && nBinary_length[i] < (32 * 1024))
		if (pBinary[0] != NULL && nBinary_length[0] > 0 && nBinary_length[0] < (32 * 1024))
		{
			//------------------------
			// Run Download
			//------------------------
			//nRet = mms100_ISC_download((const UINT8 *)pBinary[i], (const UINT16)nBinary_length[i], i);
			nRet = mms100_ISC_download((const UINT8 *)pBinary[0], (const UINT16)nBinary_length[0], 0);
		}
		else
		{
			nRet = MCSDL_RET_WRONG_BINARY;
		}
	}

	MELFAS_ROLLBACK_BASEBAND_ISR();			// Roll-back Baseband touch interrupt ISR.
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET();	// Roll-back Baseband watchdog timer

#if MELFAS_ENABLE_DBG_PRINT
	mcsdl_ISC_print_result( nRet );
#endif

#if 0
	if( pData != NULL )										// free memory alloced.
	free(pData);
#endif

	return ( nRet == MCSDL_RET_SUCCESS );

}


//------------------------------------------------------------------
//
//	Download function
//
//------------------------------------------------------------------
//static int mms100_ISC_download(const UINT8 *pBianry, const UINT16 unLength, INT8 IdxNum)
static int mms100_ISC_download(UINT8 *pBianry, UINT16 unLength, INT8 IdxNum)
{
	int nRet;
	int i=0;
	UINT8 fw_status = 0;
	INT8 dl_enable_bit = 0x00;
	UINT8 private_flash_start = ISC_PRIVATE_CONFIG_FLASH_START;
	UINT8 public_flash_start = ISC_PUBLIC_CONFIG_FLASH_START;
	UINT8 core_version;
	UINT8 flash_start[3] = {0,};
	UINT8 flash_end[3] =  {0,};

	//---------------------------------
	// Check Binary Size
	//---------------------------------
	if (unLength >= MELFAS_FIRMWARE_MAX_SIZE)
	{
		nRet = MCSDL_RET_PROGRAM_SIZE_IS_WRONG;
		goto MCSDL_DOWNLOAD_FINISH;
	}

	//---------------------------------
	// set download enable mode
	//---------------------------------
	if (MELFAS_CORE_FIRWMARE_UPDATE_ENABLE) 
	{
		dl_enable_bit |= 0x01;
		printk("<MELFAS> Core firmware download.\n");
	}
	if (MELFAS_PRIVATE_CONFIGURATION_UPDATE_ENABLE)
	{
		dl_enable_bit |= 0x02;
		printk("<MELFAS> Private Configration download.\n");
	}
	if (MELFAS_PUBLIC_CONFIGURATION_UPDATE_ENABLE)
	{
		dl_enable_bit |= 0x04;
		printk("<MELFAS> Public Configration download.\n");
	}

	//for (i = 0; i < 3; i++)
	{
		//if (dl_enable_bit & (1 << i))
		{
			//---------------------------------
			// Make it ready
			//---------------------------------
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
			printk("<MELFAS> Ready\n");
#endif

			mms100_ISC_set_ready();

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
			printk("<MELFAS> firmware_download_via_ISC start!!!\n");
#endif

			//--------------------------------------------------------------
			// INITIALIZE
			//--------------------------------------------------------------
			printk("<MELFAS> ISC_DOWNLOAD_MODE_ENTER\n\n");            
			mms100_ISC_enter_download_mode();
			mcsdl_delay(MCSDL_DELAY_100MS);

#if ISC_READ_DOWNLOAD_POSITION
			printk("<MELFAS> Read download position.\n\n");            
			private_flash_start = mms100_ISC_read_data(ISC_PRIVATE_CONFIGURATION_START_ADDR);
			public_flash_start = mms100_ISC_read_data(ISC_PUBLIC_CONFIGURATION_START_ADDR);
#endif

			flash_start[0] = 0;
			//flash_end[2] = 31;
			flash_end[0] = flash_end[2] = 31;
			//flash_start[1] = flash_end[0] = private_flash_start;
			flash_start[1] = private_flash_start;
			flash_start[2] = flash_end[1] = public_flash_start;
			printk("<MELFAS> Private Configration start at %2dKB, Public Configration start at %2dKB\n", private_flash_start, public_flash_start );

			//core_version = mms100_ISC_read_data(ISC_CORE_FIRMWARE_VERSION_ADDR);
			//printk("<MELFAS> Core firmware version : 0x%02x\n",core_version);

			mcsdl_delay(MCSDL_DELAY_60MS);

			//--------------------------------------------------------------
			// FIRMWARE UPDATE MODE ENTER
			//--------------------------------------------------------------
			printk("<MELFAS> FIRMWARE_UPDATE_MODE_ENTER\n\n");            
			mms100_ISC_firmware_update_mode_enter();
			mcsdl_delay(MCSDL_DELAY_60MS);

			fw_status = mms100_ISC_read_firmware_status();

			if (fw_status == 0x01)
			{
				printk("<MELFAS> Firmware update mode enter success!!!\n");
			}
			else
			{
				printk("<MELFAS> Error detected!! firmware status is 0x%02x.\n", fw_status);
				nRet = MCSDL_FIRMWARE_UPDATE_MODE_ENTER_FAILED;
				goto MCSDL_DOWNLOAD_FINISH;
			}

			mcsdl_delay(MCSDL_DELAY_60MS);

			//--------------------------------------------------------------
			// FIRMWARE UPDATE 
			//--------------------------------------------------------------
			printk("<MELFAS> FIRMWARE UPDATE\n\n");            
			//nRet = mms100_ISC_firmware_update((UINT8 *)pBianry, (UINT16)unLength, flash_start[i],flash_end[i]);
			nRet = mms100_ISC_firmware_update((UINT8 *)pBianry, (UINT16)unLength, flash_start[0],flash_end[0]);
			if(nRet != MCSDL_RET_SUCCESS) goto MCSDL_DOWNLOAD_FINISH;

			//--------------------------------------------------------------
			// LEAVE FIRMWARE UPDATE MODE
			//--------------------------------------------------------------
			printk("<MELFAS> LEAVE FIRMWARE UPDATE MODE\n\n");            
#if 0
			mms100_ISC_leave_firmware_update_mode();
			mcsdl_delay(MCSDL_DELAY_60MS);

			fw_status = mms100_ISC_read_firmware_status();

			if (fw_status == 0xFF || fw_status == 0x00 )
			{
				printk("<MELFAS> Living firmware update mode success!!!\n");
			}
			else
			{
				printk("<MELFAS> Error detected!! firmware status is 0x%02x.\n", fw_status);
				nRet = MCSDL_LEAVE_FIRMWARE_UPDATE_MODE_FAILED;
				goto MCSDL_DOWNLOAD_FINISH;
			}
#endif
			nRet = MCSDL_RET_SUCCESS;

MCSDL_DOWNLOAD_FINISH :

#if MELFAS_ENABLE_DBG_PRINT
			mcsdl_ISC_print_result( nRet );								// Show result
#endif

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
			printk("<MELMAS> Rebooting\n");
			printk("<MELMAS>  - Fin.\n\n");
#endif

			mms100_ISC_reboot_mcs();
		}
	}

	return nRet;
}


//------------------------------------------------------------------
//
//	Sub functions
//
//------------------------------------------------------------------
static UINT8 mms100_ISC_read_data(UINT8 addr)
{
	UINT32 wordData = 0x00000000;
	UINT8  write_buffer[4];
	UINT8 flash_start;

	mcsdl_i2c_start();
	write_buffer[0] = ISC_MODE_SLAVE_ADDRESS << 1;
	write_buffer[1] = addr; // command
	wordData = (write_buffer[0] << 24) | (write_buffer[1]<< 16);

	mcsdl_ISC_write_bits( wordData, 16 );
	mcsdl_delay(MCSDL_DELAY_10MS);

	mcsdl_i2c_start();
	// 1byte read
	wordData = (ISC_MODE_SLAVE_ADDRESS << 1 | 0x01) << 24;
	mcsdl_ISC_write_bits( wordData, 8 );
	flash_start = mcsdl_read_byte();
	wordData = (0x01) << 31;
	mcsdl_ISC_write_bits( wordData, 1 ); //Nack
	mcsdl_i2c_stop();
	return flash_start;
}

static void mms100_ISC_enter_download_mode()
{
	UINT32 wordData = 0x00000000;
	UINT8  write_buffer[4];

	mcsdl_i2c_start();
	write_buffer[0] = ISC_MODE_SLAVE_ADDRESS << 1; // slave addr
	write_buffer[1] = ISC_DOWNLOAD_MODE_ENTER; // command
	write_buffer[2] = 0x01; // sub_command
	wordData = (write_buffer[0] << 24) | (write_buffer[1]<< 16) | (write_buffer[2]<< 8);
	mcsdl_ISC_write_bits( wordData, 24 );
	mcsdl_i2c_stop();
}

static void mms100_ISC_firmware_update_mode_enter()
{
	UINT32 wordData = 0x00000000;
	mcsdl_i2c_start();
	wordData = (ISC_MODE_SLAVE_ADDRESS << 1) << 24 | (0xAE << 16) | (0x55 << 8) | (0x00);
	mcsdl_ISC_write_bits( wordData, 32 );
	wordData = 0x00000000;
	mcsdl_ISC_write_bits( wordData, 32 );
	mcsdl_ISC_write_bits( wordData, 24 );
	mcsdl_i2c_stop();
}

static UINT8 mms100_ISC_firmware_update(UINT8 *_pBinary_reordered, UINT16 _unDownload_size, UINT8 flash_start, UINT8 flash_end)
{
	int i = 0, j = 0, n, m;
	UINT8 fw_status;
	UINT32 wordData = 0x00000000;
	UINT16 nOffset = 0;
	UINT16 cLength = 8;
	UINT16 CRC_check_buf,CRC_send_buf,IN_data;
	UINT16 XOR_bit_1,XOR_bit_2,XOR_bit_3;
	UINT8  write_buffer[64];

	nOffset =  0;
	cLength = 8; //256

	printk("<MELFAS> flash start : %2d, flash end : %2d\n", flash_start, flash_end);    

	while (flash_start + nOffset < flash_end)
	{  
		CRC_check_buf = 0xFFFF;
		mcsdl_i2c_start();
		write_buffer[0] = ISC_MODE_SLAVE_ADDRESS << 1;
		write_buffer[1] = 0XAE; // command
		write_buffer[2] = 0XF1; // sub_command 
		write_buffer[3] = flash_start + nOffset;

		wordData = (write_buffer[0] << 24) | (write_buffer[1]<< 16) | (write_buffer[2]<< 8) | write_buffer[3];
		mcsdl_ISC_write_bits( wordData, 32 );
		mcsdl_delay(MCSDL_DELAY_100MS);
		mcsdl_delay(MCSDL_DELAY_100MS);

#if MELFAS_CRC_CHECK_ENABLE
		for (m = 7; m >= 0; m--)
		{
			IN_data =(write_buffer[3] >>m) & 0x01;
			XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
			XOR_bit_2 = XOR_bit_1^(CRC_check_buf>>11 & 0x01);
			XOR_bit_3 = XOR_bit_1^(CRC_check_buf>>4 & 0x01);
			CRC_send_buf = (XOR_bit_1 <<4) | (CRC_check_buf >> 12 & 0x0F);
			CRC_send_buf = (CRC_send_buf<<7) | (XOR_bit_2 <<6) | (CRC_check_buf >>5 & 0x3F);
			CRC_send_buf = (CRC_send_buf<<4) | (XOR_bit_3 <<3) | (CRC_check_buf>>1 & 0x0007);
			CRC_check_buf = CRC_send_buf;
		}
		//	printk("<MELFAS> CRC_check_buf 0x%02x, 0x%02x\n", (UINT8)(CRC_check_buf >> 8 & 0xFF), (UINT8)(CRC_check_buf & 0xFF));    				
#endif               
		if (nOffset < _unDownload_size/1024 +1)
		{
			for (j = 0; j < 32; j++)
			{
				for (i = 0; i < cLength; i++)
				{
					write_buffer[i*4+3] = _pBinary_reordered[(flash_start+nOffset)*1024+j*32+i*4+0];
					write_buffer[i*4+2] = _pBinary_reordered[(flash_start+nOffset)*1024+j*32+i*4+1];
					write_buffer[i*4+1] = _pBinary_reordered[(flash_start+nOffset)*1024+j*32+i*4+2];
					write_buffer[i*4+0] = _pBinary_reordered[(flash_start+nOffset)*1024+j*32+i*4+3];
					//printk("<MELFAS> write buffer : 0x%02x,0x%02x,0x%02x,0x%02x\n", write_buffer[i*4+0],write_buffer[i*4+1],write_buffer[i*4+2],write_buffer[i*4+3]);
#if MELFAS_CRC_CHECK_ENABLE
					for (n = 0; n < 4; n++)
					{
						for (m = 7; m >= 0; m--)
						{
							IN_data =(write_buffer[i*4+n]>>m) & 0x0001;
							XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
							XOR_bit_2 = XOR_bit_1^(CRC_check_buf>>11 & 0x01);
							XOR_bit_3 = XOR_bit_1^(CRC_check_buf>>4 & 0x01);
							CRC_send_buf = (XOR_bit_1 <<4) | (CRC_check_buf >> 12 & 0x0F);
							CRC_send_buf = (CRC_send_buf<<7) | (XOR_bit_2 <<6) | (CRC_check_buf >>5 & 0x3F);
							CRC_send_buf = (CRC_send_buf<<4) | (XOR_bit_3 <<3) | (CRC_check_buf>>1 & 0x0007);
							CRC_check_buf = CRC_send_buf;
						}
					}
					//printk("<MELFAS> CRC_check_buf 0x%02x, 0x%02x\n", (UINT8)(CRC_check_buf >> 8 & 0xFF), (UINT8)(CRC_check_buf & 0xFF));
#endif                               
				}

				for(i=0;i<cLength;i++)
				{
					wordData = (write_buffer[i*4+0] << 24) | (write_buffer[i*4+1]<< 16) | (write_buffer[i*4+2]<< 8) | write_buffer[i*4+3];
					mcsdl_ISC_write_bits( wordData, 32 );
					mcsdl_delay(MCSDL_DELAY_100US);								
				}
			}
		}

#if MELFAS_CRC_CHECK_ENABLE
		write_buffer[1] =  CRC_check_buf & 0xFF;
		write_buffer[0] = CRC_check_buf >> 8 & 0xFF;

		wordData = (write_buffer[0] << 24) | (write_buffer[1]<< 16);
		mcsdl_ISC_write_bits( wordData, 16 );
		//printk("<MELFAS> CRC_data = 0x%02x 0x%02x\n",write_buffer[0],write_buffer[1]);
		mcsdl_delay(MCSDL_DELAY_100US); 							
#endif
		mcsdl_i2c_stop();

#if MELFAS_CRC_CHECK_ENABLE
		fw_status = mms100_ISC_read_firmware_status();

		if(fw_status == 0x03)
		{
			printk("<MELFAS> Firmware update success!!!\n");
		}
		else
		{
			printk("<MELFAS> Error detected!! firmware status is 0x%02x.\n", fw_status);
			return MCSDL_FIRMWARE_UPDATE_FAILED;
		}
#endif
		nOffset += 1;
		printk("<MELFAS> %d KB Downloaded...\n",nOffset);            
	}

	return MCSDL_RET_SUCCESS;

}
    
static UINT8 mms100_ISC_read_firmware_status()
{
	UINT32 wordData = 0x00000000;
	UINT8 fw_status;
	mcsdl_i2c_start();
	// WRITE 0xAF
	wordData = (ISC_MODE_SLAVE_ADDRESS << 1) << 24 | (0xAF << 16);
	mcsdl_ISC_write_bits( wordData, 16 );
	mcsdl_i2c_stop();
	mcsdl_delay(MCSDL_DELAY_100MS);

	mcsdl_i2c_start();
	// 1byte read
	wordData = (ISC_MODE_SLAVE_ADDRESS << 1 | 0x01) << 24;
	mcsdl_ISC_write_bits( wordData, 8 );
	fw_status = mcsdl_read_byte();
	wordData = (0x01) << 31;
	mcsdl_ISC_write_bits( wordData, 1 ); //Nack
	mcsdl_i2c_stop();
	return fw_status;
}

static void mms100_ISC_slave_download_start()
{
	UINT32 wordData = 0x00000000;
	UINT8  write_buffer[4];

	mcsdl_i2c_start();
	// WRITE 0xAF
	write_buffer[0] = ISC_MODE_SLAVE_ADDRESS << 1;
	write_buffer[1] = ISC_DOWNLOAD_MODE; // command
	write_buffer[2] = ISC_SLAVE_DOWNLOAD_START; 
	mcsdl_ISC_write_bits( wordData, 24 );
	mcsdl_i2c_stop();
}

#if 0
static UINT8 mms100_ISC_slave_crc_ok()
{
	UINT32 wordData = 0x00000000;
	UINT8 CRC_status = 0;
	UINT8  write_buffer[4];

	while(MCSDL_RESETB_IS_HIGH());

	mcsdl_i2c_start();
	// 1byte read
	write_buffer[0] = ISC_MODE_SLAVE_ADDRESS << 1;
	write_buffer[1] = ISC_READ_SLAVE_CRC_OK; // command
	mcsdl_ISC_write_bits( wordData, 16 );
	mcsdl_i2c_stop();

	mcsdl_i2c_start();
	// 1byte read
	wordData = (ISC_MODE_SLAVE_ADDRESS << 1 | 0x01) << 24;
	mcsdl_ISC_write_bits( wordData, 8 );
	CRC_status = mcsdl_read_byte();
	wordData = (0x01) << 31;
	mcsdl_ISC_write_bits( wordData, 1 ); //Nack
	mcsdl_i2c_stop();

	if(CRC_status == 1) return TRUE;
	else if(CRC_status == 2) return FALSE;
	return FALSE;
}
#endif

static void mms100_ISC_leave_firmware_update_mode()
{
	UINT32 wordData = 0x00000000;
	mcsdl_i2c_start();
	wordData = (ISC_MODE_SLAVE_ADDRESS << 1) << 24 | (0xAE << 16) | (0x0F << 8) | (0xF0);
	mcsdl_ISC_write_bits( wordData, 32 );
	mcsdl_i2c_stop();
}

static void mcsdl_i2c_start(void)
{
	MCSDL_GPIO_SDA_SET_OUTPUT(1);mcsdl_delay(MCSDL_DELAY_1US);
	MCSDL_GPIO_SCL_SET_OUTPUT(1);mcsdl_delay(MCSDL_DELAY_1US);

	MCSDL_GPIO_SDA_SET_LOW();mcsdl_delay(MCSDL_DELAY_1US);
	MCSDL_GPIO_SCL_SET_LOW();
}

static void mcsdl_i2c_stop(void)
{
	MCSDL_GPIO_SCL_SET_OUTPUT(0);mcsdl_delay(MCSDL_DELAY_1US);
	MCSDL_GPIO_SDA_SET_OUTPUT(0);mcsdl_delay(MCSDL_DELAY_1US);

	MCSDL_GPIO_SCL_SET_HIGH();mcsdl_delay(MCSDL_DELAY_1US);
	MCSDL_GPIO_SDA_SET_HIGH();
}
    
static void mms100_ISC_set_ready(void)
{
	//--------------------------------------------
	// Tkey module reset
	//--------------------------------------------

	MCSDL_VDD_SET_LOW(); // power 

	//MCSDL_SET_GPIO_I2C();

	MCSDL_GPIO_SDA_SET_OUTPUT(1);
	MCSDL_GPIO_SDA_SET_HIGH();

	MCSDL_GPIO_SCL_SET_OUTPUT(1);
	MCSDL_GPIO_SCL_SET_HIGH();

	MCSDL_RESETB_SET_INPUT();

	//MCSDL_CE_SET_HIGH;
	//MCSDL_CE_SET_OUTPUT();
	mcsdl_delay(MCSDL_DELAY_60MS);						// Delay for Stable VDD

	MCSDL_VDD_SET_HIGH();

	mcsdl_delay(MCSDL_DELAY_60MS);						// Delay for Stable VDD
}


static void mms100_ISC_reboot_mcs(void)
{
	//--------------------------------------------
	// Tkey module reset
	//--------------------------------------------
	mms100_ISC_set_ready();
}

static UINT8 mcsdl_read_ack(void)
{
	int i;
	UINT8 pData = 0x00;
	MCSDL_GPIO_SDA_SET_LOW();
	MCSDL_GPIO_SDA_SET_INPUT();

	MCSDL_GPIO_SCL_SET_HIGH();  mcsdl_delay(MCSDL_DELAY_3US);       
	if ( MCSDL_GPIO_SDA_IS_HIGH()) pData = 0x01;
	MCSDL_GPIO_SCL_SET_LOW();mcsdl_delay(MCSDL_DELAY_3US);
	return pData;
}

static void mcsdl_ISC_read_32bits( UINT8 *pData )
{
	int i, j;
	MCSDL_GPIO_SDA_SET_LOW();
	MCSDL_GPIO_SDA_SET_INPUT();

        for (i = 3; i >= 0; i--)
	{
		pData[i] = 0;

		for (j = 0; j < 8; j++)
		{
			pData[i] <<= 1;

			MCSDL_GPIO_SCL_SET_HIGH();	mcsdl_delay(MCSDL_DELAY_3US);       
			if ( MCSDL_GPIO_SDA_IS_HIGH() )
				pData[i] |= 0x01;
			MCSDL_GPIO_SCL_SET_LOW();	mcsdl_delay(MCSDL_DELAY_3US);
		}
	}
}

static UINT8 mcsdl_read_byte(void)
{
	int i;
	UINT8 pData = 0x00;
	MCSDL_GPIO_SDA_SET_LOW();
	MCSDL_GPIO_SDA_SET_INPUT();

	for (i = 0; i < 8; i++)
	{
		pData <<= 1;
		MCSDL_GPIO_SCL_SET_HIGH();	mcsdl_delay(MCSDL_DELAY_3US);       
		if ( MCSDL_GPIO_SDA_IS_HIGH())	pData |= 0x01;
		MCSDL_GPIO_SCL_SET_LOW();	mcsdl_delay(MCSDL_DELAY_3US);
	}
	return pData;
}

static void mcsdl_ISC_write_bits(UINT32 wordData, int nBits)
{
	int i;

	MCSDL_GPIO_SDA_SET_OUTPUT(0);
	MCSDL_GPIO_SDA_SET_LOW();

	for (i = 0; i < nBits; i++)
	{
		if ( wordData & 0x80000000 )	{ MCSDL_GPIO_SDA_SET_HIGH(); }
		else						{ MCSDL_GPIO_SDA_SET_LOW(); }

		mcsdl_delay(MCSDL_DELAY_3US);

		MCSDL_GPIO_SCL_SET_HIGH();	mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_LOW();	mcsdl_delay(MCSDL_DELAY_3US);

		wordData <<= 1;
		if ((i % 8) == 7)
		{
			mcsdl_read_ack(); //read Ack

			MCSDL_GPIO_SDA_SET_OUTPUT(0);
			MCSDL_GPIO_SDA_SET_LOW();
		}
	}
}


//============================================================
//
//	Debugging print functions.
//
//============================================================

#ifdef MELFAS_ENABLE_DBG_PRINT
static void mcsdl_ISC_print_result(int nRet)
{
	if( nRet == MCSDL_RET_SUCCESS )
	{
		printk("<MELFAS> Firmware downloading SUCCESS.\n");
	}
	else
	{
		printk("<MELFAS> Firmware downloading FAILED  :  ");
		switch( nRet )
		{
			case MCSDL_RET_SUCCESS :							printk("<MELFAS> MCSDL_RET_SUCCESS\n" );						break;
			case MCSDL_FIRMWARE_UPDATE_MODE_ENTER_FAILED :	printk("<MELFAS> MCSDL_FIRMWARE_UPDATE_MODE_ENTER_FAILED\n" );	break;
			case MCSDL_RET_PROGRAM_VERIFY_FAILED :			printk("<MELFAS> MCSDL_RET_PROGRAM_VERIFY_FAILED\n" );			break;

			case MCSDL_RET_PROGRAM_SIZE_IS_WRONG :			printk("<MELFAS> MCSDL_RET_PROGRAM_SIZE_IS_WRONG\n" );			break;
			case MCSDL_RET_VERIFY_SIZE_IS_WRONG :				printk("<MELFAS> MCSDL_RET_VERIFY_SIZE_IS_WRONG\n" );			break;
			case MCSDL_RET_WRONG_BINARY :					printk("<MELFAS> MCSDL_RET_WRONG_BINARY\n" );					break;

			case MCSDL_RET_READING_HEXFILE_FAILED :			printk("<MELFAS> MCSDL_RET_READING_HEXFILE_FAILED\n" );			break;
			case MCSDL_RET_FILE_ACCESS_FAILED :				printk("<MELFAS> MCSDL_RET_FILE_ACCESS_FAILED\n" );				break;
			case MCSDL_RET_MELLOC_FAILED :					printk("<MELFAS> MCSDL_RET_MELLOC_FAILED\n" );					break;

			case MCSDL_RET_WRONG_MODULE_REVISION :			printk("<MELFAS> MCSDL_RET_WRONG_MODULE_REVISION\n" );			break;

			default :											printk("<MELFAS> UNKNOWN ERROR. [0x%02X].\n", nRet );				break;
		}

		printk("\n");
	}

}
#endif


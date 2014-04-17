#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include "bootloader.h"
//#include "SMAC_APP3.h"
//#include "TYE_APP3.h"
//#include "SILABS_S5360_BL006_TYE004_APP005.h"
//#include "SILABS_S5360_SMAC005_BL006_APP020.h"
//#include "SILABS_S5360_SMAC007_BL006_APP020.h"
//#include "SILABS_S5360_TYE010_BL006_APP020.h"
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
#include "SILABS_S5360T_TYE008_BL006_APP014.h"
#include "SILABS_S5360T_SMAC020_BL006_APP015.h"
#endif
unsigned int Running_CRC;
int APP_END_ADDR;
int ERASE_ADDR;

extern int tsp_i2c_write (unsigned char *rbuf, int num);
extern int tsp_i2c_read(unsigned char *rbuf, int len);
extern int TSP_MODULE_ID;
extern int FW_VERSION;
//-----------------------------------------------------------------------------
// Support Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SMB_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Writes a single byte to the slave with address specified by the <TARGET>
// variable.
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Write outgoing data to the <SMB_DATA_OUT> variable array
// 3) Call SMB_Write()
//
//-----------------------------------------------------------------------------
int SMB_Write (unsigned char *rbuf, int size)
{
    int ret;
    
      ret=tsp_i2c_write(rbuf, size);    
    return ret;
}

//-----------------------------------------------------------------------------
// SMB_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Reads a single byte from the slave with address specified by the <TARGET>
// variable.
// Calling sequence:
// 1) Write target slave address to the <TARGET> variable
// 2) Call SMB_Write()
// 3) Read input data from <SMB_DATA_IN> variable array
//
//-----------------------------------------------------------------------------
void SMB_Read (unsigned char *rbuf, int size)
{
  int ret;
    ret=tsp_i2c_read(rbuf, size);
    mdelay(5);
}

int EnterBootMode (int res)
{
   int i;
   uint8_t SMB_DATA_OUT[40];
   uint8_t SMB_DATA_IN[5];   

   for(i=0;i<9;i++) SMB_DATA_OUT[i] = 0;
   SMB_DATA_OUT[0] = 0xAA; // Memory Address
   SMB_DATA_OUT[1] = TGT_CMD_ENTER_BL_MODE;

   if(res)
   {
      SMB_DATA_OUT[2] = TGT_ENTER_BL_MODE_REQ_RSP_CODE;   
      SMB_Write(SMB_DATA_OUT, 3);
   }
   else
   {
      SMB_Write(SMB_DATA_OUT, 2);
   }

   if(res)
   {
      SMB_DATA_OUT[0] = 8; // Memory Address      
      SMB_Write(SMB_DATA_OUT, 1);
      SMB_Read(SMB_DATA_IN, 1);

      // TGT_RSP_BL_MODE (0x02) : Success

      if(SMB_DATA_IN[0] != TGT_RSP_BL_MODE) return 0;
   }

   return 1;
}

int GetInfo (void)
{
   uint8_t i;
   uint8_t FirmwareVersion;
   uint8_t SMB_DATA_OUT[40];
   uint8_t SMB_DATA_IN[5];   


   for(i=0;i<9;i++) SMB_DATA_OUT[i] = 0;
   SMB_DATA_OUT[0] = 0; // Memory Address
   SMB_DATA_OUT[1] = TGT_CMD_GET_INFO;
   SMB_Write(SMB_DATA_OUT, 2);

   mdelay(1);

   SMB_DATA_OUT[0] = 8; // Memory Address      
   SMB_Write(SMB_DATA_OUT, 1);
   SMB_Read(SMB_DATA_IN, 5);

   // SMB_DATA_IN[0] : Response, TGT_RSP_OK (0x00) = Success
   // SMB_DATA_IN[1] : TGT_BL_FW_VERSION
   // SMB_DATA_IN[2] : TGT_APP_FW_VERSION
   // SMB_DATA_IN[3] : TGT_CUSTOM_ID
   // SMB_DATA_IN[4] : MODULE VERSION

	TSP_MODULE_ID = SMB_DATA_IN[4];


   if(SMB_DATA_IN[0] != TGT_RSP_OK) return 0;
   FirmwareVersion = SMB_DATA_IN[1];


   return 1;
}

int EraseFlashPage (unsigned int addr)
{
   int i;
   uint8_t SMB_DATA_OUT[40];
   uint8_t SMB_DATA_IN[5];   

   for(i=0;i<9;i++) SMB_DATA_OUT[i] = 0;
   SMB_DATA_OUT[0] = 0; // Memory Address
   SMB_DATA_OUT[1] = TGT_CMD_ERASE_FLASH_PAGE;
   SMB_DATA_OUT[2] = KEY_CODE0;
   SMB_DATA_OUT[3] = KEY_CODE1;
   SMB_DATA_OUT[4] = addr&0xFF;
   SMB_DATA_OUT[5] = ((addr>>8)&0xFF);
   SMB_Write(SMB_DATA_OUT, 6);

   mdelay(30);

   SMB_DATA_OUT[0] = 8; // Memory Address      
   SMB_Write(SMB_DATA_OUT, 1);
   SMB_Read(SMB_DATA_IN, 1);

   // TGT_RSP_OK (0x00) : Succes
   // TGT_RSP_ADDR_INVALID (0x04) : Address Invalid

   if(SMB_DATA_IN[0] != TGT_RSP_OK) 
    {printk("[TSP] Firmware_EraseFlashPage : %d\n", SMB_DATA_IN[0]);
        return 0;   
   }
   return 1;
}

int WriteBytes (unsigned int addr, unsigned int num)
{
    #if 0 //PSJ
   int i;
   uint8_t SMB_DATA_OUT[40];
   uint8_t SMB_DATA_IN[5];   
   
   mdelay(10);

   for(i=0;i<(num+9);i++) SMB_DATA_OUT[i] = 0;
   SMB_DATA_OUT[0] = 0; // Memory Address
   SMB_DATA_OUT[1] = TGT_CMD_WRITE_FLASH_BYTES;
   SMB_DATA_OUT[2] = KEY_CODE0;
   SMB_DATA_OUT[3] = KEY_CODE1;
   SMB_DATA_OUT[4] = addr&0xFF;
   SMB_DATA_OUT[5] = ((addr>>8)&0xFF);
   SMB_DATA_OUT[6] = num&0xFF;
   SMB_DATA_OUT[7] = ((num>>8)&0xFF);
   SMB_DATA_OUT[8] = 0; // dummy
   if(TSP_MODULE_ID==0x02)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = TYE_APP03_Binary[addr+i]; 
   else if(TSP_MODULE_ID==0x03)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = SMAC_APP03_Binary[addr+i];
   else if(TSP_MODULE_ID==0x04)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = TYE_APP05_Binary[addr+i];
   else if(TSP_MODULE_ID==0x05)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = SMAC_APP20_Binary_REV05[addr+i];  
   else if(TSP_MODULE_ID==0x07)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = SMAC_APP20_Binary[addr+i];
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
   else if(TSP_MODULE_ID==0x08)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = TYE_APP14_Binary[addr+i];
   else if(TSP_MODULE_ID==0x14)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = SMAC_APP15_Binary[addr+i];
#endif
   else if(TSP_MODULE_ID==0x0A)
   for(i=0;i<num;i++) SMB_DATA_OUT[i+9] = TYE_APP20_Binary[addr+i];
   
   SMB_Write(SMB_DATA_OUT,num+9);
   mdelay(10);

   SMB_DATA_OUT[0] = 8; // Memory Address      
   SMB_Write(SMB_DATA_OUT,1);
   SMB_Read(SMB_DATA_IN,1);

   // TGT_RSP_OK (0x00) : Success
   // TGT_RSP_SIG_NOT_ERASED (0x03) : Signature not erased
   // TGT_RSP_ADDR_INVALID (0x04) : Invalid Address
   // TGT_RSP_PARAMETER_INVALID (0x06) : Invalid Parameter

   if(SMB_DATA_IN[0] != TGT_RSP_OK) return 0;   
   #endif

   return 1;
}



void Update_CRC (uint8_t newbyte)
{

   // TODO: Add code here to update CRC
   // Update the global variable "Running_CRC" (defined elsewhere).

   uint8_t i;                               // loop counter

   Running_CRC = Running_CRC ^ newbyte;

   for (i = 0; i < 8; i++)
   {
      if (Running_CRC & 0x01)
      {
         Running_CRC = Running_CRC >> 1;
         Running_CRC ^= POLY;
      }
      else
      {
         Running_CRC = Running_CRC >> 1;
      }
   }
}

int GetPageCRC (unsigned int addr)
{
    #if 0 //PSJ
   unsigned int i;
   unsigned int j;
   unsigned int CRC;
   uint8_t SMB_DATA_OUT[40];
   uint8_t SMB_DATA_IN[5];   
   uint8_t write_data;   

   for(i=0;i<8;i++) SMB_DATA_OUT[i] = 0;
   SMB_DATA_OUT[0] = 0; // Memory Address
   SMB_DATA_OUT[1] = TGT_CMD_GET_PAGE_CRC;
   SMB_DATA_OUT[2] = addr & 0xff;
   SMB_DATA_OUT[3] = ((addr >> 8) & 0xff);

   SMB_Write(SMB_DATA_OUT,4);

   mdelay(10);

   SMB_DATA_OUT[0] = 8; // Memory Address      
   SMB_Write(SMB_DATA_OUT, 1);
   SMB_Read(SMB_DATA_IN, 3);

   // TGT_RSP_OK (0x00) : Success
   // TGT_RSP_SIG_NOT_ERASED (0x03) : Signature not erased
   // TGT_RSP_ADDR_INVALID (0x04) : Invalid Address

   if(SMB_DATA_IN[0] != TGT_RSP_OK) return 0;   
   CRC = SMB_DATA_IN[2] * 256 + SMB_DATA_IN[1];

   Running_CRC = 0; // Init Running CRC to 0
   
   for (i = 0; i < 0x200 ; i++) // Page size = 0x200
   {
		if(TSP_MODULE_ID==0x02)
			write_data = TYE_APP03_Binary[addr+i]; 
		else if(TSP_MODULE_ID==0x03)
			write_data = SMAC_APP03_Binary[addr+i];
		else if(TSP_MODULE_ID==0x04)
			write_data = TYE_APP05_Binary[addr+i];
		else if(TSP_MODULE_ID==0x05)
			write_data = SMAC_APP20_Binary_REV05[addr+i];  
		else if(TSP_MODULE_ID==0x07)
			write_data = SMAC_APP20_Binary[addr+i];
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
		else if(TSP_MODULE_ID==0x08)
			write_data = TYE_APP14_Binary[addr+i];
		else if(TSP_MODULE_ID==0x14)
			write_data = SMAC_APP15_Binary[addr+i];
#endif
        else if(TSP_MODULE_ID==0x0A)
			write_data = TYE_APP20_Binary[addr+i];   
      
		Update_CRC (write_data);
   }
   
   if(CRC != Running_CRC) return 0;    
#endif
   return 1;
}

int ResetMCU (void)
{
   int i;
   uint8_t SMB_DATA_OUT[40];
   uint8_t SMB_DATA_IN[5];   

   for(i=0;i<8;i++) SMB_DATA_OUT[i] = 0;
   SMB_DATA_OUT[0] = 0; // Memory Address
   SMB_DATA_OUT[1] = TGT_CMD_RESET_MCU;

   SMB_Write(SMB_DATA_OUT, 2);

   mdelay(1);

   SMB_DATA_OUT[0] = 8; // Memory Address      
   SMB_Write(SMB_DATA_OUT, 1);
   SMB_Read(SMB_DATA_IN, 1);

   // TGT_RSP_OK (0x00) : Success

   if(SMB_DATA_IN[0] != TGT_RSP_OK) return 0;   
   return 1;
}

int Firmware_Download (void)
{
   int res;
   unsigned int addr;

   //
   // EnterBootMode - no response
   //
       
   EnterBootMode (0);
#if defined(CONFIG_TARGET_LOCALE_AUS_TEL)
   mdelay(30);   
#endif
   //
   // EnterBootMode - response
   //

   res = EnterBootMode (1);
   if(res == 0)
    {
        printk("[TSP] %s, %d\n", __func__, __LINE__ );
        return 0;
   }
   //
   // GetInfo
   //

   res = GetInfo();
   if(res == 0) 
   {
        printk("[TSP] %s, %d\n", __func__, __LINE__ );
        return 0;
   }
   //
   // EraseFlashPage - need to erase signature flash page, 0x7E00
   //

   if((TSP_MODULE_ID == 2) ||(TSP_MODULE_ID == 3) )
    ERASE_ADDR = 0x7C00;
   else
     ERASE_ADDR = 0x7A00;  

   res = EraseFlashPage ( ERASE_ADDR );
  if(res == 0)
  {
     printk("[TSP] %s, %d\n", __func__, __LINE__ );
     return 0;
  }
   //
   // Code Write & Verify
   //
   printk("TSP_MOUDLE_ID : %x\n", TSP_MODULE_ID);

   if((TSP_MODULE_ID == 2) ||(TSP_MODULE_ID == 3) )
    APP_END_ADDR = APP_END_ADDR_OLD;
   else
     APP_END_ADDR = APP_END_ADDR_NEW;  
   
   for(addr = APP_START_ADDR ; addr <= APP_END_ADDR ; addr+=32)
   {
       int numWriteBytes = 32;


       if ((addr + 32) > APP_END_ADDR)
       {
           numWriteBytes = (APP_END_ADDR - addr + 1);
       }
        //printk("[TSP] %s, %d\n", __func__, __LINE__ );
       if ((addr % 0x200) == 0)
       {
           if (EraseFlashPage(addr) == 0)
           {
               printk("[TSP] %s, %d\n", __func__, __LINE__ );
               return 0;
           }
       }
    
       if (WriteBytes(addr, numWriteBytes) == 0)   
       {
           printk("[TSP] %s, %d\n", __func__, __LINE__ );
           return 0;
       }

       if (0)// (((addr + 32) % 0x200) == 0)
       {
           if (GetPageCRC(addr & 0xFE00) == 0)
           {
           printk("[TSP] %s, %d\n", __func__, __LINE__ );
               return 0;
           }
       }
   }

   //
   // Reset MCU
   // 

   res = ResetMCU();
   if(res == 0)
    {
        printk("[TSP] %s, %d\n", __func__, __LINE__ );
        return 0;
   }
   return 1;
}



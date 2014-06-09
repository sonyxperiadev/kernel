/**
 * @file    driver_util.c
 *  
 * @authors Pete Skeggs 
 *
 * @brief   driver utility functions  
 *
 * @copyright (C) 2013-2014 EM Microelectronic
 *
 */

#include "driver_util.h"

/** \addtogroup Utilities
 *  @{
 */



bool i2c_blocking_read(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len)
{
  TransferStatus status = TS_I2C_IDLE;
   u16 rlen;

   if (i2c_read_start(handle, reg_addr, buffer, len))
   {
      while (!i2c_check_status(handle, &status, &rlen))
      {
         if (status != TS_I2C_IN_PROGRESS)
            break;
      }
   }
   if ((status == TS_I2C_COMPLETE) && (rlen == len))
   {
      insane_log("# read 0x%02X %u: ", reg_addr, len);
      while (len--) insane_log("0x%02X ", *(buffer++));
      insane_log("; %u\n", time_ms());
      return TRUE;
   }
   else
      return FALSE;
}


bool i2c_blocking_write(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len)
{
   TransferStatus status = TS_I2C_IDLE;
   u16 wlen;

   if (i2c_write_start(handle, reg_addr, buffer, len))
   {
      while (!i2c_check_status(handle, &status, &wlen))
      {
         if (status != TS_I2C_IN_PROGRESS)
            break;
      }
   }
//   else
//      info_log("i2c_blocking_write failed to start\n");
   if ((status == TS_I2C_COMPLETE) && (wlen == len))
   {
      insane_log("# write 0x%02X ", reg_addr);
      while (len--) insane_log("0x%02X ", *(buffer++));
      insane_log("; %u\n", time_ms());
      return TRUE;
   }
   else
   {
      //if (wlen != len)
      //   info_log("wlen: %u, expected %u\n", wlen, len);
      //else
      //   info_log("status != TS_I2C_COMPLETE\n"); 
      return FALSE;
   }
}


bool i2c_blocking_write_read(I2C_HANDLE_T handle, u8 *wbuffer, u16 wlen, u8 *rbuffer, u16 rlen)
{
   TransferStatus status = TS_I2C_IDLE;
   u16 tlen;

   if (i2c_write_read_start(handle, wbuffer, wlen, rbuffer, rlen))
   {
      while (!i2c_check_status(handle, &status, &tlen))
      {
         if (status != TS_I2C_IN_PROGRESS)
            break;
      }
   }
   if ((status == TS_I2C_COMPLETE) && (rlen == tlen))
      return TRUE;
   else
      return FALSE;

}


int read_line(FILE_HANDLE_T f, char *line, int max_len)
{
   int len = 0;
   if (!f || !line || !max_len)
      return len;
   do
   {
      if (!file_read(f, (u8 *)line, 1, NULL))
      {
         *line = '\0';
         break;
      }
      if ((*line == '\r') || (*line == '\n') || (*line == '\0'))
      {
         if (len == 0)                                               // just found the \n after the \r; keep going
            continue;
         *line = '\0';
         break;
      }
      line++;
      len++;
      max_len--;
   }
   while (max_len);
   return len;
}




float uint16_reg_to_float(u8 *buf)
{
   return (float)((s16)(((u16)buf[0]) + (((u16)buf[1]) << 8)));
}


float uint32_reg_to_float(u8 *buf)
{
   u32 temp =  ((u32)buf[0]) +
                 (((u32)buf[1]) << 8) +
                 (((u32)buf[2]) << 16) +
                 (((u32)buf[3]) << 24);
   return *((float *)&temp);
}



void record_error(DI_INSTANCE_T *instance, DI_ERROR_CODE_T error, int line, const char *fn) 
{
   instance->error.driver_error = error;
   if (error != DE_NONE)
   {
      instance->error.driver_error_aux = (u16)line;
      instance->error.driver_error_func = fn;
      error_log("%d at %s line %d\n", error, instance->error.driver_error_func, instance->error.driver_error_aux);
   }
   else
   {
      instance->error.driver_error_aux =  0;
      instance->error.driver_error_func = NULL;
   }
}


/** @}*/


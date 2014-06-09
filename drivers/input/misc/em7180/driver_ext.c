/**
 * \file    driver_ext.c
 *  
 * \authors Pete Skeggs 
 *
 * \brief  extended interface to the generic host driver, which provides 
 *         additional functionality to make it easier to integrate the generic
 *         host driver core with an application.
 *  
 * \copyright (C) 2013-2014 EM Microelectronic
 *
 */

#include <stdio.h>
#include <string.h> // will need conditionals to offer cross-platform version
#include "config.h"
#include "driver_ext.h"
#include "EEPROMImage.h"
#include "driver_util.h"

/** \addtogroup Driver_Ext
 *  @{
 */

#define QUERY_SENSOR_TIMEOUT_MS 1000   /**< timeout value for querying a sensor */
#define SINGLE_INSTANCE /**< only one */

#if defined(SINGLE_INSTANCE)
static /*VOLATILE*/ DI_INSTANCE_T the_instance;    /**< there is only one Sentral; this is the driver-related data for it */
#else
/// \todo user: provide a mechanism for allocating one or more instances as needed
#endif

#define CONFIG_FILE_BUF_LEN 32          /**< number of bytes to read from config file at a time while uploading to RAM; allocated on the stack; must be 4 */

/** 
 *  \brief in driver ID order, a table of human-readable names for sensor
 *         drivers */
#define NUM_DRIVER_IDS 34
static const char *driver_names[NUM_DRIVER_IDS] =
{
// 0       1             2             3             4             5                  6              7
   "none", "BMX055 old", "RM3100 mag", "AK8963 mag", "AK8975 mag", "LSM330DLC accel", "LSM330 gyro", "LSM330 accel",
//  8            9               10             11     12               13             14              15
   "BMX055 mag", "BMX055 accel", "BMX055 gyro", "n/a", "LSM9SD0 accel", "LSM9SD0 mag", "LSM9SD0 gyro", "MPU6500 accel",
// 16              17               18              19              20            21  22  23  24  25  26  27  28  29  30  31
   "MPU6500 gyro", "MPU9250 accel", "MPU9250 gyro", "MPU9250 mag", "AK09911 mag", 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
// 32              33
   "Stillness", "Pedometer"
};

/** 
 *  \brief order must match DI_SENSOR_TYPE enumeration in driver_interface.h 
 */
#define NUM_SENSOR_NAMES 11
static const char *sensor_names[NUM_SENSOR_NAMES] =
{
   "NONE", "QUATERNION", "MAG", "ACCEL", "GYRO", "BAROM", "HUMID", "TEMP", "CUST0", "CUST1", "CUST2"
};


DI_INSTANCE_T *di_init(I2C_HANDLE_T i2c_handle, IRQ_HANDLE_T irq_handle, bool reset)
{
   DI_INSTANCE_T *instance = &the_instance;

   if (!i2c_handle || !irq_handle)
   {
      INSTANCE_ERROR(DE_INVALID_PARAMETERS);
      return FALSE;
   }

   if (!i2c_init(i2c_handle))
   {
      INSTANCE_ERROR(DE_I2C_ERROR);
      return FALSE;
   }

   return di_init_core(instance, i2c_handle, irq_handle, reset);
}

bool di_deinit(DI_INSTANCE_T *instance)
{
   bool ret;
   I2C_HANDLE_T thandle;

   INSTANCE_ERROR(DE_NONE);
   thandle = instance->i2c_handle;

   ret = di_deinit_core(instance);

   // release resources
   if (thandle)
      i2c_deinit(thandle);

   instance->initialized = FALSE;
   return ret;
}


extern bool di_upload_firmware(DI_INSTANCE_T *instance, char *firmware_filename)
{
   s32 flen;
   u16 rlen;
   u16 wlen;
   FILE_HANDLE_T file;
   EEPROMHeader header;
   u32 wbuf[CONFIG_FILE_BUF_LEN / 4];

   info_log("open firmware file %s\n", firmware_filename);

   // read the file header
   flen = file_size(firmware_filename);

   file = file_open(firmware_filename);
   if (file == NULL)
   {
      INSTANCE_ERROR(DE_FILE_OPEN_ERROR);
      return FALSE;
   }

   info_log("read header...\n");
   if (!file_read(file, (u8 *)&header, sizeof(EEPROMHeader), &rlen))
   {
      file_close(file);
      INSTANCE_ERROR(DE_FILE_READ_ERROR);
      return FALSE;
   }
   if (rlen != sizeof(EEPROMHeader))
   {
      file_close(file);
      INSTANCE_ERROR(DE_FILE_READ_ERROR);
      return FALSE;
   }

   if (!di_upload_firmware_start(instance, &header))
      return FALSE;

   // upload the rest of the file
   flen -= rlen;

   while (flen > 0)
   {
      if (flen > CONFIG_FILE_BUF_LEN / 4)
         wlen = CONFIG_FILE_BUF_LEN / 4;
      else
         wlen = (u16)flen; 
      if (!file_read(file, (u8 *)wbuf, wlen, &rlen))
      {
         INSTANCE_ERROR(DE_FILE_READ_ERROR);
         break;
      }
      if (rlen != wlen)
      {
         INSTANCE_ERROR(DE_FILE_READ_ERROR);
         break;
      }
      if (!di_upload_firmware_data(instance, wbuf, wlen / 4))
         return FALSE;
      flen -= wlen;
   }

   file_close(file);

   return di_upload_firmware_finish(instance);
}


extern bool di_upload_eeprom(DI_INSTANCE_T *instance, I2C_HANDLE_T eeprom_handle, char *firmware_filename)
{
   s32 flen;
   u16 rlen;
   u16 wlen;
   FILE_HANDLE_T file;
   EEPROMHeader header;
   u32 wbuf[CONFIG_FILE_BUF_LEN / 4];

   // prepare to communicate with the EEPROM via the same I2C bus as Sentral
   if (!i2c_init(eeprom_handle))
   {
      INSTANCE_ERROR(DE_I2C_PASSTHRU_ERROR);
      return FALSE;
   }
   if (!i2c_register(eeprom_handle, NULL, 0))
   {
      INSTANCE_ERROR(DE_I2C_PASSTHRU_ERROR);
      return FALSE;
   }

   info_log("open firmware file %s\n", firmware_filename);

   // read the file header
   flen = file_size(firmware_filename);

   file = file_open(firmware_filename);
   if (file == NULL)
   {
      INSTANCE_ERROR(DE_FILE_OPEN_ERROR);
      return FALSE;
   }

   info_log("read header...\n");
   if (!file_read(file, (u8 *)&header, sizeof(EEPROMHeader), &rlen))
   {
      file_close(file);
      INSTANCE_ERROR(DE_FILE_READ_ERROR);
      return FALSE;
   }
   if (rlen != sizeof(EEPROMHeader))
   {
      file_close(file);
      INSTANCE_ERROR(DE_FILE_READ_ERROR);
      return FALSE;
   }

   if (!di_upload_eeprom_start(instance, eeprom_handle, &header))
      return FALSE;

   // upload the rest of the file
   flen -= rlen;

   while (flen > 0)
   {
      if (flen > CONFIG_FILE_BUF_LEN / 4)
         wlen = CONFIG_FILE_BUF_LEN / 4;
      else
         wlen = (u16)flen; 
      if (!file_read(file, (u8 *)wbuf, wlen, &rlen))
      {
         INSTANCE_ERROR(DE_FILE_READ_ERROR);
         break;
      }
      if (rlen != wlen)
      {
         INSTANCE_ERROR(DE_FILE_READ_ERROR);
         break;
      }
      if (!di_upload_eeprom_data(instance, eeprom_handle, wbuf, wlen / 4))
         return FALSE;
      flen -= wlen;
   }

   file_close(file);

   return di_upload_eeprom_finish(instance);
}



bool di_convert_sensor_data(DI_INSTANCE_T *instance, DI_SENSOR_TYPE_T sensor, DI_SENSOR_INT_DATA_T *int_data, DI_SENSOR_DATA_T *data, bool raw)
{
   static const DI_SENSOR_TYPE_T sensor_list[4 + MAX_FEATURE_SENSORS] = {DST_QUATERNION, DST_MAG, DST_ACCEL, DST_GYRO, 
	   DST_NONE, DST_NONE, DST_NONE};
   int num_sensors;
   int i;
   int j;
   float scale;
   DI_SENSOR_TYPE_T cur_sensor;
   DI_3AXIS_INT_DATA_T *i3 = NULL;
   DI_3AXIS_DATA_T *p3 = NULL;

   if (sensor == DST_ALL)
   {
      num_sensors = 4 + instance->num_feature_sensors;
      cur_sensor = sensor_list[0];                                   // not strictly needed, but suppresses a warning
   }
   else                                                              // just read a single sensor
   {
      num_sensors = 1;
      cur_sensor = sensor;
   }

   for (i = 0; i < num_sensors; i++)
   {
      if (sensor == DST_ALL)
      {
         if (i < 4)
            cur_sensor = sensor_list[i];
         else
            cur_sensor = instance->feature_sensors[i - 4];
      }

      switch (cur_sensor)
      {
         case DST_MAG:
            i3 = &int_data->mag;
            p3 = &data->mag;
            if (i3->valid)
            {
               scale = (float)(raw ? 1.0 : SENSOR_SCALE_MAG);
               p3->x = ((float)i3->x) * scale;
               p3->y = ((float)i3->y) * scale;
               p3->z = ((float)i3->z) * scale;
               p3->t = i3->t;
            }
            p3->valid = i3->valid;
            break;
         case DST_ACCEL:
            i3 = &int_data->accel;
            p3 = &data->accel;
            if (i3->valid)
            {
               scale = (float)(raw ? 1.0 : SENSOR_SCALE_ACCEL);
               p3->x = ((float)i3->x) * scale;
               p3->y = ((float)i3->y) * scale;
               p3->z = ((float)i3->z) * scale;
               p3->t = i3->t;
            }
            p3->valid = i3->valid;
            break;
         case DST_GYRO:
            i3 = &int_data->gyro;
            p3 = &data->gyro;
            if (i3->valid)
            {
               scale = (float)(raw ? 1.0 : SENSOR_SCALE_GYRO);
               p3->x = ((float)i3->x) * scale;
               p3->y = ((float)i3->y) * scale;
               p3->z = ((float)i3->z) * scale;
               p3->t = i3->t;
            }
            p3->valid = i3->valid;
            break;
         case DST_QUATERNION:
            if (int_data->quaternion.valid)
            {
               data->quaternion.x = *((float *)&int_data->quaternion.x);
               data->quaternion.y = *((float *)&int_data->quaternion.y);
               data->quaternion.z = *((float *)&int_data->quaternion.z);
               data->quaternion.w = *((float *)&int_data->quaternion.w);
               data->quaternion.t = int_data->quaternion.t;
            }
            data->quaternion.valid = int_data->quaternion.valid;
            break;
         case DST_BAROM:
         case DST_HUMID:
         case DST_TEMP:
         case DST_CUST0:
         case DST_CUST1:
         case DST_CUST2:
            for (j = 0; j < instance->num_feature_sensors; j++)
            {
               if (int_data->feature[j].valid)
               {
                  data->feature[j].data = int_data->feature[j].data;
                  data->feature[j].t = int_data->feature[j].t;
               }
               data->feature[j].valid = int_data->feature[j].valid;
            }
            break;
         default:
            break;
      }
   }

   return TRUE;
}


bool di_query_sensor(DI_INSTANCE_T *instance, DI_SENSOR_TYPE_T sensor, DI_SENSOR_DATA_T *data)
{
   u16 rlen;
   u32 start;
   TransferStatus complete;
   DI_SENSOR_INT_DATA_T int_data;

   // force data read
   if (instance && instance->initialized)
      instance->sensor_info[sensor].timestamp_prev = -1;

   // start the read
   if (!di_query_sensor_start(instance, sensor, FALSE))
      return FALSE;

   // wait until finished
   start = time_ms();
   for (;;)
   {
      irq_check(instance->irq_handle);
      if (!i2c_check_status(instance->i2c_handle, &complete, &rlen))
      {
         INSTANCE_ERROR(DE_I2C_ERROR);
         return FALSE;
      }
      if ((complete == TS_I2C_COMPLETE) && (rlen == instance->sensor_len))
         break;
      else if (complete == TS_I2C_IN_PROGRESS)
      {
         if (time_ms() > (start + QUERY_SENSOR_TIMEOUT_MS))
         {
            INSTANCE_ERROR(DE_SENSOR_SAMPLE_TIMEOUT);
            return FALSE;
         }
         continue;
      }
      else
      {
         INSTANCE_ERROR(DE_I2C_ERROR);
         return FALSE;
      }
   }

   if (!di_query_sensor_finish(instance, sensor, &int_data))
   {
      if (instance->error.driver_error != DE_I2C_PENDING)
         return FALSE;
   }

   // convert to floating point data
   di_convert_sensor_data(instance, sensor, &int_data, data, FALSE);

   INSTANCE_ERROR(DE_NONE);
   return TRUE;
}



const char *di_query_driver_name(u8 driver_id)
{
   if (driver_id >= NUM_DRIVER_IDS)
      return "unknown";
   else if (driver_names[driver_id])
      return driver_names[driver_id];
   else
      return "none";
}


const char *di_query_sensor_name(DI_SENSOR_TYPE_T sensor)
{
   if (sensor >= NUM_SENSOR_NAMES)
      return NULL;
   else
      return sensor_names[sensor];
}


#define NUM_PARAMS 7 /**< number of driver-related parameters */
bool display_driver_info(DI_INSTANCE_T * instance)
{
   SENSOR_DYNAMIC_RANGE *sd;
   DRIVER_ID_REV *di;
   u8 params[NUM_PARAMS] = {SP_SENSOR_DYNAMIC_RANGE_1_2, SP_SENSOR_DYNAMIC_RANGE_3_4, SP_SENSOR_DYNAMIC_RANGE_5_6,
      SP_DRIVER_ID_REV_1_2, SP_DRIVER_ID_REV_3_4, SP_DRIVER_ID_REV_5_6, SP_SPACEPOINT_REV};
   u8 values[NUM_PARAMS * 4];

   info_log("driver version: %u.%02u.%04u, %s %s\n", DRIVER_MAJOR_VERSION, DRIVER_MINOR_VERSION, DRIVER_BUILD, __DATE__, __TIME__);

   if (!di_save_parameters(instance, params, NUM_PARAMS, values))
   {
      error_log("unable to read parameter: %d to display driver info\n");
      return FALSE;
   }
   else
   {
      // params are decoded below, so no need to display raw contents
      // for (i = 0; i < NUM_PARAMS; i++) 
      //    info_log("Param %u: value: 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", params[i], values[i * 4], values[i * 4 + 1], values[i * 4 + 2], values[i * 4 + 3]);
      sd = (SENSOR_DYNAMIC_RANGE *)values;
      info_log("Dynamic Ranges: Mag:%u, Accel:%u, ", sd->range_1, sd->range_2);
      sd++;
      info_log("Gyro:%u, Cust0:%u, ", sd->range_1, sd->range_2);
      sd++;
      info_log("Cust1:%u, Cust2:%u\n", sd->range_1, sd->range_2);

      di = (DRIVER_ID_REV *)&values[12];
      info_log("Drivers and Revs: Mag:%s, %u; Accel:%s, %u; ",
                di_query_driver_name(di->id_driver_1), di->rev_driver_1,
                di_query_driver_name(di->id_driver_2), di->rev_driver_2);
      di++;
      info_log("Gyro:%s, %u; Cust0:%s, %u; ",
                di_query_driver_name(di->id_driver_1), di->rev_driver_1,
                di_query_driver_name(di->id_driver_2), di->rev_driver_2);
      di++;
      info_log("Cust1:%s, %u; Cust2:%s, %u\n",
                di_query_driver_name(di->id_driver_1), di->rev_driver_1,
                di_query_driver_name(di->id_driver_2), di->rev_driver_2);
      info_log("spacepoint rev: %u.%02u\n", values[25], values[24]);
      return TRUE;
   }
}


void display_error_info(DI_INSTANCE_T * instance)
{
   bool executing;
   bool sensing;
   u8 event_reg = instance->error.event_status.reg;
   u8 regs[32];

   if (read_sentral_error_registers(instance))
      info_log("Error: EventStatus:0x%02X SensorStatus:0x%02X SentralStatus:0x%02X AlgoStatus:0x%02X ErrorReg:0x%02X DriverError:%d @%s line %d\n",
             event_reg, instance->error.sensor_status.reg, instance->error.sentral_status.reg, instance->error.algo_status.reg, instance->error.error_register & 0x0ff, 
               instance->error.driver_error, instance->error.driver_error_func, instance->error.driver_error_aux);
   if (di_read_registers(instance, 0x51, 0x5B, regs))
   {
      info_log("          IntState:0x%02X     DebugVal:0x%02X    DebugState:0x%02X AlgoContrl:0x%02X  MagRate:0x%02X   AccelRate:0x%02X GyroRate:0x%02X\n",
                regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6]);
      info_log("         Cust0Rate:0x%02X    Cust1Rate:0x%02X     Cust2Rate:0x%02X       0x5B:0x%02X\n",
                regs[7], regs[8], regs[9], regs[10]);
   }
   if (!di_query_status(instance, &executing, &sensing))
      error_log("\nerror querying status to display error info\n");
   else
      info_log("\nexecuting: %d, sensing: %d\n", executing, sensing);
}


bool display_actual_rates(DI_INSTANCE_T * instance, DI_SENSOR_TYPE_T *sensors_present, int sensors_present_size)
{
   u16 rate;
   int i;

   for (i = 0; i < sensors_present_size; i++)
   {
      if (sensors_present[i] == DST_NONE)
         break;
      info_log("Sensor %d: type %d (%s), ", i, sensors_present[i], di_query_sensor_name(sensors_present[i]));
      if (!di_query_actual_rate(instance, sensors_present[i], &rate))
      {
         error_log("error getting rate to display actual rates\n");
         return FALSE;
      }
      else
      {
         instance->sensor_info[sensors_present[i]].actual_rate = rate;
         info_log("actual rate %u Hz\n", rate);
      }
   }
   return TRUE;
}


bool dump_all_registers(DI_INSTANCE_T * instance)
{
   u8 regs[0xa1];
   int i;

   if (di_read_registers(instance, 0, 0xa0, regs))
   {
      for (i = 0; i <= 0xa0; i++)
      {
         info_log("0x%02X: 0x%02X\n", i, regs[i]);
      }
   }
   else
   {
      error_log("error dumping registers\n");
      return FALSE;
   }
   return TRUE;
}


/** @}*/


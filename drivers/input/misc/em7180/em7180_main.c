/*
 * EM Micro SFP Sensor Hub
 *
 * Copyright  (C) 2013-2014 EM Microelectronic
 */


#define DEBUG 1

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/types.h>
#include <linux/pm.h>
#include <linux/time.h>
#include <linux/firmware.h>

#include "em7180.h"

#include "driver_core.h"
#include "host_services.h"
#include "EEPROMImage.h"

#define emdev_to_di( emdev ) ( emdev->di )
#define di_to_emdev( di )    ( (struct em7180*) (di->user_param) )


#define INFO(fmt, ...)	printk( KERN_INFO "EM7180: " fmt "\n", ## __VA_ARGS__ )

#if DEBUG>0
	#define DMSG(fmt, ...)	printk( KERN_DEBUG "EM7180: " fmt "\n", ## __VA_ARGS__ )
#else
	#define DMSG(fmt, ...)
#endif

#if DEBUG>1
	#define INSANE(fmt, ...) printk( KERN_DEBUG "(INSANE) EM7180: " fmt "\n", ## __VA_ARGS__ )
#else
	#define INSANE(fmt, ...)
#endif

static inline void em7180_lock(struct em7180 * emdev)
{
	mutex_lock(&emdev->mutex);
}

static inline void em7180_unlock(struct em7180 *emdev)
{
	mutex_unlock(&emdev->mutex);
}


static int em7180_read(struct em7180 *emdev, u8 reg)
{
	struct i2c_client *client = to_i2c_client(emdev->dev);
	int ret; 

	ret = i2c_smbus_read_byte_data(client, reg);

	INSANE("READ REG 0x%x: 0x%x (%d)", reg, ret, ret);
 
	return ret;
}

static int em7180_write(struct em7180 *emdev, u8 reg, u8 val)
{
	struct i2c_client *client = to_i2c_client(emdev->dev);

	INSANE("WRITE REG 0x%x=0x%x (%d)", reg, val, val);

	return i2c_smbus_write_byte_data(client, reg, val);
}

static int em7180_read_block( struct em7180 * emdev, u8 reg, int count, void * buf )
{
	struct i2c_client *client = to_i2c_client(emdev->dev);
	int ret = i2c_smbus_read_i2c_block_data(client, reg, count, buf);

	INSANE("READ BLOCK: %x/%d : %d", reg, count, ret);

	return ret;
}

static int em7180_write_block( struct em7180 * emdev, u8 reg, int count, const void * buf )
{
	struct i2c_client *client = to_i2c_client(emdev->dev);
	int ret = i2c_smbus_write_i2c_block_data(client, reg, count, buf);

	INSANE("WRITE BLOCK: %x/%d : %d", reg, count, ret);
	if(ret) {
		printk(KERN_ERR "failed to write block: %d\n", ret);
	}

	return ret;
}

/** driver_core required functions */
u32 time_ms(void)
{
	u32 ms; 

	struct timeval tv;
	do_gettimeofday( &tv );
	
	ms = tv.tv_sec *  MSEC_PER_SEC; 
	ms += tv.tv_usec * USEC_PER_MSEC;

	return ms;
}

bool i2c_blocking_read(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len)
{
	INSANE("DI READ: %p", handle);
	return em7180_read_block( (struct em7180*) handle, reg_addr, len, buffer) == len; 
}


bool i2c_blocking_write(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len)
{
	INSANE("DI WRITE: %p", handle );
	return em7180_write_block( (struct em7180*) handle, reg_addr, len, buffer) == 0; 
}

bool i2c_blocking_write_read(I2C_HANDLE_T handle, u8 *wbuffer, u16 wlen, u8 *rbuffer, u16 rlen)
{
	printk(KERN_ERR "write/read not supported\n");
	return FALSE;
}


bool i2c_read_start(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len)  
{ 
	return em7180_read_block( (struct em7180*) handle, reg_addr, len, buffer) == len; 
}

bool i2c_write_start(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len)  
{ 
	
	return em7180_write_block( (struct em7180*) handle, reg_addr, len, buffer) == 0; 
}


bool i2c_write_read_start(I2C_HANDLE_T handle, u8 *wbuffer, u16 wlen, u8 *rbuffer, u16 rlen)
{
	DMSG("write_read_start not implemented");
	return FALSE;
}


static irqreturn_t em7180_irq_thread(int irq, void *handle)
{
	struct em7180 *emdev = handle;
	int	 	 		ret;
	u8		 		buf[ sizeof( struct quat_sensor_data) ]; 	/* enough for all data types */
	RegEventStatus	status;
	INSANE("irq enter");

	em7180_lock(emdev);

#if 0
	emdev->di->state = DS_QUERY;
	while(emdev->di->state != DS_IDLE) {
		if(!di_task_loop(emdev->di, NULL)) {
			printk(KERN_ERR "task loop error\n");
			break;
		}
	}
#endif

	/* read all the status registers */
	ret = em7180_read_block(emdev, SR_EVENT_STATUS, sizeof( emdev->status ), &emdev->status );

	if(ret != sizeof( emdev->status) ) {
		printk(KERN_ERR "irq error: error reading status registers\n");
		em7180_unlock(emdev);
		return IRQ_HANDLED;
	}
	
	status = emdev->status.event;

	if(status.bits.Error) {
		ret = em7180_read( emdev, SR_ERROR_REGISTER );	

		DMSG("irq error: %x", ret);
	}

#if 1

#define UPDATE_TIMESTAMP() 							\
			t = d.timestamp;						\
			if( t < sensor->info->timestamp_prev )		\
				t+=USHRT_MAX;						\
			dt = t - sensor->info->timestamp_prev; 		\
			sensor->info->timestamp_prev = d.timestamp;	\
			sensor->info->timestamp_base += dt;				

	if(status.bits.AccelResult ) {
		struct em7180_sensor * sensor = &emdev->sensors[DST_ACCEL];

		if(emdev->enabled_sensors & sensor->enable_mask) {
			struct accel_sensor_data d; 
			struct input_dev     * input  = sensor->input; 

			ret = em7180_read_block( emdev, SR_AX, sizeof(d),  buf);

			if(ret==sizeof(d)) {
				u32 t, dt;
			
				d.x 		= le16_to_cpup( (u16*) (buf  ) );
				d.y 		= le16_to_cpup( (u16*) (buf+2) );
				d.z 		= le16_to_cpup( (u16*) (buf+4) );
				d.timestamp = le16_to_cpup( (u16*) (buf+6) );
			
				UPDATE_TIMESTAMP();
			
				input_report_abs( input, ABS_X, d.x);
				input_report_abs( input, ABS_Y, d.y);
				input_report_abs( input, ABS_Z, d.z);

				input_sync( input );

				INSANE("ACCEL: %d,%d,%d@%llu dt:%d", d.x, d.y, d.z, sensor->timestamp, dt);
			}
		}
	}

	if(status.bits.MagResult) {
		struct em7180_sensor * sensor = &emdev->sensors[DST_MAG];

		if(emdev->enabled_sensors & sensor->enable_mask) {
			struct mag_sensor_data d; 
			struct input_dev     * input  = sensor->input; 

			ret = em7180_read_block( emdev, SR_MX, sizeof(d),  buf);

			if(ret==sizeof(d)) {
				u32 t, dt;

				d.x 		= le16_to_cpup( (u16*) (buf  ) );
				d.y 		= le16_to_cpup( (u16*) (buf+2) );
				d.z 		= le16_to_cpup( (u16*) (buf+4) );
				d.timestamp = le16_to_cpup( (u16*) (buf+6) );

				UPDATE_TIMESTAMP();

				input_report_abs( input, ABS_X, d.x);
				input_report_abs( input, ABS_Y, d.y);
				input_report_abs( input, ABS_Z, d.z);

				input_sync( input );

				INSANE("MAG: %d,%d,%d@%lu dt:%d", d.x, d.y, d.z, sensor->info->timestamp_base, dt);
			}
		}
	}

	if(status.bits.GyroResult) {
		struct em7180_sensor * sensor = &emdev->sensors[DST_GYRO];

		if(emdev->enabled_sensors & sensor->enable_mask) {
			struct gyro_sensor_data d; 
			struct input_dev     * input  = sensor->input; 

			ret = em7180_read_block( emdev, SR_GX, sizeof(d),  buf);

			if(ret==sizeof(d)) {
				u32 t, dt;

				d.x 		= le16_to_cpup( (u16*) (buf  ) );
				d.y 		= le16_to_cpup( (u16*) (buf+2) );
				d.z 		= le16_to_cpup( (u16*) (buf+4) );
				d.timestamp = le16_to_cpup( (u16*) (buf+6) );

				UPDATE_TIMESTAMP();

				input_report_abs( input, ABS_X, d.x);
				input_report_abs( input, ABS_Y, d.y);
				input_report_abs( input, ABS_Z, d.z);

				input_sync( input );

				INSANE("GYRO: %d,%d,%d@%lu dt:%d", d.x, d.y, d.z, sensor->info->timestamp_base, dt);
			}
		}
	}

	if(status.bits.QuaternionResult) {
		struct em7180_sensor * sensor = &emdev->sensors[DST_QUATERNION];

		if(emdev->enabled_sensors & sensor->enable_mask) {
			struct quat_sensor_data d; 
			struct input_dev     * input  = sensor->input; 

			ret = em7180_read_block( emdev, SR_QX, sizeof(d),  buf);

			if(ret==sizeof(d)) {
				u32 t, dt;

				d.x 		= le32_to_cpup( (u32*) (buf   ) );
				d.y 		= le32_to_cpup( (u32*) (buf+4 ) );
				d.z 		= le32_to_cpup( (u32*) (buf+8 ) );
				d.w 		= le32_to_cpup( (u32*) (buf+12) );
				d.timestamp = le16_to_cpup( (u16*) (buf+16) );

				UPDATE_TIMESTAMP();

				input_report_abs( input, ABS_X, d.x);
				input_report_abs( input, ABS_Y, d.y);
				input_report_abs( input, ABS_Z, d.z);
				input_report_abs( input, ABS_MISC, d.w);

				input_sync( input );

				INSANE("QUAT: %d,%d,%d,%d@%lu dt:%d", d.x, d.y, d.z, d.w, sensor->info->timestamp_base, dt);
			}
		}
	}

	if(status.bits.FeatureResults) {
		/* Feature 0 sensor */
		{
			struct em7180_sensor * sensor = &emdev->sensors[DST_CUST0];

			if(sensor->info->acquisition_enable) {
				struct input_dev     * input  = sensor->input; 
				struct feature_sensor_data d; 

				ret = em7180_read_block( emdev, SR_FEATURE0, sizeof(d),  buf);

				if(ret==sizeof(d)) {
					u32 t, dt;

					d.x 		= le16_to_cpup( (u16*) (buf   ) );
					d.timestamp = le16_to_cpup( (u16*) (buf+2) );

					UPDATE_TIMESTAMP();

					input_report_abs( input, ABS_X, d.x);

					input_sync( input );

					INSANE("STILL: %d@%llu dt:%d", d.x, sensor->info->timestamp_base, dt);
				}
			}
		}
		/* Feature 1 sensor */
		{
			struct em7180_sensor * sensor = &emdev->sensors[DST_CUST1];

			if(sensor->info->acquisition_enable) {
				struct input_dev     * input  = sensor->input; 
				struct feature_sensor_data d; 
				/* in two feature mode, SR_FEATURE1 is offset */
				ret = em7180_read_block( emdev, SR_FEATURE2, sizeof(d),  buf);

				if(ret==sizeof(d)) {
					u32 t, dt;

					d.x 		= le16_to_cpup( (u16*) (buf   ) );
					d.timestamp = le16_to_cpup( (u16*) (buf+2) );

					UPDATE_TIMESTAMP();

					input_report_abs( input, ABS_X, d.x);

					input_sync( input );

					INSANE("PEDO: %d@%llu dt:%d", d.x, sensor->info->timestamp_base, dt);
				}
			}
		}
	}
#endif
	em7180_unlock(emdev);

	return IRQ_HANDLED;
}



static int _em7180_detect( struct em7180 *emdev )
{
#if 0
	if(!di_init_core(emdev->di, emdev, emdev, TRUE)) {
		printk(KERN_ERR "failed to init core\n");
		return -ENODEV;
	}

	if(!di_detect_sentral2(emdev->di)) {
		return -ENODEV;
	}

	/* query and cache all sensor types installed */
	if(!di_query_features2(emdev->di)) {
		printk(KERN_ERR "failed to query features\n");
		return -ENODEV;
	}
	return 0;
#endif	

#if 1
	int ret;
	u8	buf[4];

	ret = em7180_read_block(emdev, SR_PRODUCT_ID, 2, buf );

	if(ret != 2) {
		printk(KERN_ERR "failed reading from device: %d\n", ret);
		return ret;
	}
	
	emdev->product_id = buf[0];
	emdev->revision_id = buf[1];

	if(emdev->product_id != EM7180_PRODUCT_ID) {
		printk(KERN_ERR "invalid product ID: %x (revision ID:%x)\n", emdev->product_id, emdev->revision_id );
		return -ENODEV;
	}

	ret = em7180_read_block(emdev, SR_ROM_VERSION, 4, buf);

	if(ret != 4)
		return ret;

	emdev->rom_version = le16_to_cpup( (u16*) buf     );
	emdev->ram_version = le16_to_cpup( (u16*) (buf+2) );

	DMSG("Product ID: 0x%x:0x%x ROM: %u RAM: %u", 
		 emdev->product_id, emdev->revision_id,
		 emdev->rom_version, emdev->ram_version );
	return 0;
#endif
}


static int _em7180_accel_init( struct em7180_sensor * sensor )
{
	struct em7180 *emdev = sensor->dev;
	int ret;
	u32 rate = sensor->info->rate;

	DMSG("initializing ACCEL: %u", rate );

//	rate = (rate * sensor->rate_scale_num) / sensor->rate_scale_denom;

	ret = em7180_write(emdev, SR_ACCEL_RATE, rate );
	if( ret )  {
		printk(KERN_ERR "failed to set requested rate: %d\n", ret);
		return ret;
	}

#if 0
	mdelay(100);

	sensor->actual_rate = em7180_read(emdev, SR_ACTUAL_ACCEL_RATE);
#endif

	return 0;
}

static int _em7180_mag_init( struct em7180_sensor * sensor )
{
	struct em7180 *emdev = sensor->dev;
	int ret;
	u32 rate = sensor->info->rate;
	DMSG("initializing MAG: %d", rate);

//	rate = (rate * sensor->rate_scale_num) / sensor->rate_scale_denom;

	ret = em7180_write(emdev, SR_MAG_RATE, rate );
	if( ret )  {
		printk(KERN_ERR "failed to set requested rate: %d\n", ret);
		return ret;
	}

#if 0
	mdelay(100);

	sensor->actual_rate = em7180_read(emdev, SR_ACTUAL_MAG_RATE);
#endif

	return 0;
}

static int _em7180_gyro_init( struct em7180_sensor * sensor )
{
	struct em7180 *emdev = sensor->dev;
	int ret;
	u32 rate = sensor->info->rate;
	DMSG("initializing GYRO");

//	rate = (rate * sensor->rate_scale_num) / sensor->rate_scale_denom;

	ret = em7180_write(emdev, SR_GYRO_RATE, rate );
	if( ret )  {
		printk(KERN_ERR "failed to set requested rate: %d\n", ret);
		return ret;
	}

#if 0
	mdelay(100);

	sensor->actual_rate = em7180_read(emdev, SR_ACTUAL_GYRO_RATE);
#endif

	return 0;
}

static int _em7180_quat_init( struct em7180_sensor * sensor )
{
/*	struct em7180 *emdev = sensor->dev; */
/*	int ret; */
	DMSG("initializing QUAT");

#if 1
	/* make sure there's a non-zero value for rate */
    em7180_write(sensor->dev, SR_QRATE_DIVISOR, 1 );
#endif
	return 0;
}

static int _em7180_baro_init( struct em7180_sensor * sensor )
{
/*	struct em7180 *emdev = sensor->dev; */
/*	int ret; */
	DMSG("initializing BARO");

#if 0
	/* make sure there's a non-zero value for rate */
    em7180_write(sensor->dev, SR_QRATE_DIVISOR, 1 );
#endif
	return 0;
}

static int _em7180_temp_init( struct em7180_sensor * sensor )
{
/*	struct em7180 *emdev = sensor->dev; */
/*	int ret; */
	DMSG("initializing TEMP");

#if 0
	/* make sure there's a non-zero value for rate */
    em7180_write(sensor->dev, SR_QRATE_DIVISOR, 1 );
#endif
	return 0;
}

static int _em7180_humid_init( struct em7180_sensor * sensor )
{
/*	struct em7180 *emdev = sensor->dev; */
/*	int ret; */
	DMSG("initializing HUMID");

#if 0
	/* make sure there's a non-zero value for rate */
    em7180_write(sensor->dev, SR_QRATE_DIVISOR, 1 );
#endif
	return 0;
}

static int _em7180_cust0_init( struct em7180_sensor * sensor )
{
/*	struct em7180 *emdev = sensor->dev; */
/*	int ret; */
	DMSG("initializing CUST0");
#if 1
	/* make sure there's a non-zero value for rate */
    em7180_write(sensor->dev, SR_CUST_SENSOR1_RATE, 1 );
#endif
	return 0;
}



static int _em7180_cust1_init( struct em7180_sensor * sensor )
{
/*	struct em7180 *emdev = sensor->dev; */
/*	int ret; */
	DMSG("initializing CUST1");
#if 1
	/* make sure there's a non-zero value for rate */
    em7180_write(sensor->dev, SR_CUST_SENSOR1_RATE, 1 );
#endif
	return 0;
}

static int _em7180_cust2_init( struct em7180_sensor * sensor )
{
/*	struct em7180 *emdev = sensor->dev; */
/*	int ret; */
	DMSG("initializing CUST2");
#if 0
	/* make sure there's a non-zero value for rate */
    em7180_write(sensor->dev, SR_CUST_SENSOR1_RATE, 1 );
#endif
	return 0;
}


static int _em7180_enable( struct em7180 * emdev, int enable )
{
	int ret;

	DMSG("toggling host control: %d", enable);
#if 1
	ret = em7180_write(emdev, SR_HOST_CONTROL, enable);
	if(ret) {
		printk(KERN_ERR "failed to set state: %d\n", ret);
		return ret;
	}

	ret = em7180_read(emdev, SR_HOST_CONTROL);
	if( ret != enable ) {
		printk(KERN_ERR "failed to start host control: %x\n", ret);
		return -1;
	}
#endif

	return 0;
}


static int em7180_sensor_open( struct em7180_sensor * sensor )
{	
	struct em7180 * emdev = sensor->dev;
	int ret = 0;
	u8 	state; 


	if(!sensor->enable_mask)
		return 0;

	em7180_lock(emdev);

	DMSG("open sensor %s", sensor->path );

#if 0
	if(!sensor->info) {
		printk(KERN_ERR "sensor %s not available\n", sensor->path);
		goto unlock;
	}

	if(!di_enable_sensor_acquisition2( emdev->di, sensor->info->type, true)) {
		printk(KERN_ERR "failed to enable acquisition\n");
		ret = -1;
		goto unlock;
	}
#endif

#if 1
#if 0
	if(sensor->info->acquisition_enable) {
		DMSG("incrementing ref on %s", sensor->path);
		sensor->info->acquisition_enable++;
		ret = 0;
		goto unlock;
	}
#endif

	if(sensor->init) {
		ret = sensor->init(sensor);
		if(ret) {
			printk(KERN_ERR "failed to init sensor:%d\n", ret);
			goto unlock;
		}

/*
		DMSG("%s: requested: %d actual: %d", sensor->path, 
			sensor->info->rate, sensor->info->actual_rate); 
*/
	}

	state = emdev->enabled_sensors;

	/* multiple sensors share the feature sensors bit */
	if( sensor->enable_mask == BIT(6) ) {
		struct em7180_sensor *cust0,*cust1,*cust2;
		cust0 = &emdev->sensors[DST_CUST0];
		cust1 = &emdev->sensors[DST_CUST1];
		cust2 = &emdev->sensors[DST_CUST2];

		/* don't re-enable if the other sensor is also enabled */
		if( ( (sensor == cust0) && ( cust1->info->acquisition_enable || cust2->info->acquisition_enable ) ) || 
		    ( (sensor == cust1) && ( cust0->info->acquisition_enable || cust2->info->acquisition_enable ) ) || 
		    ( (sensor == cust2) && ( cust0->info->acquisition_enable || cust1->info->acquisition_enable ) ) ) {
			DMSG("feature sensors already enabled... skipping events register setup. state: %x", state);
			sensor->info->timestamp_base = 0;
			sensor->info->acquisition_enable = 1;
			ret = 0;
			goto unlock;
		}
	}

	state |= sensor->enable_mask;

	DMSG("setting enable_events: %x", state);

	ret = em7180_write( emdev, SR_ENABLE_EVENTS, state );

	if( ret==0 ) {
		ret = em7180_read( emdev, SR_ENABLE_EVENTS ); 

		if(ret<0) {
			printk(KERN_ERR "failed to validate enable events\n");
			goto unlock;
		}
		if( ret & sensor->enable_mask ) {
			sensor->info->timestamp_base = 0;
			sensor->info->acquisition_enable = 1;

			/* check pre-state. if no sensors were on, we also need to enable host control */
			if(emdev->enabled_sensors == 0) 
				_em7180_enable( emdev, 1 );

			INFO("sensor %s successfully enabled", sensor->path );
			emdev->enabled_sensors = state;
			ret = 0;

		} else {
			printk(KERN_ERR "failed to enable sensor %s\n", sensor->path );
			goto unlock;
		}
	}
	
#endif

unlock:
	em7180_unlock(emdev);
	
	return ret;
}

static void em7180_sensor_close( struct em7180_sensor * sensor )
{
	struct em7180 * emdev = sensor->dev;
	int ret;
	u8 	state; 

	if(!sensor->enable_mask)
		return;

	em7180_lock(emdev);

	DMSG("close sensor %s", sensor->path );

#if 0
	if(!sensor->info)
		goto unlock;

	di_enable_sensor_acquisition2( emdev->di, sensor->info->type, false);
#endif

#if 1
#if 0
	if(sensor->info->acquisition_enable > 1) {
		DMSG("decrementing ref on sensor %s", sensor->path);
		sensor->info->acquisition_enable--;
		goto unlock;
	}
#endif

	state = emdev->enabled_sensors;

	/* multiple sensors share the feature sensors bit */
	if( sensor->enable_mask == BIT(6) ) {
		struct em7180_sensor *cust0,*cust1,*cust2;
		cust0 = &emdev->sensors[DST_CUST0]; 
		cust1 = &emdev->sensors[DST_CUST1]; 
		cust2 = &emdev->sensors[DST_CUST2]; 

		/* only disable if the other sensor is also disabled */
		if( ( (sensor == cust0) && ( cust1->info->acquisition_enable || cust2->info->acquisition_enable ) ) || 
		    ( (sensor == cust1) && ( cust0->info->acquisition_enable || cust2->info->acquisition_enable ) ) || 
		    ( (sensor == cust2) && ( cust0->info->acquisition_enable || cust1->info->acquisition_enable ) ) ) {
			DMSG("other feature sensors enabled... skipping events shutdown. state: %x", state);
			sensor->info->acquisition_enable = 0;
			goto unlock;
		}
	}

	state &= ~sensor->enable_mask;

	DMSG("setting enable_events: %x", state);

	ret = em7180_write( emdev, SR_ENABLE_EVENTS, state );

	if( ret==0 ) {
		ret = em7180_read( emdev, SR_ENABLE_EVENTS ); 

		if(ret<0) {
			printk(KERN_ERR "failed to validate enable events\n");
			goto unlock;
		}
		if( ! (ret & sensor->enable_mask) ) {
			emdev->enabled_sensors = state;
			sensor->info->acquisition_enable = 0;

			INFO("sensor %s successfully disabled", sensor->path );

			/* check post-state. if no sensors on, also disable host control */
			if( state == 0 )
				_em7180_enable(emdev, 0);

		} else {
			printk(KERN_ERR "failed to disable sensor %s\n", sensor->path );
			goto unlock;
		}
	}
#endif

unlock:
	em7180_unlock(emdev);

} 

static int em7180_input_sensor_open( struct input_dev * input )
{
	struct em7180_sensor * sensor = input_get_drvdata(input);
	return em7180_sensor_open( sensor );
}

static void em7180_input_sensor_close( struct input_dev * input )
{
	struct em7180_sensor * sensor = input_get_drvdata(input);
	return em7180_sensor_close( sensor );
}



static int _em7180_init( struct em7180* emdev )
{
	int ret = 0;		
	RegAlgorithmControl ctl = { .bits.ENU = 1,
								.bits.AccelGyro6DOF = 1,
								.bits.EnhancedStillMode = 1 };

//	em7180_lock( emdev );

	ret = em7180_write(emdev, SR_ALGO_CONTROL, ctl.reg );
	
	if(ret) {
		printk( KERN_ERR "failed to set algorithm ctl: %d\n", ret);
		goto unlock;
	}

	ret = em7180_read(emdev, SR_ALGO_CONTROL );

	if(ret != ctl.reg) {
		printk( KERN_ERR "Algo control mismatch: current:%x != set:%x\n", ret, ctl.reg);
		ret = -EINVAL;
		goto unlock;
	}
	
	DMSG("init done");
	return 0;

unlock:
//	em7180_unlock( emdev );

	return ret;

}


static ssize_t em7180_sensor_rate_show( struct em7180_sensor * sensor, char *buf )
{
	u8 val;

	if(!sensor->actual_rate_reg)
		return 0;

	em7180_lock(sensor->dev);

	val = em7180_read( sensor->dev, sensor->actual_rate_reg );

	em7180_unlock(sensor->dev);

	DMSG("sensor %s rate %d", sensor->name, val);

	val = (val * sensor->rate_scale_denom) / sensor->rate_scale_num; 

	DMSG("sensor %s rate (scaled) %d", sensor->name, val);

	return sprintf(buf, "%u\n", val );
}

static ssize_t em7180_sensor_rate_store( struct em7180_sensor * sensor, const char* buf, size_t count )
{
	unsigned long val;
	int error;

	if(!sensor->rate_reg)
		return 0;

	error = strict_strtoul(buf, 10, &val);

	if (error)
		return error;

	DMSG("set sensor %s rate %ld", sensor->name, val);

	val = (val * sensor->rate_scale_num) / sensor->rate_scale_denom;

	DMSG("set sensor %s rate (scaled) %ld", sensor->name, val);

	em7180_lock(sensor->dev);

	sensor->info->rate = val;

	if( em7180_write(sensor->dev, sensor->rate_reg, val ) ) {
		printk(KERN_ERR "failed to set sensor %s rate to %ld\n", sensor->name, val);
	}

	em7180_unlock(sensor->dev);

	return count;
}

#define DECLARE_SYSFS_RATE(n,t)																					\
static ssize_t em7180_##n##_rate_show(struct device *dev, struct device_attribute *attr, char *buf) {			\
	struct em7180 *emdev = dev_get_drvdata(dev);																\
	return em7180_sensor_rate_show( &emdev->sensors[t], buf );													\
}																												\
static ssize_t em7180_##n##_rate_store(struct device *dev, struct device_attribute *attr,						\
				  					  const char *buf, size_t count) {											\
	struct em7180 *emdev = dev_get_drvdata(dev);																\
	return em7180_sensor_rate_store( &emdev->sensors[t], buf, count );											\
}																												\
static DEVICE_ATTR(n##_rate, 0664, em7180_##n##_rate_show, em7180_##n##_rate_store);								

DECLARE_SYSFS_RATE(gyro,DST_GYRO)
DECLARE_SYSFS_RATE(accel,DST_ACCEL)
DECLARE_SYSFS_RATE(mag,DST_MAG)
DECLARE_SYSFS_RATE(quat,DST_QUATERNION)

static struct attribute *em7180_attributes[] = {
	&dev_attr_accel_rate.attr,
	&dev_attr_quat_rate.attr,
	&dev_attr_mag_rate.attr,
	&dev_attr_gyro_rate.attr,
	NULL
};

static const struct attribute_group em7180_sensors_attr_group = {
	.name  = "sensors",
	.attrs = em7180_attributes,
};

#define FIRMWARE_FILENAME	"em7180.fw"


static int em7180_load_firmware( struct em7180 * emdev, const char * filename )
{
	const struct firmware * fw = 0;
	int ret;
	EEPROMHeader *header;
	u8 *data;
	u32 size;

	ret = request_firmware(&fw, filename, emdev->dev);

	if(ret) {
		printk(KERN_ERR "failed to find firmware file %s: %d\n", filename, ret);
		return ret;
	}

	DMSG("found firmware size: %d", fw->size);

	if(fw->size < sizeof(*header)) {
		printk(KERN_ERR "not large enough for firmware\n");
		ret = -ENODEV;
		goto end_upload;
	}

	header = (EEPROMHeader*) fw->data;

	if(header->magic != EEPROM_MAGIC_VALUE) {
		printk(KERN_ERR "invalid firmware magic: %x\n", header->magic);
		ret = -ENODEV;
		goto end_upload;
	}

	if((sizeof(EEPROMHeader) + header->text_length) > fw->size) {
		printk(KERN_ERR "firmware too large\n");
		ret = -ENODEV;
		goto end_upload;
	}

	if(!di_init_core(emdev->di, emdev, emdev, FALSE)) {
		printk(KERN_ERR "failed to init core\n");
		ret = -ENODEV;
		goto end_upload;
	}

	emdev->di->rom_version = emdev->rom_version;

	if(!di_upload_firmware_start(emdev->di, header)) {
		printk(KERN_ERR "failed to start upload\n");
		ret = -ENODEV;
		goto end_upload;
	}

	data = ((u8*)fw->data) + sizeof(*header);
	size = fw->size - sizeof(*header);

	if(!di_upload_firmware_data(emdev->di, (u32*) data, size/4)) {
		printk(KERN_ERR "failed to upload data\n");
		ret = -ENODEV;
		goto end_upload;
	}

	if(!di_upload_firmware_finish(emdev->di)) {
		printk(KERN_ERR "failed to finish upload\n");
		ret = -ENODEV;
		goto end_upload;
	}

	ret = _em7180_detect(emdev);

end_upload:
	release_firmware(fw);


	return ret;
}

static int em7180_probe(struct i2c_client *client,
				      			 const struct i2c_device_id *id)
{
	struct em7180 *emdev;
	int ret = 0;
	int i;

	DMSG("probe");
	dev_err(&client->dev, "%s\n", __func__);

	ret = i2c_check_functionality(client->adapter,
									I2C_FUNC_SMBUS_BYTE_DATA);

	if (!ret) {
		dev_err(&client->dev, "SMBUS Byte Data not Supported\n");
		return -EIO;
	}

	emdev = kzalloc(sizeof(*emdev), GFP_KERNEL);
	if(!emdev) {
		printk(KERN_ERR "failed to allocate em7180 data\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, emdev);

	emdev->di  = kzalloc(sizeof(struct DI_INSTANCE), GFP_KERNEL);
	if(!emdev->di) {
		printk(KERN_ERR "failed to allocate DI_INSTANCE\n");
		ret = -ENOMEM;
		goto err_free_dev;
	}

	emdev->dev = &client->dev;
	emdev->irq = client->irq;
	emdev->fastest_rate = SENSOR_GYRO_DEFAULT_RATE;

	ret = _em7180_detect(emdev);

	if(ret) {
		dev_err( emdev->dev, "attached device does not appear to be nn EM7180\n");
		ret = -ENODEV;
		goto err_free_di;
	}


	mutex_init(&emdev->mutex);

	// allow the load to fail... there may still be eeprom
	em7180_load_firmware(emdev, FIRMWARE_FILENAME);

	INSANE("initializing device");
	ret = _em7180_init(emdev);
	if (ret)
		goto err_free_di;

	//ret = sysfs_create_group(&dev->kobj, &em7180_attr_group);

#if 0
	emdev->sysfs_sensors = kobject_create_and_add( "sensors", &emdev->dev->kobj);
	if(!emdev->sysfs_sensors) {
		printk(KERN_ERR "failed to create sysfs parent node\n");
		goto err_free_di;
	}
#endif

	DMSG("creating sysfs entries");
	ret = sysfs_create_group(&emdev->dev->kobj, &em7180_sensors_attr_group);
	if (ret)
		goto err_free_di;

	for(i=DST_FIRST; i<DST_NUM_SENSOR_TYPES; i++) {
		struct em7180_sensor * sensor = &emdev->sensors[i];

		DMSG("initializing sensor %d", i);
//		if( di_has_sensor( emdev->di, i) ) {
		if( 1 ) {
			struct input_dev * input; 

			sensor->dev 		= emdev;

			input = input_allocate_device();
			if(!input) {
				printk(KERN_ERR "failed to allocate input device\n");
				continue;
			}

			sensor->input 		= input;
			sensor->info 		= &emdev->di->sensor_info[i];
			sensor->info->type	= i;

			input->phys   		= sensor->path;
			input->dev.parent	= emdev->dev;
			input->id.product 	= emdev->rom_version;	
			input->id.vendor	= emdev->product_id;	
			input->id.version 	= emdev->ram_version;
			input->id.bustype 	= BUS_I2C;	
			input->open	 		= em7180_input_sensor_open;
			input->close 		= em7180_input_sensor_close;		
			input_set_drvdata(input,sensor);

			switch( i ) {
			#define setup_sensor(s,n,d,r,en)						\
					sensor->init = _em7180_##s##_init;				\
					sensor->name = n;								\
					sensor->desc = d;								\
					input->name  = d;								\
					sensor->info->rate = r;							\
					sensor->enable_mask = en;
	
			#define setup_abs_1_axis(min,max)										\
					__set_bit(  EV_ABS, input->evbit );								\
					__set_bit(  ABS_X,  input->absbit );							\
					input_set_abs_params(input, ABS_X, min, max, 0, 0);				

			#define setup_abs_2_axis(min,max)										\
					setup_abs_1_axis(min,max)										\
					__set_bit(  ABS_Y,  input->absbit );							\
					input_set_abs_params(input, ABS_Y, min, max, 0, 0);				

			#define setup_abs_3_axis(min,max)										\
					setup_abs_2_axis(min,max)										\
					__set_bit(  ABS_Z,  input->absbit );							\
					input_set_abs_params(input, ABS_Z, min, max, 0, 0);				

		    #define setup_abs_4_axis(min,max)										\
					setup_abs_3_axis(min,max)										\
					__set_bit(  ABS_MISC,  input->absbit );							\
					input_set_abs_params(input, ABS_MISC, min, max, 0, 0);				

			#define set_rate_scale(reg,actual,n,d)												\
					sensor->rate_reg = reg; sensor->actual_rate_reg = actual; sensor->rate_scale_num = n; sensor->rate_scale_denom = d;



				case DST_QUATERNION: 
					setup_sensor(quat, "quat", "EM7180 Quaternion Sensor", SENSOR_QUAT_DEFAULT_RATE, SENSOR_ENABLE_QUAT )		
					setup_abs_4_axis(INT_MIN, INT_MAX);	
					set_rate_scale(0, 0, 1,1)
					break;
				case DST_MAG:
					setup_sensor(mag, "mag", "EM7180 Magnetometer Sensor", SENSOR_MAG_DEFAULT_RATE, SENSOR_ENABLE_MAG )			
					setup_abs_3_axis(SHRT_MIN, SHRT_MAX);
					set_rate_scale(SR_MAG_RATE, SR_ACTUAL_MAG_RATE, 1,1)
					break;
				case DST_ACCEL:
					setup_sensor(accel, "accel", "EM7180 Accelerometer Sensor", SENSOR_ACCEL_DEFAULT_RATE, SENSOR_ENABLE_ACCEL )		
					setup_abs_3_axis(SHRT_MIN, SHRT_MAX);
					set_rate_scale(SR_ACCEL_RATE, SR_ACTUAL_ACCEL_RATE, 1,10)
					break;
				case DST_GYRO:
					setup_sensor(gyro, "gyro", "EM7180 Gyroscope Sensor", SENSOR_GYRO_DEFAULT_RATE, SENSOR_ENABLE_GYRO )			
					setup_abs_3_axis(SHRT_MIN, SHRT_MAX);
					set_rate_scale(SR_GYRO_RATE, SR_ACTUAL_GYRO_RATE, 1,10)
					break;
				case DST_BAROM:
					setup_sensor(baro, "baro", "EM7180 Barometer Sensor", SENSOR_BARO_DEFAULT_RATE, 0 )			
					setup_abs_1_axis(SHRT_MIN, SHRT_MAX)	
					set_rate_scale(0, 0, 1,1)
					break;
				case DST_HUMID:
					setup_sensor(humid, "humid", "EM7180 Humidity Sensor", SENSOR_HUMID_DEFAULT_RATE, 0 )			
					setup_abs_1_axis(SHRT_MIN, SHRT_MAX)	
					set_rate_scale(0, 0, 1,1)
					break;
				case DST_TEMP:
					setup_sensor(temp, "temp", "EM7180 Temperature Sensor", SENSOR_TEMP_DEFAULT_RATE, 0 )			
					setup_abs_1_axis(SHRT_MIN, SHRT_MAX)	
					set_rate_scale(0, 0, 1,1)
					break;
				case DST_CUST0:
					setup_sensor(cust0, "cust0", "EM7180 Feature 0 Sensor", SENSOR_CUST0_DEFAULT_RATE, SENSOR_ENABLE_CUST0 )		
					setup_abs_1_axis(INT_MIN, INT_MAX)	
					set_rate_scale(SR_CUST_SENSOR0_RATE, SR_CUST_SENSOR0_RATE, 1,1)
					break;
				case DST_CUST1:
					setup_sensor(cust1, "cust1", "EM7180 Feature 1 Sensor", SENSOR_CUST1_DEFAULT_RATE, SENSOR_ENABLE_CUST1 )			
					setup_abs_1_axis(INT_MIN, INT_MAX)	
					set_rate_scale(SR_CUST_SENSOR1_RATE, SR_CUST_SENSOR1_RATE, 1,1)
					break;
				case DST_CUST2:
					setup_sensor(cust2, "cust2", "EM7180 Feature 2 Sensor", SENSOR_CUST2_DEFAULT_RATE, SENSOR_ENABLE_CUST2 )			
					setup_abs_1_axis(INT_MIN, INT_MAX)	
					set_rate_scale(SR_CUST_SENSOR2_RATE, SR_CUST_SENSOR2_RATE, 1,1)
					break;
	
				default:
					printk(KERN_ERR "unknown sensor type: %d\n", i);
			}

			snprintf(sensor->path, SENSOR_PATH_MAX, "em7180:%s", sensor->name);

			ret = input_register_device(input);
			if (ret) {
				printk(KERN_ERR "failed to register input for device %s\n", sensor->name);			
				input_free_device( input ); 
				sensor->input = NULL;
			}
		}
	}

	DMSG("requesting irq %d", emdev->irq);
	ret = request_threaded_irq(	emdev->irq, NULL, em7180_irq_thread,
				   				IRQ_TYPE_EDGE_RISING | IRQF_ONESHOT,
				   				"em7180", emdev);
	if (ret) {
		dev_err(emdev->dev, "unable to acquire em7180 irq %d\n", emdev->irq);
		goto err_free_input;
	}

	DMSG("em7180 initialized successfully");

	return 0;

//err_free_irq:
//	printk(KERN_ERR "err_free_irq\n");
//	free_irq(emdev->irq, emdev);

err_free_input:
	for(i=DST_FIRST; i<DST_NUM_SENSOR_TYPES; i++) {
		if( emdev->sensors[i].input )
			input_unregister_device(emdev->sensors[i].input);
	}

	sysfs_remove_group(&emdev->dev->kobj, &em7180_sensors_attr_group);
//	kobject_put(emdev->sysfs_sensors);

err_free_di:
	kfree(emdev->di);

err_free_dev:
	kfree(emdev);

	return ret;
}

static int em7180_remove(struct i2c_client *client)
{
	struct em7180 *emdev = i2c_get_clientdata(client);
	int i;

	free_irq(emdev->irq, emdev);

	DMSG("remove");
	for(i=DST_FIRST; i<DST_NUM_SENSOR_TYPES; i++) {
		if( emdev->sensors[i].input )
			input_unregister_device(emdev->sensors[i].input);
	}

	sysfs_remove_group(&emdev->dev->kobj, &em7180_sensors_attr_group);

//	kobject_put(emdev->sysfs_sensors);
	kfree(emdev);

	return 0;
}

#ifdef CONFIG_PM
static int em7180_suspend(struct device *dev)
{
/*	struct i2c_client *client = to_i2c_client(dev);
	struct em7180 *emdev = i2c_get_clientdata(client); */

	DMSG("suspend");

	return 0;
}

static int em7180_resume(struct device *dev)
{
/*	struct i2c_client *client = to_i2c_client(dev);
	struct em7180 *emdev = i2c_get_clientdata(client); */

	DMSG("resume");

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(em7180_pm, em7180_suspend,
			 em7180_resume);

static const struct i2c_device_id em7180_id[] = {
	{ "em7180", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, em7180_id);

static struct i2c_driver em7180_driver = {
	.driver = {
		.name = "em7180",
		.owner = THIS_MODULE,
		.pm = &em7180_pm,
	},
	.probe    = em7180_probe,
	.remove   = em7180_remove,
	.id_table = em7180_id,
};

static int __init em7180_module_init(void)
{
	return i2c_add_driver(&em7180_driver);
}
module_init(em7180_module_init);

static void __exit em7180_module_exit(void)
{
	i2c_del_driver(&em7180_driver);
}
module_exit(em7180_module_exit);

MODULE_AUTHOR("Joshua Gleaton <joshuag@treadlighttech.com>");
MODULE_DESCRIPTION("EM Micro 7180 SFP");
MODULE_LICENSE("GPL");

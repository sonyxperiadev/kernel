/*
 * Copyright (C) 2010 Information System Products Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Definitions for ami306 compass chip.
 */
#ifndef AMI_SENSOR_DEF_H
#define AMI_SENSOR_DEF_H

/*********************************************************************
 Constant
 *********************************************************************/
#define	AMI_OK				0x00			/**< Normal */
#define	AMI_PARAM_ERR		0x01			/**< Parameter Error  */
#define	AMI_SEQ_ERR			0x02			/**< Squence Error  */
#define	AMI_SYSTEM_ERR		0x10			/**< System Error  */
#define AMI_BLOCK_ERR		0x20			/**< Block Error */
#define	AMI_ERROR			0x99			/**< other Error  */

/*********************************************************************
 Type definition
 *********************************************************************/
typedef signed char	ami_sint8;			/**< signed char */
typedef unsigned char	ami_uint8;			/**< unsigned char */
typedef	signed short	ami_sint16;			/**< signed short */
typedef	unsigned short	ami_uint16;			/**< unsigned short */
typedef	signed long	ami_sint32;			/**< signed long */
typedef	unsigned long	ami_uint32;			/**< unsigned long */
typedef double		ami_double;			/**< double */

/*********************************************************************
 Struct definition
 *********************************************************************/
/** axis sensitivity(gain) calibration parameter information  */
typedef struct {
	ami_sint16 x;			/**< X-axis  */
	ami_sint16 y;			/**< Y-axis  */
	ami_sint16 z;			/**< Z-axis  */
} AMI_VECTOR3D;

/** axis interference information  */
typedef struct {
	ami_sint16 xy;		/**< Y-axis magnetic field for X-axis correction value  */
	ami_sint16 xz;		/**< Z-axis magnetic field for X-axis correction value  */
	ami_sint16 yx;		/**< X-axis magnetic field for Y-axis correction value  */
	ami_sint16 yz;		/**< Z-axis magnetic field for Y-axis correction value  */
	ami_sint16 zx;		/**< X-axis magnetic field for Z-axis correction value  */
	ami_sint16 zy;		/**< Y-axis magnetic field for Z-axis correction value  */
} AMI_INTERFERENCE;

/** sensor calibration Parameter information  */
typedef struct {
	AMI_VECTOR3D		m_gain;			/**< geomagnetic field sensor gain  */
	AMI_VECTOR3D		m_gain_cor;		/**< geomagnetic field sensor gain correction parameter  */
	AMI_VECTOR3D		m_offset;		/**< geomagnetic field sensor offset  */
	AMI_INTERFERENCE	m_interference;		/**< geomagnetic field sensor axis interference parameter */
#ifdef AMI_6AXIS
	AMI_VECTOR3D		a_gain;			/**< acceleration sensor gain  */
	AMI_VECTOR3D		a_offset;		/**< acceleration sensor offset  */
#endif
} AMI_SENSOR_PARAMETOR;

/** G2-Sensor measurement value (voltage ADC value ) */
typedef struct {
	ami_uint16	mx;				/**< geomagnetic field sensor measurement X-axis value (mounted position/direction reference) */
	ami_uint16	my;				/**< geomagnetic field sensor measurement Y-axis value (mounted position/direction reference) */
	ami_uint16	mz;				/**< geomagnetic field sensor measurement Z-axis value (mounted position/direction reference) */
#ifdef AMI_6AXIS
	ami_uint16	ax;				/**< acceleration sensor measurement X-axis value (mounted position/direction reference) */
	ami_uint16	ay;				/**< acceleration sensor measurement Y-axis value (mounted position/direction reference) */
	ami_uint16	az;				/**< acceleration sensor measurement Z-axis value (mounted position/direction reference) */
#endif
	ami_uint16	temperature;	/**< temperature sensor measurement value  */
} AMI_SENSOR_RAWVALUE;

/** Window function Parameter information  */
typedef struct {
	AMI_VECTOR3D m_fine;			/**< current fine value  */
	AMI_VECTOR3D m_fine_output;		/**< change per 1coarse */
	AMI_VECTOR3D m_0Gauss_fine;		/**< fine value at zero gauss */
#ifdef AMI304
	AMI_VECTOR3D m_b0;				/**< current b0 value  */
	AMI_VECTOR3D m_coar;			/**< current coarse value  */
	AMI_VECTOR3D m_coar_output;		/**< change per 1fine */
	AMI_VECTOR3D m_0Gauss_coar;		/**< coarse value at zero gauss */
	AMI_VECTOR3D m_delay;			/**< delay value  */
#endif
} AMI_WIN_PARAMETER;

/** AMI chip information ex) 1)model 2)s/n 3)ver 4)more info in the chip */
typedef struct {
	ami_uint16 info;	/* INFO 0x0d/0x0e reg.  */
	ami_uint16 ver;		/* VER  0xe8/0xe9 reg.  */
	ami_uint16 sn;		/* SN   0xea/0xeb reg.  */
	ami_uint8 wia;		/* WIA  0x0f      reg.  */
} AMI_CHIPINFO;

/** AMI Driver Information  */
typedef struct {
	ami_uint8 remarks[40 + 1];	/* Some Information   */
	ami_uint8 datetime[30 + 1];	/* compiled date&time */
	ami_uint8 ver_major;	/* major version */
	ami_uint8 ver_middle;	/* middle.. */
	ami_uint8 ver_minor;	/* minor .. */
} AMI_DRIVERINFO;

#endif

/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   hal_gps_hh2_glue.h
*
*
****************************************************************************/

void GL_GPS_SPI_Drv_Init(void);
UInt16 GL_GPS_SPI_Drv_u16Read_RxMax(UInt8* buffer, UInt16 count);
UInt16 GL_GPS_SPI_Drv_u16Write_RxMax(UInt8* buffer, UInt16 count);
void GL_GPS_SPI_Drv_CNTIN(Boolean flag);
UInt16 GL_GPS_SPI_Drv_u16ProcessErrors(void);
void GL_GPS_SPI_Drv_IntAck(void);
void GL_GPS_SPI_Drv_PowerUp(UInt16 timer);

typedef void (*GL_GPS_RX_DATA_CALLBACK)(void);


extern GL_GPS_RX_DATA_CALLBACK glcb_pfnGL_GPS_HandleRxIntrEvent;



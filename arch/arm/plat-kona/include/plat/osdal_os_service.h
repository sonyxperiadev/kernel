/*****************************************************************************
*  Copyright 2001 - 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/**
*
*  @file   osdal_os_service.h
*
*  @brief  OS Driver Abstraction Layer API
*          This API is shared across OSs.
*
*
*  @note   This file is to help OS independent CSL driver implementation.
*          It abstracts the following drivers in an OS independent manner
*          - GPIO and GPIO IRQ
*          - DMA
*          - Clock (clock generation)
*          - Syscfg (AHB clock gating. To be obsoleted)
****************************************************************************/

#ifndef _OSDAL_OS_SERVICE_H_
#define _OSDAL_OS_SERVICE_H_

/* OSDAL return codes */
typedef Int32 OSDAL_Status;
#define OSDAL_ERR_OK              0
#define OSDAL_ERR_UNSUPPORTED     1
#define OSDAL_ERR_INVAL           2
#define OSDAL_ERR_IO              3
#define OSDAL_ERR_MEM             4
#define OSDAL_ERR_TIMEOUT         5
#define OSDAL_ERR_OTHER           99


/**
*
*  To initialize OSDAL
*
*  @param		None
*
*  @return	    None
*
*****************************************************************************/
void OSDAL_Init(void);


/* GPIO group */

/* GPIO mode config values */
#define OSDAL_GPIO_DIR_INPUT          0x00
#define OSDAL_GPIO_DIR_OUTPUT         0x01
#define OSDAL_GPIO_INT_RISING         (1 << 1)
#define OSDAL_GPIO_INT_FALLING        (1 << 2)
#define OSDAL_GPIO_INT_BOTH           (1 << 3)

/* GPIO debounce config values */
#define OSDAL_GPIO_DB_DISABLE         0x00
#define OSDAL_GPIO_DB_ENABLE          0x01
#define OSDAL_GPIO_DB_1MS        	 	(0 << 1)
#define OSDAL_GPIO_DB_2MS        	 	(1 << 1)
#define OSDAL_GPIO_DB_4MS        	 	(2 << 1)
#define OSDAL_GPIO_DB_8MS        	 	(3 << 1)
#define OSDAL_GPIO_DB_16MS       	 	(4 << 1)
#define OSDAL_GPIO_DB_32MS       	 	(5 << 1)
#define OSDAL_GPIO_DB_64MS       	 	(6 << 1)
#define OSDAL_GPIO_DB_128MS      	 	(7 << 1)

/* GPIO pull config values */
#define OSDAL_GPIO_PULL_DISABLE        0x00
#define OSDAL_GPIO_PULL_ENABLE         0x01
#define OSDAL_GPIO_PULL_DOWN        	 (0 << 1)
#define OSDAL_GPIO_PULL_UP         	 (1 << 1)


typedef void (*OSDAL_GPIO_CB)(void);

typedef enum {
    OSDAL_GPIO_CFG_MODE,
    OSDAL_GPIO_CFG_PULL,
    OSDAL_GPIO_CFG_DEBOUNCE,
} OSDAL_GPIO_CFG_T;


/**
*
*  To configure an GPIO
*
*  @param		pin (in) GPIO id to configure
*   		    type (in) what to configure
*   		    val (in) value to configure
*
*  @return
*
*****************************************************************************/
OSDAL_Status OSDAL_GPIO_Set_Config(UInt32 pin, OSDAL_GPIO_CFG_T type, UInt32 val);

/**
*
*  To read an GPIO configure
*
*  @param		pin (in) GPIO id to read
*   		    type (in) what to read
*   		    val (out) configuration value
*
*  @return
*
*****************************************************************************/
OSDAL_Status OSDAL_GPIO_Get_Config(UInt32 pin, OSDAL_GPIO_CFG_T type, UInt32 *val);

/**
*
*  To set an output GPIO
*
*  @param		pin (in) output GPIO id
*   		    high (in) set to high or to low
*
*  @return	    None
*
*****************************************************************************/
void OSDAL_GPIO_Set(UInt32 pin, Boolean high);

/**
*
*  To read an input GPIO
*
*  @param		pin (in) input GPIO id
*
*  @return	    TRUE if the GPIO is high
*
*****************************************************************************/
Boolean OSDAL_GPIO_Get(UInt32 pin);


/**
*
*  To enable IRQ for a GPIO
*
*  @param		pin (in) GPIO IRQ to enable
*
*  @return	    None
*
*****************************************************************************/
void OSDAL_GPIO_IRQ_Enable(UInt32 pin);

/**
*
*  To disable IRQ for a GPIO
*
*  @param		pin (in) GPIO IRQ to disable
*
*  @return	    None
*
*****************************************************************************/
void OSDAL_GPIO_IRQ_Disable(UInt32 pin);

/**
*
*  To clear/ack IRQ for a GPIO
*
*  @param		pin (in) GPIO IRQ to clear
*
*  @return	    None
*
*****************************************************************************/
void OSDAL_GPIO_IRQ_Clear(UInt32 pin);

/**
*
*  To check an GPIO IRQ enable status
*
*  @param		pin (in) GPIO IRQ to check
*
*  @return	    TRUE if the IRQ is enabled
*
*****************************************************************************/
Boolean OSDAL_GPIO_IRQ_IsEnabled(UInt32 pin);

/**
*
*  To set callback for an GPIO IRQ
*
*  @param		pin (in) GPIO IRQ to set callback
*   		    cb (in)  callback
*
*  @return
*
*****************************************************************************/
OSDAL_Status OSDAL_GPIO_IRQ_Set_Callback(UInt32 pin, OSDAL_GPIO_CB cb);

/**
*
*  To clear callback for an GPIO IRQ
*
*  @param		pin (in) GPIO IRQ to clear callback
*
*  @return
*
*****************************************************************************/
OSDAL_Status OSDAL_GPIO_IRQ_Clear_Callback(UInt32 pin);


/* DMA group */

typedef enum {
    OSDAL_DMA_CLIENT_BULK_CRYPT_OUT,
    OSDAL_DMA_CLIENT_CAM,
    OSDAL_DMA_CLIENT_I2S_TX,
    OSDAL_DMA_CLIENT_I2S_RX,
    OSDAL_DMA_CLIENT_SIM,
    OSDAL_DMA_CLIENT_CRC,
    OSDAL_DMA_CLIENT_SPI_RX,
    OSDAL_DMA_CLIENT_SPI_TX,
    OSDAL_DMA_CLIENT_UARTA_RX,
    OSDAL_DMA_CLIENT_UARTA_TX,
    OSDAL_DMA_CLIENT_UARTB_RX,
    OSDAL_DMA_CLIENT_UARTB_TX,
    OSDAL_DMA_CLIENT_DES_IN,
    OSDAL_DMA_CLIENT_DES_OUT,
    OSDAL_DMA_CLIENT_USB_RX,
    OSDAL_DMA_CLIENT_USB_TX,
    OSDAL_DMA_CLIENT_HSDPA_SCATTER,
    OSDAL_DMA_CLIENT_HSDPA_GATHER,
    OSDAL_DMA_CLIENT_BULK_CRYPT_IN,
    OSDAL_DMA_CLIENT_LCD,
    OSDAL_DMA_CLIENT_MSPRO,
    OSDAL_DMA_CLIENT_DSI_CM,
    OSDAL_DMA_CLIENT_DSI_VM,
    OSDAL_DMA_CLIENT_TVENC1,
    OSDAL_DMA_CLIENT_TVENC2,
    OSDAL_DMA_CLIENT_AUDIO_IN_FIFO,
    OSDAL_DMA_CLIENT_AUDIO_OUT_FIFO,
    OSDAL_DMA_CLIENT_POLYRING_OUT_FIFO,
    OSDAL_DMA_CLIENT_AUDIO_WB_MIXERTAP,
    OSDAL_DMA_CLIENT_MEMORY,
} OSDAL_DMA_CLIENT;

typedef enum {
    OSDAL_DMA_FCTRL_MEM_TO_MEM                         = 0,
    OSDAL_DMA_FCTRL_MEM_TO_PERI                        = 1,
    OSDAL_DMA_FCTRL_PERI_TO_MEM                        = 2,
    OSDAL_DMA_FCTRL_SRCPERI_TO_DESTPERI                = 3,
    OSDAL_DMA_FCTRL_SRCPERI_TO_DESTPERI_CTRL_DESTPERI  = 4,
    OSDAL_DMA_FCTRL_MEM_TO_PERI_CTRL_PERI              = 5,
    OSDAL_DMA_FCTRL_PERI_TO_MEM_CTRL_PERI              = 6,
    OSDAL_DMA_FCTRL_SRCPERI_TO_DESTPERI_CTRL_SRCPERI   = 7
} OSDAL_DMA_CHAN_TYPE;

typedef enum {
    OSDAL_DMA_ALIGNMENT_8	= 8,
    OSDAL_DMA_ALIGNMENT_16	= 16,
    OSDAL_DMA_ALIGNMENT_32	= 32
} OSDAL_DMA_ALIGN;

typedef enum {
    OSDAL_DMA_BURST_SIZE_1,
#if defined (_HERA_)
    OSDAL_DMA_BURST_SIZE_2,
#endif
    OSDAL_DMA_BURST_SIZE_4,
    OSDAL_DMA_BURST_SIZE_8,
    OSDAL_DMA_BURST_SIZE_16,
    OSDAL_DMA_BURST_SIZE_32,
    OSDAL_DMA_BURST_SIZE_64,
    OSDAL_DMA_BURST_SIZE_128,
} OSDAL_DMA_BSIZE;

typedef enum {
    OSDAL_DMA_DATA_SIZE_8BIT  = 0x00,
    OSDAL_DMA_DATA_SIZE_16BIT = 0x01,
    OSDAL_DMA_DATA_SIZE_32BIT = 0x02
} OSDAL_DMA_DWIDTH;

typedef enum {
    OSDAL_DMA_INC_MODE_NONE = 0,
    OSDAL_DMA_INC_MODE_SRC,
    OSDAL_DMA_INC_MODE_DST,
    OSDAL_DMA_INC_MODE_BOTH,
} OSDAL_DMA_INC_MODE;

typedef enum {
    OSDAL_DMA_CALLBACK_OK = 0,
    OSDAL_DMA_CALLBACK_FAIL
} OSDAL_DMA_CALLBACK_STATUS;

typedef void (*OSDAL_DMA_CALLBACK)(OSDAL_DMA_CALLBACK_STATUS Err);

typedef struct {
    OSDAL_DMA_CHAN_TYPE     type;
    OSDAL_DMA_ALIGN         alignment;
    OSDAL_DMA_BSIZE         srcBstSize;
    OSDAL_DMA_BSIZE         dstBstSize;
    OSDAL_DMA_DWIDTH        srcDataWidth;
    OSDAL_DMA_DWIDTH        dstDataWidth;
    UInt32            priority;
    OSDAL_DMA_INC_MODE      incMode;
    OSDAL_DMA_CALLBACK xferCompleteCb;
    Boolean           freeChan;
    Boolean           bCircular;
    UInt8             srcBstLength;
    UInt8             dstBstLength;
} OSDAL_Dma_Chan_Info;

typedef struct {
    UInt32 srcAddr;
    UInt32 destAddr;
    UInt32 length;
    UInt32 bRepeat;
    UInt32 interrupt;
} OSDAL_Dma_Buffer;

typedef struct {
    OSDAL_Dma_Buffer       buffers[1];
} OSDAL_Dma_Buffer_List;

typedef struct {
    UInt32           numBuffer;
    OSDAL_Dma_Buffer_List *pBufList;
} OSDAL_Dma_Data;

/**
*
*  This function allocates dma channel
*
*  @param		srcID (in) source identification
*  @param       dstID (in) destination identification
*  @param       pChanNum (out) buffer to store channel number
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT srcID, OSDAL_DMA_CLIENT dstID, UInt32 *pChanNum);

/**
*
*  This function release dma channel
*
*  @param		chanNum (in) channel id
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_DMA_Release_Channel(UInt32 chanNum);

/**
*
*  This function configure dma channel
*
*  @param       chanNum       (in) channel number
*  @param       pChanInfo    (in) pointer to dma channe info structure
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_DMA_Config_Channel(UInt32 chanNum, OSDAL_Dma_Chan_Info *pChanInfo);

/**
*
*  This function bind data buffer for the DMA channel
*
*  @param		chanNum    (in) channel to bind data
*  @param       pData     (in) pointer to dma channel data buffer
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_DMA_Bind_Data(UInt32 chanNum, OSDAL_Dma_Data *pData);

/**
*
*  This function start dma channel transfer
*
*  @param		chanNum (in) channel identification
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_DMA_Start_Transfer(UInt32 chanNum);

/**
*
*  This function stop dma channel trnasfer
*
*  @param		chanNum (in) channel identification
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_DMA_Stop_Transfer(UInt32 chanNum);


/* Synchronization group (will move to macro header) */

OSDAL_Status OSDAL_SendEvent(void *event);
OSDAL_Status OSDAL_WaitEvent(void *event);
OSDAL_Status OSDAL_WaitEvent_Timeout(void *event, UInt32 timeout);



/* Clock group */

typedef enum {
    OSDAL_SYSCLK_ENABLE_VIDEO_CODEC = 1,
    OSDAL_SYSCLK_ENABLE_CAMARA_IF,
    OSDAL_SYSCLK_ENABLE_USB,
    OSDAL_SYSCLK_ENABLE_CRYPTO,
    OSDAL_SYSCLK_ENABLE_PKA,
    OSDAL_SYSCLK_ENABLE_UARTA,
    OSDAL_SYSCLK_ENABLE_UARTB,
    OSDAL_SYSCLK_ENABLE_DA,
    OSDAL_SYSCLK_ENABLE_LCD,
    OSDAL_SYSCLK_ENABLE_DPE,
    OSDAL_SYSCLK_ENABLE_DMAC,
    OSDAL_SYSCLK_ENABLE_SDIO1,
    OSDAL_SYSCLK_ENABLE_SDIO2,
    OSDAL_SYSCLK_ENABLE_DES,
    OSDAL_SYSCLK_ENABLE_UARTC,
    OSDAL_SYSCLK_ENABLE_RNG,
    OSDAL_SYSCLK_ENABLE_SDIO3,
    OSDAL_SYSCLK_ENABLE_FSUSBHOST,
    OSDAL_SYSCLK_ENABLE_MPHI,
    OSDAL_SYSCLK_ENABLE_DMAC_MODE,
    OSDAL_SYSCLK_ENABLE_UNSUPPORTED,
} OSDAL_SYSCLK_ENABLE_SELECT;

/**
*
*  This function to enable/gate clock for a peripheral(Obsolete in new chips)
*
*  @param		clk (in) peripheral clock to enable/gate
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_SYSCLK_Enable(OSDAL_SYSCLK_ENABLE_SELECT clk);

/**
*
*  This function to disable/ungate clock for a peripheral(Obsolete in new chips)
*
*  @param		clk (in) peripheral clock to disable/ungate
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_SYSCLK_Disable(OSDAL_SYSCLK_ENABLE_SELECT clk);


typedef enum {
    OSDAL_CLK_CAM = 1,	    /*  Camera */
    OSDAL_CLK_DSP,	 		/*  DSP */
    OSDAL_CLK_I2S,        	        /*  I2S block */
    OSDAL_CLK_I2C1,        	/*  I2C1 block */
    OSDAL_CLK_I2C2,        	/*  I2C2 block */
    OSDAL_CLK_DAM,            	/*  Digital audio module */
    OSDAL_CLK_SDIO1,			/*  SDIO controller 1 */
    OSDAL_CLK_SDIO2,			/*  SDIO controller 2 */
    OSDAL_CLK_SPI,			/*  SPI */
    OSDAL_CLK_UARTA,			/*  UART A */
    OSDAL_CLK_UARTB,			/*  UART B */
    OSDAL_CLK_UARTC,			/*  UART C (athena only) */
    OSDAL_CLK_USBPLL_ENABLE,          /*  Power up 48MHz PLL */
    OSDAL_CLK_USBPLL_OEN,             /*  Turn on 48MHz output gate */
    OSDAL_CLK_MIPIDSI_CMI,            /*  MIPI and CMI clocks */
    OSDAL_CLK_MIPIDSI_AFE,            /*  MIPI DSI and AFE clocks */
    OSDAL_CLK_ISP,                    /*  ISP */
    OSDAL_CLK_SDIO3,			/*  SDIO controller 3 */
    OSDAL_CLK_USB48,                  /*  USB 48MHz clock */
    OSDAL_CLK_TVOUT_PLL,              /*  TV out PLL */
    OSDAL_CLK_TVOUT_PLL_CHANS,        /*  TV out PLL channels */
    OSDAL_CLK_CAMINTF_CAMRX,          /*  CAM Interface receive clock */
    OSDAL_CLK_CAMINTF_CAMSYS,         /*  CAM Interface system clock */
    OSDAL_CLK_AUDIO_RX_ADC_CLOCK,     /*  Audio Rx ADC clock */
    OSDAL_CLK_AUXADC_DAC_REF_CLOCK,   /*  AUX ADC/DAC reference clock */
    OSDAL_CLK_USB_PHY_REF_CLOCK,      /*  USB PHY/TVPLL reference clock */
    OSDAL_CLK_AUDIO_TX_DAC_CLOCK,     /*  Audio Rx ADC clock */
} OSDAL_CLK_SELECT;

typedef enum {
    /* < Camera clock speed configuration */
    OSDAL_CLK_CAM_12MHZ                  = 0,
    OSDAL_CLK_CAM_13MHZ                  = 1,
    OSDAL_CLK_CAM_24MHZ                  = 2,
    OSDAL_CLK_CAM_26MHZ                  = 3,
    OSDAL_CLK_CAM_48MHZ                  = 4,

    /* < Camera IF RX clock speed configuration. */
    OSDAL_CLK_CAMINTF_CAMRX_104MHZ       = 3,
    OSDAL_CLK_CAMINTF_CAMRX_156MHZ       = 2,
    OSDAL_CLK_CAMINTF_CAMRX_208MHZ       = 1,

    /* < DSP clock speed configuration */
    OSDAL_CLK_DSP_52MHZ                  = 0,
    OSDAL_CLK_DSP_78MHZ                  = 1,
    OSDAL_CLK_DSP_104MHZ                 = 2,
    OSDAL_CLK_DSP_156MHZ                 = 3,
    OSDAL_CLK_DSP_208MHZ                 = 6,

    /* < I2C clock */
    OSDAL_CLK_I2C_13MHZ                  = 0,
    OSDAL_CLK_I2C_104MHZ                 = 1,

    /* < I2S clock speed configuration */
    /* < I2S clock for the supported bit speeds */
    OSDAL_CLK_I2S_EXT                   = 0,
    OSDAL_CLK_I2S_INT_12_0M             = 1,
    OSDAL_CLK_I2S_INT_3_0M	            = 2,
    OSDAL_CLK_I2S_INT_2_4M	            = 3,
    OSDAL_CLK_I2S_INT_1_5M	            = 4,
    OSDAL_CLK_I2S_INT_2_048M	        = 5,
    OSDAL_CLK_I2S_INT_1_536M	        = 6,
    OSDAL_CLK_I2S_INT_512K	            = 7,
    OSDAL_CLK_I2S_INT_256K	            = 8,
    OSDAL_CLK_I2S_INT_128K	            = 9,

    /* < SPI clock speed divider configuration */
    OSDAL_CLK_SPI_78MHZ                  = 0,
    OSDAL_CLK_SPI_39MHZ                  = 1,
    OSDAL_CLK_SPI_26MHZ                  = 2,
    OSDAL_CLK_SPI_19MHZ                  = 3,
    OSDAL_CLK_SPI_15MHZ                  = 4,
    OSDAL_CLK_SPI_13MHZ                  = 5,
    OSDAL_CLK_SPI_11MHZ                  = 6,
    OSDAL_CLK_SPI_9MHZ                   = 7,

    /* < CMI clock speed configuration */
    OSDAL_CLK_CMI_12MHZ                  = 0,
    OSDAL_CLK_CMI_13MHZ                  = 1,
    OSDAL_CLK_CMI_24MHZ                  = 2,
    OSDAL_CLK_CMI_26MHZ                  = 3,
    OSDAL_CLK_CMI_48MHZ                  = 4,
    OSDAL_CLK_CMI_52MHZ                  = 5,
    OSDAL_CLK_CMI_78MHZ                  = 6,

    /* < ISP clock speed configuration */
    OSDAL_CLK_ISP_52MHz                  = 0,
    OSDAL_CLK_ISP_78MHz                  = 1,
    OSDAL_CLK_ISP_104MHz                 = 2,
    OSDAL_CLK_ISP_156MHz                 = 3,
    OSDAL_CLK_ISP_208MHz                 = 4,

    /* < MIPI DSI clock speed configuration */
    OSDAL_CLK_MIPI_78MHZ                 = 0,	/* < n=0 for 156/(2*(n+1)) = 78Mhz */
    OSDAL_CLK_MIPI_39MHZ                 = 1,
    OSDAL_CLK_MIPI_26MHZ                 = 2,
    OSDAL_CLK_MIPI_19MHZ                 = 3,
    OSDAL_CLK_MIPI_15MHZ                 = 4,
    OSDAL_CLK_MIPI_13MHZ                 = 5,
    OSDAL_CLK_MIPI_11MHZ                 = 6,
    OSDAL_CLK_MIPI_9MHZ                  = 7,
    OSDAL_CLK_MIPI_156MHZ                = 8,	/* < selects 156Mhz, not divided clock */

    /* < SDIO clock divider configuration */
    OSDAL_CLK_SDIO_104MHZ_DIV_N          = 0,
    OSDAL_CLK_SDIO_48MHZ                 = 1,
    OSDAL_CLK_SDIO_24MHZ                 = 2,

} OSDAL_CLK_SPEED;

/* Per clock configuration parameters */
typedef struct {
    Boolean           enable;	/* <DSP soft reset enable */
} OSDAL_CLK_CFG_DSP;

typedef struct {
    UInt16           div;	/* < divisor when CLK_SDIO_104MHZ_DIV_N */
} OSDAL_CLK_CFG_SDIO;

typedef struct {
    UInt16  multiplier;
    UInt16  divisor;
} OSDAL_CLK_CFG_UART;

typedef struct {
    UInt8 dsiDiv;
    UInt8 cam2Div;
    UInt8 cam1Div;
} OSDAL_CLK_CFG_MIPIDSIAFE;

typedef struct {
    UInt16 nInt;
    UInt8 p1;
    UInt8 p2;
    UInt8 bypMod;
    UInt16 lowBits;
    UInt16 upperBits;
    UInt16 divisor;
} OSDAL_CLK_CFG_TVOUTPLL;

typedef struct {
    Boolean cam2_afe_disable;
    Boolean tvout_dac_disable;
    Boolean dsi2_afe_disable;
    Boolean cam1_disable;
} OSDAL_CLK_CFG_TVOUTPLL_CHANS;


typedef void *OSDAL_CLK_HANDLE;

/**
*
*  This function to open a clock
*
*  @param		clk (in) clock to open
*
*  @return	    handle to be used later
*
*****************************************************************************/
OSDAL_CLK_HANDLE OSDAL_CLK_Open(OSDAL_CLK_SELECT clk);

/**
*
*  This function to close a clock
*
*  @param		handle (in) clock to close
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status  OSDAL_CLK_Close(OSDAL_CLK_HANDLE handle);

/**
*
*  This function to configue a clock generation
*
*  @param		handle (in) clock to configure
*  		        speed (in) clock speed
*  		        cfg_data (in) clock specific config parameters
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_CLK_Set_Config(OSDAL_CLK_HANDLE handle, OSDAL_CLK_SPEED speed, void *cfg_data);

/**
*
*  This function to start a clock generation
*
*  @param		handle (in) clock to start
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_CLK_Start(OSDAL_CLK_HANDLE handle);

/**
*
*  This function to stop a clock generation
*
*  @param		handle (in) clock to stop
*
*  @return	    OSDAL return status
*
*****************************************************************************/
OSDAL_Status OSDAL_CLK_Stop(OSDAL_CLK_HANDLE handle);

#endif /* _OSDAL_OS_SERVICE_H_ */


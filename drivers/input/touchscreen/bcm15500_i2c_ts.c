/****************************************************************************
*
*	Copyright (c) 1999-2008 Broadcom Corporation
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License, version 2, as
*  published by the Free Software Foundation (the "GPL").
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php
*  or by writing to the Free Software Foundation, Inc., 59 Temple Place - Suite
*  330, Boston, MA  02111-1307, USA.
*
****************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/freezer.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/kfifo.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/firmware.h>
#include <linux/input/mt.h>

#include <linux/time.h>

#include <linux/i2c/bcm15500_i2c_ts.h>
#include <linux/i2c/bcm15500_tofe.h>

/* ----------- */
/* - Defines - */
/* ----------- */

#define BRING_UP_WORKAROUND                             1
#define BOOT_FROM_RAM                                   1
#define TOFE_CHAN_PROCESSING                            1
#define NAPA_GPIO_PROCESSING                            1
#define TOUCHCON_MTC_FULL_FIRMWARE                      1
#define ADD_INTERRUPT_HANDLING                          1
#define THROTTLE_MOVE_FRAMES                            0
#define MOVE_THROTTLE_FACTOR                            4
#define NAPA_GPIO_RESET_SUPPORT                         1

#define	NAPA_INT_WHILE									0
#define NAPA_SINGLE_READ                                1
#define NAPA_DELAY                                      0
#define NAPA_MIMIC_MT                                   1

#define NAPA_USE_TOUCH                                  1
#define NAPA_USE_MTC                                    0
#define DEBUG_INTERFACE_TUNING                          0

#define NAPA_MERGE_WRITEBACK_REGS                       0
#define NAPA_DONT_USE_12C_TRANSFER                      0

#define MOD_DEBUG_I2C_DOWNLOAD                     0x0001
#define MOD_DEBUG_TIMES                            0x0002
#define MOD_DEBUG_DOWN                             0x0040
#define MOD_DEBUG_MOVE                             0x0080
#define MOD_DEBUG_UP                               0x0100
#define MOD_DEBUG_FRAME                            0x0200
#define MOD_DEBUG_EVENT                            0x0400
#define MOD_DEBUG_THROTTLE                         0x0800
#define MOD_DEBUG_CHANNEL                          0x1000
#define MOD_DEBUG_NAPA_RESET                       0x2000
#define MOD_DEBUG_IGNORE_ERR                       0x4000
#define MOD_DEBUG_LOOP                             0x8000

#define DEBUG(x) x

/* ---- Public Variables ------------------------------------------------- */

static int mod_debug  = 0;

module_param(mod_debug, int, 0644);

#if DEBUG_INTERFACE_TUNING
static unsigned long d_i_t_fc = 0;
static unsigned long d_i_t_i_i = 0;
#endif

struct napa_i2c
{
   struct mutex       i2c_mutex;
   struct i2c_client *p_i2c_client;
   struct i2c_client *p_i2c_client1;
   int pagesize;

   int gpio_int;
   struct workqueue_struct *p_ktouch_wq;
   struct work_struct       work;
   struct mutex             mutex_wq;

   struct timer_list        poll_timer;

   struct bcm915500_platform_data platform_data;
};

static struct napa_i2c *gp_napa_i2c = NULL;
/*
struct i2c_board_info
{
   char            type[I2C_NAME_SIZE];
   unsigned short  flags;
   unsigned short  addr;
   void            *platform_data;
   struct dev_archdata     *archdata;
   struct device_node *of_node;
   int             irq;
};
*/

static struct i2c_board_info bcm915500_i2c_boardinfo =
{
   .type = BCM915500_TSC_NAME,
   .addr = HW_BCM915500_SLAVE_AHB,
   .platform_data = NULL,
};

/* -------------------- */
/* - Local Variables. - */
/* -------------------- */

#if TOFE_CHAN_PROCESSING
tofe_tool_t          tofe_tool;
ctofe_channel_t      touch_channel;
#endif

#if NAPA_GPIO_PROCESSING
int host_int_gpio_pin_addr;
int host_int_asserted;
#endif

static int napa_touch_status[NAPA_MAX_TOUCH];
static int napa_touch_x[NAPA_MAX_TOUCH];
static int napa_touch_y[NAPA_MAX_TOUCH];
static int napa_touch_event[NAPA_MAX_TOUCH];

#if THROTTLE_MOVE_FRAMES
static int g_num_move_frames = 0;
#endif

static struct input_dev *pInputDev = NULL;

static unsigned char I2Cheader[8] = {I2C_REG_DMA_ADDR, /*addr*/ 0x00, 0x00, 0x00, 0x00, /*len*/ 0x00, 0x00, MahbWrite};

static unsigned char *gp_code;
static unsigned char *gp_vector;
static unsigned char *gp_data;
int g_dowload_fw_count = 0;
int g_code_size;
int g_vector_size;
int g_data_size;

static bcmtch_data_t  bcmtch_data;
static bcmtch_data_t *p_bcmtch_data = NULL;

static struct i2c_client *gp_i2c_client_spm = NULL;
static struct i2c_client *gp_i2c_client_ahb = NULL;

/* ---------------------------------- */
/* - bcmtch dev function prototypes - */
/* ---------------------------------- */

int bcmtch_init(struct i2c_client *p_i2c_client_spm, struct i2c_client *p_i2c_client_ahb);
int bcmtch_download(void);
int bcmtch_wait_for_fw_ready(void);
void bcmtch_init_ram_contents(void);
tofe_channel_instance_cfg_t *bcmtch_get_channel_config(tofe_channel_id_t channel_id);
int bcmtch_touch_chan_init(void);
int bcmtch_touch_chan_free(void);
int bcmtch_chan_init(tofe_channel_instance_cfg_t *channel_cfg, napa_channel_t **channel);

void bcmtch_init_int_handler(void);
int bcmtch_check_wakeup(int count);
void bcmtch_request_wakeup(int cause);
void bcmtch_release_wakeup(int cause);
int bcmtch_request_sleep(void);
unsigned char bcmtch_int_asserted(void);
void bcmtch_int_deassert(void);
int bcmtch_interrupt(void);

void bcmtch_channel_read_data(napa_channel_t *channel);
uint32_t bcmtch_channel_read_header(napa_channel_t *channel);
uint32_t bcmtch_channel_read(napa_channel_t *channel);
void bcmtch_channel_write_header_pointer(napa_channel_t *channel, napa_channel_write_header_e which, uint32_t pointer_data);
unsigned bcmtch_channel_num_queued(tofe_channel_header_t *channel);
void bcmtch_channel_write_header_pointer_flags(napa_channel_t *channel, napa_channel_write_header_e which, uint32_t pointer_data, uint8_t flags);
void bcmtch_process_touch_events(napa_channel_t *channel);

void bcmtch_event_touch_down(int tag, unsigned short x, unsigned short y);
void bcmtch_event_touch_up(int tag, unsigned short x, unsigned short y);
void bcmtch_event_touch_move(int tag, unsigned short x, unsigned short y);

int  bcmtch_throttle_move_frame(void);
int bcmtch_get_soc_version(unsigned char *chipId, int chipIdBuffLen);
unsigned char bcmtch_get_power_mode(void);

/* ---------------------------------- */
/* - bcmtch com function prototypes - */
/* ---------------------------------- */

int bcmtch_com_set_interface(bcmtch_com_e com_interface);
int bcmtch_com_read(int slave_addr, int len, unsigned char *buffer);
int bcmtch_com_write(int slave_addr, int len, unsigned char *buffer);
unsigned char bcmtch_com_read_reg(int slave_addr, int reg);
int bcmtch_com_write_reg(int slave_addr, int reg, unsigned char data);
int bcmtch_com_write_mem( int mem_addr, int len, void *data);
int bcmtch_com_read_mem(int mem_addr, int len, void *data);
int bcmtch_com_write_mem_reg32(int mem_addr, int data);
int bcmtch_com_read_mem_reg32( int mem_addr, void *data);

/* --------------------------------- */
/* - bcmtch os function prototypes - */
/* --------------------------------- */

int bcmtch_mutex_lock(int mutex);
int bcmtch_mutex_release(int mutex);
void bcmtch_sleep_ms(int ms);
void bcmtch_poll(struct napa_i2c *p_napa_i2c);
int bcmtch_setup_poll_timer(struct napa_i2c *p_napa_i2c, int delay);

int bcmtch_i2c_read(int slave_addr, int len, unsigned char *buffer);
int bcmtch_i2c_write(int slave_addr, int len, unsigned char *buffer);

unsigned char bcmtch_i2c_read_reg(int slave_addr, int reg);
int bcmtch_i2c_write_reg(int slave_addr, int reg, unsigned char data);

int bcmtch_i2c_write_mem( int ahb_addr, int len, void *data);
int bcmtch_i2c_read_mem(int ahb_addr, int len, void *data);

int bcmtch_i2c_write_mem_reg32(int ahb_addr, int data);
int bcmtch_i2c_read_mem_reg32( int ahb_addr, void *data);

static ssize_t bcmtch_cli_fcn(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static DEVICE_ATTR(bcmtch_cli, S_IWUGO, NULL, bcmtch_cli_fcn);

static struct attribute *bcmtch_attributes[] = {
    &dev_attr_bcmtch_cli.attr,
    NULL,
};

static struct attribute_group bcmtch_attr_group = {

    .attrs = bcmtch_attributes
};

/* ------------------------ */
/* - bcmtch dev functions - */
/* ------------------------ */

int bcmtch_init(struct i2c_client *p_i2c_client_spm, struct i2c_client *p_i2c_client_ahb)
{
   unsigned char chipId[4];
   unsigned char regVal;
   unsigned char alfo_ctrl;
   int           ret = 0;

   unsigned int  reg;
   unsigned int  tch_version = 0xdeadbeef;
   unsigned int  remap;
   unsigned int  firmware_ok;

   gp_i2c_client_spm = p_i2c_client_spm;
   gp_i2c_client_ahb = p_i2c_client_ahb;

   /* 0. Set proper communication interface - SPI or I2C mode */
   bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_SPI_I2C_MODE, TCC_HOST_IF_I2C_MODE);
   bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?
   bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, SPI_HOST_I2CS_CHIPID, HW_BCM915500_SLAVE_AHB);
   bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() line %d ret: %d\n", __func__, __LINE__, ret);)

   /** 1. Read ChipId & VersionID --> Verify */
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() line %d 1. Read ChipId & VersionID --> Verify\n", __func__, __LINE__);)
   bcmtch_get_soc_version(chipId, 4);

   /** 2. Read Power Mode Status Register */
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() line %d 2. Read Power Mode Status Register\n", __func__, __LINE__);)
   regVal = bcmtch_get_power_mode();

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() line %d regVal = %d\n", __func__, __LINE__, regVal);)

   if(regVal != SPM_POWER_STATE_SLEEP)
   {
      bcmtch_request_sleep();
   }

   /** 3. Write WAKEUP */
   bcmtch_request_wakeup(TOFE_COMMAND_NO_COMMAND); // Napa request wakeup
   if (!bcmtch_check_wakeup(2))                    // Napa check wakeup
   {
      printk(KERN_ERR "ERROR: Napa chip wake up failed.");
      return NAPA_INIT_ERROR;
   }
   else
   {
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("Napa chip wake up succeeded/n");)
   }

   /* OTP is not programmed, so override with SW setting */
   alfo_ctrl = 0x16;
   if (alfo_ctrl)
   {
      bcmtch_com_write_mem_reg32(COMMON_FLL_CTRL0, 0xe0000002);
   }

   bcmtch_com_write_mem_reg32(COMMON_FLL_LPF_CTRL2, 0x01001007);

#if BRING_UP_WORKAROUND
   if (alfo_ctrl)
   {
      /* If SW ALFO trimming, enable FLL from SW else SPM will power it up */
      bcmtch_com_write_mem_reg32(COMMON_FLL_CTRL0, 0x00000001);
   }
#endif

   /* Set the clock dividers and clock values (as per SOC team's request) */
   bcmtch_com_write_mem_reg32(COMMON_SYS_HCLK_CTRL, 0xF01);

   /* Turn off clocks that are not needed, and enable needed clocks */
   /* For FBGA parts */
   bcmtch_com_write_mem_reg32(COMMON_CLOCK_ENABLE, 0x4387F);

#if BRING_UP_WORKAROUND
   if (alfo_ctrl)
   {
      // SW over-ride for ALFO.
      bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_ALFO_CTRL, alfo_ctrl);

      bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

   }else
   {
      // FLL powered up by SPM and ALFO trimming bits used from OTP
   }
#endif

   // Napa wait for flllock

   // Wait for FLL to lock before doing further operation.
   bcmtch_com_read_mem_reg32(COMMON_CLOCK_ENABLE, &reg);

   reg &= (1 << 18);
   if (reg == 0)
   {
      do
      {
         bcmtch_com_read_mem_reg32(COMMON_FLL_TEST_CTRL1, &reg);
         reg &= (1 << 28);
      } while (reg == 0);

      bcmtch_com_read_mem_reg32(COMMON_CLOCK_ENABLE, &reg);
      bcmtch_com_write_mem_reg32(COMMON_CLOCK_ENABLE, reg | (0x01 << 18));
   }

   bcmtch_com_read_mem_reg32(TCH_VERSION_REGISTER, &tch_version);
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("INFO: Napa Touch block version number  %08X", tch_version);)

   /* Un-remap */
   bcmtch_com_read_mem_reg32(COMMON_ARM_REMAP_ADDR, &remap);
   if (remap < 0)
   {
      printk(KERN_ERR "ERROR: Napa common blob query failure.\n");
      return(NAPA_INIT_ERROR);
   }
   remap &= ~2U; // Turn off ARM RAM1 remap bit.
   if (bcmtch_com_write_mem_reg32(COMMON_ARM_REMAP_ADDR, remap) < 0)
   {
      printk(KERN_ERR "ERROR: Napa common blob update failure.\n");
      return(NAPA_INIT_ERROR);
   }

   /* load firmware */
   firmware_ok = 0;
   if (bcmtch_download() == NAPA_INIT_SUCCESS)
   {
      printk(KERN_INFO "Napa downloaded and booted.\n");
      firmware_ok = 1;
   }
   else
   {
      printk(KERN_ERR "ERROR: Napa downloaded and boot failed.\n");
   }

   /** 4. Release WAKE up request so chip can go into power save mode */
   bcmtch_release_wakeup(TOFE_COMMAND_NO_COMMAND);

   printk("%s() Napa initialization succeeded%d\n", __func__, __LINE__);

   return NAPA_INIT_SUCCESS;
}

int bcmtch_download(void)
{
   unsigned int remap = 0;
   int ret;

   /* Hold ARM in RESET before we reload firmware */
   ret = bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_SOFT_RESETS, 0x02);

   bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() line %d ret: %d\n", __func__, __LINE__, ret);)

#if BOOT_FROM_RAM
   if (bcmtch_com_read_mem_reg32(COMMON_ARM_REMAP_ADDR, &remap) < 0)
   {
      printk(KERN_ERR "ERROR: Common blob query failure.\n");
      return NAPA_INIT_ERROR;
   }
   remap |= 1; // Turn on boot from RAM remap bit.
   if (bcmtch_com_write_mem_reg32(COMMON_ARM_REMAP_ADDR, remap) < 0)
   {
      printk(KERN_ERR "ERROR: Common blob update failure.\n");
      return NAPA_INIT_ERROR;
   }

   /* Load vectors image */
   ret = bcmtch_com_write_mem(VECTORS_OFFSET, g_vector_size, gp_vector);
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() bcmtch_com_write_mem() wrote vector, ret = %d\n", __func__, ret);)

   /* Load ROM code image */
   /* ROM_OFFSET is 0 for FPGA ROM load and ARM RAM0 (0x10000000) for RAM load */
   ret = bcmtch_com_write_mem(ROM_OFFSET, g_code_size, gp_code);
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() bcmtch_com_write_mem() wrote firwmare, ret = %d\n", __func__, ret);)
#endif

   /* load ram image from binary file */
   if(1)
   {

#if TOFE_CHAN_PROCESSING
      /* Tofe Touch Channel Setup. */
      bcmtch_touch_chan_init();
#endif

#if NAPA_GPIO_PROCESSING
      bcmtch_init_int_handler();
#endif

      ret = bcmtch_com_write_mem(DATA_OFFSET, g_data_size, gp_data);
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() bcmtch_com_write_mem() wrote data, ret = %d\n", __func__, ret);)

      /* Write ARM_SW_RESET bit to 0 in SPM_SOFT_RESETS register to get ARM out of reset
       * Clear PIN_RESET in SPM_STICKY_BITS register so microcode jumps to ARM reset vector
       */
      ret = bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR,TCC_REG_SPM_SOFT_RESETS, 0x00);

      bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() line %d ret: %d\n", __func__, __LINE__, ret);)

      ret = bcmtch_com_write_mem_reg32(SPM_STICKY_BITS, 0x02);
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() line %d ret: %d\n", __func__, __LINE__, ret);)

      /* We need to wait until firmware is running so we can proceed to send a command
       * We will sleep fro couple of seconds for now. In future we need to wait for
       * message form firmware to inform that the boot is complete
       */
      return (bcmtch_wait_for_fw_ready());

   }
   else
   {

#ifdef LOAD_N_RUN
      bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR,TCC_REG_SPM_SOFT_RESETS, 0x00);

      bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

      bcmtch_com_write_mem_reg32(SPM_STICKY_BITS, 0x02)
      return (NAPA_INIT_SUCCESS);
#else
      printk(KERN_ERR "ERROR: Napa images not loaded.\n");
      return(NAPA_INIT_ERROR);
#endif
   }

   return -1;

}

int bcmtch_wait_for_fw_ready(void)
{
   int count = 5000;
   unsigned char ready;

   /*
    * IMPORTANT
    * This function looks for specific message from FW.  If FW writes any other
    * message to host, this will fail.  Hence FW must only report error messages
    * after FW READY has been set.
    */

   ready = bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR,TCC_REG_SPM_MSG_TO_HOST);
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk(KERN_INFO "INFO: Downloading firmware and waiting for firmware ready\n");)
   while ((ready != TOFE_MESSAGE_FW_READY) && count--)
   {
      printk(KERN_INFO "INFO: Waiting for -firmware ready- 0x%0x\n", ready);
      ready = bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR,TCC_REG_SPM_MSG_TO_HOST);
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() line %d ready = %d\n", __func__, __LINE__, ready);)
   }

   if (count <= 0)
   {
      printk(KERN_ERR "ERROR: Failed to communicate with Napa FW. Error: 0x%x", ready);
      return NAPA_INIT_ERROR;
   }

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() returning SUCCESS!\n", __func__);)
   return(NAPA_INIT_SUCCESS);
}

#if TOFE_CHAN_PROCESSING
/**
 * Init Ram Contents
 */
void bcmtch_init_ram_contents(void)
{

#ifdef _X64
    long long *llPtr;
    uint32_t   jj;
#endif
    uint32_t index;

	/* get pointer to tofe firmware signature */
	tofe_tool.m_p_sig = (tofe_signature_t *)tofe_tool.m_p_image;

	/* get pointer to tofe firmware table of contents */
	tofe_tool.m_p_toc = (tofe_toc_t *)((char *)((uint32_t)tofe_tool.m_p_image + (uint32_t)TOFE_SIGNATURE_SIZE));

        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("m_p_toc  = %08x\n",(unsigned int)tofe_tool.m_p_toc);)

    /* setup cfg array */
#ifdef _X64
        jj = 0;
        llPtr = (long long *)&tofe_tool.m_p_toc->cfg[0];
        for (index = 0 ; index < TOFE_TOC_INDEX_MAX ; index+=2) {

		    tofe_tool.cfg[index+1] = *llPtr >> 32;
		    tofe_tool.cfg[index  ] = *llPtr & 0xffffffff;
            ++llPtr;

            DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL) {
                printk("tofe_tool.cfg[%d]=0x%08x (x64)\n",index,  tofe_tool.cfg[index]);
                printk("tofe_tool.cfg[%d]=0x%08x (x64)\n",index+1,tofe_tool.cfg[index+1]);
            })

            ++jj;
        }
#else
        for (index = 0 ; index < TOFE_TOC_INDEX_MAX ; ++index) {
		    tofe_tool.cfg[index] = tofe_tool.m_p_toc->cfg[index];

            DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
                printk("tofe_tool.cfg[%d]=0x%08x\n",index,  (unsigned int)tofe_tool.cfg[index]);)

        }
#endif

	tofe_tool.m_p_channel_configs =
		(tofe_channel_cfg_t *)
		( (uint32_t)tofe_tool.m_p_image
  		+ (uint32_t)tofe_tool.cfg[TOFE_TOC_INDEX_CHANNEL]
		- RAM_OFFSET );
}

/**
 *
 * Get channel specific configuration
 *
 * @param [in] channel_id Channel Id
 *
 * @retval channel_cfg     Channel Configuration
 *
 */
tofe_channel_instance_cfg_t *bcmtch_get_channel_config(tofe_channel_id_t channel_id)
{
    if(NULL == tofe_tool.m_p_sig)
        bcmtch_init_ram_contents();

    return(&(tofe_tool.m_p_channel_configs->channel_cfg[channel_id]));
}

int bcmtch_touch_chan_init(void)
{

    tofe_channel_instance_cfg_t *channel_cfg;

    // Init Touch Channel
    channel_cfg = bcmtch_get_channel_config(TOFE_CHANNEL_ID_TOUCH);
    bcmtch_chan_init(channel_cfg, &touch_channel.channel);

    DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
        printk("%s() line %d ok! \n", __func__, __LINE__);)
    return (NAPA_INIT_SUCCESS);
}

int bcmtch_touch_chan_free(void)
{
    // TODO: free allocated memory
    return (NAPA_INIT_SUCCESS);
}

int bcmtch_chan_init(tofe_channel_instance_cfg_t *channel_cfg, napa_channel_t **channel)
{
	/* locals */
	int	        host_channel_size;
        napa_channel_t *ptr;

	/* validate params */
	*channel = NULL;
	if(!channel_cfg) {
            DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
                printk("%s() line %d ERROR! !channel_cfg \n", __func__, __LINE__);)
	    return NAPA_INIT_ERROR;
        }
	/* TODO: Check if buffer is contiguous or scattered and then do singel read
	 * or multipel reads. For now do single read assuming contiguous buffer.
	 *
	 * The channel_buf_size need to be calculated for all available channels so we can read
	 * all channel's header and data in one SPI / I2C operation. */
	host_channel_size = channel_cfg->entry_num * channel_cfg->entry_size;
	host_channel_size += sizeof(tofe_channel_header_t);
	host_channel_size += sizeof(tofe_channel_instance_cfg_t);

	/* alloc memory for the channel */
        *channel = kzalloc(host_channel_size, GFP_KERNEL);
	if (*channel == NULL)
	{
            DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
                printk(KERN_ERR "%s: failed to alloc BCM915500 channel mem.\n", __func__);)
	}

	/* copy config */
        ptr = *channel;
	ptr->cfg = *channel_cfg;

#if 1
        // DEBUG....
        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk(KERN_INFO "%s: %08x %08x %08x %08x %08x %08x \n", __func__,
            ptr->cfg.entry_num,
            ptr->cfg.entry_size,
            ptr->cfg.trig_level,
            ptr->cfg.flags,
	    (unsigned int)ptr->cfg.channel_header,
	    (unsigned int)ptr->cfg.channel_data);)
#endif

        return (NAPA_INIT_SUCCESS);
}
#endif  // TOFE_CHAN_PROCESSING

#if NAPA_GPIO_PROCESSING
void bcmtch_init_int_handler(void)
{

    /* Read the GPIO config and find out which GPIO pin is used for
     * interrupting host. Stire this pin so we can poll it for data ready
     */

    // pin_num      = 0;
    // function     = 1;  // TOFE_GPIO_PIN_HOST_INT
    // direction    = 1;  // TOFE_GPIO_DIR_OUT
    // active_level = 1;
    // enabled      = 1;

    host_int_gpio_pin_addr   = 0x30180004;     // (0x30180000 + (4 << pin_num)
    host_int_asserted        = 1;              // Active Level

}

int bcmtch_check_wakeup(int count)
{
    while((bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR,TCC_REG_SPM_PSR) != SPM_POWER_STATE_ACTIVE) && count--);
    return(bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR,TCC_REG_SPM_PSR) == SPM_POWER_STATE_ACTIVE);
}

void bcmtch_request_wakeup(int cause)
{

    bcmtch_mutex_lock(NAPA_MUTEX);
    bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_MSG_FROM_HOST, cause);

    bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

    bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_Request_from_Host, TCC_SPM_REQUEST_FROM_HOST_WAKE_REQUEST);

    bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

    bcmtch_mutex_release(NAPA_MUTEX);

}

void bcmtch_release_wakeup(int cause)
{
    struct i2c_msg       xfer[2];
    unsigned char        buffer[2];
    unsigned char        buffer2[2];
    struct i2c_client    *p_i2c_client;

    bcmtch_mutex_lock(NAPA_MUTEX);
#if NAPA_DONT_USE_12C_TRANSFER
    bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_MSG_FROM_HOST, cause);

    bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_Request_from_Host, TCC_SPM_REQUEST_FROM_HOST_RELEASE_WAKE_REQUEST);
#else

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
   {
      printk("%s() reg = %d data = %d\n", __func__, TCC_REG_SPM_MSG_FROM_HOST, cause);
   })

   // Write register and data
   buffer[0] = (unsigned char)TCC_REG_SPM_MSG_FROM_HOST;
   buffer[1] = cause;

   /* Write register */
   xfer[0].addr = NAPA_I2C_SLAVE_ADDR;
   xfer[0].len = 2;
   xfer[0].flags = 0;
   xfer[0].buf = buffer;

   // Write register and data
   buffer2[0] = (unsigned char)TCC_REG_SPM_Request_from_Host;
   buffer2[1] = TCC_SPM_REQUEST_FROM_HOST_RELEASE_WAKE_REQUEST;

   /* Write register */
   xfer[1].addr = NAPA_I2C_SLAVE_ADDR;
   xfer[1].len = 2;
   xfer[1].flags = 0;
   xfer[1].buf = buffer2;

   if (xfer[0].addr == gp_i2c_client_spm->addr)
   {
      p_i2c_client = gp_i2c_client_spm;
   }
   else if (xfer[0].addr == gp_i2c_client_ahb->addr)
   {
      p_i2c_client = gp_i2c_client_ahb;
   }
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, xfer[0].addr);
      return ;
   }

   if (i2c_transfer(p_i2c_client->adapter, xfer, 2) != 2) {
       dev_err(&p_i2c_client->dev, "%s: i2c transfer failed\n", __func__);
       return ;
   }
#endif
    bcmtch_mutex_release(NAPA_MUTEX);

}

int bcmtch_request_sleep(void)
{

    int ret_code = BCMTCH_SUCCESS;
    int           data;
    int           spmReadError;

    // Napa sleep request.
    bcmtch_mutex_lock(NAPA_MUTEX);
    bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_MSG_FROM_HOST, TOFE_COMMAND_NO_COMMAND);

    data = 0;
    bcmtch_com_write_mem_reg32(SPM_PWR_CTRL, data);
    bcmtch_mutex_release(NAPA_MUTEX);

    spmReadError = 0;
    while ((++spmReadError<SPM_READ_ERROR_MAX)&&
        (bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_PSR) != SPM_POWER_STATE_SLEEP));

    if (spmReadError == SPM_READ_ERROR_MAX)
    {
        // Force chip RESET
        bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_SOFT_RESETS, 1);
        bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_SOFT_RESETS, 2);

        // Comm interface will be lost at reset
        bcmtch_com_write(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_SPI_I2C_MODE, TCC_HOST_IF_I2C_MODE);
        bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?
        bcmtch_com_write_reg(NAPA_I2C_SLAVE_ADDR, SPI_HOST_I2CS_CHIPID, HW_BCM915500_SLAVE_AHB);
        bcmtch_sleep_ms(2);   // FIXME : REMOVE / REDUCE ?

        if (bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_PSR) != SPM_POWER_STATE_SLEEP)
        {
            printk(KERN_ERR "ERROR: Napa sleep request failure.\n");
            ret_code = BCMTCH_ERROR;
        }

    }

    return ret_code;
}

unsigned char bcmtch_int_asserted(void)
{
    unsigned int host_int = 0;

    if (host_int_gpio_pin_addr)
    {
        // If FW has asserted GPIO, then it chip is in ACTIVE state
        if (!bcmtch_check_wakeup(1))
        {
	    return false;
        }

        bcmtch_com_read_mem_reg32(host_int_gpio_pin_addr, &host_int);

        if (host_int == host_int_asserted)
	{
	    return true;
        }
    }
    return false;
}

void bcmtch_int_deassert(void)
{
    if (host_int_gpio_pin_addr)
    {
        bcmtch_release_wakeup(TOFE_COMMAND_INTERRUPT_ACK);
    }
}

int bcmtch_interrupt(void)
{

    uint32_t num_touch_events = 0;
    uint32_t retVal = 0;

//    bcmtch_int_deassert();

    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("%s() line %d \n", __func__, __LINE__);)

    /*
     ** Read Touch Event Channel
     */


    num_touch_events = bcmtch_channel_read(touch_channel.channel);

    if(num_touch_events)
    {
        /*
	** Update channel read pointer = write pointer since we read all the data.
	** - write the value of the 'write pointer' into the 'read pointer' of the header
	**
	** NOTE: cannot reset the local copy 'read pointer'
	**       since it is needed below by mtr_process_events()
	*/
        bcmtch_channel_write_header_pointer_flags(
            touch_channel.channel,
            NAPA_CHANNEL_WRITE_HEADER_READER,
            touch_channel.channel->header.write,
            touch_channel.channel->header.flags & ~TOFE_CHANNEL_FLAG_STATUS_LEVEL_TRIGGER);
    }


    if(num_touch_events)
    {
        /*
        ** Write 'Read Complete' Message to Mailbox
        */
#if !NAPA_MERGE_WRITEBACK_REGS
        bcmtch_release_wakeup(TOFE_COMMAND_NO_COMMAND);
#endif
    }

    retVal = (num_touch_events);

    if(num_touch_events)
    {
        /*
	** Process Events
	*/
        bcmtch_process_touch_events(touch_channel.channel);
    }

    return(retVal);
}

void bcmtch_channel_read_data(napa_channel_t *channel)
{

    /* locals */
    uint32_t channel_size_data;

    /* validate params */
    if(channel == NULL)
    {
        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("%s() line %d channel==NULL\n", __func__, __LINE__);)
	return;
    }

    /*
    ** Calculate sizes
    ** - channel_size_data = size of channel data buffer
    */
	channel_size_data =
		(channel->cfg.entry_num * channel->cfg.entry_size);

    /*
    ** read channel data buffer
    */

    bcmtch_com_read_mem((int)channel->cfg.channel_data, channel_size_data, (void*)&channel->data);
}

uint32_t bcmtch_channel_read_header(napa_channel_t *channel)
{

    /* validate params */
    if(channel == NULL)
    {
        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("%s() line %d channel==NULL\n", __func__, __LINE__);)
	return(0);
    }

    /*
     ** read channel header
     */

    bcmtch_com_read_mem((int)channel->cfg.channel_header, sizeof(channel->header), (void*)&channel->header);

    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d channel_header %08x sizeof %08x\n", __func__, __LINE__, (unsigned int)channel->cfg.channel_header, sizeof(channel->header));)
    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d write %08x entry_num %08x entry size %08x trig_level %08x flags %08x read %08x data_offset %08x read_iterator %08x write_iterator %08x\n",__func__, __LINE__,

     channel->header.write,
     channel->header.entry_num,
     channel->header.entry_size,
     channel->header.trig_level,
     channel->header.flags,
     channel->header.read,
     channel->header.data_offset,
     channel->header.read_iterator,
     channel->header.write_iterator

    );)

    /* return count of elements in channel to read */
    return(bcmtch_channel_num_queued(&channel->header));
}

#if NAPA_SINGLE_READ
uint32_t bcmtch_channel_read(napa_channel_t *channel)
{

    /* local */
    int32_t num_elements = 0;
    uint32_t channel_size_data;
    uint32_t complete_read_size;

    /* validate params */
    if(channel == NULL)
    {
        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("%s() line %d channel==NULL\n", __func__, __LINE__);)
        return(num_elements);
    }

    /*
    ** Calculate sizes
    ** - channel_size_data = size of channel data buffer
    */
    channel_size_data =
        (channel->cfg.entry_num * channel->cfg.entry_size);

    complete_read_size = channel_size_data + sizeof(channel->header);

    /* read channel header & channel data buffer */
    bcmtch_com_read_mem((int)channel->cfg.channel_header, complete_read_size, (void*)&channel->header);

    /* get count */
    num_elements = bcmtch_channel_num_queued(&channel->header);

    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d channel_header %08x sizeof %08x\n", __func__, __LINE__, (unsigned int)channel->cfg.channel_header, sizeof(channel->header));)
    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d write %08x entry_num %08x entry size %08x trig_level %08x flags %08x read %08x data_offset %08x read_iterator %08x write_iterator %08x\n",__func__, __LINE__,

     channel->header.write,
     channel->header.entry_num,
     channel->header.entry_size,
     channel->header.trig_level,
     channel->header.flags,
     channel->header.read,
     channel->header.data_offset,
     channel->header.read_iterator,
     channel->header.write_iterator

    );)

    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d num_elements %08x \n", __func__, __LINE__, num_elements); )

    /* return count of elements in channel to read */
    return(num_elements);
}
#else
uint32_t bcmtch_channel_read(napa_channel_t *channel)
{

    /* local */
    int32_t num_elements = 0;

    /* validate params */
    if(channel == NULL)
    {
        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("%s() line %d channel==NULL\n", __func__, __LINE__);)
        return(num_elements);
    }

    /* read channel header */
    num_elements = bcmtch_channel_read_header(channel);

    if(num_elements)
    {
        /* read channel data buffer */
	bcmtch_channel_read_data(channel);
    }

    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d num_elements %08x \n", __func__, __LINE__, num_elements); )

    /* return count of elements in channel to read */
    return(num_elements);
}
#endif

void bcmtch_channel_write_header_pointer(napa_channel_t *channel, napa_channel_write_header_e which, uint32_t pointer_data)
{
#if 1
    /* validate params */
    if(channel == NULL)
    {
        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("%s() line %d channel==NULL\n", __func__, __LINE__);)
        return;
    }

    if(which == NAPA_CHANNEL_WRITE_HEADER_READER)
    {

        bcmtch_com_write_mem((int)channel->cfg.channel_header + offsetof(tofe_channel_header_t, read),
        sizeof(channel->header.read),
        &pointer_data);

     }else{

        bcmtch_com_write_mem((int)channel->cfg.channel_header + offsetof(tofe_channel_header_t, write),
        sizeof(channel->header.write),
        &pointer_data);

     }
#else

    /*
     * init variables for mem write
     */

    int ahb_addr = (int)channel->cfg.channel_header + offsetof(tofe_channel_header_t, read);
    int len = sizeof(channel->header.read);
    void *data = &pointer_data;

    /*
     * mem write
     */

    int count;
    int length = 8;
    unsigned char *I2CWritePkt;

    //allocate buffer for the num_bytes
    //ATT: we need one byte more for the offset
    I2CWritePkt = kzalloc(len + 1, GFP_KERNEL);

    //set the address
    I2Cheader[1] = (unsigned char)(ahb_addr & 0xFF);
    I2Cheader[2] = (unsigned char)((ahb_addr & 0xFF00) >> 8);
    I2Cheader[3] = (unsigned char)((ahb_addr & 0xFF0000) >> 16);
    I2Cheader[4] = (unsigned char)((ahb_addr & 0xFF000000) >> 24);
    //set the length
    I2Cheader[5] = (unsigned char)(len & 0xFF);
    I2Cheader[6] = (unsigned char)((len & 0xFF00) >> 8);
    //set the command
    I2Cheader[7] = (unsigned char)MahbWrite;

    count = (int)sizeof(I2Cheader);

    struct i2c_msg       xfer[4];
    struct i2c_client    *p_i2c_client;

    DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
       printk("%s() ahb_addr %d len %d before count = %d\n", __func__, ahb_addr, len, count);)

    /* Write DMA header */
    xfer[0].addr = NAPA_I2C_AHB_SLAVE_ADDR;
    xfer[0].len = length;
    xfer[0].flags = 0;
    xfer[0].buf = I2Cheader;

    I2CWritePkt[0] = I2C_REG_WFIFO_DATA;    //select the write FiFo
    /* Prepare the dat packet, select the write FiFo. */
    memcpy(I2CWritePkt + 1, data, len);

    /* Write DMA data */
    xfer[1].addr = NAPA_I2C_AHB_SLAVE_ADDR;
    xfer[1].len = (int)len+1;
    xfer[1].flags = 0;
    xfer[1].buf = I2CWritePkt;

    count = (int)len+1;

    /*
     * reg writes
     */

   unsigned char        buffer[2];
   unsigned char        buffer2[2];

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
   {
      printk("%s() reg = %d data = %d\n", __func__, TCC_REG_SPM_MSG_FROM_HOST, cause);
   })

   // Write register and data
   buffer[0] = (unsigned char)TCC_REG_SPM_MSG_FROM_HOST;
   buffer[1] = cause;

   /* Write register */
   xfer[2].addr = NAPA_I2C_SLAVE_ADDR;
   xfer[2].len = 2;
   xfer[2].flags = 0;
   xfer[2].buf = buffer;

   // Write register and data
   buffer2[0] = (unsigned char)TCC_REG_SPM_Request_from_Host;
   buffer2[1] = TCC_SPM_REQUEST_FROM_HOST_RELEASE_WAKE_REQUEST;

   /* Write register */
   xfer[3].addr = NAPA_I2C_SLAVE_ADDR;
   xfer[3].len = 2;
   xfer[3].flags = 0;
   xfer[3].buf = buffer2;

   if (xfer[0].addr == gp_i2c_client_spm->addr)
   {
      p_i2c_client = gp_i2c_client_spm;
   }
   else if (xfer[0].addr == gp_i2c_client_ahb->addr)
   {
      p_i2c_client = gp_i2c_client_ahb;
   }
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, xfer[0].addr);
      return -1;
   }

   if (i2c_transfer(p_i2c_client->adapter, xfer, 2) != 2) {
       dev_err(&p_i2c_client->dev, "%s: i2c transfer failed\n", __func__);
       return -EIO;
   }
#endif
}

unsigned bcmtch_channel_num_queued(tofe_channel_header_t *channel)
{

    if (channel->write >= channel->read)
        return channel->write - channel->read;
    else
        return channel->entry_num - (channel->read - channel->write);

}

void bcmtch_channel_write_header_pointer_flags(napa_channel_t *channel,
                                               napa_channel_write_header_e which,
	                                       uint32_t pointer_data,
	                                       uint8_t flags)
{

    tofe_channel_header_t target_header;

    /* validate params */
    if(channel == NULL)
    {
        DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
            printk("%s() line %d channel==NULL\n", __func__, __LINE__);)
        return;
    }

    /* set target */
    target_header = channel->header;
    target_header.flags = flags;
#if !NAPA_MERGE_WRITEBACK_REGS
    if(which == NAPA_CHANNEL_WRITE_HEADER_READER)
    {

        target_header.read = pointer_data;

        bcmtch_com_write_mem((int)channel->cfg.channel_header + offsetof(tofe_channel_header_t, entry_num),
	                   (offsetof(tofe_channel_header_t, data_offset) - offsetof(tofe_channel_header_t, entry_num)),
	                   &target_header.entry_num);

    }else{

	target_header.write = pointer_data;

  	bcmtch_com_write_mem((int)channel->cfg.channel_header + offsetof(tofe_channel_header_t, write),
  	                   (offsetof(tofe_channel_header_t, read) - offsetof(tofe_channel_header_t, write)),
  	                   &target_header.write );

    }
#else

    /*
     * init variables for mem write
     */
    if(which == NAPA_CHANNEL_WRITE_HEADER_READER)
    {
        target_header.read = pointer_data;
    }
    else
    {
        target_header.write = pointer_data;
    }

    int ahb_addr = ((int)channel->cfg.channel_header + offsetof(tofe_channel_header_t, entry_num));
    int len = (offsetof(tofe_channel_header_t, data_offset) - offsetof(tofe_channel_header_t, entry_num));
    void *data = &target_header.entry_num;

    /*
     * mem write
     */

    int count;
    int length = 8;
    unsigned char *I2CWritePkt;

    //allocate buffer for the num_bytes
    //ATT: we need one byte more for the offset
    I2CWritePkt = kzalloc(len + 1, GFP_KERNEL);

    //set the address
    I2Cheader[1] = (unsigned char)(ahb_addr & 0xFF);
    I2Cheader[2] = (unsigned char)((ahb_addr & 0xFF00) >> 8);
    I2Cheader[3] = (unsigned char)((ahb_addr & 0xFF0000) >> 16);
    I2Cheader[4] = (unsigned char)((ahb_addr & 0xFF000000) >> 24);
    //set the length
    I2Cheader[5] = (unsigned char)(len & 0xFF);
    I2Cheader[6] = (unsigned char)((len & 0xFF00) >> 8);
    //set the command
    I2Cheader[7] = (unsigned char)MahbWrite;

    count = (int)sizeof(I2Cheader);

    struct i2c_msg       xfer[4];
    struct i2c_client    *p_i2c_client;

    DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
       printk("%s() ahb_addr %d len %d before count = %d\n", __func__, ahb_addr, len, count);)

    /* Write DMA header */
    xfer[0].addr = NAPA_I2C_AHB_SLAVE_ADDR;
    xfer[0].len = length;
    xfer[0].flags = 0;
    xfer[0].buf = I2Cheader;

    I2CWritePkt[0] = I2C_REG_WFIFO_DATA;    //select the write FiFo
    /* Prepare the dat packet, select the write FiFo. */
    memcpy(I2CWritePkt + 1, data, len);

    /* Write DMA data */
    xfer[1].addr = NAPA_I2C_AHB_SLAVE_ADDR;
    xfer[1].len = (int)len+1;
    xfer[1].flags = 0;
    xfer[1].buf = I2CWritePkt;

    count = (int)len+1;

    /*
     * reg writes
     */

   unsigned char        buffer[2];
   unsigned char        buffer2[2];

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
   {
      printk("%s() reg = %d data = %d\n", __func__, TCC_REG_SPM_MSG_FROM_HOST, TOFE_COMMAND_NO_COMMAND);
   })

   // Write register and data
   buffer[0] = (unsigned char)TCC_REG_SPM_MSG_FROM_HOST;
   buffer[1] = TOFE_COMMAND_NO_COMMAND;

   /* Write register */
   xfer[2].addr = NAPA_I2C_SLAVE_ADDR;
   xfer[2].len = 2;
   xfer[2].flags = 0;
   xfer[2].buf = buffer;

   // Write register and data
   buffer2[0] = (unsigned char)TCC_REG_SPM_Request_from_Host;
   buffer2[1] = TCC_SPM_REQUEST_FROM_HOST_RELEASE_WAKE_REQUEST;

   /* Write register */
   xfer[3].addr = NAPA_I2C_SLAVE_ADDR;
   xfer[3].len = 2;
   xfer[3].flags = 0;
   xfer[3].buf = buffer2;

   if (xfer[0].addr == gp_i2c_client_spm->addr)
   {
      p_i2c_client = gp_i2c_client_spm;
   }
   else if (xfer[0].addr == gp_i2c_client_ahb->addr)
   {
      p_i2c_client = gp_i2c_client_ahb;
   }
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, xfer[0].addr);
      return -1;
   }

   if (i2c_transfer(p_i2c_client->adapter, xfer, 4) != 4) {
       dev_err(&p_i2c_client->dev, "%s: i2c transfer failed\n", __func__);
       return -EIO;
   }
#endif

}

void bcmtch_process_touch_events(napa_channel_t *touch_channel)
{

    /*
    ** Process MTC touch events from the touch event channel
    */

    int index;
    bcmtch_event_t           *pmtc_event;
    int touch_num = 0;
//  static bool have_timestamps = false;

    tofe_channel_read_begin(&touch_channel->header);

    while ( (pmtc_event = (bcmtch_event_t *)tofe_channel_read(&touch_channel->header)) )
    {
#if TOUCHCON_MTC_FULL_FIRMWARE
        bcmtch_event_touch_t     *ptch_event;
//      bcmtch_event_timestamp_t *ptimestamp_event;
	switch(pmtc_event->type)
	{
	    case BCMTCH_EVENT_TYPE_DOWN:
   	    case BCMTCH_EVENT_TYPE_UP:
	    case BCMTCH_EVENT_TYPE_MOVE:
	        ptch_event = (bcmtch_event_touch_t *)pmtc_event;

#if HW_BCM915500_AXIS_SWAP_Y
	        ptch_event->y = NAPA_MAX_Y - ptch_event->y;
#endif

#if HW_BCM915500_AXIS_SWAP_X
	        ptch_event->x = NAPA_MAX_X - ptch_event->x;
#endif

	        if (ptch_event->type == BCMTCH_EVENT_TYPE_DOWN)
	        {
                DEBUG(if (mod_debug & MOD_DEBUG_DOWN)
                  printk("%s() rxd BCMTCH_EVENT_TYPE_DOWN\n", __func__);)

                    bcmtch_event_touch_down(ptch_event->track_tag, ptch_event->x, ptch_event->y);
                    DEBUG(if ((mod_debug & MOD_DEBUG_CHANNEL) || (mod_debug & MOD_DEBUG_DOWN))
                        printk("%s() line %d BCMTCH_EVENT_TYPE_DOWN: T%d: (%d , %d)\n", __func__, __LINE__,
                                                                                    ptch_event->track_tag,
                                                                                    ptch_event->x,
                                                                                    ptch_event->y );)
	        }

	        if (ptch_event->type == BCMTCH_EVENT_TYPE_MOVE)
	        {
                DEBUG(if (mod_debug & MOD_DEBUG_MOVE)
                  printk("%s() rxd BCMTCH_EVENT_TYPE_MOVE\n", __func__);)

                    bcmtch_event_touch_move(ptch_event->track_tag, ptch_event->x, ptch_event->y);

		    /*
		     * Note: The move event is renamed to "position" for log display.
		     * This change motivated by discussion of appropriateness of labeling
		     * a move with not change in position as a "move".
		    */
                    DEBUG(if ((mod_debug & MOD_DEBUG_CHANNEL) || (mod_debug & MOD_DEBUG_MOVE))
                        printk("%s() line %d BCMTCH_EVENT_TYPE_MOVE: T%d: (%d , %d)\n", __func__, __LINE__,
                                                                                    ptch_event->track_tag,
                                                                                    ptch_event->x,
                                                                                    ptch_event->y );)
		}

		if (ptch_event->type == BCMTCH_EVENT_TYPE_UP)
		{
                DEBUG(if (mod_debug & MOD_DEBUG_UP)
                  printk("%s() rxd BCMTCH_EVENT_TYPE_UP\n", __func__);)

                    bcmtch_event_touch_up(ptch_event->track_tag, ptch_event->x, ptch_event->y);
                    DEBUG(if ((mod_debug & MOD_DEBUG_CHANNEL) || (mod_debug & MOD_DEBUG_UP))
                        printk("%s() line %d BCMTCH_EVENT_TYPE_UP: T%d: (%d , %d)\n", __func__, __LINE__,
                                                                                    ptch_event->track_tag,
                                                                                    ptch_event->x,
                                                                                    ptch_event->y );)
                }

		break;

		case BCMTCH_EVENT_TYPE_FRAME:
#if DEBUG_INTERFACE_TUNING
		        ++d_i_t_fc;
#endif
                DEBUG(if (mod_debug & MOD_DEBUG_FRAME)
                  printk("%s() rxd BCMTCH_EVENT_TYPE_FRAME\n", __func__);)
#if THROTTLE_MOVE_FRAMES
              if  (bcmtch_throttle_move_frame() == 0)
#endif
              {
                    DEBUG(if ((mod_debug & MOD_DEBUG_CHANNEL) || (mod_debug & MOD_DEBUG_FRAME))
                        printk("%s() line %d BCMTCH_EVENT_TYPE_FRAME   \n", __func__, __LINE__);)

#if NAPA_USE_MTC
                    touch_num = 0;
                    for (index = 0 ; index < NAPA_MAX_TOUCH ; ++index) {
                        input_mt_slot(pInputDev, index);
                        input_mt_report_slot_state(pInputDev, MT_TOOL_FINGER, (napa_touch_status[index]));

                        if (napa_touch_status[index]) {
                            touch_num++;
#if HW_BCM915500_AXIS_SWAP_X_Y
                            input_report_abs(pInputDev, ABS_MT_POSITION_X, napa_touch_y[index]);
                            input_report_abs(pInputDev, ABS_MT_POSITION_Y, napa_touch_x[index]);
#else
                            input_report_abs(pInputDev, ABS_MT_POSITION_X, napa_touch_x[index]);
                            input_report_abs(pInputDev, ABS_MT_POSITION_Y, napa_touch_y[index]);
#endif
#if NAPA_MIMIC_MT
                            input_mt_sync(pInputDev);
#endif
                        }
                    }
                    input_report_key(pInputDev, BTN_TOUCH, touch_num > 0);
                    input_sync(pInputDev);
#else
#if NAPA_USE_TOUCH
                    touch_num = 0;
#endif
                    for (index = 0 ; index < NAPA_MAX_TOUCH ; ++index) {
                        if (napa_touch_status[index]) {
#if NAPA_USE_TOUCH
                            touch_num++;
#endif

#if NAPA_MIMIC_MT
#if NAPA_USE_TOUCH
#else
                            input_report_abs(pInputDev, ABS_MT_TOUCH_MAJOR, 64);
#endif
#else
                            input_report_abs(pInputDev, ABS_MT_TRACKING_ID, index);
                            input_report_abs(pInputDev, ABS_MT_TOUCH_MAJOR, 1);
                            input_report_abs(pInputDev, ABS_MT_WIDTH_MAJOR, 0);
#endif
#if HW_BCM915500_AXIS_SWAP_X_Y
                            input_report_abs(pInputDev, ABS_MT_POSITION_X, napa_touch_y[index]);
                            input_report_abs(pInputDev, ABS_MT_POSITION_Y, napa_touch_x[index]);
#else
                            input_report_abs(pInputDev, ABS_MT_POSITION_X, napa_touch_x[index]);
                            input_report_abs(pInputDev, ABS_MT_POSITION_Y, napa_touch_y[index]);
#endif
#if NAPA_MIMIC_MT
                            input_mt_sync(pInputDev);
                        }
#else
                        }
                        input_mt_sync(pInputDev);
#endif
                    }
#if NAPA_USE_TOUCH
                    input_report_key(pInputDev, BTN_TOUCH, touch_num > 0);
#endif
                    input_sync(pInputDev);
#endif
               }
		    break;

		case BCMTCH_EVENT_TYPE_TIMESTAMP:
#if 0
		    ptimestamp_event = (bcmtch_event_timestamp_t *) pmtc_event;
		    {
		        double milliseconds = ((DWORD)ptimestamp_event->timestamp)/100.0;

			if (ptimestamp_event->timestamp_type == BCMTCH_EVENT_TIMESTAMP_TYPE_SCAN_BEGIN)
			{
			    remoteLog.Msg(LOG_LEVEL_DETAIL,"BCMTCH_EVENT_TIMESTAMP: SCAN_BEGIN = %#.2f msec", milliseconds);
		        }
			if (ptimestamp_event->timestamp_type == BCMTCH_EVENT_TIMESTAMP_TYPE_SCAN_END)
			{
			    remoteLog.Msg(LOG_LEVEL_ALL,"BCMTCH_EVENT_TIMESTAMP: SCAN_END = %#.2f msec", milliseconds);
			}
			if (ptimestamp_event->timestamp_type == BCMTCH_EVENT_TIMESTAMP_TYPE_MTC_BEGIN)
			{
			    remoteLog.Msg(LOG_LEVEL_ALL,"BCMTCH_EVENT_TIMESTAMP: MTC_BEGIN = %#.2f msec", milliseconds);
			}
			if (ptimestamp_event->timestamp_type == BCMTCH_EVENT_TIMESTAMP_TYPE_MTC_END)
			{
			    remoteLog.Msg(LOG_LEVEL_ALL,"BCMTCH_EVENT_TIMESTAMP: MTC_END = %#.2f msec", milliseconds);
			}
                    }

	            have_timestamps = true;
//	            remoteLog.Msg(LOG_LEVEL_DETAIL,"timestamp = %f msec\n",
//                    ((DWORD)ptimestamp_event->timestamp)/100.0);
#else

                    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
                        printk("%s() line %d BCMTCH_EVENT_TYPE_TIMESTAMP   \n", __func__, __LINE__);)
#endif
		    break;

                default:
                    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
                        printk("%s() line %d BCMTCH_EVENT_TYPE_... default \n", __func__, __LINE__);)
		    break;
            }
#endif // TOUCHCON_MTC_FULL_FIRMWARE

            // Finished processing event, so update read pointer.
	    tofe_channel_read_end(&touch_channel->header);
    }
}


void bcmtch_event_touch_down(int tag, unsigned short x, unsigned short y)
{
    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d %08x\n", __func__, __LINE__,(unsigned int)pInputDev);)

    if (tag < NAPA_MAX_TOUCH)
    {
        napa_touch_status[tag] = 2; // Down
        napa_touch_x[tag]=x;
        napa_touch_y[tag]=y;
        napa_touch_event[tag] = BCMTCH_EVENT_TYPE_DOWN;
    }
}

void bcmtch_event_touch_up(int tag, unsigned short x, unsigned short y)
{
    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d %08x\n", __func__, __LINE__, (unsigned int)pInputDev);)

    if (tag < NAPA_MAX_TOUCH)
    {
        napa_touch_status[tag] = 0;  // Up
        napa_touch_x[tag]=0;
        napa_touch_y[tag]=0;
        napa_touch_event[tag] = BCMTCH_EVENT_TYPE_UP;
    }
}

void bcmtch_event_touch_move(int tag, unsigned short x, unsigned short y)
{
    DEBUG(if (mod_debug & MOD_DEBUG_CHANNEL)
        printk("%s() line %d %08x\n", __func__, __LINE__, (unsigned int)pInputDev);)

    if (tag < NAPA_MAX_TOUCH)
    {
        napa_touch_status[tag] = 1; // Move
        napa_touch_x[tag]=x;
        napa_touch_y[tag]=y;
        napa_touch_event[tag] = BCMTCH_EVENT_TYPE_MOVE;
    }
}

#if THROTTLE_MOVE_FRAMES
int bcmtch_throttle_move_frame(void)
{
   int i;
   int num_ups_downs = 0;

   for (i = 0; i < NAPA_MAX_TOUCH; i++)
   {
      DEBUG(if (mod_debug & MOD_DEBUG_THROTTLE)
         printk("%s() event type %d\n", __func__, napa_touch_event[i]);)

      if (napa_touch_event[i] == BCMTCH_EVENT_TYPE_DOWN ||
          napa_touch_event[i] == BCMTCH_EVENT_TYPE_UP)
      {
         num_ups_downs++;
      }

      napa_touch_event[i] = BCMTCH_EVENT_TYPE_INVALID;
   }

   if (num_ups_downs > 0)
   {
      return 0;
   }

   g_num_move_frames++;

   if (g_num_move_frames%MOVE_THROTTLE_FACTOR == 0)
      return 0;

   return 1;
}
#endif /* THROTTLE_MOVE_FRAMES */
#endif /* NAPA_GPIO_PROCESSING */


int bcmtch_get_soc_version(unsigned char *chipId, int chipIdBuffLen)
{
    if (chipIdBuffLen != 4)
        return BCMTCH_ERROR;

    *chipId = bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_ChipID0);
    ++chipId;

    *chipId = bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_ChipID1);
    ++chipId;

    *chipId = bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_ChipID2);
    ++chipId;

    *chipId = bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_RevisionID);
    ++chipId;

    return BCMTCH_SUCCESS;
}

unsigned char bcmtch_get_power_mode(void)
{

    unsigned char regVal;

    regVal = bcmtch_com_read_reg(NAPA_I2C_SLAVE_ADDR, TCC_REG_SPM_PSR);

    return regVal;
}


/* ------------------------ */
/* - bcmtch com functions - */
/* ------------------------ */

int bcmtch_com_set_interface(bcmtch_com_e com_interface)
{
    int ret_code;

    if (p_bcmtch_data == NULL) {
        p_bcmtch_data = &bcmtch_data;
    }

    switch(com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            p_bcmtch_data->com_interface = com_interface;
            ret_code = BCMTCH_SUCCESS;
            break;

        case BCMTCH_COM_SPI_INTERFACE:
            printk(KERN_ERR "ERROR: bcmtch spi interface not supported.\n");
            ret_code = BCMTCH_ERROR;
            break;

        default:
            printk(KERN_ERR "ERROR: bcmtch unknown com interface (%d) not supported.\n", com_interface);
            ret_code = BCMTCH_ERROR;
            break;
    }

    return ret_code;
}

int bcmtch_com_read(int slave_addr, int len, unsigned char *buffer)
{
    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_read(slave_addr, len, buffer);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;
}

int bcmtch_com_write(int slave_addr, int len, unsigned char *buffer)
{

    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_write(slave_addr, len, buffer);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;

}

unsigned char bcmtch_com_read_reg(int slave_addr, int reg)
{

    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_read_reg(slave_addr, reg);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;

}

int bcmtch_com_write_reg(int slave_addr, int reg, unsigned char data)
{

    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_write_reg(slave_addr, reg, data);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;

}

int bcmtch_com_write_mem( int mem_addr, int len, void *data)
{

    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_write_mem(mem_addr, len, data);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;


}

int bcmtch_com_read_mem(int mem_addr, int len, void *data)
{

    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_read_mem(mem_addr, len, data);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;

}

int bcmtch_com_write_mem_reg32(int mem_addr, int data)
{

    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_write_mem_reg32(mem_addr, data);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;
}

int bcmtch_com_read_mem_reg32( int mem_addr, void *data)
{

    int ret_code;

    switch(p_bcmtch_data->com_interface) {

        case BCMTCH_COM_I2C_INTERFACE:
            ret_code = bcmtch_i2c_read_mem_reg32(mem_addr, data);
            break;

        default:
            ret_code = BCMTCH_ERROR;
            break;

    }

    return ret_code;
}

/* ------------------------ */
/* - bcmtch os functions - */
/* ------------------------ */

int bcmtch_i2c_read(int slave_addr, int len, unsigned char *buffer)
{
    int count=0;

   struct i2c_client *p_i2c_client;

   if (slave_addr == gp_i2c_client_spm->addr)
      p_i2c_client = gp_i2c_client_spm;
   else if (slave_addr == gp_i2c_client_ahb->addr)
      p_i2c_client = gp_i2c_client_ahb;
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, slave_addr);
      return -1;
   }

   count = i2c_master_recv(p_i2c_client, buffer, len);

   if (count != len)
   {
      printk(KERN_ERR "%s() Unable to send i2c slave 0x%x count %d\n",
             __func__, p_i2c_client->addr, count);
   }

    return count; // Number of bytes read
}

int bcmtch_i2c_write(int slave_addr, int length, unsigned char *buffer)
{
    int count=0;

   struct i2c_client *p_i2c_client;

   if (slave_addr == gp_i2c_client_spm->addr)
   {
      p_i2c_client = gp_i2c_client_spm;
   }
   else if (slave_addr == gp_i2c_client_ahb->addr)
   {
      p_i2c_client = gp_i2c_client_ahb;
   }
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, slave_addr);
      return -1;
   }
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() sending to slave 0x%x\n", __func__, p_i2c_client->addr);)

   count = i2c_master_send(p_i2c_client, buffer, length);

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() i2c_master_send() returned %d\n", __func__, count);)

   if (count != length)
   {
      printk(KERN_ERR "%s() Unable to send i2c slave 0x%x count %d\n",
             __func__, p_i2c_client->addr, count);
   }

   return count;  // Number of bytes written
}

unsigned char bcmtch_i2c_read_reg(int slave_addr, int reg)
{
   struct i2c_msg       xfer[2];
   unsigned char        rbuffer;
   unsigned char        wbuffer;

    DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
       printk("%s() reading register %d\n", __func__, reg);)

#if NAPA_DONT_USE_12C_TRANSFER
    int count=0;
    unsigned char buffer[2];

   // Write register that we want to read
   buffer[0] = reg;
   count = bcmtch_i2c_write(slave_addr, 1, buffer);

   // Read register
   count = bcmtch_i2c_read(slave_addr, 1, buffer);

#else

   // Write register that we want to read
   wbuffer = reg;

   /* Write register */
   xfer[0].addr = slave_addr;
   xfer[0].len = 1;
   xfer[0].flags = 0;
   xfer[0].buf = &wbuffer;

   /* Read data */
   xfer[1].addr = slave_addr;
   xfer[1].flags = I2C_M_RD;
   xfer[1].len = 1;
   xfer[1].buf = &rbuffer;

   if (i2c_transfer(gp_i2c_client_spm->adapter, xfer, 2) != 2) {
       dev_err(&gp_i2c_client_spm->dev, "%s: i2c transfer failed\n", __func__);
       return(-EIO);
   }

   return(rbuffer);
#endif
}

int bcmtch_i2c_write_reg(int slave_addr, int reg, unsigned char data)
{
    struct i2c_msg       xfer[1];
    unsigned char        buffer[2];

    DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
       printk("%s() reg = %d data = %d\n", __func__, reg, data);)

#if NAPA_DONT_USE_12C_TRANSFER
   int count=0;
   unsigned char buffer[2];

   buffer[0] = (unsigned char)reg;
   buffer[1] = data;

   count = bcmtch_i2c_write(slave_addr, 2, buffer);

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
   {
      printk("%s() count = %d\n", __func__, count);
   })

   return count;  // Number of bytes written
#else

   // Write register and data
   buffer[0] = (unsigned char)reg;
   buffer[1] = data;

   /* Write register */
   xfer[0].addr = slave_addr;
   xfer[0].len = 2;
   xfer[0].flags = 0;
   xfer[0].buf = buffer;

   if (i2c_transfer(gp_i2c_client_spm->adapter, xfer, 1) != 1) {
       dev_err(&gp_i2c_client_spm->dev, "%s: i2c transfer failed\n", __func__);
       return(-EIO);
   }

   return(2);  // Number of bytes written
#endif
}

int bcmtch_i2c_write_mem( int ahb_addr, int len, void *data)     // port note, WriteAHBI2C ahb_addr was DWORD
{
   int count;
   int length = 8;
   unsigned char *I2CWritePkt;
   struct i2c_msg       xfer[2];
   struct i2c_client    *p_i2c_client;

   //allocate buffer for the num_bytes
   //ATT: we need one byte more for the offset
   I2CWritePkt = kzalloc(len + 1, GFP_KERNEL);

   //set the address
   I2Cheader[1] = (unsigned char)(ahb_addr & 0xFF);
   I2Cheader[2] = (unsigned char)((ahb_addr & 0xFF00) >> 8);
   I2Cheader[3] = (unsigned char)((ahb_addr & 0xFF0000) >> 16);
   I2Cheader[4] = (unsigned char)((ahb_addr & 0xFF000000) >> 24);
   //set the length
   I2Cheader[5] = (unsigned char)(len & 0xFF);
   I2Cheader[6] = (unsigned char)((len & 0xFF00) >> 8);
   //set the command
   I2Cheader[7] = (unsigned char)MahbWrite;

   count = (int)sizeof(I2Cheader);

#if NAPA_DONT_USE_12C_TRANSFER
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() ahb_addr %d len %d before count = %d\n", __func__, ahb_addr, len, count);)

   count = bcmtch_i2c_write(NAPA_I2C_AHB_SLAVE_ADDR, length, (unsigned char *)I2Cheader);
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() after count = %d\n", __func__, count);)

   if (count < 0)
   {
      printk(KERN_ERR "%s() error,  bcmtch_i2c_write failed, count = %d\n", __func__, count);
      return count;
   }

   /* Should we wait here also a bit? */

   I2CWritePkt[0] = I2C_REG_WFIFO_DATA;    //select the write FiFo
   /* Prepare the dat packet, select the write FiFo. */
   memcpy(I2CWritePkt + 1, data, len);

   /* Send the data (payload). */
   count = bcmtch_i2c_write(NAPA_I2C_AHB_SLAVE_ADDR, (int)len+1, (unsigned char *)I2CWritePkt);

#else

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() ahb_addr 0x%x len %d before count = %d\n", __func__, ahb_addr, len, count);)

   /* Write DMA header */
   xfer[0].addr = NAPA_I2C_AHB_SLAVE_ADDR;
   xfer[0].len = length;
   xfer[0].flags = 0;
   xfer[0].buf = I2Cheader;

   I2CWritePkt[0] = I2C_REG_WFIFO_DATA;    //select the write FiFo
   /* Prepare the dat packet, select the write FiFo. */
   memcpy(I2CWritePkt + 1, data, len);

   /* Write DMA data */
   xfer[1].addr = NAPA_I2C_AHB_SLAVE_ADDR;
   xfer[1].len = (int)len+1;
   xfer[1].flags = 0;
   xfer[1].buf = I2CWritePkt;

   if (xfer[0].addr == gp_i2c_client_spm->addr)
   {
      p_i2c_client = gp_i2c_client_spm;
   }
   else if (xfer[0].addr == gp_i2c_client_ahb->addr)
   {
      p_i2c_client = gp_i2c_client_ahb;
   }
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, xfer[0].addr);
      return -1;
   }

   if (i2c_transfer(p_i2c_client->adapter, xfer, 2) != 2) {
       dev_err(&p_i2c_client->dev, "%s: i2c transfer failed\n", __func__);
       return -EIO;
   }

   count = (int)len+1;
#endif

   if (count < 0)
   {
      printk(KERN_ERR "ERROR: Napa AHB write failed, count = %d\n", count);
   }
   else if (count == 0)
   {
      printk(KERN_ERR "ERROR: Napa AHB write failed - no bytes written, do you have right slave address\n");
   }
   else if (count != (len + 1))
   {
      printk(KERN_ERR "ERROR: Napa AHB write failed - only a partial number of bytes written, (%d)instead of (%d)\n",
      count,len+1);
   }
   else
   {
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() line %d ok!\n", __func__, __LINE__);)
   }

   kfree(I2CWritePkt);

   return count;
}

int bcmtch_i2c_read_mem(int ahb_addr, int len, void *data)     /* port note, ReadAHBI2C ahb_addr was DWORD */
{
   unsigned char I2CreadPkt;
   int timeOut = 0;
   unsigned char status;
   int read_counter = 0;
   struct i2c_msg       xfer[3];
   unsigned char        buffer[2];
   struct i2c_client    *p_i2c_client;

   //set the address
   I2Cheader[1] = (unsigned char)(ahb_addr & 0xFF);
   I2Cheader[2] = (unsigned char)((ahb_addr & 0xFF00) >> 8);
   I2Cheader[3] = (unsigned char)((ahb_addr & 0xFF0000) >> 16);
   I2Cheader[4] = (unsigned char)((ahb_addr & 0xFF000000) >> 24);
   //set the length
   I2Cheader[5] = (unsigned char)(len & 0xFF);
   I2Cheader[6] = (unsigned char)((len & 0xFF00) >> 8);
   //set the command
   I2Cheader[7] = (unsigned char)MahbRead;

#if NAPA_DONT_USE_12C_TRANSFER
   count = bcmtch_i2c_write(NAPA_I2C_AHB_SLAVE_ADDR, (int)sizeof(I2Cheader), (unsigned char *)I2Cheader);
#else

   /* Write register */
   xfer[0].addr = NAPA_I2C_AHB_SLAVE_ADDR;
   xfer[0].len = (int)sizeof(I2Cheader);
   xfer[0].flags = 0;
   xfer[0].buf = I2Cheader;

   // Write register that we want to read
   buffer[0] = I2C_REG_STATUS;

   /* Write register */
   xfer[1].addr = NAPA_I2C_AHB_SLAVE_ADDR;
   xfer[1].len = 1;
   xfer[1].flags = 0;
   xfer[1].buf = buffer;

   /* Read data */
   xfer[2].addr = NAPA_I2C_AHB_SLAVE_ADDR;
   xfer[2].flags = I2C_M_RD;
   xfer[2].len = 1;
   xfer[2].buf = buffer;

   if (xfer[0].addr == gp_i2c_client_spm->addr)
   {
      p_i2c_client = gp_i2c_client_spm;
   }
   else if (xfer[0].addr == gp_i2c_client_ahb->addr)
   {
      p_i2c_client = gp_i2c_client_ahb;
   }
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, xfer[0].addr);
      return -1;
   }

   if (i2c_transfer(p_i2c_client->adapter, xfer, 3) != 3) {
       dev_err(&p_i2c_client->dev, "%s: i2c transfer failed\n", __func__);
       return -EIO;
   }

   status = buffer[0];
#endif

   /* Check the FiFo if result is available and we can start reading. */

   while (status != 1)
   {
      //bcmtch_sleep_ms(1);
      timeOut++;
      if (timeOut > 1000)
      {
         printk(KERN_ERR "ERROR: Napa i2c ahb read timeout\n");
         return 0;
      }
      read_counter++;

      if (read_counter > 3)
         break;

      status = bcmtch_i2c_read_reg(NAPA_I2C_AHB_SLAVE_ADDR, I2C_REG_STATUS);
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() line %d status %d\n", __func__, __LINE__, status);)

      /* PPTEST CLEAN UP NEEDED HERE! */
      if (status == 1)
         break;
   }                 //I2C complete ?

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() line %d leaving while() loop\n", __func__, __LINE__);)


   I2CreadPkt = I2C_REG_RFIFO_DATA;
#if NAPA_DONT_USE_12C_TRANSFER
   count = bcmtch_i2c_write(NAPA_I2C_AHB_SLAVE_ADDR,
                          (int)sizeof(I2CreadPkt),
                          (unsigned char *)&I2CreadPkt);

   count = bcmtch_i2c_read(NAPA_I2C_AHB_SLAVE_ADDR, len, (unsigned char *)data);
#else

   /* Write register */
   xfer[0].addr = NAPA_I2C_AHB_SLAVE_ADDR;
   xfer[0].len = (int)sizeof(I2CreadPkt);
   xfer[0].flags = 0;
   xfer[0].buf = (unsigned char *)&I2CreadPkt;

   /* Read data */
   xfer[1].addr = NAPA_I2C_AHB_SLAVE_ADDR;
   xfer[1].flags = I2C_M_RD;
   xfer[1].len = len;
   xfer[1].buf = data;

   if (i2c_transfer(p_i2c_client->adapter, xfer, 2) != 2) {
       dev_err(&p_i2c_client->dev, "%s: i2c transfer failed\n", __func__);
       return -EIO;
   }
#endif

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() line %d count %d data %d\n", __func__, __LINE__, len, (int)data);)
   return len;
}

int bcmtch_i2c_write_mem_reg32(int ahb_addr, int data)
{
   int len;
   int count;

   len = sizeof(int);

   count = bcmtch_i2c_write_mem(ahb_addr, len, (unsigned char *)&data);

   return count;
}

int bcmtch_i2c_read_mem_reg32( int ahb_addr, void *data)
{
   int len;
   int count;

   len = sizeof(int);

   count = bcmtch_i2c_read_mem(ahb_addr, len, (unsigned char *)data);
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() returning %d read: %d\n", __func__, count, (int)data);)

   return count;
}

int bcmtch_mutex_lock(int mutex)
{
    mutex_lock(&gp_napa_i2c->i2c_mutex);
    return 0;
}

int bcmtch_mutex_release(int mutex)
{
    mutex_unlock(&gp_napa_i2c->i2c_mutex);
    return 0;
}

void bcmtch_sleep_ms(int ms)
{
    msleep(ms);
}

//#ifdef ADD_POLLING
#if ADD_INTERRUPT_HANDLING == 0

void bcmtch_poll(struct napa_i2c *p_napa_i2c)
{
   //g_debug_poll_count++;

   //bcmtch_setup_poll_timer(p_napa_i2c, 2);
   if (mod_debug == MOD_DEBUG_I2C_DOWNLOAD)
      msleep(500);
   else
      usleep_range(2000, 2500);

   queue_work(p_napa_i2c->p_ktouch_wq, &p_napa_i2c->work);
}

/*
   init_timer(&stats_timer);
   stats_timer.expires = jiffies+IDT77105_STATS_TIMER_PERIOD;
   stats_timer.function = idt77105_stats_timer_func;
   add_timer(&stats_timer);
*/

int bcmtch_setup_poll_timer(struct napa_i2c *p_napa_i2c, int delay)
{
   init_timer(&p_napa_i2c->poll_timer);
   p_napa_i2c->poll_timer.expires   = jiffies + delay*HZ;
   p_napa_i2c->poll_timer.function = bcmtch_poll;
   p_napa_i2c->poll_timer.data     = (void *)p_napa_i2c;
   DEBUG(if (mod_debug == MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() adding timer\n", __func__);)
   add_timer(&p_napa_i2c->poll_timer);
}
#endif

static
int32_t
bcmtch_firmware_download(struct napa_i2c *p_device, uint16_t len, const uint8_t *p_data, uint32_t addr)
{
   printk(KERN_INFO "Downloading %d bytes from 0x%x to address 0x%x\n", len, (unsigned int)p_data, addr);

   if (addr == VECTORS_OFFSET)
   {
      g_vector_size = len;
      memcpy(gp_vector, p_data, len);
   }

   if (addr == ROM_OFFSET)
   {
      g_code_size = len;
      memcpy(gp_code, p_data, len);
   }

   if (addr == DATA_OFFSET)
   {
      g_data_size = len;
      memcpy(gp_data, p_data, len);
#if TOFE_CHAN_PROCESSING
      tofe_tool.m_p_image      = gp_data;
      tofe_tool.m_image_size   = len;
      tofe_tool.m_image_loaded = 1; // true
#endif
   }
   return 0;
}

static
int32_t
bcmtch_i2c_firmware_load(struct napa_i2c *p_napa_i2c, const char *p_name, int addr)
{
   const struct firmware *fw;
   int32_t err;

   printk(KERN_INFO "%s calling request_firmware for %s\n", __func__, p_name);

   /** call kernel to start firmware load **/
   /* request_firmware(const struct firmware **fw,
    *                  const char *name,
    *                  struct device *device);
    */
   err = request_firmware(&fw, p_name, &p_napa_i2c->p_i2c_client1->dev);
   if (err)
   {
      printk(KERN_ERR "%s: Firmware request failed (%d)\n", __func__, err);
      return(err);
   }

   /** download to chip **/
   err = bcmtch_firmware_download(p_napa_i2c, fw->size, fw->data, addr);

   /** free kernel structure */
   release_firmware(fw);

   return(err);
}

int bcmtch_write_regs(struct napa_i2c *p_napa_i2c,
                      int slave_addr,
                      u8 *p_reg,
                      u8 *p_val,
                      int length)
{
   u8  buf[10];
   int i;
   int ret = 0;
   struct i2c_client *p_i2c_client;

   if (slave_addr == p_napa_i2c->p_i2c_client->addr)
      p_i2c_client = p_napa_i2c->p_i2c_client;
   else if (slave_addr == p_napa_i2c->p_i2c_client1->addr)
      p_i2c_client = p_napa_i2c->p_i2c_client1;
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, slave_addr);
      return -1;
   }

   for (i = 0; i < length; i++)
   {
      buf[0] = p_reg[i];
      /* Sending to the slave is an I2C write operation. */
      ret = i2c_master_send(p_i2c_client, buf, 1);

      if (ret != 1)
      {
         printk(KERN_ERR "%s() Unable to send i2c slave 0x%x reg 0x%x, ret %d\n",
                __func__, p_i2c_client->addr, p_reg[i], ret);
         return -1;
      }

   }
   return 0;
}



int bcmtch_read_regs(struct napa_i2c *p_napa_i2c,
                     int slave_addr,
                     u8 *p_reg,
                     u8 *p_val,
                     int length)
{
   u8  buf[10];
   int i;
   int ret = 0;
   struct i2c_client *p_i2c_client;

   if (slave_addr == p_napa_i2c->p_i2c_client->addr)
      p_i2c_client = p_napa_i2c->p_i2c_client;
   else if (slave_addr == p_napa_i2c->p_i2c_client1->addr)
      p_i2c_client = p_napa_i2c->p_i2c_client1;
   else
   {
      printk(KERN_ERR "%s() Invalid slave address 0x%x\n",
             __func__, slave_addr);
      return -1;
   }

   for (i = 0; i < length; i++)
   {
      buf[0] = p_reg[i];
      /* Sending to the slave is an I2C write operation. */
      ret = i2c_master_send(p_i2c_client, buf, 1);

      if (ret != 1)
      {
         printk(KERN_ERR "%s() Unable to send i2c slave 0x%x reg 0x%x, ret %d\n",
                __func__, p_i2c_client->addr, p_reg[i], ret);
         return -1;
      }

      /* Receiving from the slave still requires the sending of a I2C message
         but is an I2C read operation.                                         */
      ret = i2c_master_recv(p_i2c_client, buf, 1);
      if (ret != 1)
      {
         printk(KERN_ERR "%s() Unable to recv i2c slave 0x%x reg 0x%x, ret %d\n",
                __func__, p_i2c_client->addr, p_reg[i], ret);
         return -2;
      }
      else
      {
         DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
            printk("%s() slave 0x%x Reg 0x%x val %02x\n",
                   __func__, p_i2c_client->addr, p_reg[i], buf[0]);)
      }

      p_val[i] = buf[0];
   }
   return 0;
}

#if 0
int bcmtch_get_chip_info(struct napa_i2c *p_napa_i2c)
{
   int ret    = 0;
   int length = 4;
   u8 chip_regs[4] =
   {
      BCM_TSC_CHIP_ID0,
      BCM_TSC_CHIP_ID1,
      BCM_TSC_CHIP_ID2,
      BCM_TSC_CHIP_REV,
   };
   u8 chip_vals[4];

   ret = bcmtch_read_regs(p_napa_i2c,
                          BCM_TSC_SPM_SLAVE,
                          chip_regs,
                          chip_vals,
                          length);
   if (ret < 0)
   {
      printk("%s() failed to retrieve chip info\n", __func__);
   }
   else
   {
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() bcm915500 chip info: %2x%2x%2x revision: %d\n",
                __func__, chip_vals[0], chip_vals[3], chip_vals[2], chip_vals[1]);)
   }

   length = 1;
   ret = bcmtch_write_regs(p_napa_i2c,
                           BCM_TSC_AHB_SLAVE,
                           chip_regs,
                           chip_vals,
                           length);

   return ret;
}
#endif

static struct input_dev *bcmtch_allocate_input_dev(void)
{
   int index;
   int ret;
   struct input_dev *pInputDev = NULL;

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
       printk("%s() line %d %08x  BEGIN....\n", __func__, __LINE__,(unsigned int)pInputDev);)

   for (index = 0 ; index < NAPA_MAX_TOUCH ; ++index) {
       napa_touch_status[index]=0;
       napa_touch_event[index] = BCMTCH_EVENT_TYPE_INVALID;
   }

   pInputDev = input_allocate_device();
   if (pInputDev == NULL)
   {
      printk("%s() Failed to allocate input device\n", __func__);
      return NULL;
   }

   pInputDev->name = "BCM915500 Touch Screen";
   pInputDev->phys = "I2C";
   pInputDev->id.bustype = BUS_I2C;
   pInputDev->id.vendor = 0x0EEF;
   pInputDev->id.product = 0x0020;
   pInputDev->id.version = 0x0000;

#if NAPA_USE_MTC
   set_bit(EV_ABS, pInputDev->evbit);
   set_bit(EV_KEY, pInputDev->evbit);
   set_bit(BTN_TOUCH, pInputDev->keybit);
#if NAPA_MIMIC_MT
   __set_bit(INPUT_PROP_DIRECT, pInputDev->propbit);
#else
   set_bit(BTN_TOOL_FINGER, pInputDev->keybit);
#endif

   input_mt_init_slots(pInputDev, NAPA_MAX_TOUCH);

   input_set_abs_params(pInputDev, ABS_MT_POSITION_X, 0, NAPA_MAX_X, 0, 0);
   input_set_abs_params(pInputDev, ABS_MT_POSITION_Y, 0, NAPA_MAX_Y, 0, 0);
#else
   set_bit(EV_SYN, pInputDev->evbit);
   set_bit(EV_ABS, pInputDev->evbit);

#if NAPA_USE_TOUCH
   set_bit(EV_KEY, pInputDev->evbit);
   set_bit(BTN_TOUCH, pInputDev->keybit);
#else
   input_set_abs_params(pInputDev, ABS_MT_TOUCH_MAJOR, 0, 64, 0, 0);
#endif

   input_set_abs_params(pInputDev, ABS_MT_POSITION_X, 0, NAPA_MAX_X, 0, 0);
   input_set_abs_params(pInputDev, ABS_MT_POSITION_Y, 0, NAPA_MAX_Y, 0, 0);
#if NAPA_MIMIC_MT
   __set_bit(INPUT_PROP_DIRECT, pInputDev->propbit);
#else
   input_set_abs_params(pInputDev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
   input_set_abs_params(pInputDev, ABS_MT_TRACKING_ID, 0, NAPA_MAX_TOUCH-1, 0, 0);
#endif
#endif

   input_set_events_per_packet(pInputDev, 10 * NAPA_MAX_TOUCH);
   ret = input_register_device(pInputDev);
   if (ret)
   {
      printk("%s() Unable to register input device\n", __func__);
      input_free_device(pInputDev);
      pInputDev = NULL;
      return NULL;
   }

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
       printk("%s() line %d %08x  END....\n", __func__, __LINE__, (unsigned int)pInputDev);)

   return pInputDev;
}

/*
struct napa_i2c
{
   struct mutex i2c_mutex;
   struct i2c_client *p_i2c_client;
   struct i2c_client *p_i2c_client1;
   int pagesize;

   int gpio_int;
   struct workqueue_struct *p_ktouch_wq;
   struct work_struct work;

   struct bcm915500_platform_data platform_data;
};
*/
static void bcmtch_i2c_wq(struct work_struct *work)
{
   struct napa_i2c *p_napa_i2c;
   struct i2c_client *p_i2c_client;
   int gpio;

#if DEBUG_INTERFACE_TUNING
   unsigned long d_i_t_liq = 0;
   unsigned long d_i_t_ji = jiffies;
   unsigned long d_i_t_jo = 0;

   d_i_t_fc = 0;
#endif
   struct timespec begin, end;

   DEBUG(if (mod_debug & MOD_DEBUG_TIMES)
       {getrawmonotonic(&begin);})

   if (work == NULL)
   {
      printk(KERN_ERR "%s() work == NULL\n", __func__);
      return;
   }

   p_napa_i2c = container_of(work, struct napa_i2c, work);

   if (p_napa_i2c == NULL)
   {
      printk(KERN_ERR "%s() p_napa_i2c == NULL\n", __func__);
      return;
   }

   p_i2c_client = p_napa_i2c->p_i2c_client;

   if (p_i2c_client == NULL)
   {
      printk(KERN_ERR "%s() p_i2c_client == NULL\n", __func__);
      return;
   }

   gpio = irq_to_gpio(p_i2c_client->irq);

   mutex_lock(&p_napa_i2c->mutex_wq);

#if ADD_INTERRUPT_HANDLING
   /* continue recv data while GPIO is pulled low */
#if NAPA_INT_WHILE
   while (!gpio_get_value(gpio))
#else
   if (!gpio_get_value(gpio))
#endif
   {
      /* Interrupt handler code here. */
#if DEBUG_INTERFACE_TUNING
      d_i_t_liq++;
#endif
      bcmtch_interrupt();
#if NAPA_INT_WHILE
      schedule();
   }
#else
   }
#endif

#else
   bcmtch_interrupt();

   bcmtch_poll(p_napa_i2c);
#endif

#if DEBUG_INTERFACE_TUNING
   d_i_t_jo = jiffies;
   printk(KERN_ERR "l[%d] f[%d] %x %x %x\n", d_i_t_liq, d_i_t_fc, d_i_t_i_i, d_i_t_ji, d_i_t_jo);
   d_i_t_i_i = 0;
#endif

   DEBUG(if (mod_debug & MOD_DEBUG_TIMES)
        {getrawmonotonic(&end);
        printk("%s() : b=%d:%d  e=%d:%d\n", __func__, (int)begin.tv_sec, (int)begin.tv_nsec, (int)end.tv_sec, (int)end.tv_nsec);})

   mutex_unlock(&p_napa_i2c->mutex_wq);
}

static irqreturn_t bcmtch_i2c_interrupt_handler(int irq, void *dev_id)
{
   struct napa_i2c *p_napa_i2c = (struct napa_i2c *)dev_id;

#if DEBUG_INTERFACE_TUNING
   if(d_i_t_i_i == 0)
       d_i_t_i_i = jiffies;
#endif


   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() with irq:%d\n", __func__, irq);)

   if (p_napa_i2c == NULL)
   {
      printk(KERN_ERR "%s() p_napa_i2c == NULL\n", __func__);
      return IRQ_HANDLED;
   }

   if (p_napa_i2c->p_ktouch_wq == NULL)
   {
      printk(KERN_ERR "%s() p_napa_i2c->p_ktouch_wq == NULL\n", __func__);
      return IRQ_HANDLED;
   }

   /* postpone I2C transactions to the workqueue as it may block */
   queue_work(p_napa_i2c->p_ktouch_wq, &p_napa_i2c->work);

   return IRQ_HANDLED;
}

int bcmtch_init_hw(struct napa_i2c *p_napa_i2c)
{
   int ret;

   ret = bcmtch_init(p_napa_i2c->p_i2c_client,
                   p_napa_i2c->p_i2c_client1);
   if (ret != 0)
   {
      printk(KERN_ERR "bcm915500_i2c_ts.c %s(): bcmtch_init() failed, ret %d\n",
             __func__, ret);
      return -1;
   }
   else
   {
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() succeeded\n", __func__);)
   }
   return 0;
}


static struct platform_device napa_core_device =
{
   .name              = BCM915500_TSC_NAME,
   .id                = -1,
   .dev.platform_data = NULL,
};

static int bcmtch_i2c_probe(struct i2c_client *p_i2c_client,
                          const struct i2c_device_id *id)
{
   int ret = 0;
   struct bcm915500_platform_data *p_plat_data;
   struct i2c_client  *p_new_i2c_client;
   struct i2c_adapter *p_i2c_adapter;
   DEBUG(unsigned char reg_value;)

   printk(KERN_INFO "\n%s %s %s \n", __func__, __DATE__, __TIME__);

   if (gp_napa_i2c != NULL)
   {
      printk(KERN_INFO "PPTEST %s already called\n", __func__);
      return 0;
   }

   printk(KERN_INFO "%s called, p_i2c_client->name %s\n", __func__, p_i2c_client->name);

   /* Setup bcmtch com api functions. */
   bcmtch_com_set_interface(BCMTCH_COM_I2C_INTERFACE);

   p_plat_data = (struct bcm915500_platform_data *)p_i2c_client->dev.platform_data;

   if (p_plat_data == NULL)
   {
      printk(KERN_ERR "%s() error, platform data == NULL\n", __func__);
      ret = -ENODATA;
      goto err_no_mem;
   }

   gp_napa_i2c = kzalloc(sizeof(struct napa_i2c), GFP_KERNEL);
   if (gp_napa_i2c == NULL)
   {
      printk(KERN_ERR "%s: failed to alloc mem.\n", __func__);
      ret = -ENOMEM;
      goto err_no_mem;
   }

   /* Save the platform data passed in. */
//   gp_napa_i2c->p_platform_data = p_plat_data;
   memcpy(&gp_napa_i2c->platform_data, p_plat_data, sizeof(struct bcm915500_platform_data));

   if ((gp_code = kzalloc(40000, GFP_KERNEL)) == NULL)
   {
      printk(KERN_ERR "%s() Cannot alloc memory for code\n", __func__);
      kfree(gp_napa_i2c);
      ret = -ENOMEM;
      goto err_no_mem;
   }

   if ((gp_vector = kzalloc(1000, GFP_KERNEL)) == NULL)
   {
      printk(KERN_ERR "%s() cannot alloc memory for vector\n", __func__);
      kfree(gp_napa_i2c);
      kfree(gp_code);
      ret = -ENOMEM;
      goto err_no_mem;
   }

   if ((gp_data = kzalloc(10000, GFP_KERNEL)) == NULL)
   {
      printk(KERN_ERR "%s() cannot alloc memory for data\n", __func__);
      kfree(gp_napa_i2c);
      kfree(gp_code);
      kfree(gp_vector);
      ret = -ENOMEM;
      goto err_no_mem;
   }

   //i2c_set_clientdata(p_i2c_client, bcmpmu);
   //bcmpmu->dev = &p_i2c_client->dev;
   gp_napa_i2c->p_i2c_client = p_i2c_client;

   /* Configure the second I2C slave address. */
   p_i2c_adapter = i2c_get_adapter(p_plat_data->i2c_adapter_id);

   if (p_i2c_adapter == NULL)
   {
      printk(KERN_ERR "%s() p_i2c_adapter == NULL, adapter_id: 0x%x\n",
             __func__, p_plat_data->i2c_adapter_id);
      ret = -ENODEV;
      goto err_no_device;
   }
   else
   {
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() p_i2c_adapter != NULL, adapter_id: 0x%x\n",
               __func__, p_plat_data->i2c_adapter_id);)
   }

   /*
    * i2c_new_device(struct i2c_adapter *adap,
    *                struct i2c_board_info const *info);
    */
   p_new_i2c_client = i2c_new_device(p_i2c_adapter,
                                     &bcm915500_i2c_boardinfo);

   if (p_new_i2c_client == NULL)
   {
      printk(KERN_ERR "%s() p_new_i2c_client == NULL, slave address: 0x%x\n",
             __func__, bcm915500_i2c_boardinfo.addr);
      ret = -ENODEV;
      goto err_no_device;
   }
   else
   {
      DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
         printk("%s() p_new_i2c_client != NULL, slave address: 0x%x\n",
               __func__, bcm915500_i2c_boardinfo.addr);)
   }

#if NAPA_GPIO_RESET_SUPPORT
    if (p_plat_data->gpio_reset >= 0)
    {
        ret = gpio_request(p_plat_data->gpio_reset, "napa i2c ts reset");
        if (ret < 0)
        {
            printk("ERROR: %s() line %d - Unable to request GPIO pin %d\n", __func__, __LINE__, p_plat_data->gpio_reset);
            goto err_free_dev;
        }
        gpio_direction_output(p_plat_data->gpio_reset, 1);
        gpio_set_value(p_plat_data->gpio_reset, 1);
        msleep(250);
        gpio_set_value(p_plat_data->gpio_reset, 0);
        msleep(250);
        gpio_set_value(p_plat_data->gpio_reset, 1);
     }

     msleep(1000);
#endif
   gp_napa_i2c->p_i2c_client1 = p_new_i2c_client;
   mutex_init(&gp_napa_i2c->i2c_mutex);
   mutex_init(&gp_napa_i2c->mutex_wq);


   platform_device_register(&napa_core_device);

   ret = bcmtch_i2c_firmware_load(gp_napa_i2c, BCM915500_TSC_FW_VECT, VECTORS_OFFSET); /* Vectors */
   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() bcmtch_i2c_firmware_load() returned %d\n", __func__, ret);)

   if (ret != 0)
   {
      printk("%s() Vector download failed\n", __func__);
      ret = ENOENT;
      goto err_fw_failed;
   }

   ret = bcmtch_i2c_firmware_load(gp_napa_i2c, BCM915500_TSC_FW_DATA, DATA_OFFSET); /* Data */
   if (ret != 0)
   {
      printk("%s() Data download failed\n", __func__);
      ret = ENOENT;
      goto err_fw_failed;
   }

   ret = bcmtch_i2c_firmware_load(gp_napa_i2c, BCM915500_TSC_FW_CODE, ROM_OFFSET); /* Firmware */
   if (ret != 0)
   {
      printk("%s() Code download failed\n", __func__);
      ret = ENOENT;
      goto err_fw_failed;
   }

   DEBUG(if (mod_debug & MOD_DEBUG_I2C_DOWNLOAD)
      printk("%s() downloads successful, vector size: %d, data size: %d firmware size: %d\n",
             __func__, g_vector_size, g_data_size, g_code_size);)

   if ((ret = bcmtch_init_hw(gp_napa_i2c)) != 0)
   {
      printk(KERN_ERR "%s() bcmtch_init_hw() failed, rc = %d\n", __func__, ret);
      ret = ENODEV;
      DEBUG(if (!(mod_debug & MOD_DEBUG_IGNORE_ERR)))
      goto err_init_failed;
   }

#if ADD_INTERRUPT_HANDLING
   /* Reserve GPIO for touchscreen event interrupt. */
   ret = gpio_request(p_plat_data->gpio_interrupt, "napa i2c ts event");
   if (ret < 0)
   {
      printk(KERN_ERR "WARNING: Unable to request gpio=%d\n", p_plat_data->gpio_interrupt);
      DEBUG(if (!(mod_debug & MOD_DEBUG_IGNORE_ERR)))
      goto err_no_device;
   }

   gpio_direction_input(p_plat_data->gpio_interrupt);

   /* Reserve the irq line. */
   ret = request_irq(p_i2c_client->irq,
                     bcmtch_i2c_interrupt_handler,
                     IRQF_TRIGGER_FALLING,
                     p_i2c_client->name,
                     gp_napa_i2c);
   if (ret)
   {
      printk(KERN_ERR "%s() WARNING request_irq(%d) failed\n", __func__, p_i2c_client->irq);
      ret = -EBUSY;
      DEBUG(if (!(mod_debug & MOD_DEBUG_IGNORE_ERR)))
      goto err_free_dev;
   }
#else
   bcmtch_setup_poll_timer(gp_napa_i2c, 2);
#endif

   gp_napa_i2c->p_ktouch_wq = create_workqueue("napa_touch_wq");

   if (gp_napa_i2c->p_ktouch_wq == NULL)
   {
      printk(KERN_ERR "%s() Unable to create workqueue\n", __func__);
      ret = -ENOMEM;
      DEBUG(if (!(mod_debug & MOD_DEBUG_IGNORE_ERR)))
      goto err_free_dev;
   }

   INIT_WORK(&gp_napa_i2c->work, bcmtch_i2c_wq);

   /* Store the gp_napa_i2c structure so it can be retrieved. */
   i2c_set_clientdata(p_i2c_client, gp_napa_i2c);

   ret = sysfs_create_group(&p_i2c_client->dev.kobj, &bcmtch_attr_group);
   if (ret)
   {
      printk(KERN_ERR "%s() Unable to create group\n", __func__);
      DEBUG(if (!(mod_debug & MOD_DEBUG_IGNORE_ERR)))
      goto err_free_dev;
   }

   printk(KERN_INFO "%s() exiting, SUCCESS!\n", __func__);

   DEBUG(
   if (mod_debug & MOD_DEBUG_LOOP) {
       while(1) {
           reg_value = bcmtch_i2c_read_reg(NAPA_I2C_SLAVE_ADDR, 0x40);
           printk("%s() Slave Addr: %02x Reg: %02x Value: %02x \n", __func__, NAPA_I2C_SLAVE_ADDR, 0x40, reg_value);
       }
   })

   /* Free the memory used to store the firmware files. */
   kfree(gp_code);
   kfree(gp_vector);
   kfree(gp_data);
   return ret;

err_free_dev:
err_init_failed:

err_fw_failed:

err_no_device:
   kfree(gp_napa_i2c);
   kfree(gp_code);
   kfree(gp_vector);
   kfree(gp_data);

err_no_mem:
   return ret;
}

static int bcmtch_i2c_remove(struct i2c_client *p_i2c_client)
{
   //struct bcmpmu *bcmpmu = i2c_get_clientdata(p_i2c_client);

   platform_device_unregister(&napa_core_device);
   //kfree(bcmpmu->accinfo);
   //kfree(bcmpmu);

   return 0;
}

static const struct i2c_device_id napa_i2c_id[] = {
       { BCM915500_TSC_NAME, 0 },
       { }
};
MODULE_DEVICE_TABLE(i2c, napa_i2c_id);


static struct i2c_driver bcmtch_i2c_driver = {
   .driver = {
         .name = BCM915500_TSC_NAME,
         .owner = THIS_MODULE,
   },
   .probe = bcmtch_i2c_probe,
   .remove = bcmtch_i2c_remove,
   .id_table = napa_i2c_id,
};

static int __init bcmtch_i2c_init(void)
{

   pInputDev = bcmtch_allocate_input_dev();

   return i2c_add_driver(&bcmtch_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(bcmtch_i2c_init);

static void __exit bcmtch_i2c_exit(void)
{
   i2c_del_driver(&bcmtch_i2c_driver);
}
module_exit(bcmtch_i2c_exit);


static ssize_t bcmtch_cli_fcn(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

    uint32_t      in_reg;
    uint32_t      in_addr;
    uint32_t      in_value;
    uint32_t      in_count;
    uint32_t      len;
    uint32_t      index;
    unsigned char uch;

    printk("%s() line %d buf=%s count=%d\n", __func__, __LINE__, buf, count);

    if (sscanf(buf, "poke ahb %x %x", &in_addr, &in_value)) {

        //int bcmtch_com_write_mem( int ahb_addr, int len, void *data)     // port note, WriteAHBI2C ahb_addr was DWORD

        len=4;
        bcmtch_com_write_mem(in_addr, len, &in_value);
        printk("%s() line %d Poke AHB Addr=%08x Data=%08x\n", __func__, __LINE__, in_addr, in_value);


    }else if (sscanf(buf, "peek ahb %x %x", &in_addr, &in_count)) {

        //int bcmtch_com_read_mem(int ahb_addr, int len, void *data)     /* port note, ReadAHBI2C ahb_addr was DWORD */

        for (index = 0 ; index < in_count ; ++index) {
            len=4;
            bcmtch_com_read_mem(in_addr, len, &in_value);
            printk("%s() line %d Peek AHB Addr=%08x Data=%08x\n", __func__, __LINE__, in_addr, in_value);
            in_addr += 4;
        }

    }else if (sscanf(buf, "poke spm %x %x", &in_reg, &in_value)) {

        uch = (unsigned char)in_value;
        bcmtch_com_write_reg(BCM_TSC_SPM_SLAVE, in_reg, uch);
        printk("%s() line %d Poke SPM Reg=%08x Data=%08x\n", __func__, __LINE__, in_reg, in_value);

    }else if (sscanf(buf, "peek spm %x %x", &in_reg, &in_count)) {

        for (index = 0 ; index < in_count ; ++index) {
            uch = bcmtch_com_read_reg(BCM_TSC_SPM_SLAVE, in_reg);
            in_value = (uint32_t)uch;
            printk("%s() line %d Peek SPM Reg=%08x Data=%08x\n", __func__, __LINE__, in_reg, in_value);
            in_reg += 1;
        }

    }else if (sscanf(buf, "debug %x", &in_value)) {
        mod_debug = in_value;
        printk("%s() line %d debug %08x\n", __func__, __LINE__, mod_debug );
    }

    return count;
}

MODULE_DESCRIPTION("I2C support for BCM915500 Touchscreen");
MODULE_LICENSE("GPL");


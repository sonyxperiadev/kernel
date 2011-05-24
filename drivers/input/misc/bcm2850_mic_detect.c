/*************************************************************************
* Copyright 2010  Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2 (the GPL),
* available at http://www.broadcom.com/licenses/GPLv2.php, with the following
* added to such license:
* As a special exception, the copyright holders of this software give you
* permission to link this software with independent modules, and to copy and
* distribute the resulting executable under terms of your choice, provided that
* you also meet, for each linked independent module, the terms and conditions
* of the license of that module. An independent module is a module which is not
* derived from this software.  The special exception does not apply to any
* modifications of the software.
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <linux/hrtimer.h>
#include <asm/io.h>

#include <linux/i2c/bcm2850_mic_detect.h>
#include <mach/rdb/brcm_rdb_aci.h>
#include <mach/rdb/brcm_rdb_auxmic.h>


/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
struct i2c_priv_data 
{
   struct i2c_client *p_i2c_client;
};   

struct mic_det_dev {
   struct workqueue_struct *mic_det_wq;
   struct work_struct work;
   struct mutex mutex_wq;
   struct i2c_client *client;
};

#define MIC_DET_ERR(fmt, args...)      \
                           printk(KERN_ERR "[mic-detect]: " fmt, ## args)
#define MIC_DET_DEBUG(fmt, args...)    \
                           printk(KERN_DEBUG "[mic-detect]: " fmt, ## args)


const struct MIC_DET_t        *gp_i2c_para        = NULL;

static struct mic_det_dev *p_mic_det_dev;

static DECLARE_WAIT_QUEUE_HEAD(g_event_waitqueue);

/* ---- Private Function Prototypes -------------------------------------- */
static int  i2c_driver_reg_read(unsigned short regoff, unsigned short *regval);
static int  i2c_driver_reg_write(unsigned short regoff, unsigned short regval);

/* ---- Functions -------------------------------------------------------- */

#ifdef CONFIG_PM
/* In preparation for implementing PM_SUSPEND_STANDBY. */
static int mic_det_suspend_driver(struct i2c_client *p_client,
                                  pm_message_t mesg)
{  /* Can put it into deep sleep only if the slave can be reset to bring it out. */
   disable_irq(gp_i2c_para->comp1_irq);
   return 0;
}

static int mic_det_resume_driver(struct i2c_client *p_client)
{
   enable_irq(gp_i2c_para->comp1_irq);
   return 0;
}
#endif

static void i2c_set_codec_headset(void)
{
   i2c_driver_reg_write(0, 0);
   // Disable REG_SYNC, i.e. Register Writes are not Sync'd with SYSCLK
   i2c_driver_reg_write(0x101, 0x4);
   // Turn on VMID using anti-pop features
   i2c_driver_reg_write(0x39, 0x6c);
   // Turn on VMID using anti-pop features
   i2c_driver_reg_write(0x1, 0x3);
   mdelay(15);

   //set AIF1
   //set master mode
   i2c_driver_reg_write(0x302, 0x4000);
   // Left (mono microphone) ADC output on both channels, no tdm, 16-bit, dsp mode b
   i2c_driver_reg_write(0x300, 0x98);
   // bclk1 = aif1clk / 4 (for 44.1k / 48k)
   i2c_driver_reg_write(0x303, 0x40);
   // aif1clk enabled, fll1 source
   i2c_driver_reg_write(0x200, 0x11);
    // Enable the DSP Mixer Core Clock and DSP Processing Clock for AIF1
   i2c_driver_reg_write(0x208, 0xa);

   //set Playback
   //Configure the AIF1DAC1L/R path to DAC1L/R
   i2c_driver_reg_write(0x601, 0x1);
   i2c_driver_reg_write(0x602, 0x1);
   //# Disable the DAC Mute in the DAC Digital Volume - DAC1
   i2c_driver_reg_write(0x610, 0x1c0);
   i2c_driver_reg_write(0x611, 0x1c0);
   //# Enable the DAC128OSR for High DAC SNR performance
   i2c_driver_reg_write(0x620, 0x1);
   //# Configure the Headphone PGA gain
   i2c_driver_reg_write(0x1d, 0x15f);
   // Enable DAC1L/R
   i2c_driver_reg_write(0x5, 0x303);
   // # Enable the DAC1L/R 'direct' path to the Headphone PGA
   i2c_driver_reg_write(0x2d, 0x100);
   i2c_driver_reg_write(0x2e, 0x100);
   // Enable the Charge Pump (This needs to be done before
   // volume updates on Headphone PGA)
   i2c_driver_reg_write(0x4c, 0x8000);
   mdelay(4);

   //# Configure the Headphone PGA gain
   i2c_driver_reg_write(0x1c, 0x5f);
   //# Enable the HPOUT1L/R Output Stage
   i2c_driver_reg_write(0x1, 0x303);
   //# Enable the Headphone Outputs and remove the Headphone Gnd clamp
   i2c_driver_reg_write(0x60, 0xee);
   //# Disable the AIF1DAC1 Interface Mute
   i2c_driver_reg_write(0x420, 0);
   // GPIO1 defaults to ADCLRCLK1, which causes LRCLK1 to only output
   // on DAC input, and not on ADC output. Setting GPIO1 to any other mode 
   // causes LRCLK1 to output in both modes, as required, so we set GPIO1 to 
   // be a GPIO, with other settings left as the default (input with de-bounce).
   i2c_driver_reg_write(0x700, 0x8101);

   //# Setup bit per sample (reg 0x300 = 0x98)
   //#  FLL (reg 0x220) is enabled when setup ate
   //#  Disable clock from FLL1 to AIF1
   i2c_driver_reg_write(0x200, 0x0);
   //#  fracn_ena, FLL1_disa
   i2c_driver_reg_write(0x220, 0x4);
   //# pre-divider = 1,  FLL1 source: MCLK1
   i2c_driver_reg_write(0x224, 0x0);
   //  # N=187(0bbh)
   i2c_driver_reg_write(0x223, 0x1760);
   //# K=0.5(8000h)
   i2c_driver_reg_write(0x222, 0x8000); 
   //# FLL1_OUTDIV: 8, FLL1_FRATIO: 8
   i2c_driver_reg_write(0x221, 0x704);
   //#  fracn_ena, FLL1_ENA
   i2c_driver_reg_write(0x220, 0x5);
   mdelay(3);

   //#BCLK = AIF1CLK / 8 = 1536K
   i2c_driver_reg_write(0x303, 0x40);
   //#48K sample rate, AIF1CLK / fs ratio 256
   i2c_driver_reg_write(0x210, 0x83);
   //# Enable clock from FLL1 to AIF1
   i2c_driver_reg_write(0x200, 0x11);
   MIC_DET_DEBUG("mic_det_wq -inserted\n");
}

static void i2c_set_codec_speaker(void)
{
   i2c_driver_reg_write(0, 0);
   // Disable REG_SYNC, i.e. Register Writes are not Sync'd with SYSCLK
   i2c_driver_reg_write(0x101, 0x4);
   // Turn on VMID using anti-pop features
   i2c_driver_reg_write(0x39, 0x6c);
   // Turn on VMID using anti-pop features
   i2c_driver_reg_write(0x1, 0x3);
   mdelay(15);

   //set AIF1
   //set master mode
   i2c_driver_reg_write(0x302, 0x4000);
   // Left (mono microphone) ADC output on both channels, no tdm,
   // 16-bit, dsp mode b
   i2c_driver_reg_write(0x300, 0x98);
   // bclk1 = aif1clk / 4 (for 44.1k / 48k)
   i2c_driver_reg_write(0x303, 0x40);
   // aif1clk enabled, fll1 source
    i2c_driver_reg_write(0x200, 0x11);
    // Enable the DSP Mixer Core Clock and DSP Processing Clock for AIF1
   i2c_driver_reg_write(0x208, 0xa);

   //set Playback
   //Configure the AIF1DAC1L/R path to DAC1L/R
    i2c_driver_reg_write(0x601, 0x1);
   i2c_driver_reg_write(0x602, 0x1);
   //# Disable the DAC Mute in the DAC Digital Volume - DAC1
   i2c_driver_reg_write(0x610, 0x1c0);
   i2c_driver_reg_write(0x611, 0x1c0);
   //# Enable the DAC128OSR for High DAC SNR performance
   i2c_driver_reg_write(0x620, 0x1);
   // Enable DAC1L/R
   i2c_driver_reg_write(0x5, 0x303);
   mdelay(4);

   //# Enable Speakers
   i2c_driver_reg_write(0x1, 0x3003);
   // Disable the AIF1DAC1 Interface Mute
   i2c_driver_reg_write(0x420, 0);
   // GPIO1 defaults to ADCLRCLK1, which causes LRCLK1 to only output on
   // DAC input, and not on ADC output. Setting GPIO1 to any other mode
   // causes LRCLK1 to output in both modes, as required, so we set GPIO1
   // to be a GPIO, with other settings left as the default
   // (input with de-bounce).
   i2c_driver_reg_write(0x700, 0x8101);
   //# Unmute left speaker mixer vol
   i2c_driver_reg_write(0x22, 0);
   //# Unmute right speaker mixer vol
   i2c_driver_reg_write(0x23, 0);
   //# Unmute DAC1L/R to SPKMIXL/R
   i2c_driver_reg_write(0x36, 0x3);
   // Unmute SPKMIXL_TO_SPKOUTL and SPKMIXR_TO_SPKOUTL
   i2c_driver_reg_write(0x24, 0x18);
   //# Set SPKOUTL_BOOST to max value
   //#i2c_driver_reg_write( 0x25, 0x3f);
   //# Setup bit per sample (reg 0x300 = 0x98)
   //#  FLL (reg 0x220) is enabled when setup ate
   //#  Disable clock from FLL1 to AIF1
   i2c_driver_reg_write(0x200, 0x0);
   //#  fracn_ena, FLL1_disa
   i2c_driver_reg_write(0x220, 0x4);
   //# pre-divider = 1,  FLL1 source: MCLK1
   i2c_driver_reg_write(0x224, 0x0);
   // # N=187(0bbh)
   i2c_driver_reg_write(0x223, 0x1760);
   //# K=0.5(8000h)
   i2c_driver_reg_write(0x222, 0x8000); 
   //# FLL1_OUTDIV: 8, FLL1_FRATIO: 8
   i2c_driver_reg_write(0x221, 0x704);
   //#  fracn_ena, FLL1_ENA
   i2c_driver_reg_write(0x220, 0x5);
   mdelay(3);

   //#BCLK = AIF1CLK / 8 = 1536K
   i2c_driver_reg_write(0x303, 0x40);
   //#48K sample rate, AIF1CLK / fs ratio 256
   i2c_driver_reg_write(0x210, 0x83);
   //# Enable clock from FLL1 to AIF1
   i2c_driver_reg_write(0x200, 0x11);

   MIC_DET_DEBUG("mic_det_wq -removed\n");
}

static void mic_det_wq(struct work_struct *work)
{
   struct mic_det_dev *codec_dev = container_of(work, struct mic_det_dev, work);
   static int init = 0;
   int tout = 200;

   mutex_lock(&codec_dev->mutex_wq);
   MIC_DET_DEBUG("mic_det_wq run\n");

   if(!init) {
      int tout = 100;
      /* Wait maximum 100ms for ACI_COMP_DOUT_COMP1_DOUT_CMD_ONE bit go low
        * to decide if the headset is connected in the very 1st time.
        * If it does not go low in 100ms, we think it is not going to go low since the headset
        * is connected */
      while(tout) {
         if(readl(gp_i2c_para->reg_base + ACI_COMP_DOUT_OFFSET) &
            ACI_COMP_DOUT_COMP1_DOUT_CMD_ONE) {
            mdelay(1);
            tout--;
         }
         else {
            init = 1;
            break;
         }
      }
   }

   if(readl(gp_i2c_para->reg_base + ACI_COMP_DOUT_OFFSET) &
                        ACI_COMP_DOUT_COMP1_DOUT_CMD_ONE) {
      i2c_set_codec_headset();
   }
   else {
      i2c_set_codec_speaker();
   }
   schedule();

   MIC_DET_DEBUG("mic_det_wq leave\n");
   mutex_unlock(&codec_dev->mutex_wq);
}

static irqreturn_t mic_det_driver_isr(int irq, void *dev_id)
{
   struct mic_det_dev *det_dev = (struct mic_det_dev *)dev_id;

   MIC_DET_DEBUG("mic_det_driver_isr with irq:%d\n", irq);
   writel(ACI_INT_COMP1INT_STS_MASK | ACI_INT_COMP2INT_STS_MASK |
          ACI_INT_COMP1INT_EN_MASK | ACI_INT_INV_COMP2INT_EN_MASK, 
          gp_i2c_para->reg_base + ACI_INT_OFFSET);

    /* postpone I2C transactions to the workqueue as it may block */
   queue_work(det_dev->mic_det_wq, &det_dev->work);

   return IRQ_HANDLED;   
}

int i2c_driver_reg_read(unsigned short regoff, unsigned short *regval)
{
   int rc = 0;
   unsigned char buf[2];
   
   if (p_mic_det_dev == NULL ||
       p_mic_det_dev->client == NULL)
   {
      MIC_DET_ERR("i2c_driver_reg_read() p_mic_det_dev->client == NULL\n");
      return -1;
   }
   buf[0] = regoff >> 8;
   buf[1] = regoff & 0xFF;
   if((rc = i2c_master_send(p_mic_det_dev->client,  buf,  2)) != 2) {
      MIC_DET_ERR("i2c_driver_reg_read failed -: reg %02x\n", regoff);
      return -1;
   }
   if((rc = i2c_master_recv(p_mic_det_dev->client,  buf,  2)) != 2)  {
      MIC_DET_ERR("i2c_driver_reg_read failed: reg %02x\n", regoff);
      return -1;
   };
   return rc;
}   

int  i2c_driver_reg_write(unsigned short regoff, unsigned short regval)
{
   int rc;
   unsigned char buf[4];

   buf[0] = regoff >> 8;
   buf[1] = regoff & 0xFF;
   buf[2] = regval >> 8;
   buf[3] = regval & 0xFF;
   
   if((rc = i2c_master_send(p_mic_det_dev->client,  buf,  4)) !=4){
      MIC_DET_ERR("i2c_driver_reg_write failed : reg %02x\n", regoff);
      return -1;
   }
   return rc;
}

static int mic_det_driver_probe(struct i2c_client *p_i2c_client,
                               const struct i2c_device_id *id)
{
   int rc = 0;
   
   if (p_i2c_client == NULL)
   {
      MIC_DET_ERR("%s mic_det_driver_probe() p_i2c_client == NULL\n", 
                  MIC_DET_DRIVER_NAME);
      return -1;
   }

   if (p_i2c_client->dev.platform_data == NULL)
   {
      MIC_DET_ERR("%s mic_det_driver_probe() "
                  "p_i2c_client->dev.platform_data == NULL\n",
                  MIC_DET_DRIVER_NAME);      
      return -1;
   }
   
   if (!i2c_check_functionality(p_i2c_client->adapter, I2C_FUNC_I2C))
   {                             
      MIC_DET_ERR("%s: mic_det_driver_probe() "
                  "i2c_check_functionality() failed %d\n", 
                  MIC_DET_DRIVER_NAME, -ENODEV);
      return -ENODEV;
   } 
   

   /* Get the I2C information compiled in for this platform. */   
   gp_i2c_para = (struct MIC_DET_t *)p_i2c_client->dev.platform_data;
   
   if (gp_i2c_para == NULL)
   {  /* Cannot access platform data. */   
      printk("%s:%s Cannot access platform data for I2C slave address %d\n", 
             MIC_DET_DRIVER_NAME, __FUNCTION__, p_i2c_client->addr);
      return -1;
   }    
      
   printk("%s: slave address 0x%x\n", MIC_DET_DRIVER_NAME, p_i2c_client->addr);

   writel(gp_i2c_para->comp1_threshold, 
         gp_i2c_para->reg_base + ACI_COMP1TH_OFFSET);
   writel(ACI_COMPTH_SET_COMPTH1_SET_CMD_LOADING, 
         gp_i2c_para->reg_base + ACI_COMPTH_SET_OFFSET);
   writel(AUXMIC_AUXEN_MICAUX_EN_MASK, 
          gp_i2c_para->reg_base + AUXMIC_AUXEN_OFFSET);
   writel(ACI_ACI_CTRL_SW_MIC_DATAB_MASK, 
          gp_i2c_para->reg_base + ACI_ACI_CTRL_OFFSET);
   writel(ACI_INT_COMP1INT_STS_MASK | ACI_INT_COMP2INT_STS_MASK |
          ACI_INT_COMP1INT_EN_MASK | ACI_INT_INV_COMP2INT_EN_MASK, 
          gp_i2c_para->reg_base + ACI_INT_OFFSET);

   p_mic_det_dev->client = p_i2c_client;
   mutex_init(&p_mic_det_dev->mutex_wq);   
   p_mic_det_dev->mic_det_wq = create_workqueue("mic_det_wq"); 
   INIT_WORK(&p_mic_det_dev->work, mic_det_wq);
   i2c_set_clientdata(p_i2c_client, p_mic_det_dev);
 
   if (rc < 0)
   {  /* This also ensures that the slave is actually there! */
      MIC_DET_ERR("%s mic_det_driver_probe() "
                  "failed to write to slave, rc = %d\n",
                  MIC_DET_DRIVER_NAME, rc);
      return rc;    
   }      
   else
   {
      rc = 0;
   }

   queue_work(p_mic_det_dev->mic_det_wq, &p_mic_det_dev->work);

   if ((rc = request_irq(gp_i2c_para->comp1_irq, 
                         mic_det_driver_isr, (IRQF_SHARED),
                         "MIC dection irq", p_mic_det_dev)) < 0)
   {
      MIC_DET_ERR("request_irq(%d) failed, rc = %d\n",
                  gp_i2c_para->comp1_irq, rc);
   }

   return rc;
}
 
static int __devexit mic_det_driver_remove(struct i2c_client *client)
{
   free_irq(gp_i2c_para->comp1_irq, p_mic_det_dev);
   
   /* Free all the memory that was allocated. */   
   if (p_mic_det_dev->client != NULL)
   {
      kfree(p_mic_det_dev->client);
   }
   
   if (p_mic_det_dev != NULL)
   {
      kfree(p_mic_det_dev);
   }

   return 0;
}

/* End of if using .probe in i2c_driver. */ 

static struct i2c_device_id mic_det_idtable[] = {
   { MIC_DET_DRIVER_NAME, 0 },
   { }
};

static struct i2c_driver mic_det_driver = {
   .driver = {
      .name  = MIC_DET_DRIVER_NAME,
   },   
   .id_table       = mic_det_idtable,
   .class          = I2C_CLASS_HWMON,
   .probe          = mic_det_driver_probe,
   .remove         = __devexit_p(mic_det_driver_remove),
#ifdef CONFIG_PM
   .suspend        = mic_det_suspend_driver,
   .resume         = mic_det_resume_driver,
#endif   
};

int __init mic_det_driver_init(void)
{
   int rc;
   
   p_mic_det_dev = kzalloc(sizeof(struct mic_det_dev), GFP_KERNEL);
   
   printk("%s: mic_det_driver_init() entering ...\n", MIC_DET_DRIVER_NAME);

   if (p_mic_det_dev == NULL)
   {
      printk("mic_det_driver_init(): memory allocation failed for p_codec_i2c_dev!\n");
      return -ENOMEM;
   }       
   
   rc = i2c_add_driver(&mic_det_driver);
   
   if (rc != 0) 
   {
      MIC_DET_ERR("%s mic_det_driver_init(): i2c_add_driver()"
                  "failed, errno is %d\n", MIC_DET_DRIVER_NAME, rc);
      kfree(p_mic_det_dev);
      return rc;
   }

   return rc;
}

static void __exit mic_det_driver_exit(void)
{
	i2c_del_driver(&mic_det_driver);
}

MODULE_DESCRIPTION("Mic-dection driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");

module_init(mic_det_driver_init);
module_exit(mic_det_driver_exit);

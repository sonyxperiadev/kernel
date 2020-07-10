/*
 * Copyright (C) 2010 Trusted Logic S.A.
 * Copyright (C) 2017 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
/******************************************************************************
 *
 *  The original Work has been changed by NXP Semiconductors.
 *
 *  Copyright (C) 2013-2018 NXP Semiconductors
 *   *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <asm/siginfo.h>
#include <linux/rcupdate.h>
#include <linux/version.h>
#include <linux/sched.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
#include <linux/signal.h>
#else
#include <linux/sched/signal.h>
#endif
#include <linux/workqueue.h>

#include <linux/timer.h>
#include "pn553.h"

/* 1 is equivalent to NXP pn55X chip, 0 is equivalent to NXP pn544 chip*/
#define NEXUS5x    1
#define HWINFO     0
#if NEXUS5x
#undef ISO_RST
#else
#define ISO_RST
#endif

#define DRAGON_NFC 1
#define SIG_NFC 44
#define MAX_BUFFER_SIZE 512
#define MAX_SECURE_SESSIONS 1
#define WAKEUP_SRC_TIMEOUT      (2000)
/* Macro added to disable SVDD power toggling */
/* #define JCOP_4X_VALIDATION */

struct pn544_dev    {
    wait_queue_head_t   read_wq;
    struct mutex        read_mutex;
    struct i2c_client   *client;
    struct miscdevice   pn544_device;
    unsigned int        ven_gpio;
    unsigned int        firm_gpio;
    unsigned int        irq_gpio;
    unsigned int        ese_pwr_gpio; /* gpio used by SPI to provide power to p61 via NFCC */
#ifdef ISO_RST
    unsigned int        iso_rst_gpio; /* ISO-RST pin gpio*/
#endif
    p61_access_state_t  p61_current_state; /* stores the current P61 state */
    bool                nfc_ven_enabled; /* stores the VEN pin state powered by Nfc */
    bool                spi_ven_enabled; /* stores the VEN pin state powered by Spi */
    bool                irq_enabled;
    spinlock_t          irq_enabled_lock;
    long                nfc_service_pid; /*used to signal the nfc the nfc service */
    chip_pwr_scheme_t   chip_pwr_scheme;
    unsigned int        secure_timer_cnt;
    unsigned long       dwpLinkUpdateStat; /*DWP link update status*/
    struct workqueue_struct *pSecureTimerCbWq;
    struct work_struct wq_task;
    size_t              tx_kbuf_len;
    u8                  *tx_kbuf; /*buffer for write */
    size_t              rx_kbuf_len;
    u8                  *rx_kbuf; /*buffer for read */
};

static struct pn544_dev *pn544_dev;
static struct semaphore ese_access_sema;
static struct semaphore svdd_sync_onoff_sema;
/*semaphore to wait till JNI operation is completed for SPI on/off*/
static struct completion dwp_onoff_sema;
/* semaphore to lock SPI open request until signal handling is complete */
static struct semaphore dwp_onoff_release_sema;
static struct timer_list secure_timer;
static void release_ese_lock(p61_access_state_t  p61_current_state);
int get_ese_lock(p61_access_state_t  p61_current_state, int timeout);
static long set_jcop_download_state(unsigned long arg);
static long start_seccure_timer(unsigned long timer_value);
static long secure_timer_operation(struct pn544_dev *pn544_dev, unsigned long arg);
#if HWINFO
static void check_hw_info(void);
#endif
#define SECURE_TIMER_WORK_QUEUE "SecTimerCbWq"

static void pn544_disable_irq(struct pn544_dev *pn544_dev)
{
    unsigned long flags;

    spin_lock_irqsave(&pn544_dev->irq_enabled_lock, flags);
    if (pn544_dev->irq_enabled) {
        disable_irq_nosync(pn544_dev->client->irq);
        pn544_dev->irq_enabled = false;
    }
    spin_unlock_irqrestore(&pn544_dev->irq_enabled_lock, flags);
}

static irqreturn_t pn544_dev_irq_handler(int irq, void *dev_id)
{
    struct pn544_dev *pn544_dev = dev_id;

    if (device_may_wakeup(&pn544_dev->client->dev))
        pm_wakeup_event(&pn544_dev->client->dev, WAKEUP_SRC_TIMEOUT);

    pn544_disable_irq(pn544_dev);
    /* Wake up waiting readers */
    wake_up(&pn544_dev->read_wq);


    return IRQ_HANDLED;
}

static ssize_t pn544_dev_read(struct file *filp, char __user *buf,
        size_t count, loff_t *offset)
{
    struct pn544_dev *pn544_dev = filp->private_data;
    int ret;
    u8 *tmp;

    if (count > pn544_dev->tx_kbuf_len)
        count = pn544_dev->tx_kbuf_len;

    //pr_debug("%s : reading   %zu bytes.\n", __func__, count);

    mutex_lock(&pn544_dev->read_mutex);

    if (!gpio_get_value(pn544_dev->irq_gpio)) {
        if (filp->f_flags & O_NONBLOCK) {
            ret = -EAGAIN;
            goto fail;
        }

        while (1) {
            ret = 0;
            if (!pn544_dev->irq_enabled) {
                pn544_dev->irq_enabled = true;
                enable_irq(pn544_dev->client->irq);
            }
            if (!gpio_get_value(pn544_dev->irq_gpio)) {
                ret = wait_event_interruptible(
                        pn544_dev->read_wq,
                        !pn544_dev->irq_enabled);
            }

            if (ret)
                goto fail;

            pn544_disable_irq(pn544_dev);
            if (gpio_get_value(pn544_dev->irq_gpio))
                break;

            pr_warning("%s: spurious interrupt detected\n", __func__);
        }
    }
    tmp = pn544_dev->tx_kbuf;
    memset(tmp, 0, pn544_dev->tx_kbuf_len);
    /* Read data */
    ret = i2c_master_recv(pn544_dev->client, tmp, count);
    mutex_unlock(&pn544_dev->read_mutex);

    /* pn544 seems to be slow in handling I2C read requests
     * so add 1ms delay after recv operation */
#if !NEXUS5x
    udelay(1000);
#endif

    if (ret < 0) {
        pr_err("%s: i2c_master_recv returned %d\n", __func__, ret);
        return ret;
    }
    if (ret > count) {
        pr_err("%s: received too many bytes from i2c (%d)\n",
                __func__, ret);
        return -EIO;
    }
    if (copy_to_user(buf, tmp, ret)) {
        pr_warning("%s : failed to copy to user space\n", __func__);
        return -EFAULT;
    }
    return ret;

    fail:
    mutex_unlock(&pn544_dev->read_mutex);
    return ret;
}

static ssize_t pn544_dev_write(struct file *filp, const char __user *buf,
        size_t count, loff_t *offset)
{
    struct pn544_dev  *pn544_dev;
    int ret;
    u8 *tmp;

    pn544_dev = filp->private_data;

    if (count > pn544_dev->rx_kbuf_len)
        count = pn544_dev->rx_kbuf_len;

    tmp = pn544_dev->rx_kbuf;
    memset(tmp, 0, pn544_dev->rx_kbuf_len);
    if (copy_from_user(tmp, buf, count)) {
        pr_err("%s : failed to copy from user space\n", __func__);
        return -EFAULT;
    }

    //pr_debug("%s : writing %zu bytes.\n", __func__, count);
    /* Write data */
    ret = i2c_master_send(pn544_dev->client, tmp, count);
    if (ret != count) {
        pr_err("%s : i2c_master_send returned %d\n", __func__, ret);
        ret = -EIO;
    }

    /* pn544 seems to be slow in handling I2C write requests
     * so add 1ms delay after I2C send oparation */
    udelay(1000);

    return ret;
}

static void p61_update_access_state(struct pn544_dev *pn544_dev, p61_access_state_t current_state, bool set)
{
    //pr_info("%s: Enter current_state = %x\n", __func__, pn544_dev->p61_current_state);
    if (current_state)
    {
        if(set){
            if(pn544_dev->p61_current_state == P61_STATE_IDLE)
                pn544_dev->p61_current_state = P61_STATE_INVALID;
            pn544_dev->p61_current_state |= current_state;
        }
        else{
            pn544_dev->p61_current_state ^= current_state;
            if(!pn544_dev->p61_current_state)
                pn544_dev->p61_current_state = P61_STATE_IDLE;
        }
    }
    //pr_info("%s: Exit current_state = %x\n", __func__, pn544_dev->p61_current_state);
}

static void p61_get_access_state(struct pn544_dev *pn544_dev, p61_access_state_t *current_state)
{

    if (current_state == NULL) {
        //*current_state = P61_STATE_INVALID;
        //pr_err("%s : invalid state of p61_access_state_t current state  \n", __func__);
    } else {
        *current_state = pn544_dev->p61_current_state;
    }
}

static int signal_handler(p61_access_state_t state, long nfc_pid)
{
    struct siginfo sinfo;
    pid_t pid;
    struct task_struct *task;
    int sigret = 0, ret = 0;
    //pr_info("%s: Enter\n", __func__);

    if(nfc_pid == 0)
    {
        pr_info("nfc_pid is clear don't call signal_handler.\n");
    }
    else
    {
        memset(&sinfo, 0, sizeof(struct siginfo));
        sinfo.si_signo = SIG_NFC;
        sinfo.si_code = SI_QUEUE;
        sinfo.si_int = state;
        pid = nfc_pid;

        task = pid_task(find_vpid(pid), PIDTYPE_PID);
        if(task)
        {
            pr_info("%s.\n", task->comm);
            sigret = send_sig_info(SIG_NFC, &sinfo, task);
            if(sigret < 0){
                pr_info("send_sig_info failed..... sigret %d.\n", sigret);
                ret = -1;
                //msleep(60);
            }
        }
        else{
             pr_info("finding task from PID failed\r\n");
             ret = -1;
        }
    }
    pr_info("%s: Exit ret = %d\n", __func__, ret);
    return ret;
}
static STATUS svdd_sync_onoff(long nfc_service_pid, p61_access_state_t origin)
{
    int timeout = 4500; // 4500 ms timeout
    unsigned long tempJ = msecs_to_jiffies(timeout);
    //pr_info("%s: Enter nfc_service_pid: %ld\n", __func__, nfc_service_pid);
    sema_init(&svdd_sync_onoff_sema, 0);
    if (nfc_service_pid) {
        if (0 == signal_handler(origin, nfc_service_pid)) {
            pr_info("Waiting for svdd protection response");
            if (down_timeout(&svdd_sync_onoff_sema, tempJ) != 0) {
                pr_info("svdd wait protection: Timeout");
                return STATUS_FAILED;
            }
            msleep(10);
            //pr_info("svdd wait protection : released");
        }
    }
    return (pn544_dev->dwpLinkUpdateStat == 0x00) ? STATUS_SUCCESS : STATUS_FAILED;
    //pr_info("%s: Exit\n", __func__);
}
static int release_svdd_wait(void)
{
    //pr_info("%s: Enter \n", __func__);
    up(&svdd_sync_onoff_sema);
    //pr_info("%s: Exit\n", __func__);
    return 0;
}

static STATUS dwp_OnOff(long nfc_service_pid, p61_access_state_t origin)
{
    int timeout = 4500; // 4500 ms timeout
    unsigned long tempJ = msecs_to_jiffies(timeout);
    init_completion(&dwp_onoff_sema);
    if (nfc_service_pid) {
        if (0 == signal_handler(origin, nfc_service_pid)) {
            if (wait_for_completion_timeout(&dwp_onoff_sema, tempJ) == 0) {
                pr_info("Dwp On/off wait protection: Timeout");
                return STATUS_FAILED;
            }
            //pr_info("Dwp On/Off wait protection : released");
        }
    }
    return (pn544_dev->dwpLinkUpdateStat == 0x00) ? STATUS_SUCCESS : STATUS_FAILED;
}
static int release_dwpOnOff_wait(void)
{
    int timeout = 500; // 500 ms timeout
    unsigned long tempJ = msecs_to_jiffies(timeout);
    pr_info("%s: Enter \n", __func__);
    complete(&dwp_onoff_sema);
    {
        sema_init(&dwp_onoff_release_sema, 0);
        /*release JNI only after all the SPI On related actions are completed*/
        if (down_timeout(&dwp_onoff_release_sema, tempJ) != 0) {
        //pr_info("Dwp On/off release wait protection: Timeout");
        }
    //pr_info("Dwp On/Off release wait protection : released");
    }
  return 0;
}

static int pn544_dev_open(struct inode *inode, struct file *filp)
{
    struct pn544_dev *pn544_dev = container_of(filp->private_data,
            struct pn544_dev,
            pn544_device);

    filp->private_data = pn544_dev;

    pr_debug("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));

    return 0;
}

static int set_nfc_pid(unsigned long arg)
{
    //pr_info("%s : The NFC Service PID is %ld\n", __func__, arg);
    pn544_dev->nfc_service_pid = arg;
    return 0;
}

long  pn544_dev_ioctl(struct file *filp, unsigned int cmd,
        unsigned long arg)
{
    pr_info("%s :enter cmd = %u, arg = %ld\n", __func__, cmd, arg);

    /* Free pass autobahn area, not protected. Use it carefullly. START */
    switch(cmd)
    {
        case P544_GET_ESE_ACCESS:
            return get_ese_lock(P61_STATE_WIRED, arg);
        break;
        case P544_REL_SVDD_WAIT:
            pn544_dev->dwpLinkUpdateStat = arg;
            return release_svdd_wait();
        break;
        case P544_SET_NFC_SERVICE_PID:
            return set_nfc_pid(arg);
        break;
        case P544_REL_DWPONOFF_WAIT:
           pn544_dev->dwpLinkUpdateStat = arg;
            return release_dwpOnOff_wait();
        break;
        default:
        break;
    }
    /* Free pass autobahn area, not protected. Use it carefullly. END */

    switch (cmd) {
    case PN544_SET_PWR:
    {
        p61_access_state_t current_state = P61_STATE_INVALID;
        p61_get_access_state(pn544_dev, &current_state);
        if (arg == 2) {
            if (current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO) && (pn544_dev->chip_pwr_scheme != PN80T_EXT_PMU_SCHEME))
            {
                /* NFCC fw/download should not be allowed if p61 is used
                 * by SPI
                 */
                pr_info("%s NFCC should not be allowed to reset/FW download \n", __func__);
                return -EBUSY; /* Device or resource busy */
            }
            pn544_dev->nfc_ven_enabled = true;
            if ((pn544_dev->spi_ven_enabled == false && !(pn544_dev->secure_timer_cnt))
            || (pn544_dev->chip_pwr_scheme == PN80T_EXT_PMU_SCHEME))
            {
                /* power on with firmware download (requires hw reset)
                 */
                pr_info("%s power on with firmware\n", __func__);
                gpio_set_value(pn544_dev->ven_gpio, 1);
                msleep(10);
                if (pn544_dev->firm_gpio) {
                    p61_update_access_state(pn544_dev, P61_STATE_DWNLD, true);
                    gpio_set_value(pn544_dev->firm_gpio, 1);
                }
                msleep(10);
                gpio_set_value(pn544_dev->ven_gpio, 0);
                msleep(10);
                gpio_set_value(pn544_dev->ven_gpio, 1);
                msleep(10);
            }
        } else if (arg == 1) {
            /* power on */
            pr_info("%s power on\n", __func__);
            if (pn544_dev->firm_gpio) {
                if ((current_state & (P61_STATE_WIRED|P61_STATE_SPI|P61_STATE_SPI_PRIO))== 0){
                    p61_update_access_state(pn544_dev, P61_STATE_IDLE, true);
                }
                if(current_state & P61_STATE_DWNLD){
                    p61_update_access_state(pn544_dev, P61_STATE_DWNLD, false);
                }
                gpio_set_value(pn544_dev->firm_gpio, 0);
            }

            pn544_dev->nfc_ven_enabled = true;
            if (pn544_dev->spi_ven_enabled == false || (pn544_dev->chip_pwr_scheme == PN80T_EXT_PMU_SCHEME)) {
                gpio_set_value(pn544_dev->ven_gpio, 1);
            }
        } else if (arg == 0) {
            /* power off */
            pr_info("%s power off\n", __func__);
            if (pn544_dev->firm_gpio) {
                if ((current_state & (P61_STATE_WIRED|P61_STATE_SPI|P61_STATE_SPI_PRIO))== 0){
                    p61_update_access_state(pn544_dev, P61_STATE_IDLE, true);
                }
                gpio_set_value(pn544_dev->firm_gpio, 0);
            }

            pn544_dev->nfc_ven_enabled = false;
            /* Don't change Ven state if spi made it high */
            if ((pn544_dev->spi_ven_enabled == false && !(pn544_dev->secure_timer_cnt))
            || (pn544_dev->chip_pwr_scheme == PN80T_EXT_PMU_SCHEME)) {
                gpio_set_value(pn544_dev->ven_gpio, 0);
            }
        } else if (arg == 3) {
            /*NFC Service called ISO-RST*/
            p61_access_state_t current_state = P61_STATE_INVALID;
            p61_get_access_state(pn544_dev, &current_state);
            if(current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO)) {
                return -EPERM; /* Operation not permitted */
            }
            if(current_state & P61_STATE_WIRED) {
                p61_update_access_state(pn544_dev, P61_STATE_WIRED, false);
            }
#ifdef ISO_RST
            gpio_set_value(pn544_dev->iso_rst_gpio, 0);
            msleep(50);
            gpio_set_value(pn544_dev->iso_rst_gpio, 1);
            msleep(50);
            pr_info("%s ISO RESET from DWP DONE\n", __func__);
#endif
        } else if (arg == 4) {
            pr_info("%s FW dwldioctl called from NFC \n", __func__);
            /*NFC Service called FW dwnld*/
            if (pn544_dev->firm_gpio) {
                p61_update_access_state(pn544_dev, P61_STATE_DWNLD, true);
                gpio_set_value(pn544_dev->firm_gpio, 1);
                msleep(10);
            }
        }
        else {
            pr_err("%s bad arg %lu\n", __func__, arg);
            /* changed the p61 state to idle*/
            return -EINVAL;
        }
    }
    break;
    case P61_SET_SPI_PWR:
    {
        p61_access_state_t current_state = P61_STATE_INVALID;
        bool isSignalTriggerReqd = !(arg & 0x10);
        unsigned long pwrLevel = arg & 0x0F;
        p61_get_access_state(pn544_dev, &current_state);
        /*For chipType pn557 (5th bit of arg) do not trigger signal*/
        if (pwrLevel == 1) {
            //pr_info("%s : PN61_SET_SPI_PWR - power on ese\n", __func__);
            if (((current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO)) == 0) || (current_state & P61_STATE_SPI_FAILED))
            {
                /*To handle triple mode protection signal
                NFC service when SPI session started*/
                if (isSignalTriggerReqd && !(current_state & P61_STATE_JCP_DWNLD)){
                    if(pn544_dev->nfc_service_pid){
                        //pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                        STATUS stat = dwp_OnOff(pn544_dev->nfc_service_pid, P61_STATE_SPI);
                        if(stat != STATUS_SUCCESS) {
                            pr_info(" %s DWP link activation failed. Returning..", __func__);
                            p61_update_access_state(pn544_dev, P61_STATE_SPI_FAILED, true);
                            return stat;
                        }
                    }
                    else{
                        pr_info(" invalid nfc service pid....signalling failed%s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                    }
                }

                pn544_dev->spi_ven_enabled = true;

                if(pn544_dev->chip_pwr_scheme == PN80T_EXT_PMU_SCHEME)
                    break;

                if (pn544_dev->nfc_ven_enabled == false)
                {
                    /* provide power to NFCC if, NFC service not provided */
                    gpio_set_value(pn544_dev->ven_gpio, 1);
                    msleep(10);
                }
                /* pull the gpio to high once NFCC is power on*/
                // gpio_set_value(pn544_dev->ese_pwr_gpio, 1);

                /* Delay (10ms) after SVDD_PWR_ON to allow JCOP to bootup (5ms jcop boot time + 5ms guard time) */
                // usleep_range(10000, 12000);
                if(current_state & P61_STATE_SPI_FAILED){
                    p61_update_access_state(pn544_dev, P61_STATE_SPI_FAILED, false);
                }
                p61_update_access_state(pn544_dev, P61_STATE_SPI, true);
                if (pn544_dev->nfc_service_pid) {
                  up(&dwp_onoff_release_sema);
                }

            } else if ((current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO))
                 && (gpio_get_value(pn544_dev->ese_pwr_gpio)) && (gpio_get_value(pn544_dev->ven_gpio))) {
                /* Returning success if SET_SPM_POWER called while already SPI is open */
                   return 0;
            } else {
                pr_info("%s : PN61_SET_SPI_PWR -  power on ese failed \n", __func__);
                return -EBUSY; /* Device or resource busy */
            }
        } else if (pwrLevel == 0) {
            //pr_info("%s : PN61_SET_SPI_PWR - power off ese\n", __func__);
            if(current_state & P61_STATE_SPI_PRIO){
                p61_update_access_state(pn544_dev, P61_STATE_SPI_PRIO, false);
                if (!(current_state & P61_STATE_JCP_DWNLD))
                {
                    if(pn544_dev->nfc_service_pid){
                        //pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                        if(!(current_state & P61_STATE_WIRED))
                        {
                            svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START |
                                                     P61_STATE_SPI_PRIO_END);
                        }else {
                            signal_handler(P61_STATE_SPI_PRIO_END, pn544_dev->nfc_service_pid);
                        }
                    }
                    else{
                        pr_info(" invalid nfc service pid....signalling failed%s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                    }
                } else if (!(current_state & P61_STATE_WIRED)) {
                    svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START);
                }
                pn544_dev->spi_ven_enabled = false;

                if(pn544_dev->chip_pwr_scheme == PN80T_EXT_PMU_SCHEME)
                    break;

                /* if secure timer is running, Delay the SPI close by 25ms after sending End of Apdu to enable eSE go into DPD
                    gracefully (20ms after EOS + 5ms DPD settlement time) */
                if(pn544_dev->secure_timer_cnt)
                    usleep_range(25000, 30000);

                if (!(current_state & P61_STATE_WIRED) && !(pn544_dev->secure_timer_cnt))
                {
#ifndef JCOP_4X_VALIDATION
                    // gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
                    /* Delay (2.5ms) after SVDD_PWR_OFF for the shutdown settlement time */
                    // usleep_range(2500, 3000);
#endif
                    svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_END);
                }
#ifndef JCOP_4X_VALIDATION
                if ((pn544_dev->nfc_ven_enabled == false) && !(pn544_dev->secure_timer_cnt)) {
                     gpio_set_value(pn544_dev->ven_gpio, 0);
                     msleep(10);
                 }
#endif
              }else if((current_state & P61_STATE_SPI) || (current_state & P61_STATE_SPI_FAILED)){
                  if (!(current_state & P61_STATE_WIRED) &&
                      (pn544_dev->chip_pwr_scheme != PN80T_EXT_PMU_SCHEME) &&
                      !(current_state & P61_STATE_JCP_DWNLD))
                  {
                    if(isSignalTriggerReqd) {
                        if(pn544_dev->nfc_service_pid){
                            //pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                          STATUS stat = svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START | P61_STATE_SPI_END);
                          if(stat != STATUS_SUCCESS) {
                          pr_info(" %s DWP link deactivation Failed. Returning..", __func__);
                          p61_update_access_state(pn544_dev, P61_STATE_SPI_FAILED, true);
                          return stat;
                          }
                         }
                        else{
                             pr_info(" invalid nfc service pid....signalling failed%s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                        }
                   }

                       /* if secure timer is running, Delay the SPI close by 25ms after sending End of Apdu to enable eSE go into DPD
                            gracefully (20ms after EOS + 5ms DPD settlement time) */
                       if(pn544_dev->secure_timer_cnt)
                            usleep_range(25000, 30000);

                      if (!(pn544_dev->secure_timer_cnt)) {
#ifndef JCOP_4X_VALIDATION
                          // gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
                          /* Delay (2.5ms) after SVDD_PWR_OFF for the shutdown settlement time */
                          // usleep_range(2500, 3000);
#endif
                          if(current_state & P61_STATE_SPI_FAILED) {
                              p61_update_access_state(pn544_dev, P61_STATE_SPI_FAILED, false);
                          }
                          if(current_state & P61_STATE_SPI)
                              p61_update_access_state(pn544_dev, P61_STATE_SPI,
                                                  false);
                          if(isSignalTriggerReqd)
                          svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_END);
                       }
                  }
                  /*If JCOP3.2 or 3.3 for handling triple mode
                  protection signal NFC service */
                  else
                  {
                      if(isSignalTriggerReqd) {
                          if (!(current_state & P61_STATE_JCP_DWNLD))
                          {
                              if(pn544_dev->nfc_service_pid){
                                  //pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                                  if(pn544_dev->chip_pwr_scheme == PN80T_LEGACY_PWR_SCHEME)
                                  {
                                      STATUS stat = svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START | P61_STATE_SPI_END);
                                      if(stat != STATUS_SUCCESS) {
                                          pr_info(" %s DWP link deactivation Failed. Returning..", __func__);
                                          p61_update_access_state(pn544_dev, P61_STATE_SPI_FAILED, true);
                                          return stat;
                                      }
                                  } else {
                                      signal_handler(P61_STATE_SPI_END, pn544_dev->nfc_service_pid);
                                  }
                               }
                               else{
                                   pr_info(" invalid nfc service pid....signalling failed%s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                               }
                          } else if (pn544_dev->chip_pwr_scheme == PN80T_LEGACY_PWR_SCHEME) {
                              svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START);
                          }
                      }
                      if(pn544_dev->chip_pwr_scheme == PN80T_LEGACY_PWR_SCHEME)
                      {
#ifndef JCOP_4X_VALIDATION
                          // gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
#endif
                          if(current_state & P61_STATE_SPI_FAILED){
                              p61_update_access_state(pn544_dev, P61_STATE_SPI_FAILED, false);
                          }
                          if(current_state & P61_STATE_SPI)
                              p61_update_access_state(pn544_dev, P61_STATE_SPI,
                                                  false);
                          if(isSignalTriggerReqd)
                          svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_END);
                          //pr_info("PN80T legacy ese_pwr_gpio off %s", __func__);
                      }
                  }
                  pn544_dev->spi_ven_enabled = false;
                  if (pn544_dev->nfc_ven_enabled == false && (pn544_dev->chip_pwr_scheme != PN80T_EXT_PMU_SCHEME)
                       && !(pn544_dev->secure_timer_cnt)) {
                      gpio_set_value(pn544_dev->ven_gpio, 0);
                      msleep(10);
                  }
            } else {
                pr_err("%s : PN61_SET_SPI_PWR - failed, current_state = %x \n",
                        __func__, pn544_dev->p61_current_state);
                return -EPERM; /* Operation not permitted */
            }
        }else if (arg == 2) {
            pr_info("%s : PN61_SET_SPI_PWR - reset\n", __func__);
            if (current_state & (P61_STATE_IDLE|P61_STATE_SPI|P61_STATE_SPI_PRIO)) {
                if (pn544_dev->spi_ven_enabled == false)
                {
                    pn544_dev->spi_ven_enabled = true;
                    if ((pn544_dev->nfc_ven_enabled == false) && (pn544_dev->chip_pwr_scheme != PN80T_EXT_PMU_SCHEME)) {
                        /* provide power to NFCC if, NFC service not provided */
                        gpio_set_value(pn544_dev->ven_gpio, 1);
                        msleep(10);
                    }
                }
                if(pn544_dev->chip_pwr_scheme != PN80T_EXT_PMU_SCHEME  && !(pn544_dev->secure_timer_cnt))
                {
                    svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START);
#ifndef JCOP_4X_VALIDATION
                    gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
#endif
                    svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_END);
                    msleep(10);
                    if(!gpio_get_value(pn544_dev->ese_pwr_gpio))
                        gpio_set_value(pn544_dev->ese_pwr_gpio, 1);
                    msleep(10);
                }
            } else {
                pr_info("%s : PN61_SET_SPI_PWR - reset  failed \n", __func__);
                return -EBUSY; /* Device or resource busy */
            }
        }else if (arg == 3) {
            pr_info("%s : PN61_SET_SPI_PWR - Prio Session Start power on ese\n", __func__);
            if ((current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO)) == 0) {
                p61_update_access_state(pn544_dev, P61_STATE_SPI_PRIO, true);
                if (current_state & P61_STATE_WIRED){
                    if(pn544_dev->nfc_service_pid){
                        pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                        /*signal_handler(P61_STATE_SPI_PRIO, pn544_dev->nfc_service_pid);*/
                        dwp_OnOff(pn544_dev->nfc_service_pid, P61_STATE_SPI_PRIO);
                    }
                    else{
                        pr_info(" invalid nfc service pid....signalling failed%s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                    }
                }
                pn544_dev->spi_ven_enabled = true;
                if(pn544_dev->chip_pwr_scheme != PN80T_EXT_PMU_SCHEME)
                {
                    if (pn544_dev->nfc_ven_enabled == false) {
                        /* provide power to NFCC if, NFC service not provided */
                        gpio_set_value(pn544_dev->ven_gpio, 1);
                        msleep(10);
                    }
                    /* pull the gpio to high once NFCC is power on*/
                    gpio_set_value(pn544_dev->ese_pwr_gpio, 1);

                    /* Delay (10ms) after SVDD_PWR_ON to allow JCOP to bootup (5ms jcop boot time + 5ms guard time) */
                    usleep_range(10000, 12000);
                }
            }else {
                pr_info("%s : Prio Session Start power on ese failed \n", __func__);
                return -EBUSY; /* Device or resource busy */
            }
        }else if (arg == 4) {
            if (current_state & P61_STATE_SPI_PRIO)
            {
                pr_info("%s : PN61_SET_SPI_PWR - Prio Session Ending...\n", __func__);
                p61_update_access_state(pn544_dev, P61_STATE_SPI_PRIO, false);
                /*after SPI prio timeout, the state is changing from SPI prio to SPI */
                p61_update_access_state(pn544_dev, P61_STATE_SPI, true);
                if (current_state & P61_STATE_WIRED)
                {
                    if(pn544_dev->nfc_service_pid){
                        pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                        signal_handler(P61_STATE_SPI_PRIO_END, pn544_dev->nfc_service_pid);
                    }
                    else{
                        pr_info(" invalid nfc service pid....signalling failed%s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                    }
               }
            }
            else
            {
                pr_info("%s : PN61_SET_SPI_PWR -  Prio Session End failed \n", __func__);
                return -EBADRQC; /* Device or resource busy */
            }
        } else if(arg == 5){
            release_ese_lock(P61_STATE_SPI);
        } else if (arg == 6) {
            /*SPI Service called ISO-RST*/
            p61_access_state_t current_state = P61_STATE_INVALID;
            p61_get_access_state(pn544_dev, &current_state);
            if(current_state & P61_STATE_WIRED) {
                return -EPERM; /* Operation not permitted */
            }
            if(current_state & P61_STATE_SPI) {
                p61_update_access_state(pn544_dev, P61_STATE_SPI, false);
            }else if(current_state & P61_STATE_SPI_PRIO) {
                p61_update_access_state(pn544_dev, P61_STATE_SPI_PRIO, false);
            }
#ifdef ISO_RST
            gpio_set_value(pn544_dev->iso_rst_gpio, 0);
            msleep(50);
            gpio_set_value(pn544_dev->iso_rst_gpio, 1);
            msleep(50);
            pr_info("%s ISO RESET from SPI DONE\n", __func__);
#endif
        }
        else {
            pr_info("%s bad ese pwr arg %lu\n", __func__, arg);
            return -EBADRQC; /* Invalid request code */
        }
    }
    break;

    case P61_GET_PWR_STATUS:
    {
        p61_access_state_t current_state = P61_STATE_INVALID;
        p61_get_access_state(pn544_dev, &current_state);
        //pr_info("%s: P61_GET_PWR_STATUS  = %x",__func__, current_state);
        put_user(current_state, (int __user *)arg);
    }
    break;

    case PN544_SET_DWNLD_STATUS:
    {
        long ret;
        ret = set_jcop_download_state(arg);
        if(ret < 0)
        {
            return ret;
        }
    }
    break;

    case P61_SET_WIRED_ACCESS:
    {
        p61_access_state_t current_state = P61_STATE_INVALID;
        p61_get_access_state(pn544_dev, &current_state);
        if (arg == 1)
        {
            if (current_state)
            {
               //pr_info("%s : P61_SET_WIRED_ACCESS - enabling\n", __func__);
                p61_update_access_state(pn544_dev, P61_STATE_WIRED, true);
                if (current_state & P61_STATE_SPI_PRIO)
                {
                    if(pn544_dev->nfc_service_pid){
                        //pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                        signal_handler(P61_STATE_SPI_PRIO, pn544_dev->nfc_service_pid);
                    }
                    else{
                        pr_info(" invalid nfc service pid....signalling failed%s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                    }
                }
                if((current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO)) == 0 && (pn544_dev->chip_pwr_scheme == PN67T_PWR_SCHEME))
                    gpio_set_value(pn544_dev->ese_pwr_gpio, 1);
            } else {
                pr_info("%s : P61_SET_WIRED_ACCESS -  enabling failed \n", __func__);
                return -EBUSY; /* Device or resource busy */
            }
        } else if (arg == 0) {
           //pr_info("%s : P61_SET_WIRED_ACCESS - disabling \n", __func__);
            if (current_state & P61_STATE_WIRED){
                p61_update_access_state(pn544_dev, P61_STATE_WIRED, false);
                if((current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO)) == 0 && (pn544_dev->chip_pwr_scheme == PN67T_PWR_SCHEME))
                {
                    svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START);
                    gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
                    svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_END);
                }
            } else {
                pr_err("%s : P61_SET_WIRED_ACCESS - failed, current_state = %x \n",
                        __func__, pn544_dev->p61_current_state);
                return -EPERM; /* Operation not permitted */
            }
        }
        else if(arg == 2)
        {
            //pr_info("%s : P61_ESE_GPIO_LOW  \n", __func__);
             if(pn544_dev->chip_pwr_scheme == PN67T_PWR_SCHEME)
             {
                 svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_START);
                 gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
                 svdd_sync_onoff(pn544_dev->nfc_service_pid, P61_STATE_SPI_SVDD_SYNC_END);
             }
        }
        else if(arg == 3)
        {
            //pr_info("%s : P61_ESE_GPIO_HIGH  \n", __func__);
            if(pn544_dev->chip_pwr_scheme == PN67T_PWR_SCHEME)
            gpio_set_value(pn544_dev->ese_pwr_gpio, 1);
        }
        else if(arg == 4)
        {
            release_ese_lock(P61_STATE_WIRED);
        }
        else if(arg == 5)
        {
            gpio_set_value(pn544_dev->ese_pwr_gpio, 1);
            if (gpio_get_value(pn544_dev->ese_pwr_gpio)) {
                pr_info("%s: ese_pwr gpio is enabled\n", __func__);
            }
        }
        else if(arg == 6)
        {
            gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
            pr_info("%s: ese_pwr gpio set to low\n", __func__);
        }
        else {
             pr_info("%s P61_SET_WIRED_ACCESS - bad arg %lu\n", __func__, arg);
             return -EBADRQC; /* Invalid request code */
        }
    }
    break;
    case P544_SET_POWER_SCHEME:
    {
        if(arg == PN67T_PWR_SCHEME)
        {
            pn544_dev->chip_pwr_scheme = PN67T_PWR_SCHEME;
            //pr_info("%s : The power scheme is set to PN67T legacy \n", __func__);
        }
        else if(arg == PN80T_LEGACY_PWR_SCHEME)
        {
            pn544_dev->chip_pwr_scheme = PN80T_LEGACY_PWR_SCHEME;
            //pr_info("%s : The power scheme is set to PN80T_LEGACY_PWR_SCHEME,\n", __func__);
        }
        else if(arg == PN80T_EXT_PMU_SCHEME)
        {
            pn544_dev->chip_pwr_scheme = PN80T_EXT_PMU_SCHEME;
            //pr_info("%s : The power scheme is set to PN80T_EXT_PMU_SCHEME,\n", __func__);
        }
        else
        {
            pr_info("%s : The power scheme is invalid,\n", __func__);
        }
    }
    break;
    case P544_SECURE_TIMER_SESSION:
    {
       secure_timer_operation(pn544_dev, arg);
    }
    break;
    default:
        pr_err("%s bad ioctl %u\n", __func__, cmd);
        return -EINVAL;
    }
    pr_info("%s :exit cmd = %u, arg = %ld\n", __func__, cmd, arg);
    return 0;
}
EXPORT_SYMBOL(pn544_dev_ioctl);

static void secure_timer_workqueue(struct work_struct *Wq)
{
  p61_access_state_t current_state = P61_STATE_INVALID;
  printk( KERN_INFO "secure_timer_callback: called (%lu).\n", jiffies);
  /* Locking the critical section: ESE_PWR_OFF to allow eSE to shutdown peacefully :: START */
  get_ese_lock(P61_STATE_WIRED, MAX_ESE_ACCESS_TIME_OUT_MS);
  p61_update_access_state(pn544_dev, P61_STATE_SECURE_MODE, false);
  p61_get_access_state(pn544_dev, &current_state);

  if((current_state & (P61_STATE_SPI|P61_STATE_SPI_PRIO)) == 0)
  {
      printk( KERN_INFO "secure_timer_callback: make se_pwer_gpio low, state = %d", current_state);
      gpio_set_value(pn544_dev->ese_pwr_gpio, 0);
      /* Delay (2.5ms) after SVDD_PWR_OFF for the shutdown settlement time */
      usleep_range(2500, 3000);
      if(pn544_dev->nfc_service_pid == 0x00)
      {
          gpio_set_value(pn544_dev->ven_gpio, 0);
          printk( KERN_INFO "secure_timer_callback :make ven_gpio low, state = %d", current_state);
      }
  }
  pn544_dev->secure_timer_cnt = 0;
  /* Locking the critical section: ESE_PWR_OFF to allow eSE to shutdown peacefully :: END */
  release_ese_lock(P61_STATE_WIRED);
  return;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
static void secure_timer_callback( unsigned long data )
#else
static void secure_timer_callback( struct timer_list *t )
#endif
{
    /* Flush and push the timer callback event to the bottom half(work queue)
    to be executed later, at a safer time */
    flush_workqueue(pn544_dev->pSecureTimerCbWq);
    queue_work(pn544_dev->pSecureTimerCbWq, &pn544_dev->wq_task);
    return;
}

static long start_seccure_timer(unsigned long timer_value)
{
    long ret = -EINVAL;
    pr_info("start_seccure_timer: enter\n");
    /* Delete the timer if timer pending */
    if(timer_pending(&secure_timer) == 1)
    {
        pr_info("start_seccure_timer: delete pending timer \n");
        /* delete timer if already pending */
        del_timer(&secure_timer);
    }
    /* Start the timer if timer value is non-zero */
    if(timer_value)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
        init_timer(&secure_timer);
        setup_timer( &secure_timer, secure_timer_callback, 0 );
#else
        timer_setup( &secure_timer, secure_timer_callback, 0 );
#endif

        pr_info("start_seccure_timer: timeout %lums (%lu)\n",timer_value, jiffies );
        ret = mod_timer( &secure_timer, jiffies + msecs_to_jiffies(timer_value));
        if (ret)
            pr_info("start_seccure_timer: Error in mod_timer\n");
    }
    return ret;
}

static long secure_timer_operation(struct pn544_dev *pn544_dev, unsigned long arg)
{
    long ret = -EINVAL;
    unsigned long timer_value =  arg;

    printk( KERN_INFO "secure_timer_operation, %d\n",pn544_dev->chip_pwr_scheme);
    if(pn544_dev->chip_pwr_scheme == PN80T_LEGACY_PWR_SCHEME)
    {
        ret = start_seccure_timer(timer_value);
        if(!ret)
        {
            pn544_dev->secure_timer_cnt  = 1;
            p61_update_access_state(pn544_dev, P61_STATE_SECURE_MODE, true);
        }
        else
        {
            pn544_dev->secure_timer_cnt  = 0;
            p61_update_access_state(pn544_dev, P61_STATE_SECURE_MODE, false);
            pr_info("%s :Secure timer reset \n", __func__);
        }
    }
    else
    {
        pr_info("%s :Secure timer session not applicable  \n", __func__);
    }
    return ret;
}

static long set_jcop_download_state(unsigned long arg)
{
        p61_access_state_t current_state = P61_STATE_INVALID;
        long ret = 0;
        p61_get_access_state(pn544_dev, &current_state);
        pr_info("%s:Enter PN544_SET_DWNLD_STATUS:JCOP Dwnld state arg = %ld",__func__, arg);
        if(arg == JCP_DWNLD_INIT)
        {
            if(pn544_dev->nfc_service_pid)
            {
                pr_info("nfc service pid %s   ---- %ld", __func__, pn544_dev->nfc_service_pid);
                signal_handler((p61_access_state_t)JCP_DWNLD_INIT, pn544_dev->nfc_service_pid);
            }
            else
            {
                if (current_state & P61_STATE_JCP_DWNLD)
                {
                    ret = -EINVAL;
                }
                else
                {
                    p61_update_access_state(pn544_dev, P61_STATE_JCP_DWNLD, true);
                }
            }
        }
        else if (arg == JCP_DWNLD_START)
        {
            if (current_state & P61_STATE_JCP_DWNLD)
            {
                ret = -EINVAL;
            }
            else
            {
                p61_update_access_state(pn544_dev, P61_STATE_JCP_DWNLD, true);
            }
        }
        else if (arg == JCP_SPI_DWNLD_COMPLETE)
        {
            if(pn544_dev->nfc_service_pid)
            {
                signal_handler((p61_access_state_t)JCP_DWP_DWNLD_COMPLETE, pn544_dev->nfc_service_pid);
            }
            p61_update_access_state(pn544_dev, P61_STATE_JCP_DWNLD, false);
        }
        else if (arg == JCP_DWP_DWNLD_COMPLETE)
        {
            p61_update_access_state(pn544_dev, P61_STATE_JCP_DWNLD, false);
        }
        else
        {
            pr_info("%s bad ese pwr arg %lu\n", __func__, arg);
            return -EBADRQC; /* Invalid request code */
        }
        pr_info("%s: PN544_SET_DWNLD_STATUS  = %x",__func__, current_state);

    return ret;
}

int get_ese_lock(p61_access_state_t  p61_current_state, int timeout)
{
    unsigned long tempJ = msecs_to_jiffies(timeout);
    if(down_timeout(&ese_access_sema, tempJ) != 0)
    {
        printk("get_ese_lock: timeout p61_current_state = %d\n", p61_current_state);
        return -EBUSY;
    }
    return 0;
}
EXPORT_SYMBOL(get_ese_lock);

static void release_ese_lock(p61_access_state_t  p61_current_state)
{
    up(&ese_access_sema);
}


static const struct file_operations pn544_dev_fops = {
        .owner  = THIS_MODULE,
        .llseek = no_llseek,
        .read   = pn544_dev_read,
        .write  = pn544_dev_write,
        .open   = pn544_dev_open,
        .unlocked_ioctl  = pn544_dev_ioctl,
};
#if DRAGON_NFC
static int pn544_parse_dt(struct device *dev,
    struct pn544_i2c_platform_data *data)
{
    struct device_node *np = dev->of_node;
    int errorno = 0;

#if !NEXUS5x
        data->irq_gpio = of_get_named_gpio(np, "nxp,pn544-irq", 0);
        if ((!gpio_is_valid(data->irq_gpio)))
                return -EINVAL;

        data->ven_gpio = of_get_named_gpio(np, "nxp,pn544-ven", 0);
        if ((!gpio_is_valid(data->ven_gpio)))
                return -EINVAL;

        data->firm_gpio = of_get_named_gpio(np, "nxp,pn544-fw-dwnld", 0);
        if ((!gpio_is_valid(data->firm_gpio)))
                return -EINVAL;

        data->ese_pwr_gpio = of_get_named_gpio(np, "nxp,pn544-ese-pwr", 0);
        if ((!gpio_is_valid(data->ese_pwr_gpio)))
                return -EINVAL;
        data->iso_rst_gpio = of_get_named_gpio(np, "nxp,pn544-iso-pwr-rst", 0);
        if ((!gpio_is_valid(data->iso_rst_gpio)))
                return -EINVAL;
#else
        data->ven_gpio = of_get_named_gpio_flags(np,
                                        "nxp,ven", 0, NULL);
        data->firm_gpio = of_get_named_gpio_flags(np,
                                        "nxp,dwld_en", 0, NULL);
        data->irq_gpio = of_get_named_gpio_flags(np,
                                        "nxp,irq_gpio", 0, NULL);
        data->ese_pwr_gpio = of_get_named_gpio_flags(np,
                                        "nxp,ese-pwr", 0, NULL);
#endif
    pr_info("%s: %d, %d, %d, %d, %d error:%d\n", __func__,
        data->irq_gpio, data->ven_gpio, data->firm_gpio, data->iso_rst_gpio,
        data->ese_pwr_gpio, errorno);

    return errorno;
}
#endif

static int pn544_probe(struct i2c_client *client,
        const struct i2c_device_id *id)
{
    int ret;
    struct pn544_i2c_platform_data *platform_data;
    //struct pn544_dev *pn544_dev;

#if !DRAGON_NFC
    platform_data = client->dev.platform_data;
#else
    struct device_node *node = client->dev.of_node;

    if (node) {
        platform_data = devm_kzalloc(&client->dev,
            sizeof(struct pn544_i2c_platform_data), GFP_KERNEL);
        if (!platform_data) {
            dev_err(&client->dev,
                "nfc-nci probe: Failed to allocate memory\n");
            return -ENOMEM;
        }
        ret = pn544_parse_dt(&client->dev, platform_data);
        if (ret)
        {
            pr_info("%s pn544_parse_dt failed", __func__);
        }
        client->irq = gpio_to_irq(platform_data->irq_gpio);
        if (client->irq < 0)
        {
            pr_info("%s gpio to irq failed", __func__);
        }
    } else {
        platform_data = client->dev.platform_data;
    }
#endif
    if (platform_data == NULL) {
        pr_err("%s : nfc probe fail\n", __func__);
        return  -ENODEV;
    }

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        pr_err("%s : need I2C_FUNC_I2C\n", __func__);
        return  -ENODEV;
    }
#if !DRAGON_NFC
    ret = gpio_request(platform_data->irq_gpio, "nfc_int");
    if (ret)
        return  -ENODEV;
    ret = gpio_request(platform_data->ven_gpio, "nfc_ven");
    if (ret)
        goto err_ven;
    ret = gpio_request(platform_data->ese_pwr_gpio, "nfc_ese_pwr");
    if (ret)
        goto err_ese_pwr;
    if (platform_data->firm_gpio) {
        ret = gpio_request(platform_data->firm_gpio, "nfc_firm");
        if (ret)
            goto err_firm;
    }
#ifdef ISO_RST
    if(platform_data->iso_rst_gpio) {
        ret = gpio_request(platform_data->iso_rst_gpio, "nfc_iso_rst");
        if (ret)
            goto err_iso_rst;
    }
#endif
#endif
    pn544_dev = kzalloc(sizeof(*pn544_dev), GFP_KERNEL);
    if (pn544_dev == NULL) {
        dev_err(&client->dev,
                "failed to allocate memory for module data\n");
        ret = -ENOMEM;
        goto err_exit;
    }

    pn544_dev->irq_gpio = platform_data->irq_gpio;
    pn544_dev->ven_gpio  = platform_data->ven_gpio;
    pn544_dev->firm_gpio  = platform_data->firm_gpio;
    pn544_dev->ese_pwr_gpio  = platform_data->ese_pwr_gpio;
#ifdef ISO_RST
    pn544_dev->iso_rst_gpio = platform_data->iso_rst_gpio;
#endif
    pn544_dev->p61_current_state = P61_STATE_IDLE;
    pn544_dev->nfc_ven_enabled = false;
    pn544_dev->spi_ven_enabled = false;
    pn544_dev->chip_pwr_scheme = PN67T_PWR_SCHEME;
    pn544_dev->client   = client;
    pn544_dev->secure_timer_cnt = 0;

    pn544_dev->tx_kbuf_len = MAX_BUFFER_SIZE;
    pn544_dev->rx_kbuf_len = MAX_BUFFER_SIZE;
    pn544_dev->tx_kbuf = kzalloc(MAX_BUFFER_SIZE, GFP_DMA | GFP_KERNEL);
    pn544_dev->rx_kbuf = kzalloc(MAX_BUFFER_SIZE, GFP_DMA | GFP_KERNEL);
    if (!pn544_dev->tx_kbuf || !pn544_dev->tx_kbuf) {
        dev_err(&client->dev,
            "failed to allocate memory for pn544_dev tx/rx buffer\n");
        ret = -ENOMEM;
        goto err_free_dev;
    }

    ret = gpio_direction_input(pn544_dev->irq_gpio);
    if (ret < 0) {
        pr_err("%s :not able to set irq_gpio as input\n", __func__);
        goto err_ven;
    }
    ret = gpio_direction_output(pn544_dev->ven_gpio, 0);
    if (ret < 0) {
        pr_err("%s : not able to set ven_gpio as output\n", __func__);
        goto err_firm;
    }
    ret = gpio_direction_output(pn544_dev->ese_pwr_gpio, 0);
    if (ret < 0) {
        pr_err("%s : not able to set ese_pwr gpio as output\n", __func__);
        goto err_ese_pwr;
    }
    if (platform_data->firm_gpio) {
        ret = gpio_direction_output(pn544_dev->firm_gpio, 0);
        if (ret < 0) {
            pr_err("%s : not able to set firm_gpio as output\n",
                    __func__);
            goto err_exit;
        }
    }
#ifdef ISO_RST
    ret = gpio_direction_output(pn544_dev->iso_rst_gpio, 0);
    if (ret < 0) {
        pr_err("%s : not able to set iso rst gpio as output\n", __func__);
        goto err_iso_rst;
    }
#endif
    /* init mutex and queues */
    init_waitqueue_head(&pn544_dev->read_wq);
    mutex_init(&pn544_dev->read_mutex);
    sema_init(&ese_access_sema, 1);
    sema_init(&dwp_onoff_release_sema, 0);
    spin_lock_init(&pn544_dev->irq_enabled_lock);
    pn544_dev->pSecureTimerCbWq = create_workqueue(SECURE_TIMER_WORK_QUEUE);
    INIT_WORK(&pn544_dev->wq_task, secure_timer_workqueue);
    pn544_dev->pn544_device.minor = MISC_DYNAMIC_MINOR;
    pn544_dev->pn544_device.name = "pn553";
    pn544_dev->pn544_device.fops = &pn544_dev_fops;

    ret = misc_register(&pn544_dev->pn544_device);
    if (ret) {
        pr_err("%s : misc_register failed\n", __FILE__);
        goto err_misc_register;
    }
#ifdef ISO_RST
    /* Setting ISO RESET pin high to power ESE during init */
    gpio_set_value(pn544_dev->iso_rst_gpio, 1);
#endif
    /* request irq.  the irq is set whenever the chip has data available
     * for reading.  it is cleared when all data has been read.
     */
    pr_info("%s : requesting IRQ %d\n", __func__, client->irq);
    pn544_dev->irq_enabled = true;
    ret = request_irq(client->irq, pn544_dev_irq_handler,
            IRQF_TRIGGER_HIGH, client->name, pn544_dev);
    if (ret) {
        dev_err(&client->dev, "request_irq failed\n");
        goto err_request_irq_failed;
    }
    enable_irq_wake(pn544_dev->client->irq);
    pn544_disable_irq(pn544_dev);
    device_init_wakeup(&client->dev, true);
    i2c_set_clientdata(client, pn544_dev);
#if HWINFO
    /*
     * This function is used only if
     * hardware info is required during probe*/
    check_hw_info();
#endif

    return 0;

    err_request_irq_failed:
    misc_deregister(&pn544_dev->pn544_device);
    err_misc_register:
    mutex_destroy(&pn544_dev->read_mutex);
    err_free_dev:
    kfree(pn544_dev);
    err_exit:
    if (pn544_dev->firm_gpio)
        gpio_free(platform_data->firm_gpio);
    err_firm:
    gpio_free(platform_data->ese_pwr_gpio);
    err_ese_pwr:
    gpio_free(platform_data->ven_gpio);
    err_ven:
    gpio_free(platform_data->irq_gpio);
#ifdef ISO_RST
    err_iso_rst:
    gpio_free(platform_data->iso_rst_gpio);
#endif
    return ret;
}

static int pn544_remove(struct i2c_client *client)
{
    struct pn544_dev *pn544_dev;

    pn544_dev = i2c_get_clientdata(client);
    free_irq(client->irq, pn544_dev);
    misc_deregister(&pn544_dev->pn544_device);
    mutex_destroy(&pn544_dev->read_mutex);
    gpio_free(pn544_dev->irq_gpio);
    gpio_free(pn544_dev->ven_gpio);
    gpio_free(pn544_dev->ese_pwr_gpio);
    destroy_workqueue(pn544_dev->pSecureTimerCbWq);
#ifdef ISO_RST
    gpio_free(pn544_dev->iso_rst_gpio);
#endif
    pn544_dev->p61_current_state = P61_STATE_INVALID;
    pn544_dev->nfc_ven_enabled = false;
    pn544_dev->spi_ven_enabled = false;

    if (pn544_dev->firm_gpio)
        gpio_free(pn544_dev->firm_gpio);

    kfree(pn544_dev->tx_kbuf);
    kfree(pn544_dev->rx_kbuf);
    kfree(pn544_dev);
    return 0;
}

static int pn553_pm_ops_resume(struct device *dev)
{
    struct pn544_dev *pn544_dev = dev_get_drvdata(dev);

    if (device_may_wakeup(&pn544_dev->client->dev)) {
        pr_debug("%s disable irq\n", __func__);
        disable_irq_wake(pn544_dev->client->irq);
    }

    return 0;
}

static int pn553_pm_ops_suspend(struct device *dev)
{
    struct pn544_dev *pn544_dev = dev_get_drvdata(dev);

    if (device_may_wakeup(&pn544_dev->client->dev)) {
        pr_debug("%s enable irq\n", __func__);
        enable_irq_wake(pn544_dev->client->irq);
    }

    return 0;
}

static const struct dev_pm_ops pn553_pm_ops = {
    .suspend    = pn553_pm_ops_suspend,
    .resume     = pn553_pm_ops_resume,
};

static const struct platform_device_id pn553_pm_ops_id[] = {
    { "pn553-pm-ops", 0 },
    { }
};

static const struct of_device_id pn553_pm_ops_match_table[] = {
    { .compatible = "sony,pn553-pm-ops"},
    { }
};

static int pn553_pm_ops_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "%s, probing pn553 PM OPS driver\n", __func__);

    if (!pn544_dev)
        return -ENODEV;

    platform_set_drvdata(pdev, pn544_dev);

    dev_info(&pdev->dev,
        "%s, probe pn553 PM OPS driver successfully\n", __func__);

    return 0;
}

static struct platform_driver pn553_pm_ops_driver = {
        .id_table   = pn553_pm_ops_id,
        .probe      = pn553_pm_ops_probe,
        .driver     = {
                .name  = "pn553-pm-ops",
                .of_match_table = pn553_pm_ops_match_table,
                .pm    = &pn553_pm_ops,
    },
};

static const struct i2c_device_id pn544_id[] = {
#if NEXUS5x
        { "pn553", 0 },
#else
        { "pn544", 0 },
#endif
        { }
};
#if DRAGON_NFC
static struct of_device_id pn544_i2c_dt_match[] = {
    {
#if NEXUS5x
        .compatible = "nxp,pn553",
#else
        .compatible = "nxp,pn544",
#endif
    },
    {}
};
#endif
static struct i2c_driver pn544_driver = {
        .id_table   = pn544_id,
        .probe      = pn544_probe,
        .remove     = pn544_remove,
        .driver     = {
                .owner = THIS_MODULE,
#if NEXUS5x
                .name  = "pn553",
#else
                .name  = "pn544",
#endif
#if DRAGON_NFC
                .of_match_table = pn544_i2c_dt_match,
#endif
        },
};
#if HWINFO
/******************************************************************************
 * Function         check_hw_info
 *
 * Description      This function is called during pn544_probe to retrieve
 *                  HW info.
 *                  Useful get HW information in case of previous FW download is
 *                  interrupted and core reset is not allowed.
 *                  This function checks if core reset  is allowed, if not
 *                  sets DWNLD_REQ(firm_gpio) , ven reset and sends firmware
 *                  get version command.
 *                  In response HW information will be received.
 *
 * Returns          None
 *
 ******************************************************************************/
static void check_hw_info() {
    char read_data[20];
    int ret, get_version_len = 8, retry_count = 0;
    static uint8_t cmd_reset_nci[] = {0x20, 0x00, 0x01, 0x00};
    char get_version_cmd[] =
    {0x00, 0x04, 0xF1, 0x00, 0x00, 0x00, 0x6E, 0xEF};

    pr_info("%s :Enter\n", __func__);

    /*
     * Ven Reset  before sending core Reset
     * This is to check core reset is allowed or not.
     * If not allowed then previous FW download is interrupted in between
     * */
    pr_info("%s :Ven Reset \n", __func__);
    gpio_set_value(pn544_dev->ven_gpio, 1);
    msleep(10);
    gpio_set_value(pn544_dev->ven_gpio, 0);
    msleep(10);
    gpio_set_value(pn544_dev->ven_gpio, 1);
    msleep(10);
    ret = i2c_master_send(pn544_dev->client, cmd_reset_nci, 4);

    if (ret == 4) {
        pr_info("%s : core reset write success\n", __func__);
    } else {

        /*
         * Core reset  failed.
         * set the DWNLD_REQ , do ven reset
         * send firmware download info command
         * */
        pr_err("%s : write failed\n", __func__);
        pr_info("%s power on with firmware\n", __func__);
        gpio_set_value(pn544_dev->ven_gpio, 1);
        msleep(10);
        if (pn544_dev->firm_gpio) {
            p61_update_access_state(pn544_dev, P61_STATE_DWNLD, true);
            gpio_set_value(pn544_dev->firm_gpio, 1);
        }
        msleep(10);
        gpio_set_value(pn544_dev->ven_gpio, 0);
        msleep(10);
        gpio_set_value(pn544_dev->ven_gpio, 1);
        msleep(10);
        ret = i2c_master_send(pn544_dev->client, get_version_cmd, get_version_len);
        if (ret != get_version_len) {
            ret = -EIO;
            pr_err("%s : write_failed \n", __func__);
        }
        else {
            pr_info("%s :data sent\n", __func__);
        }

        ret = 0;

        while (retry_count < 10) {

            /*
             * Wait for read interrupt
             * If spurious interrupt is received retry again
             * */
            pn544_dev->irq_enabled = true;
            enable_irq(pn544_dev->client->irq);
            enable_irq_wake(pn544_dev->client->irq);
            ret = wait_event_interruptible(
                    pn544_dev->read_wq,
                    !pn544_dev->irq_enabled);

            pn544_disable_irq(pn544_dev);

            if (gpio_get_value(pn544_dev->irq_gpio))
                break;

            pr_warning("%s: spurious interrupt detected\n", __func__);
            retry_count ++;
        }

        if(ret) {
            return;
        }

        /*
         * Read response data and copy into hw_type_info
         * */
        ret = i2c_master_recv(pn544_dev->client, read_data, 14);

        if(ret) {
            memcpy(hw_info.data, read_data, ret);
            hw_info.len = ret;
            pr_info("%s :data received len  : %d\n", __func__,hw_info.len);
        }
        else {
            pr_err("%s :Read Failed\n", __func__);
        }
    }
}
#endif
/*
 * module load/unload record keeping
 */

static int __init pn544_dev_init(void)
{
    int ret;
    pr_info("Loading pn544 driver\n");
    ret = i2c_add_driver(&pn544_driver);

    if (ret)
        goto exit;
    pr_info("Loading pn544 pm ops driver\n");
    ret = platform_driver_register(&pn553_pm_ops_driver);
    if (ret)
        goto exit_del_i2c_driver;

    return 0;

exit_del_i2c_driver:
    i2c_del_driver(&pn544_driver);

exit:
    return ret;
}
module_init(pn544_dev_init);

static void __exit pn544_dev_exit(void)
{
    pr_info("Unloading pn544 driver\n");
    i2c_del_driver(&pn544_driver);
    platform_driver_unregister(&pn553_pm_ops_driver);
}
module_exit(pn544_dev_exit);

MODULE_AUTHOR("Sylvain Fonteneau");
MODULE_DESCRIPTION("NFC PN544 driver");
MODULE_LICENSE("GPL");

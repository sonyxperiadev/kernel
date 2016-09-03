/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.


 *  Copyright (C) 2009-2014 Broadcom Corporation
 */

/*******************************************************************************
 *
 *  Filename:      fmdrv_rx.c
 *
 *  Description:   This sub-module of FM driver implements FM RX functionality.
 *
 *******************************************************************************/

#include "fmdrv.h"
#include "fmdrv_main.h"
#include "fmdrv_rx.h"
#include "fmdrv_config.h"
#include "fm_public.h"
#include "../include/v4l2_target.h"
#include "../include/v4l2_logs.h"


/*******************************************************************************
**  Constants & Macros
*******************************************************************************/

#ifndef V4L2_FM_DEBUG
#define V4L2_FM_DEBUG TRUE
#endif

/* set this module parameter to enable debug info */
extern int fm_dbg_param;

extern struct region_info region_configs[];

#if V4L2_FM_DEBUG
#define V4L2_FM_DRV_DBG(flag, fmt, arg...) \
        do { \
            if (fm_dbg_param & flag) \
                printk(KERN_DEBUG "(v4l2fmdrv):%s  "fmt"\n" , \
                                           __func__,## arg); \
        } while(0)
#else
#define V4L2_FM_DRV_DBG(flag, fmt, arg...)
#endif
#define V4L2_FM_DRV_ERR(fmt, arg...)  printk(KERN_ERR "(v4l2fmdrv):%s  "fmt"\n" , \
                                           __func__,## arg)

const unsigned short fm_sch_step_size[] =
{
    50,
    100,
    200
};

/*******************************************************************************
**  Functions
*******************************************************************************/
/*******************************************************************************
**  Helper functions
*******************************************************************************/

/* Configures Alternate Frequency switch mode */
int fm_rx_set_af_switch(struct fmdrv_ops *fmdev, u8 af_mode)
{
    u16 payload;
    int ret;

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    if (af_mode != FM_RX_RDS_AF_SWITCH_MODE_ON &&
        af_mode != FM_RX_RDS_AF_SWITCH_MODE_OFF) {
        V4L2_FM_DRV_ERR("Invalid af mode\n");
        return -EINVAL;
    }
    /* Enable/disable low RSSI interrupt based on af_mode */
    if (af_mode == FM_RX_RDS_AF_SWITCH_MODE_ON)
        fmdev->rx.fm_rds_mask |= I2C_MASK_RSSI_LOW_BIT;
    else
        fmdev->rx.fm_rds_mask &= ~I2C_MASK_RSSI_LOW_BIT;

    payload = fmdev->rx.fm_rds_mask;

    ret = fmc_send_cmd(fmdev, FM_REG_FM_RDS_MSK, &fmdev->rx.fm_rds_mask,
                            2, REG_WR,&fmdev->maintask_completion, NULL, NULL);

    if (ret < 0)
        return ret;

    fmdev->rx.af_mode = af_mode;

    return 0;
}


/*
 * Sets the signal strength level that once reached
 * will stop the auto search process
 */
int fm_rx_set_rssi_threshold(struct fmdrv_ops *fmdev, short rssi_lvl_toset)
{
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, " fm_rx_set_rssi_threshold to set is %d",\
        rssi_lvl_toset);

    if (rssi_lvl_toset < FM_RX_RSSI_THRESHOLD_MIN ||
        rssi_lvl_toset > FM_RX_RSSI_THRESHOLD_MAX) {
        V4L2_FM_DRV_ERR("Invalid RSSI threshold level\n");
        return -EINVAL;
    }

    fmdev->rx.curr_rssi_threshold = rssi_lvl_toset;
    return 0;
}

/*
 * Sets the signal strength level that once reached
 * will stop the auto search process
 */
int fm_rx_set_snr_threshold(struct fmdrv_ops *fmdev, short snr_lvl_toset)
{
    u16 payload;
    int ret;

    if (snr_lvl_toset < FM_RX_SNR_THRESHOLD_MIN ||
        snr_lvl_toset > FM_RX_SNR_THRESHOLD_MAX) {
        V4L2_FM_DRV_ERR("Invalid SNR threshold level\n");
        return -EINVAL;
    }
    payload = (u16) snr_lvl_toset;
    ret = fmc_send_cmd(fmdev, FM_SEARCH_SNR, &payload, sizeof(payload),
            REG_WR, &fmdev->maintask_completion, NULL,NULL);

    if (ret < 0)
        return ret;

    fmdev->rx.curr_snr_threshold= snr_lvl_toset;

    return 0;
}



/*
* Function to validate if the tuned/scanned frequency is valid
* or not
*/
int check_if_valid_freq(struct fmdrv_ops *fmdev, unsigned short frequency)
{
    if(frequency < fmdev->rx.region.low_bound ||
            frequency > fmdev->rx.region.high_bound)
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv)%s %d - Literally out of range", \
            __func__, FM_SET_FREQ(frequency));
        return FALSE;
    }
    else
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv)%s %d - Freq in range", __func__, \
            FM_SET_FREQ(frequency));
        return TRUE;
    }
}

/*
* Function to read the FM_RDS_FLAG registry
*/
int read_fm_rds_flag(struct fmdrv_ops *fmdev, unsigned short *value)
{
    unsigned char read_length;
    int ret;
    int resp_len;
    unsigned char resp_buf [2];

    read_length = FM_READ_2_BYTE_DATA;
    ret = fmc_send_cmd(fmdev, FM_REG_FM_RDS_FLAG, &read_length,
        sizeof(read_length), REG_RD,
                    &fmdev->maintask_completion, &resp_buf, &resp_len);
    *value = (unsigned short)resp_buf[0] +
                ((unsigned short)resp_buf[1] << 8);
    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) FM Mask : 0x%x ", *value);
    return 0;
}

/*
* Function to read the FM_RDS_FLAG registry
*/
int fm_rx_set_mask(struct fmdrv_ops *fmdev, unsigned short mask)
{
    int ret;
    unsigned short flag;

    fmdev->rx.fm_rds_flag|= FM_RDS_FLAG_CLEAN_BIT; /* clean FM_RDS_FLAG */
    ret = read_fm_rds_flag(fmdev, &flag);
    FM_CHECK_SEND_CMD_STATUS(ret);

    ret = fmc_send_cmd(fmdev, FM_REG_FM_RDS_MSK, &mask, sizeof(mask), REG_WR,
            &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);
    return ret;
}

/*
* Helper Function to initialize and start a search operation
* (SEEK or TUNE). This method is internally called by fm_rx_set_frequency()
* and fm_rx_seek_station().
*/
int init_start_search(struct fmdrv_ops *fmdev, unsigned short start_freq,
                        unsigned char mode)
{
    unsigned char payload;
    unsigned short tmp_fm_rds_mask;
    int ret;

    if(mode == FM_TUNER_SEEK_MODE)
    {
        /* Set Scan mode */
        payload = FM_TUNER_NORMAL_SCAN_MODE;
        ret = fmc_send_cmd(fmdev, FM_SEARCH_METHOD, &payload, 1, REG_WR,
                &fmdev->maintask_completion, NULL, NULL);
        FM_CHECK_SEND_CMD_STATUS(ret);

        V4L2_FM_DRV_DBG(V4L2_DBG_TX,"(fmdev) %s FM_SEARCH_METHOD set to 0x%x",\
            __func__, payload);

        /* Set Preset stations number to 0 */
        payload = 0;
        ret = fmc_send_cmd(fmdev, FM_REG_PRESET_MAX, &payload, 1, REG_WR,
                &fmdev->maintask_completion, NULL, NULL);
        FM_CHECK_SEND_CMD_STATUS(ret);

        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdev) %s FM_REG_PRESET_MAX set to 0x%x",\
            __func__, payload);

        /* Set FM Search control params to controller */
        payload = fmdev->rx.curr_rssi_threshold | (fmdev->rx.curr_sch_mode &
                                                          FM_SCAN_DIRECT_MASK);
        ret = fmc_send_cmd(fmdev, FM_REG_SCH_CTL0, &payload, 1, REG_WR,
                &fmdev->maintask_completion, NULL, NULL);
        FM_CHECK_SEND_CMD_STATUS(ret);

        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdev) %s FM_REG_SCH_CTL0 set to 0x%x",\
            __func__, payload);

    }

    /* freeze interrupt event before SCH_TUNE is commanded */
    fmdev->rx.fm_rds_flag |= FM_RDS_FLAG_SCH_FRZ_BIT;
    /* set sch_tune pending bit */
    fmdev->rx.fm_rds_flag |= FM_RDS_FLAG_SCH_BIT;

    /* Write Frequency */
    /* Write FM_REG_FM_FREQ (0x0a) register first */
    ret = fmc_send_cmd(fmdev, FM_REG_FM_FREQ, &start_freq,
            sizeof(start_freq), REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);

    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdev) %s FM_REG_FM_FREQ set to 0x%x", \
        __func__, FM_SET_FREQ(start_freq));

    /* Set the mask flag to Register FM_REG_FM_RDS_MSK(0x10)
    & FM_REG_FM_RDS_MSK1(0x11) */
    tmp_fm_rds_mask= I2C_MASK_SRH_TUNE_CMPL_BIT | I2C_MASK_SRH_TUNE_FAIL_BIT;
    ret = fm_rx_set_mask(fmdev, tmp_fm_rds_mask);
    FM_CHECK_SEND_CMD_STATUS(ret);

    /* Reset the fm_rds_flag here as for the first time we dont get
    any interrupt during ENABLE to cleanup the bit */
    fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_CLEAN_BIT;

    /* Write FM_REG_SCH_TUNE (0x09) register */
    /*payload = FM_TUNER_SEEK_MODE;*/ /* Scan parameter (0x02) */
    payload = mode;
    ret = fmc_send_cmd(fmdev, FM_REG_SCH_TUNE, &payload, sizeof(payload),
            REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);
    fmdev->rx.curr_search_state = (mode == FM_TUNER_SEEK_MODE)?
        FM_STATE_SEEKING:FM_STATE_TUNING;

    fmc_reset_rds_cache(fmdev);

    return 0;
}

/*
* Function to process a SEEK complete event. This function determines
* whether to wrap the search, or stop the search or return error
* to user-space. This is called internally by fm_rx_seek_station() function.
*/
int process_seek_event(struct fmdrv_ops *fmdev)
{
    unsigned short tmp_freq, start_freq;
    int ret = -EINVAL;
    bool is_valid_freq;

    tmp_freq = fmdev->rx.curr_freq;
    is_valid_freq = check_if_valid_freq(fmdev, tmp_freq);
    if(((FM_SET_FREQ(tmp_freq) - 5) <= FM_SET_FREQ(fmdev->rx.region.low_bound)) ||
       ((FM_SET_FREQ(tmp_freq) + 5)  >= FM_SET_FREQ(fmdev->rx.region.high_bound)))
    {
        is_valid_freq = FALSE;
    }

    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) %s tmp:%d low:%d high:%d", __func__,\
        tmp_freq, fmdev->rx.region.low_bound, fmdev->rx.region.high_bound);

    /* First check if Scan suceeded or not */
    if(fmdev->rx.curr_search_state == FM_STATE_SEEK_ERR)
    {
        fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_FRZ_BIT;
        if(!fmdev->rx.seek_wrap && !is_valid_freq)
        {
            fmdev->rx.curr_search_state = FM_STATE_SEEK_ERR;
            V4L2_FM_DRV_ERR("(fmdrv) Seek ended with out of bound frequency %d",\
                FM_SET_FREQ(tmp_freq));
           return FALSE;
        }
        else if(fmdev->rx.seek_wrap && !is_valid_freq)
        {
            V4L2_FM_DRV_ERR("(fmdrv) Scan ended with out of bound frequency." \
                "Wrapping search again..");

            start_freq = (fmdev->rx.seek_direction==FM_SCAN_DOWN)?
                (fmdev->rx.region.high_bound):(fmdev->rx.region.low_bound);
            V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdev) Current scanned frequency " \
                "is out of bounds. Resetting to freq (%d) ",
                        FM_SET_FREQ(start_freq));

            ret = init_start_search(fmdev, start_freq, FM_TUNER_SEEK_MODE);
            if(ret < 0)
            {
                fmdev->rx.curr_search_state = FM_STATE_SEEK_ERR;
                fmdev->rx.curr_freq = 0;
                V4L2_FM_DRV_ERR ("(fmdrv): Error starting search for Seek " \
                    "operation");
                return FALSE;
            }

            fmdev->rx.curr_search_state = FM_STATE_SEEKING;
            V4L2_FM_DRV_DBG (V4L2_DBG_RX, "(fmdrv): Started wrapped-up Seek " \
                "operation");
            return TRUE;
        }
        else
        {
            fmdev->rx.curr_search_state = FM_STATE_SEEK_ERR;
            V4L2_FM_DRV_ERR("(fmdrv) *** ERROR :: Seek failed for %d " \
                "frequency ***", FM_SET_FREQ(tmp_freq));
            return FALSE;
        }
    }
    else
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) Seek success!");
        fmdev->rx.curr_search_state = FM_STATE_SEEK_CMPL;
        return TRUE;
    }
}

/*******************************************************************************
**  Main functions - Called by fmdrv_main and fmdrv_v4l2.
*******************************************************************************/

/*
* Function to read current RSSI and tuned frequency
*/
int fm_rx_read_curr_rssi_freq(struct fmdrv_ops *fmdev,
                                   unsigned char rssi_only)
{
    int ret = 0, resp_len;
    unsigned char payload;
    unsigned short tmp_frq;
    unsigned char resp_buf[2];

    /* Read current RSSI */
    payload = FM_READ_1_BYTE_DATA;
    ret = fmc_send_cmd(fmdev, FM_REG_RSSI, &payload, 1,
            REG_RD, &fmdev->maintask_completion, &resp_buf[0], &resp_len);
    FM_CHECK_SEND_CMD_STATUS(ret);
    /* Calculating 2's compliment number into an absolute value number */
    fmdev->rx.curr_rssi = (unsigned char) ((0x80 - resp_buf[0]) & (~0x80));
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdev) FM_REG_RSSI : %d", \
        fmdev->rx.curr_rssi);

    if(rssi_only)
        return 0;

    /* Read current frequency */
    payload = FM_READ_2_BYTE_DATA;
    tmp_frq = 0;
    ret = fmc_send_cmd(fmdev, FM_REG_FM_FREQ, &payload, 1,
            REG_RD, &fmdev->maintask_completion, &tmp_frq, &resp_len);
    FM_CHECK_SEND_CMD_STATUS(ret);
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdev) FM_REG_FM_FREQ : %d", \
        FM_SET_FREQ(tmp_frq));

    fmdev->rx.curr_freq = tmp_frq;

    return ret;
}

/*
* Function for FM TUNE frequency implementation
*   * Set the other registries such as Search method, search
*    direction, etc.
*   * Start preset search.
*   * Based on interrupt received, read the current tuned freq
*     and validate the search.
*   * If search frequency out of bound, return error code -EAGAIN
*   * If not, read RSSI, reset RDS cache and set RDS MASK to the earlier value.
*/
int fm_rx_set_frequency(struct fmdrv_ops *fmdev, unsigned int freq_to_set)
{
    unsigned short tmp_frq;
    int ret = -EINVAL;
    unsigned long timeleft;

    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): current region = %d",\
        fmdev->rx.curr_region);

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;
    tmp_frq = FM_GET_FREQ(freq_to_set);
    if(!check_if_valid_freq(fmdev, tmp_frq))
    {
        V4L2_FM_DRV_ERR("(fmdrv): Set frequency called with %d - out of "\
            "bound range (%d-%d)",
            freq_to_set, FM_SET_FREQ(fmdev->rx.region.low_bound),
            FM_SET_FREQ(fmdev->rx.region.high_bound));
        return -EINVAL;
    }

    ret = init_start_search(fmdev, tmp_frq, FM_TUNER_PRESET_MODE);
    if(ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Error starting search for Seek operation");
        return ret;
    }

    /* Wait for tune ended interrupt */
    init_completion(&fmdev->maintask_completion);
    timeleft = wait_for_completion_timeout(&fmdev->maintask_completion,
                           FM_DRV_TX_TIMEOUT);
    if (!timeleft)
    {
        V4L2_FM_DRV_ERR("(fmdrv) Timeout(%d sec),didn't get tune ended interrupt",\
               jiffies_to_msecs(FM_DRV_TX_TIMEOUT) / 1000);
        fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_FRZ_BIT;
        return -ETIMEDOUT;
    }

    /* First check if Tune suceeded or not */
    if(fmdev->rx.curr_search_state == FM_STATE_TUNE_ERR)
    {
        V4L2_FM_DRV_ERR("(fmdrv) Tune failed for %d MHz frequency", \
            FM_SET_FREQ(tmp_frq));
        fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_FRZ_BIT;
        return -EAGAIN;
    }
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Set frequency done!");
    fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_FRZ_BIT;

    fm_rx_read_curr_rssi_freq(fmdev, FALSE);
    /* Reset RDS Cache */
    fmc_reset_rds_cache(fmdev);

    if(fmdev->rx.fm_rds_mask)
    {
        /* Update the FM_RDS_MASK to set the earlier bits */
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Update FM_RDS_MASK : 0x%x", \
        fmdev->rx.fm_rds_mask);
        ret = fmc_send_cmd(fmdev, FM_REG_FM_RDS_MSK, &fmdev->rx.fm_rds_mask,
            sizeof(fmdev->rx.fm_rds_mask), REG_WR,
            &fmdev->maintask_completion, NULL, NULL);
    }

    return 0;
}

/*
* Function to get the current tuned frequency
* This function will query controller by reading the FM_REG_FM_FREQ(0x0a)
* to determine the current tuned frequency.
*/
int fm_rx_get_frequency(struct fmdrv_ops *fmdev, unsigned int *curr_freq)
{
    unsigned char payload;
    unsigned short tmp_frq;
    int ret;
    int resp_len;

    payload = 2;
    ret = fmc_send_cmd(fmdev, FM_REG_FM_FREQ, &payload, 1, REG_RD,
            &fmdev->maintask_completion, &tmp_frq, &resp_len);
    FM_CHECK_SEND_CMD_STATUS(ret);
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdev) FM_REG_FM_FREQ : %d", \
        FM_SET_FREQ(tmp_frq));

    *curr_freq = FM_SET_FREQ(tmp_frq);

    return ret;
}

/*
* Function to start a FM SEEK Operation.
*   * Set the start frequency.
*   * Set the other registries such as Search method, search
*    direction, etc.
*   * Start search.
*   * Based on interrupt received, read the current tuned freq
*     and validate the search.
*   * If search frequency out of bound and no wrap_around needed,
*    end the search and return error code -EAGAIN
*   * If not, start the search again and check for interrupt.
*   * If no interrupt is received by 20 sec, timeout the seek operation
*/
int fm_rx_seek_station(struct fmdrv_ops *fmdev, unsigned char direction_upward,
                            unsigned char wrap_around)
{
    int ret = 0, freq;
    unsigned short tmp_freq, start_freq;
    unsigned long timeleft;

    fmdev->rx.seek_direction = (direction_upward)?FM_SCAN_UP:FM_SCAN_DOWN;
    fmdev->rx.curr_sch_mode = ((FM_TUNER_NORMAL_SCAN_MODE & 0x01) |
                                                (fmdev->rx.seek_direction & 0x80));
    fmdev->rx.seek_wrap = wrap_around;

    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) seek_direction:0x%x "\
        "curr_sch_mode:0x%x seek_wrap:0x%x", \
        fmdev->rx.seek_direction, fmdev->rx.curr_sch_mode, fmdev->rx.seek_wrap);

    ret = fm_rx_get_frequency(fmdev, &freq);
    tmp_freq = FM_GET_FREQ(freq);

    if(!check_if_valid_freq(fmdev, tmp_freq))
    {
        start_freq = (direction_upward)?(fmdev->rx.region.low_bound+
            fm_sch_step_size[fmdev->rx.sch_step])
            :(fmdev->rx.region.high_bound - fm_sch_step_size[fmdev->rx.sch_step]);
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdev) Current frequency is out of "\
            "bounds. Resetting to freq (%d) ", FM_SET_FREQ(start_freq));
    }
    else
    {
        start_freq = (direction_upward)?(tmp_freq + fm_sch_step_size[fmdev->rx.sch_step])
            :(tmp_freq - fm_sch_step_size[fmdev->rx.sch_step]);
        if(start_freq >= fmdev->rx.region.high_bound ||
            start_freq <= fmdev->rx.region.low_bound)
                start_freq =  (direction_upward)?
                (fmdev->rx.region.low_bound):(fmdev->rx.region.high_bound);
    }
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Starting FM seek (%s) from %d..", \
        (direction_upward?"SEEKUP":"SEEKDOWN"), FM_SET_FREQ(start_freq));



    ret = init_start_search(fmdev, start_freq, FM_TUNER_SEEK_MODE);
    if(ret < 0)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Error starting search for Seek operation");
        return -EINVAL;
    }

    /* Wait for tune ended interrupt */
    init_completion(&fmdev->seektask_completion);
    timeleft = wait_for_completion_timeout(&fmdev->seektask_completion,
                           FM_DRV_RX_SEEK_TIMEOUT);
    if (!timeleft)
    {
        V4L2_FM_DRV_ERR("(fmdrv) Timeout(%d sec),didn't get seek ended interrupt",\
               jiffies_to_msecs(FM_DRV_RX_SEEK_TIMEOUT) / 1000);
        fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_FRZ_BIT;
        return -ETIMEDOUT;
    }

    fm_rx_read_curr_rssi_freq(fmdev, FALSE);
    tmp_freq = fmdev->rx.curr_freq;
    ret = process_seek_event(fmdev);
    if(ret && fmdev->rx.curr_search_state == FM_STATE_SEEK_CMPL)
    {
        /* Reset RDS Cache */
        fmc_reset_rds_cache(fmdev);
        if(fmdev->rx.fm_rds_mask)
        /* Update the FM_RDS_MASK to set the earlier bits */
        ret = fmc_send_cmd(fmdev, FM_REG_FM_RDS_MSK, &fmdev->rx.fm_rds_mask,
            sizeof(fmdev->rx.fm_rds_mask), REG_WR,
            &fmdev->maintask_completion, NULL, NULL);
        return 0;
    }
    if(!ret)
    {
        V4L2_FM_DRV_ERR("(fmdrv) Error during Seek. Try again!");
        return -EAGAIN;
    }
    else if(ret && fmdev->rx.curr_search_state == FM_STATE_SEEKING)
    {

        /* Wait for tune ended interrupt */
        init_completion(&fmdev->seektask_completion);
        timeleft = wait_for_completion_timeout(&fmdev->seektask_completion,
                               FM_DRV_RX_SEEK_TIMEOUT);

        fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_SCH_FRZ_BIT;
        if (!timeleft)
        {
            V4L2_FM_DRV_ERR("(fmdrv) Timeout(%d sec),didn't get Seek ended "\
                "interrupt", jiffies_to_msecs(FM_DRV_RX_SEEK_TIMEOUT) / 1000);
            return -ETIMEDOUT;
        }

        fm_rx_read_curr_rssi_freq(fmdev, FALSE);

        /* First check if Scan suceeded or not */
        if(fmdev->rx.curr_search_state == FM_STATE_SEEK_ERR)
        {
            V4L2_FM_DRV_ERR("(fmdrv) Wrap Seek failed for %d frequency", \
                FM_SET_FREQ(fmdev->rx.curr_freq));
            return -EAGAIN;
        }
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Wrap Seek done!");
        /* Reset RDS Cache */
        fmc_reset_rds_cache(fmdev);
        /* Update the FM_RDS_MASK to set the earlier bits */
        ret = fmc_send_cmd(fmdev, FM_REG_FM_RDS_MSK, &fmdev->rx.fm_rds_mask,
            sizeof(fmdev->rx.fm_rds_mask), REG_WR,
            &fmdev->maintask_completion, NULL, NULL);
        return 0;
    }

    V4L2_FM_DRV_ERR("(fmdrv) Unhandled case in Seek");
    return -EINVAL;
}

/*
*Function to set band's high and low frequencies
*/
int fm_rx_set_band_frequencies(struct fmdrv_ops *fmdev,
                         unsigned int low_freq, unsigned int high_freq)
{
    if((fmdev->rx.region.high_bound == FM_GET_FREQ(high_freq)) &&
        (fmdev->rx.region.low_bound = FM_GET_FREQ(low_freq)))
    {
        V4L2_FM_DRV_ERR("(fmdrv) Ignoring setting the same band frequencies");
        return 0;
    }
    fmdev->rx.region.high_bound = FM_GET_FREQ(high_freq);
    fmdev->rx.region.low_bound = FM_GET_FREQ(low_freq);
    return 0;
}

/*
*Function to get the current band's high and low frequencies
*/
int fm_rx_get_band_frequencies(struct fmdrv_ops *fmdev,
                              unsigned int *low_freq,  unsigned int *high_freq)
{
    *high_freq= FM_SET_FREQ(fmdev->rx.region.high_bound);
    *low_freq= FM_SET_FREQ(fmdev->rx.region.low_bound) ;
    return 0;
}

/*
* Function to set the volume
*/
int fm_rx_set_volume(struct fmdrv_ops *fmdev, unsigned short vol_to_set)
{
    unsigned short payload;
    int ret;
    unsigned char read_length;
    unsigned short actual_volume;

    if(vol_to_set > FM_RX_VOLUME_MAX)
        actual_volume = (vol_to_set/FM_RX_VOLUME_RATIO);
    else
        actual_volume = vol_to_set;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Actual volume to set  : %d", \
        actual_volume);

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    if (actual_volume > FM_RX_VOLUME_MAX)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Volume %d is not within(%d-%d) "
            "range setting the maximum allowed", vol_to_set,FM_RX_VOLUME_MIN,\
            FM_RX_VOLUME_MAX);
        actual_volume = 0xFF;
    }

    payload = actual_volume & 0x1ff;
    ret = fmc_send_cmd(fmdev, FM_REG_VOLUME_CTRL, &payload, sizeof(payload),
        REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);

    fmdev->rx.curr_volume = vol_to_set;
    /* Read current volume */
    read_length = FM_READ_1_BYTE_DATA;
    ret = fm_rx_get_volume(fmdev, &(fmdev->rx.curr_volume));
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Volume read : %d", \
        fmdev->rx.curr_volume);
    if(ret == -ETIMEDOUT)
        return -EBUSY;
    return ret;
}

/*
*Function to Get volume
*/
int fm_rx_get_volume(struct fmdrv_ops *fmdev, unsigned short *curr_vol)
{
    int ret, resp_len;
    unsigned short resp_buf;
    unsigned char read_length;

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    if (curr_vol == NULL)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid memory");
        return -ENOMEM;
    }

    /* Read current volume */
    read_length = FM_READ_2_BYTE_DATA;
    ret = fmc_send_cmd(fmdev, FM_REG_VOLUME_CTRL, &read_length, 1, REG_RD,
                        &fmdev->maintask_completion, &resp_buf, &resp_len);
    FM_CHECK_SEND_CMD_STATUS(ret);

    *curr_vol = fmdev->rx.curr_volume = resp_buf;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Volume read : %d", \
        fmdev->rx.curr_volume);
    return ret;
}

/* Sets band (0-Europe; 1-Japan; 2-North America; 3-Russia, 4-China) */
int fm_rx_set_region(struct fmdrv_ops *fmdev,
            unsigned char region_to_set)
{
    unsigned char payload = FM_STEREO_AUTO|FM_BAND_REG_WEST;
    unsigned short boundary[2];

    int ret = -EPERM;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX,"fm_rx_set_region In region_to_set %d",\
        region_to_set);
    if (fmdev->curr_fmmode != FM_MODE_RX)
        return ret;

    if (region_to_set != FM_REGION_NA &&
        region_to_set != FM_REGION_EUR &&
        region_to_set != FM_REGION_JP &&
        region_to_set != FM_REGION_RUS &&
        region_to_set != FM_REGION_CHN &&
        region_to_set != FM_REGION_IT)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid band");
        ret = -EINVAL;
        return ret;
    }

    if (region_to_set == FM_REGION_JP ||
        region_to_set == FM_REGION_RUS ||
        region_to_set == FM_REGION_CHN)
    {
        /* set the low bound and high bound */
        memcpy(&fmdev->rx.region, &region_configs[region_to_set], sizeof(struct region_info));
        boundary[0] = fmdev->rx.region.high_bound;
        boundary[1] = fmdev->rx.region.low_bound;
        fmc_send_cmd(fmdev, FM_SEARCH_BOUNDARY, boundary, sizeof(boundary), REG_WR,
                    &fmdev->maintask_completion, NULL, NULL);
    }
    else {
        /* clear low bound and high bound */
        boundary[0] = 0;
        boundary[1] = 0;
        fmc_send_cmd(fmdev, FM_SEARCH_BOUNDARY, boundary, sizeof(boundary), REG_WR,
                    &fmdev->maintask_completion, NULL, NULL);
    }

    if (region_to_set == FM_REGION_JP)/* set japan region */
    {
        payload |= FM_BAND_REG_EAST;
    }

    /* Send cmd to set the band  */
    ret = fmc_send_cmd(fmdev, FM_REG_FM_CTRL, &payload, sizeof(payload), REG_WR,
        &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);

    fmc_update_region_info(fmdev, region_to_set);

    return ret;
}

/*
* Function to retrieve audio control param
* from controller
*/
int fm_rx_get_audio_ctrl(struct fmdrv_ops *fmdev, uint16_t *audio_ctrl)
{
    uint16_t payload = FM_READ_2_BYTE_DATA;
    int ret = -EINVAL, resp_len;

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return ret;

    /* Send cmd to set the band  */
    ret = fmc_send_cmd(fmdev, FM_REG_AUD_CTL0, &payload, sizeof(payload), REG_RD,
                &fmdev->maintask_completion, audio_ctrl, &resp_len);
    FM_CHECK_SEND_CMD_STATUS(ret);
    fmdev->aud_ctrl = fmdev->rx.aud_ctrl = *audio_ctrl;
    if(ret == -ETIMEDOUT)
        return -EBUSY;
    return ret;
}

/*
* Function to set the audio control param
* to controller
*/
int fm_rx_set_audio_ctrl(struct fmdrv_ops *fmdev,uint16_t audio_ctrl)
{
    uint16_t payload = audio_ctrl;
    int ret = -EINVAL;

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return ret;

    /* Send cmd to set the band  */
    ret = fmc_send_cmd(fmdev, FM_REG_AUD_CTL0, &payload, sizeof(payload), REG_WR,
                &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);
    fmdev->aud_ctrl = fmdev->rx.aud_ctrl = audio_ctrl;
    if(ret == -ETIMEDOUT)
        return -EBUSY;
    return ret;
}

/*
* Function to Read current mute mode (Mute Off/On)
*/
int fm_rx_get_mute_mode(struct fmdrv_ops *fmdev,
            unsigned char *curr_mute_mode)
{
    uint16_t tmp;
    int ret = -EINVAL;
    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    if (curr_mute_mode == NULL) {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid memory");
        return -ENOMEM;
    }
    ret = fm_rx_get_audio_ctrl(fmdev, &tmp);
    *curr_mute_mode = fmdev->rx.curr_mute_mode = tmp & FM_MANUAL_MUTE;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Mute is %s", ((*curr_mute_mode)?\
        "ON":"OFF"));
    return 0;
}

/*
* Configures mute mode (Mute Off/On)
*/
int fm_rx_set_mute_mode(struct fmdrv_ops *fmdev,
            unsigned char mute_mode_toset)
{
    int ret;
    uint16_t aud_ctrl;

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;
    /* First read the aud_ctrl*/
    ret = fm_rx_get_audio_ctrl(fmdev, &aud_ctrl);
    /* turn on MUTE */
    if (mute_mode_toset)
    {
        aud_ctrl|= FM_MANUAL_MUTE;
    }
    else /* unmute */
    {
        aud_ctrl &= (~FM_MANUAL_MUTE);
    }

    ret = fm_rx_set_audio_ctrl (fmdev, aud_ctrl);
    FM_CHECK_SEND_CMD_STATUS(ret);
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv) Current mute state : %d", \
        mute_mode_toset);
    fmdev->rx.curr_mute_mode = mute_mode_toset;
    return ret;
}

/* Sets RX stereo/mono modes */
int fm_rx_set_audio_mode(struct fmdrv_ops *fmdev, unsigned char mode)
{
    unsigned char audio_ctrl = FM_STEREO_SWITCH|FM_STEREO_AUTO;
    int ret;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX,"(fmdrv): fm_rx_set_audio_mode in  :mode %d"\
        "fmdev->rx.audio_mode %d", mode,fmdev->rx.audio_mode);

    if (fmdev->curr_fmmode != FM_MODE_RX)
     return -EPERM;

    if (mode != FM_STEREO_MODE && mode != FM_MONO_MODE &&
        mode != FM_AUTO_MODE && mode != FM_SWITCH_MODE)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid mode :%d", mode);
        return -EINVAL;
    }

    if (fmdev->rx.audio_mode == mode)
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): no change in audio mode");
        return 0;
    }
    switch (mode)
    {
        case FM_SWITCH_MODE: /* stereo witch in auto mode */
            /* as default */;
            break;
        case FM_MONO_MODE: /* manually set to mono, bit2 OFF is mono */
            audio_ctrl &= ~FM_STEREO_AUTO;/* set to manual mono */
            break;
        case FM_STEREO_MODE: /* manually set to stereo */
            audio_ctrl  &= ~FM_STEREO_AUTO; /* set to manual */
            audio_ctrl |= FM_STEREO_MANUAL; /* set to stereo in manual mode */
            break;
        case FM_AUTO_MODE:/* auto blend as default,  */
            audio_ctrl  &= ~FM_STEREO_SWITCH; /* turn OFF bit3 to activate blend */
            break;
        default:
            break;
    }
    /* set the region bit */
    audio_ctrl |= (fmdev->rx.curr_region == FM_REGION_JP) ? \
                                FM_BAND_REG_EAST : FM_BAND_REG_WEST;

    /* Set stereo/mono mode */
    ret = fmc_send_cmd(fmdev, FM_REG_FM_CTRL, &audio_ctrl, sizeof(audio_ctrl),
            REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);
    fmdev->rx.audio_mode = mode;
    if(mode == FM_MONO_MODE)
    {
        fmdev->device_info.rxsubchans |= V4L2_TUNER_SUB_MONO;
        fmdev->device_info.rxsubchans &= ~V4L2_TUNER_SUB_STEREO;
    }
    if(mode == FM_STEREO_MODE)
    {
        fmdev->device_info.rxsubchans |= V4L2_TUNER_SUB_STEREO;
        fmdev->device_info.rxsubchans &= ~V4L2_TUNER_SUB_MONO;
    }
    return 0;
}

/* Gets current RX stereo/mono mode */
int fm_rx_get_audio_mode(struct fmdrv_ops *fmdev, unsigned char *mode)
{
    int ret, len;
    unsigned char payload = FM_READ_1_BYTE_DATA, resp;
    if (fmdev->curr_fmmode != FM_MODE_RX)
    return -EPERM;

    if (mode == NULL)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid memory");
        return -ENOMEM;
    }
    ret = fmc_send_cmd(fmdev, FM_REG_SNR, &payload, sizeof(payload),
            REG_RD, &fmdev->maintask_completion, &resp, &len);
    V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv): resp(current snr) : %x", resp);

    if(resp<=19)
    {
        *mode = FM_MONO_MODE;
    }
    else
    {
        *mode = FM_STEREO_MODE;
    }

    return ret;
}

/* Sets RX stereo/mono modes */
int fm_rx_config_audio_path(struct fmdrv_ops *fmdev, unsigned char path)
{
    int ret;

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    if (fmdev->rx.audio_path == path)
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): no change in audio path");
        return 0;
    }
    /* if FM is on SCO and request to turn off FM over SCO */
    if (!(path & FM_AUDIO_BT_MONO) &&
                (fmdev->rx.pcm_reg & FM_PCM_ROUTE_ON_BIT))
    {
        /* disable pcm_reg CB value FM routing bit */
        fmdev->rx.pcm_reg &= ~FM_PCM_ROUTE_ON_BIT;
    }
    else if((path & FM_AUDIO_BT_MONO) &&
                !(fmdev->rx.pcm_reg & FM_PCM_ROUTE_ON_BIT)) /* turn on FM via SCO */
    {
        /* when FM to SCO active, FM enforce I2S output */
        path |= FM_AUDIO_I2S;
        /* turn on pcm_reg CB value FM routing bit */
        fmdev->rx.pcm_reg |= FM_PCM_ROUTE_ON_BIT;
    }

    /* write to PCM_ROUTE register */
    ret = fmc_send_cmd(fmdev, FM_REG_PCM_ROUTE,
            &fmdev->rx.pcm_reg, sizeof(fmdev->rx.pcm_reg), REG_WR,
            &fmdev->maintask_completion, NULL, NULL);

    FM_CHECK_SEND_CMD_STATUS(ret);

    if (path & FM_AUDIO_I2S)
        fmdev->rx.aud_ctrl |= FM_AUDIO_I2S_ON;
    else
        fmdev->rx.aud_ctrl &= ~((unsigned short)FM_AUDIO_I2S_ON);

    if (path & FM_AUDIO_DAC)
        fmdev->rx.aud_ctrl |= FM_AUDIO_DAC_ON;
    else
        fmdev->rx.aud_ctrl &= ~((unsigned short)FM_AUDIO_DAC_ON);

    ret = fm_rx_set_audio_ctrl (fmdev, fmdev->rx.aud_ctrl);

    FM_CHECK_SEND_CMD_STATUS(ret);
    fmdev->rx.audio_path = path;

    return 0;
}

/* Choose RX de-emphasis filter mode (50us/75us) */
int fm_rx_config_deemphasis(struct fmdrv_ops *fmdev, unsigned long mode)
{
    int ret;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX, "fm_rx_config_deemphasis is setting  mode "\
        "as %ld",mode);

    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    if (mode != FM_DEEMPHA_50U &&
        mode != FM_DEEMPHA_75U)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid rx de-emphasis mode");
        return -EINVAL;
    }

    if (mode == FM_DEEMPHA_50U )
        /* set to 50us by turning off 6th bit */
        fmdev->rx.aud_ctrl &=  (~FM_DEEMPHA_75_ON);
    else
        /* set to 75us by turning on 6th bit */
        fmdev->rx.aud_ctrl |=  FM_DEEMPHA_75_ON;

    ret = fm_rx_set_audio_ctrl (fmdev, fmdev->rx.aud_ctrl);

    FM_CHECK_SEND_CMD_STATUS(ret);

    return 0;
}

/*
* Function to get the current scan step.
* Returns FM_STEP_100KHZ or FM_STEP_200KHZ
*/
int fm_rx_get_scan_step(struct fmdrv_ops *fmdev,
            unsigned char *step_type)
{
    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    if (step_type == NULL)
    {
        V4L2_FM_DRV_ERR("(fmdrv): Invalid memory");
        return -ENOMEM;
    }
    *step_type = fmdev->rx.sch_step;
    return 0;
}

/*
* Sets scan step to 100 or 200 KHz based on step type :
* FM_STEP_100KHZ or FM_STEP_200KHZ
*/
int fm_rx_set_scan_step(struct fmdrv_ops *fmdev,
            unsigned char step_type)
{
    int ret;
    unsigned short payload;
    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

       /* turn on MUTE */
    if (fmdev->rx.sch_step == step_type)
    {
        V4L2_FM_DRV_DBG(V4L2_DBG_TX, "(fmdrv): no change in scan step size");
        return 0;
    }
    payload = fm_sch_step_size[step_type];
    ret = fmc_send_cmd(fmdev, FM_REG_SCH_STEP, &payload, sizeof(payload),
            REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);

    fmdev->rx.sch_step = step_type;
    return ret;
}

/*******************************************************************************
** RDS functions
*******************************************************************************/

/* Sets RDS operation mode (RDS/RDBS) */
int fm_rx_set_rds_system(struct fmdrv_ops *fmdev, unsigned char rdbs_en_dis)
{
    unsigned char payload;
    int ret;
    V4L2_FM_DRV_DBG(V4L2_DBG_TX,"(fmdrv) %s", __func__);
    if (fmdev->curr_fmmode != FM_MODE_RX)
        return -EPERM;

    /* Set RDS control */
    if (rdbs_en_dis == FM_RDBS_ENABLE)
        payload = (FM_RDS_CTRL_RBDS|FM_RDS_CTRL_FIFO_FLUSH);
    else
        payload = FM_RDS_CTRL_FIFO_FLUSH;

    ret = fmc_send_cmd(fmdev, FM_REG_RDS_CTL0, &payload, sizeof(payload),
                        REG_WR, &fmdev->maintask_completion, NULL, NULL);
    FM_CHECK_SEND_CMD_STATUS(ret);

    return 0;
}

/*
* Function to enable RDS. Called during FM enable.
*/
void fm_rx_enable_rds(struct fmdrv_ops *fmdev)
{
    unsigned char payload;
    int ret = 0;
    if(fmdev->rx.fm_func_mask & (FM_RDS_BIT | FM_RBDS_BIT))
    {
        payload = FM_RDS_UPD_TUPLE;
        /* write RDS FIFO waterline in depth of RDS tuples */
        ret = fmc_send_cmd(fmdev, FM_REG_RDS_WLINE, &payload, sizeof(payload),
                            REG_WR, &fmdev->maintask_completion, NULL, NULL);
        if(ret<0)
            V4L2_FM_DRV_ERR("(fmdrv) Error writing to RDS FIFO waterline "\
            "register");
        /* drain RDS FIFO */
        payload = FM_RDS_FIFO_MAX;
        ret = fmc_send_cmd(fmdev, FM_REG_RDS_DATA, &payload, 1,
                            REG_RD, &fmdev->maintask_completion, NULL, NULL);

        /* set new FM_RDS mask so that RDS read */
        fmdev->rx.fm_rds_mask |= I2C_MASK_RDS_FIFO_WLINE_BIT;
    }
    else
    {
        V4L2_FM_DRV_ERR("(fmdrv) RDS not enabled during FM enable");
        fmdev->rx.fm_rds_mask &= ~I2C_MASK_RDS_FIFO_WLINE_BIT;
    }
    fm_rx_set_mask(fmdev, fmdev->rx.fm_rds_mask);
    /* Reset the fm_rds_flag here as for the first time we dont get
    any interrupt during ENABLE to cleanup the bit */
    fmdev->rx.fm_rds_flag &= ~FM_RDS_FLAG_CLEAN_BIT;

}

/*
* Returns availability of RDS data in internel buffer.
* If data is present in RDS buffer, return 0. Else, return -EAGAIN.
* The V4L2 driver's poll() uses this method to determine RDS data availability.
*/
int fm_rx_is_rds_data_available(struct fmdrv_ops *fmdev, struct file *file,
                  struct poll_table_struct *pts)
{
    poll_wait(file, &fmdev->rx.rds.read_queue, pts);
    if (fmdev->rx.rds.rd_index != fmdev->rx.rds.wr_index) {
        V4L2_FM_DRV_DBG(V4L2_DBG_RX, "(fmdrv) Poll success. RDS data is "\
            "available in buffer");
        return 0;
    }
    V4L2_FM_DRV_ERR("(fmdev) RDS Buffer is empty");
    return -EAGAIN;
}

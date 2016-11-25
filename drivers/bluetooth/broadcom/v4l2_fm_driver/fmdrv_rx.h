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

/************************************************************************************
 *
 *  Filename:      fmdrv_rx.h
 *
 *  Description:   FM RX module header.
************************************************************************************/

#ifndef _FMDRV_RX_H
#define _FMDRV_RX_H

/*******************************************************************************
**  Functions
*******************************************************************************/

int fm_rx_set_frequency(struct fmdrv_ops*, unsigned int);
int fm_rx_set_band_frequencies(struct fmdrv_ops *, unsigned int, unsigned int);
int fm_rx_set_mute_mode(struct fmdrv_ops*, unsigned char);
int fm_rx_set_rds_system(struct fmdrv_ops *, unsigned char);
int fm_rx_set_volume(struct fmdrv_ops*, unsigned short);
int fm_rx_set_audio_ctrl(struct fmdrv_ops *,unsigned short);

int fm_rx_set_audio_mode(struct fmdrv_ops *, unsigned char);
int fm_rx_set_region(struct fmdrv_ops*, unsigned char);
int fm_rx_set_scan_step(struct fmdrv_ops *, unsigned char);
int fm_rx_config_audio_path(struct fmdrv_ops *, unsigned char);
int fm_rx_config_deemphasis(struct fmdrv_ops *, unsigned long);
int fm_rx_set_rssi_threshold(struct fmdrv_ops*, short);
int fm_rx_set_snr_threshold(struct fmdrv_ops*,  short);
int fm_rx_set_af_switch(struct fmdrv_ops *, u8);

int fm_rx_get_frequency(struct fmdrv_ops*, unsigned int*);
int fm_rx_get_mute_mode(struct fmdrv_ops*, unsigned char*);
int fm_rx_get_volume(struct fmdrv_ops*, unsigned short*);
int fm_rx_get_audio_mode(struct fmdrv_ops *, unsigned char *);
int fm_rx_get_scan_step(struct fmdrv_ops *, unsigned char *);
int fm_rx_get_band_frequencies(struct fmdrv_ops *,
                    unsigned int *, unsigned int *);
int fm_rx_seek_station(struct fmdrv_ops *, unsigned char, unsigned char);
int fm_rx_read_curr_rssi_freq(struct fmdrv_ops *, unsigned char);
void fm_rx_enable_rds(struct fmdrv_ops *);
int fm_rx_is_rds_data_available(struct fmdrv_ops *, struct file *,
                    struct poll_table_struct *);

#endif


/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: reg_services.h
 * This file provides prototypes of the regulatory component
 * service functions
 */

#ifndef __REG_SERVICES_COMMON_H_
#define __REG_SERVICES_COMMON_H_

#define IS_VALID_PSOC_REG_OBJ(psoc_priv_obj) (psoc_priv_obj)
#define IS_VALID_PDEV_REG_OBJ(pdev_priv_obj) (pdev_priv_obj)
#define FREQ_TO_CHAN_SCALE     5
/* The distance between the 80Mhz center and the nearest 20Mhz channel */
#define NEAREST_20MHZ_CHAN_FREQ_OFFSET     10
#define NUM_20_MHZ_CHAN_IN_80_MHZ_CHAN     4
#define NUM_20_MHZ_CHAN_IN_160_MHZ_CHAN    8

#ifdef CONFIG_CHAN_NUM_API
#define REG_MIN_24GHZ_CH_NUM channel_map[MIN_24GHZ_CHANNEL].chan_num
#define REG_MAX_24GHZ_CH_NUM channel_map[MAX_24GHZ_CHANNEL].chan_num
#define REG_MIN_5GHZ_CH_NUM channel_map[MIN_5GHZ_CHANNEL].chan_num
#define REG_MAX_5GHZ_CH_NUM channel_map[MAX_5GHZ_CHANNEL].chan_num

#define REG_IS_24GHZ_CH(chan_num) \
	(((chan_num) >= REG_MIN_24GHZ_CH_NUM) &&	\
	 ((chan_num) <= REG_MAX_24GHZ_CH_NUM))
#endif /* CONFIG_CHAN_NUM_API */

#define REG_MIN_24GHZ_CH_FREQ channel_map[MIN_24GHZ_CHANNEL].center_freq
#define REG_MAX_24GHZ_CH_FREQ channel_map[MAX_24GHZ_CHANNEL].center_freq

#define REG_IS_24GHZ_CH_FREQ(freq) \
	(((freq) >= REG_MIN_24GHZ_CH_FREQ) &&   \
	((freq) <= REG_MAX_24GHZ_CH_FREQ))

#ifdef CONFIG_CHAN_FREQ_API
#define REG_MIN_5GHZ_CH_FREQ channel_map[MIN_5GHZ_CHANNEL].center_freq
#define REG_MAX_5GHZ_CH_FREQ channel_map[MAX_5GHZ_CHANNEL].center_freq
#endif /* CONFIG_CHAN_FREQ_API */

#define REG_MIN_49GHZ_CH_FREQ channel_map[MIN_49GHZ_CHANNEL].center_freq
#define REG_MAX_49GHZ_CH_FREQ channel_map[MAX_49GHZ_CHANNEL].center_freq

#define REG_IS_49GHZ_FREQ(freq) \
	(((freq) >= REG_MIN_49GHZ_CH_FREQ) &&   \
	((freq) <= REG_MAX_49GHZ_CH_FREQ))

#ifdef CONFIG_CHAN_NUM_API
#define REG_IS_5GHZ_CH(chan_num) \
	(((chan_num) >= REG_MIN_5GHZ_CH_NUM) &&	\
	 ((chan_num) <= REG_MAX_5GHZ_CH_NUM))
#endif /* CONFIG_CHAN_NUM_API */

#define REG_IS_5GHZ_FREQ(freq) \
	(((freq) >= channel_map[MIN_5GHZ_CHANNEL].center_freq) &&	\
	 ((freq) <= channel_map[MAX_5GHZ_CHANNEL].center_freq))

/*
 * It should be 2.5 MHz actually but since we are using integer use 2
 * instead, which does not create any problem in the start edge calculation.
 */
#define HALF_5MHZ_BW     2
#define HALF_20MHZ_BW    10

#define TWO_GIG_STARTING_EDGE_FREQ (channel_map_global[MIN_24GHZ_CHANNEL]. \
				  center_freq - HALF_20MHZ_BW)
#define TWO_GIG_ENDING_EDGE_FREQ   (channel_map_global[MAX_24GHZ_CHANNEL]. \
				  center_freq + HALF_20MHZ_BW)
#define FIVE_GIG_STARTING_EDGE_FREQ (channel_map_global[MIN_49GHZ_CHANNEL]. \
				  center_freq - HALF_5MHZ_BW)
#define FIVE_GIG_ENDING_EDGE_FREQ   (channel_map_global[MAX_5GHZ_CHANNEL]. \
				  center_freq + HALF_20MHZ_BW)

#ifdef CONFIG_BAND_6GHZ
#define SIX_GIG_STARTING_EDGE_FREQ  (channel_map_global[MIN_6GHZ_CHANNEL]. \
				  center_freq - HALF_20MHZ_BW)
#define SIX_GIG_ENDING_EDGE_FREQ    (channel_map_global[MAX_6GHZ_CHANNEL]. \
				  center_freq + HALF_20MHZ_BW)

#define FREQ_LEFT_SHIFT         55
#define SIX_GHZ_NON_ORPHAN_START_FREQ \
	(channel_map_global[MIN_6GHZ_NON_ORPHAN_CHANNEL].center_freq  - 5)
#define CHAN_FREQ_5935          5935
#define NUM_80MHZ_BAND_IN_6G    16
#define NUM_PSC_FREQ            15
#define PSC_BAND_MHZ (FREQ_TO_CHAN_SCALE * NUM_80MHZ_BAND_IN_6G)
#define REG_MIN_6GHZ_CHAN_FREQ channel_map[MIN_6GHZ_CHANNEL].center_freq
#define REG_MAX_6GHZ_CHAN_FREQ channel_map[MAX_6GHZ_CHANNEL].center_freq
#else
#define FREQ_LEFT_SHIFT         0
#define SIX_GHZ_NON_ORPHAN_START_FREQ       0
#define CHAN_FREQ_5935          0
#define NUM_80MHZ_BAND_IN_6G    0
#define NUM_PSC_FREQ            0
#define PSC_BAND_MHZ (FREQ_TO_CHAN_SCALE * NUM_80MHZ_BAND_IN_6G)
#define REG_MIN_6GHZ_CHAN_FREQ  0
#define REG_MAX_6GHZ_CHAN_FREQ  0
#endif /*CONFIG_BAND_6GHZ*/

#define REG_CH_NUM(ch_enum) channel_map[ch_enum].chan_num
#define REG_CH_TO_FREQ(ch_enum) channel_map[ch_enum].center_freq

/* EEPROM setting is a country code */
#define    COUNTRY_ERD_FLAG     0x8000
#define MIN_6GHZ_OPER_CLASS 131
#define MAX_6GHZ_OPER_CLASS 136

extern const struct chan_map *channel_map;
extern const struct chan_map channel_map_us[];
extern const struct chan_map channel_map_eu[];
extern const struct chan_map channel_map_jp[];
extern const struct chan_map channel_map_china[];
extern const struct chan_map channel_map_global[];

#ifdef CONFIG_CHAN_NUM_API
/**
 * reg_get_chan_enum() - Get channel enum for given channel number
 * @chan_num: Channel number
 *
 * Return: Channel enum
 */
enum channel_enum reg_get_chan_enum(uint8_t chan_num);

/**
 * reg_get_channel_state() - Get channel state from regulatory
 * @pdev: Pointer to pdev
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					 uint8_t ch);

/**
 * reg_get_5g_bonded_channel() - get the 5G bonded channel state
 * @pdev: Pointer to pdev structure
 * @chan_num: channel number
 * @ch_width: channel width
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: channel state
 */
enum channel_state reg_get_5g_bonded_channel(
		struct wlan_objmgr_pdev *pdev, uint8_t chan_num,
		enum phy_ch_width ch_width,
		const struct bonded_channel **bonded_chan_ptr_ptr);

/**
 * reg_get_5g_bonded_channel_state() - Get channel state for 5G bonded channel
 * @pdev: Pointer to pdev
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t ch,
		enum phy_ch_width bw);

/**
 * reg_get_2g_bonded_channel_state() - Get channel state for 2G bonded channel
 * @ch: channel number.
 * @pdev: Pointer to pdev
 * @oper_ch: Primary channel number
 * @sec_ch: Secondary channel number
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev, uint8_t oper_ch, uint8_t sec_ch,
		enum phy_ch_width bw);

/**
 * reg_set_channel_params () - Sets channel parameteres for given bandwidth
 * @pdev: Pointer to pdev
 * @ch: channel number.
 * @sec_ch_2g: Secondary 2G channel
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void reg_set_channel_params(struct wlan_objmgr_pdev *pdev,
			    uint8_t ch, uint8_t sec_ch_2g,
			    struct ch_params *ch_params);

/**
 * reg_is_disable_ch() - Check if the given channel in disable state
 * @pdev: Pointer to pdev
 * @chan: channel number
 *
 * Return: True if channel state is disabled, else false
 */
bool reg_is_disable_ch(struct wlan_objmgr_pdev *pdev, uint8_t chan);
#endif /* CONFIG_CHAN_NUM_API */

/**
 * reg_read_default_country() - Get the default regulatory country
 * @psoc: The physical SoC to get default country from
 * @country_code: the buffer to populate the country code into
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code);

/**
 * reg_get_ctry_idx_max_bw_from_country_code() - Get the max 5G bandwidth
 * from country code
 * @cc : Country Code
 * @max_bw_5g : Max 5G bandwidth supported by the country
 *
 * Return : QDF_STATUS
 */

QDF_STATUS reg_get_max_5g_bw_from_country_code(uint16_t cc,
					       uint16_t *max_bw_5g);

/**
 * reg_get_max_5g_bw_from_regdomain() - Get the max 5G bandwidth
 * supported by the regdomain
 * @orig_regdmn : Regdomain pair value
 * @max_bw_5g : Max 5G bandwidth supported by the country
 *
 * Return : QDF_STATUS
 */

QDF_STATUS reg_get_max_5g_bw_from_regdomain(uint16_t regdmn,
					    uint16_t *max_bw_5g);

/**
 * reg_get_current_dfs_region () - Get the current dfs region
 * @pdev: Pointer to pdev
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_get_current_dfs_region(struct wlan_objmgr_pdev *pdev,
				enum dfs_reg *dfs_reg);

#ifdef CONFIG_CHAN_NUM_API
/**
 * reg_get_channel_reg_power() - Get the txpower for the given channel
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: txpower
 */
uint32_t reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
				   uint8_t chan_num);

/**
 * reg_get_channel_freq() - Get the channel frequency
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: frequency
 */
qdf_freq_t reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
				uint8_t chan_num);
#endif /* CONFIG_CHAN_NUM_API */

/**
 * reg_get_bw_value() - give bandwidth value
 * bw: bandwidth enum
 *
 * Return: uint16_t
 */
uint16_t reg_get_bw_value(enum phy_ch_width bw);

/**
 * reg_set_dfs_region () - Set the current dfs region
 * @pdev: Pointer to pdev
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_set_dfs_region(struct wlan_objmgr_pdev *pdev,
			enum dfs_reg dfs_reg);

#ifdef CONFIG_CHAN_NUM_API
/**
 * reg_chan_to_band() - Get band from channel number
 * @chan_num: channel number
 *
 * Return: band info
 */
enum band_info reg_chan_to_band(uint8_t chan_num);
#endif /* CONFIG_CHAN_NUM_API */

/**
 * reg_program_chan_list() - Set user country code and populate the channel list
 * @pdev: Pointer to pdev
 * @rd: Pointer to cc_regdmn_s structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_program_chan_list(struct wlan_objmgr_pdev *pdev,
				 struct cc_regdmn_s *rd);

#ifdef CONFIG_CHAN_NUM_API
/**
 * reg_update_nol_ch () - Updates NOL channels in current channel list
 * @pdev: pointer to pdev object
 * @ch_list: pointer to NOL channel list
 * @num_ch: No.of channels in list
 * @update_nol: set/reset the NOL status
 *
 * Return: None
 */
void reg_update_nol_ch(struct wlan_objmgr_pdev *pdev, uint8_t *chan_list,
		       uint8_t num_chan, bool nol_chan);

/**
 * reg_is_dfs_ch () - Checks the channel state for DFS
 * @pdev: pdev ptr
 * @chan: channel
 *
 * Return: true or false
 */
bool reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev, uint8_t chan);
#endif /* CONFIG_CHAN_NUM_API */

/**
 * reg_freq_to_chan() - Get channel number from frequency.
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: Channel number if success, otherwise 0
 */
uint8_t reg_freq_to_chan(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_chan_to_freq() - Get frequency from channel number
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: Channel frequency if success, otherwise 0
 */
qdf_freq_t reg_chan_to_freq(struct wlan_objmgr_pdev *pdev, uint8_t chan_num);

/**
 * reg_legacy_chan_to_freq() - Get freq from chan noumber, for 2G and 5G
 * @pdev: Pointer to pdev
 * @chan_num: Channel number
 *
 * Return: Channel frequency if success, otherwise 0
 */
uint16_t reg_legacy_chan_to_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan_num);

#ifdef CONFIG_CHAN_NUM_API
/**
 * reg_chan_is_49ghz() - Check if the input channel number is 4.9GHz
 * @pdev: Pdev pointer
 * @chan_num: Input channel number
 *
 * Return: true if the channel is 4.9GHz else false.
 */
bool reg_chan_is_49ghz(struct wlan_objmgr_pdev *pdev, uint8_t chan_num);
#endif /* CONFIG_CHAN_NUM_API */

/**
 * reg_program_default_cc() - Program default country code
 * @pdev: Pdev pointer
 * @regdmn: Regdomain value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_program_default_cc(struct wlan_objmgr_pdev *pdev,
				  uint16_t regdmn);

/**
 * reg_get_current_cc() - Get current country code
 * @pdev: Pdev pointer
 * @regdmn: Pointer to get current country values
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_current_cc(struct wlan_objmgr_pdev *pdev,
			      struct cc_regdmn_s *rd);

/**
 * reg_set_regdb_offloaded() - set/clear regulatory offloaded flag
 *
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS reg_set_regdb_offloaded(struct wlan_objmgr_psoc *psoc, bool val);

/**
 * reg_get_curr_regdomain() - Get current regdomain in use
 * @pdev: pdev pointer
 * @cur_regdmn: Current regdomain info
 *
 * Return: QDF status
 */
QDF_STATUS reg_get_curr_regdomain(struct wlan_objmgr_pdev *pdev,
				  struct cur_regdmn_info *cur_regdmn);

/**
 * reg_modify_chan_144() - Enable/Disable channel 144
 * @pdev: pdev pointer
 * @en_chan_144: flag to disable/enable channel 144
 *
 * Return: Success or Failure
 */
QDF_STATUS reg_modify_chan_144(struct wlan_objmgr_pdev *pdev, bool en_chan_144);

/**
 * reg_get_en_chan_144() - get en_chan_144 flag value
 * @pdev: pdev pointer
 *
 * Return: en_chan_144 flag value
 */
bool reg_get_en_chan_144(struct wlan_objmgr_pdev *pdev);

/**
 * reg_get_hal_reg_cap() - Get HAL REG capabilities
 * @psoc: psoc for country information
 *
 * Return: hal reg cap pointer
 */
struct wlan_psoc_host_hal_reg_capabilities_ext *reg_get_hal_reg_cap(
		struct wlan_objmgr_psoc *psoc);

/**
 * reg_set_hal_reg_cap() - Set HAL REG capabilities
 * @psoc: psoc for country information
 * @reg_cap: Regulatory caps pointer
 * @phy_cnt: number of phy
 *
 * Return: hal reg cap pointer
 */
QDF_STATUS reg_set_hal_reg_cap(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap,
		uint16_t phy_cnt);

/**
 * reg_update_hal_reg_cap() - Update HAL REG capabilities
 * @psoc: psoc pointer
 * @wireless_modes: 11AX wireless modes
 * @phy_id: phy id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_update_hal_reg_cap(struct wlan_objmgr_psoc *psoc,
				  uint32_t wireless_modes, uint8_t phy_id);

/**
 * reg_chan_in_range() - Check if the given channel is in pdev's channel range
 * @chan_list: Pointer to regulatory channel list.
 * @low_freq_2g: Low frequency 2G.
 * @high_freq_2g: High frequency 2G.
 * @low_freq_5g: Low frequency 5G.
 * @high_freq_5g: High frequency 5G.
 * @ch_enum: Channel enum.
 *
 * Return: true if ch_enum is with in pdev's channel range, else false.
 */
bool reg_chan_in_range(struct regulatory_channel *chan_list,
		       qdf_freq_t low_freq_2g, qdf_freq_t high_freq_2g,
		       qdf_freq_t low_freq_5g, qdf_freq_t high_freq_5g,
		       enum channel_enum ch_enum);

/**
 * reg_init_channel_map() - Initialize the channel list based on the dfs region.
 * @dfs_region: Dfs region
 */
void reg_init_channel_map(enum dfs_reg dfs_region);

/**
 * reg_get_psoc_tx_ops() - Get regdb tx ops
 * @psoc: Pointer to psoc structure
 */
struct wlan_lmac_if_reg_tx_ops *reg_get_psoc_tx_ops(
	struct wlan_objmgr_psoc *psoc);

#ifdef CONFIG_CHAN_NUM_API
/**
 * reg_update_nol_history_ch() - Set nol-history flag for the channels in the
 * list.
 * @pdev: Pdev ptr.
 * @ch_list: Input channel list.
 * @num_ch: Number of channels.
 * @nol_history_ch: NOL-History flag.
 *
 * Return: void
 */
void reg_update_nol_history_ch(struct wlan_objmgr_pdev *pdev,
			       uint8_t *chan_list,
			       uint8_t num_chan,
			       bool nol_history_chan);

/**
 * reg_is_24ghz_ch() - Check if the given channel number is 2.4GHz
 * @chan: Channel number
 *
 * Return: true if channel number is 2.4GHz, else false
 */
bool reg_is_24ghz_ch(uint32_t chan);

/**
 * reg_is_5ghz_ch() - Check if the given channel number is 5GHz
 * @chan: Channel number
 *
 * Return: true if channel number is 5GHz, else false
 */
bool reg_is_5ghz_ch(uint32_t chan);
#endif /* CONFIG_CHAN_NUM_API */

/**
 * reg_is_24ghz_ch_freq() - Check if the given channel frequency is 2.4GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 2.4GHz, else false
 */
bool reg_is_24ghz_ch_freq(uint32_t freq);

/**
 * reg_is_5ghz_ch_freq() - Check if the given channel frequency is 5GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 5GHz, else false
 */
bool reg_is_5ghz_ch_freq(uint32_t freq);

/**
 * reg_is_range_overlap_2g() - Check if the given low_freq and high_freq
 * is in the 2G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 2G range,
 * else false.
 */
bool reg_is_range_overlap_2g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * reg_is_range_overlap_5g() - Check if the given low_freq and high_freq
 * is in the 5G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 5G range,
 * else false.
 */
bool reg_is_range_overlap_5g(qdf_freq_t low_freq, qdf_freq_t high_freq);

#ifdef CONFIG_BAND_6GHZ
/**
 * reg_is_6ghz_chan_freq() - Check if the given channel frequency is 6GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 6GHz, else false
 */
bool reg_is_6ghz_chan_freq(uint16_t freq);

#ifdef CONFIG_6G_FREQ_OVERLAP
/**
 * reg_is_range_only6g() - Check if the given low_freq and high_freq is only in
 * the 6G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps only the 6G
 * range, else false.
 */
bool reg_is_range_only6g(qdf_freq_t low_freq, qdf_freq_t high_freq);

/**
 * reg_is_range_overlap_6g() - Check if the given low_freq and high_freq
 * is in the 6G range.
 *
 * @low_freq - Low frequency.
 * @high_freq - High frequency.
 *
 * Return: Return true if given low_freq and high_freq overlaps 6G range,
 * else false.
 */
bool reg_is_range_overlap_6g(qdf_freq_t low_freq, qdf_freq_t high_freq);
#endif

/**
 * REG_IS_6GHZ_FREQ() - Check if the given channel frequency is 6GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 6GHz, else false
 */
static inline bool REG_IS_6GHZ_FREQ(uint16_t freq)
{
	return ((freq >= REG_MIN_6GHZ_CHAN_FREQ) &&
		(freq <= REG_MAX_6GHZ_CHAN_FREQ));
}

/**
 * reg_is_6ghz_psc_chan_freq() - Check if the given 6GHz channel frequency is
 * preferred scanning channel frequency.
 * @freq: Channel frequency
 *
 * Return: true if given 6GHz channel frequency is preferred scanning channel
 * frequency, else false
 */
bool reg_is_6ghz_psc_chan_freq(uint16_t freq);

/**
 * reg_is_6g_freq_indoor() - Check if a 6GHz frequency is indoor.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 *
 * Return: Return true if a 6GHz frequency is indoor, else false.
 */
bool reg_is_6g_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_get_max_txpower_for_6g_tpe() - Get max txpower for 6G TPE IE.
 * @pdev: Pointer to pdev.
 * @freq: Channel frequency.
 * @bw: Channel bandwidth.
 * @reg_ap: Regulatory 6G AP type.
 * @reg_client: Regulatory 6G client type.
 * @is_psd: True if txpower is needed in PSD format, and false if needed in EIRP
 * format.
 * @tx_power: Pointer to tx-power.
 *
 * Return: Return QDF_STATUS_SUCCESS, if tx_power is filled for 6G TPE IE
 * else return QDF_STATUS_E_FAILURE.
 */
QDF_STATUS reg_get_max_txpower_for_6g_tpe(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq, uint8_t bw,
					  enum reg_6g_ap_type reg_ap,
					  enum reg_6g_client_type reg_client,
					  bool is_psd,
					  uint8_t *tx_power);

/**
 * reg_min_6ghz_chan_freq() - Get minimum 6GHz channel center frequency
 *
 * Return: Minimum 6GHz channel center frequency
 */
uint16_t reg_min_6ghz_chan_freq(void);

/**
 * reg_max_6ghz_chan_freq() - Get maximum 6GHz channel center frequency
 *
 * Return: Maximum 6GHz channel center frequency
 */
uint16_t reg_max_6ghz_chan_freq(void);
#else
static inline bool reg_is_6ghz_chan_freq(uint16_t freq)
{
	return false;
}

static inline bool
reg_is_6g_freq_indoor(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq)
{
	return false;
}

static inline QDF_STATUS
reg_get_max_txpower_for_6g_tpe(struct wlan_objmgr_pdev *pdev,
			       qdf_freq_t freq, uint8_t bw,
			       enum reg_6g_ap_type reg_ap,
			       enum reg_6g_client_type reg_client,
			       bool is_psd,
			       uint8_t *tx_power)
{
	return QDF_STATUS_E_FAILURE;
}

#ifdef CONFIG_6G_FREQ_OVERLAP
static inline bool reg_is_range_overlap_6g(qdf_freq_t low_freq,
					   qdf_freq_t high_freq)
{
	return false;
}

static inline bool reg_is_range_only6g(qdf_freq_t low_freq,
				       qdf_freq_t high_freq)
{
	return false;
}
#endif

static inline bool REG_IS_6GHZ_FREQ(uint16_t freq)
{
	return false;
}

static inline bool reg_is_6ghz_psc_chan_freq(uint16_t freq)
{
	return false;
}

static inline uint16_t reg_min_6ghz_chan_freq(void)
{
	return 0;
}

static inline uint16_t reg_max_6ghz_chan_freq(void)
{
	return 0;
}
#endif /* CONFIG_BAND_6GHZ */

/**
 * reg_get_band_channel_list() - Get the channel list and number of channels
 * @pdev: pdev ptr
 * @band_mask: Input bitmap with band set
 * @channel_list: Pointer to Channel List
 *
 * Get the given channel list and number of channels from the current channel
 * list based on input band bitmap.
 *
 * Return: Number of channels, else 0 to indicate error
 */
uint16_t reg_get_band_channel_list(struct wlan_objmgr_pdev *pdev,
				   uint8_t band_mask,
				   struct regulatory_channel *channel_list);

/**
 * reg_chan_band_to_freq - Return channel frequency based on the channel number
 * and band.
 * @pdev: pdev ptr
 * @chan: Channel Number
 * @band_mask: Bitmap for bands
 *
 * Return: Return channel frequency or return 0, if the channel is disabled or
 * if the input channel number or band_mask is invalid. Composite bands are
 * supported only for 2.4Ghz and 5Ghz bands. For other bands the following
 * priority is given: 1) 6Ghz 2) 5Ghz 3) 2.4Ghz.
 */
qdf_freq_t reg_chan_band_to_freq(struct wlan_objmgr_pdev *pdev,
				 uint8_t chan,
				 uint8_t band_mask);

/**
 * reg_is_49ghz_freq() - Check if the given channel frequency is 4.9GHz
 * @freq: Channel frequency
 *
 * Return: true if channel frequency is 4.9GHz, else false
 */
bool reg_is_49ghz_freq(qdf_freq_t freq);

/**
 * reg_ch_num() - Get channel number from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel number
 */
qdf_freq_t reg_ch_num(uint32_t ch_enum);

/**
 * reg_ch_to_freq() - Get channel frequency from channel enum
 * @ch_enum: Channel enum
 *
 * Return: channel frequency
 */
qdf_freq_t reg_ch_to_freq(uint32_t ch_enum);

#ifdef CONFIG_CHAN_NUM_API
/**
 * reg_is_same_band_channels() - Check if given channel numbers have same band
 * @chan_num1: Channel number1
 * @chan_num2: Channel number2
 *
 * Return: true if both the channels has the same band.
 */
bool reg_is_same_band_channels(uint8_t chan_num1, uint8_t chan_num2);

/**
 * reg_is_channel_valid_5g_sbs() Check if the given channel is 5G SBS.
 * @curchan: current channel
 * @newchan:new channel
 *
 * Return: true if the given channel is a valid 5G SBS
 */
bool reg_is_channel_valid_5g_sbs(uint8_t curchan, uint8_t newchan);

/**
 * reg_min_24ghz_ch_num() - Get minimum 2.4GHz channel number
 *
 * Return: Minimum 2.4GHz channel number
 */
uint8_t reg_min_24ghz_ch_num(void);

/**
 * reg_max_24ghz_ch_num() - Get maximum 2.4GHz channel number
 *
 * Return: Maximum 2.4GHz channel number
 */
uint8_t reg_max_24ghz_ch_num(void);

/**
 * reg_min_5ghz_ch_num() - Get minimum 5GHz channel number
 *
 * Return: Minimum 5GHz channel number
 */
uint8_t reg_min_5ghz_ch_num(void);

/**
 * reg_max_5ghz_ch_num() - Get maximum 5GHz channel number
 *
 * Return: Maximum 5GHz channel number
 */
uint8_t reg_max_5ghz_ch_num(void);
#endif /* CONFIG_CHAN_NUM_API */

#ifdef CONFIG_CHAN_FREQ_API
/**
 * reg_min_24ghz_chan_freq() - Get minimum 2.4GHz channel frequency
 *
 * Return: Minimum 2.4GHz channel frequency
 */
qdf_freq_t reg_min_24ghz_chan_freq(void);

/**
 * reg_max_24ghz_chan_freq() - Get maximum 2.4GHz channel frequency
 *
 * Return: Maximum 2.4GHz channel frequency
 */
qdf_freq_t reg_max_24ghz_chan_freq(void);

/**
 * reg_min_5ghz_chan_freq() - Get minimum 5GHz channel frequency
 *
 * Return: Minimum 5GHz channel frequency
 */
qdf_freq_t reg_min_5ghz_chan_freq(void);

/**
 * reg_max_5ghz_chan_freq() - Get maximum 5GHz channel frequency
 *
 * Return: Maximum 5GHz channel frequency
 */
qdf_freq_t reg_max_5ghz_chan_freq(void);
#endif /* CONFIG_CHAN_FREQ_API */

/**
 * reg_enable_dfs_channels() - Enable the use of DFS channels
 * @pdev: The physical dev to enable/disable DFS channels for
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_enable_dfs_channels(struct wlan_objmgr_pdev *pdev, bool enable);

/**
 * reg_is_regdmn_en302502_applicable() - Find if ETSI EN302_502 radar pattern
 * is applicable in current regulatory domain.
 * @pdev: Pdev object pointer.
 *
 * Return: True if en302_502 is applicable, else false.
 */
bool reg_is_regdmn_en302502_applicable(struct wlan_objmgr_pdev *pdev);

/**
 * reg_modify_pdev_chan_range() - Compute current channel list
 * in accordance with the modified reg caps.
 * @pdev: The physical dev for which channel list must be built.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_modify_pdev_chan_range(struct wlan_objmgr_pdev *pdev);

/**
 * reg_update_pdev_wireless_modes() - Update the wireless_modes in the
 * pdev_priv_obj with the input wireless_modes
 * @pdev: pointer to wlan_objmgr_pdev.
 * @wireless_modes: Wireless modes.
 *
 * Return : QDF_STATUS
 */
QDF_STATUS reg_update_pdev_wireless_modes(struct wlan_objmgr_pdev *pdev,
					  uint32_t wireless_modes);

/**
 * reg_get_phybitmap() - Get phybitmap from regulatory pdev_priv_obj
 * @pdev: pdev pointer
 * @phybitmap: pointer to phybitmap
 *
 * Return: QDF STATUS
 */
QDF_STATUS reg_get_phybitmap(struct wlan_objmgr_pdev *pdev,
			     uint16_t *phybitmap);
#ifdef DISABLE_UNII_SHARED_BANDS
/**
 * reg_disable_chan_coex() - Disable Coexisting channels based on the input
 * bitmask.
 * @pdev: pointer to wlan_objmgr_pdev.
 * unii_5g_bitmap: UNII 5G bitmap.
 *
 * Return : QDF_STATUS
 */
QDF_STATUS reg_disable_chan_coex(struct wlan_objmgr_pdev *pdev,
				 uint8_t unii_5g_bitmap);
#endif

#ifdef CONFIG_CHAN_FREQ_API
/**
 * reg_is_freq_present_in_cur_chan_list() - Check the input frequency
 * @pdev: Pointer to pdev
 * @freq: Channel center frequency in MHz
 *
 * Check if the input channel center frequency is present in the current
 * channel list
 *
 * Return: Return true if channel center frequency is present in the current
 * channel list, else return false.
 */
bool
reg_is_freq_present_in_cur_chan_list(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq);

/**
 * reg_get_chan_enum_for_freq() - Get channel enum for given channel frequency
 * @freq: Channel Frequency
 *
 * Return: Channel enum
 */
enum channel_enum reg_get_chan_enum_for_freq(qdf_freq_t freq);

/**
 * reg_get_channel_list_with_power_for_freq() - Provides the channel list with
 * power
 * @pdev: Pointer to pdev
 * @ch_list: Pointer to the channel list.
 * @num_chan: Pointer to save number of channels
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
reg_get_channel_list_with_power_for_freq(struct wlan_objmgr_pdev *pdev,
					 struct channel_power *ch_list,
					 uint8_t *num_chan);

/**
 * reg_get_channel_state_for_freq() - Get channel state from regulatory
 * @pdev: Pointer to pdev
 * @freq: channel center frequency.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
						  qdf_freq_t freq);

/**
 * reg_get_5g_bonded_channel_state_for_freq() - Get channel state for
 * 5G bonded channel using the channel frequency
 * @pdev: Pointer to pdev
 * @freq: channel center frequency.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state
reg_get_5g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t freq,
					 enum phy_ch_width bw);

/**
 * reg_get_2g_bonded_channel_state_for_freq() - Get channel state for 2G
 * bonded channel
 * @freq: channel center frequency.
 * @pdev: Pointer to pdev
 * @oper_ch_freq: Primary channel center frequency
 * @sec_ch_freq: Secondary channel center frequency
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state
reg_get_2g_bonded_channel_state_for_freq(struct wlan_objmgr_pdev *pdev,
					 qdf_freq_t oper_ch_freq,
					 qdf_freq_t sec_ch_freq,
					 enum phy_ch_width bw);

/**
 * reg_set_channel_params_for_freq () - Sets channel parameteres for given
 * bandwidth
 * @pdev: Pointer to pdev
 * @freq: Channel center frequency.
 * @sec_ch_2g_freq: Secondary 2G channel frequency
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void reg_set_channel_params_for_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq,
				     qdf_freq_t sec_ch_2g_freq,
				     struct ch_params *ch_params);

/**
 * reg_get_channel_reg_power_for_freq() - Get the txpower for the given channel
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: txpower
 */
uint8_t reg_get_channel_reg_power_for_freq(struct wlan_objmgr_pdev *pdev,
					   qdf_freq_t freq);

/**
 * reg_update_nol_ch_for_freq () - Updates NOL channels in current channel list
 * @pdev: pointer to pdev object
 * @chan_freq_list: pointer to NOL channel list
 * @num_ch: No.of channels in list
 * @update_nol: set/reset the NOL status
 *
 * Return: None
 */
void reg_update_nol_ch_for_freq(struct wlan_objmgr_pdev *pdev,
				uint16_t *chan_freq_list,
				uint8_t num_chan,
				bool nol_chan);
/**
 * reg_is_dfs_for_freq () - Checks the channel state for DFS
 * @pdev: pdev ptr
 * @freq: Channel center frequency
 *
 * Return: true or false
 */
bool reg_is_dfs_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_chan_freq_is_49ghz() - Check if the input channel center frequency is
 * 4.9GHz
 * @pdev: Pdev pointer
 * @chan_num: Input channel center frequency
 *
 * Return: true if the frequency is 4.9GHz else false.
 */
bool reg_chan_freq_is_49ghz(qdf_freq_t freq);

/**
 * reg_update_nol_history_ch_for_freq() - Set nol-history flag for the channels
 * in the list.
 * @pdev: Pdev ptr.
 * @chan_list: Input channel freqeuncy list.
 * @num_ch: Number of channels.
 * @nol_history_ch: NOL-History flag.
 *
 * Return: void
 */
void reg_update_nol_history_ch_for_freq(struct wlan_objmgr_pdev *pdev,
					uint16_t *chan_list,
					uint8_t num_chan,
					bool nol_history_chan);

/**
 * reg_is_same_5g_band_freqs() - Check if given channel center
 * frequencies have same band
 * @freq1: Channel Center Frequency 1
 * @freq2: Channel Center Frequency 2
 *
 * Return: true if both the frequencies has the same band.
 */
bool reg_is_same_band_freqs(qdf_freq_t freq1, qdf_freq_t freq2);

/**
 * reg_is_frequency_valid_5g_sbs() Check if the given frequency is 5G SBS.
 * @curfreq: current channel frequency
 * @newfreq: new channel center frequency
 *
 * Return: true if the given center frequency is a valid 5G SBS
 */
bool reg_is_frequency_valid_5g_sbs(qdf_freq_t curfreq, qdf_freq_t newfreq);

/**
 * reg_freq_to_band() - Get band from channel frequency
 * @chan_num: channel frequency
 *
 * Return: wifi band
 */
enum reg_wifi_band reg_freq_to_band(qdf_freq_t freq);

/**
 * reg_min_chan_freq() - minimum channel frequency supported
 *
 * Return: channel frequency
 */
qdf_freq_t reg_min_chan_freq(void);

/**
 * reg_max_chan_freq() - maximum channel frequency supported
 *
 * Return: channel frequency
 */
qdf_freq_t reg_max_chan_freq(void);

/**
 * reg_get_5g_bonded_channel_for_freq()- Return the channel state for a
 * 5G or 6G channel frequency based on the channel width and bonded channel
 * @pdev: Pointer to pdev.
 * @freq: Channel center frequency.
 * @ch_width: Channel Width.
 * @bonded_chan_ptr_ptr: Pointer to bonded_channel_freq.
 *
 * Return: Channel State
 */
enum channel_state
reg_get_5g_bonded_channel_for_freq(struct wlan_objmgr_pdev *pdev,
				   uint16_t freq,
				   enum phy_ch_width ch_width,
				   const struct bonded_channel_freq
				   **bonded_chan_ptr_ptr);

/**
 * reg_is_disable_for_freq() - Check if the given channel frequency in
 * disable state
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: True if channel state is disabled, else false
 */
bool reg_is_disable_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_is_passive_for_freq() - Check if the given channel frequency is in
 * passive state
 * @pdev: Pointer to pdev
 * @freq: Channel frequency
 *
 * Return: True if channel state is passive, else false
 */
bool reg_is_passive_for_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);
#endif /* CONFIG_CHAN_FREQ_API */

/**
 * reg_get_max_tx_power() - Get maximum tx power from the current channel list
 * @pdev: Pointer to pdev
 *
 * Return: return the value of the maximum tx power in the current channel list
 *
 */
uint8_t reg_get_max_tx_power(struct wlan_objmgr_pdev *pdev);

/**
 * reg_set_ignore_fw_reg_offload_ind() - Set if regdb offload indication
 * needs to be ignored
 * @psoc: Pointer to psoc
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_ignore_fw_reg_offload_ind(struct wlan_objmgr_psoc *psoc);

/**
 * reg_get_ignore_fw_reg_offload_ind() - Check whether regdb offload indication
 * needs to be ignored
 *
 * @psoc: Pointer to psoc
 */
bool reg_get_ignore_fw_reg_offload_ind(struct wlan_objmgr_psoc *psoc);

/**
 * reg_set_6ghz_supported() - Set if 6ghz is supported
 *
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS reg_set_6ghz_supported(struct wlan_objmgr_psoc *psoc,
				  bool val);

/**
 * reg_set_5dot9_ghz_supported() - Set if 5.9ghz is supported
 *
 * @psoc: Pointer to psoc
 * @val: value
 */
QDF_STATUS reg_set_5dot9_ghz_supported(struct wlan_objmgr_psoc *psoc,
				       bool val);

/**
 * reg_is_6ghz_op_class() - Check whether 6ghz oper class
 *
 * @pdev: Pointer to pdev
 * @op_class: oper class
 */
bool reg_is_6ghz_op_class(struct wlan_objmgr_pdev *pdev,
			  uint8_t op_class);

#ifdef CONFIG_REG_CLIENT
/**
 * reg_is_6ghz_supported() - Whether 6ghz is supported
 *
 * @psoc: pointer to psoc
 */
bool reg_is_6ghz_supported(struct wlan_objmgr_psoc *psoc);
#endif

/**
 * reg_is_5dot9_ghz_supported() - Whether 5.9ghz is supported
 *
 * @psoc: pointer to psoc
 */
bool reg_is_5dot9_ghz_supported(struct wlan_objmgr_psoc *psoc);

/**
 * reg_is_fcc_regdmn () - Checks if the current reg domain is FCC3/FCC8/FCC15/
 * FCC16 or not
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_fcc_regdmn(struct wlan_objmgr_pdev *pdev);

/**
 * reg_is_5dot9_ghz_freq () - Checks if the frequency is 5.9 GHz freq or not
 * @freq: frequency
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_5dot9_ghz_freq(struct wlan_objmgr_pdev *pdev, qdf_freq_t freq);

/**
 * reg_is_5dot9_ghz_chan_allowed_master_mode () - Checks if 5.9 GHz channels
 * are allowed in master mode or not.
 *
 * @pdev: pdev ptr
 *
 * Return: true or false
 */
bool reg_is_5dot9_ghz_chan_allowed_master_mode(struct wlan_objmgr_pdev *pdev);

/**
 * reg_get_unii_5g_bitmap() - get unii_5g_bitmap value
 * @pdev: pdev pointer
 * @bitmap: Pointer to retrieve the unii_5g_bitmap of enum reg_unii_band
 *
 * Return: QDF_STATUS
 */
#ifdef DISABLE_UNII_SHARED_BANDS
QDF_STATUS
reg_get_unii_5g_bitmap(struct wlan_objmgr_pdev *pdev, uint8_t *bitmap);
#endif

#ifdef CHECK_REG_PHYMODE
/**
 * reg_get_max_phymode() - Recursively find the best possible phymode given a
 * phymode, a frequency, and per-country regulations
 * @pdev: pdev pointer
 * @phy_in: phymode that the user requested
 * @freq: current operating center frequency
 *
 * Return: maximum phymode allowed in current country that is <= phy_in
 */
enum reg_phymode reg_get_max_phymode(struct wlan_objmgr_pdev *pdev,
				     enum reg_phymode phy_in,
				     qdf_freq_t freq);
#else
static inline enum reg_phymode
reg_get_max_phymode(struct wlan_objmgr_pdev *pdev,
		    enum reg_phymode phy_in,
		    qdf_freq_t freq)
{
	return REG_PHYMODE_INVALID;
}
#endif /* CHECK_REG_PHYMODE */

#ifdef CONFIG_REG_CLIENT
/**
 * reg_band_bitmap_to_band_info() - Convert the band_bitmap to a band_info enum.
 *	Since band_info enum only has combinations for 2G and 5G, 6G is not
 *	considered in this function.
 * @band_bitmap: bitmap on top of reg_wifi_band of bands enabled
 *
 * Return: BAND_ALL if both 2G and 5G band is enabled
 *	BAND_2G if 2G is enabled but 5G isn't
 *	BAND_5G if 5G is enabled but 2G isn't
 */
enum band_info reg_band_bitmap_to_band_info(uint32_t band_bitmap);
#endif

#endif

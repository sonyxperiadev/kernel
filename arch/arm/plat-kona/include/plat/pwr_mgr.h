/****************************************************************************
*
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef __KONA_POWER_MANAGER_H__
#define __KONA_POWER_MANAGER_H__

#include<mach/pwr_mgr.h>

#define KONA_PM_I2C_ENABLE		1
#define KONA_PM_I2C_DISABLE		0

#define PWRMGR_event_id_NEGEDGE_CONDITION_ENABLE_MASK	0x00000002
#define PWRMGR_event_id_POSEDGE_CONDITION_ENABLE_MASK	0x00000004
#define PWRMGR_event_id_CONDITION_ACTIVE_MASK			0x00000001

#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_VALUE_SHIFT 		28
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_VALUE_MASK 		0xF0000000
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_SHIFT 		22
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_MASK 		0x0FC00000
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_VALUE_SHIFT 		18
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_VALUE_MASK 		0x003C0000
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_SHIFT 		12
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_MASK 		0x0003F000
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_ZEROV_SHIFT 		6
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_ZEROV_MASK 		0x00000FC0
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_POINTER_SHIFT 		0
#define    PWRMGR_VO_SPECIFIC_I2C_COMMAND_POINTER_MASK 			0x0000003F

#define    PWRMGR_VI_MODEM_TO_VO_MAP_MASK                  0x00000010
#define    PWRMGR_VI_ARM_SUBSYSTEM_TO_VO_MAP_MASK          0x00000008
#define    PWRMGR_VI_HUB_TO_VO_MAP_MASK                    0x00000004
#define    PWRMGR_VI_MM_TO_VO_MAP_MASK                     0x00000002
#define    PWRMGR_VI_ARM_CORE_TO_VO_MAP_MASK               0x00000001

#define    PWRMGR_PC3_SW_OVERRIDE_VALUE_MASK      0x00080000
#define    PWRMGR_PC2_SW_OVERRIDE_VALUE_MASK      0x00040000
#define    PWRMGR_PC1_SW_OVERRIDE_VALUE_MASK      0x00020000
#define    PWRMGR_PC0_SW_OVERRIDE_VALUE_MASK      0x00010000
#define    PWRMGR_PC3_SW_OVERRIDE_ENABLE_MASK     0x00008000
#define    PWRMGR_PC2_SW_OVERRIDE_ENABLE_MASK     0x00004000
#define    PWRMGR_PC1_SW_OVERRIDE_ENABLE_MASK     0x00002000
#define    PWRMGR_PC0_SW_OVERRIDE_ENABLE_MASK     0x00001000
#define    PWRMGR_PC3_CLKREQ_OVERRIDE_VALUE_MASK  0x00000800
#define    PWRMGR_PC2_CLKREQ_OVERRIDE_VALUE_MASK  0x00000400
#define    PWRMGR_PC1_CLKREQ_OVERRIDE_VALUE_MASK  0x00000200
#define    PWRMGR_PC0_CLKREQ_OVERRIDE_VALUE_MASK  0x00000100
#define    PWRMGR_PC3_CLKREQ_OVERRIDE_ENABLE_MASK 0x00000080
#define    PWRMGR_PC2_CLKREQ_OVERRIDE_ENABLE_MASK 0x00000040
#define    PWRMGR_PC1_CLKREQ_OVERRIDE_ENABLE_MASK 0x00000020
#define    PWRMGR_PC0_CLKREQ_OVERRIDE_ENABLE_MASK 0x00000010
#define    PWRMGR_PC3_CURRENT_VALUE_MASK          0x00000008
#define    PWRMGR_PC2_CURRENT_VALUE_MASK          0x00000004
#define    PWRMGR_PC1_CURRENT_VALUE_MASK          0x00000002
#define    PWRMGR_PC0_CURRENT_VALUE_MASK          0x00000001

#define PWRMGR_PI_ON_COUNTER_ENABLE_MASK		0x80000000
#define PWRMGR_PI_ON_COUNTER_OVERFLOW_MASK		0x40000000
#define PWRMGR_PI_ON_COUNTER_MASK			0x3FFFFFFF

#if defined(CONFIG_KONA_PWRMGR_REV2)
#define PWRMGR_INTR_STATUS_OFFSET		0x40A0
#define PWRMGR_INTR_MASK_OFFSET			0x40A4
#define PWRMGR_EVENT_BANK1_OFFSET		0x40A8
#define PWRMGR_I2C_SW_CMD_CTRL_OFFSET		0x41B8

#define PWRMGR_I2C_SW_START_ADDR_SHIFT		16
#define PWRMGR_I2C_SW_START_ADDR_MASK		(0x3F << 16)
#define PWRMGR_I2C_REQ_BUSY_SHIFT		12
#define PWRMGR_I2C_REQ_BUSY_MASK		(1 << 12)
#define PWRMGR_I2C_REQ_TRG_SHIFT		8
#define PWRMGR_I2C_REQ_TRG_MASK			(0xF << 8)
#define PWRMGR_I2C_READ_DATA_SHIFT		0
#define PWRMGR_I2C_READ_DATA_MASK		0xFF

#endif /*CONFIG_KONA_PWRMGR_REV2 */

#if defined(DEBUG)
#define pwr_dbg printk
#else
#define pwr_dbg(format...)              \
	do {                            \
	    if (pwr_debug)          	\
		printk(format); 	\
	} while(0)
#endif

#ifndef PM_I2C_CMD_MAX
#define PM_I2C_CMD_MAX 64
#endif

#define EVENT_ID_ALL	0xFFFFFFFF

#define PM_POLICY_MASK	0x7

#define CCU_POLICY(p) ((p) & 0x3)
#define IS_ACTIVE_POLICY(p)	((p) & 0x4)
#define IS_SHUTDOWN_POLICY(p) ((p) == 0)
#define IS_RETN_POLICY(p) ((p) == 1)

/*I2C commands - 4 bits*/

enum {
	REG_ADDR = 0,		/*sets the next address for read/write data */
	REG_DATA = 1,		/*executes a write to I2C controller via APB */
	I2C_DATA = 2,		/*data to be written to PMU via I2C */
	I2C_VAR = 3,		/*data returned for voltage lookup (payload is the index to table) */
	WAIT_I2C_RETX = 4,	/*wait for retry from I2C control register */
	WAIT_I2C_STAT = 5,	/* wait for good status (loop until good) */
	WAIT_TIMER = 6,		/*wait for the number of clocks in the payload */
	END = 7,		/*stop and wait for new voltage request change */
	SET_PC_PINS = 8,	/*pc pins are set based on value and mask */
	SET_UPPER_DATA = 9,	/*sets the data in the upper byte of apb data bus */
#if defined(CONFIG_KONA_PWRMGR_REV2)
	READ_FIFO = 0xA,	/*Read I2C FIFO */
	SET_READ_DATA = 0xB,	/*Copy I2C read data to PWRMGR register */
#endif /*CONFIG_KONA_PWRMGR_REV2 */
	JUMP_VOLTAGE = 0xE,	/*jump to address based on current voltage request */
	JUMP = 0xF		/*jump to address defined in payload */
};

enum pm_policy {
	PM_POLICY_0,
	PM_POLICY_1,
	PM_POLICY_2,
	PM_POLICY_3,
	PM_POLICY_4,
	PM_POLICY_5,
	PM_POLICY_6,
	PM_POLICY_7
};

enum pm_trig_type {
	PM_TRIG_NONE = 0,
	PM_TRIG_POS_EDGE = (1 << 0),
	PM_TRIG_NEG_EDGE = (1 << 1),
	PM_TRIG_BOTH_EDGE = PM_TRIG_NEG_EDGE | PM_TRIG_POS_EDGE
};

enum v_set {
	VOLT0,
	VOLT1,
	VOLT2,
	V_SET_MAX
};

enum pc_pin {
	PC0,
	PC1,
	PC2,
	PC3
};

enum {
	PM_PMU_I2C = (1 << 0),
	PM_HW_SEM_NO_DFS_REQ = (1 << 1),	/*Don't request for turbo mode when acquiring HW SEM - test only */
#if defined(CONFIG_KONA_PWRMGR_REV2)
	PROCESS_EVENTS_ON_INTR = (1 << 2),
	I2C_SIMULATE_BURST_MODE = (1 << 3),
#endif
};

struct pm_policy_cfg {
	bool ac;
	bool atl;
	u32 policy;
};

struct i2c_cmd {
	u8 cmd;			/*4 bit command */
	u8 cmd_data;		/*8bit command data */
};

struct v0x_spec_i2c_cmd_ptr {
	u8 set2_val;
	u8 set2_ptr;
	u8 set1_val;
	u8 set1_ptr;
	u8 zerov_ptr;
	u8 other_ptr;
};

struct pm_special_event_range {
	u32 start;
	u32 end;
};

struct pwr_mgr_info {
	u32 flags;
	u16 num_pi;
	u32 base_addr;
	struct i2c_cmd *i2c_cmds;
	u32 num_i2c_cmds;
	u8 *i2c_var_data;
	u32 num_i2c_var_data;
	struct v0x_spec_i2c_cmd_ptr *i2c_cmd_ptr[V_SET_MAX];

	struct pm_special_event_range *special_event_list;
	u32 num_special_event_range;

#if defined(CONFIG_KONA_PWRMGR_REV2)
	u32 pwrmgr_intr;
	u32 i2c_rd_off;
	int i2c_rd_slv_id_off1;	/*slave id offset -  write reg address */
	int i2c_rd_slv_id_off2;	/*slave id offset - read reg value */
	int i2c_rd_reg_addr_off;
	int i2c_rd_nack_off;
	int i2c_rd_nack_jump_off;
	int i2c_rd_fifo_off;
	u32 i2c_wr_off;
	int i2c_wr_slv_id_off;
	int i2c_wr_reg_addr_off;
	int i2c_wr_val_addr_off;
	u32 i2c_seq_timeout;	/*timeout in ms */
#endif
};

#if defined(CONFIG_KONA_PWRMGR_REV2)
/*PWRMGR interrupts -  entries are based on offset*/
enum {
	PWRMGR_INTR_I2C_SW_SEQ,
	PWRMGR_INTR_EVENTS,
	PWRMGR_INTR_MAX,
	PWRMGR_INTR_ALL = 0xFF
};

#endif /*CONFIG_KONA_PWRMGR_REV2 */

int pwr_mgr_event_trg_enable(int event_id, int event_trg_type);
int pwr_mgr_get_event_trg_type(int event_id);
int pwr_mgr_event_clear_events(u32 event_start, u32 event_end);
bool pwr_mgr_is_event_active(int event_id);
int pwr_mgr_event_set(int event_id, int event_state);

int pwr_mgr_event_set_pi_policy(int event_id, int pi_id,
				const struct pm_policy_cfg *pm_policy_cfg);
int pwr_mgr_event_get_pi_policy(int event_id, int pi_id,
				struct pm_policy_cfg *pm_policy_cfg);

int pwr_mgr_set_pi_fixed_volt_map(int pi_id, bool activate);
int pwr_mgr_set_pi_vmap(int pi_id, int vmap, bool activate);

int pwr_mgr_pi_set_wakeup_override(int pi_id, bool value);
int pwr_mgr_set_pc_sw_override(int pc_pin, bool enable, int value);
int pwr_mgr_set_pc_clkreq_override(int pc_pin, bool enable, int value);
int pm_get_pc_value(int pc_pin);
int pm_mgr_pi_count_clear(bool clear);
int pwr_mgr_pi_counter_enable(int pi_id, bool enable);
int pwr_mgr_pi_counter_read(int pi_id, bool * over_flow);

int pwr_mgr_pm_i2c_sem_lock(void);
int pwr_mgr_pm_i2c_sem_unlock(void);
int pwr_mgr_pm_i2c_enable(bool enable);
int pwr_mgr_set_v0x_specific_i2c_cmd_ptr(int v0x,
					 const struct v0x_spec_i2c_cmd_ptr
					 *cmd_ptr);
int pwr_mgr_pm_i2c_cmd_write(const struct i2c_cmd *i2c_cmd, u32 num_cmds);
int pwr_mgr_pm_i2c_var_data_write(const u8 * var_data, int count);

int pwr_mgr_arm_core_dormant_enable(bool enable);
int pwr_mgr_pi_retn_clamp_enable(int pi_id, bool enable);
int pwr_mgr_ignore_power_ok_signal(bool ignore);
int pwr_mgr_ignore_dap_powerup_request(bool ignore);
int pwr_mgr_register_event_handler(u32 event_id,
				   void (*pwr_mgr_event_cb) (u32 event_id,
							     void *param),
				   void *param);
int pwr_mgr_unregister_event_handler(u32 event_id);
int pwr_mgr_process_events(u32 event_start, u32 event_end, int clear_event);
void pwr_mgr_init_sequencer(struct pwr_mgr_info *info);
int pwr_mgr_init(struct pwr_mgr_info *info);

#if defined(CONFIG_KONA_PWRMGR_REV2)
int pwr_mgr_mask_intr(u32 intr, bool mask);
int pwr_mgr_clr_intr_status(u32 intr);
int pwr_mgr_get_intr_status(u32 intr);
int pwr_mgr_pmu_reg_read(u8 reg_addr, u8 slave_id, u8 * reg_val);
int pwr_mgr_pmu_reg_write(u8 reg_addr, u8 slave_id, u8 reg_val);
int pwr_mgr_pmu_reg_read_mul(u8 reg_addr_start, u8 slave_id,
			     u8 count, u8 * reg_val);
int pwr_mgr_pmu_reg_write_mul(u8 reg_addr_start, u8 slave_id,
			      u8 count, u8 * reg_val);
#endif /*CONFIG_KONA_PWRMGR_REV2 */

#ifdef CONFIG_DEBUG_FS
int __init pwr_mgr_debug_init(u32 bmdm_pwr_base);
#endif
#endif /*__KONA_POWER_MANAGER_H__*/

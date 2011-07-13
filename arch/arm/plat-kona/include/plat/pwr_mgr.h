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

#define CCU_POLICY(policy) ((policy) & 0x3)

/*I2C commands - 4 bits*/

enum
{
	REG_ADDR 		= 0, /*sets the next address for read/write data*/
	REG_DATA 		= 1, /*executes a write to I2C controller via APB*/
	I2C_DATA		= 2, /*data to be written to PMU via I2C*/
	I2C_VAR 		= 3, /*data returned for voltage lookup (payload is the index to table)*/
	WAIT_I2C_RETX	= 4, /*wait for retry from I2C control register*/
	WAIT_I2C_STAT	= 5, /* wait for good status (loop until good)*/
	WAIT_TIMER		= 6, /*wait for the number of clocks in the payload*/
	END				= 7, /*stop and wait for new voltage request change*/
	SET_PC_PINS		= 8, /*pc pins are set based on value and mask*/
	SET_UPPER_DATA	= 9, 	/*sets the data in the upper byte of apb data bus*/
	JUMP_VOLTAGE	= 0xE, /*jump to address based on current voltage request*/
	JUMP			= 0xF  /*jump to address defined in payload*/
};

enum pm_policy
{
    PM_POLICY_0,
    PM_POLICY_1,
    PM_POLICY_2,
    PM_POLICY_3,
    PM_POLICY_4,
    PM_POLICY_5,
    PM_POLICY_6,
    PM_POLICY_7
};

enum pm_trig_type
{
	PM_TRIG_NONE		= 0,
	PM_TRIG_POS_EDGE 	= (1 << 0),
	PM_TRIG_NEG_EDGE 	= (1 << 1),
	PM_TRIG_BOTH_EDGE 	=  PM_TRIG_NEG_EDGE|PM_TRIG_POS_EDGE
};

enum v_set
{
	VOLT0,
	VOLT1,
	VOLT2
};

enum pc_pin
{
	PC0,
	PC1,
	PC2,
	PC3
};

enum
{
	PM_PMU_I2C = (1 << 0),

};

struct pm_policy_cfg
{
    bool ac;
    bool atl;
    u32 policy;
};

struct i2c_cmd
{
	u8 cmd; /*4 bit command*/
	u8 cmd_data; /*8bit command data */
};

struct v0x_spec_i2c_cmd_ptr
{
	u8 set2_val;
	u8 set2_ptr;
	u8 set1_val;
	u8 set1_ptr;
	u8 zerov_ptr;
	u8 other_ptr;
};

struct pwr_mgr_info
{
	u32 flags;
	u16	num_pi;
	u32 base_addr;
};

int pwr_mgr_event_trg_enable(int event_id,int event_trg_type);
int	pwr_mgr_event_clear_events(u32 event_start, u32 event_end);
bool pwr_mgr_is_event_active(int event_id);
int pwr_mgr_event_set(int event_id, int event_state);

int pwr_mgr_event_set_pi_policy(int event_id,int pi_id,const struct pm_policy_cfg* pm_policy_cfg);
int pwr_mgr_event_get_pi_policy(int event_id,int pi_id,struct pm_policy_cfg* pm_policy_cfg);

int pwr_mgr_set_pi_fixed_volt_map(int pi_id,bool activate);
int pwr_mgr_set_pi_vmap(int pi_id,int vmap, bool activate);

int	pwr_mgr_pi_set_wakeup_override(int pi_id,bool value);
int pwr_mgr_set_pc_sw_override(int pc_pin, bool enable, int value);
int pwr_mgr_set_pc_clkreq_override(int pc_pin, bool enable, int value);
int pm_get_pc_value(int pc_pin);

int pwr_mgr_pi_counter_enable(int pi_id, bool enable);
int pwr_mgr_pi_counter_read(int pi_id,bool* over_flow);

int pwr_mgr_request_pm_i2c_ownership(int value);
int pwr_mgr_verify_pm_i2c_ownership(void);
int pwr_mgr_pm_i2c_enable(bool enable);
int pwr_mgr_set_v0x_specific_i2c_cmd_ptr(int v0x, const struct v0x_spec_i2c_cmd_ptr* cmd_ptr);
int pwr_mgr_pm_i2c_cmd_write(const struct i2c_cmd* i2c_cmd , u32 num_cmds);
int pwr_mgr_pm_i2c_var_data_write(const u8* var_data,int count);

int	pwr_mgr_arm_core_dormant_enable(bool enable);
int	pwr_mgr_pi_retn_clamp_enable(int pi_id,bool enable);

int pwr_mgr_register_event_handler(u32 event_id, void (*pwr_mgr_event_cb)(u32 event_id,void* param),
											void* param);
int pwr_mgr_unregister_event_handler(u32 event_id);
int pwr_mgr_process_events(u32 event_start, u32 event_end, int clear_event);
int pwr_mgr_init(struct pwr_mgr_info* info);

#endif /*__KONA_POWER_MANAGER_H__*/

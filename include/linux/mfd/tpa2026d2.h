#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/i2c-kona.h>

#define TPA2026D2_I2C_ADDR 0xB0


#define TPA2026D2_PRINT_ERROR	(1U << 0)
#define TPA2026D2_PRINT_INIT	(1U << 1)
#define TPA2026D2_PRINT_FLOW	(1U << 2)
#define TPA2026D2_PRINT_DATA	(1U << 3)
#define TPA2026D2_PRINT_WARNING	(1U << 4)
#define TPA2026D2_PRINT_VERBOSE	(1U << 5)

#define TPA2026D2_REG_IC_FUNCTION 1
#define TPA2026D2_SPK_EN_R_SHIFT 7
#define TPA2026D2_SPK_EN_R_MASK (1 << TPA2026D2_SPK_EN_R_SHIFT)
#define TPA2026D2_SPK_EN_R (1 << TPA2026D2_SPK_EN_R_SHIFT)
#define TPA2026D2_SPK_EN_L_SHIFT 6
#define TPA2026D2_SPK_EN_L_MASK (1 << TPA2026D2_SPK_EN_L_SHIFT)
#define TPA2026D2_SPK_EN_L (1 << TPA2026D2_SPK_EN_L_SHIFT)



struct tpa2026d2_bus {
	char *name;
	struct i2c_client *i2c;
#ifdef CONFIG_DEBUG_FS
	struct dentry *dentry;
#endif	/*CONFIG_DEBUG_FS*/
struct mutex i2c_mutex;
};


struct tpa2026d2_platform_data {
	int i2c_bus_id;
	int shutdown_gpio;
};

struct tpa2026d2 {
	struct device *dev;
	struct tpa2026d2_bus *bus;
	struct tpa2026d2_platform_data *pdata;
#ifdef CONFIG_DEBUG_FS
	struct dentry *dent_tpa2026d2;
#endif	/*CONFIG_DEBUG_FS*/
	int is_on;
	int spk_l_on;
	int spk_r_on;
	struct mutex mutex;
};

extern void tpa2026d2_spk_power(int spk_l_on, int spk_r_on);



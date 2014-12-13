#include <linux/module.h>
#include <mach/msm_iomap.h>
#include <mach/cci_hw_id.h>

#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#ifdef HWID_USE_SHARED_MEMORY
#include <mach/msm_smsm.h>
#endif

static int cci_hw_id = CCI_HWID_INVALID;
static int cci_project_id = CCI_PROJECTID_INVALID;
static int cci_customer_id = CCI_CUSTOMERID_INVALID;
static int cci_sim_id = CCI_SIMID_INVALID;
static int cci_band_id = CCI_BANDID_INVALID;

#define CONFIG_CCI_HWID_READ_GPIO 1
#if CONFIG_CCI_HWID_READ_GPIO
static const char cci_hwid_gpio_label[][CCI_HWID_TYPE_STRLEN] =
{
	"gpio_project_id_2",
	"gpio_project_id_1",
	"gpio_sim_id",
	"gpio_band_id_1",
	"gpio_band_id_2",
	"gpio_hw_id_1",
	"gpio_hw_id_2",
	"gpio_hw_id_3",
	"gpio_customer_id"
};

static unsigned cci_hwid_gpio_table[NUM_CCI_HWID_GPIO] = {
	GPIO_CCI_PROJECT_ID_2,
	GPIO_CCI_PROJECT_ID_1,
	GPIO_CCI_SIM_ID,
	GPIO_CCI_BAND_ID_1,
	GPIO_CCI_BAND_ID_2,
	GPIO_CCI_HW_ID_1,
	GPIO_CCI_HW_ID_2,
	GPIO_CCI_HW_ID_3,
	GPIO_CCI_CUSTOMER_ID
};

enum cci_hwid_gpio_index {
	GPIO_INDEX_PROJ_ID = 0,
	GPIO_INDEX_SIM_ID = 2,
	GPIO_INDEX_BAND_ID = 3,
	GPIO_INDEX_HW_ID = 5,
	GPIO_INDEX_CUSTOMER_ID = 8
};

enum cci_hwid_gpio_num {
	GPIO_NUM_PROJ_ID = 2,
	GPIO_NUM_SIM_ID = 1,
	GPIO_NUM_BAND_ID = 2,
	GPIO_NUM_HW_ID = 3,
	GPIO_NUM_CUSTOMER_ID = 1
};
#endif

static const char cci_sim_type_str[][CCI_HWID_TYPE_STRLEN] = 
{
	"Single sim",
	"Dual sim",
	"Invalid"
};

static const char cci_board_type_str[][CCI_HWID_TYPE_STRLEN] = 
{
	"EVT board",
	"DVT1 board",
	"DVT1-1 board",
	"DVT2 board",
	"DVT3 board",
	"PVT board",
	"MP board",
	"Invalid"
};

static const char cci_band_type_str[][CCI_HWID_TYPE_STRLEN] = 
{
	"EU band",
	"US band",
	"Invalid"
};

static const char cci_proj_name_str[][CCI_HWID_TYPE_STRLEN] = 
{
	"VY52_53",
	"VY55_56",
	"VY58_59",
	"Undefined"
};

static const char cci_customer_type_str[][CCI_HWID_TYPE_STRLEN] = 
{
	"Customer",
	"CCI",
	"Invalid"
};

#ifdef HWID_USE_SHARED_MEMORY
static void cci_hwid_write_to_smem(void)	
{
	static unsigned int *cci_smem_hwid = NULL;
	
	printk(KERN_INFO "cci_hwid_write_to_smem enter\n");
	cci_smem_hwid = smem_alloc2(SMEM_ID_VENDOR0, sizeof(unsigned int));
	if (cci_smem_hwid) {
		*cci_smem_hwid = (unsigned int)(SET_CCI_HWID_VALUE(cci_hw_id) | 
										  SET_CCI_PROJECTID_VALUE(cci_project_id) | 
										  SET_CCI_SIMID_VALUE(cci_sim_id) | 
										  SET_CCI_BANDID_VALUE(cci_band_id));
		printk(KERN_INFO "cci_hwid_write_to_smem:Success!\n");
	} else {
		printk(KERN_INFO "cci_hwid_write_to_smem:Fail !!\n");
	}
	return;
}

static void cci_hwid_read_smem(void)
{
	unsigned hwid_smem_size;
	unsigned smem_value = 0;
	unsigned smem_hwid = 0;
	unsigned smem_projectid = 0;
	unsigned smem_simid = 0;
	unsigned smem_bandid = 0;


	printk(KERN_INFO "kernel SMEM_ID_VENDOR0 = %d\n", SMEM_ID_VENDOR0);
	smem_value = *(unsigned int *)(smem_get_entry(SMEM_ID_VENDOR0, &hwid_smem_size));
	printk(KERN_INFO "hwid check = %d\n", smem_value);

	smem_hwid = GET_CCI_HWID_VALUE(smem_value);
	smem_projectid = GET_CCI_PROJECTID_VALUE(smem_value);
	smem_simid = GET_CCI_SIMID_VALUE(smem_value);
	smem_bandid = GET_CCI_BANDID_VALUE(smem_value);
	
	printk(KERN_INFO "smem check hwid = %d, projectid = %d, simid = %d, bandid = %d\n", smem_hwid, smem_projectid, smem_simid, smem_bandid);
	
	return;
}
#endif

#if CONFIG_CCI_HWID_READ_GPIO
static int cci_hw_get_type(int gpio_index, int gpio_num) 
{
	int ret = -1, i, weight = 1, gpio_index_end, type=0;
	gpio_index_end = gpio_index+gpio_num;

	for(i = gpio_index; i < gpio_index_end; i++, weight*=2) {
		ret = gpio_request(cci_hwid_gpio_table[i], cci_hwid_gpio_label[i]);
		if (ret) {
			printk("Requesting %s: FAILED !!!!\n", cci_hwid_gpio_label[i]); 	
		}
		else {
			if (gpio_get_value(cci_hwid_gpio_table[i]) == 1) {
				type += weight;	
				//printk(KERN_ERR "In %s, Read gpio value=1\n", cci_hwid_gpio_label[i]);
			}
		}
		gpio_free(cci_hwid_gpio_table[i]);
	}

	return type;
}

static void config_cci_hwid_gpio(void)
{
	int ret = -1, i=0;

	for(i = 0; i < NUM_CCI_HWID_GPIO; i++) {
		ret = gpio_request(cci_hwid_gpio_table[i], cci_hwid_gpio_label[i]);
		if (ret) {
			printk("Requesting %s: FAILED !!!!\n", cci_hwid_gpio_label[i]);
		}
		else {
			gpio_tlmm_config(GPIO_CFG(cci_hwid_gpio_table[i], 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		}
		gpio_free(cci_hwid_gpio_table[i]);
	}
	printk(KERN_INFO "In %s, config cci hwid gpio done\n", "config_cci_hwid_gpio");
}

static void cci_hw_det_gpio(void) 
{
	int is_waterproof = 0;

	config_cci_hwid_gpio();
	cci_project_id  = cci_hw_get_type(GPIO_INDEX_PROJ_ID, GPIO_NUM_PROJ_ID);
	cci_sim_id 	  	 = cci_hw_get_type(GPIO_INDEX_SIM_ID, GPIO_NUM_SIM_ID);
	cci_band_id 	 = cci_hw_get_type(GPIO_INDEX_BAND_ID, GPIO_NUM_BAND_ID-1);
	cci_hw_id 		 = cci_hw_get_type(GPIO_INDEX_HW_ID, GPIO_NUM_HW_ID);
	cci_customer_id = cci_hw_get_type(GPIO_INDEX_CUSTOMER_ID, GPIO_NUM_CUSTOMER_ID);

	if (cci_project_id == CCI_PROJECTID_VY55_56) {
		cci_sim_id = CCI_SIMID_SS;
		is_waterproof = gpio_get_value(GPIO_CCI_WATERPROOF_ID);
		if (is_waterproof)
			cci_project_id = CCI_PROJECTID_VY58_59;
	}
	
	printk(KERN_INFO "In %s, Read CCI HW ID from GPIO\n", "cci_hw_det_gpio");
	printk(KERN_INFO "cci_project_id=%d, cci_sim_id=%d, cci_band_id=%d, cci_hw_id=%d, cci_customer_id=%d",
		cci_project_id, cci_sim_id, cci_band_id, cci_hw_id, cci_customer_id);
}
#else
static int __init cci_hw_id_from_cmdline(char *cci_hwid)
{
	sscanf(cci_hwid, "%d", &cci_hw_id);
 
	return 1;
}
__setup("hwid=", cci_hw_id_from_cmdline);

static int __init cci_project_id_from_cmdline(char *cci_projectid)
{
	sscanf(cci_projectid, "%d", &cci_project_id);

	return 1;
}
__setup("projectid=", cci_project_id_from_cmdline);

static int __init cci_sim_id_from_cmdline(char *cci_simid)
{
	sscanf(cci_simid, "%d", &cci_sim_id);

	return 1;
}
__setup("multisim=", cci_sim_id_from_cmdline);

static int __init cci_band_id_from_cmdline(char *cci_bandid)
{
	sscanf(cci_bandid, "%d", &cci_band_id);

	return 1;
}
__setup("bandid=", cci_band_id_from_cmdline);


static int __init cci_customer_id_from_cmdline(char *cci_customerid)
{
	sscanf(cci_customerid, "%d", &cci_customer_id);

	return 1;
}
__setup("customer=", cci_customer_id_from_cmdline);
#endif

int get_cci_hw_id(void)
{
	return cci_hw_id;
}
EXPORT_SYMBOL(get_cci_hw_id);

int get_cci_project_id(void)
{
	return cci_project_id;
}
EXPORT_SYMBOL(get_cci_project_id);

int get_cci_sim_id(void)
{
	return cci_sim_id;
}
EXPORT_SYMBOL(get_cci_sim_id);

int get_cci_band_id(void)
{
	return cci_band_id;
}
EXPORT_SYMBOL(get_cci_band_id);

int get_cci_customer_id(void)
{
	return cci_customer_id;
}
EXPORT_SYMBOL(get_cci_customer_id);

int cci_hw_sim_type_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
   int sim_type, len;
   sim_type = get_cci_sim_id();
   len = sprintf(page, "%s\n", cci_sim_type_str[ sim_type] );
   return len;
}

int cci_hw_proj_name_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
   int proj_type, len;
   proj_type = get_cci_project_id();
   len = sprintf(page, "%s\n", cci_proj_name_str[ proj_type] );
   return len;
}

int cci_hw_board_type_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
   int board_type, len;
   board_type = get_cci_hw_id();
   len = sprintf(page, "%s\n", cci_board_type_str[ board_type ] );
   return len;
}

int cci_hw_band_type_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
   int band_type, len;
   band_type = get_cci_band_id();
   if(band_type <= 1) {
      len = sprintf(page, "%s\n", cci_band_type_str[ band_type ] );
   } else {
      len = sprintf(page, "%s\n", cci_band_type_str[2] );
   }
   return len;
}

int cci_hw_customer_type_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
   int customer_type, len;
   customer_type = get_cci_customer_id();
   len = sprintf(page, "%s\n", cci_customer_type_str[ customer_type ] );
   return len;
}

int cci_hwid_info_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
   int len, hwid, projid, simid, bandid, customerid;

	hwid = get_cci_hw_id();
	projid = get_cci_project_id();
	simid = get_cci_sim_id();
	bandid = get_cci_band_id();
	customerid = get_cci_customer_id();

   len = sprintf(page, "%s=%d %s=%d %s=%d %s=%d %s=%d\n", "hwid", hwid, 
				"projectid", projid, "multisim", simid, "bandid", bandid, "customer", customerid);
   //printk(KERN_INFO "Read CCI HWID INFO from proc = %s!\n", page);
   return len;
}

static int __init cci_hw_id_init(void)
{
	int err = 0;
	printk(KERN_INFO "cci_hwid_init enter\n");

#if CONFIG_CCI_HWID_READ_GPIO
	cci_hw_det_gpio();
#endif

	create_proc_read_entry("cci_hw_sim_type", 0, NULL, cci_hw_sim_type_read, NULL);
	create_proc_read_entry("cci_hw_board_type", 0, NULL, cci_hw_board_type_read, NULL);
	create_proc_read_entry("cci_hw_band_type", 0, NULL, cci_hw_band_type_read, NULL);
	create_proc_read_entry("cci_hw_proj_type", 0, NULL, cci_hw_proj_name_read, NULL);
	create_proc_read_entry("cci_hw_customer_type", 0, NULL, cci_hw_customer_type_read, NULL);
	create_proc_read_entry("cci_hwid_info", 0, NULL, cci_hwid_info_read, NULL);

#ifdef HWID_USE_SHARED_MEMORY
	cci_hwid_write_to_smem();
	cci_hwid_read_smem();
#endif
	return err;
}

static void __exit cci_hw_id_exit(void)
{
	printk(KERN_INFO "cci_hwid_exit enter\n");
}

module_init(cci_hw_id_init);
module_exit(cci_hw_id_exit);

MODULE_DESCRIPTION("cci hardware ID driver");
MODULE_AUTHOR("Chewei Liang <chewei_liang@compalcomm.com>");
MODULE_LICENSE("GPL");

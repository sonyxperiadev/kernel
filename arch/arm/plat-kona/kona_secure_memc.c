/******************************************************************************/
/*                                                                            */
/*  Copyright 2012  Broadcom Corporation                                      */
/*                                                                            */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed    */
/*     to you under the terms of the GNU General Public License version 2     */
/*     (the GPL), available at                                                */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and     */
/*     to copy and distribute the resulting executable under terms of your    */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module. An     */
/*     independent module is a module which is not derived from this          */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine      */
/*     this software in any way with any other Broadcom software provided     */
/*     under a license other than the GPL, without Broadcom's express prior   */
/*     written consent.                                                       */
/*                                                                            */
/******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <plat/kona_secure_memc.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <asm/page.h>
#include <linux/broadcom/secure_memc_shared.h>


/* default master's map, when we add more masters, update the same.  */
static struct default_master_map memc_map[NUM_OF_ALLOWED_MASTERS + 1] = {
	{MASTER_FABRIC, 0x00, 0x00},
	{MASTER_A7, 0x00, 0x00},
	{MASTER_COMMS, 0x00, 0x00},
	{MASTER_MM, 0x00, 0x00},
	{INVALID, 0x1FFF, 0x1FFF},
};

u32 *memc_handle;


void add_time_stamp(struct kona_memc_logging *logging)
{
	struct timeval now;
	u32 temp, second, minute, hour;
	char time_stamp[256];

	do_gettimeofday(&now);
	temp = now.tv_sec;
	second = temp % 60;
	temp /= 60;
	minute = temp % 60;
	temp /= 60;
	hour = temp % 24;
	sprintf(time_stamp, "[%02u:%02u:%02u:%06d] ",
		hour, minute, second, now.tv_usec);

	temp = 0;
	while (time_stamp[temp] != '\0') {
		logging->memc_log[logging->memc_log_index &
			(PAGE_SIZE - 1)] = time_stamp[temp++];
		logging->memc_log_index++;
	}
}

void log_this(struct kona_memc_logging *logging, char *buf)
{
	while (*buf) {
		logging->memc_log[logging->memc_log_index &
		(PAGE_SIZE - 1)] = *buf;
		logging->memc_log_index++;
		buf++;
	}
}

/* write secure memc block: PORT_x_GID_x_cmp.  */
int write_portx_gidx_cmp(struct kona_secure_memc *memc_dev, u32 port_id,
			u32 group_num, char *name)
{
	#define GIDCMP_SIZE	(pdata->num_of_groups * sizeof(u32))
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	u32 addr, val;
	char log_buf[16];
	struct kona_memc_logging *logging = memc_dev->logging;

	addr = pdata->kona_s_memc_base + PORT0_GIDCMP0_OFFSET +
		(port_id * GIDCMP_SIZE) + (group_num * sizeof(u32));

	if (strcmp(name, "invalid"))
		val = (memc_map[port_id].axi_mask << 16) |
			(memc_map[port_id].axi_val);
	else
		val = (memc_map[NUM_OF_ALLOWED_MASTERS].axi_mask << 16) |
			(memc_map[NUM_OF_ALLOWED_MASTERS].axi_val);

	add_time_stamp(logging);
	log_this(logging, name); log_this(logging, " to port");
	sprintf(log_buf, "%d", port_id); log_this(logging, log_buf);
	log_this(logging, ":group"); sprintf(log_buf, "%d\n", group_num);
	log_this(logging, log_buf);

	writel(val, addr);

	return 0;
}

/* read secure memc block: PORT_x_GID_x_cmp.  */
int read_portx_gidx_cmp(struct kona_secure_memc *memc_dev, u32 group_num)
{
	return 0;
}

/* write secure memc block: region registers.  */

int write_region(struct kona_secure_memc *memc_dev, u32 region_num,
		enum region_parms what, u32 val)
{
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_logging *logging = memc_dev->logging;
	u32 addr, ctrl;
	char log_buf[16];

	add_time_stamp(logging);
	log_this(logging, "region");
	sprintf(log_buf, "%d", region_num);
	log_this(logging, log_buf);

	switch (what) {
	case START_ADDR:
		addr = pdata->kona_s_memc_base + REGION0_CTL_OFFSET +
		(REGION0_CTL_OFFSET * region_num) +
		REGION0_START_OFFSET;
		log_this(logging, " start_addr:");
		break;
	case END_ADDR:
		addr = pdata->kona_s_memc_base + REGION0_CTL_OFFSET +
		(REGION0_CTL_OFFSET * region_num) +
		REGION0_END_OFFSET;
		log_this(logging, " end_addr:");
		break;
	case MEM_TYPE:
		addr = pdata->kona_s_memc_base + REGION0_CTL_OFFSET +
		(REGION0_CTL_OFFSET * region_num);
		ctrl = readl(addr);
		ctrl |= (val << REGION_MEM_TYPE_SHIFT);
		val = ctrl;
		log_this(logging, " mem_type:");
		break;
	case GROUP_MASK:
		addr = pdata->kona_s_memc_base + REGION0_CTL_OFFSET +
		(REGION0_CTL_OFFSET * region_num);
		ctrl = readl(addr);
		ctrl = (ctrl & ~RGN_GID_SANDBOX_EN_MASK) |
			(val << RGN_GID_SANDBOX_EN_SHIFT);
		val = ctrl;
		log_this(logging, " group_addr:");
		break;
	case REGION_EN:
		addr = pdata->kona_s_memc_base + REGION0_CTL_OFFSET +
		(REGION0_CTL_OFFSET * region_num);
		ctrl = readl(addr);
		ctrl |= val;
		val = ctrl;
		log_this(logging, " region_enable:");
		break;
	default:
		return -1;
		break;
	}

	sprintf(log_buf, "0x%X\n", val); log_this(logging, log_buf);
	writel(val, addr);
	return 0;
}

/* read secure memc block: region registers.  */

int read_regionx(struct kona_secure_memc *memc_dev, u32 region_num)
{
	return 0;
}

/* after all validation is done, add the master to the particular group.
    update relavent bitmaps as well.  */
static int memc_add_masters_to_group(struct kona_memc_port *memc_port,
				const const char *buf, u32 group_num)
{
	struct kona_memc_master *master;
	u32 *memc_state = &memc_port->memc_dev->memc_state;
	struct kona_memc_bitmaps *memc_bitmaps =
		&memc_port->memc_dev->memc_config_status;
	struct kona_memc_logging *logging = memc_port->memc_dev->logging;
	char log_buf[16];

	master = kzalloc(sizeof(struct kona_memc_master), GFP_KERNEL);
	if (!master)
		return -ENOMEM;

	master->name = kzalloc(strlen(buf), GFP_KERNEL);
	strncpy(master->name, buf, strlen(buf)-1);
	*(master->name+strlen(buf)) = '\0';

	add_time_stamp(logging);
	log_this(logging, master->name); log_this(logging, " added to gorup ");
	sprintf(log_buf, "%d\n", group_num); log_this(logging, log_buf);


	list_add(&master->group_master_list,
		&memc_port->group[group_num]->master_list);

	*memc_state |= GROUP_CONFIGURED;

	memc_bitmaps->port_map[memc_port->port_id][group_num] = 1;
	memc_bitmaps->group_bitmap |= (0x01 << group_num);
	memc_bitmaps->port_bitmap |= (0x01 << memc_port->port_id);
	return 0;
}

/* remove all the masters from a particular group, empty the group at the end
and update relevant bitmap as well.  */
static void memc_remove_masters_from_group(struct kona_memc_port
						*memc_port, u32 group_num)
{
	struct kona_secure_memc_pdata *pdata = memc_port->memc_dev->pdata;
	struct kona_memc_port **port = memc_port->memc_dev->port;
	struct kona_memc_bitmaps *memc_bitmaps =
				&memc_port->memc_dev->memc_config_status;
	u32 port_num;
	struct list_head *tmp, *head;
	u32 *memc_state = &memc_port->memc_dev->memc_state;
	struct kona_memc_logging *logging = memc_port->memc_dev->logging;
	struct kona_memc_master *master;
	char log_buf[16];

	head = &memc_port->group[group_num]->master_list;
	tmp = head->next;
	while (tmp != head) {
		master = container_of(tmp,
			struct kona_memc_master, group_master_list);
		tmp->prev->next = tmp->next;
		tmp->next->prev = tmp->prev;
		kfree(master);
		tmp = head->next;
	}

	add_time_stamp(logging);
	log_this(logging, "group ");
	sprintf(log_buf, "%d", group_num);
	log_this(logging, log_buf);
	log_this(logging, " is empty now\n");

	*memc_state &= (~GROUP_CONFIGURED);

	/* we set memc state, if atleast one port has been
	confugred for the same group. this is usually not valid
	until hardware path changes, and result into multiple paths
	and multiple master could come throuh same port. */
	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		tmp = &port[port_num]->group[group_num]->master_list;
		if (tmp->next != tmp) {
			*memc_state |= GROUP_CONFIGURED;
			break;
		}
	}

	memc_bitmaps->port_map[memc_port->port_id][group_num] = 0;
	memc_bitmaps->group_bitmap &= ~(0x01 << group_num);
	memc_bitmaps->port_bitmap &= ~(0x01 << memc_port->port_id);

}

/* validate against existing paltform data.  */

static int match_the_existing_masters(struct kona_secure_memc *memc_dev,
						const char *buf)
{
	u32 index;
	for (index = 0; index < NUM_OF_ALLOWED_MASTERS; index++) {
		if (!strncmp(buf, memc_dev->masters[index], strlen(buf)-1))
			return 0;
	}
	return 1;
}

/* validate masters with AXI id perspective. */

static int validate_masters_for_AXI_clash(struct kona_memc_port *memc_port,
						u32 group_num)
{
	struct kona_memc_port **port = memc_port->memc_dev->port;
	struct kona_secure_memc_pdata *pdata = memc_port->memc_dev->pdata;
	u32 port_num;
	struct list_head *tmp, *head;
	struct kona_memc_logging *logging = memc_port->memc_dev->logging;
	char log_buf[16];

	/* currently we do not allow multiple masters, and we do not want
	to as long as it is absolutely not needed, in extreme case add code
	here to validate multiple masters' possible axi ID clash.  */

	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		head = &port[port_num]->group[group_num]->master_list;
		tmp = port[port_num]->group[group_num]->master_list.next;
		if (tmp != head) {
			add_time_stamp(logging);
			log_this(logging, "already configured master for PORT");
			sprintf(log_buf, "%d/\n", port_num);
			log_this(logging, log_buf);
			sprintf(log_buf, "%d; ", group_num);
			log_this(logging, log_buf);
			log_this(logging, "remove it first\n");
			return 1;
		}
	}
	return 0;
}

/* restrict the master to get ccnfigured, which does not have valid port
access.  */
static int validate_masters_for_port_clash(struct kona_memc_port *memc_port,
					const const char *buf, u32 group_num)
{
	char *invalid_master;
	struct kona_memc_logging *logging = memc_port->memc_dev->logging;
	char log_buf[16];

	if (!strncmp(buf, memc_port->default_master, strlen(buf)-1))
		return 0;

	invalid_master = kzalloc(strlen(buf), GFP_KERNEL);
	if (!invalid_master) {
		pr_err("could not allocate invalid_master\n");
		return -ENOMEM;
	}
	strncpy(invalid_master, buf, strlen(buf)-1);
	*(invalid_master + strlen(invalid_master)) = '\0';

	add_time_stamp(logging);
	log_this(logging, invalid_master);
	log_this(logging, " does not have access to PORT");
	sprintf(log_buf , "%d,", memc_port->port_id);
	log_this(logging, log_buf);
	log_this(logging, "please have a look at 'cat memc_help'\n");
	kfree(invalid_master);
	return 1;
}

/* does folowing validation.
>	validate >
>		group number.
>		count.
>		user_buf_contents.
>		duplicate masters.
>		port acess.
>		axi id clash for multiple masters.
*/
static ssize_t align_memc_masters(struct kona_memc_port *memc_port,
	const char *buf, size_t count, u32 group_num)
{
	struct list_head *tmp, *head;
	struct kona_memc_master *master;
	struct kona_memc_port **port = memc_port->memc_dev->port;
	struct kona_secure_memc_pdata *pdata = memc_port->memc_dev->pdata;
	struct kona_memc_logging *logging = memc_port->memc_dev->logging;
	char log_buf[16];
	u32 port_num;
	char *invalid_master;

	if (group_num < 0)
		return -EINVAL;
	if (count <= 0)
		return -EINVAL;
	if (0 >= strlen(buf)-1)
		return -EINVAL;
	if (!strncmp(buf, DELETE_MASTER, strlen(buf)-1)) {
		memc_remove_masters_from_group(memc_port, group_num);
		return count;
	}
	if (match_the_existing_masters(memc_port->memc_dev, buf)) {
		invalid_master = kzalloc(strlen(buf), GFP_KERNEL);
		if (!invalid_master) {
			pr_err("could not allocate invalid_master\n");
			return -ENOMEM;
		}
		strncpy(invalid_master, buf, strlen(buf)-1);
		*(invalid_master + strlen(invalid_master)) = '\0';

		add_time_stamp(logging);
		log_this(logging, invalid_master);
		log_this(logging, " does not exist\n");

		kfree(invalid_master);
		return -EINVAL;
	}

	/* validate duplicate masters.  */

	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		head = &port[port_num]->group[group_num]->master_list;
		list_for_each(tmp, head) {
			master = container_of(tmp,
			struct kona_memc_master, group_master_list);
			if (!strncmp(buf, master->name, strlen(buf)-1)) {
				add_time_stamp(logging);
				log_this(logging, master->name);
				log_this(logging, " is already added");
				log_this(logging, " to the group ");
				sprintf(log_buf, "%d\n", group_num);
				log_this(logging, log_buf);
				return -EINVAL;
			}
		}
	}

	if (!validate_masters_for_port_clash(memc_port, buf, group_num)) {
		if (!validate_masters_for_AXI_clash(memc_port, group_num))
			memc_add_masters_to_group(memc_port, buf, group_num);
	}
	return count;
}


/* list the masters configured.  */
static ssize_t extract_masters_from_group(struct kona_memc_port *memc_port,
					const char *buf, u32 group_num)
{
	u32 num_of_chars = 0;
	struct kona_memc_master *master;
	struct list_head *tmp;

	if (group_num < 0)
		return -1;

	list_for_each(tmp, &memc_port->group[group_num]->master_list) {
		master = container_of(tmp,
		struct kona_memc_master, group_master_list);
		num_of_chars = num_of_chars + sprintf((char *)buf+num_of_chars,
		"%s\n", master->name);
	}
	return num_of_chars;
}

/* sysfs show masters method.  */
static ssize_t memc_port_group_show(struct kona_memc_port *memc_port,
					char *buf, u32 group_num)
{
	return extract_masters_from_group(memc_port, buf, group_num);
}

/* sysfs store masters method.  */

static ssize_t memc_port_group_store(struct kona_memc_port *memc_port,
				const char *buf, size_t count, u32 group_num)
{
	return align_memc_masters(memc_port, buf, count, group_num);
}


/* validate and write memc region registers.  */
static int write_memc_regions(struct kona_memc_regions *memc_region,
	const char *buf, enum region_parms what)
{
	u32 val;
	struct kona_memc_regions **region = memc_region->memc_dev->region;
	u32 *memc_state = &memc_region->memc_dev->memc_state;
	struct kona_memc_bitmaps *memc_bitmaps =
				&memc_region->memc_dev->memc_config_status;
	u32 region_num = memc_region->region_id;
	struct kona_memc_logging *logging = memc_region->memc_dev->logging;
	struct kona_secure_memc_pdata *pdata = memc_region->memc_dev->pdata;
	char log_buf[16];

	add_time_stamp(logging);

	if (START_ADDR == what || END_ADDR == what || GROUP_MASK == what)
		sscanf(buf, "%x", &val);
	else
		sscanf(buf, "%ud", &val);

	if (START_ADDR == what || END_ADDR == what) {
		if (val < pdata->ddr_start || val > pdata->ddr_end) {
			log_this(logging, "invalid DDR address\n");
			goto region_config_error;
		}
		if ((END_ADDR == what) && ((val & 0x00000FFF) != 0xFFF)) {
			log_this(logging,
			"invalid end address, last 3 nibbles shall be FFF\n");
			goto region_config_error;
		}
	} else if (MEM_TYPE == what) {
		if (MEM_TYPE_USR < val) {
			log_this(logging, "invalid memory type\n");
			goto region_config_error;
		}
	} else if (GROUP_MASK == what) {
		if ((val) > 0xFF) {
			log_this(logging, "invalid group number\n");
			goto region_config_error;
		}
	}

	/* we do not allow user to swicth to new region configuration,
	without finishing the previous one.  */
	if ((REGION_CONFIGURED & *memc_state)
		&& ((region_num + 1) !=
		((*memc_state & REGION_NUM_MASK) >> REGION_NUM_SHIFT))) {
			log_this(logging, "finish the region");
			sprintf(log_buf, "%d", ((*memc_state & REGION_NUM_MASK)
					>> REGION_NUM_SHIFT)-1);
			log_this(logging, log_buf);
			log_this(logging, " configuration first\n");
			goto region_config_error;
	}

	sprintf(log_buf, "%d", region_num);

	switch (what) {
	case START_ADDR:
		region[region_num]->start_address = val;
		*memc_state |= REGION_START_CONFIGURED;
		log_this(logging, "region_start for region");
		break;
	case END_ADDR:
		region[region_num]->end_address = val;
		*memc_state |= REGION_END_CONFIGURED;
		log_this(logging, "region_end for region");
		break;
	case MEM_TYPE:
		region[region_num]->mem_type = val;
		*memc_state |= REGION_MEM_TYPE_CONFIGURED;
		log_this(logging, "mem_type for region");
		break;
	case GROUP_MASK:
		region[region_num]->group_mask = val;
		*memc_state |= REGION_GROUP_MASK_CONFIGURED;
		log_this(logging, "group_mask for region");
		break;
	default:
		return -1;
		break;
	}

	/* when whole region is configured, set the states and bitmap
	accordingly.  */
	if (REGION_CONFIGURED == (*memc_state & REGION_CONFIGURED)
		|| ((*memc_state) & ((u32)(0x1 << region_num)
		<< CONFIGURED_REGION_NUM_SHIFT))) {
		*memc_state &= ~REGION_CONFIGURED;
		*memc_state |= COMPLETE_REGION_CONFIGURED;
		*memc_state = (*memc_state & ~REGION_NUM_MASK);
		*memc_state |= (*memc_state) |
		 ((u32)((0x1 << region_num) << CONFIGURED_REGION_NUM_SHIFT));
	}
	/* here we update which region is crrently being configured.  */
	else {
		*memc_state &= ~COMPLETE_REGION_CONFIGURED;
		*memc_state |= (*memc_state & REGION_NUM_MASK)
			| ((u32)(region_num + 1) << REGION_NUM_SHIFT);
	}

	log_this(logging, log_buf);
	log_this(logging, " configured successfully >");
	log_this(logging, " with the value:");
	sprintf(log_buf, "0x%x\n", val);
	log_this(logging, log_buf);
	memc_bitmaps->region_bitmap |= (0x01 << region_num);
	return 0;

region_config_error:
		return -1;
}

/* get the current/past status of memc.  */
int get_memc_activation_status(struct kona_memc_logging *logging,
					struct kona_memc_bitmaps *memc_state)
{
	if ((MEMC_WAS_ACTIVATED == memc_state->is_memc_activated) ||
	(MEMC_IS_ACTIVATED == memc_state->is_memc_activated)) {
		add_time_stamp(logging);
		log_this(logging,
		"memc was/is activated before; "
		"and you can not change configuration now.\n"
		"reboot the device in order to change\n\n");
		return -1;
	}

	return 0;
}

/* show method, region_start.  */
static ssize_t memc_region_start_show(struct kona_memc_regions
				*memc_region, char *buf)
{
	return  sprintf(buf, "0x%X\n", memc_region->start_address);
}

/* store method, region_start.  */
static ssize_t memc_region_start_store(struct kona_memc_regions
			*memc_region, const char *buf, size_t count)
{
	struct kona_memc_bitmaps *memc_state =
		&memc_region->memc_dev->memc_config_status;
	struct kona_memc_logging *logging =
		memc_region->memc_dev->logging;

	if (get_memc_activation_status(logging, memc_state))
		return -1;

	if (!write_memc_regions(memc_region, buf, START_ADDR))
		return count;
	else
		return -1;
}

/* show method, region_end.  */
static ssize_t memc_region_end_show(struct kona_memc_regions
				*memc_region, char *buf)
{
	return sprintf(buf, "0x%X\n", memc_region->end_address);
}

/* store method, region_end.  */
static ssize_t memc_region_end_store(struct kona_memc_regions
		*memc_region, const char *buf, size_t count)
{
	struct kona_memc_bitmaps *memc_state =
		&memc_region->memc_dev->memc_config_status;
	struct kona_memc_logging *logging =
		memc_region->memc_dev->logging;

	if (get_memc_activation_status(logging, memc_state))
		return -1;

	if (!write_memc_regions(memc_region, buf, END_ADDR))
		return count;
	else
		return -1;
}

/* show method, memc_type.  */
static ssize_t memc_region_mem_type_show(struct kona_memc_regions
				*memc_region, char *buf)
{
	return sprintf(buf, "0x%X\n", memc_region->mem_type);
}

/* store method, memc_type.  */
static ssize_t memc_region_mem_type_store(struct kona_memc_regions
		*memc_region, const char *buf, size_t count)
{
	struct kona_memc_bitmaps *memc_state =
		&memc_region->memc_dev->memc_config_status;
	struct kona_memc_logging *logging =
		memc_region->memc_dev->logging;

	if (get_memc_activation_status(logging, memc_state))
		return -1;

	if (!write_memc_regions(memc_region, buf, MEM_TYPE))
		return count;
	else
		return -1;
}

/* show method, group_mask.  */
static ssize_t memc_region_group_mask_show(struct kona_memc_regions
				*memc_region, char *buf)
{
	return sprintf(buf, "0x%X\n", memc_region->group_mask);
}

/* store method, group_mask.  */
static ssize_t memc_region_group_mask_store(struct kona_memc_regions
		*memc_region, const char *buf, size_t count)
{
	struct kona_memc_bitmaps *memc_state =
		&memc_region->memc_dev->memc_config_status;
	struct kona_memc_logging *logging =
		memc_region->memc_dev->logging;

	if (get_memc_activation_status(logging, memc_state))
		return -1;

	if (!write_memc_regions(memc_region, buf, GROUP_MASK))
		return count;
	else
		return -1;
}




/************************************************************************
*			memc port configuration				*
 *************************************************************************/


struct memc_port_attribute {
	struct attribute attr;
	ssize_t(*show)(struct kona_memc_port *, char *, u32);
	ssize_t	(*store)(struct kona_memc_port *, const char *, size_t, u32);
};


/* derive the group number.  */
int canonicalize_group_num(struct memc_port_attribute *memc_attr,
	char const *buf)
{

	if (!strcmp(memc_attr->attr.name, "group0"))
		return 0;
	else if (!strcmp(memc_attr->attr.name, "group1"))
		return 1;
	else if (!strcmp(memc_attr->attr.name, "group2"))
		return 2;
	else if (!strcmp(memc_attr->attr.name, "group3"))
		return 3;
	else if (!strcmp(memc_attr->attr.name, "group4"))
		return 4;
	else if (!strcmp(memc_attr->attr.name, "group5"))
		return 5;
	else if (!strcmp(memc_attr->attr.name, "group6"))
		return 6;
	else if (!strcmp(memc_attr->attr.name, "group7"))
		return 7;
	else
		return -1;
}


/* show method: memc port.  */
static ssize_t memc_port_attr_show(struct kobject *kobj,
					struct attribute *attr, char *buf)
{
	struct kona_memc_port *memc_port;
	struct memc_port_attribute *memc_attr;

	memc_port = container_of(kobj, struct kona_memc_port, kobj);
	memc_attr = container_of(attr, struct memc_port_attribute, attr);

	return memc_attr->show(memc_port, buf,
	canonicalize_group_num(memc_attr, buf));
}

/* store method: memc port.  */
static ssize_t memc_port_attr_store(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t size)
{
	struct kona_memc_port *memc_ports;
	struct memc_port_attribute *memc_attr;
	struct kona_memc_bitmaps *memc_state;
	struct kona_memc_logging *logging;

	memc_ports = container_of(kobj, struct kona_memc_port, kobj);
	memc_attr = container_of(attr, struct memc_port_attribute, attr);

	memc_state = &memc_ports->memc_dev->memc_config_status;
	logging = memc_ports->memc_dev->logging;

	if (get_memc_activation_status(logging, memc_state))
		return -1;

	return memc_attr->store(memc_ports, buf, size,
	canonicalize_group_num(memc_attr, buf));
}

#define MEMC_PORT_ATTR(_name, _mode, _show, _store) \
	struct memc_port_attribute memc_port_attr_##_name = \
	__ATTR(_name, _mode, _show, _store)

static MEMC_PORT_ATTR(group0, 0666, memc_port_group_show,
			memc_port_group_store);
static MEMC_PORT_ATTR(group1, 0666, memc_port_group_show,
			memc_port_group_store);
static MEMC_PORT_ATTR(group2, 0666, memc_port_group_show,
			memc_port_group_store);
static MEMC_PORT_ATTR(group3, 0666, memc_port_group_show,
			memc_port_group_store);
static MEMC_PORT_ATTR(group4, 0666, memc_port_group_show,
			memc_port_group_store);
static MEMC_PORT_ATTR(group5, 0666, memc_port_group_show,
			memc_port_group_store);
static MEMC_PORT_ATTR(group6, 0666, memc_port_group_show,
			memc_port_group_store);
static MEMC_PORT_ATTR(group7, 0666, memc_port_group_show,
			memc_port_group_store);

static struct attribute *memc_port_sysfs_attrs[] = {
	&memc_port_attr_group0.attr,
	&memc_port_attr_group1.attr,
	&memc_port_attr_group2.attr,
	&memc_port_attr_group3.attr,
	&memc_port_attr_group4.attr,
	&memc_port_attr_group5.attr,
	&memc_port_attr_group6.attr,
	&memc_port_attr_group7.attr,
	NULL
};

static const struct sysfs_ops memc_port_sysfs_ops = {
	.show = memc_port_attr_show,
	.store = memc_port_attr_store,
};


static struct kobj_type ktype_secure_memc_port = {
	.default_attrs	= memc_port_sysfs_attrs,
	.sysfs_ops = &memc_port_sysfs_ops,
};



/************************************************************************
*			memc region configuration				*
 *************************************************************************/

struct memc_region_attribute {
	struct attribute attr;
	ssize_t(*show)(struct kona_memc_regions *, char *);
	ssize_t	(*store)(struct kona_memc_regions *, const char *, size_t);
};

/* show method: memc regions.  */
static ssize_t memc_region_attr_show(struct kobject *kobj,
					struct attribute *attr, char *buf)
{
	struct kona_memc_regions *memc_regions;
	struct memc_region_attribute *memc_attr;

	memc_regions = container_of(kobj, struct kona_memc_regions, kobj);
	memc_attr = container_of(attr, struct memc_region_attribute, attr);

	return memc_attr->show(memc_regions, buf);
}


/* store method: memc regions.  */
static ssize_t memc_region_attr_store(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t size)
{
	struct kona_memc_regions *memc_regions;
	struct memc_region_attribute *memc_attr;

	memc_regions = container_of(kobj, struct kona_memc_regions, kobj);
	memc_attr = container_of(attr, struct memc_region_attribute, attr);

	return memc_attr->store(memc_regions, buf, size);
}


#define MEMC_REGION_ATTR(_name, _mode, _show, _store) \
	struct memc_region_attribute memc_region_attr_##_name = \
	__ATTR(_name, _mode, _show, _store)

static MEMC_REGION_ATTR(region_start, 0666,
		memc_region_start_show, memc_region_start_store);
static MEMC_REGION_ATTR(region_end, 0666,
		memc_region_end_show, memc_region_end_store);
static MEMC_REGION_ATTR(mem_type, 0666,
		memc_region_mem_type_show, memc_region_mem_type_store);
static MEMC_REGION_ATTR(group_mask, 0666,
		memc_region_group_mask_show, memc_region_group_mask_store);


static struct attribute *memc_region_sysfs_attrs[] = {
	&memc_region_attr_region_start.attr,
	&memc_region_attr_region_end.attr,
	&memc_region_attr_mem_type.attr,
	&memc_region_attr_group_mask.attr,
	NULL
};

static const struct sysfs_ops memc_region_sysfs_ops = {
	.show = memc_region_attr_show,
	.store = memc_region_attr_store,
};


static struct kobj_type ktype_secure_memc_region = {
	.default_attrs	= memc_region_sysfs_attrs,
	.sysfs_ops = &memc_region_sysfs_ops,
};



/* validate memc configuration state: compelte/incomplete.  */
static int validate_memc_config_state(struct kona_secure_memc *memc_dev)
{
	u32 *memc_state = &memc_dev->memc_state;
	struct kona_memc_logging *logging = memc_dev->logging;
	u32 state = *memc_state &
			(GROUP_CONFIGURED | COMPLETE_REGION_CONFIGURED);

	if (state == (GROUP_CONFIGURED | COMPLETE_REGION_CONFIGURED))
		return 0;
	else {
		if (!(*memc_state & GROUP_CONFIGURED)) {
			add_time_stamp(logging);
			log_this(logging, "group not configured\n");
		}
		if (!(*memc_state & REGION_START_CONFIGURED)) {
			add_time_stamp(logging);
			log_this(logging, "region_start not configured\n");
		}
		if (!(*memc_state & REGION_END_CONFIGURED)) {
			add_time_stamp(logging);
			log_this(logging, "region_end not configured\n");
		}
		if (!(*memc_state & REGION_MEM_TYPE_CONFIGURED)) {
			add_time_stamp(logging);
			log_this(logging, "mem_type not configured\n");
		}
		if (!(*memc_state & REGION_GROUP_MASK_CONFIGURED)) {
			add_time_stamp(logging);
			log_this(logging, "group_mask not configured\n");
		}
		return 1;
	}
}


/* get first empty entity.  */
static u32 get_first_empty(u32 temp_mask)
{
	u32 num = 0;

	while (temp_mask & 0x01) {
		num++;
		temp_mask = temp_mask >> 1;
	}
	return num;
}

/* get first allocated entity.  */
static u32 get_first_allocated(u32 temp_mask)
{
	u32 num = 0;

	while (!(temp_mask & 0x01)) {
		num++;
		temp_mask = temp_mask >> 1;
	}
	return num;
}

/* check if port has been configured by the user.  */
static u32 is_port_configured(struct kona_secure_memc *memc_dev, u32 port_num)
{
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;

	if (memc_bitmaps->port_bitmap & (0x01 << port_num))
		return 1;
	else
		return 0;
}

/* find out the group which needs to be invalidated.  */
static void invalidate_rest_of_the_groups(struct kona_secure_memc *memc_dev,
						u32 port_num)
{
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_port **port = memc_dev->port;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	u32 group_num;

	for (group_num = 0; group_num < pdata->num_of_groups; group_num++) {
		if (!(memc_bitmaps->port_map[port_num][group_num]))
			port[port_num]->write_portx_gidx_cmp(memc_dev,
			port_num, group_num, "invalid");
	}
}

/* plain bubble sort, for region start_addr's.  */
static void region_sort(u32 *addr)
{
	u32 i, pass, temp;

	for (pass = 0; pass < NUM_OF_REGIONS; pass++) {
		for (i = 0; i < NUM_OF_REGIONS-1; i++) {
			if (*(addr+i) > *(addr+i+1)) {
				temp = *(addr+i);
				*(addr+i) = *(addr+i+1);
				*(addr+i+1) = temp;
			}
		}
	}
}

/* validate region for overlaping and region_end config.  */
static int validate_n_align_regions(u32 *sorted_addr, u32 *start_addr,
					u32 *end_addr, u32 *aligned_end_addr)
{
	u32 region_num, i;

	for (region_num = 0; region_num < NUM_OF_REGIONS; region_num++) {
		if (*(sorted_addr+region_num)) {
			for (i = 0; i < NUM_OF_REGIONS; i++) {
				if (*(sorted_addr+region_num) ==
					*(start_addr+i)) {
					*(aligned_end_addr+region_num) =
					*(end_addr+i);
				}
			}
		}
	}

	for (region_num = 0; region_num < NUM_OF_REGIONS-1; region_num++) {
		if (*(sorted_addr+region_num)) {
			if (*(aligned_end_addr+region_num) >=
				*(sorted_addr+region_num+1))
				return -1;
		}
	}

	for (region_num = 0; region_num < NUM_OF_REGIONS-1; region_num++) {
			if (*(sorted_addr+region_num)) {
				if (*(aligned_end_addr+region_num) <=
					*(sorted_addr+region_num))
					return -2;
			}
		}
	return 0;
}


/* generate the groupmask, which should be given to rest of the groups which
user have not configured.  */
static u32 generate_groupmask(struct kona_secure_memc *memc_dev)
{
	u32 group_num, group_mask = 0;
	u32 temp_mask = memc_dev->memc_config_status.group_bitmap;
	u32 i;

	/* whichver ports ahve been configured by user, we do not touch it;
	    but rather we generate the group mask for the rest of there
	    groups for the rest fo the ports.  */
	for (i = 0; i < NUM_OF_GROUPS; i++) {
		if ((i < NUM_OF_MEMC_PORTS) && is_port_configured(memc_dev, i))
			continue;
		group_num = get_first_empty(temp_mask);
		temp_mask |= (0x01 << group_num);
		group_mask |= (0x01 << group_num);
	}
	return group_mask;
}

/* set the region attributes, which users have not configured. */
static void set_region_attributes(struct kona_secure_memc *memc_dev, u32 num,
	u32 start_address, u32 end_address, u32 mem_type, u32 group_mask)
{
	struct kona_memc_regions **region = memc_dev->region;

	region[num]->start_address = start_address;
	region[num]->end_address = end_address;
	region[num]->mem_type = mem_type;
	region[num]->group_mask = group_mask ;
}


/* derive valid region start, region_en, memory, group mask; which are
menadotary to be set to have alid active memc configuration.  */
static int config_rest_of_the_regions(struct kona_secure_memc *memc_dev,
					u32 *start_addr, u32 *end_addr)
{
	u32 i = 0, empty_region, group_mask;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	struct kona_memc_logging *logging = memc_dev->logging;
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;

	while (!*(start_addr+i))
		i++;

	group_mask = generate_groupmask(memc_dev);

	if (*(start_addr+i) !=  pdata->ddr_start) {
		/* derive the first region.  */
		empty_region = get_first_empty(memc_bitmaps->region_bitmap);
		if (empty_region > NUM_OF_REGIONS - 1)
			goto exceed_region_error;
		memc_bitmaps->region_bitmap |= (0x01 << empty_region);
		set_region_attributes(memc_dev, empty_region, pdata->ddr_start,
			*(start_addr + i) - 1,
			MEM_TYPE_USR, group_mask);
	}


	/* derive all possible mid regions.  */
	while (i < NUM_OF_REGIONS - 1) {
		empty_region = get_first_empty(memc_bitmaps->region_bitmap);
		if (empty_region > NUM_OF_REGIONS - 1)
			goto exceed_region_error;
		if ((*(start_addr + i + 1) - 1) <= (*(end_addr + i) + 1)) {
			i++;
			continue;
		}
		memc_bitmaps->region_bitmap |= (0x01 << empty_region);
		set_region_attributes(memc_dev, empty_region, *
					(end_addr + i) + 1,
			*(start_addr + i + 1) - 1, MEM_TYPE_USR, group_mask);
		i++;
	}

	if (*(end_addr+i) != pdata->ddr_end) {
		/* derive the last region.  */
		empty_region = get_first_empty(memc_bitmaps->region_bitmap);
		if (empty_region > NUM_OF_REGIONS - 1)
			goto exceed_region_error;
		memc_bitmaps->region_bitmap |= (0x01 << empty_region);
		set_region_attributes(memc_dev, empty_region, *(end_addr+i) + 1,
				pdata->ddr_end, MEM_TYPE_USR, group_mask);
	}

	return 0;

exceed_region_error:
		add_time_stamp(logging);
		log_this(logging, "holes left; all 8 regions are full\n");
	return -1;

}


/* enable region check for individual configured region.  */
static void enable_regions(struct kona_secure_memc *memc_dev)
{
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_regions **region = memc_dev->region;
	struct kona_memc_port **port = memc_dev->port;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	u32 port_num, group_num, region_num, configured_group_mask;

	/* at this stage we are very sure that the
	configuration is completely valid, and before
	we enable the region; set the region access for groups.
	so we will know while granting and revoking the
	group accesses. */

	/* find out all the masters which users have configured.  */
	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		for (group_num = 0; group_num < pdata->num_of_groups;
			group_num++) {
			if (memc_bitmaps->port_map[port_num][group_num]) {
				for (region_num = 0;
				region_num < pdata->num_of_regions;
				region_num++) {
					configured_group_mask =
					region[region_num]->group_mask;
					if (((0x1 << region_num) &
					memc_bitmaps->region_bitmap)
					&& ((0x1 << group_num) &
					configured_group_mask)) {
						port[port_num]->group[group_num]
						->access.region_access[port_num]
						|= (0x1 << region_num);
					}
				}
			}
		}
	}

	for (region_num = 0; region_num < pdata->num_of_regions; region_num++) {
		if ((0x1 << region_num) & memc_bitmaps->region_bitmap) {
			region[region_num]->write_regionx(memc_dev, region_num,
							REGION_EN, 0x1);
		}
	}
}

void write_user_config(struct kona_secure_memc *memc_dev)
{
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_regions **region = memc_dev->region;
	struct kona_memc_port **port = memc_dev->port;
	struct kona_memc_master *master;
	struct list_head *tmp, *head;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	struct kona_memc_logging *logging = memc_dev->logging;
	u32 temp_mask = memc_bitmaps->region_bitmap;
	int port_num, group_num, region_num;
	char log_buf[16];

	/* find out all the masters which users have configured and
	write them all to the device.  */
	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		for (group_num = 0; group_num < pdata->num_of_memc_ports;
		group_num++) {
			/* currently we allow only one master t the group.
			we could combine  more than one master,
			here we have already resolved AXI id clash anway
			so just go through the master's list and
			configure memc.  */
			if (memc_bitmaps->port_map[port_num][group_num]) {
				add_time_stamp(logging);
				log_this(logging,
				"<< CONFIGURING USER DEFINEED PORT:");
				sprintf(log_buf, "%d", port_num);
				log_this(logging, log_buf);
				log_this(logging, " >>\n");
				head =
				&port[port_num]->group[group_num]->master_list;
				list_for_each(tmp, head) {
					master =
					container_of(tmp,
					struct kona_memc_master,
					group_master_list);
					port[port_num]->write_portx_gidx_cmp(
					memc_dev, port[port_num]->port_id,
					group_num, master->name);
				}
			}
		}
	}

	/* find out all the regions, whihc use has configured.  */
	for (region_num = 0; region_num < pdata->num_of_regions; region_num++) {
		if (temp_mask & 0x1) {
			add_time_stamp(logging);
			log_this(logging,
			"<< CONFIGURING USER DEFINEED REGION:");
			sprintf(log_buf, "%d", region_num);
			log_this(logging, log_buf);
			log_this(logging, " >>\n");
			region[region_num]->write_regionx(memc_dev,
			region_num, START_ADDR,
			region[region_num]->start_address);
			region[region_num]->write_regionx(memc_dev,
			region_num, END_ADDR,
			region[region_num]->end_address);
			region[region_num]->write_regionx(memc_dev,
			region_num, MEM_TYPE,
			region[region_num]->mem_type);
			region[region_num]->write_regionx(memc_dev,
			region_num, GROUP_MASK,
			region[region_num]->group_mask);
		}
		temp_mask = temp_mask >> 0x1;
	}

}


/* activate memc, which involves
>	validate region number limits
>	address alignment and validation
>	do region configuration
>	write user define configuration
>	do group and master configuration
>	enable region check for individual regions
>	activate memc by writing to memc global control
*/

static int activate_memc(struct kona_secure_memc *memc_dev)
{
	struct kona_memc_port **port = memc_dev->port;
	struct kona_memc_regions **region = memc_dev->region;
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	struct kona_memc_logging *logging = memc_dev->logging;
	u32 temp_mask = memc_bitmaps->region_bitmap, count = 0;
	u32 start_addr[NUM_OF_REGIONS] = {0}, end_addr[NUM_OF_REGIONS] = {0};
	u32 sorted_addr[NUM_OF_REGIONS] = {0}, ret = 0;
	u32 aligned_end_addr[NUM_OF_REGIONS] = {0};
	u32 port_num, empty_group_num, region_num, occupied_region_num;
	char log_buf[16];

	/* validate if regions are excedding the limits.  */
	for (region_num = 0; region_num < pdata->num_of_regions; region_num++) {
		if (temp_mask & 0x1)
			count++;
		if (count > pdata->num_of_regions) {
			add_time_stamp(logging);
			log_this(logging, "excedding more than 8 regions\n");
			goto activate_memc_err;
		}
		temp_mask = temp_mask >> 0x1;
	}


	/* validate region addresses as neither regions can overlap nor have
	any faulty addresss.  */
	temp_mask = memc_bitmaps->region_bitmap;
	while (temp_mask) {
		occupied_region_num = get_first_allocated(temp_mask);
		temp_mask &= ~(0x1 << occupied_region_num);
		region_num = occupied_region_num;
		start_addr[region_num] = region[region_num]->start_address;
		sorted_addr[region_num] = region[region_num]->start_address;
		end_addr[region_num] = region[region_num]->end_address;
	}

	region_sort(&sorted_addr[0]);
	ret = validate_n_align_regions(&sorted_addr[0], &start_addr[0],
		&end_addr[0], &aligned_end_addr[0]);
	if (ret == -1) {
		add_time_stamp(logging);
		log_this(logging, "regions can not overlap\n");
		goto activate_memc_err;
	} else if (ret == -2) {
		add_time_stamp(logging);
		log_this(logging, "invalid region end address\n");
		goto activate_memc_err;
	}

	if (config_rest_of_the_regions(memc_dev, &sorted_addr[0],
					&aligned_end_addr[0]))
		return -1;

	add_time_stamp(logging);
	log_this(logging, "........writing and activating memc........\n");

	/* write the user configuration to the device.  */
	write_user_config(memc_dev);

	/* invalidate rest of the groups.  */
	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		if (is_port_configured(memc_dev, port_num)) {
			add_time_stamp(logging);
			log_this(logging, "<< CONFIGURING PORT:");
			sprintf(log_buf, "%d", port_num);
			log_this(logging, log_buf);
			log_this(logging, " >>\n");
			invalidate_rest_of_the_groups(memc_dev, port_num);
		} else {
			add_time_stamp(logging);
			log_this(logging, "<< CONFIGURING PORT:");
			sprintf(log_buf, "%d", port_num);
			log_this(logging, log_buf);
			log_this(logging, " >>\n");
			empty_group_num =
			get_first_empty(memc_bitmaps->group_bitmap);
			port[port_num]->write_portx_gidx_cmp(memc_dev, port_num,
			empty_group_num, port[port_num]->default_master);
			memc_bitmaps->port_map[port_num][empty_group_num] = 1;
			memc_bitmaps->group_bitmap |= (0x01 << empty_group_num);
			memc_bitmaps->port_bitmap |= (0x01 << port_num);
			invalidate_rest_of_the_groups(memc_dev, port_num);
		}
	}

	/* enable region check.  */
	add_time_stamp(logging);
	log_this(logging, "<< enabling check for individual regions >>\n");
	enable_regions(memc_dev);

	writel(0x10, pdata->kona_s_memc_base);
	memc_bitmaps->is_memc_activated = MEMC_IS_ACTIVATED;
	return 0;

activate_memc_err:
	return -1;
}


static void show_current_configuration(struct kona_secure_memc *memc_dev)
{
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_port **port = memc_dev->port;
	struct kona_memc_regions **region = memc_dev->region;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	struct kona_memc_logging *logging = memc_dev->logging;
	struct kona_memc_master *master;
	struct list_head *tmp;
	u32 temp_mask = memc_bitmaps->region_bitmap;
	int port_num, group_num, region_num;
	char log_buf[16];

	log_this(logging, "\n<<current memc configuration >>\n");
	log_this(logging, "-----------------------------------\n");

	/* find out all the masters which users have configured.  */
	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		for (group_num = 0; group_num < pdata->num_of_groups;
			group_num++) {
			if (memc_bitmaps->port_map[port_num][group_num]) {
				list_for_each(tmp,
					&port[port_num]->
					group[group_num]->master_list) {
					master = container_of(tmp,
					struct kona_memc_master,
					group_master_list);
					log_this(logging, master->name);
					log_this(logging, " to port");
					sprintf(log_buf, "%d", port_num);
					log_this(logging, log_buf);
					log_this(logging, "/group");
					sprintf(log_buf, "%d\n", group_num);
					log_this(logging, log_buf);
				}
			}
		}
	}

	/* find out all the regions, and set them to default values.  */
	for (region_num = 0; region_num < pdata->num_of_regions; region_num++) {
		if (temp_mask & 0x1) {
			log_this(logging, "region");
			sprintf(log_buf, "%d\n", region_num);
			log_this(logging, log_buf);
			log_this(logging, "start_adress:");
			sprintf(log_buf, "0x%X\n",
				region[region_num]->start_address);
			log_this(logging, log_buf);
			log_this(logging, "end_adress:");
			sprintf(log_buf, "0x%X\n",
				region[region_num]->end_address);
			log_this(logging, log_buf);
			log_this(logging, "memory type:");
			sprintf(log_buf, "0x%X\n",
				region[region_num]->mem_type);
			log_this(logging, log_buf);
			log_this(logging, "group_mask:");
			sprintf(log_buf, "0x%X\n",
				region[region_num]->group_mask);
			log_this(logging, log_buf);
		}
		temp_mask = temp_mask >> 0x1;
	}
}

static void erase_memc(struct kona_secure_memc *memc_dev)
{
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_port **port = memc_dev->port;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	struct kona_memc_logging *logging = memc_dev->logging;
	u32 temp_mask = memc_bitmaps->region_bitmap;
	int port_num, group_num, region_num;

	/* find out all the masters,
	which users have configured and erase them all.  */
	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		for (group_num = 0; group_num < pdata->num_of_memc_ports;
					group_num++) {
			if (memc_bitmaps->port_map[port_num][group_num])
				memc_remove_masters_from_group(port[port_num],
								group_num);
		}
	}

	/* find out all the regions, and set them to default values.  */
	for (region_num = 0; region_num < pdata->num_of_regions; region_num++) {
		if (temp_mask & 0x1) {
			set_region_attributes(memc_dev, region_num,
						0x0, 0xFFF, 0x0, 0xFF);
		}
		temp_mask = temp_mask >> 0x1;
	}

	/* reset the states.  */
	memset((void *) memc_bitmaps, 0x0, sizeof(struct kona_memc_bitmaps));
	memc_dev->memc_state = NOT_CONFIGURED;

	add_time_stamp(logging);
	log_this(logging,
	"<< whole memc has been set to default configuration now >>\n");

}


int get_default_possible_port(char *master)
{
	u32 index;
	for (index = 0; index < NUM_OF_ALLOWED_MASTERS; index++) {
		if (!strncmp(master, memc_map[index].master,
			strlen(memc_map[index].master)))
				return index;
	}
	return -1;
}

static int initialize_memc(struct kona_secure_memc *memc_dev)
{
	struct kona_memc_port **port = memc_dev->port;
	struct kona_memc_regions **region = memc_dev->region;
	struct kona_memc_logging *logging = memc_dev->logging;
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	u32 ms, port_num, group_num, region_num;
	u32 rgn, attr;
	char *invalid_master;

	for (ms = 0; ms < sizeof(pdata->static_memc_masters)/
		sizeof(*pdata->static_memc_masters); ms++) {
		if (match_the_existing_masters(memc_dev,
			pdata->static_memc_masters[ms])) {
			invalid_master = kzalloc
			(strlen(pdata->static_memc_masters[ms]), GFP_KERNEL);
			if (!invalid_master) {
				pr_err("could not allocate invalid_master\n");
				return -ENOMEM;
			}
			strncpy(invalid_master,
			pdata->static_memc_masters[ms],
			strlen(pdata->static_memc_masters[ms])-1);
			*(invalid_master + strlen(invalid_master)) = '\0';
			add_time_stamp(logging);
			log_this(logging, invalid_master);
			log_this(logging, " does not exist\n");
			kfree(invalid_master);
			log_this(logging,
			"\n<< failed to initialize memc>>\n");
			return -EINVAL;
		}
	}

	for (ms = 0; ms < sizeof(pdata->static_memc_masters)/
		sizeof(*pdata->static_memc_masters); ms++) {
		port_num =
		get_default_possible_port(pdata->static_memc_masters[ms]);
		if (port_num < 0)
			return -EINVAL;
		group_num =
		get_first_empty(memc_dev->memc_config_status.group_bitmap);
		align_memc_masters(port[port_num],
		pdata->static_memc_masters[ms],
		sizeof(pdata->static_memc_masters[ms]), group_num);
	}

	for (rgn = 0;
		rgn < sizeof(pdata->static_memc_config)/
		sizeof(*pdata->static_memc_config); rgn++) {
		region_num = get_first_empty
		(memc_dev->memc_config_status.region_bitmap);
		for (attr = 0; attr < sizeof(pdata->static_memc_config[0])/
			sizeof(*pdata->static_memc_config[0]); attr++) {
			write_memc_regions(region[region_num],
			pdata->static_memc_config[rgn][attr], attr);
		}
	}

	if (!activate_memc(memc_dev)) {
		add_time_stamp(logging);
		log_this(logging, "<< memc activated successfully >>\n");
	} else
		return -1;

	return 0;
}


/* show method, activate memc.  */
static ssize_t memc_activate_show(struct kona_memc_logging *memc_logging,
					char *buf)
{
	u32 count = 0;
	struct kona_memc_bitmaps *memc_bitmaps =
	&memc_logging->memc_dev->memc_config_status;

	if (MEMC_IS_ACTIVATED == memc_bitmaps->is_memc_activated)
		count = sprintf(buf+count, "<< memc is active >>\n");
	else if (MEMC_WAS_ACTIVATED == memc_bitmaps->is_memc_activated)
		count = sprintf(buf+count,
		"<< memc was activated at least once in the past, "
		"but, now it is inactive >>\n");
	else if (MEMC_NOT_ACTIVATED == memc_bitmaps->is_memc_activated)
		count = sprintf(buf+count, "<< memc is inactive >>\n");

	return count;
}

/* store method, activate memc.  */
static ssize_t memc_activate_store(struct kona_memc_logging *memc_logging,
					const char *buf, size_t size)
{
	int val, ret = -1;
	struct kona_secure_memc *memc_dev = memc_logging->memc_dev;
	struct kona_secure_memc_pdata *pdata;
	struct kona_memc_bitmaps *memc_bitmaps;

	pdata = memc_dev->pdata;
	memc_bitmaps = &memc_dev->memc_config_status;

	sscanf(buf, "%d", &val);

	if (0 == val) {
		if (MEMC_IS_ACTIVATED == memc_bitmaps->is_memc_activated) {
			writel(0x00, pdata->kona_s_memc_base);
			memc_bitmaps->is_memc_activated = MEMC_WAS_ACTIVATED;
			add_time_stamp(memc_logging);
			log_this(memc_logging,
			"<< memc deactivated successfully >>\n");
		} else {
			add_time_stamp(memc_logging);
			log_this(memc_logging,
			"<< memc is already inactive >>\n");
			goto invalid_config;
		}
	} else if (1 == val) {
		/* this is just on/off switch */
		if (MEMC_WAS_ACTIVATED == memc_bitmaps->is_memc_activated) {
			writel(0x10, pdata->kona_s_memc_base);
			memc_bitmaps->is_memc_activated = MEMC_IS_ACTIVATED;
			add_time_stamp(memc_logging);
			log_this(memc_logging,
			"<< memc activated successfully >>\n");
		} else if (MEMC_IS_ACTIVATED ==
					memc_bitmaps->is_memc_activated) {
			add_time_stamp(memc_logging);
			log_this(memc_logging,
			"<< memc is already active >>\n");
			goto invalid_config;
		} else if (!memc_bitmaps->is_memc_activated) {
			if (validate_memc_config_state(memc_dev)) {
				add_time_stamp(memc_logging);
				log_this(memc_logging,
				"could not activate memc, please check "
				"the above errors;\n");
				goto invalid_config;
			}
			ret = activate_memc(memc_dev);
			if (!ret) {
				add_time_stamp(memc_logging);
				log_this(memc_logging,
				"<< memc activated successfully >>\n");
			} else {
				add_time_stamp(memc_logging);
				log_this(memc_logging,
				"could not activate memc, please check the "
				"above errors;\n");
				goto invalid_config;
			}
		}
	} else if (2 == val) {
		if (MEMC_WAS_ACTIVATED == memc_bitmaps->is_memc_activated) {
			add_time_stamp(memc_logging);
			log_this(memc_logging,
			"once ever memc was activated, "
			"configuration can not be erased\n");
			goto invalid_config;
		} else if (MEMC_IS_ACTIVATED ==
					memc_bitmaps->is_memc_activated) {
			add_time_stamp(memc_logging);
			log_this(memc_logging,
			"memc is active, "
			"configuration can not be erased\n");
			goto invalid_config;
		} else {
			add_time_stamp(memc_logging);
			log_this(memc_logging,
			"going to erase your configuration;\n");
			memc_bitmaps->is_memc_activated = MEMC_NOT_ACTIVATED;
			erase_memc(memc_dev);
		}
	} else if (3 == val) {
		show_current_configuration(memc_dev);
	} else {
		goto invalid_config;
	}

	return size;

invalid_config:
	return -1;
}


static ssize_t memc_log_show(struct kona_memc_logging *memc_logging,
				char *buf)
{
	snprintf(buf, PAGE_SIZE, "%s\n", memc_logging->memc_log);
	return PAGE_SIZE;
}

/* show method: memc help.  */
static ssize_t memc_help_show(struct kona_memc_logging *memc_logging,
				char *buf)

{
	u32 index, count = 0;
	struct kona_secure_memc *memc_dev = memc_logging->memc_dev;

	count += sprintf(buf+count, "----------------------------\n");
	count += sprintf(buf+count, "<< AVAILABLE MASTERS >>\n");
	for (index = 0; index < NUM_OF_ALLOWED_MASTERS; index++) {
		count += sprintf(buf+count, "%s\t\t  >>>>   PORT[%d]\n",
		memc_dev->masters[index], index);
	}
	count += sprintf(buf+count, "----------------------------\n");
	count += sprintf(buf+count, "<< GROUPS >>\n");
	count += sprintf(buf+count,
	"echo mstr_name > group<n> [add masters to group]\n");
	count += sprintf(buf+count, "echo del > group<n> [to empty group]\n");

	count += sprintf(buf+count, "----------------------------\n");
	count += sprintf(buf+count, "<< REGIONS >>\n");
	count += sprintf(buf+count, "region_start > [ddr start address]\n");
	count += sprintf(buf+count, "region_end > [ddr end address]\n");
	count += sprintf(buf+count, "mem_type >\n");
	count += sprintf(buf+count, "\t\t0x3: User\n");
	count += sprintf(buf+count, "\t\t0x2: VC Supervisor\n");
	count += sprintf(buf+count, "\t\t0x1: VC Secure\n");
	count += sprintf(buf+count,
	"\t\t0x0: TZ trusted for ARM sub-system,"
	"Invalid for VC3 sub-system\n");
	count += sprintf(buf+count,
	"region_group_mask > [maps group<n> to region<n>\n");
	count += sprintf(buf+count, "echo mask > group_mask\n");

	count += sprintf(buf+count, "----------------------------\n");
	count += sprintf(buf+count, "<<ACTIVATE MEMC>>\n");
	count += sprintf(buf+count, "echo 0 > activate_memc [deactivation]\n");
	count += sprintf(buf+count,
	"echo 1 > activate_memc [activate memc"
	" with current cuser configuration]\n");
	count += sprintf(buf+count, "echo 2 > erase configuration\n");
	count += sprintf(buf+count, "echo 3 > show current configuration\n");

	count += sprintf(buf+count, "----------------------------\n");
	count += sprintf(buf+count, "<<MEMC EVENT LOG>>\n");
	count += sprintf(buf+count, "cat memc_log\n");

	count += sprintf(buf+count, "----------------------------\n");
	count += sprintf(buf+count, "<<VIOLATION LOG>>\n");
	count += sprintf(buf+count, "cat violation_log\n");

	count += sprintf(buf+count, "----------------------------\n");
	count += sprintf(buf+count, "<<USER NOTES/PROTOCOL>>\n");
	count += sprintf(buf+count,
	"1) confiugre the groups the way you want\n");
	count += sprintf(buf+count,
	"2) confiugre the regions the way you want\n");
	count += sprintf(buf+count,
	"3) make sure that regions do not overlap\n");
	count += sprintf(buf+count,
	"4) make sure that end address' last 3 nibbles are FFF\n");
	count += sprintf(buf+count,
	"5) once memc is activated, never attempt to change configuration;\n"
	"   any time before activation, you can discard whole configuration,\n"
	"   by erasing memc\n");
	count += sprintf(buf+count, "6) be careful while you set groupmask\n");
	count += sprintf(buf+count,
	"7) if user has set groupmask for a"
	"   particular region for a master,\n");
	count += sprintf(buf+count,
	"   User must configure all the valid"
	" region for the same master\n");
	count += sprintf(buf+count, "   in other words\n");
	count += sprintf(buf+count,
	"   user can not configure half valid region for any master\n");
	count += sprintf(buf+count, "--------------------------------------\n");
	return count;
}

/* show method: memc violation info.  */
static ssize_t memc_violation_log_show(struct kona_memc_logging
						*memc_logging, char *buf)
{
	u32 i, count = 0;
	u32 type, axi_id, reason, port_id;

	for (i = 0; i < VIOLATION_INFO_LEN; i++) {
		if (memc_logging->violation_info[i + 1]) {
			count = count + sprintf(buf+count,
			"\nVIOLATION%d\n", i+1);
			count = count + sprintf(buf+count,
			"-------------------------------------------------\n");
			count = count + sprintf(buf+count,
				"violation addr: 0x%X\n",
				memc_logging->violation_info[i]);
			count = count + sprintf(buf+count,
				"violation info: 0x%X\n",
				memc_logging->violation_info[++i]);
			type = (memc_logging->violation_info[i] & 0x1);
			axi_id = (memc_logging->violation_info[i] & 0xFFFE)
				>> 1;
			reason = (memc_logging->violation_info[i] & 0x38000000)
				>> 27;
			port_id = (memc_logging->violation_info[i] &
							0x06000000) >> 25;
			if (type)
				count = count + sprintf(buf+count,
					"write access violation @ port%u\n",
					port_id);
			else
				count = count + sprintf(buf+count,
				"read access violation @ port%u\n", port_id);
			count = count + sprintf(buf+count,
				"by the master with axi id:0x%X\n", axi_id);
			count = count + sprintf(buf+count,
			"-------------------------------------------------\n");
		} else {
			count = count + sprintf(buf+count,
				"<< no further violations >>\n");
			i++;
		}
	}
	return count;
}


struct memc_logging_attribute {
	struct attribute attr;
	ssize_t(*show)(struct kona_memc_logging *, char *);
	ssize_t(*store)(struct kona_memc_logging *, const char *, size_t);
};


/* show method: memc logging.  */
static ssize_t memc_logging_attr_show(struct kobject *kobj,
					struct attribute *attr, char *buf)
{
	struct kona_memc_logging *memc_logging;
	struct memc_logging_attribute *memc_attr;

	memc_logging = container_of(kobj, struct kona_memc_logging, kobj);
	memc_attr = container_of(attr, struct memc_logging_attribute, attr);

	if (memc_attr->show)
		return memc_attr->show(memc_logging, buf);
	else
		return -1;
}

/* store method: memc logging.  */
static ssize_t memc_logging_attr_store(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t size)
{
	struct kona_memc_logging *memc_logging;
	struct memc_logging_attribute *memc_attr;

	memc_logging = container_of(kobj, struct kona_memc_logging, kobj);
	memc_attr = container_of(attr, struct memc_logging_attribute, attr);

	if (memc_attr->store)
		return memc_attr->store(memc_logging, buf, size);
	else
		return -1;
}

#define MEMC_LOGGING_ATTR(_name, _mode, _show, _store) \
	struct memc_logging_attribute memc_logging_attr_##_name = \
	__ATTR(_name, _mode, _show, _store)

static MEMC_LOGGING_ATTR(memc_log, 0666, memc_log_show, NULL);
static MEMC_LOGGING_ATTR(memc_violation_log, 0666,
			memc_violation_log_show, NULL);
static MEMC_LOGGING_ATTR(memc_activate, 0666, memc_activate_show,
			memc_activate_store);
static MEMC_LOGGING_ATTR(memc_help, 0666, memc_help_show, NULL);

static struct attribute *memc_logging_sysfs_attrs[] = {
	&memc_logging_attr_memc_log.attr,
	&memc_logging_attr_memc_violation_log.attr,
	&memc_logging_attr_memc_activate.attr,
	&memc_logging_attr_memc_help.attr,
	NULL
};

static const struct sysfs_ops memc_logging_sysfs_ops = {
	.show = memc_logging_attr_show,
	.store = memc_logging_attr_store,
};


static struct kobj_type ktype_secure_memc_logging = {
	.default_attrs	= memc_logging_sysfs_attrs,
	.sysfs_ops = &memc_logging_sysfs_ops,
};


/* create sysfs entries.  */
static int secure_memc_sysfs_init(struct kona_secure_memc *memc)
{
	u32 port_num, group_num, region_num;
	int ret = 0, p, g;

	memc->kobj = kobject_create_and_add("memc_config", &memc->dev->kobj);
	if (!memc->kobj) {
		dev_dbg((struct device const *)&memc->dev,
		"MEMC_CONFIG allocation failed");
		goto MEMC_CONFIG_ALLOC_FAILED;
	}


	for (port_num = 0; port_num < memc->pdata->num_of_memc_ports;
								port_num++) {
		ret = kobject_init_and_add(&memc->port[port_num]->kobj,
		&ktype_secure_memc_port, memc->kobj, "%s%d", "port", port_num);

		if (unlikely(ret != 0))
			goto PORT_KOBJ_ALLOC_FAILED;
	}

	memc->region_kobj = kobject_create_and_add("regions", memc->kobj);
	if (!memc->region_kobj) {
			dev_dbg((struct device const *)&memc->dev,
			"REGIONS allocation failed");
			goto REGION_KOBJ_ALLOC_FAILED;
	}

	for (region_num = 0; region_num < memc->pdata->num_of_regions;
								region_num++) {
		ret = kobject_init_and_add(&memc->region[region_num]->kobj,
		&ktype_secure_memc_region , memc->region_kobj,
		"%s%d", "region", region_num);
		if (unlikely(ret != 0))
			goto REGIONX_KOBJ_ALLOC_FAILED;
	}

	ret = kobject_init_and_add(&memc->logging->kobj,
		&ktype_secure_memc_logging, memc->kobj, "%s", "activate_&_log");
	if (unlikely(ret != 0))
		goto SYSFS_LOGGING_FAILED;

	return ret;

SYSFS_LOGGING_FAILED:
	kobject_put(&memc->logging->kobj);

REGIONX_KOBJ_ALLOC_FAILED:
	for (p = port_num-1; p >= 0; p--)
		kobject_put(&memc->port[p]->kobj);
	for (p = region_num; p >= 0; p--)
		kobject_put(&memc->region[region_num]->kobj);
	for (p = port_num-1; p >= 0; p--)
		kobject_put(&memc->port[p]->kobj);
	region_num = -1;
	port_num = -1;

REGION_KOBJ_ALLOC_FAILED:
	for (p = port_num-1; p >= 0; p--)
		kobject_put(&memc->port[p]->kobj);
	kobject_put(memc->region_kobj);
	port_num = -1;

PORT_KOBJ_ALLOC_FAILED:
	for (p = port_num; p >= 0; p--)
		kobject_put(&memc->port[port_num]->kobj);

MEMC_CONFIG_ALLOC_FAILED:
	kobject_put(memc->kobj);

	port_num = memc->pdata->num_of_memc_ports;
	group_num = memc->pdata->num_of_groups;
	region_num = memc->pdata->num_of_regions;

	for (p = port_num - 1; p >= 0; p--)
		for (g = group_num - 1; g >= 0; g--)
			kfree(memc->port[p]->group[g]);
	for (p = port_num - 1; p >= 0; p--)
		kfree(memc->port[p]->group);
	for (p = port_num - 1; p >= 0; p--)
		kfree(memc->port[p]);
	for (g = region_num - 1; g >= 0; g--)
		kfree(memc->region[g]);
	kfree(memc->port);
	kfree(memc->region);

return -ENOMEM;
}


/* port, groups, region allocation.  */
static int secure_memc_alloc_ports(struct kona_secure_memc *memc)
{
	struct kona_memc_port **port;
	struct kona_memc_group **group;
	struct kona_memc_regions **region;
	struct kona_memc_logging *logging;
	char **masters;
	struct kona_secure_memc_pdata *pdata = memc->pdata;

	unsigned long page;

	u32 port_num, group_num, region_num, master_num;
	u32 p, g;

	(memc->port) = kzalloc(sizeof(struct kona_memc_port *)
			* (pdata->num_of_memc_ports), GFP_KERNEL);
	if (!(memc->port)) {
		dev_dbg((struct device const *)&memc->dev,
		"could not allocate memory for memc_port\n");
		goto memc_port_alloc_failed;
	}

	port = memc->port;
	(memc->region) = kzalloc(sizeof(struct kona_memc_regions *) *
					(pdata->num_of_regions), GFP_KERNEL);
	if (!(memc->region)) {
		dev_dbg((struct device const *)&memc->dev,
		"could not allocate memory for memc_region\n");
		goto memc_region_alloc_failed;
	}

	region = memc->region;
	for (port_num = 0; port_num < pdata->num_of_memc_ports; port_num++) {
		port[port_num] = kzalloc(sizeof(struct kona_memc_port),
			GFP_KERNEL);
		if (!(port[port_num])) {
			dev_dbg((struct device const *)&memc->dev,
			"%s[%d]\n", "could not allocate memory for "
			"memc_port", port_num);
			goto memc_portx_alloc_failed;
		}
		port[port_num]->port_id = port_num;
		port[port_num]->default_master =
		pdata->default_master_map[port_num];
		port[port_num]->write_portx_gidx_cmp = write_portx_gidx_cmp;
		port[port_num]->read_portx_gidx_cmp = read_portx_gidx_cmp;
		port[port_num]->memc_dev = memc;
		port[port_num]->group = kzalloc(sizeof(struct kona_memc_group *)
				* (pdata->num_of_groups), GFP_KERNEL);

		if (!port[port_num]->group) {
			dev_dbg((struct device const *)&memc->dev,
			"%s[%d]%s\n", "could not allocate memory for "
			"memc_port", port_num, "group");
			goto memc_group_alloc_failed;
		}

		group = port[port_num]->group;
		for (group_num = 0; group_num < pdata->num_of_groups;
								group_num++) {
			group[group_num] =
			kzalloc(sizeof(struct kona_memc_group), GFP_KERNEL);
			if (!group[group_num]) {
				dev_dbg((struct device const *)&memc->dev,
				"%s[%d]%s[%d]\n", "could not allocate "
				"memory for memc_port",
				port_num, "group", group_num);
				goto memc_groupx_alloc_failed;
			}
			group[group_num]->group_id = group_num;
			memset((void *) &group[group_num]->access,
				0x0, sizeof(struct access_control));
		}

	}

	for (region_num = 0; region_num < pdata->num_of_regions; region_num++) {
		region[region_num] = kzalloc(sizeof(struct kona_memc_regions),
						GFP_KERNEL);
		if (!(region[region_num])) {
			dev_dbg((struct device const *)&memc->dev,
			"%s[%d]\n", "could not allocate memory for "
			"memc_region", region_num);
			goto memc_regionx_alloc_failed;
		}
		region[region_num]->write_regionx = write_region;
		region[region_num]->read_regionx = read_regionx;
		region[region_num]->region_id = region_num;
		region[region_num]->memc_dev = memc;
	}

	for (port_num = 0; port_num < pdata->num_of_memc_ports;
				port_num++) {
		for (group_num = 0; group_num < pdata->num_of_groups;
				group_num++) {
			INIT_LIST_HEAD
			(&port[port_num]->group[group_num]->master_list);
			INIT_LIST_HEAD
			(&port[port_num]->group[group_num]->region_list);
		}
	}

	masters = kzalloc(sizeof(pdata->masters)/
			sizeof(*pdata->masters), GFP_KERNEL);
	if (!masters) {
		dev_dbg((struct device const *)&memc->dev,
		"%s\n", "could not allocate memory for masters\n");
		goto memc_master_alloc_failed;
	}

	for (master_num = 0;
	master_num < sizeof(pdata->masters)/sizeof(*pdata->masters);
					master_num++) {
		masters[master_num] =
		kzalloc(strlen(pdata->masters[master_num]+1), GFP_KERNEL);
		if (!masters[master_num]) {
			dev_dbg((struct device const *)&memc->dev,
			"%s\n", "could not allocate memory for logging\n");
			goto memc_masterdata_alloc_failed;
		}
		strcpy(masters[master_num], pdata->masters[master_num]);
	}

	memc->masters = masters;

	memset((void *) &memc->memc_config_status, 0x0,
		sizeof(struct kona_memc_bitmaps));

	logging = kzalloc(sizeof(struct kona_memc_logging), GFP_KERNEL);
	if (!logging) {
		dev_dbg((struct device const *)&memc->dev,
		"%s\n", "could not allocate memory for logging\n");
		goto memc_logging_alloc_failed;
	}

	page = get_zeroed_page(GFP_KERNEL);
	logging->memc_log = (unsigned char *) page;
	logging->memc_log_index = 0;
	memset((void *) logging->violation_info, 0x0,
		sizeof(u32) * VIOLATION_INFO_LEN);
	logging->violation_index = 0;
	logging->memc_dev = memc;
	memc->logging = logging;

	return 0;

memc_logging_alloc_failed:
	kfree(logging);
memc_masterdata_alloc_failed:
	for (p = master_num-1; p >= 0; p++)
		kfree(masters[p]);
memc_master_alloc_failed:
	kfree(masters);

memc_regionx_alloc_failed:
	for (p = port_num - 1; p >= 0; p--)
		for (g = group_num - 1; g >= 0; g--)
			kfree(memc->port[p]->group[g]);
	for (p = port_num - 1; p >= 0; p--)
		kfree(memc->port[p]->group);
	for (p = port_num - 1; p >= 0; p--)
		kfree(memc->port[p]);
	for (g = region_num - 1; g >= 0; g--)
		kfree(memc->region[g]);
	port_num = -1;
	group_num = -1;

memc_groupx_alloc_failed:
	for (p = port_num; p >= 0; p--)
		for (g = group_num - 1; g >= 0; g--)
			kfree(memc->port[p]->group[g]);
	for (p = port_num; p >= 0; p--)
		kfree(memc->port[p]->group);
	for (p = port_num; p >= 0; p--)
		kfree(memc->port[p]);
	port_num = -1;
	group_num = -1;

memc_group_alloc_failed:
	for (p = port_num - 1; p >= 0; p--)
		kfree(memc->port[p]->group);
	for (p = port_num; p >= 0; p--)
		kfree(memc->port[p]);
	port_num = -1;
	group_num = -1;

memc_portx_alloc_failed:
	for (p = port_num - 1; p >= 0; p--)
		kfree(memc->port[p]);

memc_region_alloc_failed:
	kfree(memc->port);
	kfree(memc->region);

memc_port_alloc_failed:
	return -ENOMEM;
}



static void throw_violation(struct kona_secure_memc *memc_dev)
{
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_logging *logging = memc_dev->logging;

	u32 addr, info;

	addr = readl(pdata->kona_s_memc_base + 0x90);
	info = readl(pdata->kona_s_memc_base + 0x94);

	if (addr || info) {
		pr_warning(
		"\n************************************************\n");
		pr_warning(
		"violation @ addr:0x%X, info:0x%X\n", addr, info);
		pr_warning(
		"\n************************************************\n");
		logging->violation_info[logging->violation_index++] = addr;
		logging->violation_info[logging->violation_index++] = info;
		logging->violation_index &= (VIOLATION_INFO_LEN - 1);
	}
}

static irqreturn_t secure_sys_emi_handler(int irq, void *data)
{
	struct kona_secure_memc_pdata *pdata =
		(struct kona_secure_memc_pdata *) data;
	u32 addr, info;

	addr = readl(pdata->kona_s_memc_base + 0x90);
	info = readl(pdata->kona_s_memc_base + 0x94);

	pr_warning("\n************************************************\n");
	pr_warning("violation @ addr:0x%X, info:0x%X\n", addr, info);
	pr_warning("\n************************************************\n");

	return IRQ_HANDLED;
}

static void memc_violation_info_work(struct work_struct *work)
{
	struct kona_secure_memc *memc_dev =
		container_of(work, struct kona_secure_memc,
				memc_work.work);

	throw_violation(memc_dev);

	schedule_delayed_work(&memc_dev->memc_work,
				msecs_to_jiffies(VIOLATION_POLL_DELAY));
}

u32 *get_secure_memc_handle()
{
	return memc_handle;
}
EXPORT_SYMBOL(get_secure_memc_handle);


int do_grant_region_access(u32 *memc_handle, enum memc_masters to_who)
{
#define ALLOW_ALL_GID_MASK 0x00FF0000
#define REGION_CTRL_OFFSET 0x10

	struct kona_secure_memc *memc_dev =
		container_of(memc_handle, struct kona_secure_memc,
				handle);
	unsigned int i, val, port_num, group_num;
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	unsigned int group_mask = 0;
	unsigned int addr = pdata->kona_s_memc_base;
	unsigned int offset  = REGION_CTRL_OFFSET;

	if (ALL == to_who) {
		group_mask = ALLOW_ALL_GID_MASK;
		goto write_memc_regions;
	} else if (FABRIC == to_who)
		port_num = get_default_possible_port(MASTER_FABRIC);
	else if (AP == to_who)
		port_num = get_default_possible_port(MASTER_A7);
	else if (MM == to_who)
		port_num = get_default_possible_port(MASTER_MM);
	else if (CP == to_who)
		port_num = get_default_possible_port(MASTER_COMMS);
	else
		goto wrong_parameter;

	if (port_num < 0)
		goto wrong_parameter;

	for (group_num = 0; group_num < pdata->num_of_groups;
		group_num++) {
		if (memc_bitmaps->port_map[port_num][group_num]) {
			group_mask |= (u32) ((0x1 << group_num) <<
				CONFIGURED_REGION_NUM_SHIFT);
		}
	}

write_memc_regions:

	/* Disable gid checking */
	writel(0x00, pdata->kona_s_memc_base);

	/* grant access to all the masters. */
	for (i = 0; i < pdata->num_of_regions; i++) {
		addr += offset;
		val = readl(addr);
		if (val & 0x1) {
			val |= group_mask;
			writel(val, addr);
		}
	}
	return 0;

wrong_parameter:
	return -1;

}
EXPORT_SYMBOL(do_grant_region_access);


int do_revoke_region_access(u32 *memc_handle, enum memc_masters to_who)
{
#define ALLOW_ALL_GID_MASK 0x00FF0000
#define REGION_CTRL_OFFSET 0x10

	struct kona_secure_memc *memc_dev =
		container_of(memc_handle, struct kona_secure_memc,
				handle);
	unsigned int val, port_num, group_num, region_num, temp;
	struct kona_secure_memc_pdata *pdata = memc_dev->pdata;
	struct kona_memc_bitmaps *memc_bitmaps = &memc_dev->memc_config_status;
	struct kona_memc_port **port = memc_dev->port;
	unsigned int addr = pdata->kona_s_memc_base;
	unsigned int offset  = REGION_CTRL_OFFSET;

	if (FABRIC == to_who)
		port_num = get_default_possible_port(MASTER_FABRIC);
	else if (AP == to_who)
		port_num = get_default_possible_port(MASTER_A7);
	else if (MM == to_who)
		port_num = get_default_possible_port(MASTER_MM);
	else if (CP == to_who)
		port_num = get_default_possible_port(MASTER_COMMS);
	else
		goto wrong_parameter;

	if (port_num < 0)
		goto wrong_parameter;

	for (group_num = 0; group_num < pdata->num_of_groups;
		group_num++) {
		if (memc_bitmaps->port_map[port_num][group_num]) {
			temp = port[port_num]->group[group_num]
				->access.region_access[port_num];
			addr = pdata->kona_s_memc_base;
			for (region_num = 0; region_num < pdata->num_of_regions;
				region_num++) {
				addr += offset;
				if (!(temp & 0x01)) {
					val = readl(addr);
					if (val & 0x1) {
						val &= ~((0x1 << group_num) <<
						CONFIGURED_REGION_NUM_SHIFT);
						writel(val, addr);
					}
				}
				temp = temp >> 1;
			}
		}
	}

	/* enable gid checking. */
	writel(0x10, pdata->kona_s_memc_base);
	return 0;

wrong_parameter:
	return -1;

}
EXPORT_SYMBOL(do_revoke_region_access);




/* sec_memc_panic_event() is called by the panic handler.
 * As soon as a panic occurs, and if there is one more violation
 * within the time period of VIOLATION_POLL_DELAY,
 * This function then collects violation info.
 */
static int sec_memc_panic_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	struct kona_secure_memc *memc_dev =
			container_of(this, struct kona_secure_memc,
					sec_memc_panic_block);

	throw_violation(memc_dev);

	/* ABI loads the loader at 0x80080000,
	and it come through fabric, as it is ligitimate access,
	we shall grant it here. */
	do_grant_region_access(memc_handle, FABRIC);

	return NOTIFY_DONE;
}


static int sec_memc_reboot_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	struct kona_secure_memc *memc_dev =
			container_of(this, struct kona_secure_memc,
					sec_memc_reboot_block);

	throw_violation(memc_dev);

	/* ABI loads the loader at 0x80080000,
	and it come through fabric, as it is ligitimate access,
	we shall grant it here. */
	do_grant_region_access(memc_handle, FABRIC);

	return NOTIFY_DONE;
}

static int kona_secure_memc_probe(struct platform_device *pdev)
{
	u32 ret;
	struct kona_secure_memc_pdata *pdata =
		(struct kona_secure_memc_pdata *)pdev->dev.platform_data;
	struct kona_secure_memc *memc_dev =
		kzalloc(sizeof(struct kona_secure_memc), GFP_KERNEL);

	if (!pdata || !memc_dev) {
		kfree(memc_dev);
		return -EINVAL;
	}

	memc_dev->pdata = pdata;
	memc_dev->dev = &pdev->dev;
	memc_dev->memc_state = NOT_CONFIGURED;
	memc_dev->init_static_config_memc = initialize_memc;

	memc_handle = &memc_dev->handle;
	/* some magic number. */
	*memc_handle = 0x5A5A5A5A;

	if (secure_memc_alloc_ports(memc_dev)) {
		printk(KERN_INFO "kona_secure_memc_probe failed\n");
		return -ENOMEM;
	}

	ret = secure_memc_sysfs_init(memc_dev);
	if (ret)
		goto out;

	ret = request_irq(KONA_SECURE_MEMC_IRQ, secure_sys_emi_handler,
						0, NULL, memc_dev->pdata);
	if (ret) {
		printk(KERN_ALERT "could not register sys emi irq\n");
		goto out;
	}

	enable_irq(KONA_SECURE_MEMC_IRQ);

	INIT_DELAYED_WORK(&memc_dev->memc_work, memc_violation_info_work);
	schedule_delayed_work(&memc_dev->memc_work,
				msecs_to_jiffies(VIOLATION_POLL_DELAY));


	memc_dev->sec_memc_panic_block.notifier_call = sec_memc_panic_event;
	memc_dev->sec_memc_panic_block.priority	= INT_MIN,

	memc_dev->sec_memc_reboot_block.notifier_call = sec_memc_reboot_event;
	memc_dev->sec_memc_reboot_block.priority = INT_MIN,


	atomic_notifier_chain_register(&panic_notifier_list,
				&memc_dev->sec_memc_panic_block);
	blocking_notifier_chain_register(&reboot_notifier_list,
				&memc_dev->sec_memc_reboot_block);

	if (pdata->static_config)
		ret = memc_dev->init_static_config_memc(memc_dev);

out:
	return ret;
}

static int kona_secure_memc_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver kona_secure_memc_driver = {
	.probe = kona_secure_memc_probe,
	.remove = kona_secure_memc_remove,
	.driver = {
	.name = "kona_secure_memc",
	},
};
static int __init kona_secure_memc_init(void)
{
	int ret;
	pr_info("kona_secure_memc : inside %s\n", __func__);

	ret = platform_driver_register(&kona_secure_memc_driver);

	return ret;
}

static void __exit kona_secure_memc_exit(void)
{
	platform_driver_unregister(&kona_secure_memc_driver);
}

module_init(kona_secure_memc_init);
module_exit(kona_secure_memc_exit);

MODULE_DESCRIPTION("KONA SECURE MEMC DRIVER");
MODULE_AUTHOR("Oza");

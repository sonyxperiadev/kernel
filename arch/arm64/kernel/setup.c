/*
 * Based on arch/arm/kernel/setup.c
 *
 * Copyright (C) 1995-2001 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/utsname.h>
#include <linux/initrd.h>
#include <linux/console.h>
#include <linux/cache.h>
#include <linux/bootmem.h>
#include <linux/seq_file.h>
#include <linux/screen_info.h>
#include <linux/init.h>
#include <linux/kexec.h>
#include <linux/crash_dump.h>
#include <linux/root_dev.h>
#include <linux/clk-provider.h>
#include <linux/cpu.h>
#include <linux/interrupt.h>
#include <linux/smp.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/memblock.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/dma-mapping.h>
#include <linux/efi.h>

#include <asm/fixmap.h>
#include <asm/cputype.h>
#include <asm/elf.h>
#include <asm/cputable.h>
#include <asm/cpu_ops.h>
#include <asm/sections.h>
#include <asm/setup.h>
#include <asm/smp_plat.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/traps.h>
#include <asm/memblock.h>
#include <asm/psci.h>
#include <asm/efi.h>

unsigned int processor_id;
EXPORT_SYMBOL(processor_id);

unsigned long elf_hwcap __read_mostly;
EXPORT_SYMBOL_GPL(elf_hwcap);

unsigned int boot_reason;
EXPORT_SYMBOL(boot_reason);

unsigned int cold_boot;
EXPORT_SYMBOL(cold_boot);

char* (*arch_read_hardware_id)(void);
EXPORT_SYMBOL(arch_read_hardware_id);

#ifdef CONFIG_COMPAT
#define COMPAT_ELF_HWCAP_DEFAULT	\
				(COMPAT_HWCAP_HALF|COMPAT_HWCAP_THUMB|\
				 COMPAT_HWCAP_FAST_MULT|COMPAT_HWCAP_EDSP|\
				 COMPAT_HWCAP_TLS|COMPAT_HWCAP_VFP|\
				 COMPAT_HWCAP_VFPv3|COMPAT_HWCAP_VFPv4|\
				 COMPAT_HWCAP_NEON|COMPAT_HWCAP_IDIV)
unsigned int compat_elf_hwcap __read_mostly = COMPAT_ELF_HWCAP_DEFAULT;
unsigned int compat_elf_hwcap2 __read_mostly;
#endif

static const char *cpu_name;
static const char *machine_name;
phys_addr_t __fdt_pointer __initdata;

/*
 * Standard memory resources
 */
static struct resource mem_res[] = {
	{
		.name = "Kernel code",
		.start = 0,
		.end = 0,
		.flags = IORESOURCE_MEM
	},
	{
		.name = "Kernel data",
		.start = 0,
		.end = 0,
		.flags = IORESOURCE_MEM
	}
};

#define kernel_code mem_res[0]
#define kernel_data mem_res[1]

void __init early_print(const char *str, ...)
{
	char buf[256];
	va_list ap;

	va_start(ap, str);
	vsnprintf(buf, sizeof(buf), str, ap);
	va_end(ap);

	printk("%s", buf);
}

void __init smp_setup_processor_id(void)
{
	/*
	 * clear __my_cpu_offset on boot CPU to avoid hang caused by
	 * using percpu variable early, for example, lockdep will
	 * access percpu variable inside lock_release
	 */
	set_my_cpu_offset(0);
}

bool arch_match_cpu_phys_id(int cpu, u64 phys_id)
{
	return phys_id == cpu_logical_map(cpu);
}

struct mpidr_hash mpidr_hash;
#ifdef CONFIG_SMP
/**
 * smp_build_mpidr_hash - Pre-compute shifts required at each affinity
 *			  level in order to build a linear index from an
 *			  MPIDR value. Resulting algorithm is a collision
 *			  free hash carried out through shifting and ORing
 */
static void __init smp_build_mpidr_hash(void)
{
	u32 i, affinity, fs[4], bits[4], ls;
	u64 mask = 0;
	/*
	 * Pre-scan the list of MPIDRS and filter out bits that do
	 * not contribute to affinity levels, ie they never toggle.
	 */
	for_each_possible_cpu(i)
		mask |= (cpu_logical_map(i) ^ cpu_logical_map(0));
	pr_debug("mask of set bits %#llx\n", mask);
	/*
	 * Find and stash the last and first bit set at all affinity levels to
	 * check how many bits are required to represent them.
	 */
	for (i = 0; i < 4; i++) {
		affinity = MPIDR_AFFINITY_LEVEL(mask, i);
		/*
		 * Find the MSB bit and LSB bits position
		 * to determine how many bits are required
		 * to express the affinity level.
		 */
		ls = fls(affinity);
		fs[i] = affinity ? ffs(affinity) - 1 : 0;
		bits[i] = ls - fs[i];
	}
	/*
	 * An index can be created from the MPIDR_EL1 by isolating the
	 * significant bits at each affinity level and by shifting
	 * them in order to compress the 32 bits values space to a
	 * compressed set of values. This is equivalent to hashing
	 * the MPIDR_EL1 through shifting and ORing. It is a collision free
	 * hash though not minimal since some levels might contain a number
	 * of CPUs that is not an exact power of 2 and their bit
	 * representation might contain holes, eg MPIDR_EL1[7:0] = {0x2, 0x80}.
	 */
	mpidr_hash.shift_aff[0] = MPIDR_LEVEL_SHIFT(0) + fs[0];
	mpidr_hash.shift_aff[1] = MPIDR_LEVEL_SHIFT(1) + fs[1] - bits[0];
	mpidr_hash.shift_aff[2] = MPIDR_LEVEL_SHIFT(2) + fs[2] -
						(bits[1] + bits[0]);
	mpidr_hash.shift_aff[3] = MPIDR_LEVEL_SHIFT(3) +
				  fs[3] - (bits[2] + bits[1] + bits[0]);
	mpidr_hash.mask = mask;
	mpidr_hash.bits = bits[3] + bits[2] + bits[1] + bits[0];
	pr_debug("MPIDR hash: aff0[%u] aff1[%u] aff2[%u] aff3[%u] mask[%#llx] bits[%u]\n",
		mpidr_hash.shift_aff[0],
		mpidr_hash.shift_aff[1],
		mpidr_hash.shift_aff[2],
		mpidr_hash.shift_aff[3],
		mpidr_hash.mask,
		mpidr_hash.bits);
	/*
	 * 4x is an arbitrary value used to warn on a hash table much bigger
	 * than expected on most systems.
	 */
	if (mpidr_hash_size() > 4 * num_possible_cpus())
		pr_warn("Large number of MPIDR hash buckets detected\n");
	__flush_dcache_area(&mpidr_hash, sizeof(struct mpidr_hash));
}
#endif

static void __init setup_processor(void)
{
	struct cpu_info *cpu_info;
	u64 features, block;
	u32 cwg;
	int cls;

	cpu_info = lookup_processor_type(read_cpuid_id());
	if (!cpu_info) {
		printk("CPU configuration botched (ID %08x), unable to continue.\n",
		       read_cpuid_id());
		while (1);
	}

	cpu_name = cpu_info->cpu_name;

	printk("CPU: %s [%08x] revision %d\n",
	       cpu_name, read_cpuid_id(), read_cpuid_id() & 15);

	sprintf(init_utsname()->machine, ELF_PLATFORM);
	elf_hwcap = 0;

	/*
	 * Check for sane CTR_EL0.CWG value.
	 */
	cwg = cache_type_cwg();
	cls = cache_line_size();
	if (!cwg)
		pr_warn("No Cache Writeback Granule information, assuming cache line size %d\n",
			cls);
	if (L1_CACHE_BYTES < cls)
		pr_warn("L1_CACHE_BYTES smaller than the Cache Writeback Granule (%d < %d)\n",
			L1_CACHE_BYTES, cls);

	/*
	 * ID_AA64ISAR0_EL1 contains 4-bit wide signed feature blocks.
	 * The blocks we test below represent incremental functionality
	 * for non-negative values. Negative values are reserved.
	 */
	features = read_cpuid(ID_AA64ISAR0_EL1);
	block = (features >> 4) & 0xf;
	if (!(block & 0x8)) {
		switch (block) {
		default:
		case 2:
			elf_hwcap |= HWCAP_PMULL;
		case 1:
			elf_hwcap |= HWCAP_AES;
		case 0:
			break;
		}
	}

	block = (features >> 8) & 0xf;
	if (block && !(block & 0x8))
		elf_hwcap |= HWCAP_SHA1;

	block = (features >> 12) & 0xf;
	if (block && !(block & 0x8))
		elf_hwcap |= HWCAP_SHA2;

	block = (features >> 16) & 0xf;
	if (block && !(block & 0x8))
		elf_hwcap |= HWCAP_CRC32;

#ifdef CONFIG_COMPAT
	/*
	 * ID_ISAR5_EL1 carries similar information as above, but pertaining to
	 * the Aarch32 32-bit execution state.
	 */
	features = read_cpuid(ID_ISAR5_EL1);
	block = (features >> 4) & 0xf;
	if (!(block & 0x8)) {
		switch (block) {
		default:
		case 2:
			compat_elf_hwcap2 |= COMPAT_HWCAP2_PMULL;
		case 1:
			compat_elf_hwcap2 |= COMPAT_HWCAP2_AES;
		case 0:
			break;
		}
	}

	block = (features >> 8) & 0xf;
	if (block && !(block & 0x8))
		compat_elf_hwcap2 |= COMPAT_HWCAP2_SHA1;

	block = (features >> 12) & 0xf;
	if (block && !(block & 0x8))
		compat_elf_hwcap2 |= COMPAT_HWCAP2_SHA2;

	block = (features >> 16) & 0xf;
	if (block && !(block & 0x8))
		compat_elf_hwcap2 |= COMPAT_HWCAP2_CRC32;
#endif
}

static void __init setup_machine_fdt(phys_addr_t dt_phys)
{
	if (!dt_phys || !early_init_dt_scan(phys_to_virt(dt_phys))) {
		early_print("\n"
			"Error: invalid device tree blob at physical address 0x%p (virtual address 0x%p)\n"
			"The dtb must be 8-byte aligned and passed in the first 512MB of memory\n"
			"\nPlease check your bootloader.\n",
			dt_phys, phys_to_virt(dt_phys));

		while (true)
			cpu_relax();
	}

	machine_name = of_flat_dt_get_machine_name();
	if (machine_name)
		pr_info("Machine: %s\n", machine_name);
}

/*
 * Limit the memory size that was specified via FDT.
 */
static int __init early_mem(char *p)
{
	phys_addr_t limit;

	if (!p)
		return 1;

	limit = memparse(p, &p) & PAGE_MASK;
	pr_notice("Memory limited to %lldMB\n", limit >> 20);

	memblock_enforce_memory_limit(limit);

	return 0;
}
early_param("mem", early_mem);

static void __init request_standard_resources(void)
{
	struct memblock_region *region;
	struct resource *res;

	kernel_code.start   = virt_to_phys(_text);
	kernel_code.end     = virt_to_phys(_etext - 1);
	kernel_data.start   = virt_to_phys(_sdata);
	kernel_data.end     = virt_to_phys(_end - 1);

	for_each_memblock(memory, region) {
		res = alloc_bootmem_low(sizeof(*res));
		res->name  = "System RAM";
		res->start = __pfn_to_phys(memblock_region_memory_base_pfn(region));
		res->end = __pfn_to_phys(memblock_region_memory_end_pfn(region)) - 1;
		res->flags = IORESOURCE_MEM | IORESOURCE_BUSY;

		request_resource(&iomem_resource, res);

		if (kernel_code.start >= res->start &&
		    kernel_code.end <= res->end)
			request_resource(res, &kernel_code);
		if (kernel_data.start >= res->start &&
		    kernel_data.end <= res->end)
			request_resource(res, &kernel_data);
	}
}

u64 __cpu_logical_map[NR_CPUS] = { [0 ... NR_CPUS-1] = INVALID_HWID };

void __init __weak init_random_pool(void) { }

/*
 * HACK: These two functions sets the androidboot.mode=charger based
 * on the Sony Mobile parameters startup and warmboot.
 */
static unsigned long sony_startup;

static int __init sony_param_startup(char *p)
{
	if (kstrtoul(p, 16, &sony_startup))
		return 1;
	return 0;
}
early_param("startup", sony_param_startup);

static int __init sony_param_warmboot(char *p)
{
	unsigned long warmboot;

	if (kstrtoul(p, 16, &warmboot))
		return 1;

	if (!warmboot && (sony_startup & 0x4004))
		strlcat(boot_command_line, " androidboot.mode=charger",
			COMMAND_LINE_SIZE);
	return 0;
}
early_param("warmboot", sony_param_warmboot);

void __init setup_arch(char **cmdline_p)
{
	/*
	 * Unmask asynchronous aborts early to catch possible system errors.
	 */
	local_async_enable();

	setup_processor();

	setup_machine_fdt(__fdt_pointer);

	init_mm.start_code = (unsigned long) _text;
	init_mm.end_code   = (unsigned long) _etext;
	init_mm.end_data   = (unsigned long) _edata;
	init_mm.brk	   = (unsigned long) _end;

	*cmdline_p = boot_command_line;

	init_mem_pgprot();
	early_ioremap_init();

	parse_early_param();

	efi_init();
	arm64_memblock_init();

	paging_init();
	request_standard_resources();

	efi_idmap_init();

	unflatten_device_tree();

	psci_init();

	cpu_logical_map(0) = read_cpuid_mpidr() & MPIDR_HWID_BITMASK;
	cpu_read_bootcpu_ops();
#ifdef CONFIG_SMP
	smp_init_cpus();
	smp_build_mpidr_hash();
#endif

#ifdef CONFIG_VT
#if defined(CONFIG_VGA_CONSOLE)
	conswitchp = &vga_con;
#elif defined(CONFIG_DUMMY_CONSOLE)
	conswitchp = &dummy_con;
#endif
#endif
	init_random_pool();
}

static int __init arm64_device_init(void)
{
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
	return 0;
}
arch_initcall(arm64_device_init);

static DEFINE_PER_CPU(struct cpu, cpu_data);

static int __init topology_init(void)
{
	int i;

	for_each_possible_cpu(i) {
		struct cpu *cpu = &per_cpu(cpu_data, i);
		cpu->hotpluggable = 1;
		register_cpu(cpu, i);
	}

	return 0;
}
postcore_initcall(topology_init);

static const char *hwcap_str[] = {
	"fp",
	"asimd",
	"evtstrm",
	"aes",
	"pmull",
	"sha1",
	"sha2",
	"crc32",
	NULL
};

static int c_show(struct seq_file *m, void *v)
{
	int i;

	seq_printf(m, "Processor\t: %s rev %d (%s)\n",
		   cpu_name, read_cpuid_id() & 15, ELF_PLATFORM);

	for_each_present_cpu(i) {
		/*
		 * glibc reads /proc/cpuinfo to determine the number of
		 * online processors, looking for lines beginning with
		 * "processor".  Give glibc what it expects.
		 */
#ifdef CONFIG_SMP
		seq_printf(m, "processor\t: %d\n", i);
#endif
	}

	/* dump out the processor features */
	seq_puts(m, "Features\t: ");

	for (i = 0; hwcap_str[i]; i++)
		if (elf_hwcap & (1 << i))
			seq_printf(m, "%s ", hwcap_str[i]);
#ifdef CONFIG_ARMV7_COMPAT_CPUINFO
	if (is_compat_task()) {
		/* Print out the non-optional ARMv8 HW capabilities */
		seq_printf(m, "wp half thumb fastmult vfp edsp neon vfpv3 tlsi ");
		seq_printf(m, "vfpv4 idiva idivt ");
	}
#endif

	seq_printf(m, "\nCPU implementer\t: 0x%02x\n", read_cpuid_id() >> 24);
	seq_printf(m, "CPU architecture: 8\n");
	seq_printf(m, "CPU variant\t: 0x%x\n", (read_cpuid_id() >> 20) & 15);
	seq_printf(m, "CPU part\t: 0x%03x\n", (read_cpuid_id() >> 4) & 0xfff);
	seq_printf(m, "CPU revision\t: %d\n", read_cpuid_id() & 15);

	seq_puts(m, "\n");

	if (!arch_read_hardware_id)
		seq_printf(m, "Hardware\t: %s\n", machine_name);
	else
		seq_printf(m, "Hardware\t: %s\n", arch_read_hardware_id());

	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show
};

void arch_setup_pdev_archdata(struct platform_device *pdev)
{
	pdev->archdata.dma_mask = DMA_BIT_MASK(32);
	pdev->dev.dma_mask = &pdev->archdata.dma_mask;
}

/* Used for 8994 Only */
int msm8994_req_tlbi_wa = 1;
#define SOC_MAJOR_REV(val) (((val) & 0xF00) >> 8)

static int __init msm8994_check_tlbi_workaround(void)
{
	void __iomem *addr;
	int major_rev;
	struct device_node *dn = of_find_compatible_node(NULL,
						NULL, "qcom,cpuss-8994");
	if (dn) {
		addr = of_iomap(dn, 0);
		if (!addr)
			return -ENOMEM;
		major_rev  = SOC_MAJOR_REV((__raw_readl(addr)));
		msm8994_req_tlbi_wa = (major_rev >= 2) ? 0 : 1;
	} else {
		/* If the node does not exist disable the workaround */
		msm8994_req_tlbi_wa = 0;
	}

	return 0;
}
arch_initcall_sync(msm8994_check_tlbi_workaround);

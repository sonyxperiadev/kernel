#ifndef __BCM2708_SMI_H
#define __BCM2708_SMI_H

typedef enum {
	SMI_TRANSFER_WIDTH_MIN,
	SMI_TRANSFER_WIDTH_8BIT,
	SMI_TRANSFER_WIDTH_9BIT,
	SMI_TRANSFER_WIDTH_16BIT,
	SMI_TRANSFER_WIDTH_18BIT,
	SMI_TRANSFER_WIDTH_MAX
} SMI_TRANSFER_WIDTH_T;

typedef enum {
	SMI_MODE_MIN,
	SMI_MODE_80,
	SMI_MODE_68,
	SMI_MODE_MAX
} SMI_MODE_T;

typedef enum {
	SMI_PIXEL_FORMAT_MIN,
	SMI_PIXEL_FORMAT_DONT_CARE,
	SMI_PIXEL_FORAMT_16BIT_RGB565,
	SMI_PIXEL_FORMAT_32BIT_RGB888,
	SMI_PIXEL_FORMAT_NAND,
	SMI_PIXEL_FORMAT_NONE,
	SMI_PIXEL_FORMAT_MAX
} SMI_PIXEL_FORMAT_T;

struct smi_periph_timings {
	//setup time (in ns)
	uint32_t setup_time_in_ns;
	//strobe time (in ns)
	uint32_t strobe_time_in_ns;
	//hold time (in ns)
	uint32_t hold_time_in_ns;
	//pace time(in ns)
	uint32_t pace_time_in_ns;
	//the transfer width of the peripheral
	SMI_TRANSFER_WIDTH_T  transfer_width;
};

struct smi_periph_setup {
	//Mode of the peripheral - mode80 or mode68
	SMI_MODE_T mode;
	//the pixel format - can be SMI_PIXEL_FORMAT_DONT_CARE
	SMI_PIXEL_FORMAT_T pixel_format;
	//pixel data bits swapped? ignore if (pixel_format == SMI_PIXEL_FORMAT_DONT_CARE)
	uint32_t pixel_bits_swapped;
	// hold programmed transfers until tearing effect signal?
	uint32_t tearing_effect;
	// HVS acting as input? Only meaningful for VC4
	uint32_t hvs_input;
	//Timing parameters (in ns)
	struct smi_periph_timings read_timings;
	struct smi_periph_timings write_timings;
};

int32_t smi_write (const uint32_t slot, const uint32_t address,
		const uint32_t data_size_in_bytes, const uint32_t blocking,
		const void *data);

int32_t smi_read (const uint32_t slot, const uint32_t address,
		const uint32_t data_size_in_bytes, const uint32_t blocking,
		const void *data);

int32_t smi_wait_transfer_complete(void);

int32_t smi_setup_timing( const uint32_t slot, 
		const struct smi_periph_setup *periph_setup, const uint32_t smi_freq_hz );
#endif /* __BCM2708_SMI_H */

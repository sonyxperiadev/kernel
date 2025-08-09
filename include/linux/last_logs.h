#ifndef _LAST_LOGS_H
#define _LAST_LOGS_H

#define MAX_LAST_LOGS_REGIONS 32
#define LAST_LOGS_VERSION 0x00010000
#define LAST_LOGS_MAGIC 0x442474D4
#define MAX_NAME_LENGTH 16
#define LAST_LOGS_HEADER_SIZE sizeof(last_logs_header)

typedef struct __packed {
	char name[MAX_NAME_LENGTH];
	uint32_t offset;
	uint32_t size;
	uint32_t reserved;
} last_logs_region;

typedef struct __packed {
	uint32_t version;
	uint32_t magic;
	uint32_t num_regions;
	uint32_t reserved[3];
	last_logs_region regions[MAX_LAST_LOGS_REGIONS];
} last_logs_header;

struct last_logs_data {
	void *addr;
	uint32_t size;
	struct proc_dir_entry *procfs_file;
};

#endif /* _LAST_LOGS_H */

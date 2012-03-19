#ifndef _TOFE_H
#define _TOFE_H

#define BCM_TSC_CHIP_REV 64
#define BCM_TSC_CHIP_ID0 65
#define BCM_TSC_CHIP_ID1 66
#define BCM_TSC_CHIP_ID2 67

#define BCM_TSC_SPM_SLAVE   HW_BCM915500_SLAVE_SPM
#define BCM_TSC_AHB_SLAVE   HW_BCM915500_SLAVE_AHB

#define NAPA_MAX_TOUCH                                 10
#define NAPA_MAX_X                                   4096
#define NAPA_MAX_Y                                   4096

#define BCMTCH_SUCCESS                                  0
#define BCMTCH_ERROR                                    1

#define NAPA_INIT_SUCCESS                               0
#define NAPA_INIT_ERROR                                 1

#define NAPA_MUTEX                                      0

#define NAPA_I2C_SLAVE_ADDR                          HW_BCM915500_SLAVE_SPM
#define NAPA_I2C_AHB_SLAVE_ADDR                      HW_BCM915500_SLAVE_AHB

//mode values for DMA control
#define MahbRead                                        1
#define MahbWrite                                       3

#define BCM915500_TSC_FW_CODE "bcmtchfw_code"
#define BCM915500_TSC_FW_DATA "bcmtchfw_data"
#define BCM915500_TSC_FW_VECT "bcmtchfw_vect"

// ----------------------------------------------------------------------------------------------------
// shmem.h
// ----------------------------------------------------------------------------------------------------

#define ROM_SIZE		(32 * 1024)
#define RAM_START		0x10009000	// Do not know what to call this
#define RAM_OFFSET		RAM_START

#define I2C_REG_DMA_ADDR                              128
#define I2C_REG_STATUS                                137
#define I2C_REG_WFIFO_DATA                            146
#define I2C_REG_RFIFO_DATA                            162

#define TCC_REG_SPM_RevisionID                         64
#define TCC_REG_SPM_ChipID0                            65
#define TCC_REG_SPM_ChipID1                            66
#define TCC_REG_SPM_ChipID2                            67
#define TCC_REG_SPM_SPI_I2C_MODE                       68
#define SPI_HOST_I2CS_CHIPID                           69

#define TCC_REG_SPM_PSR                                72
#define TCC_REG_SPM_Request_from_Host                  76
#define TCC_REG_SPM_MSG_TO_HOST                        77
#define TCC_REG_SPM_MSG_FROM_HOST                      78

#define TCC_REG_SPM_SOFT_RESETS                        89

#define TCC_REG_SPM_ALFO_CTRL                          96

#define TOFE_MESSAGE_FW_READY                         128
#define TOFE_COMMAND_NO_COMMAND                         0
#define TOFE_COMMAND_INTERRUPT_ACK                      1
#define SPM_POWER_STATE_SLEEP                           0
#define SPM_POWER_STATE_ACTIVE                          4
#define TCC_HOST_IF_I2C_MODE                            0
#define TCC_SPM_REQUEST_FROM_HOST_WAKE_REQUEST          2
#define TCC_SPM_REQUEST_FROM_HOST_RELEASE_WAKE_REQUEST  0

#define COMMON_FLL_CTRL0                               (0x30110000+0x104)
#define COMMON_FLL_LPF_CTRL2                           (0x30110000+0x114)
#define COMMON_FLL_TEST_CTRL1                          (0x30110000+0x144)
#define SPM_PWR_CTRL                                   (0x30000000 + 0x1c + 0x00100000)
#define COMMON_SYS_HCLK_CTRL                           (0x30110000+0x20)
#define COMMON_CLOCK_ENABLE                            (0x30110000+0x48)
#define TCH_VERSION_REGISTER                           (0x30000000+0x00300000)
#define COMMON_ARM_REMAP_ADDR                          (0x30110000)
#define SPM_STICKY_BITS                                (0x30000000 + 0x00100000 + 0x144)

#define SPM_READ_ERROR_MAX                             200

#define VECTORS_OFFSET                                 0
#define ROM_OFFSET                                     (0x10000000)
#define DATA_OFFSET                                    (RAM_OFFSET)

// ----------------------------------------------------------------------------------------------------
// tofe_channel.h
// ----------------------------------------------------------------------------------------------------

typedef uint16_t iterator_t;

/**
	@enum tofe_channel_flag_t
    @brief Channel flag field bit assignment.
*/
typedef enum {
	TOFE_CHANNEL_FLAG_STATUS_OVERFLOW = 1 << 0,
	TOFE_CHANNEL_FLAG_STATUS_LEVEL_TRIGGER = 1 << 1,
	TOFE_CHANNEL_FLAG_OVERFLOW_STALL = 1 << 6,	// Stall on overflow
	TOFE_CHANNEL_FLAG_INBOUND = 1 << 7,	// max
} tofe_channel_flag_t;

/**
	@enum tofe_channel_level_t
    @brief Channel level control field.
*/
typedef enum {
	TOFE_CHANNEL_LEVEL_TRIGGER_OFF,
	TOFE_CHANNEL_LEVEL_TRIGGER_ON,
	TOFE_CHANNEL_LEVEL_TRIGGER_ACTIVE,
} tofe_channel_level_t;

/**
	@struct tofe_channel_header_t
    @brief Channel header object.  Channel object may be copied from controller
    to host at any time.  To ensure synchronization, updates are transactional.
    The read and write iterators are used during an update, the read and write
    fields are updated at the end of a transaction.

	Write iterator is always equal to write or ahead of it because we are in
	the middle of a write transaction.
*/
#define MAX_CHANNEL_EVENTS	255

typedef struct {
	uint32_t write;
	uint8_t entry_num;	// Number of entries.  Limited to 255 entries.
	uint8_t entry_size;	// Entry size in bytes.  Limited to 255 bytes.
	uint8_t trig_level;	// Number of entries in channel to trigger notification
	uint8_t flags;		// Bit definitions shared with configuration.
	uint32_t read;
	int32_t data_offset;	// Offset from header to data.  May be negative.
	iterator_t read_iterator;
	iterator_t write_iterator;
} tofe_channel_header_t;

/*
	Note: Internal use only function.
*/
static inline iterator_t
_tofe_channel_next_index(tofe_channel_header_t *channel, iterator_t iterator)
{
	return (iterator == channel->entry_num - 1) ? 0 : iterator + 1;
}

/*
	Note: Internal use only function.
*/
static inline char *_tofe_channel_entry(tofe_channel_header_t *channel,
					uint32_t byte_index)
{
	char *data_bytes = (char *)channel + channel->data_offset;
	return &data_bytes[byte_index];
}

/*
	Note: Internal use only function.
*/
static inline size_t
_tofe_channel_byte_index(tofe_channel_header_t *channel, iterator_t entry_index)
{
	return entry_index * channel->entry_size;
}

/**
    Check if a channel is empty.

    Events are not considered read or writen until the transaction is
    complete.  Therefore, a channel is empty even when in the middle of a
    set of writes.

    @param
		[in] channel Pointer to channel object.

    @retval
    	bool True if channel is empty.

*/
static inline bool tofe_channel_is_empty(tofe_channel_header_t *channel)
{
	return (channel->read == channel->write);
}

/**
    Begin a read transaction on a channel.  To maintain data consistency,
    reads to a channel must be bracketed by read begin/end calls.

    @param
		[in] channel Pointer to channel object.

    @retval
    	void

*/
static inline void tofe_channel_read_begin(tofe_channel_header_t *channel)
{
	channel->read_iterator = channel->read;
}

/**
    Read a single entry from a channel.  This function must be called during
    a read transaction.

    The pointer returned by this function points into the channel object itself.
    Callers should not modify or reuse this memory.  Callers may not free the
    memory.

    @param
		[in] channel Pointer to channel object.

    @retval
    	void * Pointer to returned entry.

*/
static inline void *tofe_channel_read(tofe_channel_header_t *channel)
{
	char *entry;
	size_t byte_index;

	// Validate that channel has entries.
	if (tofe_channel_is_empty(channel))
		return NULL;

	// Find entry in the channel.
	byte_index = _tofe_channel_byte_index(channel, channel->read_iterator);
	entry = (char *)_tofe_channel_entry(channel, byte_index);

	// Update the read iterator.
	channel->read_iterator =
	    _tofe_channel_next_index(channel, channel->read_iterator);

	return (void *)entry;
}

/**
    Finish a read transaction on a channel.  To maintain data consistency,
    reads to a channel must be bracketed by read begin/end calls.

    @param
		[in] channel Pointer to channel object.

    @retval
    	uint32_t Number of entries read from channel during this transaction.

*/
static inline uint32_t tofe_channel_read_end(tofe_channel_header_t *channel)
{
	uint32_t count = (channel->read_iterator >= channel->read) ?
	    (channel->read_iterator - channel->read) :
	    (channel->entry_num - (channel->read - channel->read_iterator));

	channel->read = channel->read_iterator;
	return count;
}

// ----------------------------------------------------------------------------------------------------
// tofe_toc.h
// ----------------------------------------------------------------------------------------------------

typedef
    enum tofe_toc_index {
	TOFE_TOC_INDEX_TOFE,
	TOFE_TOC_INDEX_PATCH,
	TOFE_TOC_INDEX_CHANNEL,
	TOFE_TOC_INDEX_TCH,

	TOFE_TOC_INDEX_SCAN,
	TOFE_TOC_INDEX_FILTER,
	TOFE_TOC_INDEX_DETECT,
	TOFE_TOC_INDEX_TRACK,

	TOFE_TOC_INDEX_REPORT,
	TOFE_TOC_INDEX_LOG,
	TOFE_TOC_INDEX_RECORD,
	TOFE_TOC_INDEX_MEMORY,

	TOFE_TOC_INDEX_UCODE,
	TOFE_TOC_INDEX_CMDBUFF,
	TOFE_TOC_INDEX_SDM_COEFFICIENTS,

	TOFE_TOC_INDEX_MAX = 32
} tofe_toc_index_e;

typedef struct {
	const void *const cfg[TOFE_TOC_INDEX_MAX];
} tofe_toc_t;

// ----------------------------------------------------------------------------------------------------
// tofe_sig.h
// ----------------------------------------------------------------------------------------------------

#define	TOFE_BUILD_ID_SIZE			8
#define	TOFE_SIGNATURE_MAGIC_SIZE	4

/* Magic Number - 8 bytes */
#define TOFE_MAGIC {0x00, 0x01, 0x53, 0x00}

#define TOFE_BUILD_ID(x)				#x
#define TOFE_BUILD_ID_TO_STRING(x)		TOFE_BUILD_ID(x)

/**
	@struct tofe_signature_t
    @brief Firmware ROM image signature structure.
*/
typedef struct {
	const char magic[TOFE_SIGNATURE_MAGIC_SIZE];
	const char build_release[4];
	const char build_version[TOFE_BUILD_ID_SIZE];
	const char build_date[TOFE_BUILD_ID_SIZE];
	const char build_time[TOFE_BUILD_ID_SIZE];
} tofe_signature_t;

/* Signature */
#define	TOFE_SIGNATURE_SIZE	sizeof(tofe_signature_t)

// ----------------------------------------------------------------------------------------------------
// tofe_channel_cfg.h
// ----------------------------------------------------------------------------------------------------

typedef enum {
	TOFE_CHANNEL_ID_TOUCH,	// 0 = #1
	TOFE_CHANNEL_ID_COMMAND,	// 1 = #2
	TOFE_CHANNEL_ID_LOG,	// 2 = #3
	TOFE_CHANNEL_ID_GESTURES,	// 3 = #4
	TOFE_CHANNEL_ID_RECORD,	// 4 = #5
	TOFE_CHANNEL_ID_PLAYBACK,	// 5 = #6
	TOFE_CHANNEL_ID_RESPONSE,	// 6 = #7

	/* Number of channels */
	TOFE_CHANNEL_ID_NUM
} tofe_channel_id_t;		// Used as index.

typedef struct {
	uint8_t entry_num;	// Must be > 0.
	uint8_t entry_size;	// Range [1..255].
	uint8_t trig_level;	// 0 - entry_num
	uint8_t flags;
	tofe_channel_header_t *channel_header;
	void *channel_data;
} tofe_channel_instance_cfg_t;

typedef struct {
	tofe_channel_instance_cfg_t channel_cfg[TOFE_CHANNEL_ID_NUM];
} tofe_channel_cfg_t;

// ----------------------------------------------------------------------------------------------------
// tofe_tool.h
// ----------------------------------------------------------------------------------------------------

/*! TOFE Tool Meta Data Struture. */
typedef struct {

	void *m_p_config;
	uint32_t m_config_size;
	uint8_t m_config_loaded;

	void *m_p_image;
	uint32_t m_image_size;
	uint8_t m_image_loaded;

	tofe_signature_t *m_p_sig;
	tofe_toc_t *m_p_toc;
	tofe_channel_cfg_t *m_p_channel_configs;

	const void *cfg[TOFE_TOC_INDEX_MAX];

} tofe_tool_t;

// ----------------------------------------------------------------------------------------------------
// TOFEChannel.h
// ----------------------------------------------------------------------------------------------------

typedef
    struct napa_channel {
	tofe_channel_instance_cfg_t cfg;
	tofe_channel_header_t header;
	uint32_t data;
} napa_channel_t;

typedef
    enum napa_channel_write_header {
	NAPA_CHANNEL_WRITE_HEADER_READER,
	NAPA_CHANNEL_WRITE_HEADER_WRITER
} napa_channel_write_header_e;

// ----------------------------------------------------------------------------------------------------
// tofe.h (new defines and structs in this header file)
// ----------------------------------------------------------------------------------------------------

// CTOFEChannel
typedef struct {

	napa_channel_t *channel;

} ctofe_channel_t;

// -----------------------------------------------------------------------------------------------------
// bcmtch_event.h
// -----------------------------------------------------------------------------------------------------

typedef enum {
	BCMTCH_EVENT_TYPE_INVALID,	// Don't use zero.

	// Core events.
	BCMTCH_EVENT_TYPE_FRAME,

	BCMTCH_EVENT_TYPE_DOWN,
	BCMTCH_EVENT_TYPE_MOVE,
	BCMTCH_EVENT_TYPE_UP,

	// Auxillary events.
	BCMTCH_EVENT_TYPE_TIMESTAMP,
} bcmtch_event_type_t;

typedef struct {
	uint32_t type:4;
	uint32_t:28;

	uint32_t _pad;
} bcmtch_event_t;

typedef struct {
	uint16_t type:4;
	uint16_t:12;

	uint16_t frame_id;
	uint32_t hash;
} bcmtch_event_frame_t;

typedef struct {
	uint16_t type:4;
	uint16_t track_tag:5;
	uint16_t flags:4;
	uint16_t tch_class:3;	/*      Touch class. C++ does not like class.
				   Use BCMTCH_EVENT_CLASS_TOUCH. */
	uint16_t width:8;	/* x direction lenght of bounding box */
	uint16_t height:8;	/* y direction lenght of bounding box */

	uint32_t z:8;		/* force/pressure for contact and
				   distance for hover */
	uint32_t x:12;
	uint32_t y:12;
} bcmtch_event_touch_t;

typedef enum {
	BCMTCH_EVENT_TIMESTAMP_TYPE_SCAN_BEGIN = 1,
	BCMTCH_EVENT_TIMESTAMP_TYPE_SCAN_END,
	BCMTCH_EVENT_TIMESTAMP_TYPE_MTC_BEGIN,
	BCMTCH_EVENT_TIMESTAMP_TYPE_MTC_END
} bcmtch_timestamp_type_t;

typedef struct {
	uint32_t type:4;
	uint32_t timestamp_type:4;
	uint32_t:24;

	uint32_t timestamp;
} bcmtch_event_timestamp_t;

#endif

#ifndef __UAPI_CAM_ISP_H__
#define __UAPI_CAM_ISP_H__

#include "cam_defs.h"
#include "cam_isp_vfe.h"
#include "cam_isp_ife.h"
#include "cam_cpas.h"

/* ISP driver name */
#define CAM_ISP_DEV_NAME                        "cam-isp"

/* HW type */
#define CAM_ISP_HW_BASE                         0
#define CAM_ISP_HW_CSID                         1
#define CAM_ISP_HW_VFE                          2
#define CAM_ISP_HW_IFE                          3
#define CAM_ISP_HW_ISPIF                        4
#define CAM_ISP_HW_MAX                          5

/* Color Pattern */
#define CAM_ISP_PATTERN_BAYER_RGRGRG            0
#define CAM_ISP_PATTERN_BAYER_GRGRGR            1
#define CAM_ISP_PATTERN_BAYER_BGBGBG            2
#define CAM_ISP_PATTERN_BAYER_GBGBGB            3
#define CAM_ISP_PATTERN_YUV_YCBYCR              4
#define CAM_ISP_PATTERN_YUV_YCRYCB              5
#define CAM_ISP_PATTERN_YUV_CBYCRY              6
#define CAM_ISP_PATTERN_YUV_CRYCBY              7
#define CAM_ISP_PATTERN_MAX                     8

/* Usage Type */
#define CAM_ISP_RES_USAGE_SINGLE                0
#define CAM_ISP_RES_USAGE_DUAL                  1
#define CAM_ISP_RES_USAGE_MAX                   2

/* Resource ID */
#define CAM_ISP_RES_ID_PORT                     0
#define CAM_ISP_RES_ID_CLK                      1
#define CAM_ISP_RES_ID_MAX                      2

/* Resource Type - Type of resource for the resource id
 * defined in cam_isp_vfe.h, cam_isp_ife.h
 */

/* Lane Type in input resource for Port */
#define CAM_ISP_LANE_TYPE_DPHY                  0
#define CAM_ISP_LANE_TYPE_CPHY                  1
#define CAM_ISP_LANE_TYPE_MAX                   2

/* ISP Resurce Composite Group ID */
#define CAM_ISP_RES_COMP_GROUP_NONE             0
#define CAM_ISP_RES_COMP_GROUP_ID_0             1
#define CAM_ISP_RES_COMP_GROUP_ID_1             2
#define CAM_ISP_RES_COMP_GROUP_ID_2             3
#define CAM_ISP_RES_COMP_GROUP_ID_3             4
#define CAM_ISP_RES_COMP_GROUP_ID_4             5
#define CAM_ISP_RES_COMP_GROUP_ID_5             6
#define CAM_ISP_RES_COMP_GROUP_ID_MAX           6

/* ISP packet opcode for ISP */
#define CAM_ISP_PACKET_OP_BASE                  0
#define CAM_ISP_PACKET_INIT_DEV                 1
#define CAM_ISP_PACKET_UPDATE_DEV               2
#define CAM_ISP_PACKET_OP_MAX                   3

/* ISP packet meta_data type for command buffer */
#define CAM_ISP_PACKET_META_BASE                0
#define CAM_ISP_PACKET_META_LEFT                1
#define CAM_ISP_PACKET_META_RIGHT               2
#define CAM_ISP_PACKET_META_COMMON              3
#define CAM_ISP_PACKET_META_DMI_LEFT            4
#define CAM_ISP_PACKET_META_DMI_RIGHT           5
#define CAM_ISP_PACKET_META_DMI_COMMON          6
#define CAM_ISP_PACKET_META_CLOCK               7
#define CAM_ISP_PACKET_META_CSID                8
#define CAM_ISP_PACKET_META_DUAL_CONFIG         9
#define CAM_ISP_PACKET_META_GENERIC_BLOB_LEFT   10
#define CAM_ISP_PACKET_META_GENERIC_BLOB_RIGHT  11
#define CAM_ISP_PACKET_META_GENERIC_BLOB_COMMON 12

/* DSP mode */
#define CAM_ISP_DSP_MODE_NONE                   0
#define CAM_ISP_DSP_MODE_ONE_WAY                1
#define CAM_ISP_DSP_MODE_ROUND                  2

/* ISP Generic Cmd Buffer Blob types */
#define CAM_ISP_GENERIC_BLOB_TYPE_HFR_CONFIG          0
#define CAM_ISP_GENERIC_BLOB_TYPE_CLOCK_CONFIG        1
#define CAM_ISP_GENERIC_BLOB_TYPE_BW_CONFIG           2
#define CAM_ISP_GENERIC_BLOB_TYPE_UBWC_CONFIG         3
#define CAM_ISP_GENERIC_BLOB_TYPE_CSID_CLOCK_CONFIG   4
#define CAM_ISP_GENERIC_BLOB_TYPE_FE_CONFIG           5
#define CAM_ISP_GENERIC_BLOB_TYPE_BW_CONFIG_V2        6
#define CAM_ISP_GENERIC_BLOB_TYPE_INIT_FRAME_DROP     10

/* Per Path Usage Data */
#define CAM_ISP_USAGE_INVALID     0
#define CAM_ISP_USAGE_LEFT_PX     1
#define CAM_ISP_USAGE_RIGHT_PX    2
#define CAM_ISP_USAGE_RDI         3

/* Query devices */
/**
 * struct cam_isp_dev_cap_info - A cap info for particular hw type
 *
 * @hw_type:            Hardware type for the cap info
 * @reserved:           reserved field for alignment
 * @hw_version:         Hardware version
 *
 */
struct cam_isp_dev_cap_info {
	uint32_t              hw_type;
	uint32_t              reserved;
	struct cam_hw_version hw_version;
};

/**
 * struct cam_isp_query_cap_cmd - ISP query device capability payload
 *
 * @device_iommu:               returned iommu handles for device
 * @cdm_iommu:                  returned iommu handles for cdm
 * @num_dev:                    returned number of device capabilities
 * @reserved:                   reserved field for alignment
 * @dev_caps:                   returned device capability array
 *
 */
struct cam_isp_query_cap_cmd {
	struct cam_iommu_handle       device_iommu;
	struct cam_iommu_handle       cdm_iommu;
	int32_t                       num_dev;
	uint32_t                      reserved;
	struct cam_isp_dev_cap_info   dev_caps[CAM_ISP_HW_MAX];
};

/* Acquire Device */
/**
 * struct cam_isp_out_port_info - An output port resource info
 *
 * @res_type:                   output resource type defined in file
 *                              cam_isp_vfe.h or cam_isp_ife.h
 * @format:                     output format of the resource
 * @wdith:                      output width in pixels
 * @height:                     output height in lines
 * @comp_grp_id:                composite group id for the resource.
 * @split_point:                split point in pixels for the dual VFE.
 * @secure_mode:                flag to tell if output should be run in secure
 *                              mode or not. See cam_defs.h for definition
 * @reserved:                   reserved field for alignment
 *
 */
struct cam_isp_out_port_info {
	uint32_t                res_type;
	uint32_t                format;
	uint32_t                width;
	uint32_t                height;
	uint32_t                comp_grp_id;
	uint32_t                split_point;
	uint32_t                secure_mode;
	uint32_t                reserved;
};

/**
 * struct cam_isp_in_port_info - An input port resource info
 *
 * @res_type:                   input resource type define in file
 *                              cam_isp_vfe.h or cam_isp_ife.h
 * @lane_type:                  lane type: c-phy or d-phy.
 * @lane_num:                   active lane number
 * @lane_cfg:                   lane configurations: 4 bits per lane
 * @vc:                         input virtual channel number
 * @dt:                         input data type number
 * @format:                     input format
 * @test_pattern:               test pattern for the testgen
 * @usage_type:                 whether dual vfe is required
 * @left_start:                 left input start offset in pixels
 * @left_stop:                  left input stop offset in pixels
 * @left_width:                 left input width in pixels
 * @right_start:                right input start offset in pixels.
 *                              Only for Dual VFE
 * @right_stop:                 right input stop offset in pixels.
 *                              Only for Dual VFE
 * @right_width:                right input width in pixels.
 *                              Only for dual VFE
 * @line_start:                 top of the line number
 * @line_stop:                  bottome of the line number
 * @height:                     input height in lines
 * @pixel_clk;                  sensor output clock
 * @batch_size:                 batch size for HFR mode
 * @dsp_mode:                   DSP stream mode (Defines as CAM_ISP_DSP_MODE_*)
 * @hbi_cnt:                    HBI count for the camif input
 * @reserved:                   Reserved field for alignment
 * @num_out_res:                number of the output resource associated
 * @data:                       payload that contains the output resources
 *
 */
struct cam_isp_in_port_info {
	uint32_t                        res_type;
	uint32_t                        lane_type;
	uint32_t                        lane_num;
	uint32_t                        lane_cfg;
	uint32_t                        vc;
	uint32_t                        dt;
	uint32_t                        format;
	uint32_t                        test_pattern;
	uint32_t                        usage_type;
	uint32_t                        left_start;
	uint32_t                        left_stop;
	uint32_t                        left_width;
	uint32_t                        right_start;
	uint32_t                        right_stop;
	uint32_t                        right_width;
	uint32_t                        line_start;
	uint32_t                        line_stop;
	uint32_t                        height;
	uint32_t                        pixel_clk;
	uint32_t                        batch_size;
	uint32_t                        dsp_mode;
	uint32_t                        hbi_cnt;
	uint32_t                        reserved;
	uint32_t                        num_out_res;
	struct cam_isp_out_port_info    data[1];
};

/**
 * struct cam_isp_resource - A resource bundle
 *
 * @resoruce_id:                resource id for the resource bundle
 * @length:                     length of the while resource blob
 * @handle_type:                type of the resource handle
 * @reserved:                   reserved field for alignment
 * @res_hdl:                    resource handle that points to the
 *                                     resource array;
 *
 */
struct cam_isp_resource {
	uint32_t                       resource_id;
	uint32_t                       length;
	uint32_t                       handle_type;
	uint32_t                       reserved;
	uint64_t                       res_hdl;
};

/**
 * struct cam_isp_port_hfr_config - HFR configuration for this port
 *
 * @resource_type:              Resource type
 * @subsample_pattern:          Subsample pattern. Used in HFR mode. It
 *                              should be consistent with batchSize and
 *                              CAMIF programming.
 * @subsample_period:           Subsample period. Used in HFR mode. It
 *                              should be consistent with batchSize and
 *                              CAMIF programming.
 * @framedrop_pattern:          Framedrop pattern
 * @framedrop_period:           Framedrop period
 * @reserved:                   Reserved for alignment
 */
struct cam_isp_port_hfr_config {
	uint32_t                       resource_type;
	uint32_t                       subsample_pattern;
	uint32_t                       subsample_period;
	uint32_t                       framedrop_pattern;
	uint32_t                       framedrop_period;
	uint32_t                       reserved;
} __attribute__((packed));

/**
 * struct cam_isp_resource_hfr_config - Resource HFR configuration
 *
 * @num_ports:                  Number of ports
 * @reserved:                   Reserved for alignment
 * @port_hfr_config:            HFR configuration for each IO port
 */
struct cam_isp_resource_hfr_config {
	uint32_t                       num_ports;
	uint32_t                       reserved;
	struct cam_isp_port_hfr_config port_hfr_config[1];
} __attribute__((packed));

/**
 * struct cam_isp_dual_split_params - dual isp spilt parameters
 *
 * @split_point:                Split point information x, where (0 < x < width)
 *                              left ISP's input ends at x + righ padding and
 *                              Right ISP's input starts at x - left padding
 * @right_padding:              Padding added past the split point for left
 *                              ISP's input
 * @left_padding:               Padding added before split point for right
 *                              ISP's input
 * @reserved:                   Reserved filed for alignment
 *
 */
struct cam_isp_dual_split_params {
	uint32_t                       split_point;
	uint32_t                       right_padding;
	uint32_t                       left_padding;
	uint32_t                       reserved;
};

/**
 * struct cam_isp_dual_stripe_config - stripe config per bus client
 *
 * @offset:                     Start horizontal offset relative to
 *                              output buffer
 *                              In UBWC mode, this value indicates the H_INIT
 *                              value in pixel
 * @width:                      Width of the stripe in bytes
 * @tileconfig                  Ubwc meta tile config. Contain the partial
 *                              tile info
 * @port_id:                    port id of ISP output
 *
 */
struct cam_isp_dual_stripe_config {
	uint32_t                       offset;
	uint32_t                       width;
	uint32_t                       tileconfig;
	uint32_t                       port_id;
};

/**
 * struct cam_isp_dual_config - dual isp configuration
 *
 * @num_ports                   Number of isp output ports
 * @reserved                    Reserved field for alignment
 * @split_params:               Inpput split parameters
 * @stripes:                    Stripe information
 *
 */
struct cam_isp_dual_config {
	uint32_t                           num_ports;
	uint32_t                           reserved;
	struct cam_isp_dual_split_params   split_params;
	struct cam_isp_dual_stripe_config  stripes[1];
} __attribute__((packed));

/**
 * struct cam_isp_clock_config - Clock configuration
 *
 * @usage_type:                 Usage type (Single/Dual)
 * @num_rdi:                    Number of RDI votes
 * @left_pix_hz:                Pixel Clock for Left ISP
 * @right_pix_hz:               Pixel Clock for Right ISP, valid only if Dual
 * @rdi_hz:                     RDI Clock. ISP clock will be max of RDI and
 *                              PIX clocks. For a particular context which ISP
 *                              HW the RDI is allocated to is not known to UMD.
 *                              Hence pass the clock and let KMD decide.
 */
struct cam_isp_clock_config {
	uint32_t                       usage_type;
	uint32_t                       num_rdi;
	uint64_t                       left_pix_hz;
	uint64_t                       right_pix_hz;
	uint64_t                       rdi_hz[1];
} __attribute__((packed));

/**
 * struct cam_isp_csid_clock_config - CSID clock configuration
 *
 * @csid_clock                  CSID clock
 */
struct cam_isp_csid_clock_config {
	uint64_t                       csid_clock;
} __attribute__((packed));

/**
 * struct cam_isp_bw_vote - Bandwidth vote information
 *
 * @resource_id:                Resource ID
 * @reserved:                   Reserved field for alignment
 * @cam_bw_bps:                 Bandwidth vote for CAMNOC
 * @ext_bw_bps:                 Bandwidth vote for path-to-DDR after CAMNOC
 */
struct cam_isp_bw_vote {
	uint32_t                       resource_id;
	uint32_t                       reserved;
	uint64_t                       cam_bw_bps;
	uint64_t                       ext_bw_bps;
} __attribute__((packed));

/**
 * struct cam_isp_bw_config - Bandwidth configuration
 *
 * @usage_type:                 Usage type (Single/Dual)
 * @num_rdi:                    Number of RDI votes
 * @left_pix_vote:              Bandwidth vote for left ISP
 * @right_pix_vote:             Bandwidth vote for right ISP
 * @rdi_vote:                   RDI bandwidth requirements
 */
struct cam_isp_bw_config {
	uint32_t                       usage_type;
	uint32_t                       num_rdi;
	struct cam_isp_bw_vote         left_pix_vote;
	struct cam_isp_bw_vote         right_pix_vote;
	struct cam_isp_bw_vote         rdi_vote[1];
} __attribute__((packed));


/**
 * struct cam_isp_bw_config_ab - Bandwidth configuration
 *
 * @usage_type:                    Usage type (Single/Dual)
 * @num_rdi:                       Number of RDI votes
 * @left_pix_vote_ab:              AB Bandwidth vote for left ISP
 * @right_pix_vote_ab:             AB Bandwidth vote for right ISP
 * @rdi_vote_ab:                   AB RDI bandwidth requirements
 */

struct cam_isp_bw_config_ab {
	uint32_t    usage_type;
	uint32_t    num_rdi;
	uint64_t    left_pix_vote_ab;
	uint64_t    right_pix_vote_ab;
	uint64_t    rdi_vote_ab[1];
} __attribute__((packed));

/**
 * struct cam_isp_bw_config_v2 - Bandwidth configuration
 *
 * @usage_type:                 Usage type (Single/Dual)
 * @num_paths:                  Number of axi data paths
 * @axi_path                    Per path vote info
 */
struct cam_isp_bw_config_v2 {
	uint32_t                             usage_type;
	uint32_t                             num_paths;
	struct cam_axi_per_path_bw_vote      axi_path[1];
} __attribute__((packed));

/**
 * struct cam_fe_config - Fetch Engine configuration
 *
 * @version:                    fetch engine veriosn
 * @min_vbi:                    require min vbi
 * @fs_mode:                    indicates if fs mode enabled
 * @fs_line_sync_en:            frame level sync or line level
 *                              sync for fetch engine
 * @hbi_count:                  hbi count
 * @fs_sync_enable:             indicates if fetch engine working
 *                              wokring in sync with write engine
 * @go_cmd_sel:                 softwrae go_cmd or hw go_cmd
 * @client_enable:              enable read engine
 * @source_addr:                adrress of buffer to read from
 * @width:                      buffer width
 * @height:                     buffer height
 * @stride:                     buffer stride (here equal to width)
 * @format:                     format of image in buffer
 * @unpacker_cfg:               unpacker config type
 * @latency_buf_size:           latency buffer for read engine
 */
struct cam_fe_config {
	uint64_t    version;
	uint32_t    min_vbi;
	uint32_t    fs_mode;
	uint32_t    fs_line_sync_en;
	uint32_t    hbi_count;
	uint32_t    fs_sync_enable;
	uint32_t    go_cmd_sel;
	uint32_t    client_enable;
	uint32_t    source_addr;
	uint32_t    width;
	uint32_t    height;
	uint32_t    stride;
	uint32_t    format;
	uint32_t    unpacker_cfg;
	uint32_t    latency_buf_size;
} __attribute__((packed));

/* Acquire Device/HW v2 */

/**
 * struct cam_isp_acquire_hw_info - ISP acquire HW params
 *
 * @common_info_version  : Version of common info struct used
 * @common_info_size     : Size of common info struct used
 * @common_info_offset   : Offset of common info from start of data
 * @num_inputs           : Number of inputs
 * @input_info_version   : Version of input info struct used
 * @input_info_size      : Size of input info struct used
 * @input_info_offset    : Offset of input info from start of data
 * @data                 : Start of data region
 */
struct cam_isp_acquire_hw_info {
	uint16_t                common_info_version;
	uint16_t                common_info_size;
	uint32_t                common_info_offset;
	uint32_t                num_inputs;
	uint32_t                input_info_version;
	uint32_t                input_info_size;
	uint32_t                input_info_offset;
	uint64_t                data;
};

#define CAM_ISP_ACQUIRE_COMMON_VER0         0x1000

#define CAM_ISP_ACQUIRE_COMMON_SIZE_VER0    0x0

#define CAM_ISP_ACQUIRE_INPUT_VER0          0x2000

#define CAM_ISP_ACQUIRE_INPUT_SIZE_VER0     sizeof(struct cam_isp_in_port_info)

#define CAM_ISP_ACQUIRE_OUT_VER0            0x3000

#define CAM_ISP_ACQUIRE_OUT_SIZE_VER0       sizeof(struct cam_isp_out_port_info)

/**
 * struct cam_isp_init_frame_drop_config - init frame drop configuration
 *
 * @init_frame_drop:            Initial number of frames needs to drop
 */

struct cam_isp_init_frame_drop_config {
	uint32_t                       init_frame_drop;
} __attribute__((packed));

#endif /* __UAPI_CAM_ISP_H__ */

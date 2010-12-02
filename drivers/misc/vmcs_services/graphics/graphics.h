#ifndef _GRAPHICS_H
#define _GRAPHICS_H

enum GRAPHICS_COMMAND_ID_T {
	GRAPHICS_IOCTL_RPC_ID = 0x81,
	GRAPHICS_IOCTL_RPC_TX_BULK_ID,
	GRAPHICS_IOCTL_RPC_RX_BULK_ID,
	GRAPHICS_IOCTL_CREATE_SEM_ID,
	GRAPHICS_IOCTL_ACQUIRE_SEM_ID,
	GRAPHICS_IOCTL_RELEASE_SEM_ID
};

struct graphics_ioctl_rpc;

#define GRAPHICS_IOCTL_RPC _IOWR('G', GRAPHICS_IOCTL_RPC_ID, \
	struct graphics_ioctl_rpc)

struct graphics_ioctl_rpc_tx_bulk;

#define GRAPHICS_IOCTL_RPC_TX_BULK _IOWR('G', GRAPHICS_IOCTL_RPC_TX_BULK_ID, \
        struct graphics_ioctl_rpc_tx_bulk)

struct graphics_ioctl_rpc_rx_bulk;

#define GRAPHICS_IOCTL_RPC_RX_BULK _IOWR('G', GRAPHICS_IOCTL_RPC_RX_BULK_ID, \
        struct graphics_ioctl_rpc_rx_bulk)

struct graphics_ioctl_create_sem;

#define GRAPHICS_IOCTL_CREATE_SEM _IOWR('G', GRAPHICS_IOCTL_CREATE_SEM_ID, \
        struct graphics_ioctl_create_sem)

struct graphics_ioctl_acquire_sem;

#define GRAPHICS_IOCTL_ACQUIRE_SEM _IOR('G', GRAPHICS_IOCTL_ACQUIRE_SEM_ID, \
        struct graphics_ioctl_acquire_sem)

struct graphics_ioctl_release_sem;

#define GRAPHICS_IOCTL_RELEASE_SEM _IOR('G', GRAPHICS_IOCTL_RELEASE_SEM_ID, \
        struct graphics_ioctl_release_sem)

struct graphics_txrx_ctrl {
	const uint32_t *request;
	uint32_t request_len;

	uint32_t *response;
	uint32_t response_max_len;
	uint32_t response_len;
};

struct graphics_ioctl_rpc {
	struct graphics_txrx_ctrl ctrl;
};

struct graphics_ioctl_rpc_tx_bulk {
	struct graphics_txrx_ctrl ctrl;
	const void *tx_bulk;
	uint32_t tx_bulk_len;
};

struct graphics_ioctl_rpc_rx_bulk {
	struct graphics_txrx_ctrl ctrl;
	void *rx_bulk;
};

struct graphics_ioctl_create_sem {
	uint32_t sem_no;
	uint32_t name;
	uint32_t count;
};

struct graphics_ioctl_acquire_sem {
	uint32_t sem_no;	
};

struct graphics_ioctl_release_sem {
	uint32_t sem_no;	
};

#endif

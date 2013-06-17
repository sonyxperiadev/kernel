 #define __ASM_BCMBT_LPM_H

 #include <linux/serial_core.h>
#include <linux/tty.h>
#ifdef CONFIG_HAS_WAKELOCK
 #include <linux/wakelock.h>
#endif
#ifdef CONFIG_KONA_PI_MGR
#include <mach/pi_mgr.h>
#include <plat/pi_mgr.h>
#endif


#define BTLPM_ENABLE_CLOCK 1
#define BTLPM_DISABLE_CLOCK 0

#define DISABLE_LPM 0
#define ENABLE_LPM_TYPE_OOB 1       /* BT_WAKE/HOST_WAKE fully automatic */
#define BCMBT_LPM_CLOCK_ENABLE 1
#define BCMBT_LPM_CLOCK_DISABLE 0

#ifndef BCMBT_LPM_BT_WAKE_ASSERT
#define BCMBT_LPM_BT_WAKE_ASSERT 0
#endif

#ifndef BCMBT_LPM_BT_WAKE_DEASSERT
#define BCMBT_LPM_BT_WAKE_DEASSERT (!(BCMBT_LPM_BT_WAKE_ASSERT))
#endif

#ifndef BCMBT_LPM_HOST_WAKE_ASSERT
#define BCMBT_LPM_HOST_WAKE_ASSERT 0
#endif
#ifndef BCMBT_LPM_HOST_WAKE_DEASSERT
#define BCMBT_LPM_HOST_WAKE_DEASSERT (!(BCMBT_LPM_HOST_WAKE_ASSERT))
#endif

#define BRCM_SHARED_UART_MAGIC	0x80
#define TIO_ASSERT_BT_WAKE	_IO(BRCM_SHARED_UART_MAGIC, 3)
#define TIO_DEASSERT_BT_WAKE	_IO(BRCM_SHARED_UART_MAGIC, 4)
#define TIO_GET_BT_WAKE_STATE	0x8005
#define TIO_GET_BT_UART_PORT	0x8006
#define TIO_GET_BT_FIRMWARE	0x8007

enum {
	IDLE,
	ACTIVE,
	NO_STATE
};
struct bcmbt_lpm_struct {
#ifdef CONFIG_HAS_WAKELOCK
	struct wake_lock bt_wake_lock;
	struct wake_lock host_wake_lock;
#endif
	struct pi_mgr_qos_node bt_wake_qos_node;
	struct pi_mgr_qos_node host_wake_qos_node;
	spinlock_t bcmbt_lpm_lock;
	struct timer_list hw_timer;
	uint8_t hw_timer_st;
	struct uart_port *uport;
	int host_irq;
};

struct bcmbt_platform_data {
	int bt_wake_gpio;
	int host_wake_gpio;
	int bt_uart_port;
};
struct bcmbt_lpm_entry_struct {
	struct bcmbt_platform_data *pdata;
	struct bcmbt_lpm_struct *plpm;
};


extern void serial8250_togglerts_afe(struct uart_port *port,
				unsigned int flowon);

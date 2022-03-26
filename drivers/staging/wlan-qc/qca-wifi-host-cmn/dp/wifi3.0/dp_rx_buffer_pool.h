/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _DP_RX_BUFFER_POOL_H_
#define _DP_RX_BUFFER_POOL_H_

#include "dp_types.h"
#include "qdf_nbuf.h"
#include "qdf_module.h"
#include "athdefs.h"
#include "wlan_cfg.h"
#include "dp_internal.h"
#include "dp_rx.h"
#include "dp_rx_mon.h"

#ifdef WLAN_FEATURE_RX_PREALLOC_BUFFER_POOL
/**
 * dp_rx_buffer_pool_init() - Initialize emergency buffer pool
 * @soc: SoC handle
 * @mac_id: MAC ID
 *
 * Return: None
 */
void dp_rx_buffer_pool_init(struct dp_soc *soc, u8 mac_id);

/**
 * dp_rx_buffer_pool_deinit() - De-Initialize emergency buffer pool
 * @soc: SoC handle
 * @mac_id: MAC ID
 *
 * Return: None
 */
void dp_rx_buffer_pool_deinit(struct dp_soc *soc, u8 mac_id);

/**
 * dp_rx_buffer_pool_refill() - Process the rx nbuf list and
 * refill the emergency buffer pool
 * @soc: SoC handle
 * @nbuf: RX buffer
 * @mac_id: MAC ID
 *
 * Return: Whether the rx nbuf is consumed into the pool or not.
 */
bool dp_rx_buffer_pool_refill(struct dp_soc *soc, qdf_nbuf_t nbuf, u8 mac_id);

/**
 * dp_rx_buffer_pool_nbuf_free() - Free the nbuf or queue it
 * back into the pool
 * @soc: SoC handle
 * @nbuf: RX buffer
 * @mac_id: MAC ID
 *
 * Return: None
 */
void dp_rx_buffer_pool_nbuf_free(struct dp_soc *soc, qdf_nbuf_t nbuf,
				 u8 mac_id);

/**
 * dp_rx_buffer_pool_nbuf_alloc() - Allocate nbuf for buffer replenish,
 * give nbuf from the pool if allocation fails
 * @soc: SoC handle
 * @mac_id: MAC ID
 * @rx_desc_pool: RX descriptor pool
 * @num_available_buffers: number of available buffers in the ring.
 *
 * Return: nbuf
 */
qdf_nbuf_t dp_rx_buffer_pool_nbuf_alloc(struct dp_soc *soc, uint32_t mac_id,
					struct rx_desc_pool *rx_desc_pool,
					uint32_t num_available_buffers);
#else
/**
 * dp_rx_buffer_pool_init() - Initialize emergency buffer pool
 * @soc: SoC handle
 * @mac_id: MAC ID
 *
 * Return: None
 */
static inline
void dp_rx_buffer_pool_init(struct dp_soc *soc, u8 mac_id)
{
	soc->rx_buff_pool[mac_id].is_initialized = false;
}

/**
 * dp_rx_buffer_pool_deinit() - De-Initialize emergency buffer pool
 * @soc: SoC handle
 * @mac_id: MAC ID
 *
 * Return: None
 */
static inline
void dp_rx_buffer_pool_deinit(struct dp_soc *soc, u8 mac_id)
{
}

/**
 * dp_rx_buffer_pool_refill() - Process the rx nbuf list and
 * refill the emergency buffer pool
 * @soc: SoC handle
 * @nbuf: RX buffer
 * @mac_id: MAC ID
 *
 * Return: Whether the rx nbuf is consumed into the pool or not.
 */
static inline
bool dp_rx_buffer_pool_refill(struct dp_soc *soc, qdf_nbuf_t nbuf, u8 mac_id)
{
	return false;
}

/**
 * dp_rx_buffer_pool_nbuf_free() - Free the nbuf or queue it
 * back into the pool
 * @soc: SoC handle
 * @nbuf: RX buffer
 * @mac_id: MAC ID
 *
 * Return: None
 */
static inline
void dp_rx_buffer_pool_nbuf_free(struct dp_soc *soc, qdf_nbuf_t nbuf,
				 u8 mac_id)
{
	qdf_nbuf_free(nbuf);
}

/**
 * dp_rx_buffer_pool_nbuf_alloc() - Allocate nbuf for buffer replenish,
 * give nbuf from the pool if allocation fails
 * @soc: SoC handle
 * @mac_id: MAC ID
 * @rx_desc_pool: RX descriptor pool
 * @num_available_buffers: number of available buffers in the ring.
 *
 * Return: nbuf
 */
static inline qdf_nbuf_t
dp_rx_buffer_pool_nbuf_alloc(struct dp_soc *soc, uint32_t mac_id,
			     struct rx_desc_pool *rx_desc_pool,
			     uint32_t num_available_buffers)
{
	return qdf_nbuf_alloc(soc->osdev, rx_desc_pool->buf_size,
			      RX_BUFFER_RESERVATION,
			      rx_desc_pool->buf_alignment, FALSE);
}
#endif /* WLAN_FEATURE_RX_PREALLOC_BUFFER_POOL */
#endif /* _DP_RX_BUFFER_POOL_H_ */

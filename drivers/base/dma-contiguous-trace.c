/*
 * dma-contiguous-trace.c
 *
 * Copyright (C) ST-Ericsson SA 2011
 * Author: <benjamin.gaignard@linaro.org> for ST-Ericsson.
 * License terms:  GNU General Public License (GPL), version 2
 */
#ifndef __CHECKER__

#define CREATE_TRACE_POINTS
#include "dma-contiguous-trace.h"

EXPORT_TRACEPOINT_SYMBOL(cma_alloc_start);
EXPORT_TRACEPOINT_SYMBOL(cma_alloc_end_success);
EXPORT_TRACEPOINT_SYMBOL(cma_alloc_end_failed);

EXPORT_TRACEPOINT_SYMBOL(cma_release_start);
EXPORT_TRACEPOINT_SYMBOL(cma_release_end);

#endif

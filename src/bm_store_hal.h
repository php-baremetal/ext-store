/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-store                                             |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@php-baremetal.com>         |
   +----------------------------------------------------------------------+
*/

/* Platform primitives ext-store needs; implemented by the platform's store HAL. */
#pragma once
#include <stdbool.h>
#include <stddef.h>

/* Called at MINIT. Without a configured store, ready() stays false. */
void bm_store_hal_init(void);
bool bm_store_hal_ready(void);

bool bm_store_hal_set(const char *key, const char *val);
/* buf == NULL: write the size, terminator included, into *len. false if absent. */
bool bm_store_hal_get(const char *key, char *buf, size_t *len);
bool bm_store_hal_has(const char *key);
bool bm_store_hal_delete(const char *key);
bool bm_store_hal_clear(void);
/* false when the keys cannot all be listed (out of memory, NVS error): cb was not called. */
bool bm_store_hal_keys(void (*cb)(const char *key, void *ctx), void *ctx);

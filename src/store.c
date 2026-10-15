/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-store                                             |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@aurecchia.com>             |
   +----------------------------------------------------------------------+
*/

/* store_*: a key-value store that survives resets, over the platform's store HAL. */
#include "php.h"
#include "bm_store_hal.h"
#include "zend_exceptions.h"

#define STORE_KEY_MAX 15

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_store_bool, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_store_keys, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

static inline bool key_ok(size_t len)
{
    return bm_store_hal_ready() && len > 0 && len <= STORE_KEY_MAX;
}

static void store_key_cb(const char *key, void *ctx)
{
    add_next_index_string((zval *) ctx, key);
}

/* store_set(string $key, string $value): bool */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_store_set, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()
PHP_FUNCTION(store_set)
{
    char *key, *val;
    size_t klen, vlen;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STRING(key, klen)
        Z_PARAM_STRING(val, vlen)
    ZEND_PARSE_PARAMETERS_END();

    if (!key_ok(klen)) {
        RETURN_FALSE;
    }
    RETURN_BOOL(bm_store_hal_set(key, val));
}

/* store_get(string $key, ?string $default = null): ?string */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_store_get, 0, 1, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, default, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()
PHP_FUNCTION(store_get)
{
    char *key;
    size_t klen;
    zval *def = NULL;
    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STRING(key, klen)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(def)
    ZEND_PARSE_PARAMETERS_END();

    if (key_ok(klen)) {
        size_t len = 0;
        if (bm_store_hal_get(key, NULL, &len) && len > 0) {
            char *buf = emalloc(len);
            if (bm_store_hal_get(key, buf, &len)) {
                RETVAL_STRINGL(buf, len - 1);   /* len includes the terminator */
                efree(buf);
                return;
            }
            efree(buf);
        }
    }
    if (def) {
        RETURN_ZVAL(def, 1, 0);
    }
    RETURN_NULL();
}

/* store_has(string $key): bool, shared with store_delete */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_store_key, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()
PHP_FUNCTION(store_has)
{
    char *key;
    size_t klen;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(key, klen)
    ZEND_PARSE_PARAMETERS_END();

    if (!key_ok(klen)) {
        RETURN_FALSE;
    }
    RETURN_BOOL(bm_store_hal_has(key));
}

/* store_delete(string $key): bool */
PHP_FUNCTION(store_delete)
{
    char *key;
    size_t klen;
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(key, klen)
    ZEND_PARSE_PARAMETERS_END();

    if (!key_ok(klen)) {
        RETURN_FALSE;
    }
    RETURN_BOOL(bm_store_hal_delete(key));
}

/* store_clear(): bool */
PHP_FUNCTION(store_clear)
{
    ZEND_PARSE_PARAMETERS_NONE();
    if (!bm_store_hal_ready()) {
        RETURN_FALSE;
    }
    RETURN_BOOL(bm_store_hal_clear());
}

/* store_keys(): array */
PHP_FUNCTION(store_keys)
{
    ZEND_PARSE_PARAMETERS_NONE();
    array_init(return_value);
    if (!bm_store_hal_ready()) {
        return;
    }
    if (!bm_store_hal_keys(store_key_cb, return_value)) {
        zval_ptr_dtor(return_value);
        zend_throw_exception(zend_ce_exception, "store_keys(): cannot list the keys (out of memory or NVS error)", 0);
        RETURN_THROWS();
    }
}

/* store_available(): bool */
PHP_FUNCTION(store_available)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_BOOL(bm_store_hal_ready());
}

PHP_MINIT_FUNCTION(store)
{
    bm_store_hal_init();
    return SUCCESS;
}

static const zend_function_entry store_functions[] = {
    PHP_FE(store_set,       arginfo_store_set)
    PHP_FE(store_get,       arginfo_store_get)
    PHP_FE(store_has,       arginfo_store_key)
    PHP_FE(store_delete,    arginfo_store_key)
    PHP_FE(store_clear,     arginfo_store_bool)
    PHP_FE(store_keys,      arginfo_store_keys)
    PHP_FE(store_available, arginfo_store_bool)
    PHP_FE_END
};

zend_module_entry store_module_entry = {
    STANDARD_MODULE_HEADER,
    "store",
    store_functions,
    PHP_MINIT(store),
    NULL, NULL, NULL, NULL,
    "1.0",
    STANDARD_MODULE_PROPERTIES,
};

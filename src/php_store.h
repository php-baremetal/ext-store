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

/* store_*: persistent key-value store. */
#ifndef PHP_STORE_H
#define PHP_STORE_H

#include "php.h"

extern zend_module_entry store_module_entry;
#define phpext_store_ptr (&store_module_entry)

#endif /* PHP_STORE_H */

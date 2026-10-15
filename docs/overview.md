---
eyebrow: 'Package · Extension'
lede:    'A key-value store that survives resets and power cuts: store_set() a string in one boot, store_get() it back in the next. Needs a store-hal provider for your platform and a partition size, which is off by default.'
see_also:
  - { href: './usage/php-api.md', meta: '5 min' }
  - { href: './reference/limits.md', meta: '4 min' }
  - { href: './recipes/boot-counter.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/esp32-store-hal#docs/overview.md', meta: 'HAL', label: 'esp32-store-hal' }
  - { href: 'package:php-baremetal/ext-mem#docs/overview.md', meta: 'Extension', label: 'ext-mem (in-RAM store)' }
prev: { label: 'No previous page', href: '#' }
next: { label: 'PHP API', href: './usage/php-api.md' }
---

# Persistent store

`ext-store` adds seven `store_*` functions to the PHP that runs on your board. They read and write string
values under short keys, and the values are still there after a reset or a power cut: a boot counter, the
user's settings, a calibration offset, a provisioning flag.

<!-- @code-block language="php" label="project-src/index.php — a value that survives a reset" -->
```php
<?php
$boots = (int) store_get('boots', '0') + 1;
store_set('boots', (string) $boots);
```
<!-- @endcode-block -->

## When to use it

Use it for values written rarely and read often that must survive a reset. For state that changes on
every loop tick or HTTP request, use the in-RAM `mem_*` functions from
[ext-mem](package:php-baremetal/ext-mem#docs/overview.md): they touch no flash, but are wiped on reset.

## Requirements

`ext-store` needs a **`store-hal` provider for your platform**, the package that does the actual flash
access. On ESP32 that is `php-baremetal/esp32-store-hal`, which stores the data in NVS (the chip's
wear-levelled, power-loss-safe key-value flash area) in a dedicated `phpstore` partition. PHP `>=8.3`.

## Install

<!-- @code-block language="bash" label="terminal — from the firmware project" -->
```bash
composer require php-baremetal/ext-store php-baremetal/esp32-store-hal
```
<!-- @endcode-block -->

Require the provider explicitly, as shown: the build plugin does not pick it for you on a fresh project.

## Enable the partition

The store needs its own slice of flash, so it is **off by default**. Set
`extra.baremetal.store.size_kb` in the firmware project's `composer.json`, then rebuild and flash:

<!-- @code-block language="json" label="composer.json" -->
```js
{
    "require": {
        "php-baremetal/ext-store": "^1.0",
        "php-baremetal/esp32-store-hal": "^1.0"
    },
    "extra": {
        "baremetal": {
            "platform": "esp32",
            "store": { "size_kb": 32 }
        }
    }
}
```
<!-- @endcode-block -->

| `size_kb` | Result |
| --------- | ------ |
| absent or `0` | No store partition. `store_available()` returns `false`; every other call is inert. |
| `1`–`16` | Raised to 16 KB, the minimum NVS needs. |
| `> 16` | Rounded up to the next 4 KB. |

32 KB holds a few hundred short entries. The build log prints the resolved size
(`persistent 'phpstore' NVS partition = 32K`), and the serial log at boot prints
`persistent store ready (partition 'phpstore')` when the store opened.

<!-- @callout variant="note" title="Off means inert, not missing" -->
Without a partition the functions still exist. `store_available()` returns `false`, the writes return
`false`, `store_get()` returns your default and `store_keys()` returns `[]`. Nothing throws, so one script
runs on builds with and without a store.
<!-- @endcallout -->

## PHP API

| Function | Returns | Meaning |
| -------- | ------- | ------- |
| `store_set(string $key, string $value)` | `bool` | Save a value; committed to flash before it returns. |
| `store_get(string $key, ?string $default = null)` | `?string` | The value, or `$default` when absent. |
| `store_has(string $key)` | `bool` | Whether the key exists. |
| `store_delete(string $key)` | `bool` | Remove one key. `false` if it was not there. |
| `store_clear()` | `bool` | Remove every key. |
| `store_keys()` | `array` | The stored keys, unordered. |
| `store_available()` | `bool` | Whether the store is configured and opened. |

Full reference with every failure case: [PHP API](./usage/php-api.md).

## Dependencies

Requires `php >=8.3` and the `php-baremetal/store-hal` virtual, provided on ESP32 by
`php-baremetal/esp32-store-hal`. Provides nothing; no other package depends on it.

## Notes

- **Keys are 1 to 15 bytes.** A longer or empty key is treated as missing: writes return `false`, reads
  return the default.
- **Values are strings**, about 4 KB at most, and end at the first NUL byte. Cast numbers, `json_encode()`
  structures, `base64_encode()` binary data.
- **Every write is committed** before `store_set()`, `store_delete()` or `store_clear()` returns.
- **Flash wears out.** Write on change, not on every tick or request.
- **The store can be wiped at boot.** If the partition is found full of unusable pages or written by a newer
  NVS format, it is erased and re-formatted before opening. Keep the store for settings and state, not as
  the only copy of irreplaceable data.

Details: [Limits and durability](./reference/limits.md).

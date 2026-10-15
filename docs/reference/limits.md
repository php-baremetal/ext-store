---
eyebrow: 'Package · Extension'
lede:    'What the store accepts and guarantees: keys of 1–15 bytes, string values up to about 4 KB with no NUL bytes, a commit on every write, and flash that wears out if you write in a hot loop.'
see_also:
  - { href: '../usage/php-api.md', meta: '5 min' }
  - { href: '../recipes/save-settings.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/ext-mem#docs/overview.md', meta: 'Extension', label: 'ext-mem (in-RAM store)' }
  - { href: 'https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_flash.html', meta: 'external', label: 'ESP-IDF NVS' }
prev: { label: 'PHP API', href: '../usage/php-api.md' }
next: { label: 'Count boots', href: '../recipes/boot-counter.md' }
---

# Limits and durability

The rules below are what you will hit in practice. On ESP32 the store is NVS flash, so its size limits
come from NVS.

## Keys

| Rule | When broken |
| ---- | ----------- |
| 1 to 15 bytes long (byte length, not characters) | Treated as a missing key: `store_set()` returns `false`, `store_get()` the default. |
| Unique in the store | `store_set()` on an existing key overwrites it. |
| No NUL byte | The key is cut at the NUL, so `"a\0b"` is stored as `"a"`. |

Short prefixes keep keys readable inside 15 bytes: `wifi.ssid`, `cal.x_off`, `boot.count`. While
developing, check what `store_set()` returns: a `false` usually means the key is too long.

## Values

- **Strings only.** Store numbers with `(string)` and cast back with `(int)` / `(float)`; store arrays with
  `json_encode()` and read them with `json_decode(..., true)`.
- **No NUL bytes.** A value ends at its first NUL byte; the rest is silently dropped. Encode binary data
  with `base64_encode()` or `bin2hex()`.
- **About 4 KB per value.** A longer string is rejected and `store_set()` returns `false`.
- **Bounded total.** The number of entries is limited by the partition size; when it is full,
  `store_set()` returns `false`.

<!-- @code-block language="php" label="numbers, structures, bytes" -->
```php
<?php
store_set('threshold', (string) 42);
$threshold = (int) store_get('threshold', '0');

store_set('cfg', json_encode(['ssid' => 'lab', 'ch' => 6]));
$cfg = json_decode(store_get('cfg', '{}'), true);

store_set('blob', base64_encode($bytes));      // raw bytes may contain NUL
$bytes = base64_decode(store_get('blob', ''));
```
<!-- @endcode-block -->

## Durability

- `store_set()`, `store_delete()` and `store_clear()` **commit before they return**. There is no flush call
  and no write-back cache: a value is safe the moment the call returns `true`.
- A reset or power cut during a write leaves either the old or the new value, never a torn one.
- `store_clear()` removes only the script's own keys, never system data.

<!-- @callout variant="warning" title="Flash wears out" -->
Every write erases and rewrites flash, and flash cells survive a finite number of erase cycles. NVS spreads
writes around, but a `store_set()` on every loop tick or every HTTP request will eventually exhaust the
partition. Write only when a value changes, and keep fast-changing values in `mem_*` (`ext-mem`), copying
them to the store occasionally.
<!-- @endcallout -->

<!-- @code-block language="php" label="write only on change" -->
```php
<?php
if (store_get('mode') !== $mode) {
    store_set('mode', $mode);
}
```
<!-- @endcode-block -->

<!-- @callout variant="danger" title="The store can be wiped at boot" -->
At boot, if the store partition is found full of unusable pages or written by a newer NVS format, it is
erased and re-formatted before opening, and every key is lost. Keep a way to rebuild what you store
(defaults, re-provisioning) and do not treat the store as the only copy of irreplaceable data.
<!-- @endcallout -->

## Partition size

The size comes from `extra.baremetal.store.size_kb` in the firmware project's `composer.json`.

| `size_kb` | Result |
| --------- | ------ |
| absent or `0` | No store. `store_available()` is `false`; every call is inert. |
| `1`–`16` | Raised to 16 KB, the minimum NVS needs for its bookkeeping. |
| `> 16` | Rounded up to the next 4 KB. |

A 32 KB partition holds a few hundred short entries. The store shares the flash with the application: if
the store plus the PHP source partition leave less than 1 MB for the application, the build fails and asks
you to shrink one of them or use a board with more flash.

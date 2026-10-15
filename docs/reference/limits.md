---
eyebrow: 'ext-store · Reference'
lede:    'What the store accepts and guarantees: keys of 1 to 15 bytes, string values with no NUL byte, a commit on every write, and flash that wears out if you write in a hot loop.'
see_also:
  - { href: '../usage/php-api.md', meta: 'PHP API' }
  - { href: '../recipes/save-settings.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/esp32-store-hal', meta: 'HAL', label: 'esp32-store-hal' }
  - { href: 'package:php-baremetal/ext-mem#docs/overview.md', meta: 'Extension', label: 'ext-mem (in-RAM store)' }
  - { href: 'https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_flash.html', meta: 'external', label: 'ESP-IDF NVS' }
prev: { label: 'PHP API', href: '../usage/php-api.md' }
next: { label: 'Count boots', href: '../recipes/boot-counter.md' }
---

# Limits and durability

The rules below are what you hit in practice. On ESP32 the store is NVS flash in the `phpstore` partition, so
its size limits come from NVS.

## Keys

| Rule | When broken |
| --- | --- |
| 1 to 15 bytes long (byte length, not characters) | Treated as a missing key: `store_set()` returns `false`, `store_get()` the default. |
| Unique in the store | `store_set()` on an existing key overwrites it. |
| No NUL byte | The key is cut at the NUL, so `"a\0b"` is stored as `"a"`. |

Short prefixes keep keys readable inside 15 bytes: `wifi.ssid`, `cal.x_off`, `boot.count`.

## Values

- **Strings only.** Store numbers with `(string)` and cast back with `(int)` or `(float)`. Store arrays with
  `json_encode()` and read them with `json_decode(..., true)`.
- **No NUL bytes.** A value ends at its first NUL byte and the rest is dropped without an error. Encode binary data
  with `base64_encode()` or `bin2hex()`.
- **Length.** NVS caps a string at 4000 bytes including its terminator. A longer value makes `store_set()` return
  `false`.
- **Bounded total.** The number of entries is bounded by the 24K partition; when it is full, `store_set()` returns
  `false`.

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

- `store_set()`, `store_delete()` and `store_clear()` commit before they return. There is no flush call and no
  write-back cache: a value is safe the moment the call returns `true`.
- NVS is built to survive a reset or a power cut during a write, leaving the old or the new value.
- `store_clear()` removes only the script's own keys (the `php` namespace of the partition), not other NVS data.

<!-- @callout variant="warning" title="Flash wears out" -->
Every write erases and rewrites flash, and flash survives a finite number of erase cycles. NVS spreads writes
around, but a `store_set()` on every loop tick or every HTTP request will eventually exhaust the partition. Write
only when a value changes, and keep fast-changing values in `mem_*` of ext-mem, copying them to the store now and
then.
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
At boot, if NVS reports that the partition has no free pages or holds a newer format, the HAL erases the whole
`phpstore` partition and formats it again before opening it, and every key is lost. Keep a way to rebuild what you
store (defaults, re-provisioning) and do not treat the store as the only copy of irreplaceable data.
<!-- @endcallout -->

## Where the calls run

| Aspect | Behaviour |
| --- | --- |
| Task | Every operation runs on the executor task (internal-RAM stack) because the PHP task, whose stack is in PSRAM, must not access flash. The PHP task blocks until it is done. |
| Queue timeout | A call that cannot be queued within 1000 ms fails and returns `false`. |
| `store_keys()` | The keys are gathered on the executor into a buffer that grows by doubling, then passed to PHP. An allocation or NVS error raises the exception. |
| Boot | The partition is initialised when the PHP module starts and the HAL logs `persistent store ready (partition 'phpstore')`. |
| Light sleep | `sys_light_sleep()` holds the executor, so store calls wait until it returns. |

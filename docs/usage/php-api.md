---
eyebrow: 'Package · Extension'
lede:    'Every store_* function with its signature and what it returns. None of them throws on a store problem: they return false, your default, or an empty array.'
see_also:
  - { href: '../reference/limits.md', meta: '4 min' }
  - { href: '../recipes/save-settings.md', meta: 'Recipe' }
  - { href: '../overview.md', meta: 'Overview' }
  - { href: 'package:php-baremetal/ext-mem#docs/overview.md', meta: 'Extension', label: 'ext-mem (in-RAM store)' }
prev: { label: 'Overview', href: '../overview.md' }
next: { label: 'Limits and durability', href: '../reference/limits.md' }
---

# PHP API

Seven global functions. Arguments are strictly typed: a wrong type or count raises the usual `TypeError`
or `ArgumentCountError`. Problems with the store itself never throw.

| Function | Returns | Meaning |
| -------- | ------- | ------- |
| `store_set(string $key, string $value)` | `bool` | Save a value; committed before it returns. |
| `store_get(string $key, ?string $default = null)` | `?string` | The value, or `$default` when absent. |
| `store_has(string $key)` | `bool` | Whether the key exists. |
| `store_delete(string $key)` | `bool` | Remove one key; committed before it returns. |
| `store_clear()` | `bool` | Remove every key. |
| `store_keys()` | `array` | The stored keys. |
| `store_available()` | `bool` | Whether the store is configured and opened. |

<!-- @callout variant="info" title="One rule for every key" -->
A key is valid when it is **1 to 15 bytes** long. The length is the string's byte length, so a multibyte
UTF-8 character counts as several. Every function treats an invalid key exactly like a missing one:
`store_set()` returns `false`, `store_get()` returns the default, `store_has()` returns `false`.
<!-- @endcallout -->

## store_set

<!-- @method name="store_set(string $key, string $value): bool" returns="bool" visibility="public" -->

Saves `$value` under `$key`, overwriting any previous value, and commits it to flash before returning: once
the call returns `true`, the value survives a power cut. Returns `false` when the store is not available,
the key is empty or longer than 15 bytes, the value is over the size limit (about 4 KB), or the partition
is full.

<!-- @params -->
<!-- @param name="$key" type="string" required -->
Entry name, 1–15 bytes.
<!-- @endparam -->
<!-- @param name="$value" type="string" required -->
Value to save. It ends at the first NUL byte, so encode binary data (`base64_encode()`, `bin2hex()`).
<!-- @endparam -->
<!-- @endparams -->

## store_get

<!-- @method name="store_get(string $key, ?string $default = null): ?string" returns="?string" visibility="public" -->

Returns the string stored under `$key`. Returns `$default` (`null` when not given) when the key is absent
or invalid, or the store is not available. An empty string saved with `store_set('k', '')` comes back as
`''`, not as the default.

<!-- @params -->
<!-- @param name="$key" type="string" required -->
Entry name to read.
<!-- @endparam -->
<!-- @param name="$default" type="?string" default="null" -->
Returned unchanged when there is no value.
<!-- @endparam -->
<!-- @endparams -->

## store_has

<!-- @method name="store_has(string $key): bool" returns="bool" visibility="public" -->

Returns `true` when `$key` exists. `false` for a missing or invalid key, or when the store is not
available.

<!-- @params -->
<!-- @param name="$key" type="string" required -->
Entry name to test.
<!-- @endparam -->
<!-- @endparams -->

## store_delete

<!-- @method name="store_delete(string $key): bool" returns="bool" visibility="public" -->

Removes `$key` and commits. Returns `false` when the key did not exist, is invalid, or the store is not
available, so a second delete of the same key returns `false`.

<!-- @params -->
<!-- @param name="$key" type="string" required -->
Entry name to remove.
<!-- @endparam -->
<!-- @endparams -->

## store_clear

<!-- @method name="store_clear(): bool" returns="bool" visibility="public" -->

Removes every key the script stored and commits. Only the store's own entries are touched; the system's
own NVS partition is left alone. Returns `false` when the store is not
available or the erase fails.

## store_keys

<!-- @method name="store_keys(): array" returns="array" visibility="public" -->

Returns a list (0-indexed array of strings) of the keys currently stored. The order is the backend's: do
not rely on it being sorted or in insertion order. Returns `[]` when the store is empty or not available.

## store_available

<!-- @method name="store_available(): bool" returns="bool" visibility="public" -->

Returns `true` when the store partition exists and was opened at boot. When it returns `false`, every
other function is inert. Check it once at startup to warn or fall back.

## Example

<!-- @code-block language="php" label="project-src/index.php — every function" -->
```php
<?php
if (!store_available()) {
    echo "no store: set extra.baremetal.store.size_kb\n";
    return;
}

store_set('mode', 'eco');                             // true
var_dump(store_get('mode'));                          // string(3) "eco"
var_dump(store_get('missing', 'none'));               // string(4) "none"
var_dump(store_has('mode'));                          // bool(true)
var_dump(store_set('a_key_that_is_too_long', 'x'));   // bool(false): 22 bytes > 15

print_r(store_keys());                                // Array ( [0] => mode )
store_delete('mode');                                 // true
store_delete('mode');                                 // false: already gone
store_clear();                                        // true
```
<!-- @endcode-block -->

When the same script must also run on a firmware built without the package, guard with
`function_exists('store_available')`.

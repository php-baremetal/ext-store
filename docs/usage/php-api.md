---
eyebrow: 'ext-store · PHP API'
lede:    'The seven store_* functions with their real signatures. Keys are 1 to 15 bytes, values are strings, and nothing throws except store_keys().'
see_also:
  - { href: '../reference/limits.md', meta: 'Reference' }
  - { href: '../recipes/boot-counter.md', meta: 'Recipe' }
  - { href: '../recipes/save-settings.md', meta: 'Recipe' }
prev: { label: 'Overview', href: '../overview.md' }
next: { label: 'Limits and durability', href: '../reference/limits.md' }
---

# PHP API

Seven global functions. They return `false` rather than throw: a bad key, a value the store refuses, a store that
did not open, an expired call. Check the result of `store_set()` while developing; a `false` usually means a key
longer than 15 bytes. Wrong argument types and counts raise the usual `TypeError` and `ArgumentCountError`. The
one exception is `store_keys()`.

| Function | Returns | Notes |
| --- | --- | --- |
| `store_set(string $key, string $value)` | `bool` | Overwrites an existing key. |
| `store_get(string $key, ?string $default = null)` | `?string` | `$default` when absent or invalid key. |
| `store_has(string $key)` | `bool` | |
| `store_delete(string $key)` | `bool` | `false` when the key did not exist. |
| `store_clear()` | `bool` | Removes every key. |
| `store_keys()` | `array` | List of key strings. |
| `store_available()` | `bool` | `true` once the store opened at boot. |

## store_set

<!-- @method name="store_set(string $key, string $value): bool" returns="bool" visibility="public" -->

Writes `$value` under `$key` and commits it to flash before returning.

<!-- @params -->
<!-- @param name="$key" type="string" required -->
1 to 15 bytes. Any other length returns `false`.
<!-- @endparam -->
<!-- @param name="$value" type="string" required -->
A string. Numbers, arrays and bytes need converting first; see [Limits and durability](../reference/limits.md).
<!-- @endparam -->
<!-- @endparams -->

Returns `true` once the value is in flash. Returns `false` for an invalid key, a store that is not available, a
value NVS refuses (too long, or no room left in the partition) and a flash call that timed out.

## store_get

<!-- @method name="store_get(string $key, ?string $default = null): ?string" returns="?string" visibility="public" -->

Returns the stored string, or `$default` when the key is absent, invalid, or the store is not available. With no
`$default` that is `null`. The default is returned as given, so it may be any value in practice, but the declared
type is `?string`.

## store_has

<!-- @method name="store_has(string $key): bool" returns="bool" visibility="public" -->

`true` when the key is stored. `false` for an invalid key or an unavailable store.

## store_delete

<!-- @method name="store_delete(string $key): bool" returns="bool" visibility="public" -->

Removes one key and commits. `false` when the key was not there, was invalid, or the store is not available.

## store_clear

<!-- @method name="store_clear(): bool" returns="bool" visibility="public" -->

Removes every key of the script's store and commits. Takes no arguments. `false` when the store is not available
or the erase failed.

## store_keys

<!-- @method name="store_keys(): array" returns="array" visibility="public" -->

Returns the keys as a list of strings, in no particular order, and `[]` when the store is empty or not available.
The list is collected on the executor and then handed to PHP.

Throws `\Exception` with the message `store_keys(): cannot list the keys (out of memory or NVS error)` when the
keys cannot all be listed.

## store_available

<!-- @method name="store_available(): bool" returns="bool" visibility="public" -->

`true` when the store opened at boot. Use it to guard a script that may run on a board whose partition is damaged
or unreadable.

## Example

<!-- @code-block language="php" label="project-src/index.php: the whole API" -->
```php
<?php
function setup(): void
{
    if (!store_available()) {
        return;
    }

    store_set('name', 'kitchen');
    store_set('threshold', (string) 42);

    var_dump(store_get('name'));              // string(7) "kitchen"
    var_dump(store_get('missing', 'n/a'));    // string(3) "n/a"
    var_dump(store_has('threshold'));         // bool(true)
    var_dump((int) store_get('threshold'));   // int(42)
    var_dump(store_keys());                   // ['name', 'threshold'], any order

    var_dump(store_delete('name'));           // bool(true)
    var_dump(store_delete('name'));           // bool(false): already gone
    var_dump(store_set(str_repeat('k', 16), 'x')); // bool(false): key longer than 15 bytes

    store_clear();
    var_dump(store_keys());                   // array(0) {}
}

function loop(int $tick): void
{
    delay(1000);
}
```
<!-- @endcode-block -->

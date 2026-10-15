---
eyebrow: 'Package · Extension'
lede:    'Count how many times the board has booted: read a number from the store, add one, write it back. The count keeps climbing across resets and power cuts.'
see_also:
  - { href: '../usage/php-api.md', meta: '5 min' }
  - { href: './save-settings.md', meta: 'Recipe' }
  - { href: '../overview.md', meta: 'Overview' }
prev: { label: 'Limits and durability', href: '../reference/limits.md' }
next: { label: 'Save settings', href: './save-settings.md' }
---

# Count boots

The smallest proof that a value lives in flash rather than RAM: a counter bumped once per boot.

## Goal

Print `boot #1`, `boot #2`, `boot #3`… one per reset, and write a one-time message on the very first boot.

## What you need

- A board with a firmware built from your project.
- `ext-store` and its provider, with a store partition:

<!-- @code-block language="bash" label="terminal" -->
```bash
composer require php-baremetal/ext-store php-baremetal/esp32-store-hal
```
<!-- @endcode-block -->

<!-- @code-block language="json" label="composer.json — extra" -->
```js
{
    "extra": {
        "baremetal": {
            "platform": "esp32",
            "store": { "size_kb": 32 }
        }
    }
}
```
<!-- @endcode-block -->

## The code

The script runs once per boot. Values are strings, so the count is cast on the way in and out. One write
per boot is nothing for the flash.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
if (!store_available()) {
    echo "persistence is off: set extra.baremetal.store.size_kb\n";
    return;
}

$boots = (int) store_get('boots', '0') + 1;
store_set('boots', (string) $boots);

if ($boots === 1) {
    store_set('first_msg', 'hello from boot #1');
}

printf("boot count : %d\n", $boots);
printf("first_msg  : %s\n", store_get('first_msg', '(none)'));
printf("keys       : %s\n", implode(', ', store_keys()));
```
<!-- @endcode-block -->

## What you'll see

Rebuild, flash, and reset the board a few times while watching the serial console:

<!-- @code-block language="text" label="serial output on the fourth boot" -->
```text
boot count : 4
first_msg  : hello from boot #1
keys       : boots, first_msg
```
<!-- @endcode-block -->

The order of `keys` may differ: `store_keys()` is unordered. If the console prints
`persistence is off`, the firmware was built without a store size.

<!-- @callout variant="tip" title="Start over" -->
`store_delete('boots')` resets the counter; `store_clear()` removes both keys. See
[Factory reset](./factory-reset.md).
<!-- @endcallout -->

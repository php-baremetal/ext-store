---
eyebrow: 'ext-store · Recipe'
lede:    'Count how many times the board has booted: read a number from the store, add one, write it back. The count keeps climbing across resets and power cuts.'
see_also:
  - { href: '../usage/php-api.md', meta: 'PHP API' }
  - { href: './save-settings.md', meta: 'Recipe' }
  - { href: '../overview.md', meta: 'Overview' }
prev: { label: 'Limits and durability', href: '../reference/limits.md' }
next: { label: 'Save settings', href: './save-settings.md' }
---

# Count boots

The smallest proof that a value lives in flash rather than RAM: a counter bumped once per boot.

## Goal

Print `boot count : 1`, `2`, `3`... one per reset, and write a one-time message on the very first boot.

## What you need

A project with the extension. The store HAL is added by conductor and its partition by the build:

<!-- @code-block language="bash" label="terminal" -->
```bash
composer require php-baremetal/ext-store
```
<!-- @endcode-block -->

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/boot-counter",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/ext-store": "^1.0"
  },
  "config": {
    "allow-plugins": { "php-baremetal/conductor": true },
    "platform": { "php": "8.5.11" }
  },
  "extra": {
    "baremetal": {
      "sdk": "esp32@^2.0",
      "board": "waveshare-esp32-s3-touch-lcd-1.85b",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

## The code

`setup()` runs once per boot. Values are strings, so the count is cast on the way in and out. One write per boot is
nothing for the flash.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
function setup(): void
{
    if (!store_available()) {
        echo "the store could not be opened\n";
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
}

function loop(int $tick): void
{
    delay(5000);
}
```
<!-- @endcode-block -->

## What you will see

Flash with `phpbm flash`, open `phpbm monitor`, and reset the board a few times:

<!-- @code-block language="text" label="serial output on the fourth boot" -->
```text
boot count : 4
first_msg  : hello from boot #1
keys       : boots, first_msg
```
<!-- @endcode-block -->

The order of `keys` may differ: `store_keys()` is unordered.

<!-- @callout variant="tip" title="Start over" -->
`store_delete('boots')` resets the counter; `store_clear()` removes both keys. See
[Factory reset](./factory-reset.md).
<!-- @endcallout -->

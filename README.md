# ext-store

A persistent key-value store for the PHP that runs on your board. `store_set()` writes a string to flash and
`store_get()` reads it back after a reset or a power cut. Keys are 1 to 15 bytes, values are strings, and every
write is committed before the call returns.

ext-store is platform-neutral: it requires the virtual `php-baremetal/store-hal`, and conductor adds the HAL for
your SDK platform (`php-baremetal/esp32-store-hal` on ESP32, which keeps the data in NVS flash in its own
`phpstore` partition of 24K). Nothing to configure: the partition comes with the HAL.

## Install

In a project created with `phpbm init`:

```bash
composer require php-baremetal/ext-store
```

## Example

```php
<?php
if (store_available()) {
    $boots = (int) store_get('boots', '0') + 1;   // values are strings
    store_set('boots', (string) $boots);
    echo "boot #$boots\n";
}
```

## Documentation

- [Overview](docs/overview.md): requirements, install, a first project.
- [PHP API](docs/usage/php-api.md): the seven `store_*` functions.
- [Limits and durability](docs/reference/limits.md): key and value rules, flash wear, when the store is wiped.
- Recipes: [count boots](docs/recipes/boot-counter.md), [save settings](docs/recipes/save-settings.md),
  [factory reset](docs/recipes/factory-reset.md).

Values that change often and need not survive a reset belong in [ext-mem](package:php-baremetal/ext-mem).

## License

MIT.

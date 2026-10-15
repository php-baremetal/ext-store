# ext-store

This package is part of [php-baremetal](https://github.com/php-baremetal).

## What it is for

`ext-store` adds `store_*`, a small key-value store kept in flash. A string saved with `store_set()` in one
boot comes back with `store_get()` in the next, after a reset or a power cut.

It covers:

- **storing and reading values**: `store_set()`, `store_get()`, `store_has()`, `store_delete()`,
  `store_clear()`, `store_keys()`, `store_available()`;
- **durability**: every write is committed to flash before the call returns;
- **builds without a store**: when the firmware has no store partition the functions still exist, do
  nothing, and `store_available()` returns `false`.

It needs a `store-hal` provider for your platform; on ESP32 that is `php-baremetal/esp32-store-hal`, which
keeps the data in the chip's NVS flash.

## When to use it

For values written rarely and read often that must survive a reset: a boot counter, the device settings,
calibration constants, provisioning flags.

Do not use it for values that change on every loop tick or request: flash wears out. Those belong in
`php-baremetal/ext-mem` (`mem_*`, in RAM, wiped on reset).

## Install

Require the provider for your platform explicitly next to the extension:

```bash
composer require php-baremetal/ext-store php-baremetal/esp32-store-hal
```

The store needs its own slice of flash and is **off by default**. Give it a size in the firmware project's
`composer.json`, then rebuild and flash:

```json
{
    "extra": {
        "baremetal": {
            "platform": "esp32",
            "store": { "size_kb": 32 }
        }
    }
}
```

## Example

```php
<?php
if (store_available()) {
    $boots = (int) store_get('boots', '0') + 1;
    store_set('boots', (string) $boots);
    echo "boot #$boots\n";
}
```

Keys are 1 to 15 bytes, values are strings of about 4 KB at most.

## Documentation

- [Overview](docs/overview.md): requirements, install, enabling the partition, the API at a glance.
- [PHP API](docs/usage/php-api.md): every `store_*` function, its signature and what it returns on failure.
- [Limits and durability](docs/reference/limits.md): key and value rules, commits, flash wear, partition size.
- [Count boots](docs/recipes/boot-counter.md): a counter that keeps climbing across resets.
- [Save settings](docs/recipes/save-settings.md): settings as one JSON value, loaded over defaults.
- [Factory reset](docs/recipes/factory-reset.md): erase the store when a button is held at power-on.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## AI usage

AI is used as a tool here, on top of human engineering. The architecture, the design decisions and the
hardware bring-up are done by hand. AI mostly helps with the documentation and with mechanical work that
can be verified. The full policy, including what is expected of contributions made with AI, is in
**[AI_USAGE.md](AI_USAGE.md)**.

Not for you? No worries at all. Nobody is forcing it on you, and a free, open-source project you didn't
spend a minute or a cent on owes you no explanation anyway. Take it or leave it, no hard feelings.

And for everyone who's on board: when contributing, please follow the guidelines in
[AI_USAGE.md](AI_USAGE.md). Thanks!

## License

MIT. See [LICENSE](LICENSE).

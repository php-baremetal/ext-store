---
eyebrow: 'Package · Extension'
lede:    'A key-value store in flash for PHP on the board: a string saved in one boot is there in the next, after a reset or a power cut. Neutral extension; conductor adds the store HAL for your platform.'
see_also:
  - { href: './usage/php-api.md', meta: 'PHP API' }
  - { href: './reference/limits.md', meta: 'Reference' }
  - { href: 'package:php-baremetal/esp32-store-hal', meta: 'HAL', label: 'esp32-store-hal' }
  - { href: 'package:php-baremetal/ext-mem', meta: 'Extension', label: 'ext-mem (in-RAM store)' }
prev: { label: 'No previous page', href: '#' }
next: { label: 'PHP API', href: './usage/php-api.md' }
---

# Persistent store

ext-store adds seven global functions, `store_set()`, `store_get()`, `store_has()`, `store_delete()`,
`store_clear()`, `store_keys()` and `store_available()`. They keep short strings under short keys in flash. A
write is committed before the call returns, so a value is safe the moment `store_set()` returns `true`.

Use it for values written rarely and read often: a boot counter, device settings, calibration constants,
provisioning flags. Flash wears out, so values that change on every loop tick or request belong in
[ext-mem](package:php-baremetal/ext-mem#docs/overview.md) instead.

## Requirements

| Requires | Why |
| --- | --- |
| `php-baremetal/store-hal` (virtual) | The platform side: where and how the bytes are kept. On ESP32 it is `php-baremetal/esp32-store-hal` (platform `esp32`, over `nvs_flash`), which also brings `php-baremetal/esp32-executor`. |

You never require the HAL yourself. Conductor reads the platform from `extra.baremetal.sdk` (`esp32@^1.0` gives
`esp32`) and adds `php-baremetal/esp32-store-hal` while Composer resolves. The extension's PHP module is `store`.

The ESP32 HAL declares its own flash partition: an NVS partition named `phpstore`, 24K, added to the board's
partition table by the build. There is no size to set in the project.

## Install

<!-- @steps -->
- **Install conductor once per machine**

  `composer global require php-baremetal/conductor` keeps the plugin active during the first solve of a new
  project.

- **Create the project**

  `phpbm init my-settings` writes `composer.json` with conductor (`^1.0`, allowed as a plugin), the package
  registry, and the SDK, board, runtime and storage you pick.

- **Require the extension**

  `composer require php-baremetal/ext-store`

- **Build and flash**

  `phpbm flash` builds the firmware and flashes it; `phpbm monitor` opens the serial console.
<!-- @endsteps -->

The resulting `composer.json`, for a Waveshare ESP32-S3-Touch-LCD-1.85B:

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/my-settings",
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
      "sdk": "esp32@^1.0",
      "board": "waveshare-esp32-s3-touch-lcd-1.85b",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

## A first script

`project-src/index.php`: count boots. Run it, reset the board a few times, and the number keeps climbing.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
function setup(): void
{
    if (!store_available()) {
        echo "the store could not be opened\n";
        return;
    }

    $boots = (int) store_get('boots', '0') + 1;   // values are strings: cast on the way in and out
    store_set('boots', (string) $boots);
    printf("boot #%d, keys: %s\n", $boots, implode(', ', store_keys()));
}

function loop(int $tick): void
{
    delay(5000);
}
```
<!-- @endcode-block -->

## PHP API at a glance

| Function | Returns | Meaning |
| --- | --- | --- |
| `store_set(string $key, string $value)` | `bool` | Write and commit. `false` when the key is invalid, the value is refused or the store is not ready. |
| `store_get(string $key, ?string $default = null)` | `?string` | The value, or `$default` when absent. |
| `store_has(string $key)` | `bool` | Is the key present. |
| `store_delete(string $key)` | `bool` | Remove one key. `false` when it was absent. |
| `store_clear()` | `bool` | Remove every key of the script. |
| `store_keys()` | `array` | The stored keys, in no particular order. Throws on a flash or memory error. |
| `store_available()` | `bool` | Did the store open at boot. |

Details and every failure case: [PHP API](./usage/php-api.md) and [Limits and durability](./reference/limits.md).

## Notes

- **Every flash call runs on the executor task.** The PHP task keeps its stack in PSRAM and must never touch
  flash, so each NVS operation is handed to the executor of `esp32-executor` and the PHP task waits for it. A
  call that cannot even be queued within 1 second fails and returns `false`.
- **One boot, one open.** The store is opened when the PHP module starts. `store_available()` is `false` when
  that failed, and every other function then returns `false` (or the default, or an empty list).
- **Power-save.** The store needs no wake source. `sys_light_sleep()` of ext-sys occupies the executor for its
  whole duration, so a store call made meanwhile waits. `sys_restart()` and `sys_deep_sleep()` stop the program
  cleanly; values already written are not affected.

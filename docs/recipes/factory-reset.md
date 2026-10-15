---
eyebrow: 'Package · Extension'
lede:    'Erase every stored key when a button is held at power-on, then reboot into a clean device. store_clear() removes only the script''s own keys.'
see_also:
  - { href: './save-settings.md', meta: 'Recipe' }
  - { href: '../usage/php-api.md', meta: '5 min' }
  - { href: 'package:php-baremetal/ext-gpio#docs/overview.md', meta: 'Extension', label: 'ext-gpio' }
  - { href: 'package:php-baremetal/ext-sys#docs/overview.md', meta: 'Extension', label: 'ext-sys' }
prev: { label: 'Save settings', href: './save-settings.md' }
next: { label: 'No next page', href: '#' }
---

# Factory reset

A device that saves settings needs a way back to a clean state. Here, holding a button while the board
powers up erases the store and reboots.

## Goal

At boot, if the button is held for two seconds, call `store_clear()` and restart. Otherwise start normally.

## What you need

- A push button wired between **GPIO4** and **GND**. No resistor: the pin idles high on the internal
  pull-up and reads `0` while pressed.
- `ext-store` for the store, `ext-gpio` to read the button, `ext-sys` for the delay and the restart, each
  with its ESP32 provider:

<!-- @code-block language="bash" label="terminal" -->
```bash
composer require php-baremetal/ext-store php-baremetal/esp32-store-hal \
                 php-baremetal/ext-gpio php-baremetal/esp32-gpio-hal \
                 php-baremetal/ext-sys php-baremetal/esp32-sys-hal
```
<!-- @endcode-block -->

## The code

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
const RESET_BUTTON = 4;

gpio_mode(RESET_BUTTON, GPIO_INPUT);

function held_for(int $pin, int $ms): bool
{
    for ($t = 0; $t < $ms; $t += 50) {
        if (gpio_read($pin) !== 0) {
            return false;                  // released early
        }
        sys_delay(50);
    }
    return true;
}

if (gpio_read(RESET_BUTTON) === 0 && held_for(RESET_BUTTON, 2000)) {
    echo "factory reset: erasing ", count(store_keys()), " keys\n";
    if (store_clear()) {
        sys_restart();
    }
    echo "factory reset failed: store not available\n";
}

// normal start-up from here
echo "booted with ", count(store_keys()), " stored keys\n";
```
<!-- @endcode-block -->

## What you'll see

Hold the button, press reset, and keep holding for two seconds:

<!-- @code-block language="text" label="serial output" -->
```text
factory reset: erasing 3 keys
booted with 0 stored keys
```
<!-- @endcode-block -->

The second line comes from the boot after the restart (release the button first, or it erases again,
which is harmless).

<!-- @callout variant="note" title="What store_clear() erases" -->
Only the keys written with `store_set()`. The system's own flash data and the PHP source are untouched.
A settings loader with defaults, as in [Save settings](./save-settings.md), brings the device back with
known values.
<!-- @endcallout -->

---
eyebrow: 'ext-store · Recipe'
lede:    'Erase every stored key when a button is held at power-on, then restart into a clean device.'
see_also:
  - { href: './save-settings.md', meta: 'Recipe' }
  - { href: '../usage/php-api.md', meta: 'PHP API' }
  - { href: 'package:php-baremetal/ext-gpio', meta: 'Extension', label: 'ext-gpio' }
  - { href: 'package:php-baremetal/ext-sys', meta: 'Extension', label: 'ext-sys' }
prev: { label: 'Save settings', href: './save-settings.md' }
next: { label: 'No next page', href: '#' }
---

# Factory reset

A device that saves settings needs a way back to a clean state. Here, holding a button while the board powers up
erases the store and restarts.

## Goal

At boot, if the button is held for two seconds, call `store_clear()` and restart. Otherwise start normally.

## What you need

- A push button between a GPIO and GND, with a pull-up resistor to 3V3 (10 kOhm) so the pin reads `1` when the
  button is released and `0` while it is pressed. The script uses GPIO 4; use a free pin of your board.
- `ext-store` for the store, `ext-gpio` to read the button and `ext-sys` for the delay and the restart. Conductor
  adds the HAL of each.

<!-- @code-block language="bash" label="terminal" -->
```bash
composer require php-baremetal/ext-store php-baremetal/ext-gpio php-baremetal/ext-sys
```
<!-- @endcode-block -->

## The code

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
const RESET_BUTTON = 4;

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

function setup(): void
{
    gpio_mode(RESET_BUTTON, GPIO_INPUT);

    if (gpio_read(RESET_BUTTON) === 0 && held_for(RESET_BUTTON, 2000)) {
        echo "factory reset: erasing ", count(store_keys()), " keys\n";
        if (store_clear()) {
            sys_restart();                 // stops the program and restarts the board
        }
        echo "factory reset failed: the store is not available\n";
    }

    // normal start-up from here
    echo "booted with ", count(store_keys()), " stored keys\n";
}

function loop(int $tick): void
{
    delay(1000);
}
```
<!-- @endcode-block -->

## What you will see

Hold the button, press reset, and keep holding for two seconds:

<!-- @code-block language="text" label="serial output" -->
```text
factory reset: erasing 3 keys
booted with 0 stored keys
```
<!-- @endcode-block -->

The second line comes from the boot after the restart. Release the button first, or it erases again, which is
harmless.

<!-- @callout variant="note" title="What store_clear() erases" -->
Only the keys written with `store_set()`: the script's namespace of the `phpstore` partition. The PHP sources are
untouched. A settings loader with defaults, as in [Save settings](./save-settings.md), brings the device back with
known values.
<!-- @endcallout -->

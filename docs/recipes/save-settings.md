---
eyebrow: 'ext-store · Recipe'
lede:    'Keep the settings of a device as one JSON value in the store: defaults for anything missing or corrupt, and no flash write when nothing changed.'
see_also:
  - { href: '../reference/limits.md', meta: 'Reference' }
  - { href: './boot-counter.md', meta: 'Recipe' }
  - { href: './factory-reset.md', meta: 'Recipe' }
prev: { label: 'Count boots', href: './boot-counter.md' }
next: { label: 'Factory reset', href: './factory-reset.md' }
---

# Save settings

Most devices have a handful of settings: a name, a mode, a threshold. Storing them as one JSON value keeps the keys
few and short, and comparing before writing keeps the flash healthy.

## Goal

Two functions, `settings_load()` and `settings_save()`, that the rest of the script uses. Missing or corrupt data
falls back to defaults; a save with no changes costs no flash write.

## What you need

A project with `php-baremetal/ext-store`, as in [Count boots](./boot-counter.md).

## The code

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
const SETTINGS_KEY = 'settings';          // 8 bytes: well inside the 15-byte limit

const SETTINGS_DEFAULTS = [
    'name'      => 'sensor-1',
    'mode'      => 'eco',
    'threshold' => 42,
];

function settings_load(): array
{
    $raw  = store_get(SETTINGS_KEY, '{}');
    $data = json_decode($raw, true);
    if (!is_array($data)) {
        $data = [];                        // corrupt or foreign value: use defaults
    }
    // keep only known keys, fill in any that are missing
    return array_intersect_key($data, SETTINGS_DEFAULTS) + SETTINGS_DEFAULTS;
}

function settings_save(array $settings): bool
{
    $json = json_encode($settings);
    if ($json === store_get(SETTINGS_KEY)) {
        return true;                       // unchanged: no flash write
    }
    if (strlen($json) >= 4000) {
        return false;                      // NVS caps a string at 4000 bytes with its terminator
    }
    return store_set(SETTINGS_KEY, $json);
}

function setup(): void
{
    $settings = settings_load();
    printf("%s running in %s mode\n", $settings['name'], $settings['mode']);

    // later, for example after a button press or a web form
    $settings['mode'] = 'turbo';
    if (!settings_save($settings)) {
        echo "could not save settings\n";
    }
}

function loop(int $tick): void
{
    delay(5000);
}
```
<!-- @endcode-block -->

## What you will see

The first boot prints the defaults. After `settings_save()` has run once, every following boot starts in the saved
mode:

<!-- @code-block language="text" label="serial output" -->
```text
sensor-1 running in turbo mode
```
<!-- @endcode-block -->

<!-- @callout variant="warning" title="Settings, not live data" -->
Call `settings_save()` when the user changes something, not on every loop tick or request. For a value that changes
constantly (a hit counter, the latest reading) keep it in `mem_*` from ext-mem and copy it to the store only now
and then.
<!-- @endcallout -->

Defaults also cover the case where the store is wiped at boot (see [Limits and durability](../reference/limits.md)):
the device comes back with known settings instead of failing.

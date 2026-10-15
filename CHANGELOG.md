# Changelog

## [v1.0.0]

First release.

### Added
- **`store_*` functions.** `store_set()`, `store_get()`, `store_has()`, `store_delete()`, `store_clear()`,
  `store_keys()` and `store_available()`, working on string values under keys of 1 to 15 bytes.
- **Survives resets and power cuts.** Every write is committed before the call returns.
- **Off by default.** The store gets its own flash partition only when the firmware project sets a size;
  without one the functions stay callable, do nothing, and `store_available()` returns `false`.

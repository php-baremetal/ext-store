# Changelog

## 1.1.0

- Added: ide/ext-store.stub, the PHP declarations of the package for IDEs (conductor copies it into vendor/php-baremetal/ide-stubs/).

## 1.0.2

- Fixed: a value or a key with a NUL byte was stored cut at the first NUL, and `store_set()` returned `true`.
  `store_set()` now returns `false` for them and leaves the stored value as it was; the other functions treat
  such a key as invalid. The NVS format is unchanged.

## 1.0.1

- Changed: the author's contact email.
- Docs: projects use `"sdk": "esp32@^2.0"`.

## 1.0.0

Initial release.

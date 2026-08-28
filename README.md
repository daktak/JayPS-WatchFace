# KayPS-WatchFace

Pebble Watch Face for the [Pebble Bike Android app](https://github.com/daktak/JayPS-AndroidApp).

KayPS is a GPS cycle computer for your Pebble smart watch, this simple app uses your phone's GPS to send speed, distance and altitude data to your Pebble.

Licensed under [MIT License](http://opensource.org/licenses/MIT)

![](screenshots/emery.png?raw=true)

## Building

Prerequisites: Python 3, `make`, and `pebble-tool` (`pip install pebble-tool`) with the SDK installed (`pebble sdk install latest`).

From the repository root, run:

```
make build
```

This invokes, inside `pebblebike/`:

1. `./dict2bin.sh` — converts the `locale_*.json` files into `resources/locale_*.bin` resources.
2. `pebble build` — compiles the watch face.

The resulting bundle is written to `pebblebike/build/pebblebike.pbw`.

To clean build output (and the generated `resources/locale_*.bin` files), run:

```
make clean
```

## Regenerating the locale dictionary

The English string table (`pebblebike/locale_english.json`) is generated from the `_("...")` markers in the C sources. This is a developer maintenance step, **separate from building** — the build consumes the committed `locale_*.json` files as-is.

After adding new translatable strings in `src/c`, run:

```
make gen-dict
```

(or its `make dict` alias). This regenerates `pebblebike/locale_english.json`; commit the result and, if needed, add the corresponding translations to the other `locale_*.json` files. The other locales are translator-maintained and are never overwritten by this step.

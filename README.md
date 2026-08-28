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

1. `python3 gen_dict.py src/c locale_english.json` — regenerates the English locale dictionary by scanning the C sources in `src/c`.
2. `./dict2bin.sh` — converts the `locale_*.json` files into `resources/locale_*.bin` resources.
3. `pebble build` — compiles the watch face.

The resulting bundle is written to `pebblebike/build/pebblebike.pbw`.

To clean build output (and the generated `resources/locale_*.bin` files), run:

```
make clean
```

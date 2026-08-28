PEBBLE_DIR := pebblebike
PYTHON      := python3

.PHONY: build pebble-build clean pebble-clean gen-dict dict

gen-dict dict:
	cd $(PEBBLE_DIR) && $(PYTHON) gen_dict.py src/c locale_english.json

build pebble-build:
	cd $(PEBBLE_DIR) && ./dict2bin.sh
	cd $(PEBBLE_DIR) && pebble build

clean pebble-clean:
	cd $(PEBBLE_DIR) && pebble clean
	rm -f $(PEBBLE_DIR)/resources/locale_*.bin

.DEFAULT_GOAL := build

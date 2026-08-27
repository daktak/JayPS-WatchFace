#!/bin/bash

python3 dict2bin.py  locale_english.json
python3 dict2bin.py  locale_french.json
python3 dict2bin.py  locale_spanish.json
python3 dict2bin.py  locale_german.json
python3 dict2bin.py  locale_italian.json
python3 dict2bin.py  locale_japanese.json

mv *.bin resources/

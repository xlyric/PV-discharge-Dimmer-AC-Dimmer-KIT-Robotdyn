#!/bin/sh


gzip -9 index.html -c > index.html.gz
gzip -9 css/style.css -c > css/style.css.gz
gzip -9 js/app.js -c > js/app.js.gz
gzip -9 js/i18n.js -c > js/i18n.js.gz

SRC_DIR="$(dirname "$(realpath "$0")")"
DST_DIR=$(realpath "$SRC_DIR/../data")
grep "FS_RELEASE" "../src/config/config.h" | tr -d '"' | cut -d' ' -f 3 > "$DST_DIR/version"
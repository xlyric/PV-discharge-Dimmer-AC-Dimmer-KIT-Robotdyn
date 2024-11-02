#!/bin/bash

KEEP_EXTERNAL_RESOURCES=0
[ "$1" == "--keep" -o "$1" == "-k" ] && KEEP_EXTERNAL_RESOURCES=1

SRC_DIR="$(dirname "$(realpath "$0")")"
DST_DIR=$(realpath "$SRC_DIR/../data")

JS=(
    "https://cdn.jsdelivr.net/npm/jquery@3.5.1/dist/jquery.min.js"
    "https://cdn.jsdelivr.net/npm/bootstrap@4.6.2/dist/js/bootstrap.bundle.min.js"
    "https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/vendor/jquery-easing/jquery.easing.min.js"
    "https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/js/sb-admin-2.min.js"
    "https://www.gstatic.com/charts/loader.js"
)
CSS=(
    "https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/vendor/fontawesome-free/css/all.min.css"
    "https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/css/sb-admin-2.min.css"
    "https://cdn.jsdelivr.net/npm/bootstrap@4.6.2/dist/css/bootstrap.min.css"
)
FONTS=(
    "https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/vendor/fontawesome-free/webfonts/fa-solid-900.woff2"
)

if [[ "$KEEP_EXTERNAL_RESOURCES" -eq 0 ]]; then
    echo -n "Remove old data files... "
    rm -fr "$DST_DIR"
    mkdir "$DST_DIR"
    echo done.

    echo "Generate all.min.js file:"
    rm -fr "$DST_DIR/js"
    mkdir "$DST_DIR/js"
    for url in "${JS[@]}"; do
        filename="${url##*/}"
        echo -n "- Download $filename... "
        curl -s "$url" >> "$DST_DIR/js/all.min.js"
        echo >> "$DST_DIR/js/all.min.js"
        echo "done."
    done
    echo "done."

    echo "Generate all.min.css file:"
    rm -fr "$DST_DIR/css"
    mkdir "$DST_DIR/css"
    for url in "${CSS[@]}"; do
        filename="${url##*/}"
        echo -n "- Download $filename... "
        curl -s "$url" >> "$DST_DIR/css/all.min.css"
        echo >> "$DST_DIR/css/all.min.css"
        echo "done."
    done

    echo -n "- Fix webfonts URL... "
    sed -i 's#\.\./webfonts\/##g' "$DST_DIR/css/all.min.css"
    echo "done."

    echo -n "- Replace Nunito font by web safe Tahoma one... "
    sed -i 's/Nunito/Tahoma/gi' "$DST_DIR/css/all.min.css"
    echo "done."

    echo -n "- Replace color #212529 by #858796... "
    sed -i 's/#212529/#858796/gi' "$DST_DIR/css/all.min.css"
    echo "done."

    echo "Download webfonts:"
    for url in "${FONTS[@]}"; do
        filename="${url##*/}"
        echo -n "- Download $filename... "
        curl -s -o "$DST_DIR/css/$filename" "$url"
        rm -f "$DST_DIR/$filename"
        echo "done."
    done
    echo "done."

    echo "Compress all.min.js & all.min.css files... "
    gzip "$DST_DIR/js/all.min.js" "$DST_DIR/css/all.min.css"
    echo "done."
fi

echo "Install JSON files:"
for file in $SRC_DIR/json/*.json; do
    echo -n "- $(basename "$file")... "
    cp "$file" "$DST_DIR/"
    echo "done."
done
echo "done."

echo "Install lang.json file:"
lang=$(grep -ri 'build_flags = -D LANG_' "$SRC_DIR/../platformio.ini" | cut -d'_' -f3 | cut -d' ' -f1 )
echo " - lang = $lang"
cp "$SRC_DIR/lang/lang_${lang,,}.json" "$DST_DIR/lang.json"
echo "done."

echo -n "Install favicon.ico... "
cp "$SRC_DIR/favicon.ico" "$DST_DIR/favicon.ico"
echo "done."

echo -n "Generate version file... "
grep "#define FS_RELEASE " "$SRC_DIR/../src/config/config.h" | cut -d'"' -f 2 > "$DST_DIR/version"
echo "done."

# Generate HTML files using generate-html-files.py script
"$SRC_DIR/generate-html-files.py"

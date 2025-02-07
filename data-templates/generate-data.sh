#!/bin/bash 

set -e

SRC_DIR="$(dirname "$(realpath "$0")")"
DST_DIR=$(realpath "$SRC_DIR/../data")

declare -A JS=(
    [01-jquery.min.js]="https://cdn.jsdelivr.net/npm/jquery@3.5.1/dist/jquery.min.js"
    [02-bootstrap.bundle.min.js]="https://cdn.jsdelivr.net/npm/bootstrap@4.6.2/dist/js/bootstrap.bundle.min.js"
    [03-jquery.easing.min.js]="https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/vendor/jquery-easing/jquery.easing.min.js"
    [04-sb-admin-2.min.js]="https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/js/sb-admin-2.min.js"
    [05-google-charts.min.js]="https://www.gstatic.com/charts/loader.js"
)
declare -A CSS=(
    [02-sb-admin-2.min.css]="https://raw.githubusercontent.com/StartBootstrap/startbootstrap-sb-admin-2/refs/heads/master/css/sb-admin-2.min.css"
    [03-bootstrap-4.6.2.min.css]="https://cdn.jsdelivr.net/npm/bootstrap@4.6.2/dist/css/bootstrap.min.css"
)
declare -A WEBFONTS=(

)

echo -n "Remove old data files... "
rm -fr "$DST_DIR"
mkdir "$DST_DIR"
echo done.

CACHE_DIR="$SRC_DIR/cache"
if [[ ! -d "$CACHE_DIR" ]]; then
    echo -n "Create cache directory... "
    mkdir "$CACHE_DIR"
    echo "done."
fi

echo "Generate all.min.js file:"
rm -fr "$DST_DIR/js"
mkdir "$DST_DIR/js"
# Bash associative array do not keep keys order, sort JS files by filename
mapfile -t JS_FILES < <( for filename in "${!JS[@]}"; do echo "$filename"; done | sort )
for filename in "${JS_FILES[@]}"; do
    if [[ ! -e "$CACHE_DIR/$filename" ]]; then
        echo -n "- Download $filename... "
        if ! curl --ssl-no-revoke -o "$CACHE_DIR/$filename" -s "${JS[$filename]}"; then
            echo "Failed to download $filename from '${JS[$filename]}'"
            exit 1
        fi
        echo "done."
    else
        echo "- $filename already present in cache, keep it"
    fi
    echo -n "- Include $filename... "
    cat "$CACHE_DIR/$filename" >> "$DST_DIR/js/all.min.js"
    echo >> "$DST_DIR/js/all.min.js"
    echo "done."
done
echo "done."

echo "Generate all.min.css file:"
rm -fr "$DST_DIR/css"
mkdir "$DST_DIR/css"
# Bash associative array do not keep keys order, sort CSS files by filenames
mapfile -t CSS_FILES < <( for filename in "${!CSS[@]}"; do echo "$filename"; done | sort )
for filename in "${CSS_FILES[@]}"; do
    if [[ ! -e "$CACHE_DIR/$filename" ]]; then
        echo -n "- Download $filename... "
        if ! curl --ssl-no-revoke -o "$CACHE_DIR/$filename" -s "${CSS[$filename]}"; then
            echo "Failed to download $filename from '${CSS[$filename]}'"
            exit 1
        fi
        echo "done."
    else
        echo "- $filename already present in cache, keep it"
    fi
    echo -n "- Include $filename... "
    cat "$CACHE_DIR/$filename" >> "$DST_DIR/css/all.min.css"
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

echo "Include custom CSS files:"
for file in $SRC_DIR/css/*.css; do
    echo -n "- $(basename "$file")... "
    cat "$file" >> "$DST_DIR/css/all.min.css"
    echo >> "$DST_DIR/css/all.min.css"
    echo "done."
done
echo "done."

echo "Download webfonts:"
for filename in "${!WEBFONTS[@]}"; do
    if [[ ! -e "$CACHE_DIR/$filename" ]]; then
        echo -n "- Download $filename... "
        if ! curl --ssl-no-revoke -o "$CACHE_DIR/$filename" -s "${WEBFONTS[$filename]}"; then
            echo "Failed to download $filename from '${WEBFONTS[$filename]}'"
            exit 1
        fi
        echo "done."
    else
        echo "- $filename already present in cache, keep it"
    fi
    echo -n "- Install $filename... "
    cp "$CACHE_DIR/$filename" "$DST_DIR/css/$filename"
    echo "done."
done
echo "done."

echo "Compress all.min.js & all.min.css files... "
gzip -n -9 "$DST_DIR/js/all.min.js" "$DST_DIR/css/all.min.css"
echo "done."

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

echo "Clean old cached files... "
for file in $CACHE_DIR/*; do
    filename=$( basename "$file" )
    if [[ -z "${JS[$filename]:-}" ]] && \
        [[ -z "${CSS[$filename]:-}" ]] && \
        [[ -z "${WEBFONTS[$filename]:-}" ]]; then
        echo "- drop $filename... "
        rm -f "$CACHE_DIR/$filename" || echo -n "FAILURE"
        echo "done."
    else
        echo "- keep $filename"
    fi
done
echo " clean done."

# Generate HTML files using generate-html-files.py script
python -m pip install mako
python "$SRC_DIR/generate-html-files.py"

# compression du fichier logs.html
gzip -n -9 "$DST_DIR/log.html"

[[ -n "$GITHUB_RUN_ID" ]] || read -p "Press [Enter] to exit."
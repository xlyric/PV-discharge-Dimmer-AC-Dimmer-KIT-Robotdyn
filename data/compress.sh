#!/bin/bash


# recupération des fichiers pour concatenation
echo "construction des pages"
cd ../data2/template
cat header.txt menu.txt index.txt footer.txt > ../../data/index.html
cat header.txt menu.txt index-AP.txt footer.txt > ../../data/index-AP.html
cat header.txt menu-config.txt config.txt footer.txt > ../../data/config.html

sed 's/<li class="nav-item active" id="menu_mqtt" >/<li class="nav-item active" id="menu_mqtt" hidden >/' menu-config.txt > menu-config-AP.txt
sed 's/<div class="card position-relative" id="menu_mqtt">/<div class="card position-relative" id="menu_mqtt" hidden >/' config.txt > config-AP.txt
cat header.txt menu-config-AP.txt config-AP.txt footer.txt > ../../data/config-AP.html

langue=$(grep -ri 'build_flags = -D LANG_' ../../platformio.ini | cut -d'_' -f3 |cut -d' ' -f1) 
echo "langue = $langue"


if [ "$langue" = "UA" ]; then
    cp lang_ua.json ../../data/lang.json
elif [ "$langue" = "EN" ]; then
    cp lang_en.json ../../data/lang.json
elif [ "$langue" = "FR" ]; then
    cp lang_fr.json ../../data/lang.json
fi

echo "fin de la construction des pages"

# compression des fichiers
gzip -9 ../mqtt.html -c > ../../data/mqtt.html.gz
gzip -9 ../log.html  -c > ../../data/log.html.gz

gzip -9 ../all.css  -c > ../../data/css/all.css.gz

read -p "Appuyer sur une touche pour continuer"
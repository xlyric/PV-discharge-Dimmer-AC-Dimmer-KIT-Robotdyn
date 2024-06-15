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

echo "fin de la construction des pages"

# compression des fichiers
gzip -9 ../mqtt.html -c > ../../data/mqtt.html.gz
gzip -9 ../log.html  -c > ../../data/log.html.gz

gzip -9 ../all.css  -c > ../../data/css/all.css.gz

read -p "Appuyer sur une touche pour continuer"
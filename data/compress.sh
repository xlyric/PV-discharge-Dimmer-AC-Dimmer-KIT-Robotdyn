#!/bin/bash


# recupération des fichiers pour concatenation
echo "construction des pages"
cd ../data2/template
cat header.txt menu.txt index.txt footer.txt > ../index.html

echo "fin de la construction des pages"

# compression des fichiers
gzip -9 ../data2/mqtt.html -c > mqtt.html.gz
gzip -9 ../data2/log.html  -c > log.html.gz

gzip -9 ../data2/all.css  -c > css/all.css.gz

#!/bin/bash
gzip -9 ../data2/mqtt.html -c > mqtt.html.gz
gzip -9 ../data2/log.html  -c > log.html.gz

gzip -9 ../data2/all.css  -c > css/all.css.gz

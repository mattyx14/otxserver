#!/usr/bin/env bash

wget https://github.com/slawkens/myaac/archive/master.zip -O myaac.zip
unzip -o myaac.zip -d .

mv myaac-master/* ./web
rm myaac.zip myaac-master -rf

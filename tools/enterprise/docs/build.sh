#!/bin/bash

path=`dirname $0`
pushd $path > /dev/null

pandoc -f markdown -t html -o README.html README.mdown --section-divs --css style.css --toc

popd

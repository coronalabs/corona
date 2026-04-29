#!/bin/bash

#$1 filename
#usage:  find . -type fl -exec ~/ansca/manifest-builder.sh {} \;

hash=`md5 -q "$1"`
size=`stat -f %z "$1"`
name=`ls "$1"`
echo $hash  $size "$name"

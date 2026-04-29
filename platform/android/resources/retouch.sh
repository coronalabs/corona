#!/bin/sh

# Touch a file $2 in dir $1

DST=`basename "$2"`

touch "$1/$DST"


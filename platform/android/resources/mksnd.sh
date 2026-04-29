#!/bin/sh

# Touch a file $2 in dir $1, removing spaces and changing name to lower case
# and removing dashes
# -> Invalid file name: must contain only [a-z0-9_.]

DST=`basename "$2" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9_.]/_/g'`

touch "$1/$DST"

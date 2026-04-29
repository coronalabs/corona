#!/bin/bash

# returns $2 relative to $1
source="$1"
target="$2"

pushd "$source" > /dev/null
dir=`pwd`
source="$dir"
popd > /dev/null

pushd "$target" > /dev/null
dir=`pwd`
target="$dir"
popd > /dev/null

if [[ "$source" == "$target" ]]
then
	echo "."
	exit
fi
IFS="/"

current=($source)
absolute=($target)

abssize=${#absolute[@]}
cursize=${#current[@]}

while [[ ${absolute[level]} == ${current[level]} ]]
do
	(( level++ ))
	if (( level > abssize || level > cursize ))
	then
		break
	fi
done

for ((i = level; i < cursize; i++))
do
	if ((i > level))
	then
		newpath=$newpath"/"
	fi
	newpath=$newpath".."
done

for ((i = level; i < abssize; i++))
do
	if [[ -n $newpath ]]
	then
		newpath=$newpath"/"
	fi
	newpath=$newpath${absolute[i]}
done

echo "$newpath"

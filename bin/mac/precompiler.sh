#!/bin/bash

# Example of how to precompile lua files that are inside a module
# ./precompiler.sh -load engine.lua ~/tmp7 /Volumes/rtt/platform/mac/modules/remdebug/remdebug
#

# [-load] controls whether the file generates a luaL_load (default is luaopen)
# $1  path to source path (normally absolute, but if moduleroot is provided
#       then path should be relative to module root
# $2  dst dir for output
# $3  (optional) absolute path to module root (last path component should 
#       be name of module)

precompiler="precompiler.lua"

case $1
in
	-load)
		precompiler="precompiler.constant.lua"
		shift 1
	;;
esac

if [ -z "$1" ] || [ -z "$2" ]
then
        echo "USAGE: $0 [-load] source.lua dstdir [moduleroot]"
        exit 0
fi

srcpath=$1

if [[ $2 ]] && [[ -d $2 ]]
then
	dstdir=$2
else
	echo "Error: destination directory $2 does not exist" >&2
	exit 1
fi

if [[ $3 ]] && [[ -d $3 ]]
then
	moduleroot=$3
fi


if [[ -d $TMPDIR ]]
then
	path=`dirname $0`
	filebase=`basename $1 .lua`
	srcdir=`dirname $1`
	cd $path

	if [[ -z $moduleroot ]]
	then
		./lua ../rcc.lua -c . -ORelease -o $TMPDIR/$filebase.lu $1

		./lua -epackage.path=[[loop/?.lua]] loop/$precompiler -d $dstdir -o $filebase -l $TMPDIR/?.lu -n -bytecodes $filebase
		#./lua -epackage.path=[[loop/?.lua]] loop/precompiler.constant.lua -d $dstdir -o $filebase -l $TMPDIR/?.lu -n -bytecodes $filebase

		rm $TMPDIR/$filebase.lu
	else
		relative_srcdir=$srcdir
		srcdir=$moduleroot/$srcdir
		moduleparent=`dirname $moduleroot`
		modulename=`basename $moduleroot`

		# treat srcpath as relative to moduleroot, so replace dir separators with '.'
		# and use this as the basename for the output file
		dstfilebase=$modulename.`echo "$filebase" | sed "s/\//\./g"`

		./lua ../rcc.lua -c . -ORelease -o $TMPDIR/$filebase.lu $moduleroot/$1

		./lua -epackage.path=[[loop/?.lua]] loop/$precompiler -d $dstdir -o $dstfilebase -l $TMPDIR/$filebase.lu -n -bytecodes $modulename.$filebase

		rm $TMPDIR/$filebase.lu
	fi

	cd -
fi


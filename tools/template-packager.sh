#!/bin/bash 

function package {
  dir=$1

  # save template dir path to jump to later
  templateDir=`pwd`

  # if we're on the build server this env will be set. if not use a test id
  if [ -z $BUILD_NUMBER ] ; then
    build_num="test.build"
  else
    build_num="$YEAR.$BUILD_NUMBER"
  fi

  # pull the ./ off the front of the path, and convert slashes to underscores
  subname=$(echo $dir | cut -c 3- | tr / _)
  # build full tar ball name, eg: 2012.759_template_iphone_5.0_trial.tar.bz
  name=$(echo $build_num"_template_"$subname.tar.bz)

  # move into template dir so tar paths are correct during extraction
  cd $dir

  # copy in the config_require.lua file into the templates
  cp ${WORKSPACE}/platform/resources/config_require.lua ./

  # make a tar.bz of the template dir
  tar -cjf ../../../$name ./

  cd $templateDir
}
export -f package


# when run from the templates dir in platform/iphone this will give you a list of all template combos
# -L follows symlinks
# result looks like './iphone/5.0/basic'
files=$(find -L . -maxdepth 3 -mindepth 3)


# loop over the templates from the find command and package them up
for f in $files
do
  echo $f
  package $f
done


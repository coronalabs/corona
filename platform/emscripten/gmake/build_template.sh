#!/bin/sh -e

path=`dirname $0`

CLEAN=true
config=Release

while test $# != 0
do
    case "$1" in
    -nc) CLEAN=false ;;
	-d) config=Debug
    esac
    shift
done

if [ "$CLEAN" = true ] ; then
    make clean config=$config
fi

rm -rf "$path/webtemplate_build"


rm -rf "$path/assetsFolder"
mkdir -p "$path/assetsFolder"
touch "$path/assetsFolder/CORONA_FILE_PLACEHOLDER"
mkdir -p "$path/assetsFolder/CORONA_FOLDER_PLACEHOLDER"
touch  "$path/assetsFolder/CORONA_FOLDER_PLACEHOLDER/zzz"

"$path"/build_app.sh "$path/assetsFolder" "$path/webtemplate_build/coronaHtml5App.html" $config

rm -rf "$path/assetsFolder"


pushd "$path/webtemplate_build" > /dev/null

# html
rm coronaHtml5App.html
mkdir html
mv coronaHtml5App.* html/
rm html/coronaHtml5App.data
cp ../template.webapp/emscripten.html html/index.html
cp ../template.webapp/emscripten-debug.html html/index-debug.html
cp ../template.webapp/emscripten-nosplash.html html/index-nosplash.html

cp -r ../template.webapp/fbinstant fbinstant/

# mkdir res_font
# cp ../../mac/OpenSans-Regular.ttf res_font

mkdir res_widget
cp ../../../../subrepos/widget/widget_theme_*.png res_widget/

# mkdir bin
# cp ../obj/Release/libcorona.o bin/
# cp ../../*.js bin/
# echo "return { plugin = { exportedFunctions = {'_main'}, }, }" > bin/metadata.lua

rm -f ../../webtemplate.zip
zip -r ../../webtemplate.zip ./

pushd ../../ > /dev/null
echo "Template zip:"
echo "$(pwd)/webtemplate.zip"
popd > /dev/null

popd > /dev/null
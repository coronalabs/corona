#!/bin/bash -ex	
#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

# fix path, just in case
export PATH="/usr/bin:/bin:$PATH"

# Build output.zip, emulating  server output

tmpDir=${1}
inputFile=${2}
templateFile=${3}
appName=${4}
outputFile=${5}
builder=${6}
luac=${7}
appPackage=${8}
build=${9}
plugins=${10}
splashScreen=${11}
appleTV=${12}

if [ -d "$plugins" ]; then
	prioritizedPlugins="$(find "$plugins" -mindepth 1 -maxdepth 1 -type d \( ! -name ".*" -a ! -name "shared-*" \))
$(find "$plugins" -mindepth 1 -maxdepth 1 -type d -name "shared-*")"
fi

collectedLuaPluginsDir="$tmpDir/collectedLuaPlugins"
if [ -d "$plugins" ]
then
	pluginsTmpDir="$tmpDir/luaPluginsTmp"
	rm -rf "$pluginsTmpDir" "$collectedLuaPluginsDir"
	mkdir "$pluginsTmpDir"
	for pluginDir in $prioritizedPlugins
	do
		tar -xzv -f "$pluginDir/data.tgz" -C "$pluginsTmpDir" '*.lua'
	done

	# only targeting Lua  5.1
	LUA_VM="lua_51"

	mkdir "$collectedLuaPluginsDir"
	pushd "$pluginsTmpDir" > /dev/null
		# Delete bundled lua VMs that we are not targeting.
		if [ -d "./lua" ]
		then
			find "./lua" -mindepth 1 -maxdepth 1 -type d -not -name "$LUA_VM" -print0 | xargs -0 rm -rf
		fi

		# Move the lua files into the app, while flattening:
		#	lua/$LUA_VM/plugin/foo/bar/baz.lua -> plugin.foo.bar.baz.lu
		#	plugin/foo/bar/baz.lua -> plugin.foo.bar.baz.lu
		find . -type f -name "*.lua" ! -name 'metadata.lua' | while read -r luaFile; do
			newName=$(echo "$luaFile" | sed -e 's#\./##' -e "s#^lua/##" -e "s#^$LUA_VM/##" -e 's#\.lua$#\.lu#' | tr '/' '.')
			mkdir -p "$collectedLuaPluginsDir"
			mv "$luaFile" "$collectedLuaPluginsDir/$newName"
		done
	popd > /dev/null
	rm -rf "$pluginsTmpDir" "$collectedLuaPluginsDir/metadata.lu"
fi

rm -rf "$tmpDir/output"
mkdir "$tmpDir/output"
cd "$tmpDir/output"

# extract template and replace its .lu files with ones from the input.zip
tar -xvjf "$templateFile"
if (ls "$collectedLuaPluginsDir"/*.lu) &> /dev/null
then
	mv "$collectedLuaPluginsDir"/*.lu template.app
	rmdir "$collectedLuaPluginsDir"
fi
unzip -o "$inputFile" -d template.app

# Create and merge metadata
condfigMetadata="$tmpDir/config.metadata.lua"
cat <<EOF > "$condfigMetadata"
if not application or type( application ) ~= "table" then
	application = {}
end
application.metadata = {
	appName = "$appName",
	appPackageId = "$appPackage",
	subscription = "oss",
	build = "$build",
}
EOF

touch template.app/config.lu
"$luac" -s -o "$tmpDir/config.lu" "template.app/config.lu" "$condfigMetadata"
mv "$tmpDir/config.lu" "template.app/config.lu"

# package everything into resources.car

find template.app -mindepth 1 -maxdepth 1 -iname "*.lu" | "$builder" car  -f - template.app/resource.car
find template.app -mindepth 1 -maxdepth 1 -iname "*.lu" -print0 -delete


# collect binary plugins
if [ -d "$plugins" ]
then
	buildDst="template.app/.build"
	rm -rf "$buildDst"
	mkdir -p "$buildDst"
	if [ "$appleTV" == "NO" ]
	then
		mkdir -p "$buildDst"/libtemplate
		cp libtemplate/*.lua libtemplate/libtemplate.a "$buildDst"/libtemplate
	fi

	for pluginDir in $prioritizedPlugins
	do
		pluginName=$(basename "$pluginDir")
		tar -xzvf "$pluginDir"/data.tgz -C "$pluginDir"

		mkdir "$buildDst/$pluginName"

		rsync -av --exclude='*.tgz' "$pluginDir/" "$buildDst/$pluginName"

		if [ -d "$buildDst/$pluginName/resources" ]; then
			EXISTING=$(rsync --dry-run  --recursive --prune-empty-dirs --existing --stats "$buildDst/$pluginName/resources/" template.app/  | grep 'Number of files transferred' | awk '{print $NF}' )
			if [ "$EXISTING" -gt 0 ]; then
				touch "$TMP_DIR"/build_output_failure.txt
				echo "Plugins resource file naming conflict" >> "$TMP_DIR"/build_output_failure.txt
			fi
			rsync --recursive --ignore-existing "$buildDst/$pluginName/resources/" template.app/
			rm -r "$buildDst/$pluginName/resources"
		fi
		if [ "$appleTV" == "NO" ]
		then
			rm -f template.app/template
		fi
	done
fi


# Splash screen: param is either "_NO_", "_YES_" or name of image file
if [ "$splashScreen" == "_NO_" ]
then
	# No splash screen so remove the file
	rm -v -f template.app/_CoronaSplashScreen.png
elif [ "$splashScreen" != "_YES_" ]
then
	rm -v -f template.app/_CoronaSplashScreen.png
	# Custom splash screen, put it in the right place
	if [ -n "$splashScreen" ]
	then
		mv -v -f "template.app/$splashScreen" template.app/_CoronaSplashScreen.png
	fi
fi

appNameWithoutSpaces=$(echo -n "$appName" | sed "s/ //g")
if [ -f template.app/template ]
then
	mv template.app/template template.app/"$appNameWithoutSpaces"
fi
mv template.app "$appNameWithoutSpaces.app"
zip -ry "$outputFile" -- "$appNameWithoutSpaces.app"

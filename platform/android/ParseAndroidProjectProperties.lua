--[[ 
-- Example run:
lua  -e "parameterArgTable={inputFile='/Volumes/CoronaWork6/CoronaWork6/platform/android/sdk/project.properties', templateDir='/tmp/foo', absolutePathToInputFile='/Volumes/CoronaWork6/CoronaWork6/platform/android/sdk', externalLibsPathsFile='/tmp/foo/externalLibsPaths', absolutePathToAndroidSdk='/Volumes/CoronaWork6/android-sdk'}" ParseAndroidDefaultProperties.lua 
--]]
-- Note: Path Strings are assumed to already be properly escaped.
-- This file currently does no escaping.

assert(parameterArgTable)
assert(parameterArgTable.inputFile)
assert(parameterArgTable.absolutePathToInputFile)
assert(parameterArgTable.templateDir)
assert(parameterArgTable.externalLibsPathsFile)
assert(parameterArgTable.absolutePathToAndroidSdk)

local androidLibraryList = {}

-- Makes sure the array is ordered from 1 to n and is contiguous
function CreateValidatedContiguousListFrom(the_list)
	local current_count = 1
	local return_array = {}

	-- ipairs may already skip lists that don't start at 1 or are non-contiguous.
	-- But we do extra validation to make sure and create a clean copy of the array to make sure.
	for i,v in ipairs(the_list) do
		if i ~= current_count then
			break
		end
		return_array[i] = v
		current_count = current_count + 1
	end
	return return_array
end

-- Parse the "project.properties" file for all referenced library project directories.
local file = assert(io.open(parameterArgTable.inputFile, 'r'))
for current_line in file:lines() do
	if string.match(current_line, 'android%.library%.reference%.%d+=.+') then
		-- first, avoid lines that are commented out.
		-- Warning: This is an imperfect check. Weird stuff like the hash being escaped or the hash in the middle of a line may not be supported
		if not string.match(current_line, '%.*#%.*android%.library%.reference%.%d+=.+') then
			-- We got our line. Extract out the number and the path value.
			local order_number, path_value = string.match(current_line, 'android%.library%.reference%.*(%d+)=(.+)')
			androidLibraryList[tonumber(order_number)] = path_value
		end
	end
end
file:close()
androidLibraryList = CreateValidatedContiguousListFrom(androidLibraryList)

-- Delete last externals res directories.
os.execute('rm -rf ' .. parameterArgTable.templateDir .. '/externals')

-- Traverse all referenced library project directories.
local external_libs_path_string = ''
for i,v in ipairs(androidLibraryList) do
	-- Update library project directory to use the currently selected Android SDK.
	-- This will add a "local.properties" file to to the directory so that it will compile.
	os.execute(parameterArgTable.absolutePathToAndroidSdk .. '/tools/android update lib-project -p "' .. v .. '"')
	
	-- Copy library's resource files to the template directory.
	local base_target_dir = 'externals/' .. string.format('android_library_reference_%02d', i)
	local full_target_dir = parameterArgTable.templateDir .. '/' .. base_target_dir
	local make_target_dir_command = 'mkdir -p ' .. full_target_dir
	local copy_library_command = 'cp -Rf ' .. parameterArgTable.absolutePathToInputFile .. '/' .. v .. '/res/' .. ' ' .. full_target_dir
	os.execute(make_target_dir_command)
	os.execute(copy_library_command)
	
	-- Generate/append -S flags to our string that goes into external_libs_paths
	external_libs_path_string = external_libs_path_string .. [[ -S $TEMPLATE_DIR/]] .. base_target_dir
end

-- Create the "external-libs-paths" file to be used by the build server.
-- This is needed by the Android "aapt" command to produce an R.java file in the same order it was compiled with.
file = assert(io.open(parameterArgTable.externalLibsPathsFile, 'w'))
file:write(external_libs_path_string)
file:close()

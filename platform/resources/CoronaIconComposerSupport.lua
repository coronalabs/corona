------------------------------------------------------------------------------
--
-- CoronaIconComposerSupport.lua
-- Handles Icon Composer .icon files for iOS, tvOS, and macOS builds
-- Uses actool to compile .icon files with Liquid Glass effects
--
------------------------------------------------------------------------------

local lfs = require('lfs')
local json = require('json')

local CoronaIconComposerSupport = {}

-- Utility: Quote string for shell
local function quoteString(str)
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

-- Find ictool executable (for legacy/fallback)
local function findIconTool()
	local paths = {
		"/Applications/Xcode.app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool"
	}
	
	-- Check multiple Xcode versions
	for i = 10, 30 do
		table.insert(paths, "/Applications/Xcode_" .. i .. ".app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool")
	end
	
	for _, path in ipairs(paths) do
		if os.execute("test -x " .. quoteString(path)) == 0 then
			return path
		end
	end
	
	-- Try Spotlight search
	local handle = io.popen("mdfind 'kMDItemFSName == ictool' 2>/dev/null | head -1")
	if handle then
		local path = handle:read("*l")
		handle:close()
		if path and path ~= "" and os.execute("test -x " .. quoteString(path)) == 0 then
			return path
		end
	end
	
	return nil
end

-- Platform configuration
local function getPlatformConfig(platform)
	local configs = {
		ios = {
			targetDevice = "iphone",
			platformName = "iphoneos",
			minDeployment = "11.0"
		},
		tvos = {
			targetDevice = "tv",
			platformName = "appletvos",
			minDeployment = "11.0"
		},
		macos = {
			targetDevice = "mac",
			platformName = "macosx",
			minDeployment = "10.15"
		}
	}
	
	-- Normalize platform names
	if platform == "iphone" then platform = "ios" end
	if platform == "appletvos" then platform = "tvos" end
	if platform == "osx" then platform = "macos" end
	
	return configs[platform]
end

-- Build actool command
local function buildActoolCommand(iconFilePath, outputPath, platform)
	local config = getPlatformConfig(platform)
	if not config then
		return nil, "Unsupported platform: " .. tostring(platform)
	end
	
	local plistPath = outputPath .. "/assetcatalog_info.plist"
	local iconBaseName = iconFilePath:match("([^/]+)%.icon/?$") or "AppIcon"

	local cmd = "xcrun actool " .. quoteString(iconFilePath) ..
				" --compile " .. quoteString(outputPath) ..
				" --output-format human-readable-text" ..
				" --notices --warnings --errors" ..
				" --output-partial-info-plist " .. quoteString(plistPath) ..
				" --app-icon " .. iconBaseName ..
				" --enable-on-demand-resources NO" ..
				" --development-region en" ..
				" --target-device " .. config.targetDevice ..
				" --minimum-deployment-target " .. config.minDeployment ..
				" --platform " .. config.platformName
	
	return cmd, plistPath
end

-- Validate .icon file structure
local function validateIconFile(iconFilePath)
	local attr = lfs.attributes(iconFilePath)
	if not attr or attr.mode ~= "directory" then
		return false, "Not a directory: " .. iconFilePath
	end
	
	-- Check for icon.json (Icon Composer project)
	if os.execute("test -f " .. quoteString(iconFilePath .. "/icon.json")) == 0 then
		return true, "icon_composer"
	end
	
	-- Check for Contents.json (Asset Catalog)
	if os.execute("test -f " .. quoteString(iconFilePath .. "/Contents.json")) == 0 then
		return true, "asset_catalog"
	end
	
	-- Check for PNG files
	for file in lfs.dir(iconFilePath) do
		if file:match("%.png$") then
			return true, "png_bundle"
		end
	end
	
	return false, "Invalid .icon file (missing icon.json, Contents.json, or PNG files)"
end

-- Compile .icon file with actool
local function compileIconWithActool(iconFilePath, tmpDir, platform, debugBuildProcess)
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Using actool to compile .icon file with Liquid Glass effects...")
	end
	
	-- Create output directory
	local outputPath = tmpDir .. "/CompiledAssets_" .. os.time()
	os.execute("mkdir -p " .. quoteString(outputPath))
	
	-- Build and execute actool command
	local actoolCmd, plistPath = buildActoolCommand(iconFilePath, outputPath, platform)
	if not actoolCmd then
		return nil, plistPath -- plistPath contains error message
	end
	
	if debugBuildProcess and debugBuildProcess > 1 then
		print("actool command: " .. actoolCmd)
	end
	
	local result = os.execute(actoolCmd .. " 2>&1")
	
	-- Check if Assets.car was created
	local iconBaseName = iconFilePath:match("([^/]+)%.icon/?$") or "AppIcon"
	local assetsCarPath = outputPath .. "/Assets.car"
	local testFile = io.open(assetsCarPath, "r")
	
	if testFile then
		testFile:close()
		
		if debugBuildProcess and debugBuildProcess > 0 then
			print("✓ Successfully compiled Assets.car")
			print("  Location: " .. assetsCarPath)
		end
		
		-- Clean up plist
		os.execute("rm -f " .. quoteString(plistPath))
		
		return outputPath, assetsCarPath
	else
		return nil, "actool failed to compile .icon file"
	end
end

-- Copy asset catalog structure
local function copyAssetCatalog(iconFilePath, tmpDir, debugBuildProcess)
	local tempXCAssets = tmpDir .. "/TempAssets.xcassets"
	local tempAppIconSet = tempXCAssets .. "/AppIcon.appiconset"
	
	os.execute("mkdir -p " .. quoteString(tempAppIconSet))
	
	local copyCmd = "cp -R " .. quoteString(iconFilePath) .. "/* " .. quoteString(tempAppIconSet) .. "/"
	os.execute(copyCmd)
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Copied asset catalog structure")
	end
	
	return tempXCAssets, nil
end

-- Process PNG bundle (simple PNG files)
local function processPngBundle(iconFilePath, tmpDir, platform, debugBuildProcess)
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Processing PNG bundle...")
	end
	
	local tempXCAssets = tmpDir .. "/TempAssets.xcassets"
	local tempAppIconSet = tempXCAssets .. "/AppIcon.appiconset"
	
	os.execute("mkdir -p " .. quoteString(tempAppIconSet))
	
	-- Copy all PNG files
	local copiedFiles = {}
	for file in lfs.dir(iconFilePath) do
		if file:match("%.png$") then
			local srcPath = iconFilePath .. "/" .. file
			local dstPath = tempAppIconSet .. "/" .. file
			os.execute("cp " .. quoteString(srcPath) .. " " .. quoteString(dstPath))
			table.insert(copiedFiles, file)
		end
	end
	
	if #copiedFiles == 0 then
		return nil, "No PNG files found in bundle"
	end
	
	-- Generate Contents.json for iOS universal format
	local contentsImages = {}
	
	if platform == "ios" or platform == "iphone" then
		local variants = {
			{pattern = "1024.*[Ll]ight", appearance = nil},
			{pattern = "1024.*[Dd]ark", appearance = "dark"},
			{pattern = "1024.*[Tt]inted", appearance = "tinted"}
		}
		
		for _, variant in ipairs(variants) do
			for _, file in ipairs(copiedFiles) do
				if file:match(variant.pattern) then
					local entry = {
						filename = file,
						idiom = "universal",
						platform = "ios",
						size = "1024x1024"
					}
					
					if variant.appearance then
						entry.appearances = {{
							appearance = "luminosity",
							value = variant.appearance
						}}
					end
					
					table.insert(contentsImages, entry)
					break
				end
			end
		end
	end
	
	-- Write Contents.json
	local contentsFile = io.open(tempAppIconSet .. "/Contents.json", "w")
	if not contentsFile then
		return nil, "Failed to create Contents.json"
	end
	
	contentsFile:write('{\n  "images" : [\n')
	for i, img in ipairs(contentsImages) do
		contentsFile:write('    {\n')
		
		if img.appearances then
			contentsFile:write('      "appearances" : [\n')
			contentsFile:write('        {\n')
			contentsFile:write('          "appearance" : "luminosity",\n')
			contentsFile:write('          "value" : "' .. img.appearances[1].value .. '"\n')
			contentsFile:write('        }\n')
			contentsFile:write('      ],\n')
		end
		
		contentsFile:write('      "filename" : "' .. img.filename .. '",\n')
		contentsFile:write('      "idiom" : "' .. img.idiom .. '",\n')
		contentsFile:write('      "platform" : "' .. img.platform .. '",\n')
		contentsFile:write('      "size" : "' .. img.size .. '"\n')
		contentsFile:write('    }')
		
		if i < #contentsImages then
			contentsFile:write(',')
		end
		contentsFile:write('\n')
	end
	contentsFile:write('  ],\n')
	contentsFile:write('  "info" : {\n')
	contentsFile:write('    "author" : "xcode",\n')
	contentsFile:write('    "version" : 1\n')
	contentsFile:write('  }\n')
	contentsFile:write('}\n')
	contentsFile:close()
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Successfully processed " .. #copiedFiles .. " PNG files")
	end
	
	return tempXCAssets, nil
end

-- Main: Convert .icon file to xcassets or Assets.car
function CoronaIconComposerSupport.convertIconFileToXCAssets(iconFilePath, tmpDir, platform, debugBuildProcess)
	-- Validate icon file
	local isValid, iconType = validateIconFile(iconFilePath)
	if not isValid then
		return nil, iconType
	end
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("========================================")
		print("Converting .icon file")
		print("Source: " .. iconFilePath)
		print("Type: " .. iconType)
		print("Platform: " .. platform)
		print("========================================")
	end
	
	-- Route to appropriate handler
	if iconType == "icon_composer" then
		print("Compiling .icon file with actool...")
		print(iconFilePath)
		return compileIconWithActool(iconFilePath, tmpDir, platform, debugBuildProcess)
	elseif iconType == "asset_catalog" then
		return copyAssetCatalog(iconFilePath, tmpDir, debugBuildProcess)
	elseif iconType == "png_bundle" then
		return processPngBundle(iconFilePath, tmpDir, platform, debugBuildProcess)
	end
	
	return nil, "Unknown icon type: " .. iconType
end

-- Convert alternate icon
function CoronaIconComposerSupport.convertAlternateIconToXCAssets(iconFilePath, iconName, tmpDir, platform, debugBuildProcess)
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Converting alternate icon: " .. iconName)
	end
	
	return CoronaIconComposerSupport.convertIconFileToXCAssets(iconFilePath, tmpDir, platform, debugBuildProcess)
end

-- Process all alternate icons
function CoronaIconComposerSupport.processAlternateIcons(alternateIconsConfig, srcAssets, tmpDir, platform, debugBuildProcess)
	if not alternateIconsConfig or type(alternateIconsConfig) ~= "table" then
		return nil, nil
	end
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("========================================")
		print("Processing alternate icons")
		print("========================================")
	end
	
	local alternateIconNames = {}
	local alternateIconAssets = {}
	
	for iconName, iconConfig in pairs(alternateIconsConfig) do
		if type(iconConfig) == "string" then
			-- Legacy format: just icon name
			table.insert(alternateIconNames, iconConfig)
		elseif type(iconConfig) == "table" and iconConfig.iconFile then
			-- New format: .icon file reference
			local iconFilePath = srcAssets .. "/" .. iconConfig.iconFile
			
			if lfs.attributes(iconFilePath, "mode") == "directory" then
				local outputPath, assetsCarPath = CoronaIconComposerSupport.convertAlternateIconToXCAssets(
					iconFilePath,
					iconName,
					tmpDir,
					platform,
					debugBuildProcess
				)
				
				if outputPath then
					table.insert(alternateIconNames, iconName)
					alternateIconAssets[iconName] = {
						outputPath = outputPath,
						assetsCarPath = assetsCarPath
					}
					
					if debugBuildProcess and debugBuildProcess > 0 then
						print("✓ Converted: " .. iconName)
					end
				else
					print("WARNING: Failed to convert " .. iconName .. ": " .. tostring(assetsCarPath))
				end
			else
				print("WARNING: Icon file not found: " .. iconFilePath)
			end
		end
	end
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Total alternate icons: " .. #alternateIconNames)
		print("========================================")
	end
	
	return alternateIconNames, alternateIconAssets
end

return CoronaIconComposerSupport
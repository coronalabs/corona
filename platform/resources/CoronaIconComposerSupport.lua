------------------------------------------------------------------------------
--
-- CoronaIconComposerSupport.lua
-- Handles Icon Composer .icon files for iOS, tvOS, and macOS builds
-- iOS now uses actool to compile .icon files with Liquid Glass effects
--
------------------------------------------------------------------------------

local lfs = require('lfs')
local json = require('json')

local CoronaIconComposerSupport = {}

-- Double quote a string escaping backslashes and any double quotes
local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end


-- Find ictool executable (Icon Composer's command-line tool) - for tvOS/macOS fallback
local function findIconTool()
	local paths = {
		-- Check common Xcode paths (ictool is inside Icon Composer.app)
		"/Applications/Xcode.app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool",
		"/Applications/Xcode-beta.app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool"
	}
	
	-- Check if user has multiple Xcodes (Xcode_15.app, Xcode_16.app, etc.)
	for i = 10, 30 do
		table.insert(paths, "/Applications/Xcode_" .. i .. ".app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool")
	end
	
	for _, path in ipairs(paths) do
		local testCmd = "test -x " .. quoteString(path)
		if os.execute(testCmd) == 0 then
			return path
		end
	end
	
	-- Try to find using mdfind (Spotlight search)
	local handle = io.popen("mdfind 'kMDItemFSName == ictool && kMDItemContentType == public.unix-executable' 2>/dev/null | head -1")
	if handle then
		local spotlightPath = handle:read("*l")
		handle:close()
		if spotlightPath and spotlightPath ~= "" then
			local testCmd = "test -x " .. quoteString(spotlightPath)
			if os.execute(testCmd) == 0 then
				return spotlightPath
			end
		end
	end
	
	return nil
end

-- Build actool command for compiling .icon file
local function buildActoolCommand(actool, iconFilePath, outputPath, platform, debugBuildProcess)
	local plistPath = outputPath .. "/assetcatalog_generated_info.plist"
	
	-- Determine platform-specific parameters
	local targetDevice = "iphone"
	local platformName = "iphoneos"
	local minDeployment = "13.0"
	
	if platform == "ios" or platform == "iphone" then
		targetDevice = "iphone"
		platformName = "iphoneos"
		minDeployment = "13.0"
	elseif platform == "tvos" or platform == "appletvos" then
		targetDevice = "tv"
		platformName = "appletvos"
		minDeployment = "13.0"
	elseif platform == "macos" or platform == "osx" then
		targetDevice = "mac"
		platformName = "macosx"
		minDeployment = "10.15"
	end
	
	-- Build actool command
	-- actool compiles asset catalogs (.xcassets or .icon) into Assets.car files
	local cmd =  "xcrun actool " .. quoteString(iconFilePath) ..
				" --compile " .. quoteString(outputPath) ..
				" --output-format human-readable-text" ..
				" --notices --warnings --errors" ..
				" --output-partial-info-plist " .. quoteString(plistPath) ..
				" --app-icon AppIcon" ..
				" --enable-on-demand-resources NO" ..
				" --development-region en" ..
				" --target-device " .. targetDevice ..
				" --minimum-deployment-target " .. minDeployment ..
				" --platform " .. platformName
	
	print("actool command: " .. cmd)
	
	return cmd, plistPath
end

-- Build ictool command for exporting icons (fallback for tvOS/macOS)
local function buildIconToolCommand(ictool, iconFilePath, outputPath, platform, size, appearanceMode, scale)
	scale = scale or 1
	local appearance = appearanceMode or "Light"
	
	local cmd = quoteString(ictool) .. " " .. quoteString(iconFilePath) ..
				" --export-preview " .. platform .. " " .. appearance ..
				" " .. size .. " " .. size .. " " .. scale ..
				" " .. quoteString(outputPath)
	
	return cmd
end

-- Platform-specific icon specifications (for ictool fallback)
local function getIconSpecsForPlatform(platform)
	if platform == "ios" or platform == "iphone" then
		return {
			platform = "iOS",
			sizes = {
				{name = "Icon", size = 1024, idiom = "universal", platform = "ios", sizeKey = "1024x1024", scale = nil}
			}
		}
	elseif platform == "tvos" or platform == "appletvos" then
		return {
			platform = "tvOS",
			sizes = {
				{name = "App-Icon-Small", size = 400, idiom = "tv", scale = "1x", sizeKey = "400x240"},
				{name = "App-Icon-Small@2x", size = 800, idiom = "tv", scale = "2x", sizeKey = "400x240"},
				{name = "App-Icon", size = 1280, idiom = "tv", scale = "1x", sizeKey = "1280x768"},
				{name = "App-Icon@2x", size = 2560, idiom = "tv", scale = "2x", sizeKey = "1280x768"},
				{name = "Top-Shelf-Image", size = 1920, idiom = "tv", scale = "1x", sizeKey = "1920x720"},
				{name = "Top-Shelf-Image@2x", size = 3840, idiom = "tv", scale = "2x", sizeKey = "1920x720"}
			}
		}
	elseif platform == "macos" or platform == "osx" then
		return {
			platform = "macOS",
			sizes = {
				{name = "icon_16x16", size = 16, idiom = "mac", scale = "1x", sizeKey = "16x16"},
				{name = "icon_16x16@2x", size = 32, idiom = "mac", scale = "2x", sizeKey = "16x16"},
				{name = "icon_32x32", size = 32, idiom = "mac", scale = "1x", sizeKey = "32x32"},
				{name = "icon_32x32@2x", size = 64, idiom = "mac", scale = "2x", sizeKey = "32x32"},
				{name = "icon_128x128", size = 128, idiom = "mac", scale = "1x", sizeKey = "128x128"},
				{name = "icon_128x128@2x", size = 256, idiom = "mac", scale = "2x", sizeKey = "128x128"},
				{name = "icon_256x256", size = 256, idiom = "mac", scale = "1x", sizeKey = "256x256"},
				{name = "icon_256x256@2x", size = 512, idiom = "mac", scale = "2x", sizeKey = "256x256"},
				{name = "icon_512x512", size = 512, idiom = "mac", scale = "1x", sizeKey = "512x512"},
				{name = "icon_512x512@2x", size = 1024, idiom = "mac", scale = "2x", sizeKey = "512x512"}
			}
		}
	end
	
	return nil
end

-- Check if .icon file is a valid bundle
local function validateIconFile(iconFilePath)
	-- Check if it's a directory
	local attr = lfs.attributes(iconFilePath)
	if not attr or attr.mode ~= "directory" then
		return false, "Not a directory: " .. iconFilePath
	end
	
	-- Check for icon.json (Icon Composer project)
	local iconJson = iconFilePath .. "/icon.json"
	local hasIconJson = os.execute("test -f " .. quoteString(iconJson)) == 0
	
	if hasIconJson then
		return true, "icon_composer"
	end
	
	-- Check for Contents.json (Asset Catalog format inside .icon)
	local contentsJson = iconFilePath .. "/Contents.json"
	local hasContentsJson = os.execute("test -f " .. quoteString(contentsJson)) == 0
	
	if hasContentsJson then
		return true, "asset_catalog"
	end
	
	-- Check if it contains PNG files directly
	local hasPngs = false
	for file in lfs.dir(iconFilePath) do
		if file:match("%.png$") then
			hasPngs = true
			break
		end
	end
	
	if hasPngs then
		return true, "png_bundle"
	end
	
	return false, "Invalid .icon file structure (no icon.json, Contents.json, or PNG files)"
end

-- Copy PNG files directly from .icon bundle to xcassets
local function copyPngBundleToXCAssets(iconFilePath, tempAppIconSet, iconSpecs, debugBuildProcess, platform)
	local contentsImages = {}
	local copiedFiles = {}
	
	-- Copy all PNG files from .icon directory
	for file in lfs.dir(iconFilePath) do
		if file:match("%.png$") then
			local srcPath = iconFilePath .. "/" .. file
			local dstPath = tempAppIconSet .. "/" .. file
			
			-- Copy the file
			local copyCmd = "cp " .. quoteString(srcPath) .. " " .. quoteString(dstPath)
			os.execute(copyCmd)
			
			if debugBuildProcess and debugBuildProcess > 1 then
				print("  Copied: " .. file)
			end
			
			table.insert(copiedFiles, file)
		end
	end
	
	-- For iOS universal format, look for 1024x1024 icons
	if platform == "ios" or platform == "iphone" then
		-- Look for light, dark, and tinted variants
		local variants = {
			{pattern = "1024.*[Ll]ight", appearance = nil, suffix = "Light"},
			{pattern = "1024.*[Dd]ark", appearance = "dark", suffix = "Dark"},
			{pattern = "1024.*[Tt]inted", appearance = "tinted", suffix = "Tinted"}
		}
		
		for _, variant in ipairs(variants) do
			local matchedFile = nil
			for _, file in ipairs(copiedFiles) do
				if file:match(variant.pattern) then
					matchedFile = file
					break
				end
			end
			
			if matchedFile then
				local imageEntry = {
					filename = matchedFile,
					idiom = "universal",
					platform = "ios",
					size = "1024x1024"
				}
				
				if variant.appearance then
					imageEntry.appearances = {{
						appearance = "luminosity",
						value = variant.appearance
					}}
				end
				
				table.insert(contentsImages, imageEntry)
			end
		end
	else
		-- For other platforms, use the old matching logic
		for _, spec in ipairs(iconSpecs.sizes) do
			local basePattern = spec.size .. "x" .. spec.size
			local matchedFile = nil
			
			for _, file in ipairs(copiedFiles) do
				if file:match(basePattern) or file:match(spec.name) then
					matchedFile = file
					break
				end
			end
			
			if matchedFile then
				local imageEntry = {
					filename = matchedFile,
					idiom = spec.idiom,
					scale = spec.scale,
					size = spec.sizeKey
				}
				table.insert(contentsImages, imageEntry)
			end
		end
	end
	
	return contentsImages, #copiedFiles
end

-- Compile .icon file using actool (for iOS with Liquid Glass effects)
local function compileIconWithActool(actool, iconFilePath, tmpDir, platform, debugBuildProcess)
	
		print("Using actool to compile .icon file with Liquid Glass effects...")
	
	-- Create output directory for compiled assets
	local outputPath = tmpDir .. "/CompiledAssets"
	os.execute("mkdir -p " .. quoteString(outputPath))
	
	-- Build and execute actool command
	local actoolCmd, plistPath = buildActoolCommand(actool, iconFilePath, outputPath, platform, debugBuildProcess)
	
	if debugBuildProcess and debugBuildProcess > 1 then
		print("Executing actool...")
	end
	
	local result = os.execute(actoolCmd .. " 2>&1")
	
	-- Check if Assets.car was created
	local assetsCarPath = outputPath .. "/Assets.car"
	local testFile = io.open(assetsCarPath, "r")
	if testFile then
		testFile:close()
		
		
			print("✓ Successfully compiled Assets.car with actool")
			print("  Location: " .. assetsCarPath)
		
		-- Clean up the generated plist file
		os.execute("rm -f " .. quoteString(plistPath))
		
		return outputPath, assetsCarPath
	else
		return nil, "actool failed to compile .icon file. Check that the .icon file is valid."
	end
end

-- Export icons from .icon file using ictool (fallback for tvOS/macOS)
local function exportIconWithIctool(ictool, iconFilePath, tmpDir, platform, iconSpecs, debugBuildProcess)
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Using ictool to export icon layers...")
	end
	
	local tempXCAssets = tmpDir .. "/TempAssets.xcassets"
	local tempAppIconSet = tempXCAssets .. "/AppIcon.appiconset"
	
	os.execute("mkdir -p " .. quoteString(tempAppIconSet))
	
	-- Appearance modes to export
	local appearances = {
		{mode = "Light", suffix = "-Light", jsonAppearance = nil},
		{mode = "Dark", suffix = "-Dark", jsonAppearance = "dark"}
	}
	
	-- Add tinted appearance for iOS (though we prefer actool for iOS)
	if platform == "ios" or platform == "iphone" then
		table.insert(appearances, {mode = "Tinted", suffix = "-Tinted", jsonAppearance = "tinted"})
	end
	
	local contentsImages = {}
	local successCount = 0
	local failCount = 0
	
	-- Export each icon size and appearance using ictool
	for _, spec in ipairs(iconSpecs.sizes) do
		for _, appearance in ipairs(appearances) do
			local filename = spec.name .. appearance.suffix .. "-" .. spec.sizeKey .. ".png"
			local outputPath = tempAppIconSet .. "/" .. filename
			
			-- Build ictool command
			local ictoolCmd = buildIconToolCommand(ictool, iconFilePath, outputPath, iconSpecs.platform, spec.size, appearance.mode, 1)
			
			if debugBuildProcess and debugBuildProcess > 1 then
				print("  Exporting: " .. filename .. " (" .. spec.size .. "x" .. spec.size .. ", " .. appearance.mode .. ")")
			end
			
			os.execute(ictoolCmd .. " 2>&1")
			
			-- Check if file was created
			local testFile = io.open(outputPath, "r")
			if testFile then
				testFile:close()
				successCount = successCount + 1
				
				-- Build Contents.json entry
				local imageEntry = {
					filename = filename,
					idiom = spec.idiom,
					size = spec.sizeKey
				}
				
				-- Add platform for iOS universal format
				if platform == "ios" or platform == "iphone" then
					imageEntry.platform = "ios"
				end
				
				-- Add scale for non-iOS platforms
				if spec.scale then
					imageEntry.scale = spec.scale
				end
				
				-- Add appearance info for dark and tinted modes
				if appearance.jsonAppearance then
					imageEntry.appearances = {{
						appearance = "luminosity",
						value = appearance.jsonAppearance
					}}
				end
				
				table.insert(contentsImages, imageEntry)
			else
				failCount = failCount + 1
				if debugBuildProcess and debugBuildProcess > 1 then
					print("    WARNING: Failed to export " .. filename)
				end
			end
		end
	end
	
	if successCount == 0 then
		return nil, "Failed to export any icons with ictool"
	end
	
	-- Generate Contents.json
	local contentsFile = io.open(tempAppIconSet .. "/Contents.json", "w")
	if not contentsFile then
		return nil, "Failed to create Contents.json"
	end
	
	-- Write Contents.json
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
		contentsFile:write('      "idiom" : "' .. img.idiom .. '"')
		
		if img.platform then
			contentsFile:write(',\n      "platform" : "' .. img.platform .. '"')
		end
		
		if img.scale then
			contentsFile:write(',\n      "scale" : "' .. img.scale .. '"')
		end
		
		contentsFile:write(',\n      "size" : "' .. img.size .. '"')
		contentsFile:write('\n    }')
		
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
		print("Successfully exported " .. successCount .. " icon variants")
		if failCount > 0 then
			print("WARNING: " .. failCount .. " icon exports failed")
		end
	end
	
	return tempXCAssets, nil
end

-- Main function: Convert .icon file to xcassets or Assets.car
function CoronaIconComposerSupport.convertIconFileToXCAssets(iconFilePath, tmpDir, platform, debugBuildProcess)
	-- Validate .icon file and determine its type
	local isValid, iconType = validateIconFile(iconFilePath)
	
	if not isValid then
		return nil, iconType -- iconType contains error message
	end
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("========================================")
		print("Converting .icon file")
		print("Source: " .. iconFilePath)
		print("Type: " .. iconType)
		print("Platform: " .. platform)
		print("========================================")
	end
	
	-- Get platform-specific icon sizes
	local iconSpecs = getIconSpecsForPlatform(platform)
	if not iconSpecs then
		return nil, "Unsupported platform for Icon Composer: " .. tostring(platform)
	end
	
	-- Create temporary xcassets structure
	local tempXCAssets = tmpDir .. "/TempAssets.xcassets"
	local tempAppIconSet = tempXCAssets .. "/AppIcon.appiconset"
	
	os.execute("mkdir -p " .. quoteString(tempAppIconSet))
	
	local contentsImages = {}
	local successCount = 0
	
	-- Handle different .icon file types
	if iconType == "png_bundle" then
		-- Simple PNG bundle - just copy files
		if debugBuildProcess and debugBuildProcess > 0 then
			print("Processing PNG bundle...")
		end
		
		contentsImages, successCount = copyPngBundleToXCAssets(iconFilePath, tempAppIconSet, iconSpecs, debugBuildProcess, platform)
		
	elseif iconType == "asset_catalog" then
		-- Asset catalog inside .icon - copy the whole structure
		if debugBuildProcess and debugBuildProcess > 0 then
			print("Processing asset catalog...")
		end
		
		local copyCmd = "cp -R " .. quoteString(iconFilePath) .. "/* " .. quoteString(tempAppIconSet) .. "/"
		os.execute(copyCmd)
		
		if debugBuildProcess and debugBuildProcess > 0 then
			print("Copied asset catalog structure")
		end
		
		return tempXCAssets, nil
		
	else -- iconType == "icon_composer"
		-- Icon Composer project - use actool for iOS, ictool for others
		
		local outputPath, assetsCarPath = compileIconWithActool("xcrun actool", iconFilePath, tmpDir, platform, debugBuildProcess)
				
		if outputPath then
			print("Generated compiled assets with Liquid Glass effects")
			print("========================================")
			return outputPath, assetsCarPath
		else
			-- Fall back to ictool if actool fails
			if debugBuildProcess and debugBuildProcess > 0 then
				print("WARNING: actool compilation failed, falling back to ictool")
			end
		end
		
	
	end
	
	if successCount == 0 then
		return nil, "Failed to export any icons from " .. iconFilePath .. ". Check that the .icon file is valid."
	end
	
	-- Generate Contents.json for PNG bundle
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
		contentsFile:write('      "idiom" : "' .. img.idiom .. '"')
		
		if img.platform then
			contentsFile:write(',\n      "platform" : "' .. img.platform .. '"')
		end
		
		if img.scale then
			contentsFile:write(',\n      "scale" : "' .. img.scale .. '"')
		end
		
		contentsFile:write(',\n      "size" : "' .. img.size .. '"')
		contentsFile:write('\n    }')
		
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
		print("Successfully processed " .. successCount .. " icon variants")
		print("Generated xcassets: " .. tempXCAssets)
		print("========================================")
	end
	
	return tempXCAssets, nil
end

-- Export alternate icon from .icon file
function CoronaIconComposerSupport.convertAlternateIconToXCAssets(iconFilePath, iconName, tmpDir, platform, debugBuildProcess)
	-- For iOS, try actool first
	if platform == "ios" or platform == "iphone" then
		local actool = findActool()
		
		if actool then
				print("Converting alternate icon: " .. iconName .. " with actool")
			
			local outputPath, assetsCarPath = compileIconWithActool(actool, iconFilePath, tmpDir, platform, debugBuildProcess)
			
			if outputPath then
				return outputPath, assetsCarPath
			end
		end
	end
	
	-- Fallback to ictool
	local ictool = findIconTool()
	
	if not ictool then
		return nil, "Neither actool nor ictool found. Xcode must be installed."
	end
	
	-- Verify .icon file exists
	local iconJson = iconFilePath .. "/icon.json"
	local hasIconJson = os.execute("test -f " .. quoteString(iconJson)) == 0
	
	if not hasIconJson then
		return nil, "Not a valid Icon Composer .icon file: " .. iconFilePath
	end
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Converting alternate icon: " .. iconName .. " from " .. iconFilePath)
	end
	
	-- Get platform-specific icon sizes
	local iconSpecs = getIconSpecsForPlatform(platform)
	if not iconSpecs then
		return nil, "Unsupported platform: " .. tostring(platform)
	end
	
	return exportIconWithIctool(ictool, iconFilePath, tmpDir, platform, iconSpecs, debugBuildProcess)
end

-- Process all alternate icons from build.settings
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
	local tempXCAssets = tmpDir .. "/AlternateIcons.xcassets"
	local hasAlternateIcons = false
	
	for iconName, iconConfig in pairs(alternateIconsConfig) do
		local iconFilePath = nil
		
		if type(iconConfig) == "string" then
			-- Old format: alternateIcons = { "Icon1", "Icon2" }
			table.insert(alternateIconNames, iconConfig)
			if debugBuildProcess and debugBuildProcess > 0 then
				print("Alternate icon (xcassets): " .. iconConfig)
			end
		elseif type(iconConfig) == "table" and iconConfig.iconFile then
			-- New format: alternateIcons = { DarkIcon = { iconFile = "DarkIcon.icon" } }
			iconFilePath = srcAssets .. "/" .. iconConfig.iconFile
			
			if iconFilePath:match("%.icon/?$") and lfs.attributes(iconFilePath, "mode") == "directory" then
				local altXCAssets, errMsg = CoronaIconComposerSupport.convertAlternateIconToXCAssets(
					iconFilePath,
					iconName,
					tmpDir,
					platform,
					debugBuildProcess
				)
				
				if altXCAssets then
					hasAlternateIcons = true
					table.insert(alternateIconNames, iconName)
					if debugBuildProcess and debugBuildProcess > 0 then
						print("Converted alternate icon: " .. iconName .. " from " .. iconConfig.iconFile)
					end
				else
					print("WARNING: Failed to convert alternate icon: " .. tostring(errMsg))
				end
			else
				print("WARNING: Alternate icon file not found: " .. iconFilePath)
			end
		end
	end
	
	if #alternateIconNames == 0 then
		return nil, nil
	end
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("Total alternate icons: " .. #alternateIconNames)
		print("========================================")
	end
	
	return alternateIconNames, (hasAlternateIcons and tempXCAssets or nil)
end



return CoronaIconComposerSupport
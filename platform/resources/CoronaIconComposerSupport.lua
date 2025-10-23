------------------------------------------------------------------------------
--
-- CoronaIconComposerSupport.lua
-- Handles Icon Composer .icon files for iOS, tvOS, and macOS builds
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

-- Find ictool executable (Icon Composer's command-line tool)
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

-- Build ictool command for exporting icons
-- Format: ictool input-document --export-preview platform appearance width height scale output-png-path
local function buildIconToolCommand(ictool, iconFilePath, outputPath, platform, size, appearanceMode, scale)
	scale = scale or 1
	local appearance = appearanceMode or "Light"
	
	-- ictool command format (based on official usage)
	-- ictool input-document --export-preview platform appearance width height scale output-png-path
	local cmd = quoteString(ictool) .. " " .. quoteString(iconFilePath) ..
				" --export-preview " .. platform .. " " .. appearance ..
				" " .. size .. " " .. size .. " " .. scale ..
				" " .. quoteString(outputPath)

	return cmd
end

-- Detect ictool command format and return command builder function
local function getIconToolCommandBuilder(ictool, debugBuildProcess)
	-- Get help output to detect command format
	local handle = io.popen(quoteString(ictool) .. " --help 2>&1")
	local helpOutput = ""
	if handle then
		helpOutput = handle:read("*a") or ""
		handle:close()
	end
	
	local usesExportSubcommand = helpOutput:match("export") ~= nil
	
	if debugBuildProcess and debugBuildProcess > 1 then
		print("ictool format: " .. (usesExportSubcommand and "modern (export subcommand)" or "legacy"))
	end
	
	-- Return function that builds commands based on detected format
	return function(iconFilePath, outputPath, platform, size, appearanceMode)
		scale = scale or 1
		local appearance = appearanceMode or "Light"
		local cmd = quoteString(ictool) .. " " .. quoteString(iconFilePath) ..
				" --export-preview " .. platform .. " " .. appearance ..
				" " .. size .. " " .. size .. " " .. scale ..
				" " .. quoteString(outputPath)
		return cmd
	end
end

-- Platform-specific icon specifications
local function getIconSpecsForPlatform(platform)
	if platform == "ios" or platform == "iphone" then
		return {
			platform = "iOS",
			sizes = {
				-- iPhone
				{name = "Icon-App-20x20@2x", size = 40, idiom = "iphone", scale = "2x", sizeKey = "20x20"},
				{name = "Icon-App-20x20@3x", size = 60, idiom = "iphone", scale = "3x", sizeKey = "20x20"},
				{name = "Icon-App-29x29@2x", size = 58, idiom = "iphone", scale = "2x", sizeKey = "29x29"},
				{name = "Icon-App-29x29@3x", size = 87, idiom = "iphone", scale = "3x", sizeKey = "29x29"},
				{name = "Icon-App-40x40@2x", size = 80, idiom = "iphone", scale = "2x", sizeKey = "40x40"},
				{name = "Icon-App-40x40@3x", size = 120, idiom = "iphone", scale = "3x", sizeKey = "40x40"},
				{name = "Icon-App-60x60@2x", size = 120, idiom = "iphone", scale = "2x", sizeKey = "60x60"},
				{name = "Icon-App-60x60@3x", size = 180, idiom = "iphone", scale = "3x", sizeKey = "60x60"},
				-- iPad
				{name = "Icon-App-20x20@1x", size = 20, idiom = "ipad", scale = "1x", sizeKey = "20x20"},
				{name = "Icon-App-20x20@2x", size = 40, idiom = "ipad", scale = "2x", sizeKey = "20x20"},
				{name = "Icon-App-29x29@1x", size = 29, idiom = "ipad", scale = "1x", sizeKey = "29x29"},
				{name = "Icon-App-29x29@2x", size = 58, idiom = "ipad", scale = "2x", sizeKey = "29x29"},
				{name = "Icon-App-40x40@1x", size = 40, idiom = "ipad", scale = "1x", sizeKey = "40x40"},
				{name = "Icon-App-40x40@2x", size = 80, idiom = "ipad", scale = "2x", sizeKey = "40x40"},
				{name = "Icon-App-76x76@1x", size = 76, idiom = "ipad", scale = "1x", sizeKey = "76x76"},
				{name = "Icon-App-76x76@2x", size = 152, idiom = "ipad", scale = "2x", sizeKey = "76x76"},
				{name = "Icon-App-83.5x83.5@2x", size = 167, idiom = "ipad", scale = "2x", sizeKey = "83.5x83.5"},
				-- App Store
				{name = "Icon-App-1024x1024@1x", size = 1024, idiom = "ios-marketing", scale = "1x", sizeKey = "1024x1024"}
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
local function copyPngBundleToXCAssets(iconFilePath, tempAppIconSet, iconSpecs, debugBuildProcess)
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
	
	-- Try to match files to expected icon sizes
	for _, spec in ipairs(iconSpecs.sizes) do
		-- Look for matching files
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
	
	return contentsImages, #copiedFiles
end

-- Export icons from .icon file using ictool
function CoronaIconComposerSupport.convertIconFileToXCAssets(iconFilePath, tmpDir, platform, debugBuildProcess)
	-- Validate .icon file and determine its type
	local isValid, iconType = validateIconFile(iconFilePath)
	
	if not isValid then
		return nil, iconType -- iconType contains error message
	end
	
	if debugBuildProcess and debugBuildProcess > 0 then
		print("========================================")
		print("Converting .icon file to xcassets")
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
	local tempXCAssets = tmpDir .. "/GeneratedIconAssets.xcassets"
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
		
		contentsImages, successCount = copyPngBundleToXCAssets(iconFilePath, tempAppIconSet, iconSpecs, debugBuildProcess)
		
	elseif iconType == "asset_catalog" then
		-- Asset catalog inside .icon - copy the whole structure
		if debugBuildProcess and debugBuildProcess > 0 then
			print("Processing asset catalog...")
		end
		
		local copyCmd = "cp -R " .. quoteString(iconFilePath) .. "/* " .. quoteString(tempAppIconSet) .. "/"
		os.execute(copyCmd)
		
		-- The Contents.json already exists, just return success
		if debugBuildProcess and debugBuildProcess > 0 then
			print("Copied asset catalog structure")
		end
		
		return tempXCAssets, nil
		
	else -- iconType == "icon_composer"
		-- Icon Composer project - use ictool to render
		local ictool = findIconTool()
		
		if not ictool then
			return nil, "ictool not found. Icon Composer must be installed with Xcode. Try: xcrun -f ictool"
		end
		
		if debugBuildProcess and debugBuildProcess > 0 then
			print("Using ictool: " .. ictool)
		end
		
		-- Get command builder
		local buildCommand = getIconToolCommandBuilder(ictool, debugBuildProcess)
		-- Get command builder
		local buildCommand = getIconToolCommandBuilder(ictool, debugBuildProcess)
		
		-- Appearance modes to export
		local appearances = {
			{mode = "Light", suffix = "", jsonAppearance = nil},
			{mode = "Dark", suffix = "-dark", jsonAppearance = "dark"}
		}
		
		local failCount = 0
		
		-- Export each icon size and appearance using ictool
		for _, spec in ipairs(iconSpecs.sizes) do
			for _, appearance in ipairs(appearances) do
				local filename = spec.name .. appearance.suffix .. ".png"
				local outputPath = tempAppIconSet .. "/" .. filename
				
				-- Build ictool command using detected format
				local ictoolCmd = buildCommand(iconFilePath, outputPath, iconSpecs.platform, spec.size, appearance.mode)
				
				if debugBuildProcess and debugBuildProcess > 1 then
					print("  Exporting: " .. filename .. " (" .. spec.size .. "x" .. spec.size .. ", " .. appearance.mode .. ")")
					print("  Command: " .. ictoolCmd)
				end
				
				local result = os.execute(ictoolCmd .. " 2>&1")
				
				-- Check if file was created
				local testFile = io.open(outputPath, "r")
				if testFile then
					testFile:close()
					successCount = successCount + 1
					
					-- Build Contents.json entry
					local imageEntry = {
						filename = filename,
						idiom = spec.idiom,
						scale = spec.scale,
						size = spec.sizeKey
					}
					
					-- Add appearance info for dark mode
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
		
		if debugBuildProcess and debugBuildProcess > 0 then
			if failCount > 0 then
				print("WARNING: " .. failCount .. " icon exports failed")
			end
		end
	end
	
	if successCount == 0 then
		return nil, "Failed to export any icons from " .. iconFilePath .. ". Check that the .icon file is valid."
	end
	
	-- Generate Contents.json
	local contentsFile = io.open(tempAppIconSet .. "/Contents.json", "w")
	if not contentsFile then
		return nil, "Failed to create Contents.json"
	end
	
	-- Write Contents.json manually (simple JSON generation)
	contentsFile:write('{\n  "images" : [\n')
	for i, img in ipairs(contentsImages) do
		contentsFile:write('    {\n')
		contentsFile:write('      "filename" : "' .. img.filename .. '",\n')
		contentsFile:write('      "idiom" : "' .. img.idiom .. '",\n')
		contentsFile:write('      "scale" : "' .. img.scale .. '",\n')
		contentsFile:write('      "size" : "' .. img.size .. '"')
		
		if img.appearances then
			contentsFile:write(',\n      "appearances" : [\n')
			contentsFile:write('        {\n')
			contentsFile:write('          "appearance" : "luminosity",\n')
			contentsFile:write('          "value" : "' .. img.appearances[1].value .. '"\n')
			contentsFile:write('        }\n')
			contentsFile:write('      ]')
		end
		
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
		print("Generated xcassets: " .. tempXCAssets)
		print("========================================")
	end
	
	return tempXCAssets, nil
end

-- Export alternate icon from .icon file
function CoronaIconComposerSupport.convertAlternateIconToXCAssets(iconFilePath, iconName, tmpDir, platform, debugBuildProcess)
	local ictool = findIconTool()
	
	if not ictool then
		return nil, "ictool not found. Icon Composer must be installed with Xcode. Try: xcrun -f ictool"
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
	
	-- Get platform-specific icon sizes (only need specific sizes for alternate icons)
	local iconSpecs = getIconSpecsForPlatform(platform)
	if not iconSpecs then
		return nil, "Unsupported platform: " .. tostring(platform)
	end
	
	-- Create xcassets structure for this alternate icon
	local tempXCAssets = tmpDir .. "/AlternateIcons.xcassets"
	local tempAppIconSet = tempXCAssets .. "/" .. iconName .. ".appiconset"
	
	os.execute("mkdir -p " .. quoteString(tempAppIconSet))
	
	-- Appearance modes
	local appearances = {
		{mode = "Light", suffix = "", jsonAppearance = nil},
		{mode = "Dark", suffix = "-dark", jsonAppearance = "dark"}
	}
	
	local contentsImages = {}
	local successCount = 0
	
	-- Export each icon size
	for _, spec in ipairs(iconSpecs.sizes) do
		-- Alternate icons typically don't need all sizes, focus on app icons
		-- Skip iPad and marketing sizes for alternate icons unless needed
		local skipSize = false
		if platform == "ios" or platform == "iphone" then
			-- Only include iPhone app icons for alternates
			if spec.idiom ~= "iphone" then
				skipSize = true
			end
		end
		
		if not skipSize then
			for _, appearance in ipairs(appearances) do
				local filename = spec.name .. appearance.suffix .. ".png"
				local outputPath = tempAppIconSet .. "/" .. filename
				
				-- Build ictool command
				local ictoolCmd = buildIconToolCommand(ictool, iconFilePath, outputPath, iconSpecs.platform, spec.size, appearance.mode, 1)
				
				if debugBuildProcess and debugBuildProcess > 1 then
					print("  Exporting alternate: " .. filename)
				end
				
				os.execute(ictoolCmd .. " 2>&1")
				
				local testFile = io.open(outputPath, "r")
				if testFile then
					testFile:close()
					successCount = successCount + 1
					
					local imageEntry = {
						filename = filename,
						idiom = spec.idiom,
						scale = spec.scale,
						size = spec.sizeKey
					}
					
					if appearance.jsonAppearance then
						imageEntry.appearances = {{
							appearance = "luminosity",
							value = appearance.jsonAppearance
						}}
					end
					
					table.insert(contentsImages, imageEntry)
				end
			end
		end
	end
	
	if successCount == 0 then
		return nil, "Failed to export any alternate icons from " .. iconFilePath
	end
	
	-- Generate Contents.json for alternate icon
	local contentsFile = io.open(tempAppIconSet .. "/Contents.json", "w")
	if not contentsFile then
		return nil, "Failed to create Contents.json for alternate icon"
	end
	
	contentsFile:write('{\n  "images" : [\n')
	for i, img in ipairs(contentsImages) do
		contentsFile:write('    {\n')
		contentsFile:write('      "filename" : "' .. img.filename .. '",\n')
		contentsFile:write('      "idiom" : "' .. img.idiom .. '",\n')
		contentsFile:write('      "scale" : "' .. img.scale .. '",\n')
		contentsFile:write('      "size" : "' .. img.size .. '"')
		
		if img.appearances then
			contentsFile:write(',\n      "appearances" : [\n')
			contentsFile:write('        {\n')
			contentsFile:write('          "appearance" : "luminosity",\n')
			contentsFile:write('          "value" : "' .. img.appearances[1].value .. '"\n')
			contentsFile:write('        }\n')
			contentsFile:write('      ]')
		end
		
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
		print("Exported " .. successCount .. " variants for alternate icon: " .. iconName)
	end
	
	return tempXCAssets, nil
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
	
	-- alternateIconsConfig can be:
	-- 1. Array of icon names (existing behavior - looks for .xcassets)
	-- 2. Table with iconFile paths (new .icon support)
	
	for iconName, iconConfig in pairs(alternateIconsConfig) do
		-- Check if this is a .icon file reference
		local iconFilePath = nil
		
		if type(iconConfig) == "string" then
			-- Old format: alternateIcons = { "Icon1", "Icon2" }
			-- This means look in existing xcassets
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

-- Diagnostic function to help troubleshoot icon tool issues
function CoronaIconComposerSupport.diagnoseIconTool()
	print("========================================")
	print("Icon Composer Tool Diagnostics")
	print("========================================")
	
	-- Check for ictool
	local ictool = findIconTool()
	if ictool then
		print("✓ Found ictool: " .. ictool)
		
		-- Get version
		local versionCmd = quoteString(ictool) .. " --version 2>&1"
		print("\nVersion:")
		os.execute(versionCmd)
		
		-- Show example usage
		print("\nExample usage:")
		print("  " .. ictool .. " input.icon --export-preview iOS Light 180 180 1 output.png")
		
	else
		print("✗ ictool not found")
		print("\nSearched in:")
		print("  /Applications/Xcode.app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool")
		print("  /Applications/Xcode_XX.app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool")
		print("\nTo locate manually:")
		print("  mdfind 'kMDItemFSName == ictool'")
		print("\nIcon Composer.app should be inside Xcode.app bundle")
	end
	
	print("========================================")
end

-- Test if ictool is available and working
function CoronaIconComposerSupport.testIconTool()
	local ictool = findIconTool()
	
	if not ictool then
		print("ERROR: ictool not found")
		print("Icon Composer must be inside Xcode.app bundle")
		print("Path should be: /Applications/Xcode.app/Contents/Applications/Icon Composer.app/Contents/Executables/ictool")
		return false
	end
	
	print("Found ictool at: " .. ictool)
	
	-- Test version command
	local handle = io.popen(quoteString(ictool) .. " --version 2>&1")
	local versionOutput = ""
	if handle then
		versionOutput = handle:read("*a")
		handle:close()
	end
	
	if versionOutput and versionOutput ~= "" then
		print("ictool version: " .. versionOutput:gsub("\n", " "))
		print("ictool is working correctly")
		return true
	else
		print("WARNING: ictool found but --version command failed")
		return false
	end
end

return CoronaIconComposerSupport
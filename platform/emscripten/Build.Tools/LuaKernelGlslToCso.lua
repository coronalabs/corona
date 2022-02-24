----------------------------------------------------------------------------------------------------
-- Converts a "kernel" Lua file's OpenGL shader code into compiled Direct3D HLSL binary.
-- 
-- Note:  This tool is needed for Windows Phone 8.0 builds since that platform version does not
--        support dynamic shader compilation.
----------------------------------------------------------------------------------------------------


-- Fetch command line arguments.
local sourceFilePath = arg[1]
local targetFilePath = arg[2]
local intermediatePath = arg[3]
local glslConverterFilePath = arg[4]

-- Do not continue if missing required arguments.
if (not sourceFilePath) or (not targetFilePath) or (not intermediatePath) or (not glslConverterFilePath) then
	print("Converts a \"kernel\" Lua file's OpenGL shader code into compiled Direct3D HLSL binary.\n")
	print("\n")
	print("Usage:  " .. arg[0] .. " SourceFilePath TargetFilePath IntermediatePath GlslConverterFilePath\n")
	print("  SourceFilePath         Path to the \"kernel_*.lua\" file to be converted.\n")
	print("  TargetFilePath         The new kernel file to be created containing compiled HLSL.\n")
	print("  IntermediatePath       Directory where temporary files can be created for the conversion process.\n")
	print("  GlslConverterFilePath  Path to the Angle Project's OpenGL shader converter tool.\n")
	os.exit(-1)
end

-- Load the given Lua source file.
luaChunk, errorMessage = loadfile(sourceFilePath)
if (not luaChunk) then
	print("Failed to load file: " .. tostring(sourceFilePath) .. "\n")
	if (type(errorMessage) == "string") and (string.len(errorMessage) > 0) then
		print("Reason:\n" .. errorMessage)
	end
	os.exit(-1)
end

-- Fetch the source file's "kernel" table.
local kernel = luaChunk()
if (type(kernel) ~= "table") then
	print("Failed to obtain \"kernel\" table from file: " .. tostring(sourceFilePath))
	os.exit(-1)
end

-- Do not continue if the given Lua script does not contain any shaders.
-- This is typically the case for "kernel" scripts that contain a "graph" table.
if (type(kernel.fragment) ~= "string") and (type(kernel.vertex) ~= "string") then
	os.exit(1)		-- Exit code 1 to indicate that the given shader script cannot be converted.
end

-- Do not continue if the given Lua script's shader is not supported on Windows Phone.
-- This is typically the case for shaders that cannot be converted/compiled to HLSL for this platform.
if (type(kernel.unsupportedPlatforms) == "table") and kernel.unsupportedPlatforms.WinPhone then
	os.exit(1)		-- Exit code 1 to indicate that the given shader script cannot be converted.
end

-- If the given Lua script is missing a fragment or vertex shader, then use the default kernel file's shader.
local hasFragmentShader = ((type(kernel.fragment) == "string") and (string.len(kernel.fragment) > 0))
local hasVertexShader = ((type(kernel.vertex) == "string") and (string.len(kernel.vertex) > 0))
if (not hasFragmentShader) or (not hasVertexShader) then
	local defaultKernelFilePath = string.match(sourceFilePath, "(.-)([^\\/]-%.?([^%.\\/]*))$") .. "kernel_default_gl.lua"
	local luaChunk, errorMessage = loadfile(defaultKernelFilePath)
	if luaChunk then
		local defaultKernel = luaChunk()
		if (type(defaultKernel) == "table") then
			if not hasFragmentShader then
				kernel.fragment = defaultKernel.fragment
			end
			if not hasVertexShader then
				kernel.vertex = defaultKernel.vertex
			end
		end
	end
end

-- Extract the source file name, stripping out the path and extension.
local function getFileNameWithoutExtensionFrom(filePath)
	local wasExtensionFound = false
	local lastCharIndex = string.len(filePath)
	local firstCharIndex = 1
	for index = lastCharIndex, 1, -1 do
		local nextChar = string.sub(filePath, index, index)
		if (not wasExtensionFound) and ("." == nextChar) then
			lastCharIndex = index - 1
			wasExtensionFound = true
		elseif ("\\" == nextChar) or ("/" == nextChar) then
			firstCharIndex = index + 1
			break
		end
	end
	if (firstCharIndex > lastCharIndex) then
		firstCharIndex = lastCharIndex
	end
	return string.sub(filePath, firstCharIndex, lastCharIndex)
end
local sourceFileNameWithoutExtension = getFileNameWithoutExtensionFrom(sourceFilePath)

-- Load the default shell script and obtain its "shell" table.
-- The shell shader script needs to be injected above the given kernel's shader script.
local shellFilePath = string.match(sourceFilePath, "(.-)([^\\/]-%.?([^%.\\/]*))$") .. "shell_default_gl.lua"
local luaChunk, errorMessage = loadfile(shellFilePath)
if (not luaChunk) then
	print("Failed to load file: " .. tostring(shellFilePath) .. "\n")
	if (type(errorMessage) == "string") and (string.len(errorMessage) > 0) then
		print("Reason:\n" .. errorMessage)
	end
	os.exit(-1)
end
local shell = luaChunk()
if (type(shell) ~= "table") then
	print("Failed to obtain \"shell\" table from file: " .. tostring(shellFilePath))
	os.exit(-1)
end


-- Creates a fragment and vertex shader file for the given mode, version, and mask count,
-- and then converts and compiles the OpenGL shaders into a Direct3D HLSL *.cso binary.
-- @param shaderModeName Expected to be set to "default" or "mode25D".
-- @param shaderVersionName Expected to be set to "maskCount0", "maskCount1", "maskCount2", "maskCount3", or "wireframe".
-- @param maskCount Expected to be set to 0, 1, 2, or 3.
-- @returns Returns the compiled HLSL binary as a string.
local function createDirect3DShaderBinaryFor(shaderModeName, shaderVersionName, maskCount)
	local fragmentShaderFileHandle = nil
	local vertexShaderFileHandle = nil
	local compiledShaderFileHandle = nil
	local errorMessage = nil

	-- Validate arguments.
	if ("string" ~= type(shaderModeName)) or (string.len(shaderModeName) <= 0) then
		return nil
	end

	-- Ensure the given mask count is set to a valid value.
	if (type(maskCount) ~= "number") or (maskCount < 0) then
		maskCount = 0
	end

	-- Create the intermediate file path\names needed to compiled a Direct3D HLSL shader.
	local intermediateFilePathWithoutExtension =
				intermediatePath .. "\\" .. sourceFileNameWithoutExtension ..
				"_" .. shaderModeName .. "_" .. shaderVersionName
	local fragmentShaderFilePath = intermediateFilePathWithoutExtension .. ".frag"
	local vertexShaderFilePath = intermediateFilePathWithoutExtension .. ".vert"
	local compiledShaderFilePath = intermediateFilePathWithoutExtension .. ".cso"

	-- Create the shader header using the given mask count.
	local shaderHeaderFormat =
	[[
		#define P_DEFAULT	highp
		#define P_RANDOM	highp
		#define P_POSITION	mediump
		#define P_NORMAL	mediump
		#define P_UV		mediump
		#define P_COLOR		lowp
		#define FRAGMENT_SHADER_SUPPORTS_HIGHP	1
		#define MASK_COUNT	%d
	]]
	local shaderHeader = string.format(shaderHeaderFormat, maskCount)
	if ("mode25D" == shaderModeName) then
		shaderHeader = shaderHeader .. "#define TEX_COORD_Z 1\n"
	end
	
	-- Create the fragment shader file.
	fragmentShaderFileHandle, errorMessage = io.open(fragmentShaderFilePath, "wt+")
	if not fragmentShaderFileHandle then
		print("Failed to create file: " .. tostring(fragmentShaderFilePath) .. "\n")
		if (type(errorMessage) == "string") and (string.len(errorMessage) > 0) then
			print("Reason:\n" .. errorMessage)
		end
		os.exit(-1)
	end
	fragmentShaderFileHandle:write(shaderHeader)
	if ("wireframe" == shaderVersionName) then
		local wireframeSourceCode =
		[[
			void main()
			{
				gl_FragColor = vec4(1.0);
			}
		]]
		fragmentShaderFileHandle:write(wireframeSourceCode)
	else
		fragmentShaderFileHandle:write(shell.fragment)
		if (type(kernel.fragment) == "string") then
			fragmentShaderFileHandle:write(kernel.fragment)
		end
	end
	io.close(fragmentShaderFileHandle)
	fragmentShaderFileHandle = nil

	-- Create the vertex shader file.
	vertexShaderFileHandle, errorMessage = io.open(vertexShaderFilePath, "wt+")
	if not vertexShaderFileHandle then
		print("Failed to create file: " .. tostring(vertexShaderFilePath) .. "\n")
		if (type(errorMessage) == "string") and (string.len(errorMessage) > 0) then
			print("Reason:\n" .. errorMessage)
		end
		os.exit(-1)
	end
	vertexShaderFileHandle:write(shaderHeader)
	vertexShaderFileHandle:write(shell.vertex)
	if (type(kernel.vertex) == "string") then
		vertexShaderFileHandle:write(kernel.vertex)
	end
	io.close(vertexShaderFileHandle)
	vertexShaderFileHandle = nil

	-- Compile the fragment and vertex shaders into a single binary file.
	local commandLine =
			'"' .. glslConverterFilePath .. '"' ..
			' -p=wp8' ..
			' -f="' .. fragmentShaderFilePath .. '"' ..
			' -v="' .. vertexShaderFilePath .. '"' ..
			' -o="' .. compiledShaderFilePath .. '"'
	local exitCode = os.execute('"' .. commandLine .. '"')
	if (exitCode ~= 0) then
		print("Direct3D shader compiler failed with exit code (" .. tostring(exitCode) .. ") for file:\n")
		print(sourceFilePath)
		os.exit(-1)
	end

	-- Fetch the compiled shader file's binary as a string.
	compiledShaderFileHandle, errorMessage = io.open(compiledShaderFilePath, "rb")
	if not compiledShaderFileHandle then
		print("Failed to create compiled shader file: " .. tostring(compiledShaderFilePath) .. "\n")
		os.exit(-1)
	end
	local binary = compiledShaderFileHandle:read("*a")
	io.close(compiledShaderFileHandle)
	
	-- Return the binary.
	return binary
end

-- Create a table for storing all compiled shader entries that Corona requires.
local compiledShadersTable =
{
	default =
	{
		maskCount0 = { maskCount = 0 },
		maskCount1 = { maskCount = 1 },
		maskCount2 = { maskCount = 2 },
		maskCount3 = { maskCount = 3 },
		wireframe = { maskCount = 0 },
	},
	mode25D =
	{
		maskCount0 = { maskCount = 0 },
		maskCount1 = { maskCount = 1 },
		maskCount2 = { maskCount = 2 },
		maskCount3 = { maskCount = 3 },
		wireframe = { maskCount = 0 },
	},
}

-- Compile a Direct3D HLSL shader for every shader mode/version permutation.
-- Note: The below compile function will call os.exit() if a shader compile error occurs.
for shaderModeName, shaderModeEntry in pairs(compiledShadersTable) do
	for shaderVersionName, shaderVersionEntry in pairs(shaderModeEntry) do
		shaderVersionEntry.binary = createDirect3DShaderBinaryFor(
											shaderModeName, shaderVersionName, shaderVersionEntry.maskCount)
	end
end

-- Open the original kernel Lua file.
-- We need to copy its contents to the new Lua file below.
local sourceFileHandle = nil
sourceFileHandle, errorMessage = io.open(sourceFilePath, "rt")
if not sourceFileHandle then
	print("Failed to open file: " .. tostring(sourceFilePath) .. "\n")
	if (type(errorMessage) == "string") and (string.len(errorMessage) > 0) then
		print("Reason:\n" .. errorMessage)
	end
	os.exit(-1)
end

-- Create a new kernel Lua file that will contain the compiled HLSL shader binaries.
local targetFileHandle = nil
targetFileHandle, errorMessage = io.open(targetFilePath, "wt+")
if not targetFileHandle then
	print("Failed to create file: " .. tostring(targetFilePath) .. "\n")
	if (type(errorMessage) == "string") and (string.len(errorMessage) > 0) then
		print("Reason:\n" .. errorMessage)
	end
	os.exit(-1)
end

-- Generate the Lua script code.
targetFileHandle:write("local function createKernel()\n\n")
for nextLine in sourceFileHandle:lines() do
	targetFileHandle:write("\t")
	targetFileHandle:write(nextLine)
	targetFileHandle:write("\n")
end
targetFileHandle:write("end\n\n")
targetFileHandle:write("local kernel = createKernel()\n\n")
targetFileHandle:write("kernel.compiledShaders =\n")
targetFileHandle:write("{\n")
for shaderModeName, shaderModeEntry in pairs(compiledShadersTable) do
	targetFileHandle:write("\t" .. shaderModeName .. " =\n")
	targetFileHandle:write("\t{\n")
	for shaderVersionName, shaderVersionEntry in pairs(shaderModeEntry) do
		local binary = shaderVersionEntry.binary
		if (type(binary) == "string") then
			targetFileHandle:write("\t\t" .. shaderVersionName .. " = \"")
			for byteIndex = 1, #binary do
				targetFileHandle:write(string.format("%02x", string.byte(binary, byteIndex)))
			end
			targetFileHandle:write("\",\n")
		end
	end
	targetFileHandle:write("\t},\n")
end
targetFileHandle:write("}\n")
targetFileHandle:write("return kernel\n")

-- Close all files.
io.close(sourceFileHandle)
io.close(targetFileHandle)

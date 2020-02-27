local dstDir, dstName, pluginsDir = ...

local builder = require "Builder"

local libtemplateDir = pluginsDir .. "/libtemplate"

-- Build
local options =
{
	dstPath = dstDir .. '/' .. dstName,
	dstDir = dstDir,
	librarySearchPaths = { libtemplateDir, },
	pluginsDir = pluginsDir,
	--verbose = true,
}

return builder:build( options )

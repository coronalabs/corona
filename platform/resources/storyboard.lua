-----------------------------------------------------------------------------------------
--
-- storyboard.lua
--
-----------------------------------------------------------------------------------------
local storyboard = {}

-----------------------------------------------------------------------------------------

local stage = display.newGroup()	-- top-level group which scene views are inserted
local currentModule 	-- reference to currently loaded scene's module name (string)
local currentScene		-- reference to currently loaded scene (display group)
local previousScene		-- string that keeps track of the previously loaded scene
local currentOverlay    -- reference to currently shown overlay/popup scene
local touchOverlay		-- forward declaration for touch-disabling overlay (created once)
local modalRect  -- forward declaration for touch-disabling rect that goes behind overlay scenes
storyboard.loadedSceneMods = {}	-- this will hold a history of most recently used scenes
storyboard.scenes = {}		-- table to replace use of package.loaded
storyboard.stage = stage 	-- allows external access to storyboard's display group
storyboard.disableAutoPurge = false -- if true, no scenes will auto-purge on low memory
storyboard.purgeOnSceneChange = false -- if true, will automatically purge non-active scenes on scene change
storyboard.isDebug = false	-- if true, will print useful info to the terminal in some situations

--Localize variables to prevent table lookups
local _tonumber = tonumber
local _pairs = pairs
local _ioOpen = io.open
local _ioClose = io.close
local _ioWrite = io.write
local _toString = tostring
local _stringSub = string.sub
local _stringFind = string.find
local _type = type
local _collectGarbage = collectgarbage
local _stringFormat = string.format
local _getInfo = system.getInfo
local displayW = display.contentWidth
local displayH = display.contentHeight

-----------------------------------------------------------------------------------------

-- TRANSITION EFFECTS

local effectList = {
	["fade"] =
	{
		["from"] =
		{
			alphaStart = 1.0,
			alphaEnd = 0,
		},

		["to"] =
		{
			alphaStart = 0,
			alphaEnd = 1.0
		}
	},
	
	["zoomOutIn"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			yEnd = displayH*0.5,
			xScaleEnd = 0.001,
			yScaleEnd = 0.001
		},

		["to"] =
		{
			xScaleStart = 0.001,
			yScaleStart = 0.001,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = displayW*0.5,
			yStart = displayH*0.5,
			xEnd = 0,
			yEnd = 0
		},
		hideOnOut = true
	},
	
	["zoomOutInFade"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			yEnd = displayH*0.5,
			xScaleEnd = 0.001,
			yScaleEnd = 0.001,
			alphaStart = 1.0,
			alphaEnd = 0
		},

		["to"] =
		{
			xScaleStart = 0.001,
			yScaleStart = 0.001,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = displayW*0.5,
			yStart = displayH*0.5,
			xEnd = 0,
			yEnd = 0,
			alphaStart = 0,
			alphaEnd = 1.0
		},
		hideOnOut = true
	},
	
	["zoomInOut"] =
	{
		["from"] =
		{
			xEnd = -displayW*0.5,
			yEnd = -displayH*0.5,
			xScaleEnd = 2.0,
			yScaleEnd = 2.0
		},

		["to"] =
		{
			xScaleStart = 2.0,
			yScaleStart = 2.0,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = -displayW*0.5,
			yStart = -displayH*0.5,
			xEnd = 0,
			yEnd = 0
		},
		hideOnOut = true
	},
	
	["zoomInOutFade"] =
	{
		["from"] =
		{
			xEnd = -displayW*0.5,
			yEnd = -displayH*0.5,
			xScaleEnd = 2.0,
			yScaleEnd = 2.0,
			alphaStart = 1.0,
			alphaEnd = 0
		},

		["to"] =
		{
			xScaleStart = 2.0,
			yScaleStart = 2.0,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = -displayW*0.5,
			yStart = -displayH*0.5,
			xEnd = 0,
			yEnd = 0,
			alphaStart = 0,
			alphaEnd = 1.0
		},
		hideOnOut = true
	},
	
	["flip"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			xScaleEnd = 0.001
		},

		["to"] =
		{
			xScaleStart = 0.001,
			xScaleEnd = 1.0,
			xStart = displayW*0.5,
			xEnd = 0
		}
	},
	
	["flipFadeOutIn"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			xScaleEnd = 0.001,
			alphaStart = 1.0,
			alphaEnd = 0
		},

		["to"] =
		{
			xScaleStart = 0.001,
			xScaleEnd = 1.0,
			xStart = displayW*0.5,
			xEnd = 0,
			alphaStart = 0,
			alphaEnd = 1.0
		}
	},
	
	["zoomOutInRotate"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			yEnd = displayH*0.5,
			xScaleEnd = 0.001,
			yScaleEnd = 0.001,
			rotationStart = 0,
			rotationEnd = -360
		},

		["to"] =
		{
			xScaleStart = 0.001,
			yScaleStart = 0.001,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = displayW*0.5,
			yStart = displayH*0.5,
			xEnd = 0,
			yEnd = 0,
			rotationStart = -360,
			rotationEnd = 0
		},
		hideOnOut = true
	},
	
	["zoomOutInFadeRotate"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			yEnd = displayH*0.5,
			xScaleEnd = 0.001,
			yScaleEnd = 0.001,
			rotationStart = 0,
			rotationEnd = -360,
			alphaStart = 1.0,
			alphaEnd = 0
		},

		["to"] =
		{
			xScaleStart = 0.001,
			yScaleStart = 0.001,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = displayW*0.5,
			yStart = displayH*0.5,
			xEnd = 0,
			yEnd = 0,
			rotationStart = -360,
			rotationEnd = 0,
			alphaStart = 0,
			alphaEnd = 1.0
		},
		hideOnOut = true
	},
	
	["zoomInOutRotate"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			yEnd = displayH*0.5,
			xScaleEnd = 2.0,
			yScaleEnd = 2.0,
			rotationStart = 0,
			rotationEnd = -360
		},

		["to"] =
		{
			xScaleStart = 2.0,
			yScaleStart = 2.0,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = displayW*0.5,
			yStart = displayH*0.5,
			xEnd = 0,
			yEnd = 0,
			rotationStart = -360,
			rotationEnd = 0
		},
		hideOnOut = true
	},
	
	["zoomInOutFadeRotate"] =
	{
		["from"] =
		{
			xEnd = displayW*0.5,
			yEnd = displayH*0.5,
			xScaleEnd = 2.0,
			yScaleEnd = 2.0,
			rotationStart = 0,
			rotationEnd = -360,
			alphaStart = 1.0,
			alphaEnd = 0
		},

		["to"] =
		{
			xScaleStart = 2.0,
			yScaleStart = 2.0,
			xScaleEnd = 1.0,
			yScaleEnd = 1.0,
			xStart = displayW*0.5,
			yStart = displayH*0.5,
			xEnd = 0,
			yEnd = 0,
			rotationStart = -360,
			rotationEnd = 0,
			alphaStart = 0,
			alphaEnd = 1.0
		},
		hideOnOut = true
	},
	
	["fromRight"] =
	{
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = displayW,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true,
		sceneAbove = true
	},
	
	["fromLeft"] =
	{
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = -displayW,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true,
		sceneAbove = true
	},
	
	["fromTop"] =
	{
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = 0,
			yStart = -displayH,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true,
		sceneAbove = true
	},
	
	["fromBottom"] =
	{
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = 0,
			yStart = displayH,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true,
		sceneAbove = true
	},
	
	["slideLeft"] =
	{
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = -displayW,
			yEnd = 0,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = displayW,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true
	},
	
	["slideRight"] =
	{
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = displayW,
			yEnd = 0,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = -displayW,
			yStart = 0,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true
	},
	
	["slideDown"] =
	{ 
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = 0,
			yEnd = displayH,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = 0,
			yStart = -displayH,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true
	},
	
	["slideUp"] =
	{
		["from"] =
		{
			xStart = 0,
			yStart = 0,
			xEnd = 0,
			yEnd = -displayH,
			transition = easing.outQuad
		},

		["to"] =
		{
			xStart = 0,
			yStart = displayH,
			xEnd = 0,
			yEnd = 0,
			transition = easing.outQuad
		},
		concurrent = true
	},
	
	["crossFade"] =
	{
		["from"] =
		{
			alphaStart = 1.0,
			alphaEnd = 0,
		},

		["to"] =
		{
			alphaStart = 0,
			alphaEnd = 1.0
		},
		concurrent = true
	}
}
storyboard.effectList = effectList

-----------------------------------------------------------------------------------------

local function debug_print( ... )
	_ioWrite( "STORYBOARD > " )
	print( ... )
	print( "" )
end

-----------------------------------------------------------------------------------------

local function findSceneIndex( sceneName )
	for i=1,#storyboard.loadedSceneMods do
		if storyboard.loadedSceneMods[i] == sceneName then
			return i
		end
	end
end

-----------------------------------------------------------------------------------------

local function removeFromSceneHistory( sceneName )
	local index = findSceneIndex( sceneName )
	if index then
		table.remove( storyboard.loadedSceneMods, index )
	end
end

-----------------------------------------------------------------------------------------

local function addToSceneHistory( sceneName )
	removeFromSceneHistory( sceneName )
	storyboard.loadedSceneMods[#storyboard.loadedSceneMods+1] = sceneName
end

-----------------------------------------------------------------------------------------

local function saveSceneAndHide( currentScene, newModule, noEffect )
	if not currentScene then return; end
    local screenshot
    if currentScene and currentScene.numChildren and currentScene.numChildren > 0 and not noEffect then
        --screenshot = display.capture( currentScene )
        screenshot = currentScene
    elseif noEffect and currentScene then
    	currentScene.isVisible = false
    end
	
	-- Since display.capture() only captures the group as far as content width/height,
	-- we must make calculations to account for groups that are both less than the total width/height
	-- of the screen, as well as groups that are offset have elements that are not on the screen:
	local bounds = currentScene.contentBounds
	local xMin, xMax = bounds.xMin, bounds.xMax
	local yMin, yMax = bounds.yMin, bounds.yMax

	local objectsOutsideLeft = xMin < display.screenOriginX
	local objectsOutsideRight = xMax > displayW+(-display.screenOriginX)
	local objectsAboveTop = yMin < display.screenOriginY
	local objectsBelowBottom = yMax > displayH+(-display.screenOriginY)
	
	-- Calculate xMin and xMax
	if xMin < 0 then xMin = 0; end
	if xMax > displayW then
		xMax = displayW
	end
	
	-- Caluclate yMin and yMax
	if yMin < 0 then yMin = 0; end
	if yMax > displayH then
		yMax = displayH
	end

	-- Calculate actual width/height of screen capture
	local width = xMax - xMin
	local height = yMax - yMin
	
	-- loop through current scene and remove potential Runtime table listeners
	for i=currentScene.numChildren,1,-1 do
		if currentScene[i].enterFrame then Runtime:removeEventListener( "enterFrame", currentScene[i] ); end
	end
	
	-- dispatch current scene's exitScene event
	if currentModule and storyboard.scenes[currentModule] then
		local event = {}
		event.name = "exitScene"
		storyboard.scenes[currentModule]:dispatchEvent( event )
	end
	
	-- set new currentModule
	currentModule = newModule

	-- display screenshot of previous scene
    if screenshot then
        stage:insert( screenshot )
        return screenshot
    end
end

-----------------------------------------------------------------------------------------

local function createTouchOverlay()
	local function disableTouches(event) return true; end	-- don't allow touches on scene

	local overlayRect = display.newRect( 0, 0, displayW, displayH )
	overlayRect:setFillColor( 0 )
	overlayRect.isVisible = false
	overlayRect.isHitTestable = true	-- allow touches when invisible
	overlayRect:addEventListener( "touch", disableTouches )
	overlayRect:addEventListener( "tap", disableTouches )
	
	return overlayRect
end

-----------------------------------------------------------------------------------------

function storyboard.removeScene( sceneName )
	-- remove scene, its display group, and global reference in storyboard.scenes table
	storyboard.purgeScene( sceneName )
	
	-- remove global reference
	storyboard.scenes[sceneName] = nil
	package.loaded[sceneName] = nil
end

-----------------------------------------------------------------------------------------

function storyboard.removeAll()
	storyboard.hideOverlay()

	-- removes all scenes (except for the one that is currently showing)
	for i=#storyboard.loadedSceneMods,1,-1 do
		local sceneToUnload = storyboard.loadedSceneMods[i]
		
		if sceneToUnload ~= currentModule then
			storyboard.removeScene( sceneToUnload )
		end
	end
end

-----------------------------------------------------------------------------------------

function storyboard.purgeScene( sceneName )
	-- Unload a scene and remove its display group
	-- NOTE: global reference in storyboard.scenes is kept
	local scene = storyboard.scenes[sceneName]
	if scene and scene.view then
		local event = {}
		event.name = "destroyScene"
		scene:dispatchEvent( event )
		
		-- remove module from scene history table
		removeFromSceneHistory( sceneName )
		
		if scene.view then
			display.remove( scene.view )
			scene.view = nil
			_collectGarbage( "collect" )
		end
	elseif storyboard.isDebug then
		if not scene then
			debug_print( sceneName .. " was not purged because it does not exist. Use storyboard.loadScene() or storyboard.gotoScene()." )
		elseif scene and not scene.view then
			debug_print( sceneName .. " was not purged because it's view (display group) does not exist. This means it has already been purged or the view was never created." )
		end
	end
end

-----------------------------------------------------------------------------------------

function storyboard.purgeAll()
	local purge_count = 0

	-- Purges all scenes (except for the one that is currently showing)
	for i=#storyboard.loadedSceneMods,1,-1 do
		local sceneToUnload = storyboard.loadedSceneMods[i]
		
		if sceneToUnload ~= currentModule then
			purge_count = purge_count + 1
			storyboard.purgeScene( sceneToUnload )
		end
	end

	if storyboard.isDebug then
		local msg = "A total of [" .. purge_count .. "] scene(s) have been purged."
		if purge_count == 0 then
			msg = "No scenes were purged."
		end
		debug_print( msg )
	end
end

-----------------------------------------------------------------------------------------

function storyboard.getPrevious()
	-- Returns the name (string) of the previous scene (or nil if active scene is first)
	return previousScene
end

-----------------------------------------------------------------------------------------

function storyboard.getScene( sceneName )
	-- Returns a reference (scene object) to the specified sceneName
	local scene = storyboard.scenes[sceneName]

	if storyboard.isDebug and not scene then
		debug_print( "The specified scene, " .. sceneName .. ", does not exist." )
	end

	return scene
end

-----------------------------------------------------------------------------------------

function storyboard.getCurrentSceneName()
	return currentModule
end

-----------------------------------------------------------------------------------------

local function loadObjects( self, options )  -- scene method
	local options = options or {}
	local file = options.file
	local baseDirectory = options.baseDir or system.ResourceDirectory
	local assetDirectory = options.assetDir or "assets"
	local physicsDisabled = options.excludePhysics
	local listener = options.listener

	if not file then
		error( "You must specify a scene data file via the 'file' option when calling scene:createViewFromData()." )
	end

	if listener then
		self:addEventListener( "createObject", listener )
	end

	local physics
	if not physicsDisabled then
		physics = physics or require "physics"
		physics.start()
		physics.pause()
	end

	-- load from scene data file
	local json = require "json"
	local _jsonDecode = json.decode
	local sceneDataFile = file
	local sceneData

	-- strip slashes from assetDirectory string (e.g. "/assets", "/assets/", or "assets/" becomes just "assets")
	if _stringSub( assetDirectory, 1, 1 ) == "/" then
		assetDirectory = _stringSub( assetDirectory, 2 )
	end
	if _stringSub( assetDirectory, -1 ) == "/" then
		assetDirectory = _stringSub( assetDirectory, 1, #assetDirectory-1 )
	end

	-- load scene data from file; or from storyboard.loadedSceneDataFiles table (if file was previously loaded)
	if (not storyboard.loadedSceneDataFiles) or (not storyboard.loadedSceneDataFiles[sceneDataFile]) then
		storyboard.loadedSceneDataFiles = {}

		-- load external scene data file
		local path = system.pathForFile( sceneDataFile, baseDirectory )
		local fh = _ioOpen( path, "r" )
		
		if fh then
			sceneData = _jsonDecode( fh:read("*a") )
			_ioClose( fh ); fh = nil
			storyboard.loadedSceneDataFiles[sceneDataFile] = sceneData 	-- storing loaded table into storyboard.loadedSceneDataFiles (to prevent unnecessary filesystem access for future loads)
		end
	else
		sceneData = storyboard.loadedSceneDataFiles[sceneDataFile]
	end

	if sceneData and sceneData.objects then
		self.objects = {}  -- table to get specific objects by id (e.g. local obj = scene.objects["Object-Id"]; )

		-- creates group/object hierarchy based on table structure
		local function createGroupStructure( groupTable, isTopLevel )
			local g = self.view
			if not isTopLevel then g =display.newGroup(); end

			if groupTable.children then
				for i=1,#groupTable.children do
					local o = groupTable.children[i]
					local obj

					-- setup event table w/ keys common to all objects
					local event = {}
					event.name = "createObject"
					event.phase = "began"
					event.objectId = o.id
					event.objectProperties = o

					local function add_physics( o, obj )
						if o.physicsEnabled and not physicsDisabled then
							local bodyShape, radius
							
							if o.radius and o.radius ~= 0 then
								radius = o.radius
							else
								-- construct physics bodyShape from vector points
								if o.bodyShape then
									bodyShape = {}
									for i=1,#o.bodyShape do
										bodyShape[#bodyShape+1] = o.bodyShape[i].x * o.xScale
										bodyShape[#bodyShape+1] = o.bodyShape[i].y * o.yScale
									end
								end
							end
							physics.addBody( obj, o.bodyType, { bounce=o.bounce, density=o.density, friction=o.friction, shape=bodyShape, radius=radius } )
						end
					end
					
					-- create objects, depending on their type
					if o.isGroup then

						-- GROUPS

						-- dispatch "createObject" event with "began" phase
						event.objectIsGroup = true
						obj = self:dispatchEvent( event )	-- user's event listener should return a display object (group in this case)
						
						if not obj or (obj.numChildren ~= nil) then  -- returned object must be a group
							obj = createGroupStructure( o )
						end
						
						-- add physics
						add_physics( o, obj )
					
					elseif o.isText then
						
						-- TEXT OBJECTS

						if o.font == "native.systemFont" then o.font = native.systemFont; end
						if o.font == "native.systemFontBold" then o.font = native.systemFontBold; end
						
						-- dispatch "createObject" event ("began" phase)
						-- potential uses: override object creation to replace object with sprite object, for instance
						event.objectIsText = true
						obj = self:dispatchEvent( event )	-- user's event listener should return a display object

						if not obj then
							obj = display.newText( o.text, 0, 0, o.font, o.size )
							obj:setTextColor( o.textColor.r, o.textColor.g, o.textColor.b )
						end
						obj:setReferencePoint( display.CenterReferencePoint )
						
						-- set object properties
						for k,v in _pairs( o ) do
							if k ~= "text" and k ~= "size" then	-- properties already set
								obj[k] = v
							end
						end

					elseif o.isRect then

						-- RECTANGLE OBJECTS

						-- dispatch "createObject" event ("began" phase)
						-- potential uses: override object creation to replace object with sprite object, for instance
						event.objectIsRect = true
						obj = self:dispatchEvent( event )	-- user's event listener should return a display object

						if not obj then
							obj = display.newRect( 0, 0, o.rectWidth, o.rectHeight )
							obj:setFillColor( o.fillColor.r, o.fillColor.g, o.fillColor.b, 255 )
							obj:setStrokeColor( o.strokeColor.r, o.strokeColor.g, o.strokeColor.b, 255 )
						end

						-- add physics
						add_physics( o, obj )

						-- set object properties
						for k,v in _pairs( o ) do
							if k ~= "fillColor" and k ~= "strokeColor" and k ~= "rectWidth" and k ~= "rectHeight" then	-- properties already set
								obj[k] = v
							end
						end

					elseif o.isCircle then

						-- CIRCLE OBJECTS

						-- dispatch "createObject" event ("began" phase)
						-- potential uses: override object creation to replace object with sprite object, for instance
						event.objectIsCircle = true
						obj = self:dispatchEvent( event )	-- user's event listener should return a display object

						if not obj then
							obj = display.newCircle( 0, 0, o.circleRadius )
							obj:setFillColor( o.fillColor.r, o.fillColor.g, o.fillColor.b, 255 )
							obj:setStrokeColor( o.strokeColor.r, o.strokeColor.g, o.strokeColor.b, 255 )
						end

						-- add physics
						add_physics( o, obj )

						-- set object properties
						for k,v in _pairs( o ) do
							if k ~= "fillColor" and k ~= "strokeColor" and k ~= "circleRadius" then	-- properties already set
								obj[k] = v
							end
						end

					elseif o.isLine then

						-- LINE OBJECTS

						-- dispatch "createObject" event ("began" phase)
						-- potential uses: override object creation to replace object with sprite object, for instance
						event.objectIsLine = true
						obj = self:dispatchEvent( event )	-- user's event listener should return a display object

						if not obj then
							obj = display.newLine( o.x1, o.y1, o.x2, o.y2 )
							obj:setColor( o.lineColor[1], o.lineColor[2], o.lineColor[3], o.lineColor[4] )
							obj.width = o.lineWidth
						end

						-- set object properties
						for k,v in _pairs( o ) do
							if k ~= "lineColor" and k ~= "lineWidth" and k ~= "contentWidth" and k ~= "contentHeight" then	-- properties already set
								obj[k] = v
							end
						end

						obj:translate( -o.contentWidth*0.5, -o.contentHeight*0.5 )
					else
					
						-- NORMAL DISPLAY OBJECTS (non-groups and non-text)
						
						local imageFile = o.imageFile
						local starti, endi = _stringFind( imageFile, "assets/" )
						imageFile = assetDirectory .. "/" .. (_stringSub( imageFile, endi+1 ))

						-- dispatch "createObject" event with "began" phase
						-- potential uses: override object creation to replace object with sprite object, for instance
						obj = self:dispatchEvent( event )	-- user's event listener should return a display object

						if not obj then
							obj = display.newImage( imageFile, baseDirectory )
						end
						obj:setReferencePoint( display.CenterReferencePoint )
						
						-- add physics
						add_physics( o, obj )
						
						-- set object properties
						for k,v in _pairs( o ) do
							obj[k] = v
						end
					end

					-- insert object into parent group (and create a reference to it in 'objects' table)
					self.objects[obj.id] = obj
					g:insert( obj )

					-- modify removeSelf() method of obj to completely remove object from parent group
					if obj and not obj.isGroup then
						local cached_removeSelf = obj.removeSelf
						local scene = self
						function obj:removeSelf()
							local id = self.id
							cached_removeSelf( self )
							scene.objects[id] = nil 	-- this is the important step; which is why sub-classing 'removeSelf' is necessary
						end

						-- dispatch "createObject" event with "ended" phase
						if not o.isGroup then
							event.object = obj  -- ended phase provides direct reference to created object
							event.phase = "ended"
							self:dispatchEvent( event )
						end

					elseif obj and obj.isGroup then

						-- Groups will remove their children individually first (to hit above block for each child object)

						local cached_removeSelf = obj.removeSelf
						local scene = self
						function obj:removeSelf()
							if self.numChildren then
								for i=self.numChildren,1,-1 do
									self[i]:removeSelf()
								end
							end
							local id = self.id
							cached_removeSelf( self )
							scene.objects[id] = nil 	-- this is the important step; which is why sub-classing 'removeSelf' is necessary
						end
					end
				end
			end
			
			-- set group properties
			for k,v in _pairs( groupTable ) do
				if k ~= "children" then
					g[k] = v
				end
			end

			-- dispatch "createObject" event with "ended" phase
			local groupEvent = {}
			groupEvent.name = "createObject"
			groupEvent.phase = "ended"
			groupEvent.objectId = g.id or "topLevelGroup"
			groupEvent.objectIsGroup = true
			groupEvent.object = g  -- ended phase provides direct reference to created object
			self:dispatchEvent( groupEvent )
			
			return g
		end

		createGroupStructure( sceneData.objects, true )
		--self.properties = sceneData.scene      -- table to hold scene-specific properties (set in Corona Levels)
	else
		error( "The specified scene data file, " .. sceneDataFile .. ", does not exist." )
	end
	self.name = sceneData.scene.name

	-- set physics gravity based on settings from data file
	if not physicsDisabled then
		local xGravity, yGravity = sceneData.scene.xGravity or 0, sceneData.scene.yGravity or 9.8
		physics.setGravity( xGravity, yGravity )
	end

	-- set background color based on settings from data file
	if sceneData.scene.bgColor and sceneData.scene.bgColor.r and sceneData.scene.bgColor.g and sceneData.scene.bgColor.b then
		local bg = sceneData.scene.bgColor
		display.setDefault( "background", bg.r, bg.g, bg.b )
	end
end

-----------------------------------------------------------------------------------------

function storyboard.newScene( sceneName )
	-- sceneName is optional if they don't want to use external module
	local s = Runtime._super:new()	-- TODO: Get real event listener class (we're cheating by using this)
	
	if sceneName and not storyboard.scenes[sceneName] then
		storyboard.scenes[sceneName] = s
	end

	-- method to load scene object from data file (generated via Corona Levels app)
	s.loadObjects = loadObjects

	return s
end

-----------------------------------------------------------------------------------------

local function nextTransition( sceneGroup, fx, effectTime, touchOverlay, oldScreenshot, customParams )
	
	-- remove touch disabling overlay rectangle:
	local disableOverlay = function()
		touchOverlay.isHitTestable = false	-- disable touches when invisible
		--display.remove( oldScreenshot ); oldScreenshot = nil
		if oldScreenshot then oldScreenshot.isVisible = false; end

		-- dispatch previous scene's didExitScene event
		local previous = storyboard.getPrevious()
		if previous and storyboard.scenes[previous] then
			local event = {}
			event.name = "didExitScene"
			storyboard.scenes[previous]:dispatchEvent( event )
		end
		
		-- dispatch scene's enterScene event
		if currentModule and storyboard.scenes[currentModule] then
			addToSceneHistory( currentModule )
			local event = {}
			event.name = "enterScene"
			event.params = customParams
			storyboard.scenes[currentModule]:dispatchEvent( event )

			if storyboard.purgeOnSceneChange then
				storyboard.purgeAll()
			end
		end
	end

	-- dispatch "willEnterScene" event
	if storyboard.scenes[currentModule] then
		local event = {}
		event.name = "willEnterScene"
		event.params = customParams
		storyboard.scenes[currentModule]:dispatchEvent( event )
	end
	
	local options = {}
	options.x = fx.to.xEnd
	options.y = fx.to.yEnd
	options.alpha = fx.to.alphaEnd
	options.xScale = fx.to.xScaleEnd
	options.yScale = fx.to.yScaleEnd
	options.rotation =  fx.to.rotationEnd
	options.time = effectTime or 500
	options.transition = fx.to.transition
	options.onComplete = disableOverlay
	
	if oldScreenshot and fx.hideOnOut then
		oldScreenshot.isVisible = false
	end
	sceneGroup.isVisible = true -- unhide next scene
	local sceneTransition = transition.to( sceneGroup, options )
end

-----------------------------------------------------------------------------------------

--
--
-- storyboard.hideOverlay()
-- This function will hide the currently displayed overlay (e.g. "pop up") scene.
-- Will also dispatch an "overlayEnded" event to currently active storyboard scene.
--

function storyboard.hideOverlay( purgeOnly, effect, effectTime, argOffset )
	display.remove( modalRect ); modalRect = nil

	if currentOverlay then
		-- auto-correct if colon syntax was used instead of dot syntax
		if purgeOnly and purgeOnly == storyboard then
			purgeOnly = effect
			effect = effectTime
			effectTime = argOffset

			if storyboard.isDebug then
				debug_print( "WARNING: You should use dot-syntax when calling storyboard functions. For example, storyboard.hideOverlay() instead of storyboard:hideOverlay()." )
			end
		end

		-- correct arguments
		if purgeOnly and _type(purgeOnly) == "string" then
			if effect then
				effectTime = effect
			end
			effect = purgeOnly
			purgeOnly = nil
		end

		local function dispatchSceneEvents()
			-- dispatch "exitScene" event on overlay scene before purge/removal
			local event = {}
			event.name = "didExitScene"
			currentOverlay:dispatchEvent( event )

			-- check to see if overlay scene is also being used as a normal scene (in which case we won't remove; only purge)
			local sceneExistsAsNormal = findSceneIndex( currentOverlay.name )
			if sceneExistsAsNormal then purgeOnly = true; end

			if purgeOnly then
				storyboard.purgeScene( currentOverlay.name )
			else
				storyboard.removeScene( currentOverlay.name )
			end

			-- on current scene (not overlay), dispatch "overlayEnded" event
			if currentModule then
				local current = storyboard.scenes[currentModule]
				local event = {}
				event.name = "overlayEnded"
				event.sceneName = currentOverlay.name
				current:dispatchEvent( event )
			end
			currentOverlay = nil
			touchOverlay.isHitTestable = false -- ensure touches are enabled
		end

		local event = {}
		event.name = "exitScene"
		currentOverlay:dispatchEvent( event )

		if effect and effectList[effect] then
			local fx = effectList[effect].from

			local function overlayTransitionComplete()
				dispatchSceneEvents()
			end

			-- set scene up according to effect (start)
			currentOverlay.view.x = fx.xStart or 0
			currentOverlay.view.y = fx.yStart or 0
			currentOverlay.view.alpha = fx.alphaStart or 1.0
			currentOverlay.view.xScale = fx.xScaleStart or 1.0
			currentOverlay.view.yScale = fx.yScaleStart or 1.0
			currentOverlay.view.rotation = fx.rotationStart or 0

			-- set transition options table up according to effect (end)
			local o = {}
			o.x = fx.xEnd
			o.y = fx.yEnd
			o.alpha = fx.alphaEnd
			o.xScale = fx.xScaleEnd
			o.yScale = fx.yScaleEnd
			o.rotation = fx.rotationEnd
			o.time = fxTime
			o.transition = fx.transition
			o.onComplete = overlayTransitionComplete

			local fxTransition = transition.to( currentOverlay.view, o ) 
		else
			dispatchSceneEvents()
		end
	end
end

-----------------------------------------------------------------------------------------

--
--
-- storyboard.showOverlay()
-- This function will "pop up" a scene and overlay it above current scene and
-- also disptach a "overlayBegan" event to currently active storyboard scene.
--
--

function storyboard.showOverlay( sceneName, options, argOffset )
	-- first, hide any overlay that may currently be showing
	storyboard.hideOverlay()

	-- auto-correct if colon syntax is used instead of dot
	if sceneName == storyboard then
		if options and _type(options) == "string" then
			sceneName = options
			if argOffset then options = argOffset; end
		end
	end

	-- parse arguments
	local options = options or {}
	local effect = options.effect
	local fxTime = options.time or 500
	local params = options.params  -- optional table user can pass to scene
	local isModal = options.isModal -- disables touches to calling scene (non-overlay, active scene)

	-- check to see if scene has already been loaded
	local scene = storyboard.scenes[sceneName]
	
	if scene then
		-- scene exists

		-- if view does not exist, create it and re-dispatch "createScene" event
		if not scene.view then
			scene.view = display.newGroup()
			local event = {}
			event.name = "createScene"
			event.params = params
			storyboard.scenes[sceneName]:dispatchEvent( event )
		end
	else
		storyboard.scenes[sceneName] = require( sceneName )
		scene = storyboard.scenes[sceneName]

		if _type(scene) == 'boolean' then
			error( "Attempting to load scene from invalid scene module (" .. sceneName .. ".lua). Did you forget to return the scene object at the end of the scene module? (e.g. 'return scene')" )
		end
		
		-- create the scene's view
		scene.view = scene.view or display.newGroup()
		local event = {}
		event.name = "createScene"
		event.params = params
		storyboard.scenes[sceneName]:dispatchEvent( event )
	end

	-- dispatch "willEnterScene" event
	local event = {}
	event.name = "willEnterScene"
	event.params = params
	scene:dispatchEvent( event )

	local function dispatchSceneEvents()
		-- dispatch "enterScene" event
		local event = {}
		event.name = "enterScene"
		event.params = params
		scene:dispatchEvent( event )

		-- dispatch "overlayBegan" event to current scene
		if currentModule then
			local current = storyboard.scenes[currentModule]
			local event = {}
			event.name = "overlayBegan"
			event.sceneName = sceneName  -- name of overlay scene
			event.params = params
			current:dispatchEvent( event )
		end

		touchOverlay.isHitTestable = false	-- re-enable touches
	end

	-- begin transition w/ or w/out effect
	if effect and effectList[effect] then
		local fx = effectList[effect].to
		touchOverlay.isHitTestable = true	-- disable touches during transition

		local function overlayTransitionComplete()
			dispatchSceneEvents( event )
		end

		-- set scene up according to effect (start)
		scene.view.x = fx.xStart or 0
		scene.view.y = fx.yStart or 0
		scene.view.alpha = fx.alphaStart or 1.0
		scene.view.xScale = fx.xScaleStart or 1.0
		scene.view.yScale = fx.yScaleStart or 1.0
		scene.view.rotation = fx.rotationStart or 0
		scene.view.isVisible = true

		-- set transition options table up according to effect (end)
		local o = {}
		o.x = fx.xEnd
		o.y = fx.yEnd
		o.alpha = fx.alphaEnd
		o.xScale = fx.xScaleEnd
		o.yScale = fx.yScaleEnd
		o.rotation = fx.rotationEnd
		o.time = fxTime
		o.transition = fx.transition
		o.onComplete = overlayTransitionComplete

		local fxTransition = transition.to( scene.view, o )
	else
		-- instant transition (no effect)
		touchOverlay.isHitTestable = false
		scene.isVisible = true
		scene.view.x, scene.view.y = 0, 0

		dispatchSceneEvents()
	end

	currentOverlay = scene
	currentOverlay.name = sceneName

	if isModal then
		modalRect = display.newRect( 0, 0, display.contentWidth, display.contentHeight )
		modalRect.isVisible = false
		modalRect.isHitTestable = true
		local function disableTouches() return true; end
		modalRect.touch = disableTouches
		modalRect.tap = disableTouches
		modalRect:addEventListener( "touch" )
		modalRect:addEventListener( "tap" )
		stage:insert( modalRect )
	end

	stage:insert( scene.view )	-- ensure the overlay scene is above current scene
end


-----------------------------------------------------------------------------------------

--
--
-- storyboard.reloadScene()
-- Same as calling storyboard.gotoScene() on current scene (effects are not available)
--
--

function storyboard.reloadScene()
	if not currentModule then return; end
	storyboard.hideOverlay()	-- hide any overlay/popup scenes that may be showing

	local scene = storyboard.getScene( currentModule )
	if not scene then return; end

	local function next_render( callback )
		return timer.performWithDelay( 1, callback, 1 )
	end

	local function dispatch_enterScene()
		scene:dispatchEvent( { name="enterScene" } )
	end
	
	local function dispatch_willEnterScene()
		scene:dispatchEvent( { name="willEnterScene" } )
		next_render( dispatch_enterScene )
	end

	local function dispatch_createScene()
		if not scene.view then
			scene.view = display.newGroup()
			scene:dispatchEvent( { name="createScene" } )
			currentScene = scene.view
			stage:insert( currentScene )
		end
		next_render( dispatch_willEnterScene )
	end

	local function dispatch_didExitScene()
		scene:dispatchEvent( { name="didExitScene" } )
		next_render( dispatch_createScene )
	end

	scene:dispatchEvent( { name="exitScene" } )
	next_render( dispatch_didExitScene )
end

-----------------------------------------------------------------------------------------

--
--
-- storyboard.loadScene()
-- Same as storyboard.gotoScene(), but no transition is initiated.
--
--

function storyboard.loadScene( sceneName, doNotLoadView, params )
	-- SYNTAX: storyboard.loadScene( sceneName [, doNotLoadView, params ] )	-- params is optional table w/ custom data

	-- check for dot syntax (to prevent errors)
	if sceneName == storyboard then
		error( "You must use a dot (instead of a colon) when calling storyboard.loadScene()" )
	end

	-- check to see if scene has already been loaded
	local scene = storyboard.scenes[sceneName]

	if doNotLoadView ~= nil and _type(doNotLoadView) ~= "boolean" then
		params = doNotLoadView
	end
	
	if scene then
		-- scene exists

		-- if view does not exist, create it and re-dispatch "createScene" event
		if not scene.view and not doNotLoadView then
			scene.view = display.newGroup()
			local event = {}
			event.name = "createScene"
			event.params = params
			storyboard.scenes[sceneName]:dispatchEvent( event )
			addToSceneHistory( sceneName )
		end
	else
		storyboard.scenes[sceneName] = require( sceneName )
		scene = storyboard.scenes[sceneName]
		
		-- scene's view will be created (default), unless user explicity
		-- tells it not to by setting doNotLoadView to true
		if not doNotLoadView then
			scene.view = scene.view or display.newGroup()
			local event = {}
			event.name = "createScene"
			event.params = params
			storyboard.scenes[sceneName]:dispatchEvent( event )
			addToSceneHistory( sceneName )
		end
	end

	if not doNotLoadView then
		scene.view.isVisible = false	-- ensure the view is invisible
		stage:insert( 1, scene.view )	-- insert this scene's view 'behind' the currently loaded scene
	end

	return scene
end

-----------------------------------------------------------------------------------------

function storyboard.gotoScene( ... )
	-- OLD SYNTAX: storyboard.gotoScene( sceneName [, effect, effectTime] )
	--
	-- NEW SYNTAX:
	--
	-- local options = {
	--     effect = "slideLeft"
	--     time = 800,
	--     params = { any="vars", can="go", here=true }	-- optional params table to pass to scene event
	-- }
	-- storyboard.gotoScene( sceneName, options )
	--
	-- NOTE: params table will only be visible in the following events: "createScene", "willEnterScene" and "enterScene"
    --

    storyboard.hideOverlay()	-- hide any overlay that may be currently showing
	
	-- parse arguments
	local arg = {...}
	local argOffset = 0

	-- if user uses colon syntax (storyboard:gotoScene()), autocorrect to prevent errors
	if arg[1] and arg[1] == storyboard then
		argOffset = 1

		if storyboard.isDebug then
			debug_print( "WARNING: You should use dot-syntax when calling storyboard functions. For example, storyboard.gotoScene() instead of storyboard:gotoScene()." )
		end
	end
	
	if arg and _type(arg[1+argOffset]) == "boolean" then
		argOffset = argOffset + 1	-- showActivityIndicator parameter has been deprecated; users should control this on their own
	end
	local newScene = arg[1+argOffset]
	local options, params, effect, effectTime

	if _type(arg[2+argOffset]) == "table" then
		options = arg[2+argOffset]
		effect = options.effect
		effectTime = _tonumber(options.time)
		params = options.params		-- params is an optional table that users can pass to the next scene

	elseif arg[2+argOffset] then
		effect = arg[2+argOffset]
		effectTime = _tonumber(arg[3+argOffset])
	end
	
	----- end parse args
	
	-- create a reference to current module
	if not currentModule then
		currentModule = newScene
	elseif currentModule == newScene then
		storyboard.reloadScene()
		return
	elseif currentModule then
		previousScene = currentModule
	end
	
	local fx = effectList[effect] or {}
	local noEffect = not effect
	local screenshot = saveSceneAndHide( currentScene, newScene, noEffect )	-- save screenshot, remove current scene, show scene capture
	if not touchOverlay then
		touchOverlay = createTouchOverlay()	-- creates overlay that disables touches on entire device screen (during scene transition)
	else
		touchOverlay.isHitTestable = true	-- allow touches when invisible
	end
	
	-- load the scene (first check if scene has already been loaded)
	local scene = storyboard.scenes[newScene]
	
	-- Create the specified scene and view group if necessary. Then set the
	-- currentScene variable to specified scene (to be transitioned to)
	
	if scene then
		if not scene.view then
			-- if view does not exist, create it and re-dispatch "createScene" event
			scene.view = display.newGroup()
			local event = {}
			event.name = "createScene"
			event.params = params
			storyboard.scenes[newScene]:dispatchEvent( event )
		end
		currentScene = scene.view
	else
		local success, msg = pcall( function() storyboard.scenes[newScene] = require( newScene ) end )
		if not success and msg then
			if storyboard.isDebug then
				debug_print( "Cannot transition to scene: " .. _toString(newScene) .. ". There is either an error in the scene module, or you are attempting to go to a scene that does not exist. If you called storyboard.removeScene() on a scene that is NOT represented by a module, the scene must be re-created before transitioning back to it." )
			end
			error( msg )
		end
		scene = storyboard.scenes[newScene]
		if _type(scene) == 'boolean' then
			error( "Attempting to load scene from invalid scene module (" .. sceneName .. ".lua). Did you forget to return the scene object at the end of the scene module? (e.g. 'return scene')" )
		end
		scene.view = scene.view or display.newGroup()
		currentScene = scene.view
		
		local event = {}
		event.name = "createScene"
		event.params = params
		storyboard.scenes[newScene]:dispatchEvent( event )
	end
	
	-- Set initial values for scene that will be transitioned into (and other relevant elements, such as touchOverlay)
	if fx.sceneAbove then
		stage:insert( currentScene )
	else
		stage:insert( 1, currentScene )	-- ensure new scene is in storyboard's 'stage' display group
	end
	touchOverlay:toFront()	-- make sure touch overlay is in front of newly loaded scene

	-- set starting properties for the next scene (currentScene)
    currentScene.isVisible = false

    if fx.to then
    	currentScene.x = fx.to.xStart or 0
    	currentScene.y = fx.to.yStart or 0
    	currentScene.alpha = fx.to.alphaStart or 1.0
    	currentScene.xScale = fx.to.xScaleStart or 1.0
    	currentScene.yScale = fx.to.yScaleStart or 1.0
    	currentScene.rotation = fx.to.rotationStart or 0
    end
	
	-- onComplete listener for first transition (previous scene; screenshot)
	local transitionNewScene = function() nextTransition( currentScene, fx, effectTime, touchOverlay, screenshot, params ); end
	
	-- transition the previous scene out (the screenshot):
	if effect then
		-- create transition options table (for the scene that's on the way out)
		local options = {}
		options.x = fx.from.xEnd
		options.y = fx.from.yEnd
		options.alpha = fx.from.alphaEnd
		options.xScale = fx.from.xScaleEnd
		options.yScale = fx.from.yScaleEnd
		options.rotation = fx.from.rotationEnd
		options.time = effectTime or 500
		options.transition = fx.from.transition
		options.onComplete = transitionNewScene
		
		-- for effects where both scenes should transition concurrently, remove onComplete listener
		if fx.concurrent then options.onComplete = nil; end
		
		-- begin scene transitions
		if screenshot then
			if not fx.concurrent and options.onComplete then

				-- next scene should transition AFTER first scene (e.g. scene1 -> done. -> scene2)
				local sceneTransition = transition.to( screenshot, options )
			else
				-- first and next scene should transition at the same time (e.g. scene1 -> scene2 )

				local sceneTransition = transition.to( screenshot, options )
				transitionNewScene()
			end
		else
			-- no screenshot, meaning there was no previous scene (first scene; coming from main.lua, most likely)
			transitionNewScene()
		end
	else
		--if screenshot then display.remove( screenshot ); screenshot = nil; end   -- for screen capture logic
		touchOverlay.isHitTestable = false
		currentScene.isVisible = true
		currentScene.x, currentScene.y = 0, 0

		-- dispatch previous scene's didExitScene event
		local previous = storyboard.getPrevious()
		if previous and storyboard.scenes[previous] then
			local event = {}
			event.name = "didExitScene"
			storyboard.scenes[previous]:dispatchEvent( event )
		end
		
		-- dispatch current scene's willEnterScene and enterScene events
		if storyboard.scenes[currentModule] then
			local event = {}
			event.name = "willEnterScene"
			event.params = params
			storyboard.scenes[currentModule]:dispatchEvent( event )

			addToSceneHistory( currentModule )
			local event = {}
			event.name = "enterScene"
			event.params = params
			storyboard.scenes[currentModule]:dispatchEvent( event )
			

			if storyboard.purgeOnSceneChange then
				storyboard.purgeAll()
			end
		end
	end
end

-----------------------------------------------------------------------------------------

-- debug function
function storyboard.printMemUsage()   
	if storyboard.isDebug then	
	   	local memUsed = (_collectGarbage("count")) / 1000
	   	local texUsed = _getInfo( "textureMemoryUsed" ) / 1000000
	   	
	   	print("\n---------MEMORY USAGE INFORMATION---------")
	    print("System Memory Used:", _stringFormat("%.03f", memUsed), "Mb")
		print("Texture Memory Used:", _stringFormat("%.03f", texUsed), "Mb")
	    print("------------------------------------------\n")
	    
	    return true
	end
end

-----------------------------------------------------------------------------------------

-- on low memory warning, automatically purge least recently used scene
local function purgeLruScene( event )	-- Lru = "least recently used"
	if not storyboard.disableAutoPurge then
		local lruScene = storyboard.loadedSceneMods[1]
		
		-- ensure the "lruScene" is not the currently loaded scene
		-- also ensure that there are at least 2 scenes left (to prevent
		-- currently transitioning-out scene from being purged)
		if lruScene and lruScene ~= currentModule and #storyboard.loadedSceneMods > 2 then
			if storyboard.isDebug then
				debug_print( "Auto-purging scene: " .. lruScene " due to low memory. If you want to disable auto-purging on low memory, set storyboard.disableAutoPurge to true." )
			end
			storyboard.purgeScene( lruScene )
		end
	elseif storyboard.isDebug and not storyboard.purgeOnSceneChange then
		debug_print( "Low memory warning recieved (auto-purging is disabled). You should manually purge un-needed scenes at this time." )
	end
end
Runtime:addEventListener( "memoryWarning", purgeLruScene )

return storyboard

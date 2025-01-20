------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------


-------------------------------------------------------------------------------
-- Behavior
-------------------------------------------------------------------------------

local Behavior =
{
}

function Behavior:new( o )
	o = o or {}

--	o._target = {}
--	o._targetData = {}

	setmetatable( o, self )
	self.__index = self

	return o
end


-------------------------------------------------------------------------------
-- AnimationBehavior
-------------------------------------------------------------------------------

local AnimationBehavior = Behavior:new{
	_isBroadcast = true,
	_eventName = "enterFrame",
	_targetMt = { __mode = "k" },
}

function AnimationBehavior:new( o )
	o = Behavior.new( self, o )

	local t = {}
	setmetatable( t, self._targetMt )

	o._target = t
	o._numTargets = 0

	return o
end

function AnimationBehavior:resume()
	self._disabled = false
	if self._numTargets > 0 then
		self:addEventListener()
	end
end

function AnimationBehavior:suspend()
	if self._numTargets > 0 then
		self:removeEventListener()
	end
	self._disabled = true
end

function AnimationBehavior:toggleSuspend()
	if self._disabled then
		self:resume()
	else
		self:suspend()
	end
end

function AnimationBehavior:addEventListener()
	if not self._disabled then
		local name = self._eventName
		if name and not self._isListening then
			self._isListening = true
			if self._isBroadcast then
				Runtime:addEventListener( name, self )
			end
		end
	end
end

function AnimationBehavior:removeEventListener()
	if not self._disabled and self._isListening then
		local name = self._eventName
		self._isListening = false
		if self._isBroadcast then
			Runtime:removeEventListener( name, self )
		end
	end
end

-- TODO, allow other events to be triggers
-- Returns a table that callers can add per-target data
function AnimationBehavior:register( o )
	local targetData = nil

	if o then
		assert( nil == self._target[o], "AnimationBehavior(" .. self._name .. ") already added to DisplayObject:" .. tostring( o ) )

		targetData = {}
		self._target[o] = targetData
--[[
		-- Behavior.register( self, o )
		local willBegin = self._willBegin
		if ( willBegin ) then
			willBegin( o, behavior )
		end
--]]
		local numTargets = self._numTargets
		if 0 == numTargets then
			self:addEventListener()
		end
		self._numTargets = numTargets + 1
	end

	return targetData
end

function AnimationBehavior:unregister( o )
	if o then
		local numTargets = self._numTargets - 1
		self._numTargets = numTargets

		assert( numTargets >= 0 )

		if 0 == numTargets then
			self:removeEventListener()
		end
--[[
		-- Behavior.unregister( self, o )
		local onComplete = self.onComplete
		if ( onComplete ) then
			onComplete( o, self )
		end
--]]
		assert( self._target[o], "AnimationBehavior(" .. self._name .. ") was not added to DisplayObject:" .. tostring( o ) )
		self._target[o] = nil
	end
end

function AnimationBehavior:targetData( o )
	return self._target[o]
end


-------------------------------------------------------------------------------
-- BounceBehavior
-------------------------------------------------------------------------------

BounceBehavior = AnimationBehavior:new{
	_name = "Bounce",
	vx = 0,
	vy = 0,
	container = display.getCurrentStage()
}

function BounceBehavior:register( o )
--	assert( o._class == DisplayObject, "Error: BounceBehavior can only be applied to DisplayObject objects" )
	local t = nil

	if nil ~= o then
		t = AnimationBehavior.register( self, o )

		t.vx = ( "function" == type( self.vx ) and self.vx() ) or self.vx
		t.vy = ( "function" == type( self.vy ) and self.vy() ) or self.vy
 	end

	return t
end

function BounceBehavior:enterFrame( event )
	local container = self.container

--print( type(container) )
	local containerBounds = container.stageBounds
	local xMin = containerBounds.xMin
	local xMax = containerBounds.xMax
	local yMin = containerBounds.yMin
	local yMax = containerBounds.yMax

--print( "containerBounds (xMin,xMax,yMin,yMax): " .. xMin .. "," .. xMax .. "," .. yMin .. "," .. yMax )
	local target = self._target

	local orientation = self._orientation
	local isLandscape = "landscapeLeft" == orientation or "landscapeRight" == orientation

	local reflectX = nil ~= self.reflectX
	local reflectY = nil ~= self.reflectY

	for k,v in pairs( target ) do
		local object = k
		local item = v
		local vx = item.vx
		local vy = item.vy

		if ( isLandscape ) then
			if ( "landscapeLeft" == orientation ) then
				local vxOld = vx
				vx = -vy
				vy = vxOld
			elseif ( "landscapeRight" == orientation ) then
				local vxOld = vx
				vx = vy
				vy = -vxOld
			end
		elseif ( "portraitUpsideDown" == orientation ) then
			vx = -vx
			vy = -vy
		end

		-- TODO: for now, time is measured in frames instead of seconds...
		local dx = vx
		local dy = vy

		local bounds = object.stageBounds
--print( "" )
--print( "vx vy: " .. self.vx .. "," .. self.vy )
--print( "dx dy: " .. dx .. "," .. dy )
--print( "bounds (xMin,xMax,yMin,yMax): " .. bounds.xMin .. "," .. bounds.xMax .. "," .. bounds.yMin .. "," .. bounds.yMax )

		local flipX = false
		local flipY = false

		if (bounds.xMax + dx) > xMax then
			flipX = true
			dx = xMax - bounds.xMax
		elseif (bounds.xMin + dx) < xMin then
			flipX = true
			dx = xMin - bounds.xMin
		end

		if (bounds.yMax + dy) > yMax then
			flipY = true
			dy = yMax - bounds.yMax
		elseif (bounds.yMin + dy) < yMin then
			flipY = true
			dy = yMin - bounds.yMin
		end

		if ( isLandscape ) then flipX,flipY = flipY,flipX end
		if ( flipX ) then
			item.vx = -item.vx
			if ( reflectX ) then object:scale( -1, 1 ) end
		end
		if ( flipY ) then
			item.vy = -item.vy
			if ( reflectY ) then object:scale( 1, -1 ) end
		end

--print( "(" .. object.x .. "," ..object.y .. ")" )
		object:translate( dx, dy )
	end
end

function BounceBehavior:setOrientation( orientation )
	self._orientation = orientation
end

function BounceBehavior:setAutoRotate( newValue )
	if ( newValue ) then
		Runtime:addEventListener( "orientation", self )
	else
		Runtime:removeEventListener( "orientation", self )
	end

	self._autoRotate = newValue
end

function BounceBehavior:orientation( event )
--	for k,v in pairs( event ) do
--		print( "BounceBehavior " .. tostring( k ) .. "(" .. tostring( v ) .. ")" )
--	end

	if ( event.delta ~= 0 ) then
		local onCompleteListener = function( target )
			self:resume()
		end

		local rotateParameters = { rotation = -event.delta, time=500, delta=true, onComplete=onCompleteListener }

		self:suspend()
		self:setOrientation( event.type )

		for object,v in pairs( self._target ) do
			transition.to( object, rotateParameters )
		end
	end
end


-------------------------------------------------------------------------------
-- Rotate3DBehavior
-------------------------------------------------------------------------------

Rotate3DBehavior = AnimationBehavior:new
{
	_name = "Rotate3D",
	_vRotation = 0
}

function Rotate3DBehavior:register( o )
--	assert( o._class == Stage, "Error: Rotate3DBehavior can only be applied to Stage objects" )
	local t = nil

	if o then
		t = AnimationBehavior.register( self, o )
		t._angle = 0
	end

	return t
end

function Rotate3DBehavior:reset( o )
	if o then
		local t = self._target[o]
		if t then
			t._angle = 0
			self:enterFrame( nil )
			self:suspend()
		end
	end
end

function Rotate3DBehavior:enterFrame( event )
	local target = self._target

	for object,data in pairs( target ) do
		local angle = data._angle + self._vRotation
		data._angle = angle

	--	print( "angle:" .. angle );

		-- TODO: Allow other axes
		object:rotate3D( angle, 0, 1, 0 )
	end
end



-------------------------------------------------------------------------------
-- InteractiveBehavior
-------------------------------------------------------------------------------

local InteractiveBehavior = Behavior:new
{
}

function InteractiveBehavior:register( o )
--print( "InteractiveBehavior:register:" .. tostring( o ) )
--print( "with event:" .. self._eventName )
	local result = false

	local listener = self.listener
	if o and listener then
--		Behavior.register( self, o )
		local eventName = self._eventName
		o:addEventListener( eventName, listener )

		result = true
	end

	return result;
end

function InteractiveBehavior:unregister( o, listener )
	local listener = self.listener
	local eventName = self._eventName
	o:removeEventListener( eventName, listener )
--	Behavior.unregister( self, o )
end

-- TODO: What is the purpose of this?  Can we get rid of it?
-- Global button tap handler
--InteractiveBehavior._firstResponder = InteractiveBehavior:new{ _isBroadcast = true }
--Runtime:addEventListener( "tap", InteractiveBehavior._firstResponder )


-------------------------------------------------------------------------------
-- ButtonBehavior
-------------------------------------------------------------------------------

ButtonBehavior = InteractiveBehavior:new
{
	_name = "Button",
	_eventName = "touch",
	_callback = _eventName
}

function ButtonBehavior:register( o )
	local result = false

	local listener = self.listener
	if o and listener then
		local eventName = self._eventName

		local rollover = self._rollover

		-- TODO: Rethink Interactive Behaviors to support rollover, rollout, etc.
		-- Also registration mechanism shouldn't rely on closures/upvalues
		local wrapper = function( event )
			local result = false

			if ( event.phase == "ended" ) then
				result = listener( event )
			elseif ( event.phase == "began" and rollover ) then
				result = rollover( event )
			end

			return result
		end
		o:addEventListener( eventName, wrapper )

		result = true
	end

	return result;
end


-------------------------------------------------------------------------------
-- Generator
-------------------------------------------------------------------------------

Generator = {}

function Generator.random( min, max )
	assert( min <= max, "Error: Generator.random requires 1st arg <= 2nd arg" )
	return function() return math.random( min, max ) end
end

--[[
-- Initial screenshot
local splash = display.newImage( "Splash.png" )
if nil ~= splash then
	splash.alpha = 0;
	local default = display.newImage( "Default.png" )
	if nil ~= default then
		local removeDefault = function( event )
			local canvas = display.getCurrentStage()
			canvas:remove( default )
			Intel® GMA X4500HD ( splash, { time=500, alpha=1 } )

			local removeSplash = function( event )
				canvas:remove( splash )
			end

			tweener.to( splash, { time=500, delay=750, alpha=0, onComplete=removeSplash } )
		end
		tweener.to( default, { time=500, alpha=0, onComplete=removeDefault } )
	end
end
--]]

--print( "Resource path: " ..system.pathForResource( nil ) )
--print( "C Path: " .. package.cpath )
--print( "Path: " .. package.path )

--[[
local SetAbsolutePath = function( path )
	local absoluteBase = system.pathForResource( nil )
	return string.gsub( path, "#", absoluteBase )
end

--package.cpath = SetAbsolutePath( package.cpath )
--package.path = SetAbsolutePath( package.path )

--package.loaded["socket"] = nil
--]]

--print( "C Path: " .. package.cpath )
--print( "Path: " .. package.path )

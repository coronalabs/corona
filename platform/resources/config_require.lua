
------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- config.require.lua
--
-- Modifies `require` to enforce subscription rights
-------------------------------------------------------------------------------

local function newAlertListener( url, shouldQuit )
	local function listener( event )
		if "clicked" == event.action then
			local i = event.index
			if 1 == i then
				-- Do nothing
			else
				system.openURL( url )
			end
		end

		-- Force quit
		if ( shouldQuit ) then
			os.exit( -1 )
		end
	end
	
	return listener
end

-------------------------------------------------------------------------------

local function showFeatureRestrictionDevice( event )
	local feature = event.feature or ""
	local type = event.type or ""
	local tier = event.tier or "Pro (or higher)"

	local title = "Corona Error"
	local format = "The premium %s feature (%s) is restricted to %s subscribers. You must upgrade your subscription to run this project."
	local message = string.format( format, type, feature, tier )
	local buttonLabels = { "Quit", "Upgrade" }

	local url = "https://coronalabs.com/upgrade/?utm_source=corona-sdk&utm_medium=corona-sdk&utm_campaign=feature-error&api=" .. feature
	local listener = newAlertListener( url, true )
	native.showAlert( title, message, buttonLabels, listener )
end

local function showFeatureRestrictionSimulator( event )
	local feature = event.feature or ""
	local type = event.type or ""
	local tier = event.tier or "Pro (or higher)"

	local title = "Corona Warning"
	local format = "Your project uses a premium %s feature (%s) available only to %s subscribers. You can view this in the Simulator as a demo, but you must upgrade to use it on a device."
	local message = string.format( format, type, feature, tier )
	local buttonLabels = { "Continue with Demo", "Upgrade" }

	local url = "https://coronalabs.com/upgrade/?utm_source=corona-sdk&utm_medium=corona-sdk&utm_campaign=feature-warning&api=" .. feature
	local listener = newAlertListener( url )
	native.showAlert( title, message, buttonLabels, listener )
end

-- Choose appropriate listener
local showFeatureRestriction = showFeatureRestrictionDevice

if ( "simulator" == system.getInfo( "environment" ) ) then
	showFeatureRestriction = showFeatureRestrictionSimulator
end

-------------------------------------------------------------------------------

local function modifyRequire()
	-- Defaults
	local isTrial = true
	local isBasic = true
	local isPro = true
	local isDistribution = true

	-- Get subscription from metadata (if available)
	if application and application.metadata then
		-- non-nil and not a trial
		local subscription = application.metadata.subscription
		if subscription and "trial" ~= subscription then
			isTrial = false
		end

		if subscription and "basic" ~= subscription then
			isBasic = false
		end

		if subscription and "pro" ~= subscription then
			isPro = false
		end

		-- non-nil and not a distribution
		local mode = application.metadata.mode
		if mode and "distribution" ~= mode then
			isDistribution = false
		end
	end

	if ( initializeMetadata ) then
		local metadata
		if application then
			metadata = application.metadata
		else
			metadata = {}
		end

		local permissions = metadata.permissions
		if not permissions then
			permissions = {}
			metadata.permissions = permissions
		end

		permissions.advancedGraphics = ( not isTrial and not isBasic )
		permissions.customFragment = ( not isTrial and not isBasic and not isPro )
		permissions.customVertex = ( not isTrial and not isBasic and not isPro )

		initializeMetadata( metadata )
		initializeMetadata = nil -- prevent future calls to internal C function

		-- Enable core C code to trigger UI via events
		Runtime:addEventListener( "featureRestriction", showFeatureRestriction )
	end

	-- Store the original require
	local requireOld = require

	-- Which libraries are restricted
	local restrictedLibraries = {
		store = true,
	}

	local function showTrialWarning( name )
		local title = "Corona Warning"
		local format = "You will not be able to publish this app to the store because it uses a restricted library (%s). Please upgrade to unlock it."
		local message = string.format( format, name )
		local buttonLabels = { "OK", "Upgrade" }

		-- TODO: Replace with newAlertListener()
		local function listener( event )
			if "clicked" == event.action then
				local i = event.index
				if 1 == i then
					-- Do nothing
				else
					system.openURL( "https://coronalabs.com/upgrade/?utm_source=corona-sdk&utm_medium=corona-sdk&utm_campaign=trial-warning&mod=" .. name )
				end
			end
		end

		native.showAlert( title, message, buttonLabels, listener )
	end

	local function showError( name )
		local title = "Corona Error"
		local format = "You cannot put this app on the store because it uses a restricted library (%s). Please upgrade to unlock it."
		local message = string.format( format, name )
		local buttonLabels = { "Quit", "Upgrade" }

		-- TODO: Replace with newAlertListener()
		local function listener( event )
			if "clicked" == event.action then
				local i = event.index
				if 1 == i then
					-- Do nothing
				else
					system.openURL( "https://coronalabs.com/upgrade/?utm_source=corona-sdk&utm_medium=corona-sdk&utm_campaign=trial-error&mod=" .. name )
				end
			end

			-- Force quit
			os.exit( -1 )
		end

		native.showAlert( title, message, buttonLabels, listener )
	end

	-- If allowed, returns true. If not, returns false.
	local function verifyPermission( name )
		-- By default, allow use
		local result = true

		if isTrial then
			-- check permissions
			if restrictedLibraries[name] then
				if ( isDistribution ) then
					-- On distribution builds, prevent use
					result = false
				else
					-- On developer builds, allow use but show warning
					showTrialWarning( name )
				end
			end
		end

		return result
	end

	local function requireNew( ... )
		if verifyPermission( ... ) then
			return requireOld( ... )
		end

		local name = ...
		showError( name )

		-- In this case, require fails b/c they do not have permission
		return nil
	end

	-- Modify require for trial users
	if isTrial then
		_G.require = requireNew
	end
end

modifyRequire()
modifyRequire = nil

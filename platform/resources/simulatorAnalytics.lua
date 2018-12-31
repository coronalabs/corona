------------------------------------------------------------------------------
--
-- Copyright (C) 2018 Corona Labs Inc.
-- Contact: support@coronalabs.com
--
-- This file is part of the Corona game engine.
--
-- Commercial License Usage
-- Licensees holding valid commercial Corona licenses may use this file in
-- accordance with the commercial license agreement between you and 
-- Corona Labs Inc. For licensing terms and conditions please contact
-- support@coronalabs.com or visit https://coronalabs.com/com-license
--
-- GNU General Public License Usage
-- Alternatively, this file may be used under the terms of the GNU General
-- Public license version 3. The license is as published by the Free Software
-- Foundation and appearing in the file LICENSE.GPL3 included in the packaging
-- of this file. Please review the following information to ensure the GNU 
-- General Public License requirements will
-- be met: https://www.gnu.org/licenses/gpl-3.0.html
--
-- For overview and more information on licensing please refer to README.md
--
------------------------------------------------------------------------------

--[[
-- Uncomment to find unscoped variables

local mt = getmetatable(_G)
if mt == nil then
  mt = {}
  setmetatable(_G, mt)
end

__STRICT = true
mt.__declared = {}

mt.__newindex = function (t, n, v)
  if __STRICT and not mt.__declared[n] then
    local w = debug.getinfo(2, "S").what
    if w ~= "main" and w ~= "C" then
      error("STRICT: assign to undeclared variable '"..n.."'", 2)
    end
    mt.__declared[n] = true
  end
  rawset(t, n, v)
end
  
mt.__index = function (t, n)
  if not mt.__declared[n] and debug.getinfo(2, "S").what ~= "C" then
    error("STRICT: variable '"..n.."' is not declared", 2)
  end
  return rawget(t, n)
end

function global(...)
   for _, v in ipairs{...} do mt.__declared[v] = true end
end

--]]

--[[

Analysis of Amplitude API requests

Start message:
[
    {
        "api_properties": {
            "max_id": 1,
            "special": "session_start"
        },
        "build_version_release": "Version 10.8.4 (Build 12E55)",
        "client": "mac",
        "country": "United States",
        "custom_properties": {},
        "device_id": "db3ffaaf695d877bdd3c0a1767ba37e4",
        "event_id": 1,
        "event_type": "session_start",
        "global_properties": {},
        "language": "English",
        "phone_carrier": null,
        "phone_model": "MacBook Air",
        "platform_string": "MacBookAir5,2",
        "properties": {
            "max_id": 1,
            "special": "session_start"
        },
        "session_id": 1375379650937,
        "timestamp": 1375379650937,
        "user_id": "db3ffaaf695d877bdd3c0a1767ba37e4",
        "version_name": "1.1"
    }
]

Start template:
[
    {
        "api_properties": {
            "max_id": %EVENT_ID%,
            "special": "session_start"
        },
        "build_version_release": "%OS_VERSION%",
        "client": "win",
        "country": "%COUNTRY_NAME%",
        "custom_properties": {},
        "device_id": "%DEVICE_ID%",
        "event_id": %EVENT_ID%,
        "event_type": "session_start",
        "global_properties": {},
        "language": "%LANGUAGE_NAME%",
        "phone_carrier": null,
        "phone_model": "%COMPUTER_MODEL%",
        "platform_string": "%COMPUTER_MODEL_ID%",
        "properties": {
            "max_id": %EVENT_ID%,
            "special": "session_start"
        },
        "session_id": %SESSION_START_TIME%,
        "timestamp": %CURRENT_TIME%,
        "user_id": "%DEVICE_ID%",
        "version_name": "%APPLICATION_VERSION%"
    }
]

End message:
[
    {
        "api_properties": {
            "max_id": 2,
            "special": "session_end"
        },
        "build_version_release": "Version 10.8.4 (Build 12E55)",
        "client": "mac",
        "country": "United States",
        "custom_properties": {},
        "device_id": "db3ffaaf695d877bdd3c0a1767ba37e4",
        "event_id": 2,
        "event_type": "session_end",
        "global_properties": {},
        "language": "English",
        "phone_carrier": null,
        "phone_model": "MacBook Air",
        "platform_string": "MacBookAir5,2",
        "properties": {
            "max_id": 2,
            "special": "session_end"
        },
        "session_id": 1375379650937,
        "timestamp": 1375379652948,
        "user_id": "db3ffaaf695d877bdd3c0a1767ba37e4",
        "version_name": "1.1"
    }
]

Request headers:
	Accept: */*
	Accept-Language: en-us
	Connection: keep-alive
	User-Agent: Crasher/1 CFNetwork/596.4.3 Darwin/12.4.0 (x86_64) (MacBookAir5%2C2)
	Accept-Encoding: gzip, deflate
	Content-Type: application/x-www-form-urlencoded
	Content-Length: 1751
	Host: api.amplitude.com

POST Params:

	v=2
	client=%API_KEY%
	[client_api_key=%CLIENT_API_KEY%]
	e=%JSON%
	upload_time=%CURRENT_TIME%
	checksum=%CHECKSUM%

Tags:

	%MESSAGE_TYPE%			"session_start" | "session_end"
	%DEVICE_ID% 			MD5 of MAC address
	%EVENT_ID%				Monotonically increasing (per message) integer (starts at 1)
	%SESSION_START_TIME%	Epoch time
	%CURRENT_TIME%			Epoch time
	%CHECKSUM%				MD5 of complete POST
	%APPLICATION_VERSION%	Simulator build id
	%COMPUTER_MODEL%		"Windows PC"
	%COMPUTER_MODEL_ID%		"Windows PC"
	%COUNTRY_NAME%			system.getPreference( "locale", "country" )
	%LANGUAGE_NAME%			system.getPreference( "locale", "language" )

--]]

local json = require "json"
local crypto = require "crypto"
local network = require "network"
local string = require "string"

--
-- This is a minimal implementation of Amplitude's client that will transmit session starts, ends and log simple events
--
-- It is really a simulation of their API because I didn't want to reverse engineer their current version only to have
-- baked in assumptions trip us up later.  This approach should be easy to modify as they evolve their API.
--
-- Ideally we would have a timer fire every 10 seconds to see if there are any events to send but we can't use timers
-- in this context so we try to fake it by sending any accumulated events in analytics:log() if it's been 10 seconds
-- since we last sent anything.
--
function amplitudeSimulatorAnalytics(subscriptionType, uid, cachePath, getInfoFunc, getPreferenceFunc, isParticipating )

	local apiKey = "a8f6d25b9a8557334af0f0116af439e8" -- Simulator -- "1b06cabc5fedeb5d95eac340cecd2177" -- testSim -- 

	local apiVersion = 2
    local maxDispatchedBatches = 10 -- enough for a typical session
	local url = "http://stats.coronalabs.com/analytics/simulator" -- "https://api.amplitude.com"
    local pendingEvents = {}
    local dispatchedBatches = {}
    local maxPendingEvents = 10
	local deviceId = getInfoFunc( "deviceID" ) or MD5( "1234567890" )

	local debug = function (...) 
		-- print("amplitudeSimulatorAnalytics: ", ...)
	end

	local MD5 = function( input )
		return crypto.digest( crypto.md5, input )
	end

	local function GetPlatformName()
	    local platform = system.getInfo( "platformName" )
        
        if (platform == "Mac OS X") then
            return "mac"
        elseif (platform == "Win") then
            return "win"
        else
            return platform
        end
    end

	local function GetPlatformDescription()
	    local platform = system.getInfo( "platformName" )
        
        if (platform == "Mac OS X") then
            return "Macintosh"
        elseif (platform == "Win") then
            return "Windows PC"
        else
            return platform
        end
    end

	local function GetOSVersion()
	    local version = system.getInfo( "platformVersion" )

	    local versionMap = { ["5.0"] = "Windows 2000",
	                         ["5.1"] = "Windows XP",
	                         ["5.2"] = "Windows Server 2003",
	                         ["6.0"] = "Windows Vista",
	                         ["6.1"] = "Windows 7",
	                         ["6.2"] = "Windows 8",
	                         ["6.3"] = "Windows 8.1",
	                         ["10.0"] = "Windows 10" }

        if (GetPlatformName() == 'win') then
            if (versionMap[version] == nil) then
                return ("Windows "..tostring(version))
            else
                return versionMap[version]
            end
        else
            return version
        end
	end

	local system = system
	
	local function urlencode(str)
	      if (str) then
	          str = string.gsub (str, "\n", "\r\n")
	          str = string.gsub (str, "([^%w ])",
	              function (c) return string.format ("%%%02X", string.byte(c)) end)
	          str = string.gsub (str, " ", "+")
	      end
	      return str
	end

	local networkRequestFunc = function (...) 
		debug("Calling network.request(", ..., ")")
		network.request(...)
	end
    
    -- Count the number of elements in the associative array
    local function arrayLength(array)
        local count = 0
        for checksum, payload in pairs(array) do
            count = count + 1
        end
        
        return count
    end

	
	local eventId = 1
	local sessionStartTime = os.time() * 1000

    local function escapeStringForJSON(str)

        str = string.gsub(tostring(str), "%\\", "%\\%\\")
        
        return str
    end

	local function createJSON(sessionType, sessionStart, customProperties)
		local sessionItems = {
				["@PLATFORM@"] = 			GetPlatformName(),
				["@OS_VERSION@"] = 			GetOSVersion(),
				["@DEVICE_ID@"] = 			deviceId,
				["@EVENT_ID@"] =			eventId,
				["@CUSTOM_PROPERTIES@"] =	customProperties,
				["@SESSION_START_TIME@"] =	sessionStart,
				["@CURRENT_TIME@"] =		os.time() * 1000,
				["@APPLICATION_VERSION@"] =	getInfoFunc( "build" ),
				["@COMPUTER_MODEL@"] =		GetPlatformDescription(),
				["@COMPUTER_MODEL_ID@"] =	GetPlatformDescription(),
				["@COUNTRY_NAME@"] =		getPreferenceFunc( "locale", "country" ),
				["@LANGUAGE_NAME@"] = 		getPreferenceFunc( "locale", "language" ),
				["@SUBSCRIPTION_TYPE@"] = 	subscriptionType,
			}

		local jsonTemplate = [[ 			    
                {
			        "api_properties":{
			            "max_id":@EVENT_ID@,
			            "special":"@MESSAGE_TYPE@"
			        },
			        "build_version_release":"@OS_VERSION@",
			        "client":"@PLATFORM@",
			        "country":"@COUNTRY_NAME@",
			        "custom_properties":{ @CUSTOM_PROPERTIES@ },
			        "device_id":"@DEVICE_ID@",
			        "event_id":@EVENT_ID@,
			        "event_type":"@MESSAGE_TYPE@",
			        "global_properties":{},
			        "language":"@LANGUAGE_NAME@",
			        "phone_carrier":"@SUBSCRIPTION_TYPE@",
			        "phone_model":"@COMPUTER_MODEL@",
			        "platform_string":"@COMPUTER_MODEL_ID@",
			        "properties":{
			            "max_id":@EVENT_ID@,
			            "special":"@MESSAGE_TYPE@"
			        },
			        "session_id":@SESSION_START_TIME@,
			        "timestamp":@CURRENT_TIME@,
			        "user_id":"@DEVICE_ID@",
			        "version_name":"@APPLICATION_VERSION@"
			    }
		]]

		local eventJSON = jsonTemplate
		for k in pairs(sessionItems) do 
            -- escape string for gsub
            sessionItems[k] = escapeStringForJSON(sessionItems[k]):gsub("%%","%%%%")
			debug("k: "..tostring(k), "sessionItems[k]: "..sessionItems[k])
			eventJSON = string.gsub(eventJSON, k, sessionItems[k])
		end
        
        -- escape string for gsub
        sessionType = sessionType:gsub("%%","%%%%")

		eventJSON = string.gsub(eventJSON, "@MESSAGE_TYPE@", sessionType)

		-- Clean up whitespace in the JSON
		eventJSON = string.gsub(eventJSON, "^[  ]*", "")
		eventJSON = string.gsub(eventJSON, "\n[	 ]*", "")

		debug("eventJSON: ".. eventJSON)

        eventId = eventId + 1
        
		return eventJSON
	end
    
    local function saveDispatchedBatches()
        local f = io.open( cachePath, "w+b" )
        if f then
            -- debug( "[analytics] writing to: " .. cachePath )
            local data = json.encode( dispatchedBatches )
            f:write( data )
            f:close()
        else
            debug( "[analytics] ERROR: could not write to: " .. cachePath )
        end
    end

    local function addDispatchedEvent(requestId, request)
        dispatchedBatches[requestId] = request
        
        saveDispatchedBatches()
    end
        
    local function networkListener(event, requestId)
        debug("networkListener: requestId: ".. requestId .."; status: ".. event.status .."; isError: ".. tostring(event.isError) .."; response: ".. tostring(event.response))

        if (dispatchedBatches[requestId] == nil) then
            debug("networkListener: dispatchedBatches["..requestId.."] unexpectedly nil")
        end

        if (event.isError ~= true and (event.status >= 200 and event.status < 300)) then
            -- event successfully sent, remove it from the list
            dispatchedBatches[requestId] = nil
            
            saveDispatchedBatches()
        end
    end

    local function sendSavedEvents()
        if (isParticipating) then
            local savedDispatchedBatches = {}
            local f = io.open( cachePath, "rb" )
            if f then
            
                debug("loading events from " .. cachePath);

                local data = f:read( "*all" )

                -- only decode non-empty file
                if data ~= nil and "" ~= data then
                    savedDispatchedBatches = json.decode( data )
                end

                f:close()

                -- delete file
                os.remove( cachePath )
            end
            
            local eventCount = arrayLength(savedDispatchedBatches)

            debug("sendSavedEvents: sending ", eventCount, " pending events")

            -- We want to send the last 'maxDispatchedBatches' events from the saved table
            -- (the table is recreated so its maximum size is the number of events
            -- in a session + 'maxDispatchedBatches')
            for checksum, payload in pairs(savedDispatchedBatches) do
                if ((eventCount - maxDispatchedBatches) <= 0) then
                
                    debug("sendSavedEvents: sending saved event: ", checksum, payload)
                    
                    local params = { body = payload, headers = { ["Content-Type"] = "application/x-www-form-urlencoded" } }

                    addDispatchedEvent(checksum, payload)
                    
                    pcall( function() networkRequestFunc( url, "POST", function(event)
                            networkListener(event, checksum)
                        end,
                        params
                    ) end )
                end
                eventCount = eventCount - 1
           end
        end
    end
    
    local lastSendPendingEvents = os.time()
    
    local function sendPendingEvents()
    
        if (#pendingEvents == 0) then
            return -- nothing to do
        end
        
        -- Make a JSON array of the pending events
        local batchedPayload = ""
        for i = 1, #pendingEvents do
            if (batchedPayload ~= "") then
                batchedPayload = batchedPayload .. ", "
            end
            
            batchedPayload = batchedPayload .. pendingEvents[i]
        end
        
        batchedPayload = "[ " .. batchedPayload .. " ]"

        -- The currently pending events have been processed (the "dispatched events" mechanism
        -- will handle transmission issues)
        pendingEvents = {}

        -- Build a payload and checksum to submit to the network
        local payload = ""
        local upload_time = (os.time() * 1000)

        payload = payload .. "v=" .. tostring(apiVersion)
        payload = payload .. "&client=" .. apiKey
        -- [client_api_key=%CLIENT_API_KEY%]
        payload = payload .. "&e=" .. urlencode( batchedPayload )
        payload = payload .. "&upload_time=" .. tostring(upload_time)

        debug("payload: ".. tostring(payload))
        
        -- Calculate checksum
        -- apiVersionString, _apiKey, events, timestampString
        local checksum = MD5(tostring(apiVersion) .. apiKey .. batchedPayload .. tostring(upload_time) )
        payload = payload .. "&checksum=" .. checksum

        local params = { body = payload, headers = { ["Content-Type"] = "application/x-www-form-urlencoded" } }

        -- Remember the submission in case it doesn't send
        addDispatchedEvent(checksum, payload)

        pcall( function() networkRequestFunc( url, "POST", function(event)
                                    networkListener(event, checksum)
                                end,
                                params
                            ) end )

        -- Record the last time we sent any events so we can send stragglers
        lastSendPendingEvents = os.time()
    end

    local function batchEvent(event)
        -- Save the event
        pendingEvents[#pendingEvents + 1] = event
 
        if (#pendingEvents > maxPendingEvents) then
        
            sendPendingEvents()
            
        end
    end
        
	local function sendAnalytics(sessionType, customData)
        if (isParticipating) then
            
            if (sessionType == "session_start" and subscriptionType ~= nil and subscriptionType ~= "") then
                -- Put "subscriptionType" and "uid" into a JSON array which will show up as custom event properties
                customData = '"subscription" : "'..tostring(subscriptionType)..'", "uid" : "'..tostring(uid)..'"'
            end

            if (customData == nil) then
                customData = ""
            end

            local event = createJSON(sessionType, sessionStartTime, customData)

            batchEvent(event)
        end
	end
    
    -- this is homebrew, perhaps we should use the mechanism described here: http://docs.intercom.io/api#creating_an_impression
    local function pingIntercom(currentUid, tag)
        local intercomURL = "https://api.intercom.io/vjs/users/ping"
        local intercomKey = "Bxf60u8tigUM7-iOcRGFEhD-L_cvfSqqllyM_Cvu"
        local intercomItems = {
            ["@INTERCOM_APP_ID@"] = "3046ee18ea912a752faf5a6c76cb57645b637e41",
            ["@INTERCOM_USER_ID@"] = currentUid,
            ["@TAG@"] = tag,
            ["@SECURE_HASH@"] = '"user_hash":"' .. crypto.hmac( crypto.sha256, tostring(currentUid), intercomKey ) .. '",',
            ["@CURRENT_TIME@"] = os.time(),
        }

        local intercomTemplate = 'referer=http://simulator.coronalabs.com/@TAG@&app_id=@INTERCOM_APP_ID@&user_data={"user_id":"@INTERCOM_USER_ID@", @SECURE_HASH@ "site_visited":"@TAG@", "@TAG@_at":@CURRENT_TIME@, "app_id":"@INTERCOM_APP_ID@", "increments":{ "@TAG@s":1 } }'
        
        local intercomRequest = intercomTemplate
		for k in pairs(intercomItems) do 
            -- escape string for gsub
            intercomItems[k] = escapeStringForJSON(intercomItems[k]):gsub("%%","%%%%")
			-- debug("k: "..tostring(k), "intercomItems[k]: "..tostring(intercomItems[k]))
			intercomRequest = string.gsub(intercomRequest, k, intercomItems[k])
		end
        
        debug("pingIntercom: ", intercomRequest)

        local params = { body = intercomRequest, headers = { ["Content-Type"] = "application/x-www-form-urlencoded", ["User-Agent"] = "Corona_Simulator/"..getInfoFunc( "build" ) } }
        -- local params = { body = intercomRequest, headers = { ["Content-Type"] = "application/x-www-form-urlencoded" } }

        if ( isParticipating ) then
            -- Fire and forget
            pcall( function() networkRequestFunc( intercomURL, "POST", params ) end )
        end
        
    end

    -- Perform startup actions

    sendSavedEvents()
        
	sendAnalytics("session_start")
    
    -- Flush "session_start"
    sendPendingEvents()

	-- Create object methods
	
	local analytics = {}

	function analytics:beginSession(uid)
            -- Put things here that we want to happen when the Simulator starts and when the user logs in
            pingIntercom(uid, "simulator")
	end
    
	function analytics:log( eventName, customData )
		debug( "analytics:log: "..eventName.."; customData: "..tostring(customData) )

        if (customData ~= nil and customData ~= "") then
            -- Make "customData" into a JSON array with a key of "for"
            customData = '"for" : "'..tostring(customData)..'"'
        end
        
        -- If they are doing a build, tell Intercom
        -- (note that "-"s have to be escaped with "%"s in Lua patterns)
        if (string.match(eventName, "^build%-for%-(%w+)")) then
            pingIntercom(uid, eventName)
        end
        
        sendAnalytics(eventName, customData)
        
        -- If it's been more than 10 seconds since we last sent data, send what's accumulated now
        if ((os.time() - lastSendPendingEvents) > 10) then
            sendPendingEvents()
        end
	end

	function analytics:endSession()
		debug( "analytics:endProject: session lasted: ".. tostring((os.time() * 1000) - sessionStartTime).." seconds")

        sendAnalytics("session_end")

        sendPendingEvents()
    end

	-- make this c-tor unreachable
	_G.amplitudeSimulatorAnalytics = nil

	return analytics

end


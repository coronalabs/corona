---------------------------------------------------------------------
-- XML-RPC over HTTP.
-- See Copyright Notice in license.html
-- $Id: http.lua,v 1.2 2004/09/27 16:39:37 tomas Exp $
---------------------------------------------------------------------

require"socket"
require"xmlrpc"

local post = socket.http.post

xmlrpc.http = {}

---------------------------------------------------------------------
-- Call a remote method.
-- @param url String with the location of the server.
-- @param method String with the name of the method to be called.
-- @return Table with the response (could be a `fault' or a `params'
--	XML-RPC element).
---------------------------------------------------------------------
function xmlrpc.http.call (url, method, ...)
	local body, headers, code, err = post {
		url = url,
		body = xmlrpc.clEncode (method, unpack (arg)),
		headers = {
			["User-agent"] = "LuaXMLRPC",
			["Content-type"] = "text/xml",
		},
	}
	if tonumber (code) == 200 then
		return xmlrpc.clDecode (body)
	else
		error (err or code)
	end
end

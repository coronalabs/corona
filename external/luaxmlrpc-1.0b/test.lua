#!/usr/local/bin/lua.5.0
-- See Copyright Notice in license.html

require "xmlrpc"

function table._print (tab, indent, spacing)
	spacing = spacing or ""
	indent = indent or "\t"
    io.write ("{\n")
    for nome, val in pairs (tab) do
        io.write (spacing..indent)
        local t = type(nome)
		if t == "string" then
            io.write (string.format ("[%q] = ", tostring (nome)))
		elseif t == "number" or t == "boolean" then
            io.write (string.format ("[%s] = ", tostring (nome)))
        else
            io.write (t)
        end
        t = type(val)
        if t == "string" or t == "number" then
            io.write (string.format ("%q", val))
        elseif t == "table" then
            table.print (val, indent, spacing..indent)
        else
            io.write (t)
        end
        io.write (",\n")
    end
    io.write (spacing.."}")
end

function table._tostring (tab, indent, spacing)
	local s = {}
	spacing = spacing or ""
	indent = indent or "\t"
    table.insert (s, "{\n")
    for nome, val in pairs (tab) do
        table.insert (s, spacing..indent)
        local t = type(nome)
		if t == "string" then
            table.insert (s, string.format ("[%q] = ", tostring (nome)))
		elseif t == "number" or t == "boolean" then
            table.insert (s, string.format ("[%s] = ", tostring (nome)))
        else
            table.insert (s, t)
        end
        t = type(val)
        if t == "string" or t == "number" then
            table.insert (s, string.format ("%q", val))
        elseif t == "table" then
            table.insert (s, table._tostring (val, indent, spacing..indent))
        else
            table.insert (s, t)
        end
        table.insert (s, ",\n")
    end
    table.insert (s, spacing.."}")
	return table.concat (s)
end

function table.print (tab, indent, spacing)
	io.write (table._tostring (tab, indent, spacing))
end

function table.equal2 (t1, t2)
	if type(t1) ~= "table" or type(t2) ~= "table" then
		return false
	end
	for key, v1 in t1 do
		local v2 = rawget (t2, key)
		if type(v1) == "table" and type(v2) == "table" then
			if not table.equal (v1, v2) then
				return false
			end
		elseif v1 ~= v2 then
			return false
		end
	end
	return true
end

function table.equal (t1, t2)
	if type(t1) ~= "table" or type(t2) ~= "table" then
		return false
	end
	local s1 = table._tostring (t1)
	local s2 = table._tostring (t2)
	s1 = string.gsub (s1, "%s*", "")
	s2 = string.gsub (s2, "%s*", "")
	--if s1 ~= s2 then
		--print(s1, "!!!!", s2)
	--end
	return s1 == s2, s1.."!!!"..s2
end

function remove_type (tab)
	if type(tab) ~= "table" then
		return tab
	elseif tab["*type"] then
		return tab["*value"]
	end
	for i,v in pairs (tab) do
		if type(v) == "table" then
			tab[i] = remove_type (v)
		end
	end
	return tab
end

function call_test (xml_call, method, ...)
	local xc = string.gsub (xml_call, "(%p)", "%%%1")
	--xc = string.gsub (xc, "\r?\n%s*", "%%s*")
	xc = string.gsub (xc, "%s*", "")
	arg.n = nil

	-- client enconding test
	local meth_call = xmlrpc.clEncode (method, unpack (arg))
	meth_call = string.gsub (meth_call, "%s*", "")
	local s = string.gsub (meth_call, xc, "")
	s = string.gsub (s, "%s*", "")
	assert (s == "", s.."\n!!!\n"..xc)

	-- server decoding test
	local meth_call, param = xmlrpc.srvDecode (xml_call)
	assert (meth_call == method, meth_call)
	arg = remove_type (arg)
	assert (table.equal (arg, param))
end

function response_test (xml_resp, lua_obj)
	-- client decoding test
	local ok, obj = xmlrpc.clDecode (xml_resp)

	-- server encoding test
	xml_resp = string.gsub (xml_resp, "(%p)", "%%%1")
	xml_resp = string.gsub (xml_resp, "\r?\n%s*", "%%s*")
	local meth_resp = xmlrpc.srvEncode (lua_obj)

	if type (obj) == "table" then
		lua_obj = remove_type (lua_obj)
		assert (table.equal (obj, lua_obj))
	else
		assert (obj == lua_obj)
	end
	local s = string.gsub (meth_resp, xml_resp, "")
	s = string.gsub (s, "%s*", "")
	assert (s == "", s)
end

function fault_test (xml_resp, message, code)
	-- client decoding test
	local ok, str, n = xmlrpc.clDecode (xml_resp)
	assert (str == message)
	assert (n == code)

	-- server encoding test
	xml_resp = string.gsub (xml_resp, "(%p)", "%%%1")
	xml_resp = string.gsub (xml_resp, "\r?\n%s*", "%%s*")
	local meth_resp = xmlrpc.srvEncode ({ message = message, code = code }, true)
	local s = string.gsub (meth_resp, xml_resp, "")
	s = string.gsub (s, "%s*", "")
	assert (s == "", s)
end

---------------------------------------------------------------------
-- call tests.
---------------------------------------------------------------------
call_test ([[<?xml version="1.0"?>
<methodCall>
   <methodName>examples.getStateName</methodName>
   <params>
      <param>
         <value><int>41</int></value>
         </param>
      </params>
   </methodCall>]], "examples.getStateName", 41)

local double_139 = xmlrpc.newTypedValue (139, "double")
call_test ([[<?xml version="1.0"?>
<methodCall>
   <methodName>examples.getSomething</methodName>
   <params>
      <param>
         <value>
<struct>
   <member>
      <name>lowerBound</name>
      <value><double>18.2</double></value>
      </member>
   <member>
      <name>upperBound</name>
      <value><double>139</double></value>
      </member>
   </struct>
           </value>
         </param>
      </params>
   </methodCall>]], "examples.getSomething", { lowerBound = 18.2, upperBound = double_139 })

local double_array = xmlrpc.newArray ("double")
call_test ([[<?xml version="1.0"?>
<methodCall>
  <methodName>test</methodName>
    <params>
      <param><value><array><data>
<value><double>1</double></value>
<value><double>2</double></value>
<value><double>3</double></value>
<value><double>4</double></value>
</data></array></value></param>
    </params>
</methodCall>]],
	"test", 
	xmlrpc.newTypedValue ({ 1, 2, 3, 4, }, double_array)
)

local double_array_array = xmlrpc.newArray (double_array)
call_test ([[<?xml version="1.0"?>
<methodCall>
  <methodName>test</methodName>
    <params>
      <param><value><array><data>
<value><array><data>
<value><double>1</double></value>
<value><double>2</double></value>
<value><double>3</double></value>
<value><double>4</double></value>
</data></array></value>
<value><array><data>
<value><double>11</double></value>
<value><double>12</double></value>
<value><double>13</double></value>
<value><double>14</double></value>
</data></array></value>
<value><array><data>
<value><double>21</double></value>
<value><double>22</double></value>
<value><double>23</double></value>
<value><double>24</double></value>
</data></array></value>
</data></array></value></param>
    </params>
</methodCall>]],
	"test",
	xmlrpc.newTypedValue (
		{
			{ 1, 2, 3, 4, },
			{ 11, 12, 13, 14, },
			{ 21, 22, 23, 24, },
		},
		double_array_array
	)
)


call_test ([[<?xml version="1.0"?>
<methodCall>
  <methodName>insertTable</methodName>
  <params>
    <param><value><string>people</string></value></param>
    <param><value>
      <array><data>
        <value>
          <struct>
            <member>
              <name>name</name>
              <value><string>Fulano</string></value>
            </member>
            <member>
              <name>email</name>
              <value><string>fulano@nowhere.world</string></value>
            </member>
          </struct>
        </value>
        <value>
          <struct>
            <member>
              <name>name</name>
              <value><string>Beltrano</string></value>
            </member>
            <member>
              <name>email</name>
              <value><string>beltrano@nowhere.world</string></value>
            </member>
          </struct>
        </value>
        <value>
          <struct>
            <member>
              <name>name</name>
              <value><string>Cicrano</string></value>
            </member>
            <member>
              <name>email</name>
              <value><string>cicrano@nowhere.world</string></value>
            </member>
          </struct>
        </value>
      </data></array>
    </value></param> 
  </params>
</methodCall>]],
	"insertTable",
	"people",
	xmlrpc.newTypedValue (
		{
			{ name = "Fulano", email = "fulano@nowhere.world", },
			{ name = "Beltrano", email = "beltrano@nowhere.world", },
			{ name = "Cicrano", email = "cicrano@nowhere.world", },
		},
		xmlrpc.newArray ("struct")
	)
)

---------------------------------------------------------------------
-- response tests.
---------------------------------------------------------------------
response_test ([[<?xml version="1.0"?>
<methodResponse>
   <params>
      <param>
         <value><string>South Dakota</string></value>
         </param>
      </params>
   </methodResponse>]], "South Dakota")

response_test ([[<?xml version="1.0"?>
<methodResponse>
   <params>
      <param>
         <value>
<struct>
   <member>
      <name>lowerBound</name>
      <value><double>18.2</double></value>
      </member>
   <member>
      <name>upperBound</name>
      <value><double>139</double></value>
      </member>
   </struct>
           </value>
         </param>
      </params>
   </methodResponse>]], { lowerBound = 18.2, upperBound = double_139 })

fault_test ([[<?xml version="1.0"?>
<methodResponse>
    <fault>
      <value>
        <struct>
          <member>
            <name>faultCode</name>
            <value><int>1</int></value>
          </member>
          <member>
            <name>faultString</name>
            <value><string>error string</string></value>
          </member>
        </struct>
      </value>
    </fault>
</methodResponse>]], "error string", 1)

print("Ok!")

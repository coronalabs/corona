-- testing lstrip

-- this function does nothing
-- this is another comment
function f(n,...)
	local s=[[
--header...

	bye
]]
	for i=1,n do
		s = s .. '  ' .. 12 .. " " .. ...
	end
	print(">>>", ... .. s, ...)
end

--[[
Now test it...
Not that it means anything...
]]

f(1 - -2, "alpha", "beta", "gamma")

require"remdebug.engine"

remdebug.engine.start()
    
local tab = {
  foo = 1,
  bar = 2
}

print("Start")

function bar()
  print("In bar 1")
  print("In bar 2")
end

for i = 1, 10 do
  print("Loop")
  bar()
  tab.foo = tab.foo * 2
end

print("End")


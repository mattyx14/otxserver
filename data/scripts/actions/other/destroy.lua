local scimitarPos = {x = 33205, y = 32537, z = 6}

local function removeScimitar(position)
	local scimitarItem = Tile(position):getItemById(5858)
	if scimitarItem then
		scimitarItem:remove()
	end
end

local destroy = Action()

function destroy.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	return onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end

for id = 3264, 3292 do
    destroy:id(id)
end
for id = 3296, 3303 do
    destroy:id(id)
end
for id = 3305, 3307 do
    destroy:id(id)
end
for id = 3309, 3341 do
    destroy:id(id)
end
destroy:id(3294)

destroy:register()

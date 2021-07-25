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

for id = 2376, 2404 do
    destroy:id(id)
end
for id = 2408, 2415 do
    destroy:id(id)
end
for id = 2417, 2419 do
    destroy:id(id)
end
for id = 2421, 2453 do
    destroy:id(id)
end
destroy:id(2406)

destroy:register()

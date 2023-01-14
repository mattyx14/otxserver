local scimitarPos = {x = 33205, y = 32537, z = 6}
local caveEntrancePos = {x = 33206, y = 32536, z = 6}
local scimitarItemId = 3307
local placedSimitarItemId = 5858
local caveEntranceId = 7181

local function removeEntrance()
	local scimitarItem = Tile(scimitarPos):getItemById(placedSimitarItemId)
	local caveEntranceItem = Tile(caveEntrancePos):getItemById(caveEntranceId)

	if scimitarItem then
		scimitarItem:remove()
	end

	if caveEntranceItem then
		caveEntranceItem:transform(1085)
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
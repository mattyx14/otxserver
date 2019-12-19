local exhausth = 3600 --em quantos segundos podera usar denovo
local holes = {468, 481, 483, 7932, 23712}
local pools = {2016, 2017, 2018, 2019, 2020, 2021, 2025, 2026, 2027, 2028, 2029, 2030}
function onUse(cid, item, fromPosition, itemEx, toPosition)
	if isInArray(pools, itemEx.itemid) then
        itemEx = Tile(toPosition):getGround()
    end

	if targetId == 23712 then
		target:transform(23713)
		addEvent(revertItem, 30 * 1000, toPosition, 23713, 23712)
	else
		return false
	end
   return true
end



function shovelNormal(cid, item, fromPosition, itemEx, toPosition)
local target = itemEx
    local player = Player(cid)
    local iEx = Item(itemEx.uid)
    if isInArray(holes, itemEx.itemid) then
        iEx:transform(itemEx.itemid + 1)
        iEx:decay()
 elseif isInArray(pools, target.itemid) then
        local hole = 0
        for i = 1, #holes do
            local tile = Tile(target:getPosition()):getItemById(holes[i])
            if tile then
                hole = tile
            end
        end
        if hole ~= 0 then
            hole:transform(hole:getId() + 1)
            hole:decay()
        else
            return false
        end
    elseif itemEx.itemid == 231 or itemEx.itemid == 9059 or itemEx.itemid == 22672 then
        local rand = math.random(1, 100)
        if(itemEx.actionid  == 100 and rand <= 20) then
        iEx:transform(489)
        iEx:decay()
        elseif rand == 1 then
            Game.createItem(2159, 1, toPosition)
        elseif rand > 95 then
            Game.createMonster("Rat", toPosition)
        end
        toPosition:sendMagicEffect(CONST_ME_POFF)
	elseif targetId == 23712 then
		target:transform(23713)
		addEvent(revertItem, 30 * 1000, toPosition, 23713, 23712)
    else
        return false
    end
    return true
end

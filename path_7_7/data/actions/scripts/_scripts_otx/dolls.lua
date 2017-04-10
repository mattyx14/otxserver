local dolls = {
	[5080] = {"Hug me."}
}

function onUse(player, item, fromPosition, target, toPosition)
	local sounds = dolls[item.itemid]
	if not sounds then
		return false
	end

	if fromPosition.x == CONTAINER_POSITION then
		fromPosition = player:getPosition()
	end

	local random = math.random(#sounds)
	local sound = sounds[random]

	sound = sound:gsub('|PLAYERNAME|', player:getName())
	player:say(sound, TALKTYPE_MONSTER_SAY, false, 0, fromPosition)
	return true
end

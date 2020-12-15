local failPosition = {
	[5500] = {
		mission = Storage.FirstSacrifice.fireAxe,
		fromPosition = Position(890, 1021, 11),
		missionName = "require fixe axe quest"
	},
	[5501] = {
		mission = Storage.FirstSacrifice.brightSword,
		fromPosition = Position(931, 1084, 11),
		missionName = "require bright sword quest"
	},
	[5502] = {
		mission = Storage.FirstSacrifice.skullStaff,
		fromPosition = Position(724, 821, 12),
		missionName = "require skull staff quest"
	},
	[5503] = {
		mission = Storage.FirstQuest.Fynn,
		fromPosition = Position(925, 981, 7),
		missionName = "the first quest of fynn"
	},
	[5504] = {
		mission = 7730,
		fromPosition = Position(937, 1062, 7),
		missionName = "require blue legs quest"
	},
	[5505] = {
		mission = Storage.AnniQuest.blessedSceptreDone,
		fromPosition = Position(940, 1076, 8),
		missionName = "require blessed sceptre quest"
	},
	[5506] = {
		mission = Storage.AnniQuest.royalAxeDone,
		fromPosition = Position(940, 1076, 8),
		missionName = "require royal axe quest"
	},
	[5507] = {
		mission = Storage.AnniQuest.justiceSeekerDone,
		fromPosition = Position(940, 1076, 8),
		missionName = "require the justice seeker quest"
	},
	[5508] = {
		mission = Storage.FirstQuest.Misidia,
		fromPosition = Position(1066, 319, 7),
		missionName = "the first quest of misidia"
	},
	[5509] = {
		mission = Storage.FirstSacrifice.noSacrifice.axeKnight,
		fromPosition = Position(977, 945, 8),
		missionName = "require make the first sacrifice knight axe"
	},
	[5510] = {
		mission = Storage.FirstSacrifice.noSacrifice.fangWyvern,
		fromPosition = Position(973, 1033, 7),
		missionName = "require make the first sacrifice wyvern fang"
	},
	[5511] = {
		mission = Storage.FirstSacrifice.noSacrifice.hammerDragon,
		fromPosition = Position(562, 1266, 6),
		missionName = "require make the first sacrifice dragon hammer"
	},
}

function onStepIn(creature, item, position, fromPosition)
	local config = failPosition[item.actionid]
	if not config then
		return true
	end

	local player = creature:getPlayer()
	if not player then
		return true
	end

	if player:getStorageValue(config.mission) == 1 then
		return true
	end

	player:teleportTo(config.fromPosition)
	config.fromPosition:sendMagicEffect(CONST_ME_MAGIC_BLUE)
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'This area ' .. config.missionName .. '.')
	return true
end

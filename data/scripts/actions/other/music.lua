local MusicEffect = {
	[2070] = CONST_ME_SOUND_GREEN, 	--Wooden Flute
	[2071] = CONST_ME_SOUND_GREEN, 	--Lyre
	[2072] = CONST_ME_SOUND_GREEN, 	--Lute
	[2073] = CONST_ME_SOUND_GREEN, 	--Drum
	[2074] = CONST_ME_SOUND_BLUE, 	--Panpipes
	[2075] = CONST_ME_SOUND_GREEN, 	--Simple Fanfare
	[2076] = CONST_ME_SOUND_GREEN, 	--Fanfare
	[2077] = CONST_ME_SOUND_GREEN, 	--Royal Fanfare
	[2078] = CONST_ME_SOUND_GREEN, 	--Post Horn
	[2079] = CONST_ME_SOUND_RED, 	--War Horn
	[2080] = CONST_ME_SOUND_BLUE, 	--Piano
	[2081] = CONST_ME_SOUND_BLUE, 	--Piano
	[2082] = CONST_ME_SOUND_BLUE, 	--Piano
	[2083] = CONST_ME_SOUND_BLUE, 	--Piano
	[2084] = CONST_ME_SOUND_BLUE, 	--Harp
	[2085] = CONST_ME_SOUND_BLUE, 	--Harp
	[2332] = CONST_ME_SOUND_GREEN, 	--Waldo's Post Horn
	[2364] = CONST_ME_SOUND_GREEN, 	--Post Horn
	-- non movable instruments
	[2367] = CONST_ME_SOUND_GREEN, 	--Drum
	[2368] = CONST_ME_SOUND_GREEN, 	--Simple Fanfare
	[3957] = CONST_ME_SOUND_YELLOW, --Cornucopia
	[2370] = CONST_ME_SOUND_GREEN, 	--Lute
	[2371] = CONST_ME_SOUND_BLUE, 	--Horn of Sundering
	[2372] = CONST_ME_SOUND_GREEN, 	--Lyre
	[2373] = CONST_ME_SOUND_BLUE, 	--Panpipes
	[2070] = CONST_ME_SOUND_GREEN, 	--Wooden Flute
	--
	[3951] = CONST_ME_SOUND_BLUE, 	--Bongo Drum
	[3952] = CONST_ME_SOUND_GREEN, 	--Didgeridoo
	[3953] = CONST_ME_SOUND_RED, 	--War Drum
	[5786] = CONST_ME_SOUND_GREEN, 	--Wooden Whistle
	[13759] = CONST_ME_SOUND_BLUE, 	--Small Whistle
}

local music = Action()

function music.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	player:addAchievementProgress('Rockstar', 10000)
	item:getPosition():sendMagicEffect(MusicEffect[item.itemid])
	return true
end

for index, value in pairs(MusicEffect) do
	music:id(index)
end

music:register()

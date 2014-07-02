local reloadInfo = {
	{RELOAD_ACTIONS, "actions", "action"},
	{RELOAD_CHAT, "chat", "channels"},
	{RELOAD_CONFIG, "config", "configuration"},
	{RELOAD_CREATUREEVENTS, "creatureevents", "creature events", "creaturescripts", "creature scripts", "creatureevent", "creature event", "creature script", "creaturescript", "creature"},
	{RELOAD_GAMESERVERS, "gameservers", "game servers", "servers"},
	{RELOAD_GLOBALEVENTS, "globalevents", "global events", "globalevent", "global event", "global"},
	{RELOAD_GROUPS, "groups", "playergroups", "group"},
	{RELOAD_HIGHSCORES, "highscores", "scores", "highscore", "score"},
	{RELOAD_ITEMS, "items", "item"},
	{RELOAD_MONSTERS, "monsters", "monster"},
	{RELOAD_MOVEEVENTS, "moveevents", "move events", "movements", "move", "movement"},
	{RELOAD_NPCS, "npcs", "npc"},
	{RELOAD_OUTFITS, "outfits", "outfit"},
	{RELOAD_QUESTS, "quests", "quest"},
	{RELOAD_RAIDS, "raids", "raid"},
	{RELOAD_SPELLS, "spells", "spell"},
	{RELOAD_STAGES, "stages", "experience"},
	{RELOAD_TALKACTIONS, "talkactions", "talk actions", "talk", "commands", "talkaction", "talk action"},
	{RELOAD_VOCATIONS, "vocations", "vocation"},
	{RELOAD_WEAPONS, "weapons", "weapon"},
	{RELOAD_MODS, "mods", "modifications"},
	{RELOAD_ALL, "all", "everything"}
}

function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	param = param:lower()
	local str = "Reload type not found."
	for _, v in ipairs(reloadInfo) do
		if(table.isStrIn(param, v)) then
			doReloadInfo(v[1], cid)
			str = "Reloading " .. v[2] .. "..."
			break
		end
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, str)
	return true
end

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)					npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)				npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)			npcHandler:onCreatureSay(cid, type, msg)	end
function onThink()								npcHandler:onThink()						end

function creatureSayCallback(cid, type, msg)
	if (not npcHandler:isFocused(cid)) then
		return false
	end

	if msgcontains(msg, 'rhyves') and not isPlayerPzLocked(cid) then
		doTeleportThing(cid, {
			x = 159, y = 338 , z = 6
		})
		self:releaseFocus(cid)
		if(doPlayerRemoveMoney(cid, 50)) then
			selfSay(cid, 'Here you are.')
		else
			selfSay(cid, 'Sorry, you don\'t have enough gold.')
		end
	end

	if msgcontains(msg, 'jorvik') and not isPlayerPzLocked(cid) then
		doTeleportThing(cid, {
			x = 420, y = 256, z = 6
		})
		self:releaseFocus(cid)
		if(doPlayerRemoveMoney(cid, 50)) then
			selfSay(cid, 'Here you are.')
		else
			selfSay(cid, 'Sorry, you don\'t have enough gold.')
		end
	end

	if isPlayerPzLocked(cid) then
		npcHandler:say(cid, 'You can\'t travel, you have pz!')
	end

	return true
end

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
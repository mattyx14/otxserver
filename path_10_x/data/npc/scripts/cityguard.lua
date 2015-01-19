local target = 0
local prevTarget = 0
local maxChaseDistance = 30
local origPos = 0
local lastAttack = 0
local followTimeout = 10

local attacks = {
	min = -50,
	max = -100
}

local function isSkulled(cid)
	if(getCreatureSkullType(cid) >= SKULL_WHITE and isPlayerPzLocked(cid)) then
		return true
	end

	return false
end

local function goToOrigPos()
	target = 0
	lastAttack  = 0
	selfFollow(0)
	doTeleportThing(getNpcCid(), origPos)
end

local function updateTarget()
	if(not isPlayer(target)) then
		goToOrigPos()
	elseif(not isSkulled(target)) then
		selfSay("Now, behave in the future.")
		goToOrigPos()
	end

	if(target == 0) then
		local list = getSpectators(getNpcPos(), 9, 9, false)
		for i = 1, table.getn(list) do
			local _target = list[i]
			if(_target ~= 0) then
				if(isPlayer(_target) and isSkulled(_target)) then
					if(not getTilePzInfo(getCreaturePosition(_target))) then
						if(selfFollow(_target)) then
							target = _target
							if(target ~= prevTarget) then
								selfSay("We do not tolerate people like you here!")
							end

							prevTarget = target
							break
						end
					end
				end
			end
		end
	end
end

function onCreatureAppear(cid)
	if(cid == getNpcCid()) then
		origPos = getNpcPos()
	end
end

function onCreatureDisappear(cid)
	if(cid == target) then
		goToOrigPos()
	end
end

function onCreatureMove(creature, oldPos, newPos)
	return true
end

function onThink()
	updateTarget()

	if(target == 0) then
		return
	end

	local playerPos = getCreaturePosition(target)
	local myPos = getNpcPos()

	if(myPos.z ~= playerPos.z) then
		goToOrigPos()
		return
	end

	if(math.abs(myPos.x - origPos.x) > maxChaseDistance or math.abs(myPos.y - origPos.y) > maxChaseDistance) then
		selfSay("I'll catch you next time.")
		goToOrigPos()
		return
	end

	if(lastAttack == 0) then
		lastAttack = os.clock()
	end

	if(os.clock() - lastAttack > followTimeout) then
		selfSay("You got me this time, but just wait.")
		goToOrigPos()
		return
	end

	if((math.abs(playerPos.x - myPos.x) <= 1) and (math.abs(playerPos.y - myPos.y) <= 1)) then
		doTargetCombatHealth(getNpcCid(), target, COMBAT_LIFEDRAIN, attacks.min, attacks.max, CONST_ME_BLOCKHIT)
		lastAttack = os.clock()
	end

	return true
end
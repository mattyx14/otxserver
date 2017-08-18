function rem(pet1)
	doRemoveCreature(pet1)
end

function transport(pet1, cid)
	local posit = getCreaturePosition(cid)
	if not posit then
		return
	end

	local maxDistance = 6
	if Tile(posit):hasFlag(TILESTATE_PROTECTIONZONE) then -- we need to edit source to enable pet enter on pz
		return
	else
		if getCreaturePosition(pet1).z ~= getCreaturePosition(cid).z or getDistanceBetween(getCreaturePosition(pet1), getCreaturePosition(cid)) > maxDistance then
			posit.y = posit.y + 1
			doTeleportThing(pet1, posit, true)
		end
	end

	addEvent(transport, 1*1*1000, pet1, cid)
end

local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_NONE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_BLOCKHIT)

local area = createCombatArea(AREA_CROSS1X1)
setCombatArea(combat, area)

local maxsummons = 1
function onCastSpell(cid, var)
	doCreatureSay(cid, "My Power your Power", TALKTYPE_ORANGE_1)
	local summoncount = getCreatureSummons(cid)
	if #summoncount < 1 then
		for i = 1, maxsummons - #summoncount do
			vocationid = cid:getVocation():getId()
			if vocationid == 1 or vocationid == 5 then
				pet = "thundergiant"
			elseif vocationid == 2 or vocationid == 6 then
				pet = "grovebeast"
			elseif vocationid == 3 or vocationid == 7 then
				pet = "emberwing"
			elseif vocationid == 4 or vocationid == 8 then
				pet = "skullfrost"
			end

			local pet1 = Game.createMonster(pet, getCreaturePosition(cid))
			pet1:setMaster(cid)
			addEvent(rem, 15*60*1000, pet1.uid)
			addEvent(transport, 1*10*1000, pet1.uid, cid.uid)
			if pet1 == false then
				return combat:execute(creature, var)
			end

			doConvinceCreature(cid, pet1)
		end
	end

	return doCombat(cid, combat, var)
end

local vocation = {
	VOCATION.BASE_ID.SORCERER,
	VOCATION.BASE_ID.DRUID,
	VOCATION.BASE_ID.PALADIN,
	VOCATION.BASE_ID.KNIGHT
}

local area = {
	{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
	{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
	{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
	{0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
	}

local createArea = createCombatArea(area)

local combat = Combat()
combat:setArea(createArea)

function onTargetTile(creature, pos)
	local creatureTable = {}
	local n, i = Tile({x=pos.x, y=pos.y, z=pos.z}).creatures, 1
	if n ~= 0 then
		local v = getThingfromPos({x=pos.x, y=pos.y, z=pos.z, stackpos=i}).uid
		while v ~= 0 do
			if isCreature(v) == true then
				table.insert(creatureTable, v)
				if n == #creatureTable then
					break
				end
			end
			i = i + 1
			v = getThingfromPos({x=pos.x, y=pos.y, z=pos.z, stackpos=i}).uid
		end
	end
	if #creatureTable ~= nil and #creatureTable > 0 then
		for r = 1, #creatureTable do
			if creatureTable[r] ~= creature then
				local min = 15000
				local max = 15000
				local player = Player(creatureTable[r])

				if isPlayer(creatureTable[r]) == true and table.contains(vocation, player:getVocation():getClientId()) then
					doTargetCombatHealth(creature, creatureTable[r], COMBAT_FIREDAMAGE, -min, -max, CONST_ME_NONE)
				elseif isMonster(creatureTable[r]) == true then
					doTargetCombatHealth(creature, creatureTable[r], COMBAT_FIREDAMAGE, -min, -max, CONST_ME_NONE)
				end
			end
		end
	end
	pos:sendMagicEffect(CONST_ME_FIREATTACK)
	return true
end

combat:setCallback(CALLBACK_PARAM_TARGETTILE, "onTargetTile")

local function delayedCastSpell(cid, var)
	local creature = Creature(cid)
	if not creature then
		return
	end
	creature:say("MUHAHAHAHA", TALKTYPE_ORANGE_2)
	return combat:execute(creature, positionToVariant(creature:getPosition()))
end

local spell = Spell("instant")

function spell.onCastSpell(creature, var)
	creature:say("I SMELL FEEEEAAAAAR!", TALKTYPE_ORANGE_2)
	addEvent(delayedCastSpell, 8000, creature:getId(), var)
	return true
end

spell:name("demon death")
spell:words("###500")
spell:isAggressive(true)
spell:blockWalls(true)
spell:needLearn(true)
spell:register()

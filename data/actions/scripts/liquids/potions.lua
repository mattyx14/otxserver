local config = {
	-- strong health potion
	[7588] = {health = {250, 350}, vocations = {3, 4}, text = 'paladins and knights', level = 50, emptyId = 7634},
	-- strong mana potion
	[7589] = {mana = {115, 185}, vocations = {1, 2, 3}, text = 'sorcerers, druids and paladins', level = 50, emptyId = 7634},
	-- great mana potion
	[7590] = {mana = {150, 250}, vocations = {1, 2}, text = 'sorcerers and druids', level = 80, emptyId = 7635},
	-- great health potion
	[7591] = {health = {425, 575}, vocations = {4}, text = 'knights', level = 80, emptyId = 7635},
	-- health potion potion
	[7618] = {health = {125, 175}, emptyId = 7636},
	-- mana potion potion
	[7620] = {mana = {75, 125}, emptyId = 7636},
	-- great spirit potion
	[8472] = {health = {250, 350}, mana = {100, 200}, vocations = {3}, text = 'paladins', level = 80, emptyId = 7635},
	-- ultimate health potion
	[8473] = {health = {650, 850}, vocations = {4}, text = 'knights', level = 130, emptyId = 7635},
	-- antidote potion
	[8474] = {antidote = true, emptyId = 7636},
	-- small health potion
	[8704] = {health = {60, 85}, emptyId = 7636}
}

local antidote = Combat()
antidote:setParameter(COMBAT_PARAM_TYPE, COMBAT_HEALING)
antidote:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
antidote:setParameter(COMBAT_PARAM_TARGETCASTERORTOPMOST, true)
antidote:setParameter(COMBAT_PARAM_AGGRESSIVE, false)
antidote:setParameter(COMBAT_PARAM_DISPEL, CONDITION_POISON)

local exhaust = Condition(CONDITION_EXHAUST_HEAL)
exhaust:setParameter(CONDITION_PARAM_TICKS, (configManager.getNumber(configKeys.EX_ACTIONS_DELAY_INTERVAL) - 100))
-- 1000 - 100 due to exact condition timing. -100 doesn't hurt us, and players don't have reminding ~50ms exhaustion.

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local potion = config[item.itemid]
	if not potion then
		return true
	end

	if target.itemid ~= 1 or target.type ~= THING_TYPE_PLAYER then
		return true
	end

	if player:getCondition(CONDITION_EXHAUST_HEAL) then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, Game.getReturnMessage(RETURNVALUE_YOUAREEXHAUSTED))
		return true
	end

	if potion.antidote and not antidote:execute(target, Variant(target.uid)) then
		return false
	end

	if (potion.level and player:getLevel() < potion.level)
			or (type(potion.vocations) == 'table' and not isInArray(potion.vocations, player:getVocation():getBase():getId()))
			and not (player:getGroup():getId() >= 2) then
		player:say(string.format('This potion can only be consumed by %s of level %d or higher.', potion.text, potion.level), TALKTYPE_MONSTER_SAY)
		return true
	end

	if type(potion.health) == 'table' and not doTargetCombatHealth(0, target, COMBAT_HEALING, potion.health[1], potion.health[2], CONST_ME_MAGIC_BLUE) then
		return false
	end

	if type(potion.mana) == 'table' and not doTargetCombatMana(0, target, potion.mana[1], potion.mana[2], CONST_ME_MAGIC_BLUE) then
		return false
	end

	local cStorage = player:getStorageValue(Storage.Achievements.PotionAddict)
	if cStorage < 100000 then
		player:setStorageValue(Storage.Achievements.PotionAddict, math.max(1, cStorage) + 1)
	elseif cStorage == 100000 then
		player:addAchievement('Potion Addict')
		player:setStorageValue(Storage.Achievements.PotionAddict, 100001)
	end

	player:addCondition(exhaust)
	doCreatureSayWithRadius(target, 'Aaaah...', TALKTYPE_MONSTER_SAY, 2, 2, toPosition)
	item:remove(1)
	if fromPosition.x == CONTAINER_POSITION then
		player:addItem(potion.emptyId, 1)
	else
		Game.createItem(potion.emptyId, 1, fromPosition)
	end
	return true
end

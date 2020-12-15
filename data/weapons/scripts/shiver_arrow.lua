local area = createCombatArea({
	{0, 1, 0},
	{1, 3, 1},
	{0, 1, 0}
 })

 local combat = Combat()
 combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_ICEDAMAGE)
 combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_ICEATTACK)
 combat:setParameter(COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_SHIVERARROW)
 combat:setParameter(COMBAT_PARAM_BLOCKARMOR, true)
 combat:setFormula(COMBAT_FORMULA_SKILL, 0, 0, 1, 0)
 combat:setArea(area)

 function onUseWeapon(player, variant)
 	return combat:execute(player, variant)
 end

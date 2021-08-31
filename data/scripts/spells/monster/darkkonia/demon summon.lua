local spell = Spell("instant")

function onCastSpell(creature, var)
local t, spectator = Game.getSpectators(creature:getPosition(), false, false, 5, 5, 5, 5)
    local check = 0
    if #t ~= nil then
        for i = 1, #t do
		spectator = t[i]
            if spectator:getName() == "Diabilic imp" then
               check = check + 1
            end
        end
    end
	local hp = (creature:getHealth()/creature:getMaxHealth())* 100
	if ((check < 2) and hp <= 95) or ((check < 4) and hp <= 75) or ((check < 6) and hp <= 55) or ((check < 10) and hp <= 35) then
		for j = 1, 6 do
			creature:say("Son's! come now!!!", TALKTYPE_ORANGE_1)
		end
		for k = 1, 2 do
			local monster = Game.createMonster("diabolic imp", creature:getPosition(), true, false)
			if not monster then
				return
			end
			creature:getPosition():sendMagicEffect(CONST_ME_SOUND_RED)
		end
		else
	end
return true
end

spell:name("demon summon")
spell:words("###503")
spell:blockWalls(true)
spell:needLearn(true)
spell:register()

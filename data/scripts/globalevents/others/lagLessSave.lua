local noLagSave = GlobalEvent("noLagSave")
local timeBetweenSaves = 1 * 60 * 60 * 1000 -- in milliseconds
local delayBetweenSaves = 100

function delayedSave(cid)
    local p = Player(cid)
    if p then
        p:save()
    end
end

function noLagSave.onThink(interval, lastExecution)
    local players = Game.getPlayers()
    local delay = 0

    for _, player in ipairs(players) do
        delay = delay + delayBetweenSaves
        addEvent(delayedSave, delay, player:getId())
    end
end

noLagSave:interval(timeBetweenSaves)
noLagSave:register()
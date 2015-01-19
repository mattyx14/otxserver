-- r1 = Simple spawn if not exist engine.
-- r2 = accurate spawn timer. Make sure to have lower global interval.

------- CONFIG -------
local monsterName = "" -- "Monstername"
local MonsterPos = {x,y,z} -- {x,y,z} position where it shall spawn
local spawnMin = 10 -- 10 min cooldown between monster spawn
local storage = 1000 -- Storage to fix cooldown/spawntime (globalstorage)
------- /CONFIG ------

function onThink()
	if getCreatureByName(monsterName) == false then
		local calculation = spawnMin * 60
		if os.time() >= (getStorage(storage)+calculation) then
			if (getStorage(storage) == 0 then
				return true
			else
				doCreateMonster(monsterName, MonsterPos)
				doSetStorage(storage,os.time())
			end
		else
			if getStorage(storage) > 0 then
				local nextspawn = ((getStorage(storage)+calculation) - os.time())
				addEvent(mspawn,(1000*nextspawn))
				doSetStorage(storage,0)
			end
		end
	end

	return true
end

function mspawn()
	if getStorage(storage) == 0 then
		doCreateMonster(monsterName, MonsterPos)
		doSetStorage(storage,os.time())
	end

	return true
end
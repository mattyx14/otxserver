-- default equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:id(2504, 11304)
equip:register()

-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("legs")
deEquip:id(2477, 2488, 2470, 2504, 7885, 7894, 7895, 7896, 8923, 9777, 11304, 15409, 15412, 15490, 18400, 18405, 21700)
deEquip:register()

-- knights and paladins equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(2477, 2488, 2470)
equip:register()

-- lvl 75 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:level(75)
equip:id(15490)
equip:register()

-- knight lvl 185 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:level(185)
equip:vocation("knight", true, true)
equip:vocation("elite knight")
equip:id(15412)
equip:register()

-- sorcerer and druids equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(21700)
equip:register()

-- sorcerer and druids lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:level(150)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(18400)
equip:register()

-- sorcerer and druids lvl 130 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:level(130)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(15409)
equip:register()

-- sorcerer and druids lvl 40 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:level(40)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(7885, 7894, 7895, 7896)
equip:register()

-- paladin lvl 80 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:level(60)
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(9777)
equip:register()

-- paladin lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:level(150)
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(18405)
equip:register()

-- paladin equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("legs")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(8923)
equip:register()

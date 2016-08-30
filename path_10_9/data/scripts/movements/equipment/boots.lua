-- default equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("feet")
equip:id(2195, 6132, 2640, 11303)
equip:register()

-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("feet")
deEquip:id(2195, 6132, 2640, 7886, 7891, 7892, 7893, 9932, 9933, 11117, 11118, 11240, 11303, 12646, 15410, 18406)
deEquip:register()

-- level 130 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("feet")
equip:level(130)
equip:id(9932, 9933)
equip:register()

-- Sorcerers and Druids with lvl 35 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("feet")
equip:level(35)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(7886, 7891, 7892, 7893)
equip:register()

-- Knights and Paladins with lvl 70 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("feet")
equip:level(70)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(11117, 11118, 11240)
equip:register()

-- Knights and Paladins with lvl 80 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("feet")
equip:level(80)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(12646)
equip:register()

-- knights with lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("feet")
equip:level(150)
equip:vocation("knight", true, true)
equip:vocation("elite knight")
equip:id(15410)
equip:register()

-- Paladins with lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("feet")
equip:level(150)
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(18406)
equip:register()

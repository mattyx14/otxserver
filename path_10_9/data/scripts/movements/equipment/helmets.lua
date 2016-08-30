-- default equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:id(5461, 2474, 2343, 2502, 7459)
equip:register()

-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("head")
deEquip:id(5461, 2474, 2343, 2323, 2502, 7459, 7900, 7901, 7902, 7903, 8820, 9778, 10016, 10570, 11302, 11368, 12630, 12645, 15408, 18403, 18398, 13756)
deEquip:register()

-- knights and paladins equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(11302)
equip:register()

-- lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:level(150)
equip:id(15408)
equip:register()

-- knight lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:level(150)
equip:vocation("knight", true, true)
equip:vocation("elite knight")
equip:id(18403)
equip:register()

-- sorcerer and druids equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(2323, 7900, 7901, 7902, 7903, 8820, 10570, 12630, 13756)
equip:register()

-- sorcerer and druids lvl 150 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:level(150)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(18398)
equip:register()

-- sorcerer and druids lvl 80 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:level(80)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(9778)
equip:register()

-- sorcerer and druids lvl 60 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:level(60)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(11368)
equip:register()

-- sorcerer and druids lvl 50 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:level(50)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(10016)
equip:register()

-- paladin lvl 100 equip
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("head")
equip:level(100)
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(12645)
equip:register()

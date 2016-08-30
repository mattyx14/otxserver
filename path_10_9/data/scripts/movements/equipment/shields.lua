-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("shield")
deEquip:id(2175, 8900, 8901, 8902, 8903, 8904, 8905, 8906, 8907, 8908, 8909, 8918, 12644, 12647, 15411, 15413, 16112, 18401, 18410, 22422, 22424)
deEquip:register()

-- Equip shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(30)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(2175)
equip:register()

-- Equip level 30 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(30)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(8900)
equip:register()

-- Equip level 40 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(40)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(8901)
equip:register()

-- Equip level 50 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(50)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(8902)
equip:register()

-- Equip level 60 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(60)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(8903)
equip:register()

-- Equip level 70 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(70)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(8904)
equip:register()

-- Equip level 75 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(75)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(22422)
equip:register()

-- Equip level 80 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(80)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(8918)
equip:register()

-- Equip level 80 shield(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(80)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(12644)
equip:register()

-- Equip level 100 shield(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(100)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(8905, 8906, 8907, 8908, 8909)
equip:register()

-- Equip level 100 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(100)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(12647)
equip:register()

-- Equip level 120 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(120)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(15411, 22423)
equip:register()

-- Equip level 130 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(130)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(18401)
equip:register()

-- Equip level 130 shield(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(130)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(15413)
equip:register()

-- Equip level 150 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(150)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(16112)
equip:register()

-- Equip level 150 shield(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(150)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(18410)
equip:register()

-- Equip level 250 shield(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("shield")
equip:level(250)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, false)
equip:vocation("elder druid")
equip:id(22424)
equip:register()

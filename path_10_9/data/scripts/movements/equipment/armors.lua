-- default deEquip
local deEquip = MoveEvent()
deEquip.onDeEquip = defaultDeEquip
deEquip:type("deequip")
deEquip:slot("armor")
deEquip:id(2466, 2472, 2476, 2487, 2492, 2500, 2503, 2660, 7884, 7897, 7898, 7899, 8819, 8821, 8865, 8866, 8867, 8868, 8869, 8870, 8871, 8872, 8877, 8878, 8879, 8880, 8881, 8882, 8883, 8884, 8885, 8886, 8887, 8888, 8889, 8890, 8891, 8892, 9776, 11301, 11355, 12607, 12642, 12643, 15406, 15407, 15489, 18399, 18404, 21706, 21725)
deEquip:register()

-- Equip armor(s) by any vocation / level
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:id(2503, 21706)
equip:register()

-- Equip armor(s) for sorceres and druids at any level
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(8819, 8892, 8870, 8871)
equip:register()

-- Equip armor(s) for knights and paladins at any level
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:vocation("knight", true, true)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(2466, 2492, 2472, 2476, 2487)
equip:register()

-- Equip armor(s) for paladins at any level
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(2660, 8891, 8872)
equip:register()

-- Equip level 50 armor(s) for any vocation
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(50)
equip:id(8821)
equip:register()

-- Equip level 50 armor(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(50)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(7884, 7897, 7898, 7899)
equip:register()

-- Equip level 50 armor(s) for knights and paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(50)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(11301)
equip:register()

-- Equip level 60 armor(s) for sorcerer and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(60)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(11355, 11356)
equip:register()

-- Equip level 60 armor(s) for paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(60)
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(2500)
equip:register()

-- Equip level 60 armor(s) for knights and paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(60)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(8877, 8878, 8879, 8880)
equip:register()

-- Equip level 65 armor(s) for sorcerers
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(65)
equip:vocation("sorcerer", true, true)
equip:vocation("master sorcerer")
equip:id(8865)
equip:register()

-- Equip level 75 armor(s) for paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(75)
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(8885, 8886, 8887)
equip:register()

-- Equip level 75 armor(s) for sorcerers
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(75)
equip:vocation("sorcerer", true, true)
equip:vocation("master sorcerer")
equip:id(8867, 8868)
equip:register()

-- Equip level 75 armor(s) for druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(75)
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(8866, 8869)
equip:register()

-- Equip level 80 armor(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(80)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(9776)
equip:register()

-- Equip level 80 armor(s) for sorcerers and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(80)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(15489)
equip:register()

-- Equip level 85 armor(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(85)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(8889)
equip:register()

-- Equip level 100 armor(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(100)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(8881, 8882, 8883, 8884, 12642)
equip:register()

-- Equip level 100 armor(s) for sorcerers
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(100)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:id(8890)
equip:register()

-- Equip level 100 armor(s) for paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(100)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(8888)
equip:register()

-- Equip level 100 armor(s) for knights and paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(100)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(12607)
equip:register()

-- Equip level 100 armor(s) for sorcerers and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(100)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(12643)
equip:register()

-- Equip level 120 armor(s) for knights and paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(120)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(18404)
equip:register()

-- Equip level 130 armor(s) for sorcerers and druids
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(130)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(21725)
equip:register()

-- Equip level 150 armor(s) for paladins
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(150)
equip:vocation("paladin", true, true)
equip:vocation("royal paladin")
equip:id(15407)
equip:register()

-- Equip level 150 armor(s) for sorcerers and druids?
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(150)
equip:vocation("sorcerer", true, false)
equip:vocation("master sorcerer")
equip:vocation("druid", true, true)
equip:vocation("elder druid")
equip:id(18399)
equip:register()

-- Equip level 200 armor(s) for knights
local equip = MoveEvent()
equip.onEquip = defaultEquip
equip:type("equip")
equip:slot("armor")
equip:level(200)
equip:vocation("knight", true, false)
equip:vocation("elite knight")
equip:id(15406)
equip:register()
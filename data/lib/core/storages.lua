Storage = {
	-- General storages
	isTraining = 30000,
	NpcExhaust = 30001,
	NpcExhaustOnBuy = 30002,
	KawillBlessing = 30014,
	RentedHorseTimer = 30015,
	-- Promotion Storage cannot be changed, it is set in source code
	Promotion = 30018,
	combatProtectionStorage = 30023,
	Factions = 30024,
	blockMovementStorage = 30025,
	PetSummon = 30026,
	NpcSpawn = 30028,
	ExerciseDummyExhaust = 30029,
	StrawberryCupcake = 30032,
	StoreExaust = 30051,
	LemonCupcake = 30052,
	BlueberryCupcake = 30053,
	PetSummonEvent10 = 30054,
	PetSummonEvent60 = 30055,
	FreeQuests = 990000,
	PremiumAccount = 998899,

	-- Reserverd from Global
	Grimvale = {
		-- Reserved storage from 50380 - 50399
		WereHelmetEnchant = 50381
	},
	ForgottenKnowledge = {
		-- Reserved storage from 50470 - 50519
		AccessDeath = 50470,
		AccessViolet = 50471,
		AccessEarth = 50472,
		AccessFire = 50473,
		AccessIce = 50474,
		AccessGolden = 50475,
		AccessLast = 50476,
		OldDesk = 50477,
		GirlPicture = 50478,
		SilverKey = 50479,
		Phial = 50480,
		BirdCounter = 50481,
		PlantCounter = 50482,
		GoldenServantCounter = 50483,
		DiamondServantCounter = 50484,
		AccessPortals = 50485,
		AccessMachine = 50486,
		LadyTenebrisTimer = 50487,
		LadyTenebrisKilled = 50488,
		LloydTimer = 50489,
		LloydKilled = 50490,
		ThornKnightTimer = 50491,
		ThornKnightKilled = 50492,
		DragonkingTimer = 50493,
		DragonkingKilled = 50494,
		HorrorTimer = 50495,
		HorrorKilled = 50496,
		TimeGuardianTimer = 50497,
		TimeGuardianKilled = 50498,
		LastLoreTimer = 50499,
		LastLoreKilled = 50501,
		BirdCage = 50502,
		AccessLavaTeleport = 50503,
		Ivalisse = 50504,
		Chalice = 50505,
		BabyDragon = 50507,
		SpiderWeb = 50508
	},
	OutfitQuest = {
		-- Golden Outfit
		GoldenOutfit = 51015
	},
	AdventurersGuild = {
		Stone = 52130,
	},
	TheOrderOfTheLion = {
		-- Reserved storage 52360-52395 (TheRookieGuard)
		-- Reserved storage 52396-52410 (TheOrderOfTheLion)
		Drume = {
			Commander = 52396, -- Global
			TotalLionCommanders = 52397, -- Global
			TotalUsurperCommanders = 52398, -- Global
			Timer = 52399
		},
	},
}

GlobalStorage = {
	ExpBoost = 65004,
	XpDisplayMode = 65006,
	CobraBastionFlask = 65012,
	Tomes = 50506,
}

-- Values extraction function
local function extractValues(tab, ret)
	if type(tab) == "number" then
		table.insert(ret, tab)
	else
		for _, v in pairs(tab) do
			extractValues(v, ret)
		end
	end
end

local benchmark = os.clock()
local extraction = {}
extractValues(Storage, extraction) -- Call function
table.sort(extraction) -- Sort the table
-- The choice of sorting is due to the fact that sorting is very cheap O (n log2 (n))
-- And then we can simply compare one by one the elements finding duplicates in O(n)

-- Scroll through the extracted table for duplicates
if #extraction > 1 then
	for i = 1, #extraction - 1 do
		if extraction[i] == extraction[i+1] then
			Spdlog.warn(string.format("Duplicate storage value found: %d",
				extraction[i]))
			Spdlog.warn(string.format("Processed in %.4f(s)", os.clock() - benchmark))
		end
	end
end

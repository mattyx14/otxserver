Storage = {
	-- General storages
	isTraining = 30000,
	NpcExhaust = 30001,
	-- Promotion Storage cannot be changed, it is set in source code
	Promotion = 30018,
	combatProtectionStorage = 30023,
	Factions = 30024,
	blockMovementStorage = 30025,
	FamiliarSummon = 30026,
	TrainerRoom = 30027,
	NpcSpawn = 30028,
	ExerciseDummyExhaust = 30029,
	StoreExaust = 30051,
	LemonCupcake = 30052,
	BlueberryCupcake = 30053,
	FamiliarSummonEvent10 = 30054,
	FamiliarSummonEvent60 = 30055,
	FreeQuests = 30057,
	PremiumAccount = 30058,

	--[[
	Old storages
	Over time, this will be dropped and replaced by the table above
	]]
	Grimvale = {
		-- Reserved storage from 50380 - 50399
		WereHelmetEnchant = 50381
	},
	KillingInTheNameOf = {
		-- Reserved storage from 51610 - 51629
		LugriNecromancers = 51610,
		BudrikMinos = 51611,
		MissionTiquandasRevenge = 51612,
		MissionDemodras = 51613,
		BossPoints = 51614,
		QuestLogEntry = 51615,
		PawAndFurRank = 51616,
		GreenDjinnTask = 51617,
		BlueDjinnTask = 51618,
		PirateTask = 51619,
		TrollTask = 51620,
		GoblinTask = 51621,
		RotwormTask = 51622,
		CyclopsTask = 51623
	},
	DemonOak = {
		-- Reserved storage from 51700 - 51709
		Done = 51700,
		Progress = 51701,
		Squares = 51702,
		AxeBlowsBird = 51703,
		AxeBlowsLeft = 51704,
		AxeBlowsRight = 51705,
		AxeBlowsFace = 51706
	},
	AdventurersGuild = {
		-- Reserved storage from 52130 - 52159
		Stone = 52130
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
	}
}

GlobalStorage = {
	ExpBoost = 65004,
	XpDisplayMode = 65006,
	CobraBastionFlask = 65012,
	UglyMonster = 65017,
	KeysUpdate = 40000, -- Reserved storage from 40000 - 40000
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
		end
	end
end

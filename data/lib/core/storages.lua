Storage = {
	-- General storages
	isTraining = 30000,
	NpcExhaust = 30001,
	RentedHorseTimer = 30015,
	-- Promotion Storage cannot be changed, it is set in source code
	Promotion = 30018,
	combatProtectionStorage = 30023,
	Factions = 30024,
	blockMovementStorage = 30025,
	FamiliarSummon = 30026,
	ExerciseDummyExhaust = 30029,
	StrawberryCupcake = 30032,
	StoreExaust = 30051,
	LemonCupcake = 30052,
	BlueberryCupcake = 30053,
	FamiliarSummonEvent10 = 30054,
	FamiliarSummonEvent60 = 30055,
	PremiumAccount = 30058,

	--[[
	Old storages
	Over time, this will be dropped and replaced by the table above
	]]
	Grimvale = {
		-- Reserved storage from 50380 - 50399
		SilverVein = 50380,
		WereHelmetEnchant = 50381
	},
	OutfitQuest = {
		-- Reserved storage from 50960 - 51039
		-- Until all outfit quests are completed
		DefaultStart = 50960,
		Ref = 50961,
		-- Golden Outfit
		GoldenOutfit = 51015,
	},
	AdventurersGuild = {
		-- Reserved storage from 52130 - 52159
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

	-- OTX Reserved
	-- Star 30100
	NpcSpawn = 30100,
	DefaultStartQuest = 30101,
	AnnihilatorDone = 30102,
	ForgottenKnowledge = {
		Tomes = 30111,
	},
	DemonOak = {
		Done = 30112,
		Progress = 30113,
		Squares = 30114,

		AxeBlowsBird = 30115,
		AxeBlowsLeft = 30116,
		AxeBlowsRight = 30117,
		AxeBlowsFace = 30118
	},
	FirstQuest = {
		FirstWeapon = 30140,
		FirstWeaponClub = 30266,
		FirstWeaponSword = 30267,
		FirstWeaponAxe = 30268
	},
	OutfitQuest = {
		DefaultStart = 30223,
		Ref = 30224,
		-- Citizen Addons Quest
		Citizen = {
			-- Mission storages for temporary questlog entries
			MissionHat = 30225,
			AddonHat = 30226,
			MissionBackpack = 30227,
			AddonBackpack = 30228,
			AddonBackpackTimer = 30229
		},
		-- Hunter Addons Quest
		HunterHatAddon = 30230,
		Hunter = {
			AddonGlove = 30231,
			AddonHat = 30232
		},
		-- Knight Addons Quest
		Knight = {
			AddonSword = 30233,
			MissionHelmet = 30234,
			AddonHelmet = 30235,
			AddonHelmetTimer = 30236,
			RamsaysHelmetDoor = 30237
		},
		-- Warrior-outfit Quest
		WarriorShoulderAddon = 30238,
		WarriorSwordAddon = 30239,
		WarriorShoulderTimer = 30240,
		-- Mage/Summoner-outfit Quest
		MageSummoner = {
			AddonWand = 30241,
			AddonBelt = 30242,
			MissionHatCloak = 30243,
			AddonHatCloak = 30244,
			AddonWandTimer = 30245
		}
	},
}

GlobalStorage = {
	ExpBoost = 65004,
	XpDisplayMode = 65006,
	OberonEventTime = 65009,
	ScarlettEtzelEventTime = 65011,
	CobraBastionFlask = 65012
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

-- The OTX Server Config

	-- Owner Data
	ownerName = "Mattyx14"
	ownerEmail = "darkylive@live.com.mx"
	url = ""
	site = ""
	host = "https://github.com/mattyx14/otxserver/tree/otxserv2"
	location = ""
	advertisingBlock = "ba1ak;ip;b4iak;b4i4k;abertura;h0st;crle;c0nta;br;otserv;20.;21;sv;ddns;inaugur;.com;online;war;baiak;192;m4caco;m4c4co;m4c4c0;pr3to;pr3t0;191;sv;balan;macaco;preto;b@i@k;net;servegame;org;pl;biz;tk;mapa;iiak;web;www;35;34;21"

	-- Messages
	motd = "Welcome to the OTX Server!"
	serverName = "OTXSERVER"
	loginMessage = "Welcome to The OTX Server!"
	displayGamemastersWithOnlineCommand = false

	-- Misc
	-- mw replace system !mw old/new
	useMwReplaceSystem = false
	mwSpriteId = 10181
	newSpriteIdMW = 3642
	-- use max connection in same IP
	UseMaxIpConnect = true
	MaxIpConnections = 10
	-- use autoloot system
	Autoloot_enabled = false
	AutoLoot_BlockIDs = "" 
	AutoLoot_MoneyIDs = "2148;2152;2160;9971" 
	AutoLoot_MaxItem = 5
	AutoLoot_MaxItemPremium = 10
	AutoLoot_MaxItemFree = 5
	-- add if enable cast without password 
	expInCast = true
	expPercentIncast = 5
	-- use life/mana in percentual 100/100
	lifeAndManaInPercentual = false
	-- add frag if player kill mc / same ip
	addFragToSameIp = false
	-- use reset system
	resetSystemEnable = false
	-- use max absorbAll (prevents absorb +100% / SSA + might ring)
	useMaxAbsorbAll = true
	maxAbsorbPercent = 80.0
	-- delete player with monster/forbidden name?
	deletePlayersWithMonsterName = true
	forbiddenNames = "gm;adm;cm;support;god;tutor;god ; god; adm;adm ; gm;gm ; cm;cm ;"
	-- display messages death channel on death
	displayDeathChannelMessages = true
	-- modify damage to K 219000 -> 219.0K
	modifyDamageInK = false
	-- modify exp to K/mi 2.000.000 exp -> 2.0 mi
	modifyExperienceInK= false
	-- display broadcast in distro
	displayBroadcastLog = true
	-- enable dodge and critical in source (by storage)
	enableCriticalAndDodgeSource = true
	-- player can push other player in PZ
	pushInProtectZone = false
	-- set time to exhaust potion 1500 = 1.5s
	exhaustPotionMiliSeconds = 1500
	-- when using potion, it will exhaust other use item, MW / SD etc..., don't let use pot + rune
	exhaustItemAtUsePotion = true
	-- exhaust for the spectator speak again in seconds
	exhaust_spectatorSay = 5
	-- monster can walk in field if no have way to attack player
	monstersCanWalkInFields = true

	-- MySql
	sqlType = "mysql"
	sqlHost = "127.0.0.1"
	sqlPort = 3306
	sqlUser = "root"
	sqlPass = ""
	sqlDatabase = ""
	sqlFile = "schemas/otxserver.s3db"
	sqlKeepAlive = 0
	mysqlReadTimeout = 10
	mysqlWriteTimeout = 10
	mysqlReconnectionAttempts = 5
	encryptionType = "sha1" --// encryptionType can be (plain, sha1).

	-- World / Ip / Port
	worldId = 0
	ip = "127.0.0.1"
	worldType = "open"
	bindOnlyGlobalAddress = false
	loginPort = 7171
	gamePort = "7172"
	statusPort = 7171
	loginOnlyWithLoginServer = false
	blockedVps = "google;amazon;amazon.com;oracle;azure;vultr;google.com"
	permitedVps = 0

	-- Account manager
	accountManager = true
	namelockManager = true
	newPlayerChooseVoc = true
	newPlayerSpawnPosX = 159
	newPlayerSpawnPosY = 387
	newPlayerSpawnPosZ = 6
	newPlayerTownId = 2
	newPlayerLevel = 8
	newPlayerMagicLevel = 0
	generateAccountNumber = false
	generateAccountSalt = true

	-- Limits on frags / Time
	useFragHandler = true
	fragsLimit = 24 * 60 * 60
	fragsSecondLimit = 7 * 24 * 60 * 60
	fragsThirdLimit = 30 * 24 * 60 * 60

	-- Red Skull Config
	fragsToRedSkull = 3
	fragsSecondToRedSkull = 5
	fragsThirdToRedSkull = 10
	redSkullLength = 3 * 24 * 60 * 60

	-- Black Skull Config
	fragsToBlackSkull = 6
	fragsSecondToBlackSkull = 10
	fragsThirdToBlackSkull = 20
	blackSkulledDeathHealth = 40
	blackSkulledDeathMana = 0
	blackSkullLength = 6 * 24 * 60 * 60
	useBlackSkull = true

	-- Banishment Config
	-- killsBanLength works only if useBlackSkull option is disabled.
	notationsToBan = 3
	warningsToFinalBan = 4
	warningsToDeletion = 5
	banLength = 7 * 24 * 60 * 60
	killsBanLength = 7 * 24 * 60 * 60
	finalBanLength = 30 * 24 * 60 * 60
	ipBanLength = 1 * 24 * 60 * 60
	fragsToBanishment = 7
	fragsSecondToBanishment = 21
	fragsThirdToBanishment = 41

	-- Battle
	-- NOTE: showHealth/ManaChangeForMonsters inherites from showHealth/ManaChange.
	protectionLevel = 1
	pvpTileIgnoreLevelAndVocationProtection = true
	pzLocked = 60 * 1000
	pzlockOnAttackSkulledPlayers = false
	huntingDuration = 60 * 1000
	criticalHitMultiplier = 1
	displayCriticalHitNotify = true
	removeWeaponAmmunition = true
	removeWeaponCharges = true
	removeRuneCharges = true
	whiteSkullTime = 15 * 60 * 1000
	advancedFragList = true
	useFragHandler = true
	noDamageToSameLookfeet = false
	showHealthChange = true
	showManaChange = true
	showHealthChangeForMonsters = true
	showManaChangeForMonsters = true
	fieldOwnershipDuration = 5 * 1000
	stopAttackingAtExit = true
	loginProtectionPeriod = 5 * 1000
	diagonalPush = true
	deathLostPercent = 10
	stairhopDelay = 1 * 1000
	pushCreatureDelay = 1 * 1000
	deathContainerId = 1987
	gainExperienceColor = 215
	addManaSpentInPvPZone = true
	recoverManaAfterDeathInPvPZone = true
	squareColor = 0
	broadcastBanishments = false
	maxViolationCommentSize = 60
	violationNameReportActionType = 2
	optionalWarAttackableAlly = true
	fistBaseAttack = 7
	criticalHitChance = 7
	noDamageToGuildMates = false
	noDamageToPartyMembers = true

	-- Optional PVP
	optionalProtection = false

	-- Corpse Block
	-- If set to true, players won't be able to immediately throw fields on top of corpses after killing the monster
	allowCorpseBlock = false

	-- Rook System
	rookLevelTo = 5
	rookLevelToLeaveRook = 8
	rookTownId = 1
	useRookSystem = true

	-- Paralyze delay
	paralyzeDelay = 1500

	-- Depot
	-- set playerFollowExhaust to 2000 if someone causes lags and kicks by following unreachable creatures too often
	useCapacity = true
	defaultDepotSize = 400
	defaultDepotSizePremium = 400
	enableProtectionQuestForGM = false
	cleanItemsInMap = false
	playerFollowExhaust = 500

	-- 8.7x extras
	monsterSpawnWalkback = false
	allowBlockSpawn = true
	classicEquipmentSlots = true

	-- Summons and monsters
	NoShareExpSummonMonster = false

	-- Others
	enableLootBagDisplay = true
	highscoreDisplayPlayers = 10
	updateHighscoresAfterMinutes = 60
	attackImmediatelyAfterLoggingIn = false
	exhaustionNPC = true
	exhaustionInSecondsNPC = 0.5

	-- Advanced Version
	-- Note: If you use another protocol than the one we set as you will have functional failures.
	-- Supported (860) = 8.60
	manualVersionConfig = true
	versionMin = 860
	versionMax = 860
	versionMsg = "Only clients with protocol 8.60 allowed!"

	-- Connection config
	loginTries = 20
	retryTimeout = 5 * 1000
	loginTimeout = 60 * 1000
	maxPlayers = 200
	useFilaOnStartup = true
	displayOnOrOffAtCharlist = false
	onePlayerOnlinePerAccount = true
	allowClones = 0
	statusTimeout = 1000
	replaceKickOnLogin = true
	forceSlowConnectionsToDisconnect = false
	premiumPlayerSkipWaitList = true
	packetsPerSecond = 50
	loginProtectionTime = 5
	tibiaClassicSlots = true

	-- Death List and Blessings
	-- Function retroPVP true change it:
	-- deathAssistCount to 1
	-- useFairfightReduction to false
	-- fairFightTimeRange = 30
	deathListEnabled = true
	deathListRequiredTime = 1 * 60 * 1000
	deathAssistCount = 20
	maxDeathRecords = 5
	multipleNames = false

	-- Guilds
	-- NOTE: externalGuildWarsManagement supports Automatic Account Creator(webpage or whatever you want)
	externalGuildWarsManagement = false
	ingameGuildManagement = true
	levelToFormGuild = 20
	premiumDaysToFormGuild = 0
	guildNameMinLength = 4
	guildNameMaxLength = 20

	-- Houses
	buyableAndSellableHouses = true
	houseNeedPremium = true
	bedsRequirePremium = true
	levelToBuyHouse = 20
	housesPerAccount = 1
	houseRentAsPrice = false
	housePriceAsRent = false
	housePriceEachSquare = 1000
	houseRentPeriod = "weekly"
	houseCleanOld = 8 * 24 * 60 * 60
	guildHalls = true
	houseSkipInitialRent = true
	houseProtection = true

	-- Item usage
	timeBetweenActions = 200
	timeBetweenExActions = 1000
	timeBetweenCustomActions = 500
	checkCorpseOwner = true
	hotkeyAimbotEnabled = true
	maximumDoorLevel = 999
	tradeLimit = 100
	canOnlyRopePlayers = true

	-- Map
	-- NOTE: storeTrash costs more memory, but will perform alot faster cleaning.
	-- houseDataStorage usage may be found at how-use-internal-functions.log
	mapAuthor = "Komic"
	randomizeTiles = true
	houseDataStorage = "binary-tilebased"
	storeTrash = true
	cleanProtectedZones = true
	mapName = "forgotten.otbm"

	-- Mailbox
	mailMaxAttempts = 5
	mailBlockPeriod = 30 * 60 * 1000
	mailAttemptsFadeTime = 5 * 60 * 1000
	mailboxDisabledTowns = ""
		-- Example disable rook depot (temple) "4"
		-- mailboxDisabledTowns = "4"

	-- Startup
	-- For Linux use "-1" is default
	-- daemonize works only on *nix, same as niceLevel
	daemonize = false
	defaultPriority = "higher"
	niceLevel = 5
	serviceThreads = 1
	coresUsed = "-1"
	startupDatabaseOptimization = true
	removePremiumOnInit = true
	confirmOutdatedVersion = false
	skipItemsVersionCheck = false

	-- Muted buffer
	maxMessageBuffer = 4

	-- Miscellaneous
	dataDirectory = "data/"
	logsDirectory = "data/logs/"
	disableOutfitsForPrivilegedPlayers = false
	bankSystem = true
	spellNameInsteadOfWords = false
	emoteSpells = true
	unifiedSpells = true
	promptExceptionTracerErrorBox = true
	storePlayerDirection = false
	savePlayerData = true
	monsterLootMessage = 3
	monsterLootMessageType = 25
	separateViplistPerCharacter = false
	vipListDefaultLimit = 20
	vipListDefaultPremiumLimit = 100

	-- Outfits
	allowChangeOutfit = true
	allowChangeColors = true
	allowChangeAddons = true
	addonsOnlyPremium = true

	-- Ghost mode
	ghostModeInvisibleEffect = false
	ghostModeSpellEffects = true

	-- Limits
	-- Tile Limits set to 0 for prevent crash
	idleWarningTime = 14 * 60 * 1000
	idleKickTime = 15 * 60 * 1000
	expireReportsAfterReads = 1
	playerQueryDeepness = -1
	protectionTileLimit = 10
	houseTileLimit = 10
	tileLimit = 10

	-- Premium-related
	freePremium = false
	premiumForPromotion = true
	updatePremiumStateAtStartup = true

	-- Blessing
	blessings = true
	blessingOnlyPremium = false
	blessingReductionBase = 30
	blessingReductionDecrement = 5
	eachBlessReduction = 8
	useFairfightReduction = true
	pvpBlessingThreshold = 20
	fairFightTimeRange = 60

	-- Rates
	experienceStages = false
	rateExperience = 5
	rateExperienceFromPlayers = 0
	levelToOfflineTraining = 8
	rateSkill = 1.0
	rateSkillOffline = 0.5
	rateMagic = 1.0
	rateMagicOffline = 0.5
	rateLoot = 2.0
	rateSpawn = 1
	rateSpawnMin = 1
	rateSpawnMax = 2
	formulaLevel = 5.0
	formulaMagic = 1.0
	rateMonsterHealth = 1.0
	rateMonsterMana = 1.0
	rateMonsterAttack = 0.8
	rateMonsterDefense = 1.5

	-- Experience from players
	minLevelThresholdForKilledPlayer = 0.9
	maxLevelThresholdForKilledPlayer = 1.1

	-- Stamina System
	-- NOTE: The Stamina gain will only start counting after 10 minutes of being offline
	-- rateStaminaLoss = The amount of Stamina(in minutes) a player will loose after 1 minute of hunting
	-- rateStaminaGain = The amount of time(in minutes) a player will need to be offline to get 1 minute of Stamina(from 0 stamina to 'staminaRatingLimitTop')
	-- rateStaminaThresholdGain = The amount of time(in minutes) a player will need to be offline to get 1 minute of Stamina(from 'staminaRatingLimitTop' to full stamina)
	-- staminaRatingLimitTop = Above this Stamina players will receive 'rateStaminaAboveNormal' times the amount of the exp
	-- staminaRatingLimitBottom = Below this Stamina players will receive 'rateStaminaUnderNormal' times the amount of the exp
	-- staminaLootLimit = Below this Stamina players will no longer receive any loot
	-- rateStaminaAboveNormal = The experience multiplier for players above 'staminaRatingLimitTop' Stamina(1.5 means the player will receive 50% extra experience)
	-- rateStaminaUnderNormal = The experience multiplier for players below 'staminaRatingLimitBottom' Stamina(0.5 means the player will receive only half the experience)
	rateStaminaLoss = 1
	rateStaminaGain = 3
	rateStaminaThresholdGain = 12
	staminaRatingLimitTop = 40 * 60
	staminaRatingLimitBottom = 14 * 60
	staminaLootLimit = 14 * 60
	rateStaminaAboveNormal = 1.5
	rateStaminaUnderNormal = 0.5
	staminaThresholdOnlyPremium = true

	-- Party System
	experienceShareRadiusX = 30
	experienceShareRadiusY = 30
	experienceShareRadiusZ = 1
	experienceShareLevelDifference = 2 / 3
	extraPartyExperienceLimit = 20
	extraPartyExperiencePercent = 5
	experienceShareActivity = 2 * 60 * 1000

	-- Party Multiplier Experience
	enablePartyVocationBonus = true
	twoVocationExpMultiplier = 1.4
	threeVocationExpMultiplier = 1.6
	fourVocationExpMultiplier = 2.0

	-- Global save
	globalSaveEnabled = false
	globalSaveHour = 8
	globalSaveMinute = 0
	shutdownAtGlobalSave = true
	cleanMapAtGlobalSave = false

	-- Spawns
	minRateSpawn = 1
	maxRateSpawn = 3
	deSpawnRange = 2
	deSpawnRadius = 50

	-- Summons
	maxPlayerSummons = 2
	teleportAllSummons = false
	teleportPlayerSummons = true

	-- Logs
	disableLuaErrors = false
	adminLogs = true
	displayPlayersLogging = true
	prefixChannelLogs = ""
	runFile = "server/run.log"
	outputLog = "server/out.log"
	truncateLogOnStartup = false
	logPlayersStatements = false

	-- Manager
	-- NOTE: managerPassword left blank disables manager.
	managerPort = 7171
	managerLogs = true
	managerPassword = ""
	managerLocalhostOnly = true
	managerConnectionsLimit = 1

	-- Admin
	-- NOTE: adminPassword left blank disables manager.
	-- Set to anything if you set adminRequireLogin to false.
	-- adminEncryption available options: rsa1024xtea;
	-- remember to set correct data!
	adminPort = 7171
	adminPassword = ""
	adminLocalhostOnly = true
	adminConnectionsLimit = 1
	adminRequireLogin = true
	adminEncryption = ""
	adminEncryptionData = ""

	-- Don't edit use at your own risk
	saveGlobalStorage = true
	bufferMutedOnSpellFailure = false

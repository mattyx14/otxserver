function onUpdateDatabase()
	print("> Updating database to version 27 (Fix bonus reroll and wrap/unwrap casks)")
	db.query("ALTER TABLE `players` DROP COLUMN `bonus_reroll`")
	db.query("ALTER TABLE `players` ADD COLUMN `bonus_reroll` int(11) NOT NULL DEFAULT '0' AFTER `prey_column`")
	db.query("ALTER TABLE `store_history` ADD COLUMN `timestamp` int(11) NOT NULL DEFAULT 0 AFTER `time`")
	return true
end

function onUpdateDatabase()
	print("> Updating database to version 24 (OTX Migrations)")
	--db.query("UPDATE `player_depotitems` SET `pid` = 17 WHERE `pid` = 0")
	--db.query("UPDATE `player_depotitems` SET `pid` = 17 WHERE `pid` > 17")
	db.query("CREATE TABLE IF NOT EXISTS `player_rewards` ( `player_id` int(11) NOT NULL, `sid` int(11) NOT NULL, `pid` int(11) NOT NULL DEFAULT '0', `itemtype` smallint(6) NOT NULL, `count` smallint(5) NOT NULL DEFAULT '0', `attributes` blob NOT NULL, UNIQUE KEY `player_id_2` (`player_id`, `sid`), FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE ) ENGINE=InnoDB;")
	return true
end

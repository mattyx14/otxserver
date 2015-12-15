function onUpdateDatabase()
	print("> Updating database to version 19 (Reward Boss System)")
	db.query("CREATE TABLE IF NOT EXISTS `player_rewards` (`player_id` int(11) NOT NULL, `sid` int(11) NOT NULL COMMENT 'any given range eg 0-100 will be reserved for reward containers and all > 100 will be then normal items inside the bag', `pid` int(11) NOT NULL DEFAULT '0', `itemtype` smallint(6) NOT NULL, `count` smallint(5) NOT NULL DEFAULT '0', `attributes` blob NOT NULL, UNIQUE KEY `player_id_2` (`player_id`, `sid`), FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE) ENGINE=InnoDB;")
	return true
end

function onUpdateDatabase()
	print("> Updating database to version 19 (Old Skull System)")
	db.query([[CREATE TABLE IF NOT EXISTS `player_murders` (`id` bigint(20) NOT NULL, `player_id` int(11) NOT NULL, `date` bigint(20) NOT NULL) ENGINE=MyISAM DEFAULT CHARSET=utf8;]])
	return true
end

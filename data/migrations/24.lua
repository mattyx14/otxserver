function onUpdateDatabase()
	print("> Updating database to version 24 (Unjust Panel Kills)")
	db.query("CREATE TABLE IF NOT EXISTS `player_kills` (`player_id` INT(11) NOT NULL , `time` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' , `target` INT(11) NOT NULL , `unavenged` BOOLEAN NOT NULL DEFAULT FALSE, FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE ) ENGINE = InnoDB;")
	return true
end

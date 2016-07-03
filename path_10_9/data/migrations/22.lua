function onUpdateDatabase()
	print("> Updating database to version 22 (Reward System)")
	db.query([[CREATE TABLE `player_rewards` ( `player_id` int(11) NOT NULL, `sid` int(11) NOT NULL, `pid` int(11) NOT NULL DEFAULT '0', `itemtype` smallint(6) NOT NULL, `count` smallint(5) NOT NULL DEFAULT '0', `attributes` blob NOT NULL ) ENGINE=InnoDB DEFAULT CHARSET=latin1;]])
	return true
end

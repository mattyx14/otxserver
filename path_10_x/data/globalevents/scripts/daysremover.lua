function onTimer()
	db.executeQuery("UPDATE accounts SET vipdays = vipdays - 1 WHERE vipdays > 0;")

	return true
end

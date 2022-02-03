-- Minlevel and multiplier are MANDATORY
-- Maxlevel is OPTIONAL, but is considered infinite by default
-- Create a stage with minlevel 1 and no maxlevel to disable stages
experienceStages = {
	{
		minlevel = 1,
		maxlevel = 50,
		multiplier = 20
	},
	{
		minlevel = 51,
		maxlevel = 100,
		multiplier = 15
	},
	{
		minlevel = 101,
		maxlevel = 200,
		multiplier = 10
	},
	{
		minlevel = 201,
		maxlevel = 400,
		multiplier = 7
	},
	{
		minlevel = 401,
		maxlevel = 600,
		multiplier = 5
	},
	{
		minlevel = 601,
		multiplier = 3
	}
}

skillsStages = {
	{
		minlevel = 1,
		maxlevel = 600,
		multiplier = 50
	},
	{
		minlevel = 601,
		multiplier = 60
	}
}

magicLevelStages = {
	{
		minlevel = 1,
		maxlevel = 600,
		multiplier = 25
	},
	{
		minlevel = 601,
		multiplier = 30
	}
}

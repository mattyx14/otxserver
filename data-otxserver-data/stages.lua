-- Minlevel and multiplier are MANDATORY
-- Maxlevel is OPTIONAL, but is considered infinite by default
-- Create a stage with minlevel 1 and no maxlevel to disable stages
experienceStages = {
	{
		minlevel = 1,
		maxlevel = 100,
		multiplier = 5
	},
	{
		minlevel = 101,
		maxlevel = 200,
		multiplier = 4.5
	},
	{
		minlevel = 201,
		maxlevel = 450,
		multiplier = 3.5
	},
	{
		minlevel = 451,
		maxlevel = 1000,
		multiplier = 3
	},
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

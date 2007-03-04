-- The start-up script
LogMessage("Up and running. =)");

-- Set up the playlist
RunScript("playlist.lua");

-- Declare some stuff

-- Start a new game
function NewGame()
	separator14 = 0;
	separator25 = 0;
	separator36 = 0;
end


-- Utility function for random real numbers between [i, j]
function RangeRandom(i, j)
	return i + math.random()*(j-i);
end

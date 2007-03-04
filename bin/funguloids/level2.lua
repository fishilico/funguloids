-- Level 2 script

-- Level dimensions
playfieldWidth = 100.0;
playfieldHeight = 100.0;

-- Next level
nextLevel = "level3.lua";

-- Background image
local bg = math.random(1,2);


-- Creates the level background
function BackgroundCreate()
	starsCount = math.random(100,200);
	nebulaCount = math.random(8, 12);

	if(bg == 1) then
		nebula1_R = 0.15; nebula1_G = 0.2;  nebula1_B = 0.1;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.05; nebula3_G = 0.15; nebula3_B = 0.0;
		backgroundImage = "bg1.dds";
	else
		nebula1_R = 0.15; nebula1_G = 0.2;  nebula1_B = 0.1;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.10; nebula3_G = 0.15; nebula3_B = 0.0;
		backgroundImage = "bg5.dds";
	end
end


-- Creates the level lighting
function LightsCreate()
	if(bg == 1) then
		CreateLight("DistantLight", -0.5, 1.0, 0.7,  0.75,1,0.5);
	else
		CreateLight("SunLight",      -1,  0, 0.9,  0.4,5,0.01);
		CreateLight("DistantLight", -0.5, 1.0, 0.7,  0.75,1,0.5);
	end
	SetAmbientLight(0.02, 0.08, 0.02);
end


-- Creates the level objects
function LevelCreate()
	-- Create the bases
	CreateBase("Base", 1, 0, 0, nextLevel);
	playerHomeBase = "Base";


	-- Create asteroids
	for i = 0,7 do
		local mesh = "Asteroid" .. math.random(1,3) .. ".mesh";
		local scale = RangeRandom(1.8, 2.5);
		local x, y = GetFreePosition(0, 0, 10, 35);
		CreateAsteroid("Asteroid" .. i, mesh, x, y, scale, "Level2AsteroidMat");
	end



	-- Create mushrooms
	for i = 0,2 do
		local x, y = GetFreePosition(0, 0, 4);
		local color = i % 3;
		CreateMushroom("Mushroom" .. i, color, x, y);
	end


	-- Create whirler
	if(separator25 == 1) then
		local x, y = GetFreePosition(0, 0, 1);
		CreateWhirler("Whirler", x, y);
		levelRotation = 0;
	else
		-- Create tentacles
		CreateTentacles("Tentacles", 0, 0);
		levelRotation = 9.0;
	end

	-- Change level
	if(separator25 == 0) then
		separator25 = 1;
	else
		separator25 = 0;
	end
end





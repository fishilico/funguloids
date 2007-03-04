-- Level 1 script

-- Level dimensions
if(separator14 == 0) then
	playfieldWidth = 100.0;
	playfieldHeight = 100.0;
else
	playfieldWidth = 110.0;
	playfieldHeight = 110.0;
end


-- Next level
nextLevel = "level2.lua";

-- Background image
local bg = math.random(1,2);


-- Creates the level background
function BackgroundCreate()
	starsCount = math.random(100,200);
	nebulaCount = math.random(8, 12);

	if(bg == 1) then
		nebula1_R = 0.1;  nebula1_G = 0.15; nebula1_B = 0.2;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.05; nebula3_G = 0.0;  nebula3_B = 0.15;
		backgroundImage = "bg2.dds";
	else
		nebula1_R = 0.1;  nebula1_G = 0.15; nebula1_B = 0.2;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.05; nebula3_G = 0.10;  nebula3_B = 0.15;
		backgroundImage = "bg6.dds";
	end
end


-- Creates the level lighting
function LightsCreate()
	CreateLight("SunLight",      1,  -1, 0.5,  0.05,0.2,0.5);
	if(bg == 1) then
		CreateLight("DistantLight", 0.1, 1.0, 0.75,  1,0.75,1);
	else
		CreateLight("DistantLight", 0.1, 1.0, 0.75,  0.60,0.70,1);
	end
	SetAmbientLight(0.02, 0.02, 0.08);
end


-- Creates the level objects
function LevelCreate()
	-- Create the bases
	CreateBase("Base", 2, 0, 0, nextLevel);
	playerHomeBase = "Base";
	levelRotation = 0;

	-- Create asteroids
	for i = 0,7 do
		local mesh = "Asteroid" .. math.random(1,3) .. ".mesh";
		local scale = RangeRandom(1.9, 2.5);
		local x, y = GetFreePosition(0, 0, 10, 35);
		CreateAsteroid("Asteroid" .. i, mesh, x, y, scale);
	end


	-- Create mushrooms
	for i = 0,2 do
		local x, y = GetFreePosition(0, 0, 4);
		local color = i % 3;
		CreateMushroom("Mushroom" .. i, color, x, y);
	end


	-- Create whirler
	if(separator14 == 0) then
		local x, y = GetFreePosition(0, 0, 1);
		CreateWhirler("Whirler", x, y);
	else
		-- Create the Super Massive Black Hole
		CreateBlackHole("BlackHole1", 0, 40, 45.0, 6.0);
		CreateBlackHole("BlackHole2", 40, 0, 45.0, 6.0);
		CreateBlackHole("BlackHole3", 0, -40, 45.0, 6.0);
		CreateBlackHole("BlackHole4", -40, 0, 45.0, 6.0);
	end


	if(separator14 == 0) then
		separator14 = 1;
	else
		separator14 = 0;
	end
end

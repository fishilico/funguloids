-- Menu background "level" script

-- Level dimensions
playfieldWidth = 100.0;
playfieldHeight = 100.0;


-- Creates the level background
function BackgroundCreate()
	starsCount = math.random(100,200);
	nebulaCount = math.random(8, 12);

	local bg = math.random(1,2);
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
	CreateLight("DistantLight", 0.1, 1.0, 0.75,  1,0.75,1);
	SetAmbientLight(0.02, 0.02, 0.08);
end


-- Creates the level objects
function LevelCreate()


	-- Create asteroids
	for i = 0,8 do
		local mesh = "Asteroid" .. math.random(1,3) .. ".mesh";
		local scale = RangeRandom(1.9, 2.5);
		local x, y = GetFreePosition(0, 0, 10, 10);
		CreateAsteroid("Asteroid" .. i, mesh, x, y, scale);
	end


	-- Create mushrooms
	for i = 0,2 do
		local x, y = GetFreePosition(0, 0, 4);
		local color = i % 3;
		CreateMushroom("Mushroom" .. i, color, x, y);
	end
end

-- Special level script

-- Level dimensions
playfieldWidth = 90.0;
playfieldHeight = 90.0;


-- Creates the level background
function BackgroundCreate()
	starsCount = math.random(100,200);
	nebulaCount = math.random(8, 12);

	local bg = math.random(1,2);
	if(bg == 1) then
		nebula1_R = 0.3;  nebula1_G = 0.15; nebula1_B = 0.1;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.20; nebula3_G = 0.05;  nebula3_B = 0.1;
		backgroundImage = "bg4.png";
	else
		nebula1_R = 0.2;  nebula1_G = 0.15; nebula1_B = 0.05;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.10; nebula3_G = 0.12;  nebula3_B = 0.15;
		backgroundImage = "bg8.png";
	end
--	nebula1_R = 0.2;  nebula1_G = 0.15; nebula1_B = 0.1;
--	nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
--	nebula3_R = 0.15; nebula3_G = 0.0;  nebula3_B = 0.05;
end


-- Creates the level lighting
function LightsCreate()
	CreateLight("SunLight",      -1,  -1, -0.5,  0.75,0.5,0.1);
	CreateLight("DistantLight", 0.1, 1.0, 0.75,  1,1,1);
	SetAmbientLight(0.04, 0.04, 0.02);
end


-- Creates the level objects
function LevelCreate()
	-- Create the bases
	CreateBase("Base", 3, 0, 0, nextLevel);
	playerHomeBase = "Base";
	levelRotation = 0;


	-- Create mushrooms
	for i = 0,2 do
		local x, y = GetFreePosition(0, 0, 4);
		local color = i % 3;
		CreateMushroom("Mushroom" .. i, color, x, y);
	end
end

-- Level 3 script

-- Level dimensions
if(separator36 == 0) then
	playfieldWidth = 100.0;
	playfieldHeight = 100.0;
else
	playfieldWidth = 110.0;
	playfieldHeight = 100.0;
end


-- Next level
nextLevel = "level1.lua";


-- Creates the level background
function BackgroundCreate()
	starsCount = math.random(100,200);
	nebulaCount = math.random(8, 12);

	local bg = math.random(1,2);
	if(bg == 1) then
		nebula1_R = 0.2;  nebula1_G = 0.1;  nebula1_B = 0.15;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.15; nebula3_G = 0.0;  nebula3_B = 0.05;
		backgroundImage = "bg3.png";
	else
		nebula1_R = 0.2;  nebula1_G = 0.1;  nebula1_B = 0.15;
		nebula2_R = 0.0;  nebula2_G = 0.0;  nebula2_B = 0.0;
		nebula3_R = 0.15; nebula3_G = 0.05;  nebula3_B = 0.05;
		backgroundImage = "bg7.png";
	end
end


-- Creates the level lighting
function LightsCreate()
	CreateLight("DistantLight", -0.5, -1.0, 0.5,  1,0.25,0.1);
	SetAmbientLight(0.00, 0.00, 0.00);
end


-- Creates the level objects
function LevelCreate()
	-- Create the bases
	CreateBase("Base", 0, 0, 0, nextLevel);
	playerHomeBase = "Base";
	levelRotation = 0;

	-- Create asteroids
	local count;
	if(separator36 == 0) then
		count = 7;
	else
		count = 6;
	end
	for i = 0,count do
		local mesh = "Asteroid" .. math.random(1,3) .. ".mesh";
		local scale = RangeRandom(1.9, 2.5);
		local x, y = GetFreePosition(0, 0, 10, 35);
		CreateAsteroid("Asteroid" .. i, mesh, x, y, scale, "Level3AsteroidMat");
	end


	-- Create mushrooms
	for i = 0,2 do
		local x, y = GetFreePosition(0, 0, 4);
		local color = i % 3;
		CreateMushroom("Mushroom" .. i, color, x, y);
	end


	-- Create whirler
	if(separator36 == 0) then
		local x, y = GetFreePosition(0, 0, 1);
		CreateWhirler("Whirler", x, y);
	else
		-- Create ball worms
		CreateBallWorm("BallWorm1", 0, 0, 40, -0.3, 72.5);
		CreateBallWorm("BallWorm2", 0, 0, 40, 0.3, 72);
	end


	-- Change level
	if(separator36 == 0) then
		separator36 = 1;
	else
		separator36 = 0;
	end
end

function setDirection()
	math.randomseed(os.time());
	
	--Need to do this to get good random numbers
	--Otherwise, it just picks the same number over and over
	math.random(); math.random(); math.random();	
	chanceX = math.random(0,99);
	
	if chanceX < 33 then
		newX = 1;
		--newZ = 0;
		--yRotation = 90;
	elseif chanceX > 33 and chanceX < 66 then
		newX = 0;
	else
		newX = -1;
		--newZ = 1;
		--yRotation = 0
	end
	
	math.random(); math.random(); math.random();	
	chanceZ = math.random(0,99);
	
	if chanceZ < 33 then
		--newX = -1;
		newZ = 1;
		--yRotation = 270
	elseif chanceZ > 33 and chanceZ < 66 then
		newZ = 0
	else
		--newX = 0;
		newZ = -1;
		--yRotation = 180;
	end
	
	if newX == 1 and newZ == 0 then
		yRotation = 0;
	elseif newX == 1 and newZ == 1 then
		yRotation = 45;
	elseif newX == 0 and newZ == 1 then
		yRotation = 90;
	elseif newX == -1 and newZ == 1 then
		yRotation = 135;
	elseif newX == -1 and newZ == 0 then
		yRotation = 180;
	elseif newX == -1 and newZ == -1 then
		yRotation = 225;
	elseif newX == 0 and newZ == -1 then
		yRotation = 270;
	else 
		yRotation = 315;
	end
		
	return newX,newZ,yRotation;
	
end

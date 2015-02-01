function setDirection()
	math.randomseed(os.time());
	
	--Need to do this to get good random numbers
	--Otherwise, it just picks the same number over and over
	math.random(); math.random(); math.random();
	
	chance = math.random(0,100);
	--print(chance);
	
	if chance < 25 then
		newX = 1;
		newZ = 0;
		yRotation = 90;
	elseif chance > 25 and chance < 50 then
		newX = 0;
		newZ = 1;
		yRotation = 0
	elseif chance > 50 and chance < 75 then
		newX = -1;
		newZ = 0;
		yRotation = 270
	else
		newX = 0;
		newZ = -1;
		yRotation = 180;
	end
		
	return newX,newZ,yRotation;
	
end

function pickRandomLocation(size)
	math.randomseed(os.time());
	math.random(); math.random(); math.random();	
	
	selection = math.random(0,size-1);
	
	return selection
end

function setupClueMsg()
	--msg = "You have found the clue. The killer is in the top floor"
	msg = "Don't ask how I know this, but you will find the killer in the living room"
	
	return msg
end

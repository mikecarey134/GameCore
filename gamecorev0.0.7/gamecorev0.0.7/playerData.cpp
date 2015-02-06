
#include"playerData.h"

//sets state to something system will over write.
playerData::playerData() : state_(10)
{
	
}

playerData::~playerData()
{

}


//sets data as our current user. used to match incoming data
void playerData::setGuid(const std::string guid)
{
 	 guid_ = guid;
}

//returns guid stored to a remoteplayers data.
std::string playerData::getGuid()
{
	return guid_;
}

//used to parse incoming data.
std::istream& playerData::getInput(std::istream& in)
{
	char guid[20];
	int mType;

	in >> guid >> mType >> (float)cord_.X >>(float) cord_.Y >> (float) cord_.Z >> state_ >> weaponNum_; 
		//myDirection_  camPos_ 
	return in;
}

std::istream& operator>>(std::istream& in, playerData& ourPlayerData)
{
	ourPlayerData.getInput(in);
	return in;
};

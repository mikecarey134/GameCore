#include "NetworkData.h"


#include "remotePlayer.h"
#include <string.h>
#include <iostream>
#include <irrlicht.h>

NetworkData::NetworkData(irr::IrrlichtDevice* device,char* filename,
						 irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
						 irrklang::ISoundEngine* engine, irrBulletWorld* world) : currentPlayers_(0), maxPlayers_(8),
						 device_(device),smgr_(smgr),driver_(driver), engine_(engine), world_(world)
{
	//players_.reserve(MAX_PLAYERS);//reserve this as the max amount of players
}


NetworkData::~NetworkData()
	{

}


const char* NetworkData::getNetworkData() const
{
	char* sendingString = new char [ourNetworkData.str().length() + 1];
	strcpy(sendingString, ourNetworkData.str().c_str()); 
	return sendingString;
}


int NetworkData::length()
{
	dataSize_ = ourNetworkData.str().length();
	return dataSize_;
}


int NetworkData::chatLength()
{
	return ourNetworkChatMine.length();
}


void NetworkData::setString(player& thereInput)
{
	//set to 0 at start tell clients its PlayerData not chat
	ourNetworkData.str(""); 
	ourNetworkData << "0 " << thereInput.getname() << " " <<thereInput.getPosition().X 
		<< " " << thereInput.getPosition().Y << " " << thereInput.getPosition().Z << " " <<thereInput.getRotation().X 
		<< " " << thereInput.getRotation().Y << " " << thereInput.getRotation().Z;
}


//This function takes in data from the server and store it into a player data to be used 
//by the client 
void NetworkData::setRemote(const char* ourData)
{
	//get our remote information from the server
	std::string ID;
	std::string mType;
	std::stringstream ourID;
	float rem_x,rem_y,rem_z,rem_rotX,rem_rotY,rem_rotZ;
	ourID.str(ourData);
	ourID >> mType >> ID >>rem_x >>rem_y >>rem_z >> rem_rotX >> rem_rotY >> rem_rotZ;

	//is it playerData
	if( mType == "0")
	{
		bool notFound = true;
		std::stringstream otherPlayer;
		otherPlayer << ourData;

		//Update current Players
		for(int x =0; x < currentPlayers_ && notFound; x++)
		{
			//update the player based off the key from the server
			if(players_.find(ID) != players_.end())
			{
				otherPlayer >> ourPlayerData[x];
				players_[ID].setposition(irr::core::vector3df(rem_x,rem_y,rem_z));
				players_[ID].setRotation(irr::core::vector3df(rem_rotX,rem_rotY,rem_rotZ));
				players_[ID].drawName();
				notFound = false;//set the player not found to false to indicate an exsisting player
			}
		}

		//Add our new player
		if(notFound == true)
		{
			//add to next empty
			for(int x =0; x < 8 && notFound; x++)
			{
				//When you find empty one store it
				if(ourPlayerData[x].getState() == 10)
				{
					printf("New Player Joined");
					remotePlayer newPlayer(device_,"characters/stick_mike.ms3d",smgr_,driver_,engine_,world_);
					//newPlayer.setName(ID);
					//players_.push_back(newPlayer);
					players_[ID]= newPlayer;
					players_[ID].setName(ID);
					ourPlayerData[x].setGuid(ID);
					otherPlayer >> ourPlayerData[x];
					ourPlayerData->setState(-10);
					++currentPlayers_;
				

				

					break;
				}
			}
		}
		else
		{
			//if they where found set not found back to true
			notFound = true;
		}
	}
	else if(mType == "1")
	{
		//is it chat data
		//chat message
		char* temp = new char[100];
		int foundSpace = 0;
		

		int start = ID.length() + mType.length()+ 2;
		int length = strlen(ourData);
		
		for(int i=start;i<length;i++)
		{
			if(isalpha((unsigned char)ourData[i])) 
			{
				temp[i-start]=ourData[i];
			}
			else if(ourData[i] == ' ')
			{
				temp[i-start] = ' ';
			}
			else 
			{
				temp[i-start] = 0;
				break;
			}
			
		}

		//strcpy(temp, ourData);
		
		

		//ourData
		ourNetworkChatTheres = "";

		ourNetworkChatTheres = temp;

	}
	else if (mType == "99")
	{
		std::cout<<"Player:" << ID <<" left";
		players_[ID].delete_player();
		players_.erase(ID);
	}

}
//Get remote takes in a ref to a player in the game and sets all of its values based
//on information it receives from the server (stored in player data)
void NetworkData::getRemote(remotePlayer& worldRemote, int dataIndex)
{
	worldRemote.setposition(ourPlayerData[dataIndex].getworldCoord());
	//worldRemote.setCamPos(ourPlayerData[dataIndex].getcamPosition());
	//worldRemote.setDirection(ourPlayerData[dataIndex].getDirection());
	//worldRemote.setState(ourPlayerData[dataIndex].getState());
	//worldRemote.setPlayerWeapon(ourPlayerData[dataIndex].getWeapon());

}

//Sets the number max number of players we are allowing
int NetworkData::getMaxPlayers()
{
	return maxPlayers_;
}


//Used to transfer chat messages.
void NetworkData::setChatMessage(char* chat)
{
	std::string clientChat(chat);//make a conversion from c_string to string
	//set to 1 at start tell clients its chat data and not playerData
	ourNetworkChatMine = "1 ";
	ourNetworkChatMine += clientChat;

	//ourNetworkChatMine.append(clientChat);//append the message here
	
	//ourNetworkChatMine << "1 " << clientChat;
	
}

const std::string NetworkData::getChatMessage()
{	
	std::string temp;
	temp = ourNetworkChatMine;
	ourNetworkChatMine="";
	return temp;
}

const std::string NetworkData::getThereChatMessage()
{	
	std::string temp;
	temp = ourNetworkChatTheres;
	ourNetworkChatTheres="";
	return temp;
}
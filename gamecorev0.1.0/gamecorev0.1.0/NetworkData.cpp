#include "NetworkData.h"


#include "remotePlayer.h"
#include <string.h>
#include <iostream>
#include <irrlicht.h>

NetworkData::NetworkData(irr::IrrlichtDevice* device,char* filename,
						 irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
						 irrklang::ISoundEngine* engine, irrBulletWorld* world,ChatQueue* message_buffer) : currentPlayers_(0), maxPlayers_(8),
						 device_(device),smgr_(smgr),driver_(driver), engine_(engine), world_(world),message_buffer_(message_buffer){}

NetworkData::~NetworkData(){}

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
	std::string ID;//player_key
	std::string mType;//code from server 0 = player data 1 = chat 99 = quit
	std::stringstream ourID;
	float rem_x,rem_y,rem_z,rem_rotX,rem_rotY,rem_rotZ;
	ourID.str(ourData);
	ourID >> mType >> ID >>rem_x >>rem_y >>rem_z >> rem_rotX >> rem_rotY >> rem_rotZ;//set up incoming variables

	//is it playerData
	if( mType == "0")
	{
		bool notFound = true;
		std::stringstream otherPlayer;
		otherPlayer << ourData;

		//update the player based off the player_key from the server
		if(players_.find(ID) != players_.end())
		{
			players_[ID].setposition(irr::core::vector3df(rem_x,rem_y,rem_z));
			players_[ID].setRotation(irr::core::vector3df(rem_rotX,rem_rotY,rem_rotZ));
			players_[ID].drawName();
			notFound = false;//set the player not found to false to indicate an existing player
		}

		if(notFound == true)//Add our new player if they arent found
		{
			//tell the player someone has joined
			std::string join_msg = "<";
			join_msg+= ID;
			join_msg+="> ";
			join_msg+="Joined";
			message_buffer_->addMessage(join_msg.c_str(),irr::video::SColor(255,0,255,0));
			
			//add new player to map
			//printf("New Player Joined\n");
			std::cout<<ID<<" Joined\n";
			remotePlayer newPlayer(device_,"characters/stick_mike.ms3d",smgr_,driver_,engine_,world_);
			players_[ID]= newPlayer;
			players_[ID].setName(ID);
			++currentPlayers_;//update the current amount of players
	
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

		//show message in our chat queue
		std::string buffered ="<";
		buffered+=ID;
		buffered+= "> ";
		buffered+= temp;
		message_buffer_->addMessage(buffered.c_str());

	}
	else if (mType == "99")
	{
		//tell the players someone has left
		std::string quit_msg = "<";
		quit_msg+= ID;
		quit_msg+="> ";
		quit_msg+="Left";
		message_buffer_->addMessage(quit_msg.c_str(),irr::video::SColor(255,255,0,0));

		std::cout<< ID <<" left\n";
		players_[ID].delete_player();
		players_.erase(ID);
	}

}
//Get remote takes in a ref to a player in the game and sets all of its values based
//on information it receives from the server (stored in player data)
void NetworkData::getRemote(remotePlayer& worldRemote, int dataIndex)
{
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
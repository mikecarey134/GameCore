/*
//////////////////////////////////////////////////////////////////////////
NETWORKDATA.CPP
used to transport data from the client to the host
as well as store and remove remote players 
By: Michael Carey
//////////////////////////////////////////////////////////////////////////
*/


#include "NetworkData.h"
//#include "remotePlayer.h"
//#include <string.h>
//#include <iostream>
//#include <irrlicht.h>

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
		<< " " << thereInput.getRotation().Y << " " << thereInput.getRotation().Z << " " <<thereInput.get_model_type()
		<< " " << thereInput.get_curr_state();
}


//This function takes in data from the server and store it into a player data to be used 
//by the client 
void NetworkData::setRemote(const char* ourData)
{
	//get our remote information from the server
	std::string ID;//player_key
	std::string mType;//code from server 0 = player data 1 = chat 99 = quit
	std::string rem_msg;//player_message
	std::stringstream ourID;
	
	float rem_x,rem_y,rem_z,rem_rotX,rem_rotY,rem_rotZ;
	int model_type,current_state;
	ourID.str(ourData);
	ourID >> mType >> ID >>rem_x >>rem_y >>rem_z 
		>> rem_rotX >> rem_rotY >> rem_rotZ >> model_type >> current_state;//set up incoming variables

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
			players_[ID].setState(current_state);
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
			//remotePlayer newPlayer(device_,"characters/stick_mike.ms3d",smgr_,driver_,engine_,world_);
			remotePlayer newPlayer(device_,smgr_,driver_,engine_,world_,model_type);
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
	else if(mType == "1")//it is chat data
	{
		//chat message
		char* temp = new char[100];
		int foundSpace = 0;
		

		int start = ID.length() + mType.length()+ 2;//start where the message is
		int length = strlen(ourData);
		
		for(int i=start;i < length;i++)//copy the message contents 
		{
			temp[i-start]=ourData[i];
		}

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
	else if (mType == "99")//quit message
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

//Sets the number max number of players we are allowing
int NetworkData::getMaxPlayers()
{
	return maxPlayers_;
}

//Used to transfer chat messages.
void NetworkData::setChatMessage(char* chat)
{
	std::string clientChat(chat);     //make a conversion from c_string to string
	ourNetworkChatMine = "1 ";        //set to 1 at start tell clients its chat data and not playerData
	ourNetworkChatMine += clientChat; //glue the messages together
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
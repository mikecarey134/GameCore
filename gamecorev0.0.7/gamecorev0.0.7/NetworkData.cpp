#include "NetworkData.h"


#include "remotePlayer.h"
#include <string.h>
#include <iostream>

NetworkData::NetworkData() : currentPlayers_(0), maxPlayers_(8)
{

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
	ourNetworkData << "0 " << thereInput.getPosition().X << "," << thereInput.getPosition().Y << "," << thereInput.getPosition().Z;
}


//This function takes in data from the server and store it into a player data to be used 
//by the client 
void NetworkData::setRemote(const char* ourData)
{
	std::string ID;
	std::string mType;
	std::stringstream ourID;
	ourID.str(ourData);
	ourID >> ID >> mType;

	//is it playerData
	if(mType == "0")
	{
		bool notFound = true;
		std::stringstream otherPlayer;
		otherPlayer << ourData;

		//Update current Players
		for(int x =0; x < currentPlayers_ && notFound; x++)
		{
			if(ourPlayerData[x].getGuid() == ID)
			{
				otherPlayer >> ourPlayerData[x];
				notFound = false;
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
					ourPlayerData[x].setGuid(ID);
					otherPlayer >> ourPlayerData[x];

					if(ourPlayerData[x].getState() != 10)
					{
						++currentPlayers_;
					}

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

}
//Get remote takes in a ref to a player in the game and sets all of its values based
//on information it receives from the server (stored in player data)
void NetworkData::getRemote(remotePlayer& worldRemote, int dataIndex)
{/*
	worldRemote.setPosition(ourPlayerData[dataIndex].getworldCoord());
	worldRemote.setCamPos(ourPlayerData[dataIndex].getcamPosition());
	worldRemote.setDirection(ourPlayerData[dataIndex].getDirection());
	worldRemote.setState(ourPlayerData[dataIndex].getState());
	worldRemote.setPlayerWeapon(ourPlayerData[dataIndex].getWeapon());
*/
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

//////////////////////////////////////////////////////////////////////////
/*
 BSD License 
 For RakNet software

 Copyright (c) 2014, Oculus VR, Inc.
 All rights reserved.*/
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/*
	InternalServer.cpp 
	By : Michael Carey
	Purpose : provide communication to game Server from the client side
	in the forms of chat and player data
*/
//////////////////////////////////////////////////////////////////////////




#include "InternalServer.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "PacketLogger.h"
#include <string>


using namespace RakNet;
using namespace std;

InternalServer::InternalServer(bool online,irr::IrrlichtDevice* device,char* filename,
							   irr::scene::ISceneManager* smgr,irr::video::IVideoDriver* driver,
							   irrklang::ISoundEngine* engine, irrBulletWorld* world, char* ip,
							   int remote_port, int local_port,ChatQueue* message_buffer, 
							   player* thePlayer): online_(online),
							   device_(device),smgr_(smgr),driver_(driver), engine_(engine), world_(world),
							   networkData_(device,filename,smgr,driver,engine,world, message_buffer, thePlayer),
							   message_buffer_(message_buffer)
{
	if(online_)
	{
		//convert value passed in to block style c_string 
		//because racknet connect only works with that form of c_string
		char ip_[MAX_IP_SIZE];
		
		int index =0;
		while(ip[index])
		{
			ip_[index] = ip[index];
			++index;
		}
		ip_[index-1]='\0';//insert the null terminator 
		

		//////////////////////////////////////////////////////////////////////////
		//current server info the user will specify this later
		//////////////////////////////////////////////////////////////////////////

		client_=RakNet::RakPeerInterface::GetInstance();
		// Connecting the client is very simple.  0 means we don't care about
		// a connectionValidationInteger, and false for low priority threads
		SocketDescriptor socketDescriptor(local_port,0);
		socketDescriptor.socketFamily=AF_INET;

		// Record the first client that connects to us so we can pass it to the ping function
		//RakNet::SystemAddress clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		isServer=false;

		if(online_)
		{
			// Record the first client that connects to us so we can pass it to the ping function
			//SystemAddress clientID = UNASSIGNED_SYSTEM_ADDRESS;


			// A client
			isServer=false;

			client_->AllowConnectionResponseIPMigration(false);

			client_->Startup(8,&socketDescriptor,1);
			client_->SetOccasionalPing(true);

			RakNet::ConnectionAttemptResult car = client_->Connect(ip_, remote_port, "0", (int) strlen("0"));
			bool b = (car==RakNet::CONNECTION_ATTEMPT_STARTED);

			if (b)
				puts("Connecting");
			else
			{
				puts("Connection Failed Bad Connection Attempt!");
				exit(1);
			}
		}
	}
}

InternalServer::~InternalServer(void)
{
}
void InternalServer::messageLoop(player& thePlayer, GUI& GUI,irr::gui::IGUIFont* font)
{
	
	if (online_ == true)//if the online parameter is set maybe we will have a user set this
	{

		RakNet::SystemAddress clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;

		// message is the data to send
		// strlen(message)+1 is to send the null terminator
		// HIGH_PRIORITY doesn't actually matter here because we don't use any other priority
		// RELIABLE_ORDERED means make sure the message arrives in the right order


		//send chat messages if we have them
		networkData_.setString(thePlayer);
		//since i made get read only once 

		std::string myMessage= "1 ";//append the one as a message identifier
		myMessage+= thePlayer.getname();//add the name of the player to the message
		myMessage+=" ";//separator
		myMessage+= thePlayer.getChatMessage();
		//+= "\0";
		
		//display the current player's message in the buffer
		std::string buffered_message = "<";
		buffered_message+= thePlayer.getname();
		buffered_message+= "> ";
		buffered_message+= thePlayer.getChatMessage();

		//if player in our player's list is damaged send it to the server
		std::vector<remotePlayer*> damagedPlayers = networkData_.findDamagedPlayers();
		if (!damagedPlayers.empty())
		{
			for (int i = 0; i < damagedPlayers.size(); ++i)
			{
				std::string damageMsg = "3 ";
				damageMsg += damagedPlayers[i]->getName();
				client_->Send(damageMsg.c_str(), damageMsg.length() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			}
		}
		
		if(thePlayer.getCurrentState() == DEAD)
		{
			std::string damageMsg = "4 ";
			damageMsg += thePlayer.getname();
			client_->Send(damageMsg.c_str(), damageMsg.length() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			thePlayer.setState(SPECTATOR);		
		}

		//Only send if we have a new message

		if(!emptyMess(thePlayer.getChatMessage())) //if we have more than just the messageID aka its empty send
		{
			client_->Send(myMessage.c_str(), myMessage.length() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			thePlayer.setChatMessage("\0");
			message_buffer_->addMessage(buffered_message.c_str());
		}

		

		//Sends our network info about our player.
		client_->Send(networkData_.getNetworkData(), networkData_.length() + 1 , HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	}


	if(online_)
	{
		    unsigned char packetIdentifier;
			
		    // Get a packet from either the server or the client
			for (p=client_->Receive(); p; client_->DeallocatePacket(p), p = client_->Receive())
			{
				// We got a packet, get the identifier with our handy function
				packetIdentifier = GetPacketIdentifier(p);

				// Check if this is a network message packet
				switch (packetIdentifier)
				{
				case ID_DISCONNECTION_NOTIFICATION:
					// Connection lost normally
					printf("Player Disconnected\n");
					break;

				case ID_ALREADY_CONNECTED:
					// Connection lost normally
					printf("Already Connected with guid %" PRINTF_64_BIT_MODIFIER "u\n", p->guid);
					break;

				case ID_INCOMPATIBLE_PROTOCOL_VERSION:
					printf("Incompatible Protocol\n");
					online_ = false;
					break;

				case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
					printf("Player Disconnected\n"); 
					break;

				case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
					printf("Remote Connection Lost\n");
					break;

				case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
					printf("Incoming Connection\n");
					break;

				case ID_CONNECTION_BANNED: // Banned from this server
					printf("You are banned from this server.\n");
					break;

				case ID_CONNECTION_ATTEMPT_FAILED:
					printf("Connection attempt failed\n");
					
					online_= false;
					
					break;

				case ID_NO_FREE_INCOMING_CONNECTIONS:
					// Sorry, the server is full.  I don't do anything here but
					// A real app should tell the user
					printf("Server is Full\n");
					online_ = false;
					break;

				case ID_INVALID_PASSWORD:
					printf("Invalid Password\n");
					break;

				case ID_CONNECTION_LOST:
					// Couldn't deliver a reliable packet - i.e. the other system was abnormally
					// terminated
					printf("Connection Lost!\n");
					break;

				case ID_CONNECTION_REQUEST_ACCEPTED:
					// This tells the client they have connected
					printf("Connected To Server\n");
					//printf("external address is %s\n", client_->GetExternalID(p->systemAddress).ToString(true));
					break;
				case ID_CONNECTED_PING:
				case ID_UNCONNECTED_PING:
					printf("Ping from %s\n", p->systemAddress.ToString(true));
					break;
				
				default:// It's a client

					if(p->data[0]!= '0' && p->data[0]!= '9' && p->data[0]!= '1' && p->data[0]!= '3'){//if the packet id is 0 or 99 do not print it
						printf("%s", p->data);
						puts("\n");
						message_buffer_->addMessage(p->data);//print out server messages on the message buffer
					}
					networkData_.setRemote((char*)p->data);
					break;
				}
			}

		}


	//}
}

bool InternalServer::emptyMess(char* message)
{	
	int length=0;
	while(*message!= NULL)
	{
		++length;
		++message;
	}

	return length == 0;
}
void InternalServer::shutdown(player& thePlayer)
{
	// Be nice and let the server know we quit
	std::string quit_message ="99 ";
	quit_message += thePlayer.getname();
	client_->Send(quit_message.c_str(), quit_message.length() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	client_->Shutdown(300);
	RakNet::RakPeerInterface::DestroyInstance(client_);
	puts("Shutting Down Internal Server");
}

unsigned char GetPacketIdentifier(RakNet::Packet *p)
{
	if (p==0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char) p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
		return (unsigned char) p->data[0];
}

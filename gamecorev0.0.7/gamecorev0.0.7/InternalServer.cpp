#include "InternalServer.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "PacketLogger.h"


using namespace RakNet;

InternalServer::InternalServer(bool online):online_(online)
{
	if(online_)
	{

		char ip[] = "mikesmcs.ddns.net";
		char serverPort[] = "1080";
		char clientPort[]= "1000";

		client_=RakNet::RakPeerInterface::GetInstance();
		// Connecting the client is very simple.  0 means we don't care about
		// a connectionValidationInteger, and false for low priority threads
		SocketDescriptor socketDescriptor(atoi(clientPort),0);
		socketDescriptor.socketFamily=AF_INET;

		// Record the first client that connects to us so we can pass it to the ping function
		RakNet::SystemAddress clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		isServer=false;

		if(online_)
		{
			// Record the first client that connects to us so we can pass it to the ping function
			SystemAddress clientID = UNASSIGNED_SYSTEM_ADDRESS;


			// A client
			isServer=false;

			client_->AllowConnectionResponseIPMigration(false);

			client_->Startup(8,&socketDescriptor,1);
			client_->SetOccasionalPing(true);
			//bool b = client->Connect(ip, SERVER_PORT, "",;	
			//bool b = client->Connect(ip, atoi(serverPort), "0", (int) strlen("0"));
			RakNet::ConnectionAttemptResult car = client_->Connect(ip, atoi(serverPort), "0", (int) strlen("0"));
			bool b = (car==RakNet::CONNECTION_ATTEMPT_STARTED);

			if (b)
				puts("Attempting connection");
			else
			{
				puts("Bad connection attempt.  Terminating.");
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
	
	if (online_ == true)
	{
		// Notice what is not here: something to keep our network running.  It's
		// fine to block on gets or anything we want
		// Because the network engine was painstakingly written using threads.

		// message is the data to send
		// strlen(message)+1 is to send the null terminator
		// HIGH_PRIORITY doesn't actually matter here because we don't use any other priority
		// RELIABLE_ORDERED means make sure the message arrives in the right order
		//client->Send(message, (int) strlen(message.length())+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);


		//Sends our network info about our player.
		//client->Send(ourPlayer.getNetworkData(), ourPlayer.length() + 1 , HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		//since i made get read only once 
		char* myMessage = thePlayer.getChatMessage(); 
		//+= "\0";

		//Only send if we have a new message
		//if(myMessage != "")

		if(!emptyMess(myMessage))
		{
			client_->Send(thePlayer.getChatMessage(), strlen(thePlayer.getChatMessage()) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			thePlayer.setChatMessage("\0");
		}

	}


	//for (p=client_->Receive(); p; client_->DeallocatePacket(p), p=client_->Receive())
	//{	
		
	if(online_)
	{
			// Get a packet from either the server or the client
			for (p=client_->Receive(); p; client_->DeallocatePacket(p), p=client_->Receive())
			{
				// We got a packet, get the identifier with our handy function
				packetIdentifier = GetPacketIdentifier(p);

				// Check if this is a network message packet
				switch (packetIdentifier)
				{
				case ID_DISCONNECTION_NOTIFICATION:
					// Connection lost normally
					printf("ID_DISCONNECTION_NOTIFICATION\n");
					break;
				case ID_ALREADY_CONNECTED:
					// Connection lost normally
					printf("ID_ALREADY_CONNECTED\n");
					break;
				case ID_INCOMPATIBLE_PROTOCOL_VERSION:
					printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
					break;
				case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
					printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n"); 
					break;
				case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
					printf("ID_REMOTE_CONNECTION_LOST\n");
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
					printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
					break;
				case ID_CONNECTION_BANNED: // Banned from this server
					printf("We are banned from this server.\n");
					break;			
				case ID_CONNECTION_ATTEMPT_FAILED:
					printf("Connection attempt failed\n");
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					// Sorry, the server is full.  I don't do anything here but
					// A real app should tell the user
					printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
					break;

				case ID_INVALID_PASSWORD:
					printf("ID_INVALID_PASSWORD\n");
					break;

				case ID_CONNECTION_LOST:
					// Couldn't deliver a reliable packet - i.e. the other system was abnormally
					// terminated
					printf("ID_CONNECTION_LOST\n");
					break;

				case ID_CONNECTION_REQUEST_ACCEPTED:
					// This tells the client they have connected
					printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
					printf("My external address is %s\n", client_->GetExternalID(p->systemAddress).ToString(true));
					break;
				default:
					// It's a client, so just show the message
					printf("%s", p->data);
					puts("\n");

					
					GUI.drawMessage(font,irr::core::rect<irr::s32>(325,275,475,325),(char*)p->data);
					
					//Takes data from server and decides how to handle it
					//ourPlayer.setRemote((char*)p->data);
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
void InternalServer::shutdown()
{
	// Be nice and let the server know we quit.
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

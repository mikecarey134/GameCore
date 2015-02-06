
//////////////////////////////////////////////////////////////////////////
/*
 BSD License 
 For RakNet software

 Copyright (c) 2014, Oculus VR, Inc.
 All rights reserved.*/
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/*
	Server.cpp 
	Adapted By : Michael Carey
	Purpose : provide communication to game clients 
	in the forms of chat and player data
*/
//////////////////////////////////////////////////////////////////////////


#include "MessageIdentifiers.h"

#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "RakSleep.h"
#include "PacketLogger.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include "Kbhit.h"
#include <stdio.h>
#include <string.h>
#include "Gets.h"

#define _USE_RAK_MEMORY_OVERRIDE

#define MAX_CONNECTIONS 2//maximum amount of connections
						 //limited to 2 for debugging
#define DEFAULT_PORT_NUMBER 1080
//default port number for connections 

unsigned char GetPacketIdentifier(RakNet::Packet *p);


int main(void)
{
	bool isOver = false;//are we done

	// Pointers to the interfaces of our server and client.
	// Note we can easily have both in the same program
	RakNet::RakPeerInterface *server=RakNet::RakPeerInterface::GetInstance();
	RakNet::RakNetStatistics *rss;
	server->SetIncomingPassword("0", (int)strlen("0"));
	server->SetTimeoutTime(30000,RakNet::UNASSIGNED_SYSTEM_ADDRESS);


	// Holds packets
	RakNet::Packet* p;

	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;

	// Record the first client that connects to us so we can pass it to the ping function
	RakNet::SystemAddress clientID=RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	// Holds user data
	char portstring[30];
	printf("---------------------------------------------------------\n");
	printf("Game Server v0.1.0 Alpha\n");
	printf("---------------------------------------------------------\n");
	// A server
	puts("Enter the server port to listen on\nOr press enter for default(1080)\n");
	printf("---------------------------------------------------------\n");

	printf("->");
	Gets(portstring,sizeof(portstring));
	if (portstring[0]==0)
		strcpy(portstring, "1080");//default port number to start on
	
	puts("Starting server.");
	
	// Starting the server is very simple.  2 players allowed.
	// 0 means we don't care about a connectionValidationInteger, and false
	// for low priority threads
	// I am creating two socketDesciptors, to create two sockets. One using IPV6 and the other IPV4
	RakNet::SocketDescriptor socketDescriptors[2];
	socketDescriptors[0].port=atoi(portstring);
	socketDescriptors[0].socketFamily=AF_INET; // Test out IPV4
	socketDescriptors[1].port=atoi(portstring);
	socketDescriptors[1].socketFamily=AF_INET6; // Test out IPV6
	bool b = server->Startup(4, socketDescriptors, 2 )==RakNet::RAKNET_STARTED;

	server->SetMaximumIncomingConnections(MAX_CONNECTIONS);//max incoming is 2
	if (!b)
	{
		//printf("Failed to start dual IPV4 and IPV6 ports. Trying IPV4 only.\n");

		// Try again, but leave out IPV6
		b = server->Startup(4, socketDescriptors, 1 )==RakNet::RAKNET_STARTED;
		if (!b)
		{
			puts("Server failed to start.  Terminating.");
			exit(1);
		}
	}
	printf("---------------------------------------------------------\n");
	printf("Done!\n");
	printf("---------------------------------------------------------\n");
	server->SetOccasionalPing(true);
	server->SetUnreliableTimeout(1000);

	DataStructures::List< RakNet::RakNetSocket2* > sockets;
	server->GetSockets(sockets);

	printf("\nMy IP addresses:\n");
	

	for (unsigned int i=0; i < server->GetNumberOfAddresses(); i++)
	{
		RakNet::SystemAddress sa = server->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS, i);
		printf("%i. %s (LAN=%i)\n", i+1, sa.ToString(false), sa.IsLANAddress());
	}
	puts("---------------------------------------------------------");

	char message[2048];

	// Loop for input
	while (1)
	{

	// This sleep keeps RakNet responsive
	RakSleep(30);

	if (kbhit())
	{
		// Notice what is not here: something to keep our network running.  It's
		// fine to block on gets or anything we want
		// Because the network engine was painstakingly written using threads.
		
		printf("->");
		Gets(message,sizeof(message));
		
		if (strcmp(message, "help")==0)
		{
			puts("'stat' to show stats. 'ping' to ping.\n'pingip' to ping an ip address\n'ban' to ban an IP from connecting.\n'kick to kick the first connected player.\nType to talk.");
			puts("---------------------------------------------------------");
			continue;
		}
		
		/*
		if (strcmp(message, "quit")==0)
		{
			puts("Quitting.");
			isOver=true;
			break;
		}
		*/

		if (strcmp(message, "stat")==0)
		{
			
			rss=server->GetStatistics(server->GetSystemAddressFromIndex(0));
			StatisticsToString(rss, message, 2);
			printf("%s", message);
			printf("Ping %i\n", server->GetAveragePing(server->GetSystemAddressFromIndex(0)));
			puts("---------------------------------------------------------");
			continue;
		}

		if (strcmp(message, "ping")==0)
		{
			server->Ping(clientID);
			puts("---------------------------------------------------------");
			continue;
		}

		if (strcmp(message, "pingip")==0)
		{
			printf("Enter IP: ");
			Gets(message,sizeof(message));
			printf("Enter port: ");
			Gets(portstring,sizeof(portstring));
			if (portstring[0]==0)
				strcpy(portstring, "1234");
			server->Ping(message, atoi(portstring), false);
			puts("---------------------------------------------------------");
			continue;
		}

		if (strcmp(message, "kick")==0)
		{
			server->CloseConnection(clientID, true, 0);
			puts("---------------------------------------------------------");
			continue;
		}
		if (strcmp(message, "about")==0)
		{
			printf("Programmed by: Michael Carey (2015)\n");
			puts("---------------------------------------------------------");
			continue;
		}
		if (strcmp(message, "getconnectionlist")==0)
		{
			RakNet::SystemAddress systems[10];
			unsigned short numConnections=10;
			server->GetConnectionList((RakNet::SystemAddress*) &systems, &numConnections);
			for (int i=0; i < numConnections; i++)
			{
				printf("%i. %s\n", i+1, systems[i].ToString(true));
			}
			puts("---------------------------------------------------------");
			continue;
		}

		if (strcmp(message, "ban")==0)
		{
			printf("Enter IP to ban.  You can use * as a wildcard\n");
			puts("---------------------------------------------------------");
			Gets(message,sizeof(message));
			server->AddToBanList(message);
			printf("IP %s added to ban list.\n", message);
			puts("---------------------------------------------------------");
			continue;
		}

		if (strcmp(message, "clear")==0)
		{
			system("cls");
			printf("Console Cleared!\n");
			puts("---------------------------------------------------------");
			continue;
		}

		// Message now holds what we want to broadcast
		char message2[2048];
		// Append Server: to the message so clients know that it ORIGINATED from the server
		// All messages to all clients come from the server either directly or by being
		// relayed from other clients
		message2[0]=0;
		const static char prefix[] = "[Server]: ";
		strncpy(message2, prefix, sizeof(message2));
		strncat(message2, message, sizeof(message2) - strlen(prefix) - 1);
	
		// message2 is the data to send
		// strlen(message2)+1 is to send the null terminator
		// HIGH_PRIORITY doesn't actually matter here because we don't use any other priority
		// RELIABLE_ORDERED means make sure the message arrives in the right order
		// We arbitrarily pick 0 for the ordering stream
		// RakNet::UNASSIGNED_SYSTEM_ADDRESS means don't exclude anyone from the broadcast
		// true means broadcast the message to everyone connected
		server->Send(message2, (const int) strlen(message2)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

		// Get a packet from either the server or the client

		for (p=server->Receive(); p; server->DeallocatePacket(p), p=server->Receive())
		{
			// We got a packet, get the identifier with our handy function
			packetIdentifier = GetPacketIdentifier(p);

			// Check if this is a network message packet
			switch (packetIdentifier)
			{
			case ID_DISCONNECTION_NOTIFICATION:
				// Connection lost normally
				printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString(true));;
				break;


			case ID_NEW_INCOMING_CONNECTION:
				// Somebody connected.  We have their IP now
				printf("ID_NEW_INCOMING_CONNECTION from %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
				clientID=p->systemAddress; // Record the player ID of the client

				printf("Remote internal IDs:\n");
				for (int index=0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++)
				{
					RakNet::SystemAddress internalId = server->GetInternalID(p->systemAddress, index);
					if (internalId!=RakNet::UNASSIGNED_SYSTEM_ADDRESS)
					{
						printf("%i. %s\n", index+1, internalId.ToString(true));
						puts("-----------------------------------------------\n");
					}
				}

				break;

			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;

			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				printf("Ping from %s\n", p->systemAddress.ToString(true));
				puts("-------------------------------------\n");
				break;
		

			case ID_CONNECTION_LOST:
				// Couldn't deliver a reliable packet - i.e. the other system was abnormally
				// terminated
				printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString(true));;
				break;

			default:
				// The server knows the static data of all clients, so we can prefix the message
				// With the name data
				if(p->data[0]!= '0'){//if the packet id isnt one dont print it
					printf("%s\n", p->data);
					sprintf(message, "%s", p->data);
				}
				// Relay the message.  We prefix the name for other clients.  This demonstrates
				// That messages can be changed on the server before being broadcast
				// Sending is the same as before
				
				server->Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);

				break;
			}

		}
	}

	//if(isOver)
	//{
		server->Shutdown(300);
		//We're done with the network
		RakNet::RakPeerInterface::DestroyInstance(server);
	//}
	return 0;
}




// If the first byte is ID_TIMESTAMP, then we want the 5th byte
// Otherwise we want the 1st byte
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

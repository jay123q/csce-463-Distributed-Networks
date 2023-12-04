// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023

#include "pch.h"

#include <iostream>

// Get current flag
#define MAX_RETRIES 3
#define N 30
#define IP_HDR_SIZE 20 /* RFC 791 */
#define ICMP_HDR_SIZE 8 /* RFC 792 */
/* max payload size of an ICMP message originated in the program */
#define MAX_SIZE 65200
/* max size of an IP datagram */
#define MAX_ICMP_SIZE (MAX_SIZE + ICMP_HDR_SIZE)
/* the returned ICMP message will most likely include only 8 bytes
* of the original message plus the IP header (as per RFC 792); however,
* longer replies (e.g., 68 bytes) are possible */
#define MAX_REPLY_SIZE (IP_HDR_SIZE + ICMP_HDR_SIZE + MAX_ICMP_SIZE)
/* ICMP packet types */
#define ICMP_ECHO_REPLY 0
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRED 11
#define ICMP_ECHO_REQUEST 8 
using namespace std;


int runMainFunction(string host)
{

	int seqNumber = 0;
	string query = host;
	printf("Lookup  : %s\n", query.c_str());

	int ttlCounter = 1;

	DWORD IP = inet_addr(host.c_str());




	cout << " IP " << gethostbyname(host.c_str()) << std::endl;
	cout << " IP " << IP << std::endl;


	// handle errors

	packetHelper* pk  = new packetHelper(IP, host);


	// IPHeader* ip = (IPHeader*)send_buf;
	// ip->checksum = 0;
	// ip->proto = 6; // UDP
	// ip->dest_ip = IP;
	// ip->source_ip = gethostbyaddr((char*)&(ip->dest_ip), 4, AF_INET);;
	// memcpy(send_buf + sizeof(ICMPHeader), icmp, sizeof(IPHeader));
	HANDLE sendPackets[N+1];
	for (int i = 0; i < N ; i++)
	{
		pk->createPacket(i,i);
	}	
	
	for (int i = 0; i < N ; i++)
	{
		pk->sendPacket(i,i);
	}


	/* calculate the checksum */
	// int packet_size = sizeof(ICMPHeader); // 8 bytes
		// set proper TTL
		// need Ws2tcpip.h for IP_TTL, which is equal to 4; there is another constant with the same
		// name in multicast headers – do not use it!

	// handle errors 
	/*
	char buffer[sizeof(IPHeader) + sizeof(ICMPHeader)];
	memset(buffer, 0, sizeof(IPHeader)+ sizeof(ICMPHeader));
	memcpy(buffer, &icmp, sizeof(ICMPHeader));
	*/
	for (int i = 0; i < N; i++)
	{
		sendPackets[i] = pk->pd[i].complete;
	}

	sendPackets[N] = pk->socketReceiveReady;
	while (true)
	{
		int waitSocket = WaitForMultipleObjects(N + 1, sendPackets, false, INFINITE);
		if (waitSocket == 30)
		{
			break;
		}
	}

	int count = 0;
	while (true)
	{
		// send request to the server

			char buf[MAX_REPLY_SIZE];
			memset(buf, 0, MAX_REPLY_SIZE);
			struct sockaddr_in response;
			int responseSize = sizeof(response);
			int bytes = recvfrom(pk->sock, buf, MAX_REPLY_SIZE, 0, (struct sockaddr*)&response, &responseSize);

			IPHeader* routerIpHeader = (IPHeader*)buf;
			ICMPHeader* routerIcmpHead = (ICMPHeader*)(buf + 1);
			if (bytes >= 56)
			{
				// error processing
				// check if this packet came from the server to which we sent the query earlier

				printf("received a packet with size %d\n", htons(routerIpHeader->len));
				printf(" router type %d | router code %d \n", routerIcmpHead->type, routerIcmpHead->code);
				if (routerIcmpHead->type == ICMP_TTL_EXPIRED && routerIcmpHead->code == 0 )
				{
					IPHeader* packetIpHeader = (IPHeader*)(routerIcmpHead + 1);
					ICMPHeader* packetIcmpHeader = (ICMPHeader*)(packetIpHeader + 1);
					printf(" r source ip %d | r dest ip %d | original source ip %d | original dest %d |\n",
					routerIpHeader->source_ip, routerIpHeader->dest_ip, packetIpHeader->source_ip, packetIpHeader->dest_ip);

				}

			//	break;
			}
			else if (bytes == 28 && routerIcmpHead->type == ICMP_ECHO_REPLY )
			{
				printf(" router type %d | router code %d \n", routerIcmpHead->type, routerIcmpHead->code);
				printf(" icmp seq %d | icmp id %d \n", routerIcmpHead->seq, routerIcmpHead->id);

			}



	}

	WSACleanup();
	closesocket(pk->sock);

	return 0;
}


int main(int argc, char* argv[])
{

	/*
	if (argc != 3)
	{
		cout << " CHECK HOW YOU RUNNING OR WHAT WE ARE TESTING ./hw2.exe query ip \n";
		return 0;
	}
	*/

	 string query("www.yahoo.com" );
	runMainFunction(query);

	/*




	string query("www.google.com");
	string DNS ( "8.8.8.8" );

	string query("www.dhs.gov");
	string DNS("128.194.135.85");

	string query("randomA.irl" );
	string DNS ( "128.194.135.82" );

	 string DNS ( "128.194.135.85" );

	 string query("128.194.138.19" );
	 string DNS ( "128.194.135.85" );

	 runMainFunction(query, DNS);
	*/
	/*
	vector<string> happyQuery = { "www.google.com","www.dhs.gov","randomA.irl","yahoo.com","23.203.88.222" ,"128.194.135.77"};
	vector<string> happyDNS = { "8.8.8.8","128.194.135.85","128.194.135.82",  "128.194.135.85","128.194.138.85","128.194.135.85"};

   for (int i = 0; i < happyQuery.size(); i++)
   {
	   runMainFunction(happyQuery[i], happyDNS[i]);
	   printf("\n\n\n\n\n\n");

   }
	vector<string> unhappyQuery = { "www.google.c","12.190.0.107","random2.irl","random9.irl","randomB.irl","google.com" };
	vector<string> unhappyDNS = { "128.194.135.85","128.194.135.85","128.194.135.82","128.194.135.82","128.194.135.82","128.194.135.9" };
	for (int i = 0; i < unhappyQuery.size(); i++)
	{
		runMainFunction(unhappyQuery[i], unhappyDNS[i]);
		printf("\n\n\n\n\n\n");

	}

   */
   /*
  //randomX.irl 1-9 A-B
   vector<string> randomQuery = { "random0.irl", "random1.irl","random2.irl", "random3.irl","random4.irl", "random5.irl","random6.irl", "random7.irl","random8.irl","random9.irl","randomA.irl","randomB.irl" };
	// vector<string> randomQuery = {"random5.irl","random5.irl","random5.irl","random5.irl","random5.irl" };
   for (int i = 0; i < randomQuery.size(); i++)
   {
	   runMainFunction(randomQuery[i], "128.194.135.82");
	   printf("\n\n\n\n\n\n");

   }
   string query("random4.irl");
   runMainFunction(query , "128.194.135.82");
   printf("\n\n\n\n\n\n");
   runMainFunction(query , "128.194.135.82");
   printf("\n\n\n\n\n\n");
   runMainFunction(query , "128.194.135.82");
   printf("\n\n\n\n\n\n");
   runMainFunction(query , "128.194.135.85");
   printf("\n\n\n\n\n\n");
   */
	return 0;

}
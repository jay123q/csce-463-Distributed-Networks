// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023

#include "pch.h"

#include <iostream>

// Get current flag

using namespace std;
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
/* remember the current packing state */
#pragma pack (push)
#pragma pack (1)
/* define the IP header (20 bytes) */
class IPHeader {
public:
	u_char h_len : 4; /* lower 4 bits: length of the header in dwords */
	u_char version : 4; /* upper 4 bits: version of IP, i.e., 4 */
	u_char tos; /* type of service (TOS), ignore */
	u_short len; /* length of packet */
	u_short ident; /* unique identifier */
	u_short flags; /* flags together with fragment offset - 16 bits */
	u_char ttl; /* time to live */
	u_char proto; /* protocol number (6=TCP, 17=UDP, etc.) */
	u_short checksum; /* IP header checksum */
	u_long source_ip;
	u_long dest_ip;
};
/* define the ICMP header (8 bytes) */
class ICMPHeader {
public:
	u_char type; /* ICMP packet type */
	u_char code; /* type subcode */
	u_short checksum; /* checksum of the ICMP */
	u_short id; /* application-specific ID */
	u_short seq; /* application-specific sequence */
};
/* now restore the previous packing state */
#pragma pack (pop) 

struct sockaddr_in remote;




int runMainFunction(string host)
{
	int seqNumber = 0;
	string query = host;
	printf("Lookup  : %s\n", query.c_str());

	int ttlCounter = 1;

	DWORD IP = inet_addr(host.c_str());

	if (IP == INADDR_NONE)
	{
		struct hostent* r;
			r = gethostbyname(host.c_str());
		if ( r == NULL)
		{
			// printf("Connection error: %d\n", WSAGetLastError());
			return false;
		}
		else // take the first IP address and copy into sin_addr
		{

			memcpy((char*)&(remote.sin_addr), r->h_addr, r->h_length);
		}
		// debugging! 
		// 
		//	printf ("Connection error: %d\n", WSAGetLastError ());

	}
	else
	{
		remote.sin_addr.S_un.S_addr = IP;
	}

	//handle socket creation and connection 

	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error sart up %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	// open a TCP socket
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock == INVALID_SOCKET)
	{
		printf(" unable to create a raw socket: error %d \n", WSAGetLastError());
	}
	// handle errors

	 char send_buf[MAX_ICMP_SIZE]; /* IP header is not present here */
	ICMPHeader* icmp = (ICMPHeader*)send_buf;
	icmp->type = ICMP_ECHO_REQUEST;
	icmp->code = 0;
	icmp->id = GetCurrentProcessId();
	icmp->seq = htons(seqNumber++);;
	icmp->checksum = 0;
	int packet_size = sizeof(IPHeader)+sizeof(ICMPHeader);
	checksum cc;
	icmp->checksum = cc.ip_checksum((u_short*)send_buf, packet_size);
	memcpy(send_buf, icmp, sizeof(ICMPHeader));


	IPHeader* ip = (IPHeader*)send_buf;
	ip->checksum = 0;
	ip->proto = 6; // UDP
	ip->dest_ip = IP;
	// ip->source_ip = gethostbyaddr((char*)&(ip->dest_ip), 4, AF_INET);;
	// memcpy(send_buf + sizeof(ICMPHeader), icmp, sizeof(IPHeader));




	/* calculate the checksum */
	// int packet_size = sizeof(ICMPHeader); // 8 bytes
		// set proper TTL
	int ttl = ttlCounter;
		// need Ws2tcpip.h for IP_TTL, which is equal to 4; there is another constant with the same
		// name in multicast headers – do not use it!
		if (setsockopt(sock, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
		{
			printf("setsockopt failed with %d\n", WSAGetLastError());
				closesocket(sock);
			// some cleanup
			exit(-1);
		}

	struct sockaddr_in remote;
	memset(&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_addr.S_un.S_addr = inet_addr(query.c_str()); // server’s IP
	remote.sin_port = htons(53); // DNS port on server

//	printf("Server  : %s\n", DNS.c_str());
	printf("********************************\n");
	char buffer[sizeof(IPHeader) + sizeof(ICMPHeader)];
	memset(buffer, 0, sizeof(IPHeader) + sizeof(ICMPHeader));



	if (sendto(sock, buffer, sizeof(ICMPHeader), 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR)
	{
		printf(" send to error %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 0;

	}
	// handle errors 


	int count = 0;
	bool TerminateRun = false;
	while (true)
	{
		// send request to the server
		clock_t start = clock();
		timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		fd_set fd;
		FD_ZERO(&fd); // clear the set
		FD_SET(sock, &fd); // add your socket to the set
		int available = select(0, &fd, NULL, NULL, &timeout);
		if (available > 0)
		{
			char buf[MAX_REPLY_SIZE];
			struct sockaddr_in response;
			int responseSize = sizeof(response);
			int bytes = recvfrom(sock, buf, MAX_REPLY_SIZE, 0, (struct sockaddr*)&response, &responseSize);
			if (bytes != SOCKET_ERROR)
			{
				// error processing
				// check if this packet came from the server to which we sent the query earlier


				

				cout << "  recieved properly \n";
			}
			else
			{
				printf(" recfrom error %d\n", WSAGetLastError());
				break;
			}

		}
		else if (available == 0)
		{
			printf("timeout in %d ms\n", (int)(1000 * ((double)(clock() - start)) / CLOCKS_PER_SEC));
		}
		else
		{
			printf("select error %d\n", WSAGetLastError());
			// closesocket(sock);
			// WSACleanup();
			//return 0;
		}


	}

	WSACleanup();
	closesocket(sock);

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

	 string query("yahoo.com" );
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
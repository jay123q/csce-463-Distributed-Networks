// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023

#include "pch.h"

#include <iostream>

// Get current flag
#define MAX_RETRIES 3
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
queue<string> parseTXTFile(std::string filename)
{
	// cout << " in parser \n";
	ifstream file(filename, ios::binary | ios::in);
	std::string line;
	std::queue <std::string> queueTotal;


	while (!file.eof())
	{
		getline(file, line);
		if (line[0] == '\0')
		{
			break;
		}
		line = line.substr(0, line.size() - 1);
		//this->intFileSize += strlen(line.c_str());
	 //   cout << " the line is " << line << std::endl;
	//    cout << " push the file " << line << std::endl;
		queueTotal.push(line);
	}
	return queueTotal;
}

int runMainFunction(string host)
{
	string query = host;
#ifdef reportWork
	queue<string> q = parseTXTFile("URL-input-1M.txt");
	query = q.front();
	q.pop();

#endif // reportWork



	// printf("Lookup  : %s\n", query.c_str());




	// handle errors

	packetHelper* pk  = new packetHelper(host);


	// remove handles later
	// single socket, send all on that one
	// send all icmp packets 30
	// handle all 30
	// manage and recieve all 30 timeouts
	// no mulithreads
	// print from heap for all
	// 


	for (int i = sendNumber; i < N+1 ; i++)
	{
		pk->createPacket(i);
	}	
	
	for (int i = sendNumber; i < N+1 ; i++)
	{
		pk->sendPacket(i);
	}



	while (true)
	{
		pk->recvPackets();
		if (pk->errorBreak == true)
		{
			break;
		}
		if(pk->checkComplete() == true)
		{
			break;
		}
		pk->retransmitPackets();
	}

	pk->finalPrint();
	printf(" Traceroute complete \n");

	WSACleanup();
	closesocket(pk->sock);
	delete pk;
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
	string query("yahoo.com");
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
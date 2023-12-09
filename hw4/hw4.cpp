// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023

#include "pch.h"
#include <utility>
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


 pair < int, std::set<u_long>> count_uniqueIps;

vector<pair<int, string>> hop_host;
vector<double> executionVector;


string parseString(string link) {

	// cout << "URL: " << link << std::endl;
	// cout << "\t   Parsing URL... ";

	if (link.substr(0, 7) != "http://") {
		//   cout << "failed with invalid scheme " << std::endl;
		return "";
	}

	string hostStart = link.substr(7);

	size_t fragmentIndex = hostStart.find_first_of('#');
	if (fragmentIndex != string::npos) {
		// this->fragment = hostStart.substr(fragmentIndex + 1);
		hostStart = hostStart.substr(0, fragmentIndex);
	}

	size_t queryIndex = hostStart.find_first_of('?');
	if (queryIndex != string::npos) {
		// this->query = hostStart.substr(queryIndex + 1);
		hostStart = hostStart.substr(0, queryIndex);
	}

	size_t pathIndex = hostStart.find_first_of('/');
	if (pathIndex != string::npos) {
		// this->path = hostStart.substr(pathIndex);
		hostStart = hostStart.substr(0, pathIndex);
	}
	else {
		// this->path = "/";
	}

	// cout <<" host " << this->host << " the port is " << this->port << " path is " << this->path << " query is " << this->query << " fragment is  " << this->fragment << std::endl;
	 //this->host = hostStart;
#ifndef reportWork12
	std::cout << hostStart << endl;
#endif // reportWork12
	return hostStart;

}

queue<string> parseTXTFile(std::string filename)
{

	ifstream file(filename, ios::binary | ios::in);
	std::string line; 
	std::queue <std::string> queueTotal;
	// std::cout << file.is_open() << endl;

	while (!file.eof())
	{
		getline(file, line);
		if (line[0] == '\0')
		{
		//	cout << " in parser \n";
			break;
		}
		line = line.substr(0, line.size() - 1);
		//this->intFileSize += strlen(line.c_str());
	   // cout << " the line is " << line << std::endl;
	//    cout << " push the file " << line << std::endl;
		queueTotal.push(line);
	}
	return queueTotal;
}

int runMainFunction(string host)
{
	string query = host;

	// handle errors

	packetHelper* pk  = new packetHelper(host);
	if (pk->errorBreak == true)
	{
		delete pk;
		return 0;
	}


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


	clock_t start = clock();
	while (true)
	{
		pk->recvPackets();
		if (pk->errorBreak == true)
		{
			break;
		}
		if(pk->checkComplete() == true)
		{
		//	cout << " count to break " << pk->countSeq << endl;
			std::string seqPrint = std::to_string(pk->countSeq) + " ";
			seqPrint += pk->printLast;
			pk->pd[pk->countSeq].printString = seqPrint;
			// debug this
			break;
		}
		pk->retransmitPackets();
	}

	double exectutionTime = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000;
#ifndef reportWork12
	pk->finalPrint();
	printf("\n Total execution time: %f ms  \n",
		exectutionTime
		);
#endif // !reportWork12

	WSACleanup();
	closesocket(pk->sock);

	if (pk->errorBreak == true)
	{
		return 0;
	}

#ifdef reportWork12
	hop_host.push_back( { pk->countSeq, host } );
	// std::cout << " packet hops is " << pk->countSeq << " packet host is " << host << endl;
#endif

#ifdef reportWork34
	// a.singleIpCount = pk->countIp;
	/*
	executionVector.push_back(exectutionTime);
	count_uniqueIps.first += pk->unique_ip.size(),
		std::cout << " before insert size " << pk->unique_ip.size() << std::endl;
	count_uniqueIps.second.insert(pk->unique_ip.begin(), pk->unique_ip.end());
	std::cout << " execution delay " << exectutionTime << endl;
	std::cout << " total ips is " << count_uniqueIps.first << "  unique ips " << count_uniqueIps.second.size() << endl;
	*/

#endif // DEBUG

	delete pk;
	return true;
}

void writeTxtFile()
{
	ofstream writeFile;
	writeFile.open("hopCount.txt");
	for (size_t i = 0; i < hop_host.size(); i++)
	{
		writeFile << hop_host.at(i).first << " | " << hop_host.at(i).second << '\n';
	}

	writeFile.close();
}

struct MultiThread  {
	queue<string> q;

};

void reportWork()
{
	queue<string> q = parseTXTFile("URL-input-1M-2019.txt");
	double exectutionTime = 0.0;
	ofstream writeFile("hopCount.txt");
	for (size_t i = 0; i < 10000; i++)
	{
		// writeFile.open("hopCount.txt");
		if (q.size() == 0)
		{
			break;
		}
		std::string query = q.front();
		if (runMainFunction(parseString(query)) == false)
		{
			// remove the place, just grab anouther q
			i--;
		}

		q.pop();

		writeFile << hop_host.at(i).first << " | " << hop_host.at(i).second << '\n';


	}
		writeFile.close();
	// writeTxtFile();
	int binSizes[20];
	for (int i = 0; i < 20; i++)
	{
		binSizes[i] = 0;
	}
	for (size_t i = 0; i < executionVector.size(); i++)
	{
		exectutionTime = executionVector.at(i);

		if (exectutionTime <= 50)
		{
			binSizes[0]++;
		}
		else if (exectutionTime <= 100)
		{
			binSizes[1]++;
		}
		else if (exectutionTime <= 150)
		{
			binSizes[2]++;
		}
		else if (exectutionTime <= 200)
		{
			binSizes[3]++;
		}
		else if (exectutionTime <= 300)
		{
			binSizes[4]++;
		}
		else if (exectutionTime <= 400)
		{
			binSizes[5]++;
		}
		else if (exectutionTime <= 450)
		{
			binSizes[6]++;
		}
		else if (exectutionTime <= 500)
		{
			binSizes[7]++;
		}
		else if (exectutionTime <= 550)
		{
			binSizes[8]++;
		}
		else if (exectutionTime <= 600)
		{
			binSizes[9]++;
		}
		else if (exectutionTime <= 650)
		{
			binSizes[10]++;
		}
		else if (exectutionTime <= 700)
		{
			binSizes[11]++;
		}
		else if (exectutionTime <= 750)
		{
			binSizes[12]++;
		}
		else if (exectutionTime <= 800)
		{
			binSizes[13]++;
		}
		else if (exectutionTime <= 850)
		{
			binSizes[14]++;
		}
		else if (exectutionTime <= 900)
		{
			binSizes[15]++;
		}
		else if (exectutionTime <= 950)
		{
			binSizes[16]++;
		}
		else if (exectutionTime <= 1000)
		{
			binSizes[17]++;
		}
		else if (exectutionTime <= 1050)
		{
			binSizes[18]++;
		}
		else
		{
			binSizes[19]++;
		}
	}
	ofstream write34("histandIp.txt");

	write34 << " total number of ips " << count_uniqueIps.first << " unqiue Ips " << count_uniqueIps.second.size() << endl;
	write34 << " now bins " << endl;
	for (size_t i = 0; i < 20; i++)
	{
		write34 << binSizes[i] << ", ";
	}

	write34 << endl;
	write34.close();


}
int main(int argc, char* argv[])
{

	/*
	if (argc != 2)
	{
		cout << " CHECK HOW YOU RUNNING OR WHAT WE ARE TESTING ./hw2.exe query ip \n";
		return 0;
	}

	runMainFunction(argv[1]);
	*/
#ifdef reportWork12
	reportWork();

	
#endif // reportWork12
#ifndef reportWork12
	/*

	string query("google.com");
	runMainFunction(query);
	*/
#endif // !reportWork

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
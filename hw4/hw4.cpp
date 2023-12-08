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


std::set<u_long> unique_ip;


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
struct params {
		vector<double> executiontime;
		int numberofIps;
		bool badData;
		vector<pair<int, std::string>>  hopCount;

};
struct makeReportParams
{
	double singleExec;
	int singleIpCount;
	bool badData;
	pair<int, std::string> hopPair;
};
makeReportParams runMainFunction(string host)
{
	string query = host;


		pair<int, std::string> hopPair;
		makeReportParams a{ -1,-1,true, hopPair };


	// printf("Lookup  : %s\n", query.c_str());




	// handle errors

	packetHelper* pk  = new packetHelper(host);
	if (pk->errorBreak == true)
	{
		return a;
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
			pk->pd[pk->countSeq].printString = pk->printLast;
			break;
		}
		pk->retransmitPackets();
	}

	pk->finalPrint();
	double exectutionTime = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000;
	printf("\n Total execution time: %f ms  \n",
		exectutionTime
		);

	WSACleanup();
	closesocket(pk->sock);


	a.badData = false;
	a.singleExec = exectutionTime;
	a.hopPair = { pk->countSeq, host };
	a.singleIpCount = pk->countIp;
	unique_ip.insert(pk->unique_ip.begin(), pk->unique_ip.end());

	delete pk;
	return a;
}

void writeTxtFile(vector<pair<int, std::string>>& hopCount)
{
	ofstream writeFile("hopCount.txt");

	for (size_t i = 0; i < hopCount.size(); i++)
	{
		writeFile << hopCount.at(i).first << " | " << hopCount.at(i).second << '\n';
	}

	writeFile.close();
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
#ifdef reportWork
	queue<string> q = parseTXTFile("URL-input-1M.txt");
	vector<pair<int, std::string>>  hopCount;
	params p;
	double exectutionTime = 0.0;
	for (size_t i = 0; i < q.size(); i++)
	{
		std::string query = q.front();
		makeReportParams rp = runMainFunction(query) ;
		q.pop();
		if (rp.badData == true)
		{
			continue;
		}
		p.hopCount.push_back(rp.hopPair);
		p.numberofIps += rp.singleIpCount;

	}
	writeTxtFile(p.hopCount);
	int binSizes[20];
	for (size_t i = 0; i < p.executiontime.size() ; i++)
	{
		 exectutionTime = p.executiontime.at(i);

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
		else if (exectutionTime <= 1100)
		{
			binSizes[19]++;
		}
	}
	ofstream writeFile("histandIp.txt");

	writeFile << " total number of ips " << p.numberofIps << " unqiue Ips " << unique_ip.size() << endl;
	writeFile << " now bins " << endl;
	for (size_t i = 0; i < 20; i++)
	{
		writeFile << binSizes[i] << ", ";
	}

	writeFile << endl;
	writeFile.close();


#endif // reportWork
#ifndef reportWork


	string query("yahoo.com");
	runMainFunction(query);
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
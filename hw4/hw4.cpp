// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
#include <ctype.h> 
#include <stdio.h> 
#include "pch.h"
#include <string>
#include <windows.h>
#include <iostream>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

// Get current flag

using namespace std;
#define DNS_A 1 /* name -> IP */
#define DNS_NS 2 /* name server */
#define DNS_CNAME 5 /* canonical name */
#define DNS_PTR 12 /* IP -> name */
#define DNS_HINFO 13 /* host info/SOA */
#define DNS_MX 15 /* mail exchange */
#define DNS_AXFR 252 /* request for zone transfer */
#define DNS_ANY 255 /* all records */ 
#define DNS_INET 1  //  pass to the qClass

/* flags */
#define DNS_QUERY (0 << 15) /* 0 = query; 1 = response */
#define DNS_RESPONSE (1 << 15)
#define DNS_STDQUERY (0 << 11) /* opcode - 4 bits */
#define DNS_AA (1 << 10) /* authoritative answer */
#define DNS_TC (1 << 9) /* truncated */
#define DNS_RD (1 << 8) /* recursion desired */
#define DNS_RA (1 << 7) /* recursion available */ 

#define MAX_DNS_SIZE 512 // largest valid UDP packet
#define MAX_ATTEMPTS 3



#pragma pack(push,1) // sets struct padding/alignment to 1 byte
class DNSanswerHdr {
public:
	u_short type;
	u_short classDef;
	u_short len;
	u_short TTL1;
	u_short TTL2;
};


/*

*/
class QueryHeader {
public:
	USHORT qType;
	USHORT qClass;
};

class FixedDNSheader {
public:
	USHORT ID;
	USHORT flags;
	USHORT questions;
	USHORT answers;
	USHORT authority;
	USHORT additional;

};
#pragma pack(pop) // restores old packing

string removeNumbers(string str)
{
	for (int i = 0; i < strlen(str.c_str()); i++)
	{
		char checkChar = str[i];
		int checkDigit = str[i];
		if (checkDigit >= 1 && checkDigit <= 47)
		{
			str[i] = '.';
		}
		else if (checkChar == '\n')
		{
			str[i] = '.';
		}
		checkChar += 1;
	}
	/*
	if (str[0] == '.')
	{
		str.erase(str.cbegin());
	}
	*/

	return str;
}
void makeDNSquestion(char* buf, string query)
{
	string tempQuery = query;
	string total = "";
	size_t size_of_next_word = 0;

	//  int indexParse = 0;
	int i = 0;
	while (true) {
		size_of_next_word = tempQuery.find('.');

		if (size_of_next_word == string::npos || size_of_next_word > tempQuery.size())
		{
			buf[i++] = strlen((char*)tempQuery.c_str());
			memcpy(buf + i, (char*)tempQuery.c_str(), strlen((char*)tempQuery.c_str()));
			i += strlen((char*)tempQuery.c_str());

			break;
		}
		buf[i++] = (char)size_of_next_word;
		memcpy(buf + i, (char*)tempQuery.c_str(), size_of_next_word);
		i += size_of_next_word;
		// if I have www, then we need www. removed
		tempQuery = tempQuery.substr(size_of_next_word + 1, query.size());
		//  size_last_pos = size_of_next_word;
	}

	buf[i] = 0; // last word NULL-terminated


}

bool checkErrors(char checkJump, string answer, bool addNewLine)
{

	if (answer == "random0")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		//++ invalid record: jump into fixed DNS header 
		// random0
		printf("\t++ invalid record: jump into fixed DNS header\n");
		return true;
		// return 0;
	}
	if (answer == "random1")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		//++ invalid section: not enough records
		// random1
		printf("\t++ invalid section: not enough records");
		return true;
		// return 0;
	}
	if (answer == "random7")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		// ++invalid record : truncated jump offset
		// random7
		printf("\t++ invalid record: truncated jump offset\n");
		return true;
		// return 0;
	}
	if (answer == "random4p1")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		printf("\t++ invalid record: truncated name\n");
		return true;
	}
	if (answer == "random4p2")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		printf("\t++ Invalid record: truncated RR answer header\n");
		return true;
	}
	if (answer == "random4p3")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		printf("\t++ invalid record: RR value length stretches the answer beyond packet\n");
		return true;
	}
	if (answer == "random5")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		// ++ invalid record: jump beyond packet boundary 
		// random5
		printf("\t++ invalid record: jump beyond packet boundary\n");
		return true;
		// return 0;
	}
	if (answer == "random6")
	{
		if (addNewLine)
		{
			printf("\n");
		}
		// ++ invalid record: jump beyond packet boundary 
		// random6
		printf("\t++ invalid record: jump loop\n");
		return true;
		// return 0;
	}
	return false;
}

string jump(u_char* ans, string& populateUncompressed, int& curPos, int& firstJump, bool& jumpCheck, bool& checkCompression, bool& breakCheck, int packet_size, string& jumpLoopCheck)
{

	// -64 is the signed jump offset
	// 11000000
	int currentPosInDec = ans[curPos];
	int currentPosInDecPlus1 = ans[curPos + 1];
	if (currentPosInDec == 0 && currentPosInDecPlus1 == 0)
	{
		breakCheck = true;
		return "random5";
	}
	int constantJumpCheckInDec = 0xC0;
	if (ans[curPos] >= 0xC0)
	{

		jumpCheck = true;
		int off = ((ans[curPos] & 0x3F) << 8) + ans[curPos + 1];
		int checkJump = jumpLoopCheck.find(to_string(off));
		if (checkJump != string::npos)
		{
			int check2 = jumpLoopCheck.substr(checkJump).find(to_string(off));
			if (check2 == 0 && firstJump > packet_size)
			{
				breakCheck = true;
				return "random6";

			}
		}

		jumpLoopCheck += to_string(off) + ',';
		// check copy string here
		string copyString((char*)ans + off); // skip the first number		

		int offCheckInitial = off + copyString.size();
		int findIndex = copyString.find(-64);
		// int copyStringAdd1 = ans[off + 1];
		if (findIndex != string::npos)
		{
			copyString = copyString.substr(0, findIndex);
		}
		int findTruncateName = copyString.find(-52);
		if (findTruncateName != string::npos)
		{
			if ((off > packet_size || off <= 12))
			{
				if (curPos + 1 == packet_size)
				{
					breakCheck = true;
					return "random7";
				}
				breakCheck = true;
				return "random5";
			}
			breakCheck = true;
			return "random4p1";
		}
		if (off < 12)
		{
			breakCheck = true;
			return "random0";
		}
		copyString = removeNumbers(copyString);
		// return copyString + jump(ans, off, name, firstJump, jumpOccur);i
		int offCheckUpdated = copyString.size() + off;
		offCheckInitial = off + copyString.size();


		if ((ans[offCheckInitial] == 0x0 || ans[offCheckUpdated] == 0x0) && off < 512) // this should be the 0th bit
		{
			return copyString;
		}
		firstJump++;
		return copyString + jump(ans, populateUncompressed, offCheckInitial, firstJump, jumpCheck, checkCompression, breakCheck, packet_size, jumpLoopCheck);

	}
	else
	{
		string modifier((char*)ans + curPos);
		// read the leading bytess
		int findIndex = modifier.find(-64);
		if (findIndex != string::npos)
		{

			modifier = modifier.substr(0, findIndex);
			int holdChangedPos = modifier.size() + curPos; // ajdust for recursive case
			// printf(" inside happy path, find a jump \n ");
			int findTruncateName = modifier.find(-52);
			if (findTruncateName != string::npos)
			{
				breakCheck = true;
				return "random4p1";
			}

			// second jump handle
			// indIndex += curPos;

			int off = ((ans[curPos + findIndex] & 0x3F) << 8) + ans[curPos + findIndex + 1];
			int checkJump = jumpLoopCheck.find(to_string(off));
			if (checkJump != string::npos)
			{
				int check2 = jumpLoopCheck.substr(checkJump).find(to_string(off));
				if (check2 == 0 && firstJump > packet_size)
				{
					breakCheck = true;
					return "random6";

				}
			}
			jumpLoopCheck += to_string(off) + ',';
			if (off < 12)
			{
				breakCheck = true;
				return "random0";
			}

			if (firstJump == 0)
			{
				populateUncompressed = modifier;
				checkCompression = true;
			}
			modifier = removeNumbers(modifier);
			if (ans[holdChangedPos] == 0x0) // this should be the 0th bit
			{
				return modifier;
			}
			else
			{
				firstJump++;
				return modifier + jump(ans, populateUncompressed, holdChangedPos, firstJump, jumpCheck, checkCompression, breakCheck, packet_size, jumpLoopCheck);
			}

		}
		modifier = removeNumbers(modifier);
		int findTruncateName = modifier.find(-52);
		if (findTruncateName != string::npos)
		{
			breakCheck = true;
			return "random4p1";
		}
		return modifier;
		// uncompressed answer


	}


	// 00111111 0x3F
	// 11000011 0xC3
	// 11 U ans[curPos + 1]
	// ans[curPos +1] right 8 bits
	// 
	// find the next jump and then return the first jump + next;

	// accurate jumps here this was 45, when we needed 44
	// pastHeader = pastHeader + answer.size() -1 + sizeof(reply); // now theres two empty bytes,

	// return copyString;
}
string processJump(u_char* buf, int& pastHeader, int firstJumpPos, int packet_size) {
	// logic here is going to be, see if I jump at the front
	if ((char)buf[pastHeader] == -52)
	{
		// printf("++ invalid section: not enough records \n");
		return"random1";
	}


	string answer = "";
	bool jumpOccur = false;//enter into at 0xc0
	bool checkCompressed = false;
	bool breakCheck = false;
	int jumpCount = 0;
	string uncompressed = "";
	string jumpLoopCheck = "";
	// answer += jump( buf, pastHeader, name, firstJumpPos, jumpOccur);
	// int countJumps = 0;
	answer += jump(buf, uncompressed, pastHeader, jumpCount, jumpOccur, checkCompressed, breakCheck, packet_size, jumpLoopCheck);

	if (breakCheck == true)
	{
		while (answer[0] == '.')
		{
			answer.erase(answer.cbegin());
		}
		return answer;
	}
	// printf(" jump count %d", countJumps);
	if (checkCompressed == true)
	{ // this is going to be if theres a jump and another jump 
		// printf(" x new jump ");
		pastHeader += uncompressed.size() + 2; // land on class type
		// pastHeader += 1; // shift bits by 2
		return answer;
	}
	if (jumpOccur == false)
	{

		pastHeader += answer.size(); // land on class type

	}
	pastHeader += 1;

	if (isalpha(buf[pastHeader - 2]) && buf[pastHeader - 2] == answer[answer.size() - 1] && buf[pastHeader - 1] == 0 && buf[pastHeader] == 0)
	{
		// if when we left it was a alphabetical, we have not missed a byte, we are not on a zero byte, and ar not missing ajump
		// printf(" remove me later\n");
		pastHeader--;
	}
	char a = buf[pastHeader + (int)sizeof(DNSanswerHdr*)];
	int b = (int)sizeof(DNSanswerHdr);
	if ((char)buf[pastHeader + (int)sizeof(DNSanswerHdr*)] == -52)
	{
		return "random4p2";
	}


	return answer;
}


int runMainFunction(string queryReplace, string DNSReplace)
{
	string query = queryReplace;
	string DNS = DNSReplace;
	printf("Lookup  : %s\n", query.c_str());


	DWORD IP = inet_addr((char*)query.c_str());
	bool amI1or12 = true;
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		amI1or12 = true;

	}
	else
	{
		amI1or12 = false;
		// here parse the query and remove the .com adding .in-addr.arpa
		int findPeriod = query.find('.');
		string copyQuery = query;
		string periodString = ".";
		string subQuery = "";
		subQuery.insert(0, copyQuery.substr(0, findPeriod).c_str());
		subQuery.insert(0, periodString.c_str());
		copyQuery = copyQuery.substr(findPeriod + 1);
		findPeriod = copyQuery.find('.');

		subQuery.insert(0, copyQuery.substr(0, findPeriod).c_str());
		subQuery.insert(0, periodString.c_str());
		copyQuery = copyQuery.substr(findPeriod + 1);
		findPeriod = copyQuery.find('.');

		subQuery.insert(0, copyQuery.substr(0, findPeriod).c_str());
		subQuery.insert(0, periodString.c_str());
		copyQuery = copyQuery.substr(findPeriod + 1);
		findPeriod = copyQuery.find('.');

		subQuery.insert(0, copyQuery.substr(0, findPeriod).c_str());



		query = subQuery + ".in-addr.arpa";


	}

	// char query[] = �www.google.com�;



	int pkt_size = strlen(query.c_str()) + 2 + sizeof(FixedDNSheader) + sizeof(QueryHeader); // handdle space issues 
	char* buf = new char[pkt_size];
	FixedDNSheader* fdh = (FixedDNSheader*)buf;
	QueryHeader* qh = (QueryHeader*)(buf + pkt_size - sizeof(QueryHeader));


	fdh->ID = htons(0xAA03);
	fdh->flags = htons(DNS_QUERY | DNS_RD | DNS_STDQUERY);
	fdh->questions = htons(1);
	fdh->answers = htons(0);
	fdh->authority = htons(0);
	fdh->additional = htons(0);

	if (amI1or12 == true)
	{
		qh->qType = htons(DNS_A);
	}
	else
	{
		qh->qType = htons(DNS_PTR);

	}
	qh->qClass = htons(DNS_INET);
	// fixed field initialization
	// https://datatracker.ietf.org/doc/html/rfc1035


	// handle the dns change here 
	// 3www4tamu3edu0 reply whould be

	makeDNSquestion((char*)(fdh + 1), (char*)query.c_str());
	// cout << " the modified query is " << modifiedQuery << std::endl;

	// printf(" %3u, %3u , %3u, %3u, %3u , %3u  ", htons( fdh->ID ), htons( fdh->flags ) , htons( fdh->questions ) , htons( fdh->answers ) , htons( fdh->authority )  , htons( fdh->additional ) );
	// char* ptr = (char*)(fdh + 1);
	// printf(" print buffer ", ptr);

	printf("Query   : %s, type %d, TXID 0x%4x\n", query.c_str(), htons(qh->qType), htons(fdh->ID));


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
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	// handle errors
	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(0);
	if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf(" bind error %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 0;

	}

	struct sockaddr_in remote;
	memset(&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_addr.S_un.S_addr = inet_addr(DNS.c_str()); // server�s IP
	remote.sin_port = htons(53); // DNS port on server

	printf("Server  : %s\n", DNS.c_str());
	printf("********************************\n");

	if (sendto(sock, buf, pkt_size, 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR)
	{
		printf(" send to error %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 0;

	}
	// handle errors 


	int count = 0;
	bool TerminateRun = false;
	while (count++ < MAX_ATTEMPTS)
	{
		// send request to the server
		clock_t start = clock();
		timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		printf("Attempt %d with %d bytes... ", count - 1, pkt_size);
		fd_set fd;
		FD_ZERO(&fd); // clear the set
		FD_SET(sock, &fd); // add your socket to the set
		int available = select(0, &fd, NULL, NULL, &timeout);
		if (available > 0)
		{
			char buf[MAX_DNS_SIZE];
			struct sockaddr_in response;
			int responseSize = sizeof(response);
			int bytes = recvfrom(sock, buf, MAX_DNS_SIZE, 0, (struct sockaddr*)&response, &responseSize);
			if (bytes != SOCKET_ERROR)
			{
				// error processing
				// check if this packet came from the server to which we sent the query earlier



			}
		}

	WSACleanup();
	closesocket(sock);

	return 0;
}


int main(int argc, char* argv[])
{

	if (argc != 3)
	{
		cout << " CHECK HOW YOU RUNNING OR WHAT WE ARE TESTING ./hw2.exe query ip \n";
		return 0;
	}

	runMainFunction(argv[1], argv[2]);

	/*




	string query("www.google.com");
	string DNS ( "8.8.8.8" );

	string query("www.dhs.gov");
	string DNS("128.194.135.85");

	string query("randomA.irl" );
	string DNS ( "128.194.135.82" );

	 string query("yahoo.com" );
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
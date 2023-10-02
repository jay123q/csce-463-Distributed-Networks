// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
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
	u_int TTL;
	u_short len;
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
/*
processJump() {
   // extract everything you can without jumping
   // Identify where the next jump is
   return whatIJustParsed ++ processJump(jmpLocation);
}
*/
string jump(u_char * ans, int curPos, char* name, int firstJump )
{	
	/*
		if size is 0 of the final array, return the substring of all replies back
	*/
	// string copyString = "";
	u_char jumpBack = ans[curPos];
	// int jumpTo = curPos - jumpBack;
	string copyString( (char *) ans + jumpBack + 1 ) ; // skip the first number
	// printf(" answer is = %s", copyString);

	int currentPosInDec = ans[curPos];
	int constantJumpCheckInDec = 0xC0;
	if (ans[curPos] >= 0xC0)
	{
		printf(" gotta jump more \n ");
		// jump more
	}
	else
	{
		// uncompressed keep running and shutup
		printf(" can proceed \n ");
	}
	

	int off = ((ans[curPos] & 0x3F) << 8) + ans[curPos + 1];
	if (ans[off] == 0) // this should be the 0th bit
	{
		return "";
	}
	return copyString + jump(ans, off, name, firstJump);

	// 00111111 0x3F
	// 11000011 0xC3
	// 11 U ans[curPos + 1]
	// ans[curPos +1] right 8 bits
	// 
	// find the next jump and then return the first jump + next;
						
	// accurate jumps here this was 45, when we needed 44
	// pastHeader = pastHeader + answer.size() -1 + sizeof(reply); // now theres two empty bytes,

	return copyString;
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


	// sendDns.generateQuery(argv[1], argv[2])
	 string query("yahoo.com" );
	 string DNS ( "128.194.135.85" );

	/*
	string query("randomA.irl" );
	string DNS ( "128.194.135.82" );
	*/
	printf("Lookup  : %s\n", query.c_str() );


	DWORD IP = inet_addr((char* ) query.c_str());
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
		subQuery.insert(0, copyQuery.substr(0,findPeriod).c_str());
		subQuery.insert(0,periodString.c_str());
		copyQuery = copyQuery.substr(findPeriod +1 );
		findPeriod = query.find('.');
		
		subQuery.insert(0, copyQuery.substr(0,findPeriod).c_str());
		subQuery.insert(0,periodString.c_str());
		copyQuery = copyQuery.substr(findPeriod +1);
		findPeriod = query.find('.');
		
		subQuery.insert(0, copyQuery.substr(0,findPeriod).c_str());
		subQuery.insert(0,periodString.c_str());
		copyQuery = copyQuery.substr(findPeriod +1);
		findPeriod = query.find('.');
		
		subQuery.insert(0, copyQuery.substr(0, findPeriod).c_str());



		query = subQuery + ".in-addr.arpa";


	}

	// char query[] = “www.google.com”;



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

	makeDNSquestion( (char * ) (fdh+1),  (char *) query.c_str() );
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
	remote.sin_addr.S_un.S_addr = inet_addr(DNS.c_str()); // server’s IP
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
	while (count++ < MAX_ATTEMPTS)
	{
		// send request to the server
		clock_t start = clock();
		timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		printf("Attempt %d with %d bytes... ", count, pkt_size );
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

				if (response.sin_addr.S_un.S_addr != remote.sin_addr.S_un.S_addr || response.sin_port != remote.sin_port)
				{
					// bogus reply, complain
					printf(" DNS or PORT error  %d\n", WSAGetLastError());
					break;
					// closesocket(sock);
					// WSACleanup();
					// delete buf;
					// return 0;					
				}

				// search for packet
				double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
				printf("response in %.1f ms with %d bytes \n", duration * 1000, bytes);

				
				// error checking here
				// printf(" print buffer %s",saveBuffer);
				FixedDNSheader* fdhRec = (FixedDNSheader*)buf;
				// printf("succeeded with Rcode = %d", fdhRec->answers);

				// read fdh->ID and other fields
			

				printf("\tTXID 0x%.4x flags 0x%x questions %d answers %d authority %d additional %d\n",
					htons(fdhRec->ID),
					htons(fdhRec->flags),
					htons(fdhRec->questions),
					htons(fdhRec->answers),
					htons(fdhRec->authority),
					htons(fdhRec->additional)

					);
				// https://datatracker.ietf.org/doc/html/rfc1035 
				// auto rCodeCheck = htons(fdhRec->flags);
				if (  (htons(fdhRec->flags) & 15 ) != 0)
				{
					printf("\tfailed with Rcode = %d\n ", htons(fdhRec->flags));
					// closesocket(sock);
					// WSACleanup();
					// delete buf;
					break;
					// return 0;
				}
				printf("\tsucceeded with Rcode = %d\n", htons(fdhRec->flags));



				if (fdhRec->ID != fdh->ID) {
					printf("\t++ invalid reply: TXID mismatch, sent %0.4x, received x%0.4x\n", htons(fdh->ID), htons(fdhRec->ID));
				//	WSACleanup();
				//	closesocket(sock);
					break;
					// delete buf;
					// return 0;
				}

				int offset = 0;
			// parse questions and arrive to the answer section
					int pastHeader = 13; // this is 12, adding 1 to remove the leading number
				if (htons(fdhRec->questions) > 0)
				{
					printf("\t------------ [questions] ----------\n");
					/*
					char bufProcess[512];
					memcpy(bufProcess, buf, sizeof(buf));
					printf("\t  %c", bufProcess[i]);
					for (int i = 0; i < sizeof(bufProcess); i++)
					{
						printf("\t  %c", bufProcess[i]);
						
					}
					*/
					for (int i = 0; i < htons(fdhRec->questions); i++)
					{

						string linkCheck(buf + pastHeader);
						// remove the number in the middle
						// only run once
						/*
						if (amI1or12 == true && i == 0)
						{
							linkCheck[linkCheck.size() - 4] = '.';
						}
						else if( i == 0)
						{
							// 19.138.194.128.in-addr.arpa 
							linkCheck[linkCheck.size() - 4] = '.';
							linkCheck[linkCheck.size() - 14] = '.';
						}
						*/
						// loop through link buffer and change all unknown chars into " "
						for (int i = 0; i < strlen( linkCheck.c_str() ); i++)
						{ 
							// char checkChar = linkCheck[i];
							int checkDigit = linkCheck[i];
							if (checkDigit >= 1 && checkDigit <= 9 )
							{
								linkCheck[i] = '.';
							}
						}
						
						


						printf("\t\t");
						printf(linkCheck.c_str());

						pastHeader = linkCheck.size() + pastHeader + 2 ; // now theres two empty bytes,
						int typeBuf;
						typeBuf = (int) buf[pastHeader];
						printf(" %d",typeBuf);
						pastHeader += 2;

						// pastHeader += 2; // next single bytes 
						int classBuf;
						classBuf = (int)buf[pastHeader];
						printf(" %d\n",classBuf);
						pastHeader += 2;

						// printf("%s", saveBuffer);
					}

				}				
				if (htons(fdhRec->answers) > 0)
				{
					printf("\t------------ [answers] ----------\n");
					// error check
						// this is a consistent 16 byte jump, I can take advantage of this

					for (int i = 0; i < htons(fdhRec->answers); i++)
					{
						string passIntoJump(buf + pastHeader);
						int holdOldPointer = pastHeader;
						char name[MAX_DNS_SIZE];

						string answer = jump( (u_char * ) buf , pastHeader, name , pastHeader );
						DNSanswerHdr* reply = (DNSanswerHdr*) ( answer.c_str() + pastHeader );
						int a = sizeof(reply);
						a++;

						for (int i = 0; i < strlen(answer.c_str()); i++)
						{
							// char checkChar = linkCheck[i];
							int checkDigit = answer[i];
							if (checkDigit >= 1 && checkDigit <= 9)
							{
								answer[i] = '.';
							}
						}
						// remove the number in the middle
						// adjust byte pointer


						string holdOldStringOut = answer;
						printf("\t\t");
						printf(holdOldStringOut.c_str()); // this is the old web dns


						// remove answer.size() null ptr
						pastHeader = pastHeader + answer.size() -1 + sizeof(reply); // now theres two empty bytes,
						if (htons(reply->type) == DNS_A)
						{
							printf("A ");
						}
						else if (htons(reply->type) == DNS_NS)
						{
							printf("NS ");
						}
						else if (htons(reply->type) == DNS_CNAME)
						{
							printf("CNAME ");

						}
						else if (htons(reply->type) == DNS_PTR)
						{
							printf("PTR ");

						}


						/*
						pastHeader += 2; // 2 for the \0

						int classifyType;
						pastHeader += 4;
						classifyType = (int)buf[pastHeader];
						*/
						// we are at the end of one// 1 byte to adjust, 2 to reach the Q

						// char name[MAX_DNS_SIZE];
						
						
						
						// replies cannot be largert than NAME




						printf(answer.c_str());
					}
				}
				if (htons(fdhRec->authority) > 0)
				{
					printf("\t------------ [authority] ----------\n");
					// error check
				}
				if (htons(fdhRec->additional) > 0)
				{
					printf("\t------------ [additional] ----------\n");
					// error check
				}
			// suppose off is the current position in the packet
				// int off = 0;
				// DNSanswerHdr * dah = (DNSanswerHdr*)(buf + off);
				// read dah->len and other fields 
			// parse the response
			// break from the loop
				printf("breaking line 335 \n");
				break;
			}
			else
			{
				printf(" recfrom error %d\n", WSAGetLastError());
				break;
				// WSACleanup();
				// closesocket(sock);
			}

		}
		else if (available == 0)
		{
			cout << " time out occured \n";
		}
		else
		{
			printf(" select error %d\n", WSAGetLastError());
			// closesocket(sock);
			// WSACleanup();
			//return 0;
		}


	}

		WSACleanup();
		closesocket(sock);

	return 0;
}

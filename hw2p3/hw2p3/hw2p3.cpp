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

string jump(string ans , int curPos)
{	
	/*
		if size is 0 of the final array, return the substring of all replies back
	*/
	printf(" answer is = %s", ans);

	int off = ((ans[curPos] & 0x3F) << 8) + ans[curPos + 1];
	
	return " ";
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
	string query("randomA.irl" );
	//string DNS ( "128.194.135.85" );
	string DNS ( "128.194.135.82" );

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
		query = query + ".in-addr.arpa";


	}

	// char query[] = �www.google.com�;



	int pkt_size = strlen(query.c_str()) + 2 + sizeof(FixedDNSheader) + sizeof(QueryHeader); // handdle space issues 
	char* buf = new char[pkt_size];
	FixedDNSheader* fdh = (FixedDNSheader*)buf;
	QueryHeader* qh = (QueryHeader*)(buf + pkt_size - sizeof(QueryHeader));


	fdh->ID = htons(1237);
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
	printf("Query   : %s, type %d, TXID 0x%4d\n", query.c_str(), htons(qh->qType), htons(fdh->ID));


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
				printf("done in %.1f ms with %d bytes \n", duration * 1000, bytes);

				
				// error checking here
				// printf(" print buffer %s",saveBuffer);
				FixedDNSheader* fdhRec = (FixedDNSheader*)buf;
				// printf("succeeded with Rcode = %d", fdhRec->answers);

				// read fdh->ID and other fields
			

				printf("\tTXID %.4x flags %d questions %d answers %d authority %d additional %d\n",
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
					int moveBuf = 13;
					string test(buf + moveBuf);
					int indexInteger = test.find("/[1 - 9]/");
					printf(test.c_str());



					moveBuf = sizeof(test);
					string queryType(buf + 2*pkt_size);
					// printf("%s", saveBuffer);
					 // printf("%s", htons(fdhRec->questions));
					// error check
					// string questions( (char * ) htons(fdhRec->questions) );
					// printf(" questions print  %s", questions);

				}				
				if (htons(fdhRec->answers) > 0)
				{
					printf("\t------------ [answers] ----------\n");
					// error check
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

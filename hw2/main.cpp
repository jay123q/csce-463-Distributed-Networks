// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
#include "pch.h"
#include <string>
#include <windows.h>
#include <iostream>
#include <vector>
#include "DNShelper.h"
#pragma comment(lib, "Ws2_32.lib")

#pragma warning(disable:4996)
#pragma warning(disable:4099)
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


int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << " CHECK HOW YOU RUNNING OR WHAT WE ARE TESTING ./hw2.exe query ip \n";
		return 0;
	}

	DnsHelper sendDns;
	// sendDns.generateQuery(argv[1], argv[2])
	string query( "www.xyz.com" );
	string DNS ( "128.194.135.79" );


	DWORD IP = inet_addr(query.c_str());
	sendDns.attemptCount = 0;
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		sendDns.amI1or12 = true;

	}
	else
	{
		sendDns.amI1or12 = false;
		// here parse the query and remove the .com adding .in-addr.arpa
		query = query + ".in-addr.arpa";

	}
	// this->ip = server.sin_addr;
	sendDns.ip = IP;


	// char host[] = “www.google.com”;

	int pkt_size = strlen(query.c_str()) + 2 + sizeof(FixedDNSheader) + sizeof(QueryHeader); // handdle space issues 
	char* buf = new char[pkt_size];
	FixedDNSheader* fdh = (FixedDNSheader*)buf;
	QueryHeader* qh = (QueryHeader*)(buf + pkt_size - sizeof(QueryHeader));

	// fixed field initialization
	// https://datatracker.ietf.org/doc/html/rfc1035
	fdh->ID = htons(1);
	fdh->flags = htons(DNS_QUERY | DNS_RD | DNS_STDQUERY);
	fdh->questions = htons(1);
	fdh->answers = htons(0);
	fdh->authority = htons(0);
	fdh->additional = htons(0);


	if (sendDns.amI1or12 == true)
	{
		qh->qType = htons(DNS_A);
	}
	else
	{
		qh->qType = htons(DNS_PTR);

	}
	qh->qClass = htons(DNS_INET);
	// handle the dns change here 
	// 3www4tamu3edu0 reply whould be

	string modifiedQuery = sendDns.replacePeriodWithNumber(query);
	cout << " the modified query is " << modifiedQuery << std::endl;
	// makeDNSquestion(fdh + 1, modifiedQuery);
	// printf("Query\t: %s, type %d, TXID 0x%04d\n", query.c_str(), htons(qh->qType), htons(fdh->ID));
	// sendto(sock, buf, );
	// delete buf;
	// ayo these are guess, I cant get the vs community to open the properties tab and then show the defaul to throw in the texts
	// no idea tf is happening.

	return 0;
}

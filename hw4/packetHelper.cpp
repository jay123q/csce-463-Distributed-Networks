
#include "pch.h"
#include "packetHelper.h"


packetHelper::packetHelper(std::string host) {
	//handle socket creation and connection 
	storeIP = inet_addr(host.c_str());
	countSeq = 1;
	WSADATA wsaData;
	socketReceiveReady = CreateEvent(NULL, false, false, NULL);
	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error sart up %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	// open a TCP socket
	this->sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock == INVALID_SOCKET)
	{
		printf(" unable to create a raw socket: error %d \n", WSAGetLastError());
		return;
	}
	this->pd = new packetDetails[N+1];
	errorBreak = false;

	packet_size = sizeof(ICMPHeader);


	memset(&remote, 0, sizeof(sockaddr_in));
	remote.sin_family = AF_INET;
	// remote.sin_port = htons(53);

	if (storeIP == INADDR_NONE)
	{
		struct hostent* r;
		r = gethostbyname(host.c_str());
		if (r == NULL)
		{
			 // printf("Connection error: %d\n", WSAGetLastError());
			errorBreak = true;
			return;
		}
		else // take the first IP address and copy into sin_addr
		{
			memcpy((char*)&(remote.sin_addr), r->h_addr, r->h_length);
		}

	}
	else
	{
		remote.sin_addr.S_un.S_addr = storeIP;
	}
	IPforlastPrint = inet_ntoa(remote.sin_addr);
	WSAEventSelect(this->sock, this->socketReceiveReady, FD_READ);
	printf("Tracerouting to %s...\n", IPforlastPrint.c_str());
}
packetHelper::~packetHelper() {

}
void packetHelper::createPacket( int seq)
{


	memset(pd[seq].send_buf, 0, sizeof(MAX_ICMP_SIZE));
	pd[seq].icmpPacket.type = (u_char) ICMP_ECHO_REQUEST;
	pd[seq].icmpPacket.code = (u_char) 0;
	pd[seq].icmpPacket.id = (u_short) GetCurrentProcessId();
	pd[seq].icmpPacket.seq = (u_short)seq;
	pd[seq].icmpPacket.checksum = 0;
	pd[seq].icmpPacket.checksum = cc.ip_checksum((u_short*)pd[seq].send_buf, packet_size);
	memcpy(pd[seq].send_buf, &pd[seq].icmpPacket, sizeof(ICMPHeader));
	pd[seq].probe = 1;
	pd[seq].icmpComplete = false;
	firstIteration = true;


}
void packetHelper::sendPacket( int seq )
{
	int ttl = seq;
	if (setsockopt(sock, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
	{
		printf("setsockopt failed with %d\n", WSAGetLastError());
		closesocket(sock);
		// some cleanup
		exit(-1);
	}
	pd[seq].startTimer = clock();

	// we need to take to send the ICMP, and the IPheader part the server will take car of
	if (sendto(sock, (char * ) pd[seq].send_buf , sizeof(ICMPHeader), 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR)
	{
		printf(" send to error %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		exit(-1);

	}

}
void packetHelper::resendPacket(int seq)
{
	// std::cout << " resend packet seq # " << seq << std::endl;
	if (sendto(sock, (char*) pd[seq].send_buf, sizeof(ICMPHeader), 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR)
	{
		// printf(" send to error %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		exit(-1);

	}
}


void packetHelper::retransmitPackets() {
	for (int i = sendNumber; i < N+1; i++)
	{
		if (pd[i].icmpComplete != true  && pd[i].probe <= 3)
		{

				if (pd[i].probe < 3)
				{
					// std::cout << "resending packet " << i << std::endl;
					sendPacket(i);
				}
				pd[i].probe += 1;
		}
		else if(pd[i].icmpComplete == false && pd[i].probe > 3)
		{
			// next time around we know we did not send
			pd[i].icmpComplete = true;
			pd[i].printString = std::to_string(i) + " *";
			

			// printf(" empty  domain failed to reach edit me later in retransmist \n");
		}
	}

}


bool packetHelper::checkComplete()
{
	for (int i = sendNumber; i < N+1; i++)
	{
		if (pd[i].icmpComplete == false)
		{
			return false;
		}
	}
	return true;
}

void packetHelper::finalPrint() {
	for (int i = sendNumber; i < N+1; i++)
	{
		std::cout << pd[i].printString << std::endl;
	}
}

std::string packetHelper::DNSlookup(std::string IP) {
#ifndef reportWork
	struct hostent* r;


	r = gethostbyaddr(IP.c_str(), sizeof(remote), AF_INET);
	/*
	struct hostent* a;
	a = gethostbyname(IP.c_str());
	std::cout << a << std::endl;
	*/
	
	if (r == NULL)
	{
		std::string ipAddy = "<no DNS entry>";
		return ipAddy;
	}
	else
	{
		std::string ipAddy(r->h_name);
		return ipAddy;

	}
#endif // reportWork
	return " report work ";
}


double packetHelper::setRTO()
{

	int validRtt = 0;
	double generalAverage = 0.0;
	for (int i = sendNumber; i < N; i++)
	{
		if (pd[i].icmpComplete == false)
		{
			if (pd[i - 1].icmpComplete == true && pd[i + 1].icmpComplete == true)
			{
				double average = (pd[i - 1].RTT + pd[i + 1].RTT) / 2;
				return average;
			}
			else
			{
				return generalAverage / i;
			}
		}
		else
		{
			generalAverage += pd[i].RTT;
			validRtt += 1;
		}
	}

	return generalAverage/ validRtt;
}


void packetHelper::recvPackets()
{

	fd_set readFds;
	timeval timeout;
	FD_ZERO(&readFds); // this sets the file descriptor 
	FD_SET(sock, &readFds); // assign a socket to a descriptor
	if (firstIteration)
	{
		firstIteration = false;
		timeout.tv_sec = 0;
		timeout.tv_usec = (500 * 1000);
	}
	else
	{
		double RTO = setRTO();
		timeout.tv_sec = RTO/1000;
		timeout.tv_usec = (RTO/1000 - timeout.tv_sec) * 1000;

	}

	/*
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	*/

	// set / RESET RTT HERE
	int countGarbage = 0;
	int oldSeq = 0;

	int ret = select(0, &readFds, NULL, NULL, &timeout);
	if (ret > 0)
	{
		while (true)
		{
				u_char buf[MAX_REPLY_SIZE];
				memset(buf, 0, MAX_REPLY_SIZE);
				struct sockaddr_in response;
				int responseSize = sizeof(response);

				IPHeader* routerIpHeader = (IPHeader*)buf;
				ICMPHeader* routerIcmpHead = (ICMPHeader*)(routerIpHeader + 1);
				IPHeader* originalIpHeader = (IPHeader*)(routerIcmpHead + 1);
				ICMPHeader* originalIcmpHeader = (ICMPHeader*)(originalIpHeader + 1);
				int bytes = recvfrom(sock, (char*)&buf, MAX_REPLY_SIZE, 0, (struct sockaddr*)&response, &responseSize);
				// printf(" icmp seq %d | icmp id %d \n", originalIcmpHeader->seq , originalIcmpHeader->id);

				if (bytes >= 56 && routerIcmpHead->type == ICMP_TTL_EXPIRED && routerIcmpHead->code == 0)
				{
				//	printf(" icmp_ttl_expired \n");
					countSeq += 1;
					pd[originalIcmpHeader->seq].icmpComplete = true;
					// DNS RESPONSE HERE 
					// thread
					// gethostbyname
					std::string printME = "";
					// seq
					std::string seqPrint = std::to_string(originalIcmpHeader->seq) + " ";
					printME += seqPrint;
					// DNS
					u_long temp = (routerIpHeader->source_ip);
#ifdef reportWork34
					unique_ip.insert(temp);
					countIp++;
#endif
					u_char* IPArray = (u_char*)&temp;
					std::string IP((char*)IPArray);
					 printME += DNSlookup(IP);
					// IP
					std::string Ipheader = " (" + std::to_string(IPArray[0]) + '.' + std::to_string(IPArray[1]) + '.' + std::to_string(IPArray[2]) + '.' + std::to_string(IPArray[3]) + ") ";
					printME += Ipheader;
					// RTT
					pd[originalIcmpHeader->seq].RTT = ((double)(clock() - pd[originalIcmpHeader->seq].startTimer ) / CLOCKS_PER_SEC)*1000;
					std::string rttPrint = std::to_string(pd[originalIcmpHeader->seq].RTT)+" ms ";
					printME += rttPrint;
					// PROBE
					std::string probePrint = "("+std::to_string(pd[originalIcmpHeader->seq].probe)+")";
					printME += probePrint;
					pd[originalIcmpHeader->seq].printString = printME;
				//	 std::cout << printME << std::endl;
				}
				else if (bytes == -1 && routerIcmpHead->type == ICMP_ECHO_REPLY && routerIcmpHead->code == 0)
				{
					// last packet recieved
					// store RTT of last packet in another dumby variable?
					// check helper for all packets outbound to know if I can store
					/*
					std::cout << " router source name " << routerIpHeader->source_ip << std::endl;
					std::cout << " router  destin name " << routerIpHeader->dest_ip << std::endl;
					std::cout << " original source name " << originalIpHeader->source_ip << std::endl;
					std::cout << " original  destin name " << originalIpHeader->dest_ip << std::endl;
					std::cout << " seq num " << routerIcmpHead->seq << std::endl;
					std::cout << " my count seq " << countSeq << std::endl;
					*/
					std::string printME = "";
					// seq

					// DNS
					// https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getnameinfo
					char returnHostName[256];
					char serviceBuffer[256];
					sockaddr_in address;
					memset(&address, 0, sizeof(address));
					address.sin_family = AF_INET;
					address.sin_addr.s_addr = inet_addr(IPforlastPrint.c_str());
					int response = getnameinfo((sockaddr*)&address,sizeof(address),returnHostName,256,serviceBuffer,256,0);
					// printME += DNSlookup(IPforlastPrint) +" ";
					// 
					// IP
					std::string lastName(returnHostName);
					printME += lastName+" ";

					u_char* IPArray = (u_char*)IPforlastPrint.c_str();
					std::string IP((char*)IPArray);
					printME += IP;
					// RTT
					RTTlast = ((double)(clock() - pd[countSeq].startTimer) / CLOCKS_PER_SEC) * 1000;
					printME += " " + std::to_string(RTTlast);
					// PROBE
					std::string probePrint = " (" + std::to_string(pd[countSeq].probe) + ")";
					printME += probePrint;
					printLast = printME;
					std::cout << printLast << std::endl;
					break;

				}
				else
				{

					printf(" error detected is: ");
					handleError(routerIcmpHead->type, routerIcmpHead->code);
					errorBreak = true;
				}


		}
	}
	else if (ret == 0)
	{
		printf(" timed out \n");
		countSeq++;
		return;
	}
	else
	{
		printf(" no packets recieved \n");
		return;
	}
}


bool packetHelper::handleError(int type, int code)
{
	if (type == 3 && code == 0)
	{
		printf(" Destination Network unreachable \n");
		return true;
	}
	else if (type == 3 && code == 1)
	{
		printf(" Destination Host unreachable \n");
		return true;
	}
	else if (type == 3 && code == 2)
	{
		printf(" Destination Protocol unreachable \n");
		return true;
	}
	else if (type == 3 && code == 3)
	{
		printf(" Destination Port unreachable \n");
		return true;
	}
	else if (type == 3 && code == 4)
	{
		printf(" Fragmentation Required \n");
		return true;
	}
	else if (type == 3 && code == 5)
	{
		printf(" Source route failed \n");
		return true;
	}
	else if (type == 3 && code == 6)
	{
		printf(" Destination network unkown \n");
		return true;
	}
	else if (type == 3 && code == 7 )
	{
		printf(" Destination host unknown \n");
		return true;
	}
	else if (type == 3 && code == 8)
	{
		printf(" Source host isolated \n");
		return true;
	}
	else if (type == 3 && code == 9)
	{
		printf(" Network administratively prohibited \n");
		return true;
	}
	else if (type == 3 && code == 10)
	{
		printf(" Host administratively prohibited \n");
		return true;
	}
	else if (type == 3 && code == 11)
	{
		printf(" Network unreachable for ToS \n");
		return true;
	}
	else if (type == 3 && code == 12)
	{
		printf(" Host unreachable for ToS \n");
		return true;
	}
	else if (type == 3 && code == 13)
	{
		printf(" Communication administratively prohibited \n");
		return true;
	}
	else if (type == 3 && code == 14)
	{
		printf(" Host Precedence Violation \n");
		return true;
	}
	else if (type == 3 && code == 15)
	{
		printf(" Precedence cutoff in effect \n");
		return true;
	}
	else if (type == 11 && code == 0 )
	{
		printf(" TTL expired in transit \n");
		return true;
	}
	else if (type == 11 && code == 1 )
	{
		printf(" Fragment reassembly time exceeded \n");
		return true;
	}
	else if (type == 12 && code == 0 )
	{
		printf(" Pointer indicates the error \n");
		return true;
	}
	else if (type == 12 && code == 1 )
	{
		printf(" Missing a required option \n");
		return true;
	}
	else if (type == 12 && code == 2 )
	{
		printf(" Bad length \n");
		return true;
	}
	else if (type == 40 )
	{
		printf(" Photuris, Security failures \n");
		return true;
	}
	else if (type == 43 && code == 1)
	{
		printf(" Malformed Query \n");
		return true;
	}
	else if (type == 43 && code == 2)
	{
		printf(" No Such Interface \n");
		return true;
	}
	else if (type == 43 && code == 3)
	{
		printf(" No Such Table Entry \n");
		return true;
	}
	else if (type == 43 && code == 4)
	{
		printf(" Multiple Interfaces Satisfy Query \n");
		return true;
		
	}
	return false;
} 
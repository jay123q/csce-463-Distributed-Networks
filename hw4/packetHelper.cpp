
#include "pch.h"
#include "packetHelper.h"

packetHelper::packetHelper(DWORD IP, std::string host) {
	//handle socket creation and connection 
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
	}
	this->pd = new packetDetails[30];


	packet_size = sizeof(ICMPHeader);


	memset(&remote, 0, sizeof(sockaddr_in));
	remote.sin_family = AF_INET;
	// remote.sin_port = htons(53);

	if (IP == INADDR_NONE)
	{
		struct hostent* r;
		r = gethostbyname(host.c_str());
		if (r == NULL)
		{
			printf("Connection error: %d\n", WSAGetLastError());
			return;
		}
		else // take the first IP address and copy into sin_addr
		{
			memcpy((char*)&(remote.sin_addr), r->h_addr, r->h_length);
		}

	}
	else
	{
		remote.sin_addr.S_un.S_addr = IP;
	}

	WSAEventSelect(this->sock, this->socketReceiveReady, FD_READ);
	printf("Tracerouting to %s...\n", inet_ntoa(remote.sin_addr));
}
packetHelper::~packetHelper() {

}
void packetHelper::createPacket( int seq)
{
	int ttl = seq + 1;
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
	pd[seq].giveUp = false;
	dnsComplete = false;
	pd[seq].dnsIp = 0;
	RTO = 500;
	pd[seq].dnsHost = "";


}
void packetHelper::sendPacket( int seq )
{
	int ttl = seq + 1;
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
	if (sendto(sock, (char*) pd[seq].send_buf, sizeof(ICMPHeader), 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR)
	{
		printf(" send to error %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		exit(-1);

	}
}


void packetHelper::retransmitPackets() {
	for (int i = 0; i < N; i++)
	{
		if (pd[i].icmpComplete != true)
		{
			if (pd[i].giveUp == false)
			{
				if (pd[i].probe <= 3)
				{
					pd[i].giveUp = true;
					resendPacket(i);
				}
				pd[i].probe += 1;
				

			}
			else
			{
				printf(" empty  domain failed to reach edit me later in retransmist \n");
			}

		}
	}

}

std::string packetHelper::DNSlookup(std::string IP) {
	struct hostent* r;
	r = gethostbyaddr(IP.c_str(), sizeof(remote), AF_INET);
	std::string ipAddy(r->h_name);
	return ipAddy;
}


void packetHelper::recvPackets()
{

	fd_set readFds;
	timeval timeout;
	FD_ZERO(&readFds); // this sets the file descriptor 
	FD_SET(sock, &readFds); // assign a socket to a descriptor



	// set / RESET RTT HERE



	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
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
			if (bytes >= 56 && routerIcmpHead->type == ICMP_TTL_EXPIRED && routerIcmpHead->code == 0)
			{
				pd[originalIcmpHeader->seq].icmpComplete = true;


				// DNS RESPONSE HERE 
				// thread
				// gethostbyname
				std::string printME = "";
				u_long temp = (routerIpHeader->source_ip);
				u_char* IPArray = (u_char*)&temp;
				std::string IP((char*)IPArray);
				std::string dnsString = DNSlookup(IP);
				std::cout << " dns " << dnsString;
				pd[originalIcmpHeader->seq].RTT = (double)(pd[originalIcmpHeader->seq].startTimer - clock()) / CLOCKS_PER_SEC;
				// u_char* IPArray = (u_char*)&temp;
				printf(" (%d.%d.%d.%d)", IPArray[0], IPArray[1], IPArray[2], IPArray[3]);
				printf(" %3f", pd[originalIcmpHeader->seq].RTT);
				printf(" (%d)\n", pd[originalIcmpHeader->seq].probe);
				//	break;
			}
			else if (bytes >= 28 && routerIcmpHead->type == ICMP_ECHO_REPLY && routerIcmpHead->code == 0)
			{
				pd[originalIcmpHeader->seq].icmpComplete = true;
				u_long temp = (routerIpHeader->source_ip);
				// this reply is my orginal destination
				u_char* IPArray = (u_char*)&temp;
				std::string IP ((char*) IPArray);


				printf("hopNumber %d %d.%d.%d.%d \n", routerIcmpHead->seq + 1, IPArray[0], IPArray[1], IPArray[2], IPArray[3]);
				printf(" router type %d | router code %d \n", routerIcmpHead->type, routerIcmpHead->code);
				printf(" icmp seq %d | icmp id %d \n", htons(routerIcmpHead->seq), htons(routerIcmpHead->id));
				//		pk->pd[routerIcmpHead->seq].icmpComplete = true;

			}


	}
	}
	else if (ret == 0)
	{
		printf(" timed out \n");
	}
	else
	{
		printf(" no packets recieved \n");
	}
}

void packetHelper::handleError(int error)
{
	if (error == 0)
	{
		
		printf(" error timeout \n");
	}
}
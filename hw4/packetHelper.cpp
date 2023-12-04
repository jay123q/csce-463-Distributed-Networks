#pragma once
#include "pch.h"


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
}
packetHelper::~packetHelper() {

}
void packetHelper::createPacket(int seq , int ttl)
{

	memset(pd[seq].send_buf, 0, sizeof(MAX_ICMP_SIZE));
	pd[seq].icmpPacket.type = (u_char) ICMP_ECHO_REQUEST;
	pd[seq].icmpPacket.code = (u_char) 0;
	pd[seq].icmpPacket.id = (u_short) GetCurrentProcessId();
	pd[seq].icmpPacket.seq = (u_short) ttl;
	pd[seq].icmpPacket.checksum = (u_short) 0;
	pd[seq].icmpPacket.checksum = cc.ip_checksum((u_short*)pd[seq].send_buf, packet_size);
	memcpy(pd[seq].send_buf, &pd[seq].icmpPacket, sizeof(ICMPHeader));
	pd[seq].probe = 1;
	
	pd[seq].icmpComplete = false;
	pd[seq].dnsComplete = false;
	pd[seq].dnsIp = 0;
	pd[seq].RTO = 500;
	pd[seq].dnsHost = "";
	pd[seq].complete = CreateEvent(NULL, false, false, NULL);
	/*
	ICMPHeader* icmp = (ICMPHeader*)send_buf;
	icmp->type = ICMP_ECHO_REQUEST;
	icmp->code = 0;
	icmp->id = htons(GetCurrentProcessId());
	icmp->seq = htons(seq);
	icmp->checksum = 0;
	int packet_size = sizeof(IPHeader) + sizeof(ICMPHeader);
	checksum cc;
	icmp->checksum = cc.ip_checksum((u_short*)send_buf, packet_size);
	*/
}
void packetHelper::sendPacket(int seq, int ttl)
{

	if (setsockopt(sock, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
	{
		printf("setsockopt failed with %d\n", WSAGetLastError());
		closesocket(sock);
		// some cleanup
		exit(-1);
	}
	pd[seq].startTimer = clock();

	// we need to take to send the ICMP, and the IPheader part the server will take car of
	if (sendto(sock, pd[seq].send_buf , sizeof(ICMPHeader), 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR)
	{
		printf(" send to error %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		exit(-1);

	}

}
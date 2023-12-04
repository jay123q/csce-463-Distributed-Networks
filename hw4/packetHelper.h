#pragma once

#include "pch.h"

struct packetDetails {
	ICMPHeader icmpPacket;
	u_char send_buf[MAX_ICMP_SIZE];
	int probe;
	double startTimer;
	bool icmpComplete;
	DWORD dnsIp;
	std::string dnsHost;
	HANDLE complete;
};


class packetHelper {
public:
	double RTO;
	bool dnsComplete;
	HANDLE socketReceiveReady;
	struct sockaddr_in remote;
	packetDetails* pd;
	checksum cc;
	int packet_size;
	double RTT;
	SOCKET sock;
	packetHelper(DWORD IP, std::string host);
	~packetHelper();
	void createPacket( int seq);
	void sendPacket( int seq);
	void resendPacket(int seq);
	void retransmitPackets();
	void recvPackets();
};
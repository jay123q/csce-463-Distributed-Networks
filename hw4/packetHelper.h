#pragma once

#include "pch.h"

struct packetDetails {
	ICMPHeader icmpPacket;
	char send_buf[MAX_ICMP_SIZE];
	int probe;
	double startTimer;
	double RTO;
	bool icmpComplete;
	bool dnsComplete;
	DWORD dnsIp;
	std::string dnsHost;
	HANDLE complete;
};


class packetHelper {
public:
	HANDLE socketReceiveReady;
	struct sockaddr_in remote;
	packetDetails* pd;
	checksum cc;
	int packet_size;
	double RTT;
	SOCKET sock;
	packetHelper(DWORD IP, std::string host);
	~packetHelper();
	void createPacket(int seq);
	void sendPacket(int ttl);

};
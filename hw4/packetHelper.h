#pragma once
#include "pch.h"

struct packetDetails {
	ICMPHeader icmpPacket;
	u_char send_buf[MAX_ICMP_SIZE];
	int probe;
	double startTimer;
	bool icmpComplete;
	bool giveUp;
	DWORD dnsIp;
	std::string dnsHost;
	double RTT;
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
	SOCKET sock;
	// array size of 30  string, 
	std::string print[30];
	packetHelper(DWORD IP, std::string host);
	~packetHelper();
	void createPacket( int seq);
	void sendPacket( int seq);
	void resendPacket(int seq);
	void retransmitPackets();
	void recvPackets();
	std::string DNSlookup(std::string IP);
	void handleError(int error);
};
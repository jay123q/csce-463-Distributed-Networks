#pragma once
#include "pch.h"

struct packetDetails {
	ICMPHeader icmpPacket;
	u_char send_buf[MAX_ICMP_SIZE];
	int probe;
	double startTimer;
	bool icmpComplete;
	std::string printString;
	double RTT;
};


class packetHelper {
public:
	bool firstIteration;
	HANDLE socketReceiveReady;
	struct sockaddr_in remote;
	packetDetails* pd;
	checksum cc;
	int packet_size;
	SOCKET sock;

	packetHelper(DWORD IP, std::string host);
	~packetHelper();
	void createPacket( int seq);
	void sendPacket( int seq);
	void resendPacket(int seq);
	void retransmitPackets();
	bool checkComplete();
	void recvPackets();
	void finalPrint();
	std::string DNSlookup(std::string IP);
	void handleError(int error);
	double setRTO();
};
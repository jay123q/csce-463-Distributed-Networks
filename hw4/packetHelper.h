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
	packetDetails pd [N + 1];
	checksum cc;
	int countSeq;
	int packet_size;
	DWORD storeIP;
	std::string IPforlastPrint;
	std::string printLast;
	std::set<u_long> unique_ip; // only used for report
	double RTTlast;
	SOCKET sock;
	bool errorBreak;
	packetHelper(std::string host);
	~packetHelper();
	void createPacket( int seq);
	void sendPacket( int seq);
	void resendPacket(int seq);
	void retransmitPackets();
	bool checkComplete();
	void recvPackets();
	void finalPrint();
	std::string DNSlookup(std::string IP);
	bool handleError(int type, int code);
	double setRTO();
};
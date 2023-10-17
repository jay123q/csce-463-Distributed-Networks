#pragma once
#include "pch.h"
#include <ctype.h> 
#include <stdio.h> 
#include <windows.h>
#include <string>
#include <iostream>
using namespace std;


#pragma pack(push,1) // restores old packing
class LinkProperties {
public:
    // transfer parameters
    float RTT; // propagation RTT (in sec)
    float speed; // bottleneck bandwidth (in bits/sec)
    float pLoss[2]; // probability of loss in each direction
    DWORD bufferSize; // buffer size of emulated routers (in packets)
    LinkProperties() { memset(this, 0, sizeof(*this)); }
};

#define MAGIC_PROTOCOL 0x8311AA
class Flags {
public:
    DWORD reserved : 5; // must be zero
    DWORD SYN : 1;
    DWORD ACK : 1;
    DWORD FIN : 1;
    DWORD magic : 24;
    Flags() { memset(this, 0, sizeof(*this)); magic = MAGIC_PROTOCOL; }
};

class SenderDataHeader {
public:
    Flags flags;
    DWORD seq; // must begin from 0
};
class ReceiverHeader {
public:
    Flags flags;
    DWORD recvWnd; // receiver window for flow control (in pkts)
    DWORD ackSeq; // ack value = next expected sequence
};
class SenderSynHeader {
public:
    SenderDataHeader sdh;
    LinkProperties lp;
};

#pragma pack(pop,1) // restores old packing


class SenderSocket {
    struct sockaddr_in local;
    struct hostent* remote;
    struct sockaddr_in server;
    int bytes;
    LinkProperties* lp;
    string host;
    DWORD IP;

public:
    bool opened;
    double RTT;
    SenderSynHeader* packet;
    SOCKET sock;
    clock_t time;
    SenderSocket();
    DWORD Open(string host, int portNumber, int senderWindow, LinkProperties* lp);
    DWORD Send(char* pointer, UINT64 bytes );
    DWORD recvFrom(long RTOsec, long RTOusec, bool inOpen);
    DWORD Close();

};
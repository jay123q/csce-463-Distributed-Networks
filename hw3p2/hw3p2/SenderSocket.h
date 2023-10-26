#pragma once
#include "pch.h"
#include "Checksum.h"
#include <ctype.h> 
#include <stdio.h> 
#include <windows.h>
#include <string>
#include <iostream>
using namespace std;

#define MAGIC_PROTOCOL 0x8311AA
#define MAX_PKT_SIZE (1500-28) // maximum UDP packet size accepted by receiver 



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




struct statsThread {
    HANDLE statusEvent;

    clock_t startTimerStats; // in open
    clock_t prevTimerStats; // in stats
    DWORD packetsToSendStats;
    double bytesAckedStats;
    DWORD nextSeqNumStats; // in recv
    int timeoutCountStats; // in rcv
    int fastRetransmitCountStats;
    int effectiveWindowStats; // min btwn sndWin and rcvWin
    DWORD rcvWinStats;
    DWORD sndWinStats;
    double goodPutStats; // speed reciever processes data from app
    double estimateRttStats;

};

class Checksum;
class SenderSocket {
    struct sockaddr_in local;
    struct hostent* remote;
    struct sockaddr_in server;
    int bytes;
    LinkProperties* lp;
    string host;
    DWORD IP;

public:

    statsThread st;

    // handle finding RTT
    double setRTO;
    double estimateRTT;
    double deviationRTT;

    // handle opening an closing
    int bytesRec;
    bool opened;
    double sampleRTT;
    SOCKET sock;
    clock_t time;
    double closeCalledTime;
    clock_t timeToAckforSampleRTT;
    SenderSynHeader * packetSyn;
    SenderSynHeader * packetFin;

    // all check sum params
    Checksum checkValidity;
    DWORD checkSum;
    char* checkSumCharBuffer;
    int totalBytesTransferCheckSum;

    SenderSocket();
    DWORD Open(string host, int portNumber, int senderWindow, LinkProperties* lp);
    DWORD Send(char* pointer, UINT64 bytes );
    DWORD recvFrom(long RTOsec, long RTOusec, bool inOpen);
    DWORD Close();
    DWORD statusThread();

    void setEstimateRTT();
    void setDeviationRTT();
    void findRTO();

};
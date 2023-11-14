#pragma once
#include "pch.h"
#include <stdio.h>
#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include "Checksum.h"
#include <ctype.h> 
#include <string>
#include <synchapi.h>
#include <iostream>
using namespace std;
#include <queue>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAGIC_PROTOCOL 0x8311AA
#define MAX_PKT_SIZE (1500-28) // maximum UDP packet size accepted by receiver 
#pragma warning(disable:4996) 


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

class Packet {
public:
    int type; // SYN, FIN, data
    int size; // bytes in packet data
    clock_t txTime; // transmission time
    char packetContents[MAX_PKT_SIZE]; // packet with header
    SenderDataHeader sdh;
};

#pragma pack(pop,1) // restores old packing




struct statsThread {
    HANDLE statusEvent;

    clock_t startTimerStats; // in open
    DWORD packetsToSendStats; 
    DWORD packetsSendBaseStats;
    int timeoutCountStats; // in rcv
    int fastRetransmitCountStats;
    int effectiveWindowStats; // min btwn sndWin and rcvWin
    DWORD rcvWinStats; // set in from RCV pick snd or rcv
    DWORD sndWinStats; // set in from open pick snd or rcv
    double estimateRttStats;
    int timerPrint2sec;
    int packetesSendBaseNextPacketStats;

    clock_t prevTimerStats; // in stats good put?
    double goodPutStats; // speed reciever processes data from app
    DWORD prevPrintBase;


    double bytesTotal;
    int nNumberPrints;
    bool breakThread; // used in closae 

};

class Checksum;
class SenderSocket {
    struct sockaddr_in local;
    struct hostent* remote;
    int bytes;
    LinkProperties* lp;
    string host;
    DWORD IP;

public:
    struct sockaddr_in server;
    statsThread st;
    Packet * packetsSharedQueue;


    // handle finding RTT
    double setRTO;
    double estimateRTT;
    double deviationRTT;
    int packetsSendBase;

    // handle opening an closing
    int dupAck;
    bool opened;
    double sampleRTT;
    int recvBufferLast;
    SOCKET sock;
    clock_t time;
    double closeCalledTime;
    clock_t timeToAckforSampleRTT;
    SenderSynHeader * packetSyn;
    SenderSynHeader * packetFin;
    DWORD senderWindow;
    clock_t timeAtClose;
    bool closeCalled;

    // all check sum params
    Checksum checkValidity;
    char* sendBufCheckSum;
    int packetSizeSend;
    DWORD hexDumpPost;

    float pLossForwardFin;
    float pLossBackwardFin;
    float RTTFin;
    float speedFin;

    // sephamore variables
    HANDLE empty;
    HANDLE full;
    
    // handlers
    HANDLE workers;
    HANDLE stats;

    HANDLE closeConnection;
    HANDLE complete;
    HANDLE socketReceiveReady;

    SenderSocket();
    ~SenderSocket();
    DWORD Open(string host, int portNumber, int senderWindow, LinkProperties* lp);
    // DWORD Send(char* pointer, UINT64 bytes );
    int Send(char* data, int size);

    // DWORD recvFrom(long RTOsec, long RTOusec);
    DWORD Close();

    static DWORD WINAPI workerThreads(LPVOID tempPointer);
    static DWORD WINAPI runStats(LPVOID tempPointer);

    DWORD ReceiveACK( int * dupCount );

    void setEstimateRTT();
    void setDeviationRTT();
    void findRTO();

};
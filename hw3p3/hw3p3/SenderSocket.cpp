#include "pch.h"
#include "SenderSocket.h"
#include <ctype.h> 
#include <stdio.h> 
#include <windows.h>
#include <string>

#include <iostream>
using namespace std;

#define STATUS_OK 0 // no error
#define ALREADY_CONNECTED 1 // second call to ss.Open() without closing connection
#define NOT_CONNECTED 2 // call to ss.Send()/Close() without ss.Open()
#define INVALID_NAME 3 // ss.Open() with targetHost that has no DNS entry
#define FAILED_SEND 4 // sendto() failed in kernel
#define TIMEOUT 5 // timeout after all retx attempts are exhausted
#define FAILED_RECV 6 // recvfrom() failed in kernel
#define RETX_OCCURED 10 // recvfrom() failed in kernel

#define FORWARD_PATH 0
#define RETURN_PATH 1
#define MAGIC_PROTOCOL 0x8311AA

#define MAGIC_PORT 22345 // receiver listens on this port
#define MAX_PKT_SIZE (1500-28) // maximum UDP packet size accepted by receiver 

#define REATTEMPTS_ALLOWED 50

class LinkProperties;
class Flags;
class SenderDataHeader;
class ReceiverHeader;
class SenderSynHeader;
class SenderSocket;
class Packet;
struct statsThread;
#pragma comment(lib, "ws2_32.lib")


DWORD WINAPI SenderSocket::statusThread(LPVOID * tempPointer)
{
    SenderSocket* stats = (SenderSocket*)tempPointer;
    bool printMe = false;
    
    stats->st.prevPrintBase = stats->st.packetsSendBaseStats;
    while (((WaitForSingleObject(stats->st.statusEvent, 2000) == WAIT_TIMEOUT) || !printMe) && stats->opened )
    {
        if (stats->st.breakThread == true)
        {
            // avoid final erronous rpint
            return 20;
        }
        stats->st.effectiveWindowStats = min(stats->st.rcvWinStats , stats->st.sndWinStats);
        
        
        
            // help here TA ASK
        stats->st.goodPutStats = (stats->st.packetsSendBaseStats - stats->st.prevPrintBase ) / stats->st.nNumberPrints * 8 * (MAX_PKT_SIZE - sizeof(SenderDataHeader));
        
        
        
        printMe = true;
        printf("[ %1d] B %d ( %.2f MB) N %d T %d F %d W %d S %.3f Mbps RTT %.3f\n",
            // (int)(double)(st.prevTimerStats - st.startTimerStats) / CLOCKS_PER_SEC,
            stats->st.timerPrint2sec,
            stats->st.packetsSendBaseStats,
            stats->st.bytesTotal / 8e6,
            stats->st.packetsSendBaseStats,
            stats->st.timeoutCountStats,
            stats->st.fastRetransmitCountStats,
            stats->st.effectiveWindowStats,
            stats->st.goodPutStats / 2e6,
            stats->st.estimateRttStats
        );
        stats->st.timerPrint2sec += 2;
        stats->st.nNumberPrints++;
        // st.prevPrintBase = st.packetsSendBase;
       // st.prevTimerStats = st.startTimerStats;
       // st.startTimerStats = clock();
        ResetEvent(stats->st.statusEvent);
    }
    return 20;
}

SenderSocket::SenderSocket()
{
    st.statusEvent = CreateEvent(NULL, true, false, NULL);
    st.breakThread = false;


    this->time = clock();
    memset(&remote, 0, sizeof(remote));
    memset(&server, 0, sizeof(server));

    empty = CreateEvent(NULL, false, false, NULL);
    receive = CreateEvent(NULL, false, false, NULL);
    closeConnection = CreateEvent(NULL, true, false, NULL);
    full = CreateSemaphore(NULL, 0, this->senderWindow, NULL);

}
DWORD SenderSocket::Open(string host, int portNumber, int senderWindow, LinkProperties* lp) {
    if (opened)
    {
        return ALREADY_CONNECTED;
    }

    packetsSharedQueue = new Packet[senderWindow];




    // stats thread initialization
    st.startTimerStats = clock();
    st.timerPrint2sec = 2;
    st.packetsSendBaseStats = 0;
    st.bytesTotal = 0;
    st.timeoutCountStats = 0;
    st.fastRetransmitCountStats = 0;
    st.effectiveWindowStats = 0;
    st.rcvWinStats = 0;
    st.sndWinStats = 0;
    st.goodPutStats = 0;
    st.estimateRttStats = 0;

    st.nNumberPrints = 1;
    st.prevTimerStats = 0.0;
    dupAck = 0;

    this->senderWindow = senderWindow;

    st.prevPrintBase = st.packetsSendBaseStats;



    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        WSACleanup();
        return -1;
    }

    this->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sock == INVALID_SOCKET) {
        printf(" [%.3f] --> target %s is invalid\n",
            (double) (clock() - time) / CLOCKS_PER_SEC,
            host.c_str()
        );
        printf(" invalid socket erorr \n ");
        closesocket(sock);
        WSACleanup();
        return GetLastError();
    }

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;

    if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
        printf(" [%.3f] --> target %s is invalid\n",
            (double) (clock() - time) / CLOCKS_PER_SEC,
            host.c_str()
        );
        closesocket(sock);
        WSACleanup();
        printf(" invalid bind erorr \n ");
        return GetLastError();
    }


    this->packetSyn = new SenderSynHeader();
    memset(packetSyn, 0, sizeof(SenderSynHeader));
    packetSyn->lp.bufferSize = senderWindow + 3 ; // window size is 10 with retransmit it 3x
    packetSyn->lp.pLoss[0] = lp->pLoss[0];
    packetSyn->lp.pLoss[1] = lp->pLoss[1];
    packetSyn->lp.RTT = lp->RTT;
    packetSyn->lp.speed = lp->speed;

    packetSyn->sdh.flags.reserved = 0;
    packetSyn->sdh.flags.magic = MAGIC_PROTOCOL;
    packetSyn->sdh.flags.SYN = 1;
    packetSyn->sdh.flags.FIN = 0;
    packetSyn->sdh.flags.ACK = 0;
    packetSyn->sdh.seq = 0;


    pLossForwardFin = lp->pLoss[0];
    pLossBackwardFin = lp->pLoss[1];
    RTTFin = lp->RTT;
    speedFin = lp->speed;

    st.sndWinStats = senderWindow;

    this->host = host.c_str();
    DWORD IP = inet_addr(host.c_str());
    this->IP = IP;
    if (IP == INADDR_NONE)
    {
        if ((this->remote = gethostbyname(host.c_str())) == NULL)
        {
            printf(" [%.3f] --> target %s is invalid\n",
                (double)(clock() - time) / CLOCKS_PER_SEC,
                host.c_str()
            );
            closesocket(sock);
            WSACleanup();
            return INVALID_NAME;
        }
        else // take the first IP address and copy into sin_addr
        {
            memcpy((char*)&(this->server.sin_addr), this->remote->h_addr, this->remote->h_length);
        }
    }
    else
    {
        this->server.sin_addr.S_un.S_addr = IP;
    }


    server.sin_family = AF_INET;
    server.sin_port = htons(portNumber); // DNS port on server
    long RTOsec = 1.0000 + 2 * lp->RTT;
    long RTOusec = 0;
    DWORD recvReturn;
    this->timeToAckforSampleRTT = clock();
    for (int i = 1; i < REATTEMPTS_ALLOWED ; i++)
    {
        // you saw cc cc cc cc showing that the packet waas on the heap due to & being used. 
       // printf(" [%.3f] --> SYN 0 (attempt %d of 3, RTO %.3f) to %s\n",

        if (sendto(sock, (char*) packetSyn, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            
            printf(" [%.3f] --> failed sendto with %d\n",
                (double)(clock() - time) / CLOCKS_PER_SEC,
                WSAGetLastError());
            
            closesocket(sock);
            WSACleanup();
            return FAILED_SEND;

        }
        
        timeval timeout;
        timeout.tv_sec = RTOsec;
        timeout.tv_usec = RTOusec;
        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        int available = select(0, &fd, NULL, NULL, &timeout);
        if (available <= 0)
        {
            // restart clock
            timeToAckforSampleRTT = clock();
            st.timeoutCountStats++;
            return TIMEOUT;
        }
        else
        {




            ReceiverHeader rh;
            struct sockaddr_in response;
            int responseSize = sizeof(response);

            int bytes = recvfrom(sock, (char*)&rh, sizeof(ReceiverHeader), 0, (struct sockaddr*)&response, &responseSize);
            if (bytes == SOCKET_ERROR)
            {
                printf("failed recvfrom with %d\n",
                    WSAGetLastError());
                closesocket(sock);
                WSACleanup();
                return FAILED_RECV;
                // return GetLastError();
            }

            this->sampleRTT = (double)(clock() - timeToAckforSampleRTT) / CLOCKS_PER_SEC;
            // findRTO();


            st.estimateRttStats = sampleRTT;
            estimateRTT = st.estimateRttStats;
            setRTO = st.estimateRttStats * 3;
        }
       
        recvBufferLast = min(senderWindow, rh.recvWnd);
        ReleaseSemaphore(empty, recvBufferLast, NULL);

        // WSAEventSelect(this->sock, this->receive, FD_READ);

        workers = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Worker, this, 0, NULL);
        
    }

    opened = true;
    return recvReturn; 
}

DWORD SenderSocket::recvFrom(long RTOsec, long RTOusec)
{
        
        timeval timeout;
        timeout.tv_sec = RTOsec;
        timeout.tv_usec = RTOusec;
        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        int available = select(0, &fd, NULL, NULL, &timeout);
        if (available <= 0)
        {
            // restart clock
            timeToAckforSampleRTT = clock();
            st.timeoutCountStats++;
            return TIMEOUT;
        }
        else
        {
            // (available > 0)
            ReceiverHeader rh;
            struct sockaddr_in response;
            int responseSize = sizeof(response);

            int bytes = recvfrom(sock, (char*)&rh, sizeof(ReceiverHeader), 0, (struct sockaddr*)&response, &responseSize);
            if (bytes == SOCKET_ERROR)
            {
                printf("failed recvfrom with %d\n",
                    WSAGetLastError());
                closesocket(sock);
                WSACleanup();
                return FAILED_RECV;
                // return GetLastError();
            }

            this->sampleRTT = (double)(clock() - timeToAckforSampleRTT) / CLOCKS_PER_SEC;
            findRTO();

            // in close, handle snd && recieve more pkts, adjust buffer
            // n == ackSeq, adjust, timer, move window, inc seq
            // else buf in recvwin

            if (rh.flags.FIN != 0 && rh.flags.SYN != 0)
            {
                // do nuthing?? , hold seq numebr
            }
            else if (rh.ackSeq >= st.packetsSendBaseStats + 1)
            {

                dupAck = 0;

                // potential crit section
                // stats theard packetsToSend
                if (rh.ackSeq == st.packetsSendBaseStats + 1)
                {

                    st.rcvWinStats = rh.recvWnd;
                    st.bytesTotal += MAX_PKT_SIZE;
                    st.packetsSendBaseStats++;
                }
                else
                {
                    // out of order packets??
                    timeToAckforSampleRTT = clock();
                    printf(" pakcets out of order \n");
                }



            }
            else if (rh.ackSeq == st.packetsSendBaseStats)
            {
                dupAck++;
                st.packetsSendBaseStats = rh.ackSeq;
                if (dupAck == 3)
                {
                    st.fastRetransmitCountStats++;
                    return RETX_OCCURED;
                }
            }
            else
            {
                printf(" I am concerned how you got here bud, recv acked a PKT < than snd \n");
            }




        }




    return STATUS_OK;
}

    int SenderSocket::Send(char* data, int size)
    {
        HANDLE arr[] = { closeConnection, empty };
        WaitForMultipleObjects(2, arr, false, INFINITE);
        // old this->timeToAckforSampleRTT = clock();
        
        // no need for mutex as no shared variables are modified
        int slot = st.packetsSendBaseStats % st.sndWinStats;
        Packet* p = packetsSharedQueue + slot; // pointer to packet struct
        // SenderDataHeader* sdh = p->pkt;
        p->sdh.seq = st.packetsSendBaseStats;
        p->sdh.flags.reserved = 0;
        p->sdh.flags.magic = MAGIC_PROTOCOL;
        p->sdh.flags.ACK = 0;
        p->sdh.flags.SYN = 0;
        p->sdh.flags.FIN = 0;
        p->sdh.seq = st.packetsSendBaseStats;
        p->size = size + sizeof(SenderDataHeader);
        p->txTime = clock();
        memcpy(p->packetsPending , data, size);
        st.packetsSendBaseStats++;
        ReleaseSemaphore(full, 1);
    }
/*
    this->timeToAckforSampleRTT = clock();
    SenderDataHeader * packet = new SenderDataHeader();
    packet->flags.reserved = 0;
    packet->flags.magic = MAGIC_PROTOCOL;
    packet->flags.SYN = 0;
    packet->flags.FIN = 0;
    packet->flags.ACK = 0;
    packet->seq = st.packetsSendBase;

    // potential crit section
    // stats theard packetsToSend
    // memcpy(packet, pointer, bytes);

    DWORD recvReturn;
    int RTOsec = floor(3 * this->sampleRTT);
    int RTOusec = (3 * this->sampleRTT - RTOsec)*1e6;
    for (int i = 1; i < REATTEMPTS_ALLOWED ; i++)
    {
        // you saw cc cc cc cc showing that the packet waas on the heap due to & being used. 
       // printf(" [%.3f] --> SYN 0 (attempt %d of 3, RTO %.3f) to %s\n",

        if (sendto(sock, (char*)packet, sizeof(*packet), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            
            printf(" [%.3f] --> failed sendto with %d\n",
                (double)(clock() - time) / CLOCKS_PER_SEC,
                WSAGetLastError());
            closesocket(sock);
            WSACleanup();



            return FAILED_SEND;

        }
        
        
        // estimate RTT HERE


        recvReturn = recvFrom(RTOsec, RTOusec, false);

        if (recvReturn == STATUS_OK || recvReturn == FAILED_RECV)
        {
            break;
        }
    }


    return recvReturn;
*/

DWORD SenderSocket::Close() {
    this->timeToAckforSampleRTT = clock();

    this->packetFin = new SenderSynHeader();
    memset(packetFin, 0, sizeof(SenderSynHeader));


    packetFin->lp.bufferSize = this->senderWindow + 3; // window size is 10 with retransmit it 3x
    packetFin->lp.pLoss[0] = pLossForwardFin;
    packetFin->lp.pLoss[1] = pLossBackwardFin;
    packetFin->lp.RTT = sampleRTT;
    packetFin->lp.speed = speedFin;
    packetFin->sdh.flags.reserved = 0;
    packetFin->sdh.flags.magic = MAGIC_PROTOCOL;
    packetFin->sdh.flags.SYN = 0;
    packetFin->sdh.flags.FIN = 1;
    packetFin->sdh.flags.ACK = 0;
    packetFin->sdh.seq = st.packetsSendBaseStats;


    int RTOsec = floor(3 * this->sampleRTT);
    int RTOusec = (3 * this->sampleRTT - RTOsec) * 1e6;
    /*
    int RTOsec = 1;
    int RTOusec = 0;
    */


    this->closeCalledTime = clock();
    int count = 0;
    DWORD recvReturn;
    while (count < 5)
    {
        count++;
        if (sendto(sock, (char*) packetFin, sizeof(*packetFin), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf(" [%.3f] --> failed sendto with %d\n",
                (double)(clock() - time) / CLOCKS_PER_SEC,
                WSAGetLastError());

            closesocket(sock);
            WSACleanup();
            return FAILED_SEND;
        }
        
        timeval timeout;
        timeout.tv_sec = RTOsec;
        timeout.tv_usec = RTOusec;
        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        int available = select(0, &fd, NULL, NULL, &timeout);
        if (available <= 0)
        {
            // restart clock
            timeToAckforSampleRTT = clock();
            st.timeoutCountStats++;
            return TIMEOUT;
        }
        else
        {
            ReceiverHeader rh;
            struct sockaddr_in response;
            int responseSize = sizeof(response);

            int bytes = recvfrom(sock, (char*)&rh, sizeof(ReceiverHeader), 0, (struct sockaddr*)&response, &responseSize);
            if (bytes == SOCKET_ERROR)
            {
                printf("failed recvfrom with %d\n",
                    WSAGetLastError());
                closesocket(sock);
                WSACleanup();
                return FAILED_RECV;
                // return GetLastError();
            }

            this->sampleRTT = (double)(clock() - timeToAckforSampleRTT) / CLOCKS_PER_SEC;
            findRTO();

        }
        printf(" finsihed and closing \n");
        WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        
        
        
        /*
        recvReturn = recvFrom(RTOsec, RTOusec, false);
        if (recvReturn == STATUS_OK || recvReturn == FAILED_RECV)
        {
            timeAtClose = clock();
            break;
        }
        */
    }
    
    
    // chcksum here
    this->hexDumpPost = checkValidity.CRC32( (unsigned char *) this->sendBufCheckSum, this->packetSizeSend );





    return recvReturn; // happy
}

DWORD WINAPI Worker(LPVOID * tempPointer )
{
    SenderSocket* worker = (SenderSocket*)tempPointer;
    int kernelBuffer = 20e6; // 20 meg
    if (setsockopt(worker->sock, SOL_SOCKET, SO_RCVBUF, (char *) & kernelBuffer, sizeof(int)) == SOCKET_ERROR)
    {
        return 0;
    }


    kernelBuffer = 20e6; // 20 meg
    if (setsockopt(worker->sock, SOL_SOCKET, SO_SNDBUF, (char *) & kernelBuffer, sizeof(int)) == SOCKET_ERROR)
    {
        return 0;
    }
    
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);


    HANDLE events[] = { worker->socketReceiveReady, worker->full , worker->empty };
    while (true)
    {
        double timeout = 0;
        if (worker->packetsSharedQueue)
        {
            timeout = timerExpire - cur_time;
        }
        else
        {
            timeout = INFINITE;
        }
        int ret = WaitForMultipleObjects(2, events, false, timeout);
        switch (ret)
        {
        case WAIT_OBJECT_0:
            sendto(worker->sock,
                (char*)&worker->packetsSharedQueue[worker->st.packetsSendBaseStats % worker->st.effectiveWindowStats].sdh,
                worker->packetsSharedQueue[worker->st.packetsSendBaseStats % worker->st.effectiveWindowStats].size,
                0,
                (struct sockaddr*)&worker->server, sizeof(worker->server)) == SOCKET_ERROR
                );
                break;
        case WAIT_OBJECT_0+1: // move senderBase; update RTT; handle fast retx; do flow control
            // ReceiveACK();
            int RTOsec = floor(3 * worker->sampleRTT);
            int RTOusec = (3 * worker->sampleRTT - RTOsec) * 1e6;
            recvfrom(RTOsec, RTOusec)
            break;
        case WAIT_OBJECT_0+2:
            sendto(worker->sock,
                (char*)&worker->packetsSharedQueue[worker->st.packetsSendBaseStats % worker->st.effectiveWindowStats].sdh,
                worker->packetsSharedQueue[worker->st.packetsSendBaseStats % worker->st.effectiveWindowStats].size,
                0,
                (struct sockaddr*)&worker->server, sizeof(worker->server)) == SOCKET_ERROR
                );
                packetsSendBaseStats++;
                break;
        default: handle failed wait;
        }
        if (first packet of window || just did a retx(timeout / 3 - dup ACK)
            || senderBase moved forward)
        {
            recompute timerExpire;
        }


    }
    return STATUS_OK;
}



void SenderSocket::setDeviationRTT() {
    double b = 0.25;
    this->deviationRTT = (1 - b) * this->deviationRTT + b * abs(this->sampleRTT - this->estimateRTT);
}
void SenderSocket::setEstimateRTT() {
    double a = 0.125;
    this->estimateRTT = (1 - a) * this->estimateRTT + a * this->sampleRTT;
    st.estimateRttStats = this->estimateRTT;
    // perhaps merg into one
}
void SenderSocket::findRTO() {
    setDeviationRTT();
    setEstimateRTT();
    this->setRTO = this->estimateRTT + 4 * max(this->deviationRTT, 0.01);
}
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

#define FORWARD_PATH 0
#define RETURN_PATH 1
#define MAGIC_PROTOCOL 0x8311AA

#define MAGIC_PORT 22345 // receiver listens on this port
#define MAX_PKT_SIZE (1500-28) // maximum UDP packet size accepted by receiver 

class LinkProperties;
class Flags;
class SenderDataHeader;
class ReceiverHeader;
class SenderSynHeader;
class SenderSocket;
struct statsThread;
#pragma comment(lib, "ws2_32.lib")

SenderSocket::SenderSocket()
{
    st.statusEvent = CreateEvent(NULL, true, false, NULL);
    st.breakThread = false;


    this->time = clock();
    memset(&remote, 0, sizeof(remote));
    memset(&server, 0, sizeof(server));


}

DWORD SenderSocket::Open(string host, int portNumber, int senderWindow, LinkProperties* lp) {
    if (opened)
    {
        return ALREADY_CONNECTED;
    }

    // stats thread initialization
    st.startTimerStats = clock();
    st.bytesAckedStats = 0;
    st.packetsToSendStats = 0;
    st.bytesAckedStats = 0;
    st.nextSeqNumStats = 0;
    st.timeoutCountStats = 0;
    st.fastRetransmitCountStats = 0;
    st.effectiveWindowStats = 0;
    st.rcvWinStats = 0;
    st.sndWinStats = 0;
    st.goodPutStats = 0;
    st.estimateRttStats = 0;

    this->senderWindow = senderWindow;


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
    long RTOsec = 1.0000;
    long RTOusec = 0;
    DWORD recvReturn;
    this->timeToAckforSampleRTT = clock();
    for (int i = 1; i < 4; i++)
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

        recvReturn = recvFrom(RTOsec + 2*lp->RTT, RTOusec, true);
        if (recvReturn  == STATUS_OK || recvReturn == FAILED_RECV )
        {

            break;
        }
    }


    this->sampleRTT = (double)(clock() - this->timeToAckforSampleRTT) / CLOCKS_PER_SEC;
    return recvReturn; 
}

DWORD SenderSocket::recvFrom(long RTOsec, long RTOusec, bool inOpen)
{
    /*
        char* recvBuffer = new char[sizeof(ReceiverHeader)];
        memset(recvBuffer, 0, sizeof(ReceiverHeader));
    */
    


        timeval timeout;
        timeout.tv_sec = RTOsec;
        timeout.tv_usec = RTOusec;
        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        int available = select(0, &fd, NULL, NULL, &timeout);
        if (available <= 0)
        {
            // potential crit section
            // stats theard packetsToSend
            st.timeoutCountStats++;
            return TIMEOUT;
        }
        else
        {
            // (available > 0)
            ReceiverHeader rh;
            struct sockaddr_in response;
            int responseSize = sizeof(response);

            int bytes = recvfrom(sock, (char *) &rh, sizeof(ReceiverHeader), 0, (struct sockaddr*)&response, &responseSize);
            if (bytes == SOCKET_ERROR)
            {
                printf("failed recvfrom with %d\n",
                    WSAGetLastError());
                closesocket(sock);
                WSACleanup();
                return FAILED_RECV;
                // return GetLastError();
            }
            

            // packet had to be acked here
            this->sampleRTT = (double)(clock() - timeToAckforSampleRTT) / CLOCKS_PER_SEC;
            setEstimateRTT();
            setDeviationRTT();
            findRTO();
            if (inOpen)
            {

                return STATUS_OK;
            }

            if (!inOpen)
            {

                // in close, handle snd && recieve more pkts, adjust buffer
                // n == ackSeq, adjust, timer, move window, inc seq
                // else buf in recvwin
                if (rh.ackSeq == st.nextSeqNumStats+1)
                {
                    st.nextSeqNumStats++;
                    st.rcvWinStats = rh.recvWnd;
                    st.bytesAckedStats += MAX_PKT_SIZE;
                    return STATUS_OK;


                }
                // adjust RTT, RTO, ETC


            }

            return -100;

        }




    return STATUS_OK;
}

DWORD SenderSocket::Send(char* pointer, UINT64 bytes ) {
    this->timeToAckforSampleRTT = clock();
    SenderDataHeader * packet = new SenderDataHeader();
    packet->flags.reserved = 0;
    packet->flags.magic = MAGIC_PROTOCOL;
    packet->flags.SYN = 0;
    packet->flags.FIN = 0;
    packet->flags.ACK = 0;
    packet->seq = st.nextSeqNumStats;

    // potential crit section
    // stats theard packetsToSend
    // memcpy(packet, pointer, bytes);

    DWORD recvReturn;
    int RTOsec = floor(3 * this->sampleRTT);
    int RTOusec = (3 * this->sampleRTT - RTOsec)*1e6;
    for (int i = 1; i < 4; i++)
    {
        // you saw cc cc cc cc showing that the packet waas on the heap due to & being used. 
       // printf(" [%.3f] --> SYN 0 (attempt %d of 3, RTO %.3f) to %s\n",

        if (sendto(sock, (char*)packet, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
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
}
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
    packetFin->sdh.seq = st.nextSeqNumStats;


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
        if (sendto(sock, (char*) packetFin, sizeof(SenderDataHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf(" [%.3f] --> failed sendto with %d\n",
                (double)(clock() - time) / CLOCKS_PER_SEC,
                WSAGetLastError());

            closesocket(sock);
            WSACleanup();
            return FAILED_SEND;
        }

        recvReturn = recvFrom(RTOsec, RTOusec, false);
        if (recvReturn == STATUS_OK || recvReturn == FAILED_RECV)
        {
            timeAtClose = clock();
            break;
        }
    }
    
    
    // chcksum here
    this->hexDumpPost = checkValidity.CRC32( (unsigned char *) this->sendBufCheckSum, this->packetSizeSend );





    return recvReturn; // happy
}



DWORD SenderSocket::statusThread()
{
    bool printMe = false;
    st.prevTimerStats = clock();
    while ((WaitForSingleObject(st.statusEvent, 2000) == WAIT_TIMEOUT) || !printMe)
    {
        st.effectiveWindowStats = st.rcvWinStats - st.sndWinStats;
        printMe = true;
        printf("[ %d] B %d ( %.2f MB) N %d T %d F %d W %d S %.4f Mbps RTT %.4f\n",
            (int)(double)(st.prevTimerStats - st.startTimerStats) / CLOCKS_PER_SEC,
            st.packetsToSendStats,
            st.bytesAckedStats,
            st.nextSeqNumStats,
            st.timeoutCountStats,
            st.fastRetransmitCountStats,
            st.effectiveWindowStats,
            st.goodPutStats,
            st.estimateRttStats
        );

        ResetEvent(st.statusEvent);
        if (st.breakThread == true)
        {
            return 20;
        }
    }
    return 20;
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
    this->setRTO = 4 * max(this->deviationRTT, 0.01);
}
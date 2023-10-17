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

SenderSocket::SenderSocket()
{
    this->time = clock();
    memset(&remote, 0, sizeof(remote));
    memset(&server, 0, sizeof(server));
}

DWORD SenderSocket::Open(string host, int portNumber, int senderWindow, LinkProperties* lp) {
    if (opened)
    {
        return ALREADY_CONNECTED;
    }
    struct sockaddr_in remote;
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        WSACleanup();
        return -1;
    }

    this->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sock == INVALID_SOCKET) {
        printf(" [%0.3f] --> target %s is invalid\n",
            (clock() - time) / CLOCKS_PER_SEC,
            host
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
        printf(" [%0.3f] --> target %s is invalid\n",
            (clock() - time) / CLOCKS_PER_SEC,
            host
        );
        closesocket(sock);
        WSACleanup();
        printf(" invalid bind erorr \n ");
        return GetLastError();
    }


    this->packet = new SenderSynHeader();
    packet->lp.bufferSize = senderWindow + 3 ; // window size is 10 with retransmit it 3x
    packet->lp.pLoss[0] = lp->pLoss[0];
    packet->lp.pLoss[1] = lp->pLoss[1];
    packet->lp.RTT = lp->RTT;
    packet->lp.speed = lp->speed;

    packet->sdh.flags.reserved = 0;
    packet->sdh.flags.magic = MAGIC_PROTOCOL;
    packet->sdh.flags.SYN = 1;
    packet->sdh.flags.FIN = 0;
    packet->sdh.flags.ACK = 0;
    packet->sdh.seq = 0;

    this->host = host.c_str();
    DWORD IP = inet_addr(host.c_str());
    this->IP = IP;
    if (IP == INADDR_NONE)
    {
        if ((this->remote = gethostbyname(host.c_str())) == NULL)
        {
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
   // server.sin_addr.S_un.S_addr = inet_addr(host.c_str()); // server’s IP
    server.sin_port = htons(portNumber); // DNS port on server
    // server.sin_port = portNumber; // DNS port on server
    long RTOsec = 1;
    long RTOusec = 0;
    DWORD recvReturn;
    clock_t startRTT = clock();
    int a = sizeof(SenderSynHeader);
    for (int i = 1; i < 4; i++)
    {

        if (sendto(sock, (char*)&packet, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
            // if (sendto(sock, pointer , bytes , 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf(" [%0.3f] --> failed sendto with %d\n",
                (double)(clock() - time) / CLOCKS_PER_SEC,
                WSAGetLastError());
            closesocket(sock);
            WSACleanup();
           // return GetLastError();
            return FAILED_SEND;

        }
        printf(" [%0.3f] --> SYN 0 (attempt %d of 3, RTO %0.3g) to %d\n",
            (double)(clock() - time) / CLOCKS_PER_SEC,
            i,
            (double)RTOsec + (double)RTOusec / 1e6
            , IP
        );

        recvReturn = recvFrom(RTOsec, RTOusec, true);
        if (recvReturn  == STATUS_OK || recvReturn == FAILED_RECV )
        {
            break;
        }
    }
    this->RTT = (clock() - startRTT) / CLOCKS_PER_SEC;
    return recvReturn; 
}

DWORD SenderSocket::recvFrom(long RTOsec, long RTOusec, bool inOpen)
{
    // send request to the server
    timeval timeout;
    if (inOpen)
    {
        timeout.tv_sec = RTOsec;
        timeout.tv_usec = RTOusec;
        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        int available = select(0, &fd, NULL, NULL, &timeout);
        if (available > 0)
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
            // packet->lp.RTT = (clock() - time) / CLOCKS_PER_SEC;
            printf(" [%0.3f] <-- ",
                (double)(clock() - time) / CLOCKS_PER_SEC);
            printf("SYN-ACK 0 window 1; setting initial RTO to %0.3g\n",
                (double)RTOsec + (double)RTOusec / 1e6
            );
        }
        else if (available >= 0)
        {
            return TIMEOUT;
        }
    }
    else
    {
        timeout.tv_sec = RTOsec;
        timeout.tv_usec = RTOusec;
        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        int available = select(0, &fd, NULL, NULL, &timeout);
        if (available > 0)
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
            // packet->lp.RTT = (clock() - time) / CLOCKS_PER_SEC;
            printf(" [%0.3f] <-- ",
                (double)(clock() - time) / CLOCKS_PER_SEC);
            printf("FIN-ACK 0 window 0; setting initial RTO to %0.3g\n",
               3*RTT
            );
        }
        else if (available >= 0)
        {
            return TIMEOUT;
        }
    }
    return STATUS_OK;
}

DWORD SenderSocket::Send(char* pointer, UINT64 bytes ) {
    return STATUS_OK;
}
DWORD SenderSocket::Close() {
    int count = 0;
    DWORD recvReturn;
    while (count < 5)
    {
        count++;
        if (sendto(sock, (char*)&packet, sizeof(SenderDataHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf(" [%0.3f] --> failed sendto with %d\n",
                (double) (clock() - time) / CLOCKS_PER_SEC,
                WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            // return GetLastError();
            return FAILED_SEND;
        }
        printf(" [%0.3f] --> FIN 0 (attempt %d of 5, RTO %0.3g)\n",
            (double) (clock() - time) / CLOCKS_PER_SEC,
            count,
            3*RTT
        );

        recvReturn = recvFrom(1, 0, true);
        if (recvReturn == STATUS_OK || recvReturn == FAILED_RECV)
        {
            break;
        }
    }
    return STATUS_OK; // happy
}

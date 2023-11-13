// hw3p1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <iostream>
#include <string>
#include "SenderSocket.h"
#include "Checksum.h"


#define STATUS_OK 0 // no error
#define ALREADY_CONNECTED 1 // second call to ss.Open() without closing connection
#define NOT_CONNECTED 2 // call to ss.Send()/Close() without ss.Open()
#define INVALID_NAME 3 // ss.Open() with targetHost that has no DNS entry
#define FAILED_SEND 4 // sendto() failed in kernel
#define TIMEOUT 5 // timeout after all retx attempts are exhausted
#define FAILED_RECV 6 // recvfrom() failed in kernel

#define FORWARD_PATH 0
#define RETURN_PATH 1

#define MAGIC_PORT 22345 // receiver listens on this port
#define MAX_PKT_SIZE (1500-28) // maximum UDP packet size accepted by receiver 
using namespace std;

DWORD WINAPI sendThreads(LPVOID tempPointer)
{


    SenderSocket* ss = (SenderSocket*)tempPointer;
    char* charBuf = ss->sendBufCheckSum;
    UINT64 byteBufferSize = ss->packetSizeSend;
    DWORD status = STATUS_OK;
    UINT64 off = 0; // current position in buffer
    // sender is not producing? for the worker to consume properly? check sender and worker for fix

    while (off < byteBufferSize)
    {
        // decide the size of next chunk
        int bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));
        // send chunk into socket
        // ask if this popualtes send like it should and then creates the vector as needed
        if ((status = ss->Send(charBuf + off, bytes)) != STATUS_OK)
        {
            printf("Main: connect failed with status %d\n", status);
            return status;
            // error handing: print status and quit

        }

        off += bytes;
    }
    return status;
}


void main(int argc, char** argv)
{
    LinkProperties linkedProperties;




   // std::string host("128.194.135.1");
   // std::string host("128.194.135.82");
   // std::string host("0.0.0.0");
    std::string host("127.0.0.1");
  // std::string host("s3.irl.cs.tamu.edu");

    int power = 25;
    int sendingWindow = 5000;
    linkedProperties.RTT = 0.2;
    linkedProperties.pLoss[FORWARD_PATH] = 0.001;
    // should converge to this loss rate per n packets
    linkedProperties.pLoss[RETURN_PATH] = 0.0001;
    linkedProperties.speed = 1000;

    /*
    if (argc != 8)
    {
        printf(" Incorrect args, propper format is host, power, RTT, forward loss, backward loss, and speed \n");
        return;
    }
    std::string host(argv[1]);
    int power = atoi(argv[2]);
    int sendingWindow = atoi(argv[3]);
    linkedProperties.RTT = atof(argv[4]);
    linkedProperties.pLoss[FORWARD_PATH] = atof(argv[5]);
    linkedProperties.pLoss[RETURN_PATH] = atof(argv[6]);
    linkedProperties.speed = atof(argv[7]);
    */


    clock_t timeOpen = clock();
    printf("Main:   sender W = %d, RTT %.3f sec, loss %g / %g, link %g Mbps\n",
        sendingWindow,
        linkedProperties.RTT,
        linkedProperties.pLoss[FORWARD_PATH],
        linkedProperties.pLoss[RETURN_PATH],
        linkedProperties.speed
    );



    linkedProperties.speed *= 1e6;
    // parse command-line parameters
    char* targetHost = (char*)host.c_str();
    UINT64 dwordBufSize = (UINT64)1 << power;
    DWORD* dwordBuf = new DWORD[dwordBufSize]; // user-requested buffer
    for (UINT64 i = 0; i < dwordBufSize; i++) // required initialization
        dwordBuf[i] = i;
    SenderSocket ss; // instance of your class
    DWORD status;
    printf("Main:   initializing DWORD array with 2^%d elements... done in %d ms\n",
        power,
        ((clock() - timeOpen) * 1000) / CLOCKS_PER_SEC

    );

    // stats theard start thread

    if ((status = ss.Open(targetHost, MAGIC_PORT, sendingWindow, &linkedProperties)) != STATUS_OK)
    {
        // error handling: print status and quit
        printf("Main:   connect failed with status %d\n", status);
        ss.opened = false;
        return;

    }
    clock_t firstDataPacketSend = clock();
    
    printf("Main:   connected to %s in %.3f sec, pkt size %d bytes\n",
        host.c_str(),
        ss.sampleRTT,
        MAX_PKT_SIZE
    );

    char* charBuf = (char*)dwordBuf; // this buffer goes into socket
    UINT64 byteBufferSize = dwordBufSize << 2; // convert to bytes
    UINT64 bytes = 0;

    ss.packetSizeSend = byteBufferSize;
    ss.sendBufCheckSum = charBuf;

    /*
    UINT64 off = 0; // current position in buffer
    // sender is not producing? for the worker to consume properly? check sender and worker for fix


    while (off < byteBufferSize)
    {
        // decide the size of next chunk
        int bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));
        // send chunk into socket
        // ask if this popualtes send like it should and then creates the vector as needed
        if ((status = ss.Send(charBuf + off, bytes)) != STATUS_OK)
        {
            printf("Main: connect failed with status %d\n", status);
            return;
            // error handing: print status and quit

        }

        off += bytes;
    }

    */

    HANDLE sendThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) sendThreads, &ss, 0, NULL);
    WaitForSingleObject(sendThread, INFINITE);
    CloseHandle(sendThread);
    /*
    */

        double elapsedTime = (double)(clock() - firstDataPacketSend) / CLOCKS_PER_SEC;


        SetEvent(ss.closeConnection);
        WaitForSingleObject(ss.workers, INFINITE);
        CloseHandle(ss.workers);


        ss.st.breakThread = true;
        WaitForSingleObject(ss.st.statusEvent, INFINITE);
        CloseHandle(ss.st.statusEvent);


        /*
        SetEvent(st.statusEvent);
        WaitForSingleObject(st.statusEvent, INFINITE);
        CloseHandle(st.statusEvent);
        */


    // recieve from
    // double connectToMain = (clock() - ss.startRTT)/CLOCKS_PER_SEC;
    if ((status = ss.Close()) != STATUS_OK)
    {
        // error handing: print status and quit
        ss.opened = false;
        printf("Main:   connect failed with status %d\n", status);
        return;

    }
    printf("[%.3f] <-- FIN-ACK %d window %X\n",
        (double) (ss.timeAtClose - firstDataPacketSend ) /CLOCKS_PER_SEC,
        ss.st.packetsSendBaseStats,
        ss.hexDumpPost
        );


    // help here TA ASK

    printf("Main:   transfer finished in %.3f sec, %.2f Kbps, checksum %X\n",
        (double) elapsedTime,
        (byteBufferSize * 8 / elapsedTime ) / 1000 ,
        ss.hexDumpPost
    );




    printf("Main: estRTT %.3f, ideal rate %.3f Kbps \n",
        ss.estimateRTT,
       ( ( MAX_PKT_SIZE - sizeof(SenderDataHeader) )  / ( 1000 * ss.estimateRTT ) )  * 8// windoq is always 1
    );


}


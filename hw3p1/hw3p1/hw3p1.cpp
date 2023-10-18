// hw3p1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <ctype.h> 
#include <stdio.h> 
#include <string>
#include <windows.h>
#include <iostream>
#include "pch.h"
#include "SenderSocket.h"
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

#pragma comment(lib, "ws2_32")

#include "pch.h"

using namespace std;



void main(int argc, char** argv)
{
     LinkProperties linkedProperties;
    std::string host("s3.irl.cs.tamu.edu");
   // std::string host("127.0.0.1");
    int power = 20;
    int sendingWindow = 10;
    linkedProperties.RTT = 0.2;

    linkedProperties.pLoss[FORWARD_PATH] = 0.6;
    linkedProperties.pLoss[RETURN_PATH] = 0.0001;
    linkedProperties.speed = 100.0;

    /*
    LinkProperties linkedProperties;
    std::string host(argv[1]);
    int power = atoi(argv[2]);
    int sendingWindow = atoi(argv[3]);
    linkedProperties.RTT = atof( argv[4] );
    linkedProperties.pLoss[FORWARD_PATH] = atof( argv[5] );
    linkedProperties.pLoss[RETURN_PATH] = atof( argv[6] );
    linkedProperties.speed = atof( argv[7] );
    */


    clock_t timeOpen = clock();
    printf("Main: sender W = %d, RTT %.3f sec, loss %g / %g, link %g Mbps\n",
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
    printf("Main: initializing DWORD array with 2^%d elements... done in %d ms\n",
        power,
        ((clock() - timeOpen) * 1000) / CLOCKS_PER_SEC
        
        );

    if ((status = ss.Open(targetHost, MAGIC_PORT, sendingWindow, &linkedProperties)) != STATUS_OK)
    {
        // error handling: print status and quit
        printf("Main: connect failed with status %d\n", status);
        ss.opened = false;
                return;

    }
    /*

    char* charBuf = (char*)dwordBuf; // this buffer goes into socket
    UINT64 byteBufferSize = dwordBufSize << 2; // convert to bytes
    UINT64 bytes = 0;

        UINT64 off = 0; // current position in buffer


        while (off < byteBufferSize)
        {
            // decide the size of next chunk
            bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));
            // send chunk into socket
            if ((status = ss.Send(charBuf + off, bytes)) != STATUS_OK)
            {
                printf("Main: connect failed with status %d\n", status);
                return;
                // error handing: print status and quit

            }


                off += bytes;
        }
    */
    char* charBuf = (char*)dwordBuf; // this buffer goes into socket
    UINT64 byteBufferSize = dwordBufSize << 2; // convert to bytes
        UINT64 off = 0; // current position in buffer
    UINT64 bytes = 0;
    bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));

    printf("Main: connected to %s in %.3f sec, pkt size %d bytes\n",
        host.c_str(),
        ss.RTT,
        bytes + ss.bytesRec
        );
    // recieve from
    // double connectToMain = (clock() - ss.startRTT)/CLOCKS_PER_SEC;
    if ((status = ss.Close()) != STATUS_OK)
    {
        // error handing: print status and quit
        ss.opened = false;
        printf("Main: connect failed with status %d\n", status);
                return;

    }

    printf("Main: transfer finished in %.3f sec\n",
          (double) ((ss.closeTime- ss.startRTT) / CLOCKS_PER_SEC)
    );

    printf(" close called time is %.3f \n startRTT time is %.3f \n", (double)ss.closeTime/1000, (double)ss.startRTT);
}


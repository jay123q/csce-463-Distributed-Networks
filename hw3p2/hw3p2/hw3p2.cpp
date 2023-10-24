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
using namespace std;

DWORD WINAPI status_thread_run (LPVOID tempPointer)
{
    SenderSocket* tempSocket = (SenderSocket*)tempPointer;
    return tempSocket->statusThread();

}


void main(int argc, char** argv)
{
    LinkProperties linkedProperties;




   // std::string host("128.194.135.1");
   std::string host("s3.irl.cs.tamu.edu");
   // std::string host("128.194.135.82");
   // std::string host("0.0.0.0");
   // std::string host("127.0.0.1");

    int power = 15;
    int sendingWindow = 1;
    linkedProperties.RTT = 0.1;
    linkedProperties.pLoss[FORWARD_PATH] = 0;
    linkedProperties.pLoss[RETURN_PATH] = 0;
    linkedProperties.speed = 14;

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
    ss.st.statusEvent = new HANDLE[0];
    ss.st.statusEvent = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)status_thread_run, &ss, 0, NULL);






    if ((status = ss.Open(targetHost, MAGIC_PORT, sendingWindow, &linkedProperties)) != STATUS_OK)
    {
        // error handling: print status and quit
        printf("Main:   connect failed with status %d\n", status);
        ss.opened = false;
        return;

    }
    clock_t OpenReturnTime = clock();
    

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


            // potential crit section?
            // stats theard bytesAcked
            ss.st.bytesAcked += bytes;



                off += bytes;
        }
    


    // recieve from
    // double connectToMain = (clock() - ss.startRTT)/CLOCKS_PER_SEC;
    if ((status = ss.Close()) != STATUS_OK)
    {
        // error handing: print status and quit
        ss.opened = false;
        printf("Main:   connect failed with status %d\n", status);
        return;

    }

    printf("Main:   transfer finished in %.3f sec\n",
        (double)((ss.closeCalledTime - OpenReturnTime) / CLOCKS_PER_SEC)
    );

}


#include "pch.h"
#include "SenderSocket.h"
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")
#define STATUS_OK 0 // no error
#define ALREADY_CONNECTED 1 // second call to ss.Open() without closing connection
#define NOT_CONNECTED 2 // call to ss.Send()/Close() without ss.Open()
#define INVALID_NAME 3 // ss.Open() with targetHost that has no DNS entry
#define FAILED_SEND 4 // sendto() failed in kernel
#define TIMEOUT 5 // timeout after all retx attempts are exhausted
#define FAILED_RECV 6 // recvfrom() failed in kernel
#define RETX_OCCURED 10 // recvfrom() failed in kernel
#define DEFAULT_QUIET_COMPILER 50
#define DO_NOTHING 100
#define BREAK_CASE 999


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

 // #define debug
 #define sephamore
// #define ackWindow

DWORD WINAPI SenderSocket::runStats(LPVOID tempPointer)
{
    SenderSocket* stats = (SenderSocket*)tempPointer;
    bool printMe = false;

    stats->st.prevPrintBase = stats->st.packetsSendBaseStats;
    while (((WaitForSingleObject(stats->st.statusEvent, 2000) == WAIT_TIMEOUT) || !printMe) && stats->opened)
    {
        if (stats->st.breakThread == true)
        {
            // avoid final erronous rpint

            return 20;
        }



        // help here TA ASK
        stats->st.goodPutStats = (stats->st.packetsSendBaseStats - stats->st.prevPrintBase) / stats->st.nNumberPrints * 8 * (MAX_PKT_SIZE - sizeof(SenderDataHeader));

        stats->st.countRTTs += 1;

        printMe = true;
        printf("[ %1d] B %d ( %.1f MB) N %d T %d F %d W %d S %.3f Mbps RTT %.3f\n",
            // (int)(double)(st.prevTimerStats - st.startTimerStats) / CLOCKS_PER_SEC,
            stats->st.timerPrint2sec,
            stats->st.packetsSendBaseStats,
            stats->st.bytesTotal / 1e6,
            stats->sendSequenceNumber,
            stats->st.timeoutCountStats,
            stats->st.fastRetransmitCountStats,
            stats->st.effectiveWindowStats,
            stats->st.goodPutStats / 2e6,
            stats->st.estimateRttStats / 1000
        );
        stats->st.averageRTT += (stats->st.estimateRttStats);
        stats->st.timerPrint2sec += 2;
        stats->st.nNumberPrints++;
        // st.prevPrintBase = st.packetsSendBase;
       // st.prevTimerStats = st.startTimerStats;
       // st.startTimerStats = clock();
        ResetEvent(stats->st.statusEvent);
#ifdef ackWindow
        printf(" the ack winddow of the reciver is %d | the ack sequence is %d | the most recently sentpkt is %d |\n", stats->st.rhWindow, stats->st.rhSeqNum, stats->st.packetsSendBaseStats);
#endif // ackWindow

    }
    return 20;
}

SenderSocket::SenderSocket()
{

    st.breakThread = false;
#ifdef ackWindow
    st.rhWindow = 0;
    st.rhSeqNum = 0;
#endif // ackWindow

    this->time = clock();
    memset(&remote, 0, sizeof(remote));
    memset(&server, 0, sizeof(server));


}
SenderSocket::~SenderSocket()
{
    DWORD areThreadsAlive;
    // figure out if the workers are still wokrin
    GetExitCodeThread(workers, &areThreadsAlive);

    if (areThreadsAlive == STILL_ACTIVE)
    {
        SetEvent(closeConnection);
        // if therer are packets wait
        WaitForSingleObject(workers, INFINITE);
        CloseHandle(workers);

        WaitForSingleObject(stats, INFINITE);
        CloseHandle(stats);
    }

}
DWORD SenderSocket::Open(string host, int portNumber, int senderWindow, LinkProperties* lp) {
    if (opened)
    {
        return ALREADY_CONNECTED;
    }

    packetsSharedQueue = new Packet[senderWindow];
    // polulate this queue

    triggerClosingonWokers = false;

    // stats thread initialization
    st.startTimerStats = clock();
    st.timerPrint2sec = 2;
    st.packetsSendBaseStats = 0;
    st.bytesTotal = 0;
    st.timeoutCountStats = 0;
    st.fastRetransmitCountStats = 0;
    st.effectiveWindowStats = senderWindow;
    st.goodPutStats = 0;
    st.estimateRttStats = 0;
    sendSequenceNumber = 0;
    st.nNumberPrints = 1;
    st.prevTimerStats = 0.0;
    this->lastReleased = 0;
    dupAck = 0;
    closeCalled = false;
    this->senderWindow = senderWindow;
    nextToSend = 0;
    st.prevPrintBase = st.packetsSendBaseStats;

#ifndef  sephamore

    empty = CreateSemaphore(NULL, this->senderWindow, this->senderWindow, NULL);

#endif // ! sephamore
#ifdef sephamore

    empty = CreateSemaphore(NULL, 0 , this->senderWindow, NULL);
#endif // sephamore

    full = CreateSemaphore(NULL, 0, this->senderWindow, NULL);


    st.statusEvent = CreateEvent(NULL, true, false, NULL);
    socketReceiveReady = CreateEvent(NULL, false, false, NULL);
    closeConnection = CreateEvent(NULL, false, false, NULL);


    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        WSACleanup();
        return -1;
    }

    this->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (this->sock == INVALID_SOCKET) {
        printf(" [%.3f] --> target %s is invalid\n",
            (double)(clock() - time) / CLOCKS_PER_SEC,
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
            (double)(clock() - time) / CLOCKS_PER_SEC,
            host.c_str()
        );
        closesocket(sock);
        WSACleanup();
        printf(" invalid bind erorr \n ");
        return GetLastError();
    }


    this->packetSyn = new SenderSynHeader();
    memset(packetSyn, 0, sizeof(SenderSynHeader));
    packetSyn->lp.bufferSize = senderWindow + 3; // window size is 10 with retransmit it 3x
    packetSyn->lp.pLoss[0] = lp->pLoss[0];
    packetSyn->lp.pLoss[1] = lp->pLoss[1];
    packetSyn->lp.RTT = lp->RTT;
    packetSyn->lp.speed = lp->speed;
    this->st.averageRTT = 0;
    this->st.countRTTs = 0;
    packetSyn->sdh.flags.reserved = 0;
    packetSyn->sdh.flags.magic = MAGIC_PROTOCOL;
    packetSyn->sdh.flags.SYN = 1;
    packetSyn->sdh.flags.FIN = 0;
    packetSyn->sdh.flags.ACK = 0;
    packetSyn->sdh.seq = 0;


    this->numRTX = 0;
    duplicateAckCheck = -1;



    // packetsSharedQueue.push(packetSyn);


    pLossForwardFin = lp->pLoss[0];
    pLossBackwardFin = lp->pLoss[1];
    RTTFin = lp->RTT;
    speedFin = lp->speed;
    this->sendSequenceNumber = 0;

    this->host = host.c_str();
    DWORD IP = inet_addr(host.c_str());
    this->IP = IP;
    if (IP == INADDR_NONE)
    {
        if ((this->remote = gethostbyname(host.c_str())) == NULL)
        {

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

    this->breakConnection = false;
    server.sin_family = AF_INET;
    server.sin_port = htons(portNumber); // DNS port on server
    long RTOsec = max(1.0000,  2 * lp->RTT);
    long RTOusec = 0;
    for (int i = 1; i < REATTEMPTS_ALLOWED; i++)
    {
    this->timeToAckforSampleRTT = clock();
        // you saw cc cc cc cc showing that the packet waas on the heap due to & being used. 
       // printf(" [%.3f] --> SYN 0 (attempt %d of 3, RTO %.3f) to %s\n",

        if (sendto(sock, (char*)packetSyn, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            continue;


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
            this->setRTO *= 2; // dont timeout again
            continue;
            // return TIMEOUT;
        }
        else
        {
            ReceiverHeader rh;
            struct sockaddr_in response;
            int responseSize = sizeof(response);

            int bytes = recvfrom(sock, (char*)&rh, sizeof(ReceiverHeader), 0, (struct sockaddr*)&response, &responseSize);
            if (bytes == SOCKET_ERROR)
            {
                this->setRTO *= 2; // dont timeout again
                continue;
                // return GetLastError();
            }

            this->sampleRTT = 1000*((double)(clock() - timeToAckforSampleRTT) / CLOCKS_PER_SEC);
            this->estimateRTT = this->sampleRTT;
            this->deviationRTT = 0;
            this->setRTO = this->estimateRTT + 4*max(10, this->deviationRTT);

         
#ifdef sephamore
            this->lastReleased = min(this->senderWindow, rh.recvWnd);
            ReleaseSemaphore(empty, lastReleased, NULL);
#endif


            if (WSAEventSelect(this->sock, this->socketReceiveReady, FD_READ) != 0)
            {
                printf(" intenal screaming aaaaaaaaaaaaaa \n");
            }

            stats = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)runStats, this, 0, NULL);
            workers = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)workerThreads, this, 0, NULL);
            // send the stats thread
            break;

        }
    }
    opened = true;
    return STATUS_OK;
}

DWORD SenderSocket::Close() {


   // int RTOsec = floor(3 * this->sampleRTT);
   // int RTOusec = (3 * this->sampleRTT - RTOsec) * 1e6;
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
    packetFin->sdh.seq = this->sendSequenceNumber;
    pastAckedPkt = 0;
    this->closeCalledTime = clock();
    int count = 0;
   
    while (count < REATTEMPTS_ALLOWED)
    {
        this->timeToAckforSampleRTT = clock();
        count++;
       // printf(" Close Transmit #%d \n", count);
        if (sendto(sock, (char*)packetFin, sizeof(*packetFin), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf(" close sendto error %d \n", WSAGetLastError());
        }
     //   printf(" passed send in close \n");
        timeval timeout;
        timeout.tv_sec = this->setRTO/1000;
        timeout.tv_usec = (3 * this->sampleRTT - setRTO) * 1e6;;
        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        int available = select(0, &fd, NULL, NULL, &timeout);
       // printf(" passed select in close \n");
        if (available <= 0)
        {
            // restart clock
            this->setRTO *= 2; // dont timeout again
         //   printf(" available failed with %d \n", available);
        //        printf(" close select error %d \n", WSAGetLastError());
                continue;
        }
        else
        {
            ReceiverHeader rh;
            struct sockaddr_in response;
            int responseSize = sizeof(response);

            int bytes = recvfrom(sock, (char*)&rh, sizeof(ReceiverHeader), 0, (struct sockaddr*)&response, &responseSize);
       // printf(" passed recv from in close \n");
            if (bytes == SOCKET_ERROR)
            {
     //        printf(" close recvfrom error %d \n", WSAGetLastError());
                break;
                // return GetLastError();
            }
            /*
            this->sampleRTT = (double)(clock() - timeToAckforSampleRTT) / CLOCKS_PER_SEC;
            findRTO();
            */

        }
     //   printf(" finsihed and closing \n");
        
        closesocket(sock);
        WSACleanup();
        break;
    }


    // chcksum here
    this->hexDumpPost = checkValidity.CRC32((unsigned char*)this->sendBufCheckSum, this->packetSizeSend);
        timeAtClose = clock();
    return STATUS_OK;





}
int SenderSocket::Send(char* data, int size)
{
    // worker waits on     HANDLE events[] = { worker->socketReceiveReady , worker->full };

    HANDLE arr[] = { closeConnection, empty };
    WaitForMultipleObjects(2, arr, false, INFINITE);
    // old this->timeToAckforSampleRTT = clock();
   // printf(" sending pkt %d \n", sendSequenceNumber);
    // timeToAckforSampleRTT
    // no need for mutex as no shared variables are modified
    int slot = sendSequenceNumber % this->senderWindow;
     Packet* p = packetsSharedQueue + slot; // pointer to packet struct
    // Packet* p; // pointer to packet struct
    
     // cout << " the sequence number is " << sendSequenceNumber << endl;
    //SenderDataHeader* sdh = (SenderDataHeader*)p->packetContents;
    SenderDataHeader sdh;
    // this->countSentPkts++;
    sdh.flags.reserved = 0;
    sdh.flags.magic = MAGIC_PROTOCOL;
    sdh.flags.ACK = 0;
    sdh.flags.SYN = 0;
    sdh.flags.FIN = 0;
    sdh.seq = this->sendSequenceNumber;
    p->size = size + sizeof(SenderDataHeader);
    p->txTime = clock();
    // printf(" send seq numebr %d \n", st.packetesSendBaseNextPacketStats);
    memcpy_s(p->packetContents, p->size, (char*) &sdh, sizeof(SenderDataHeader));
    // printf("asdfafaf %s",a);
    memcpy_s(p->packetContents + sizeof(SenderDataHeader), p->size, data, size);
    this->sendSequenceNumber++;
    // packetsSharedQueue[ slot ] = p;
    ReleaseSemaphore(full, 1, NULL);
    return STATUS_OK;
}
DWORD WINAPI SenderSocket::workerThreads(LPVOID tempPointer)
{
    SenderSocket* worker = (SenderSocket*)tempPointer;
    int kernelBuffer = 20e6; // 20 meg
    if (setsockopt(worker->sock, SOL_SOCKET, SO_RCVBUF, (char*)&kernelBuffer, sizeof(int)) == SOCKET_ERROR)
    {
        return 0;
    }


    kernelBuffer = 20e6; // 20 meg
    if (setsockopt(worker->sock, SOL_SOCKET, SO_SNDBUF, (char*)&kernelBuffer, sizeof(int)) == SOCKET_ERROR)
    {
        return 0;
    }

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    HANDLE events[] = { worker->socketReceiveReady , worker->full };
    // main raises event, that then checks to say close annd hold to have acks
    // and once workers clossed then close stats
    int index = 0;
    int countRemainingThreads = 0;
    char* fullpkt = nullptr;
    double timerExpire = 0;
    while (true)
    {
        // check this lter
        if (worker->closeCalled && (worker->st.packetsSendBaseStats == worker->sendSequenceNumber))
        {
          //  printf(" broke properly \n");
            break;
        }


// int indexOfCircularArray = index % worker->st.effectiveWindowStats;
        timerExpire = clock() + CLOCKS_PER_SEC * (worker->setRTO/1000);
        double timeout = 0;
        if (worker->packetsSharedQueue->packetContents[0] != -52 )
        {
            // RTO - START
            timeout = ((double) (timerExpire - clock()) /  CLOCKS_PER_SEC);
            timeout *= 1000;

        }
        else
        {
            timeout = INFINITE;
        }

        // timeout = INFINITE;
     // while to end all remaining threads
        int indexOfCircularArray = 0;
           // timeout = INFINITE;
        int ret = WaitForMultipleObjects(2, events, false, timeout);


        bool timeoutOccured = false;
        bool fastRetransmitOccured = false;
        bool sendBaseMoveForward = false;
        DWORD status = DEFAULT_QUIET_COMPILER;
      // printf(" timeout is %f \n", timeout);
        int slot = worker->st.packetsSendBaseStats % worker->senderWindow;
  
        switch (ret)
        {
        case WAIT_TIMEOUT: // normal retransmit
#ifdef debug

           // printf(" Timeout: sendbase %d | dup Ack send base %d | dup ack count %d | num RTX count %d | timeout time is %f | Estimated RTT is %f | \n", worker->st.packetsSendBaseStats, worker->duplicateAckCheck, worker->dupAck, worker->numRTX, timeout/1000 , (double)worker->st.estimateRttStats / 1000);
#endif 
            worker->packetsSharedQueue[worker->st.packetsSendBaseStats % worker->senderWindow].txTime = (double) clock();
            if (sendto(
                worker->sock,
                worker->packetsSharedQueue[worker->st.packetsSendBaseStats % worker->senderWindow].packetContents,
                worker->packetsSharedQueue[worker->st.packetsSendBaseStats % worker->senderWindow].size,
                0,
                (struct sockaddr*)&worker->server, sizeof(worker->server)
            ) == SOCKET_ERROR)
            {
               printf(" wait timeout with %d\n",
                   WSAGetLastError());

            }
            worker->st.timeoutCountStats++;
            timeoutOccured = true;

            break;


        case WAIT_OBJECT_0 + 0: // move senderBase; update RTT; handle fast retx; do flow control
            status = worker->ReceiveACK();


            if (status == RETX_OCCURED)
            {
                fastRetransmitOccured = true;
                // worker->dupAck = 0;
            }
            else if (status == STATUS_OK)
            {
                sendBaseMoveForward = true;

            }
            else if (status == TIMEOUT)
            {
                timeoutOccured = true;
            }
            else if (status == DO_NOTHING)
            {
                // this is if the dupACK != 3
                // nuthin
              //  printf(" timeout is %f \n", timeout);
            }


            break;
        case WAIT_OBJECT_0 + 1:

            // printf(" worker send packet %d \n", indexOfCircularArray);

            indexOfCircularArray = worker->nextToSend % worker->senderWindow;
            worker->packetsSharedQueue[indexOfCircularArray].txTime = (double) clock();
            if (worker->st.packetsSendBaseStats == worker->duplicateAckCheck)
            {
                timeoutOccured = true;
                }
            /*
            */
            #ifdef debug

          // printf(" send pkt: worker sends base %d | sendbase %d | dup Ack send base %d | dup ack count %d | Estimated RTT is %f | \n", worker->nextToSend, worker->st.packetsSendBaseStats, worker->duplicateAckCheck, worker->dupAck ,(double) worker->st.estimateRttStats/1000 );
            #endif 
            if (sendto(worker->sock,
                worker->packetsSharedQueue[indexOfCircularArray].packetContents,
                worker->packetsSharedQueue[indexOfCircularArray].size,
                0,
                (struct sockaddr*)&worker->server, sizeof(worker->server)) == SOCKET_ERROR
                ) {
                printf("failed with %d\n",
                    WSAGetLastError());
            }
            worker->nextToSend++;
            // worker->countSentPkts--;
             // set here seocketreadyHandler
            break;
        default:
            // handle failed wait;
            printf(" bgger badder error occcured, sendersocket.cpp 641 \n");
            break;
        }
        if ( timeoutOccured || fastRetransmitOccured  || sendBaseMoveForward)
        {

            timerExpire = clock() + CLOCKS_PER_SEC * (worker->setRTO)/1000;
            timeoutOccured = false;
            fastRetransmitOccured = false;
            sendBaseMoveForward = false;
#ifdef debug
            printf(" recalc expire %f \n", timerExpire);

#endif // !debug

        }

    }
    return STATUS_OK;
}


DWORD SenderSocket::ReceiveACK()
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
        return WSAGetLastError();
        // return GetLastError();
    }
    if (rh.ackSeq < this->st.packetsSendBaseStats)
    {
        return DO_NOTHING;
    }
    if (dupAck > 0)
    {
      //  printf("RECIEVE ACK NORMAL: numberRTX %d | ack check %d | ackSeq %d | send base %d | last pkt sent %d | estimated RTT is %f \n", numRTX, dupAck, rh.ackSeq, this->st.packetsSendBaseStats, this->nextToSend, this->st.estimateRttStats);

   }
    DWORD returnValue;
    this->numRTX++;
    if (rh.ackSeq > this->st.packetsSendBaseStats)
    { // 
        returnValue = STATUS_OK;
        int numberOfPacketsRecieved = rh.ackSeq - this->st.packetsSendBaseStats;
        int packetSize = packetsSharedQueue[(rh.ackSeq - 1) % this->senderWindow].size - sizeof(SenderDataHeader);
        /*
        if (packetSize != 1464)
        {
            printf(" aaaaaaaaaaaaaaa \n");
            string a (packetsSharedQueue[(rh.ackSeq - 1) % this->senderWindow].packetContents);
            int b = this->st.packetsSendBaseStats;
            
            printf(" base is %d \n", b);
            //2933720 
            printf(" aaaaaaaaaaaaaaa \n");

        }
        */
        this->st.bytesTotal += packetSize +(numberOfPacketsRecieved-1)*(MAX_PKT_SIZE-sizeof(SenderDataHeader));

        // if we have dun goofed, do not count the ack
        // printf(" the packet time is %f \n", (double) packetsSharedQueue[(rh.ackSeq - 1) % this->senderWindow].txTime);
        if (dupAck == 0 && this->numRTX == 1 && rh.ackSeq-1 == this->st.packetsSendBaseStats )
        {
            // first two are the to confirm first send, next is to ensure that we take the RTT of what ack is expcting and not measure RTX or timeout inside
            double measuredRTT = 1000* ( (double) (clock() - packetsSharedQueue[(rh.ackSeq-1) % this->senderWindow].txTime) / CLOCKS_PER_SEC );
            // findRTO();
            double b = 0.25;
            this->deviationRTT = (1 - b) * this->deviationRTT + b * abs(measuredRTT - this->estimateRTT);
            double a = 0.125;
            this->estimateRTT = (1 - a) * this->estimateRTT + a * measuredRTT;
            st.estimateRttStats = this->estimateRTT;
            #ifdef debug
             printf("IN Recv: numberRTX %d | ack check %d | ackSeq %d | send base %d | last pkt sent %d | estimated RTT is %f \n", numRTX, dupAck, rh.ackSeq, this->st.packetsSendBaseStats, this->nextToSend,this->st.estimateRttStats );
            #endif 
            this->setRTO = this->estimateRTT + 4 * max(this->deviationRTT, 10);
            st.effectiveWindowStats = min(this->senderWindow, rh.recvWnd);
#ifdef ackWindow
            st.rhWindow =rh.recvWnd;
            st.rhSeqNum = rh.ackSeq;
#endif // ackWindow
           // printf(" sample RTT %f | estimate RTT %f | deviation RTT %f | RTO %f \n", this->sampleRTT, this->estimateRTT, this->deviationRTT, this->setRTO);
        }



        this->st.packetsSendBaseStats = rh.ackSeq;
        this->dupAck = 0;
        this->numRTX = 0;
        duplicateAckCheck = -1;
#ifndef sephamore

        ReleaseSemaphore(this->empty, numberOfPacketsRecieved, NULL);
#endif // !sephamore


#ifdef sephamore
            // how much we can advance the semaphore
            int windowStats = min( this->senderWindow , rh.recvWnd);
            int newReleased = st.packetsSendBaseStats + windowStats - this->lastReleased;
            ReleaseSemaphore(this->empty, newReleased,NULL);
            this->lastReleased += newReleased;
#endif
        // not deallocating 8 bits on empty here
       // printf(" send packet number %d acked number %d \n ", debugSendBase, rh.recvWnd );
        return returnValue;
        // handing no packets acked as a thw switch
    }
    else
    {
       this->st.packetsSendBaseStats = rh.ackSeq;
        // this->nextToSend = rh.ackSeq;
       if (this->dupAck == 0)
       {
           duplicateAckCheck = rh.ackSeq;
            this->dupAck++;
       }
       if (duplicateAckCheck == rh.ackSeq)
        {

         this->dupAck++;
             if (this->dupAck == 3)
             {

                st.fastRetransmitCountStats++;
              //  printf(" retransmited packet's seq number %d \n", ( this->st.packetsSendBaseStats % this->st.effectiveWindowStats ));
             #ifdef debug
               printf("In RTX : numberRTX %d | ack check %d | ackSeq %d | send base %d | estimated RTT is %f \n", numRTX, dupAck, rh.ackSeq, this->st.packetsSendBaseStats, this->st.estimateRttStats );
             #endif
                this->packetsSharedQueue[st.packetsSendBaseStats % this->senderWindow ].txTime = (double) clock();
                sendto(
                    this->sock,
                    this->packetsSharedQueue[st.packetsSendBaseStats % this->senderWindow ].packetContents,
                    this->packetsSharedQueue[st.packetsSendBaseStats % this->senderWindow ].size,
                    0,
                    (struct sockaddr*)&this->server, sizeof(this->server)
                );
                // retransmit
                return RETX_OCCURED;
             }

        }
        else if( numRTX > 4 )
        {
           duplicateAckCheck = -1;

            // numRTX = 0;
        }
        return DO_NOTHING;


    }

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
    this->setRTO = this->estimateRTT + 4 * max(this->deviationRTT, 10);
}

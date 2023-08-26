#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>

#pragma once


const int INITIAL_BUF_SIZE = 8512;

class Socket {
	SOCKET sock; // socket handle
	char* buf[INITIAL_BUF_SIZE]; // current buffer
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer
	// extra stuff as needed
	Socket();
	bool Read(void);
	// void CreateSocket(void);
};


Socket::Socket()
{
	// create this buffer once, then possibly reuse for multiple connections in Part 3
		this->allocatedSize = INITIAL_BUF_SIZE;
		this->curPos = 0;
		// ripping from winsock
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
		{
			printf("socket() generated error %d\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		/*
		* perhaps need this to connect the socket
		if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			printf("Connection error: %d\n", WSAGetLastError());
			return;
		}
		*/


}


bool Socket::Read(void)
{
	//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select
	// 
	// set timeout to 10 seconds
	// int timer = clock();
	fd_set readFds;
	fd_set writeFds;
	fd_set exceptFds;

	// check this
	timeval* timeout;
	timeout->tv_sec = 10;
	timeout->tv_usec = 0;
	// check this

	clock_t start = clock(); /// suspect
	clock_t finish = clock(); // shut up compiler
	while (true)
	{
		// wait to see if socket has any data (see MSDN)
		int ret = select(0, &readFds, &writeFds, &exceptFds, timeout);
		if ( ret  > 0)
		{
			// new data available; now read the next segment
			int bytes = recv( sock, *(this->buf) + this->curPos, (allocatedSize - this->curPos), 0 );
			if (errors)
				// print WSAGetLastError()
				break;
			if (connection closed)
				// NULL-terminate buffer
				return true; // normal completion
			curPos += bytes; // adjust where the next recv goes
			if (allocatedSize - curPos < 1024)
				// resize buffer; you can use realloc(), HeapReAlloc(), or
			   // memcpy the buffer into a bigger array
		}
		else if (timeout)
		{
			// report timeout
			buf[curPos + 1] = '\0'; // something done goofed, wrap it up chief.
			finish = clock();
			double duration = (double)(finish - start) / CLOCKS_PER_SEC;
			printf("finished at %. durration with %d bytes \n", duration * 1000, curPos);
			
			break;
		}
		else
			// print WSAGetLastError()
			// ret returned a 0 which means no sockkets opened

			printf("Connection error: %d\n", WSAGetLastError());

			break;
	}
	return false;
}
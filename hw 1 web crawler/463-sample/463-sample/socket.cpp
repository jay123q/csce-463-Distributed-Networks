#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>

#pragma once


const int INITIAL_BUF_SIZE = 8512;

class Socket {
	SOCKET sock; // socket handle
	char* buf; // current buffer
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
		char* buf[INITIAL_BUF_SIZE]; // either new char [INITIAL_BUF_SIZE] or malloc (INITIAL_BUF_SIZE)
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
	clock_t timeout = clock(); // I think this is 10 seconds check
	// check this

	clock_t start = clock(); /// suspect
	while (true)
	{
		// wait to see if socket has any data (see MSDN)
		int ret = select(0, &readFds, &writeFds, &exceptFds, timeout);
		if ( ret  > 0)
		{
			// new data available; now read the next segment
			int bytes = recv( sock, buf + curPos, allocatedSize – curPos, 0 );
			if (errors)
				// print WSAGetLastError()
				break;
			if (connection closed)
				// NULL-terminate buffer
				return true; // normal completion
			curPos += bytes; // adjust where the next recv goes
			if (allocatedSize – curPos < THRESHOLD)
				// resize buffer; you can use realloc(), HeapReAlloc(), or
			   // memcpy the buffer into a bigger array
		}
		else if (timeout)
			// report timeout
			clock_t finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			break;
		else
			// print WSAGetLastError()
			// ret returned a 0 which means no sockkets opened
			printf("Connection error: %d\n", WSAGetLastError());

			break;
	}
	return false;
}
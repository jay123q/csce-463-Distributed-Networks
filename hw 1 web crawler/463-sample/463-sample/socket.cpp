#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string.h>
using namespace std;
#pragma once
#include "socket.h"




Socket::Socket()
{
	// create this buffer once, then possibly reuse for multiple connections in Part 3
		this->allocatedSize = INITIAL_BUF_SIZE;
		this->curPos = 0;
		// ripping from winsock
		this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
		{
			printf("socket() generated error %d\n", WSAGetLastError());
			WSACleanup();
			return;
		}


}

void Socket::closeSocket()
{
	closesocket(this->sock);
}


bool Socket::Send(string link, string host, int port, string pathQueryFragment)
{

	cout << "URL: " << link << std::endl;
	cout << "\tParsing URL... ";
	const char* httpLinkCheck = strstr(link.c_str(), "://");

	if (httpLinkCheck  == nullptr)
	{
		//cout << " linkcheck HTTPS " << httpLinkCheck << std::endl;
		cout << "failed with invalid scheme " << std::endl;

		return false;
	}
	if (port == 65536)
	{
		cout << " failed with invalid port " << std::endl;
		return false;
	}
	cout << "host " << host << ", port " << port << ", request /" << pathQueryFragment << std::endl;


	struct hostent* remote;
	struct sockaddr_in server;
	// given in slides
	// 
	// 
	// task 2
	cout << '\t' << "Doing DNS... ";
	clock_t start = clock();
	clock_t finish = clock(); // compiler wont shut up about this


	double ip = inet_addr(host.c_str());
	if (ip == INADDR_NONE)
	{
		// dns lookup
		remote = gethostbyname(host.c_str());

		if (remote == nullptr)
		{
			printf("failed with %d\n", WSAGetLastError());
			return false;
		}
		
		
		// check this if it aint working


		memcpy( &server.sin_addr, remote->h_addr, remote->h_length);

	}
	else
	{
		server.sin_addr.S_un.S_addr = ip;
	}
	finish = clock();
	double timepassed = double(finish - start) / (double)CLOCKS_PER_SEC;

	cout << "done in " << timepassed * 1000 << " ms, found " << inet_ntoa(server.sin_addr) << std::endl;
	



	// task 3
	cout << '\t' << "Connecting on page... ";
	start = clock();
	server.sin_family = AF_INET; // IPv4
	server.sin_port = port; // port #

		if (connect(sock, (struct sockaddr*)&server,sizeof(struct sockaddr_in)) == SOCKET_ERROR)
		{
			printf("Connection error: %d\n", WSAGetLastError());
			return false;
		}


	// print
	finish = clock();
	timepassed = double(finish - start) / (double)CLOCKS_PER_SEC;
	cout << "done in " << timepassed * 1000 << " ms " << std::endl;

	//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
	// add a +1 for nnull terminator
	if (send(this->sock, link.c_str(), strlen(link.c_str()) + 1, 0) == SOCKET_ERROR)
	{
		printf("Connection error: %d\n", WSAGetLastError());
		return false;
	}


	return true;
}

bool Socket::Read(void)
{
	cout << '\t' << "Loading... ";
	//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select
	// 
	// set timeout to 10 seconds
	// int timer = clock();
	fd_set readFds;
	fd_set writeFds;
	fd_set exceptFds;

	// check this
	timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	// check this

	clock_t start = clock(); /// suspect
	clock_t finish = clock(); // shut up compiler
	while (true)
	{
		// wait to see if socket has any data (see MSDN)
		int ret = select(0, &readFds, &writeFds, &exceptFds, &timeout);
		if (ret > 0)
		{
			// new data available; now read the next segment
			int bytes = recv(sock, *(this->buf) + this->curPos, (allocatedSize - this->curPos), 0);
			if (bytes < 0)
			{
				printf("Connection error: %d\n", WSAGetLastError());
				break;
			}
			if (bytes == 0)
			{
				*buf[curPos + 1] = '\0';
				// NULL-terminate buffer
				finish = clock();
				double duration = (double)(finish - start) / CLOCKS_PER_SEC;
				printf("finished at %.1f durration with %d bytes \n", duration * 1000, curPos);
				curPos += bytes; // adjust where the next recv goes
				return true; // normal completion
			}
			if (this->allocatedSize - curPos < 1024)
			{
				//resize
				char* tmp = new char [this->allocatedSize * 2];
				memcpy(tmp, buf, this->allocatedSize);
				this->allocatedSize *= 2;
				*buf = tmp;
				delete [] tmp;

				// resize buffer; you can use realloc(), HeapReAlloc(), or
			   // memcpy the buffer into a bigger array
			}
		}
		else if (ret == 0 )
		{
			// ret returned a 0 which means no sockkets opened
			// report timeout
			cout << "connecction timeod out, failed on ret, returned 0 meaning no sockets opened, select in socket.cpp" << std::endl;
			break;
		}
		else
			// print WSAGetLastError()

			printf("Connection error: %d\n", WSAGetLastError());

			break;
	}
	return false;
}
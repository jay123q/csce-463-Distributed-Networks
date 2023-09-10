// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023

#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <set>
using namespace std;
#pragma once
#include "socket.h"

int PAGE_MAX = 2097152; // 2mb
int ROBOT_MAX = 16384; // 16 KB


Socket::Socket()
{
	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}
	// create this buffer once, then possibly reuse for multiple connections in Part 3
		this->allocatedSize = INITIAL_BUF_SIZE;
		this->curPos = 0;
		this->buf = new char [INITIAL_BUF_SIZE];
		this->robots = true;
		this->remote = nullptr;
		//this->server = nullptr;
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
	// https://stackoverflow.com/questions/15504016/c-winsock-socket-error-10038-wsaenotsock#:~:text=%22%20Select()%20function%20error%20code,checked%20by%20the%20select%20function.
	// yoinked from  ^^^^ 
	if (closesocket(sock) == SOCKET_ERROR)
	{
		printf("closesocket() generate error: %d", WSAGetLastError());
		return;
	}
	// delete buf;
	// closesocket(sock);
	// cout << " close the socket plox " << sock << std::endl;
	this->sock = INVALID_SOCKET;
	


}

bool Socket::DNSCheck(std::string host)
{

	DWORD IP = inet_addr(host.c_str());

	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		// cout << " find invalid memoery  hceck " << this->remote << std::endl;
		if ((this->remote = gethostbyname(host.c_str())) == NULL)
		{
			printf("Connection error: %d\n", WSAGetLastError());
			return false;
		}
		else // take the first IP address and copy into sin_addr
		{

			memcpy((char*)&(this->server.sin_addr), this->remote->h_addr, this->remote->h_length);
		}
		// debugging! 
		// 
		//	printf ("Connection error: %d\n", WSAGetLastError ());

	}
	else
	{
		this->server.sin_addr.S_un.S_addr = IP;
	}

	
	clock_t start = clock();
	clock_t finish = clock(); // compiler wont shut up about this


	finish = clock();
	double timepassed = double(finish - start) / (double)CLOCKS_PER_SEC;


	cout << "done in " << timepassed * 1000 << " ms, found " << inet_ntoa(this->server.sin_addr) << std::endl;
	// this->IP = inet_ntoa(this->server.sin_addr);
	return true;
}

bool Socket::Connect(int port)
{
	// cout << " socket is " << sock << std::endl;
	clock_t start = clock();
	this->server.sin_family = AF_INET; // IPv4
	this->server.sin_port = htons(port); // port #


	if (connect(this->sock, (struct sockaddr*)&(this->server), sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("Connection error: %d\n", WSAGetLastError());
		return false;
	}
	double timepassed = double(clock() - start) / (double)CLOCKS_PER_SEC;
	cout << "done in " << timepassed * 1000 << " ms " << std::endl;

	// print
	return true;
}

bool Socket::Send(string sendRequest , string host)
{
	if (strlen(buf) > 32768)
	{
		// delete buffer and reallocate
		delete[] buf;
		this->buf = new char[INITIAL_BUF_SIZE];
	}
	// 
	// 
	// task 2
	// cout << " server connction S_addr" << this->server.sin_addr.S_un.S_addr << std::endl;
	// cout << " server connction sin family " << this->server.sin_family << std::endl;
	// cout << " server connction sin_port " << this->server.sin_port << std::endl;
	
	// task 3
	// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect
	// yoinked from here ^
	// cout << '\t' << " * Connecting on page... ";

	//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-send
	// add a +1 for nnull terminator
	// std::string tamusendfuckyou("GET / HTTP/1.0\r\n User-agent: myTAMUcrawler/1.0\r\n Host: tamu.edu\r\n Connection: close\r\n");
	// char* sendBuf = new char[strlen(sendRequest.c_str()+1)];
//	cout <<'|' << sendRequest.length() << '|' << std::endl;
//	printf(" send bytes:: ", sendRequest.length());
	// string get_http = "GET / HTTP/1.1\r\nHost: http://tamu.edu\r\nConnection: close\r\n\r\n";
	// const char* httpRequest = "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n";
	if (send( this->sock, sendRequest.c_str() , strlen(sendRequest.c_str()), 0) == SOCKET_ERROR)
	{

		printf("Connection error: %d\n", WSAGetLastError());
		return false;
	}
	// cout << " passed successfully connected " << std::endl;

	return true;
}

bool Socket::Read(void)
{
	cout << '\t' << "   Loading... ";
	//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select
	// 
	// set timeout to 10 seconds
	// int timer = clock();
	fd_set readFds;
	// fd_set writeFds;
	// fd_set exceptFds;

	// check this
	timeval timeout;
	timeval futureTime;
	// futureTime.tv_sec = clock() + 10;

	// if hanging your error is in how you handle the path


	clock_t start = clock(); /// suspect
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	// check this
	this->curPos = 0;

	int counter = 0;
	while (true)
	{
		timeout.tv_sec -= (double) (clock() - start) / CLOCKS_PER_SEC;
		FD_ZERO(&readFds); // this sets the file descriptor 
		// I learend a good lesson here, it dont work if you dont got a file descriiptor
		// https://learn.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-fd_set
		FD_SET(sock, &readFds); // assign a socket to a descriptor
		start = clock();

		// wait to see if socket has any data (see MSDN)
		// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select
		int ret = select(0 ,  &readFds, NULL , NULL , &timeout);
		// cout << " loop  counter " << counter << std::endl;
		counter++;
		// cout << " return from select " << ret << std::endl;
		// cout << " timer check " << (double)timeout.tv_sec << std::endl;
		if (ret > 0)
		{
			// cout << " select has passed! " << '\n';
			// new data available; now read the next segment
			int bytes = recv(sock, this->buf + curPos, allocatedSize - curPos, 0);
			// cout << " bytes from recv " << bytes << std::endl;
			if (bytes < 0)
			{
				printf("Failed with %d\n", WSAGetLastError());
				break;
			}
			if (bytes == 0)
			{
				if (this->curPos < 50)
				{
					cout << "failed with non-HTTP header (does not begin with HTTP/) \n";
					return false;
				}
				this->buf[curPos] = '\0'; // 3rd notes said +1 was wrong
				// NULL-terminate buffer
				clock_t finish = clock(); // shut up compiler
				double duration = (double)(finish - start) / CLOCKS_PER_SEC;
				printf("done in %.1f ms with %d bytes \n", duration * 1000, curPos);
				return true; // normal completion
			}

			curPos += bytes; // update cursor
			// take 512 bites beofre resizing
			if (this->allocatedSize - curPos < this->allocatedSize/4 )
			{
				// cout << " the allocated size is " << this->allocatedSize << std::endl;
				// before reallocating
				/**/

				if (!this->robots && this->allocatedSize ==  PAGE_MAX)
				{
					cout << "failed exceeding max \n";
					break;
				}
				else if (this->robots && this->allocatedSize  == ROBOT_MAX)
				{
					cout << "failed exceeding max \n";
					break;

				}
				
				char * tmp = new char[this->allocatedSize * 2];
				memcpy(tmp, buf, this->allocatedSize);
				this->allocatedSize *= 2;
				delete buf;
				this->buf = tmp;
				// delete[] tmp;

				// resize buffer; you can use realloc(), HeapReAlloc(), or
			   // memcpy the buffer into a bigger array
			}


			// cout << this->allocatedSize - curPos << " bytes from the buffer " << std::endl;

			


		}
		else if (ret == 0)
		{
			// ret returned a 0 which means no sockkets opened
			// report timeout
			cout << " failed with timeout" << std::endl;
			break;
		}
		else
			// print WSAGetLastError()
		{
			printf("Connection error: %d\n", WSAGetLastError());

			break;
		}
	}
	
	return false;
}



/*
void Socket::ReadSendCheckStatus(parsedHtml *parser)
{
	parser.generateRequesttoSend("HEAD");
	bool socketCheck = this->Send(parser.total, parser.wholeLink, parser.host, parser.port, parser.printPathQueryFragment());

	if (socketCheck)
	{
		// now try to read
		if (Read())
		{
			closeSocket(); // maybe move this into read? 
			// so now the html should return the buffer soo
			const char* result = this->printBuf().c_str();
			// cout << " the result  is " << this->printBuf() << std::endl;
			double statusCode = stod(this->printBuf().substr(9.3).c_str());
			cout << "\t Verifying header... ";
			cout << " status code " << statusCode << std::endl;
			if (statusCode <= 199 || statusCode >= 299)
			{
				clock_t start = clock();
				clock_t finish = clock();
				cout << "\t Parsing page... ";
				HTMLParserBase htmlLinkRipper;
				int nLinks = 0;
				char* linkCounter = htmlLinkRipper.Parse((char * ) result ,strlen(result),(char *) parser.wholeLink.c_str() , strlen(parser.wholeLink.c_str()), &nLinks);
				if (nLinks < 0)
					nLinks = 0;
				finish = clock();
				double timer = (double)(finish - start) / CLOCKS_PER_SEC;
				printf("done in %.1f ms with %d links", timer *1000 ,nLinks);


			}
			else
			{
				//int skipBadLinks = webSocket->printBuf().find("\r\n\r\n");
				cout << "status code " << statusCode << std::endl;
			}


			// cout << "status code " << statusCode << std::endl;
			// const char* httpStatus = strstr(webSocket->printBuf(), "HTTP/1.1");
			// llook for \r\n\r\n to parse
			// int skipBadLinks = this->printBuf().find("\r\n\r\n");
			// now modify the pointer/string so we can remove the essentail first half

		}
	}
}
*/





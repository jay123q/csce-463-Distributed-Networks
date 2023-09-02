#pragma once
#include <string>
#include <iostream>
#include "parsedHTML.h"
#include "HTMLParserBase.h"

const int INITIAL_BUF_SIZE = 8192;

class Socket {
private:
	SOCKET sock; // socket handle
	char* buf; // current buffer
	bool robots;
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer
	struct hostent* remote;
	struct sockaddr_in server;
public:
	// extra stuff as needed
	Socket();
	bool Read(void);
	bool Send(std::string sendRequest, std::string host, int port);
	bool DNSCheck(std::string host);
	void closeSocket();
	//void ReadSendCheckStatus(parsedHtml &parser);
	const char * printBuf()
	{
		return buf;
	}
	int getCurPos()
	{
		return this->curPos;
	}
	hostent* getRemote()
	{
		return this->remote;
	}
	sockaddr_in getServer()
	{
		return this->server;
	}
	// void CreateSocket(void);
};
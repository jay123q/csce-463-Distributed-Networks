// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023

#pragma once
#include <string>
#include <iostream>
#include "parsedHTML.h"
#include "HTMLParserBase.h"

const int INITIAL_BUF_SIZE = 8192;

class Socket {
private:
	char* buf; // current buffer // kill this
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer
	struct hostent* remote;
	struct sockaddr_in server;
public:
	// extra stuff as needed
	SOCKET sock; // socket handle
	bool robots;
	// DWORD IP;
	// bool printDNStiming;
	Socket();
	~Socket() 
	{
		// closeSocket();
		// delete server;

	}
	bool Read(void);
	bool Send(std::string sendRequest, std::string host);
	bool DNSCheck(std::string host);
	bool Connect(int port);
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
	void setServer(struct sockaddr_in parserServerToSet)
	{
		this->server = parserServerToSet;
	}
	
	// void CreateSocket(void);
};
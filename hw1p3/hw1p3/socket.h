// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023

#pragma once
#include <string>
#include <iostream>
#include "parsedHTML.h"
#include "HTMLParserBase.h"
#include <set>
#include <queue>

const int INITIAL_BUF_SIZE = 8192;


class Socket {
private:
	char* buf; // current buffer // kill this
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer
	struct hostent* remote;
public:
	// extra stuff as needed
	struct sockaddr_in server;
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
	int parser(char * stringResult, int bytes_file , char * wholeLink)
	{
		int nLinks = 0;
		HTMLParserBase htmlLinkRipper;
		char* linkCounter = htmlLinkRipper.Parse( stringResult , bytes_file,
			wholeLink, strlen( wholeLink ), &nLinks);  // 43
		return nLinks;
	}
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
	struct sockaddr_in getServer()
	{
		return this->server;
	}
	void setServer(struct sockaddr_in parserServerToSet)
	{
		this->server = parserServerToSet;
	}
	
	// void CreateSocket(void);
};
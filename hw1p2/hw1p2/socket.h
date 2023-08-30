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
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer
public:
	// extra stuff as needed
	Socket();
	bool Read(void);
	bool Send(std::string sendRequest , std::string link, std::string host, int port, std::string pathQueryFragment);
	void closeSocket();
	//void ReadSendCheckStatus(parsedHtml &parser);
	std::string printBuf()
	{
		return buf;
	}
	int getCurPos()
	{
		return this->curPos;
	}
	// void CreateSocket(void);
};
// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
#pragma once
#include "pch.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "HTMLParserBase.h"
#include "socket.h"
#include <string>
#include <set>


class Socket;

using namespace std;
class parsedHtml
{

	public:
		set<string> seenHosts;
		set<DWORD> seenIPs;
		int port;
		string host;
		string fragment;
		string query;
		string path;
		string wholeLink;
		string total;
		string httpStatus;
		Socket* webSocket;
		struct sockaddr_in serverParserTemp;
		char* readFileBuf;
		int intFileSize;
		void resetParser(void);
		bool parseString(string link); // parses the url
		void generateGETrequestToSend(void);
		void generateHEADrequestToSend(void);
		vector <string> parseTXTFile(std::string filename);
		char * parseTXTFileBROKEN(std::string filename);

		bool RobotSendRead(void);
		bool ReconnectHostSend(void);
		bool urlCheck(std::string link, string pathQueryFragment);
		void transferSetServer(struct sockaddr_in webSocketServer)
		{
			this->serverParserTemp = webSocketServer;
		}
		string printHost()
		{
			return "http://" + this->host;
		}
		string printPath()
		{
			return this->path;
		}
		string printPort()
		{

			return ':' + std::to_string(this->port);
		}
		string printfragment()
		{
			return '#' + this->fragment;
		}
		string printQuery()
		{
			return '?' + this->query;
		}
		string printWholeLink()
		{
			return this->wholeLink;
		}
		string printPathQueryFragment()
		{
			string total = this->path;

			if ( this->query[0] != '\0' )
			{
				total += '?' + this->query;
			}


			return total;
		}
};
// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
#pragma once
#include "pch.h"
#include <string>
#include <fstream>
#include <queue>
#include <iostream>
#include "HTMLParserBase.h"
#include "socket.h"
#include <string>
#include <set>


class Socket;
class HTMLParserBase;
using namespace std;
class parsedHtml
{

	public:
		HTMLParserBase * htmlLinkRipper;
		int totalExtractedNoSub;
		int newNumberBytesInBatch;
		int newNumberPagesInBatch;
		int numberExtractedURL; // inside file read
		int numberUniqueHost; // inside the first check
		int numberDnsLookup; // inside the first check
		int numberIpUnique; // insidde the first check 
		int numberRobotPass; // inside the robots function, passed robots 
		int numberSuccessfullyCrawled; //  full http inside of the reconnect and resent function
		int numberTotalLinks; // inside of reconnecct and send parser
		int http200;
		int http300;
		int http400;
		int http500;
		int httpXXX;

		int tamuCounterStack;
		int tamuCounterPrint;

		set<string> seenHosts;
		set<DWORD> seenIPs;
		CRITICAL_SECTION extractUrlLock;
		CRITICAL_SECTION genericSyntaxLock;
		CRITICAL_SECTION urlCheckLock; // this increments when the url returns a status 200 found in parser, reconnect and run 
		CRITICAL_SECTION dnsCheckLock; // url check dns increment
		CRITICAL_SECTION ipCheckLock; // rl ccheck ip increment
		CRITICAL_SECTION robotCheckLock; // handles counting the  robots
		CRITICAL_SECTION bitHandlingCheckLock; // handles pages and curpos bytes
		CRITICAL_SECTION hostCheckUnique; // host check in url parser
		CRITICAL_SECTION linkCkeckLock; // links found in parser, reconnect and run 

		CRITICAL_SECTION statusCheckMux;// this is for the html status found in parser, reconnect and run 

		int parserHelper(HTMLParserBase* htmlLinkRipper, char* stringResult, int bytes_file, char* wholeLink)
		{
			int nLinks = 0;

			char* linkCounter = htmlLinkRipper->Parse(stringResult, bytes_file,
				wholeLink, strlen(wholeLink), &nLinks);  // 43


			// tamu check
			bool tamu = true;
			if (tamu)
			{
				for (int i = 0; i < nLinks; i++)
				{
					std::string linkTamu(linkCounter);

					string hostStart = linkTamu.substr(7);

					size_t fragmentIndex = hostStart.find_first_of('#');
					
					if (fragmentIndex != string::npos) {
						std::string fragmentTamu = hostStart.substr(fragmentIndex + 1);
						hostStart = hostStart.substr(0, fragmentIndex);
					}

					size_t queryIndex = hostStart.find_first_of('?');
					if (queryIndex != string::npos) {
						std::string queryTamu = hostStart.substr(queryIndex + 1);
						hostStart = hostStart.substr(0, queryIndex);
					}

					size_t pathIndex = hostStart.find_first_of('/');
					if (pathIndex != string::npos) {
						std::string pathTamu = hostStart.substr(pathIndex);
						hostStart = hostStart.substr(0, pathIndex);
					}
					else {
						std::string pathTamu = "/";
					}
					size_t found = hostStart.find("tamu.edu");

					if (found != string::npos )
					{// 7 for http and 8 for tamu.edu
						/*
						if ((linkTamu[found + 15] == '/' || linkTamu[found + 15] == '\0'))
						{
							cout << "/ or \0 " << this->wholeLink << std::endl;
						}
						else
						{
							cout << " general tamu check passed " << this->wholeLink << std::endl;

						}
						*/
						// tamuLinkCountStack = tamuLinkCountStack + hostStart + "\r\n";
						this->tamuCounterStack += 1;

						/*
						out.open("output.txt");
						if (out.is_open())
						{

							out << hostStart << std::endl;
							out.close();
						}
						else
						{
							cout << " couldnt open file \n";
						}
						*/
					}

				}

			}


			return nLinks;
		}
		std::ofstream out;
		int port;
		string host;
		string fragment;
		string query;
		string path;
		string wholeLink;
		string total;
		string urlLink;
		string httpStatus;
	// struct sockaddr_in server;
		Socket* webSocket;
		string tamuLinkCountStack;
		string tamuLinkCountPrint;
		struct sockaddr_in serverParserTemp;
		char* readFileBuf;
		int intFileSize;


		parsedHtml();
		~parsedHtml();
		void setNumbersForCrawling();
		void resetParser(void);
		void runOnce(const char * urlLink);
		bool parseString(string link); // parses the url
		void generateGETrequestToSend(void);
		void generateHEADrequestToSend(void);
		queue <string> parseTXTFile(std::string filename);
		char * parseTXTFileBROKEN(std::string filename);

		bool RobotSendRead(int portPassed);
		bool ReconnectHostSend(int portportPassed);
		bool urlCheck(std::string link);
		void transferSetServer(struct sockaddr_in webSocketServer)
		{
			memcpy(&serverParserTemp, &webSocketServer, sizeof(sockaddr_in));
				/*
			serverParserTemp.sin_addr = webSocketServer.sin_addr;
			this->serverParserTemp.sin_family = webSocketServer.sin_family;
			this->serverParserTemp.sin_port = webSocketServer.sin_port;
				*/
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
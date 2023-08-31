#pragma once
#include "pch.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
using namespace std;
#include "HTMLParserBase.h"
#include "socket.h"
class parsedHtml
{

	public:
		int port;
		string host;
		string fragment;
		string query;
		string path;
		string wholeLink;
		string total;
		string httpStatus;
		char* readFileBuf;
		int intFileSize;

		void parseString(const char* link);
		void generateRequesttoSend( string request );
		std::vector <std::string> parseTXTFile(std::string filename);

		void ParseSendRead(string url);
		string printHost()
		{
			return "http://" + this->host;
		};
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
			if ( this->fragment[0] != '\0')
			{
				total += '#' + this->fragment;
			}

			return total;
		}
};
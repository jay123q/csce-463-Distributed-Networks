#pragma once
#include "pch.h"
#include <string>
#include <iostream>
using namespace std;

class parsedHtml
{

	public:
		int port;
		string host;
		string fragment;
		string query;
		string path;
		string wholeLink;
		string httpStatus;


		void parseString(const char* link);
		string printerGenerate( string request );
		string printHost()
		{
			return "http://" + this->host;
		};
		string printPath()
		{
			return '/' + this->path;
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
			cout << " path die " << this->path << std::endl;
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
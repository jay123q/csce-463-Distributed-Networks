// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
#include "pch.h"
#include <string>
#include "socket.h"
#include "parsedHTML.h"
#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;
/*

*/

void winsock_test(void);

void continueRunning(parsedHtml* parser, const char * urlLink )
{
	// cout << " size of  main  parser " << sizeof(parser) << std::endl;

	// parser->parseString(urlLink);
	bool urlPass = parser->urlCheck( urlLink , parser->printPathQueryFragment());
	parser->webSocket->robots = true;
	// parser->webSocket->printDNStiming = true;
	// 
	if (urlPass != true)
	{
		// cout << " URL FAILED moving on to next url move this main.cpp \n";
		return;
	}

	// taking a copy of the server
	parser->transferSetServer(parser->webSocket->getServer());
	// cout << " the socket is " << parser->webSocket->sock << std::endl;


	bool robotPass = parser->RobotSendRead();
	if (robotPass != true)
	{
	//	cout << "ROBOT FAILED  sending to robots failed in main, moving on to next \n";
		return;
	}

	// parser->webSocket->printDNStiming = false;
	parser->webSocket = new Socket();
	parser->webSocket->setServer(parser->serverParserTemp);
	bool sendPass = parser->ReconnectHostSend();
	if (sendPass != true)
	{
	//	cout << "RECONNECT HOST FAILED sending the request has failed in main, could not be a issue, moving to next remove me \n";
		return;
	}
	// cout << " finished the  main function contiune running 42 \n";
	// parser->webSocket->~Socket();
	

	
}



int main(int argc, char* argv[])
{

	// ayo these are guess, I cant get the vs community to open the properties tab and then show the defaul to throw in the texts
	// no idea tf is happening.
	int numberThreads = 0;
	bool runVector = false;
	parsedHtml parser;
	// parser.resetParser();
	parser.webSocket = new Socket();
	if (argc == 2)
	{
		// std::string filename = "http://allafrica.com/stories/201501021178.html";
		std::string filename = argv[1];
		continueRunning(&parser, filename.c_str());
	}
	else if (argc == 3)
	{
		numberThreads = stoi(argv[1]);
		std::string filename = argv[2];
		vector<string> totalVector =  parser.parseTXTFile(filename);
		// int stall = 0;
		cout << "Opened " << filename << " with size " << parser.intFileSize << std::endl;
		for (int i = 0; i < totalVector.size() ; i++)
		{
			continueRunning(&parser,totalVector.at(i).c_str());
			parser.resetParser();
			parser.webSocket = new Socket();
			// cin >> stall;
		}

	}
	else
	{
		cout << " invalid form! " << std::endl;
		cout << " should be ./main name.txt " << std::endl;
		return 0;
	}
	
	/* 
	numberThreads = 1;
	std::string filename = "100url.txt";
	vector<string> totalVector =  parser.parseTXTFile(filename);

	for (int i = 0; i < totalVector.size() ; i++)
	{
		continueRunning(&parser,totalVector.at(i).c_str());
		parser.resetParser();
		parser.webSocket = new Socket();
	}

	std::string filename = "http://www.weatherline.net/";
		continueRunning(&parser,filename.c_str());
	*/
	// parsedHtml parser2;
	/*
	std::string filename = "abchttp://";
	parser.resetParser();
	parser.webSocket = new Socket();
	// parser.webSocket->sock += 1;
		continueRunning(&parser,filename.c_str());

	*/
	// this was used for all testing locally


	
	//parser.parseTXTFile(url.argv[1]);

	//}

	return 0;
}

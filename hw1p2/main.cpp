/*
Joshua Clapp
Csce 463-500
*/
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

int contiuneRunning(parsedHtml* parser, const char * urlLink )
{
	parser->parseString(urlLink);
	bool urlPass = parser->urlCheck(parser->wholeLink, parser->printPathQueryFragment());
	parser->webSocket->robots = true;
	// parser->webSocket->printDNStiming = true;
	// 
	if (urlPass != true)
	{
		cout << " moving on to next url move this laster \n main.cpp \n";
		return -2;
	}

	// taking a copy of the server
	parser->transferSetServer(parser->webSocket->getServer());


	bool robotPass = parser->RobotSendRead();
	if (robotPass != true)
	{
		cout << " sending to robots failed in main, moving on to next \n";
		return -2;
	}

	// parser->webSocket->printDNStiming = false;
	parser->webSocket = new Socket();
	parser->webSocket->setServer(parser->serverParserTemp);
	bool sendPass = parser->ReconnectHostSend();
	if (sendPass != true)
	{
		cout << " sending the request has failed in main, could not be a issue, moving to next remove me \n";
		return -2;
	}
	cout << " finished the  main function contiune running 42 \n";
	return 0;
}



int main(int argc, char* argv[])
{

	// ayo these are guess, I cant get the vs community to open the properties tab and then show the defaul to throw in the texts
	// no idea tf is happening.
	int numberThreads = 0;
	bool runVector = false;
	parsedHtml parser;
	parser.resetParser();
	std::string url = "http://irl.cs.tamu.edu/";
	return contiuneRunning(&parser, url.c_str());
	/*
	if (argc == 2)
	{
		std::string filename = argv[1];
	}
	else if (argc == 3)
	{
		numberThreads = stoi(argv[1]);
		std::string filename = argv[2];
		std::vector<std::string> totalVector = parser.parseTXTFile(filename);
		for (int i = 0; i < totalVector.size() ; i++)
		{

		}

	}
	else
	{
		cout << " invalid form! " << std::endl;
		cout << " should be ./main name.hmtl " << std::endl;
		return 0;
	}
	


	*/
	
	// std::vector<string> urlList = parser.parseTXTFile("100url.txt");


	// this was used for all testing locally


	
	//parser.parseTXTFile(url.argv[1]);

	//}

	return 0;
}

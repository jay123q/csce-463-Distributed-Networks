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
#include "Crawler.h"
using namespace std;
/*

*/

void winsock_test(void);





int main(int argc, char* argv[])
{

	// ayo these are guess, I cant get the vs community to open the properties tab and then show the defaul to throw in the texts
	// no idea tf is happening.
	int numberThreads = 0;
	bool runVector = false;
	parsedHtml parser;
	Crawler crawler;
	/* 
	if (argc == 2)
	{
		// std::string filename = "http://allafrica.com/stories/201501021178.html";
		std::string filename = argv[1];
		runOnce( filename.c_str() );
	}
	else if (argc == 3)
	{
		numberThreads = stoi(argv[1]);
		std::string filename = argv[2];
		queue<string> totalVector =  parser.parseTXTFile(filename);
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
	
	numberThreads = 1;
	std::string filename = "100url.txt";
	vector<string> totalVector =  parser.parseTXTFile(filename);

	for (int i = 0; i < totalVector.size() ; i++)
	{
		continueRunning(&parser,totalVector.at(i).c_str());
		parser.resetParser();
		parser.webSocket = new Socket();
	}

	*/
	// numberThreads = stoi(argv[1]);
	// std::string filename = argv[2];
	std::string filename("100url.txt");
	queue<string> q = parser.parseTXTFile(filename);
	// int stall = 0;
	cout << "Opened " << filename << " with size " << parser.intFileSize << std::endl;


	// crawler start


	while(q.empty() != true )
	{
		// crawler.runParsingRobotsSendingStatus( q.front().c_str() );
		parser.resetParser();
		q.pop();
		// parser.webSocket = new Socket();
		// cin >> stall;
	}
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

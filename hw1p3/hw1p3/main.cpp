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
#pragma warning(disable:4996)
#pragma warning(disable:4099)
// Get current flag

using namespace std;
/*

*/
DWORD WINAPI crawler_thread_starter(LPVOID that)
{
	// cout << " afddsafs " << ((Crawler*)that)->q.front() << std::endl;
	Crawler* thatNew = (Crawler*)that;
	// cout << " count thread " << counter << std::endl;
	// cout << " front is " << thatNew->q.front() << std::endl;
	// Crawler* passCrawler = static_cast<Crawler*>(that);
	return thatNew->runParsingRobotsSendingStatus();
}

DWORD WINAPI status_thread_starter(LPVOID that)
{
	Crawler* thatNew = (Crawler*)that;

	return  thatNew->twoSecondPrint();
}

void handleThreads(Crawler * crawler, int numberThread)
{
	cout << "Opened " << crawler->crawlerFileName << " with size " << crawler->parserStats->intFileSize << std::endl;
	crawler->crawlersThread = new HANDLE[numberThread];
	crawler->startTimer = clock();
	crawler->statusThread = new HANDLE[0];
	// cout << " crawl check front " << crawler->q.front() << std::endl;
	crawler->statusThread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)status_thread_starter, crawler, 0, NULL);

	for (int i = 0; i < numberThread ; i++)
	{
		// cout << " i counter " << i << std::endl;
		crawler->crawlersThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)crawler_thread_starter, crawler , 0, NULL);

		EnterCriticalSection(&(crawler->threadQueueLock));
		// cout << " thread amount " << i << std::endl;
		crawler->numberThread++;
		LeaveCriticalSection(&(crawler->threadQueueLock));

	}


	for (int i = 0; i < numberThread; i++) {
		WaitForSingleObject(crawler->crawlersThread[i], INFINITE);
		CloseHandle(crawler->crawlersThread[i]);
	}
	//



/*
	if (checkMe != true)
	{

	}
*/
	bool checkMe = SetEvent(crawler->statusEvent);
	WaitForSingleObject(crawler->statusThread[0], INFINITE);
	CloseHandle(crawler->statusThread[0]);

	crawler->finalPrint();
}



int main(int argc, char* argv[])
{

	// ayo these are guess, I cant get the vs community to open the properties tab and then show the defaul to throw in the texts
	// no idea tf is happening.
	int numberThreads = 0;
	bool runVector = false;
	parsedHtml parser;

	Crawler * crawler = new Crawler();



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
	*/
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	int numberThread = 500;
	std::string filename("URL-input-1M.txt");
	crawler->crawlerFileName = filename;
	// crawler->startTimer = clock();
	crawler->q = crawler->parserStats->parseTXTFile(filename);
	// cout << "asdfasdf " << crawler->q.front() << std::endl;
	handleThreads(&(*crawler), numberThread);

	return 0;
}

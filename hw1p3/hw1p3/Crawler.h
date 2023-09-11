#pragma once
#include <string>
#include <queue>
#include <iostream>
#include "parsedHTML.h"

class parsedHtml;

class Crawler
{
public:


	int numberThread;

	double bytesDownloadedInBatch;
	double pagesDownloadedInBatch;
	double startTimer;
	double totalBytes;
	double totalPages;
	HANDLE* crawlersThread;
	HANDLE* statsThread;
	parsedHtml * parserHelper;
	string crawlerFileName;
	queue < std::string > q;

	// mux parameters
	CRITICAL_SECTION threadQueueLock;
	CRITICAL_SECTION editQueueLink;
	//CRITICAL_SECTION statusCheckLock;
	HANDLE statusEvent;
	Crawler();
	~Crawler();


	// void handleThreads(int threadNumbers );
	DWORD runParsingRobotsSendingStatus();
	DWORD twoSecondPrint();
	void finalPrint();

};
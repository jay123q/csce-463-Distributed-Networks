#pragma once
#include <string>
#include <iostream>
#include "parsedHTML.h"

class parsedHtml;

class Crawler
{
public:


	int numberSizePendingQueue;
	double bytesDownloadedInBatch;
	double pagesDownloadedInBatch;
	double startTimer;
	double totalBytes;
	double totalPages;
	HANDLE* crawlersThread;
	HANDLE* statsThread;
	parsedHtml * parserHelper;

	// mux parameters
	CRITICAL_SECTION extractedQueueLock;
	//CRITICAL_SECTION statusCheckLock;
	HANDLE statusEvent;
	Crawler();



	void createThreads(int threadNumbers );
	void WINAPI runParsingRobotsSendingStatus(const char* urlLink);
	void WINAPI twoSecondPrint();
	void finalPrint();

};
#pragma once
#include <string>
#include <queue>
#include <iostream>
#include "parsedHTML.h"

class parsedHtml;

class Crawler
{
public:


		int numberThread = 0;
		double bytesDownloadedInBatch;
		double pagesDownloadedInBatch;
		clock_t startTimer;
		double totalBytes;
		double totalPages;
		set<string> seenHosts;
		set<DWORD> seenIPs;
	HANDLE* crawlersThread;
	HANDLE* statusThread;

	parsedHtml * parserStats;
	string crawlerFileName;
	queue < std::string > q;
	CRITICAL_SECTION genericSyntaxLock;
	// mux parameters
	CRITICAL_SECTION threadQueueLock;
	CRITICAL_SECTION editQueueLink;
	//CRITICAL_SECTION statusCheckLock;
	HANDLE statusEvent;
	Crawler();
	~Crawler();


	// void handleThreads(int threadNumbers );
	DWORD runParsingRobotsSendingStatus();
	bool urlCrawler(std::string link);
	DWORD twoSecondPrint();
	void finalPrint();

};
#pragma once
#include <string>
#include <iostream>
#include "parsedHTML.h"

class parsedHtml;

class Crawler
{
public:


	int numberSizePendingQueue;
	int numberExtractedURL;
	double bytesDownloadedInBatch;
	double pagesDownloadedInBatch;
	double startTimer;
	double totalBytes;
	double totalPages;

	parsedHtml * parserHelper;

	void runParsingRobotsSendingStatus(const char* urlLink);
	void twoSecondPrint();
	void finalPrint();

};
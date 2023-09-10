#include "pch.h"
#include "Crawler.h"

void Crawler::runParsingRobotsSendingStatus(const char* urlLink)
{
	// cout << " size of  main  parser " << sizeof(parser) << std::endl;
	// parser.resetParser();
	this->parserHelper->webSocket = new Socket();
	// parser->parseString(urlLink);
	bool urlPass = parserHelper->urlCheck(urlLink, parserHelper->printPathQueryFragment());
	parserHelper->webSocket->robots = true;
	// parser->webSocket->printDNStiming = true;
	// 
	if (urlPass != true)
	{
		// cout << " URL FAILED moving on to next url move this main.cpp \n";
		parserHelper->resetParser();
		return;
	}

	// taking a copy of the server
	parserHelper->transferSetServer(parserHelper->webSocket->getServer());
	// cout << " the socket is " << parser->webSocket->sock << std::endl;

	// lock here??

	bool robotPass = parserHelper->RobotSendRead();
	if (robotPass != true)
	{
		//	cout << "ROBOT FAILED  sending to robots failed in main, moving on to next \n";
		parserHelper->resetParser();
		return;
	}

	// unlock here ??

	// parser->webSocket->printDNStiming = false;
	
	// lock here


	parserHelper->webSocket = new Socket();
	parserHelper->webSocket->setServer(parserHelper->serverParserTemp);
	bool sendPass = parserHelper->ReconnectHostSend();
	if (sendPass != true)
	{
		//	cout << "RECONNECT HOST FAILED sending the request has failed in main, could not be a issue, moving to next remove me \n";
		parserHelper->resetParser();
		return;
	}

	// unlock here
	// 
	// 
	// cout << " finished the  main function contiune running 42 \n";
	// parser->webSocket->~Socket();
		parserHelper->resetParser();



}


void Crawler::twoSecondPrint()
{
	printf("[%3d] %4d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n",
		(double)(clock() - this->startTimer) / CLOCKS_PER_SEC,
		this->numberSizePendingQueue,
		this->numberExtractedURL,
		this->parserHelper->numberUniqueHost,
		this->parserHelper->numberDnsLookup,
		this->parserHelper->numberIpUnique,
		this->parserHelper->numberRobotPass,
		this->parserHelper->numberSuccessfullyCrawled, // this is non robbot pages
		this->parserHelper->numberTotalLinks / 1000);

	this->bytesDownloadedInBatch = (this->parserHelper->newNumberBytesInBatch - this->totalBytes) / (double)((clock() - this->startTimer) / CLOCKS_PER_SEC);
	this->pagesDownloadedInBatch = (this->parserHelper->newNumberPagesInBatch - this->totalPages) / (double)((clock() - this->startTimer) / CLOCKS_PER_SEC);

	// Output crawling information
	printf("\t*** crawling %.1f pps @ %.1f Mbps\n", this->pagesDownloadedInBatch, this->bytesDownloadedInBatch / double(125000));
	this->totalBytes = this->parserHelper->newNumberBytesInBatch;
	this->totalPages = this->parserHelper->newNumberPagesInBatch;
	this->parserHelper->newNumberBytesInBatch = 0;
	this->parserHelper->newNumberPagesInBatch = 0;
}
void Crawler::finalPrint()
{

	double totalTimeElapsed = (double)((double)clock() - this->startTimer) / (double)CLOCKS_PER_SEC;

	// Output final stats
	printf("Extracted %d URLs @ %d/s\n", this->numberExtractedURL, (int)( this->numberExtractedURL / totalTimeElapsed));
	printf("Looked up %d DNS names @ %d/s\n", this->parserHelper->numberUniqueHost, (int)(this->parserHelper->numberDnsLookup / totalTimeElapsed));
	printf("Attempted %d robots @ %d/s\n", this->parserHelper->numberIpUnique, (int)(this->parserHelper->numberRobotPass / totalTimeElapsed));
	printf("Crawled %d pages @ %d/s (%.2f MB)\n", this->parserHelper->numberSuccessfullyCrawled, (int)(this->parserHelper->numberSuccessfullyCrawled / totalTimeElapsed), (double)(this->totalBytes / (double)1048576));
	printf("Parsed %d links @ %d/s\n", this->parserHelper->numberTotalLinks, (int)(this->parserHelper->numberTotalLinks / totalTimeElapsed));
	printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n", this->parserHelper->http200, this->parserHelper->http300, this->parserHelper->http400, this->parserHelper->http500, this->parserHelper->httpXXX);



}

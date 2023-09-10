#include "pch.h"
#include "Crawler.h"




Crawler::Crawler()
{

	InitializeCriticalSection(&extractedQueueLock);
	// InitializeCriticalSection(&statusCheckLock);


	InitializeCriticalSection(&(this->parserHelper->urlCheckLock)); 
	InitializeCriticalSection(&(this->parserHelper->dnsCheckLock));  
	InitializeCriticalSection(&(this->parserHelper->ipCheckLock));   
	InitializeCriticalSection(&(this->parserHelper->statusCheckMux));
	InitializeCriticalSection(&(this->parserHelper->robotCheckLock));
	InitializeCriticalSection(&(this->parserHelper->hostCheckUnique));
	InitializeCriticalSection(&(this->parserHelper->linkCkeckLock));
	// EnterCriticalSection(&CriticalSection);


	// LeaveCriticalSection(&CriticalSection);
	this->statusEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


}
Crawler::~Crawler()
{
	// DeleteCriticalSection(&CriticalSection);

	DeleteCriticalSection(&extractedQueueLock);
	// DeleteCriticalSection(&statusCheckLock);

	DeleteCriticalSection(&(this->parserHelper->urlCheckLock));
	DeleteCriticalSection(&(this->parserHelper->dnsCheckLock));
	DeleteCriticalSection(&(this->parserHelper->ipCheckLock));
	DeleteCriticalSection(&(this->parserHelper->statusCheckMux));
	DeleteCriticalSection(&(this->parserHelper->robotCheckLock));
	DeleteCriticalSection(&(this->parserHelper->hostCheckUnique));
	DeleteCriticalSection(&(this->parserHelper->linkCkeckLock));
}

void Crawler::createThreads(int threadNumber)
{

	this->crawlersThread = new HANDLE[threadNumber];

	this->statsThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &twoSecondPrint, NULL, 0, NULL);

	for (int i = 0; i < threadNumber; i++) {
		crawlersThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&runParsingRobotsSendingStatus, NULL, 0, NULL);

		WaitForSingleObject(params.paramMutex, INFINITE);
		params.numThreadsRunning++;
		ReleaseMutex(params.paramMutex);
	}

	WaitForSingleObject(fileLinks, INFINITE);

	for (int i = 0; i < numThreads; i++) {
		WaitForSingleObject(crawlers[i], INFINITE);
		CloseHandle(crawlers[i]);
	}
	//



	SetEvent(statusEvent);
}



void WINAPI Crawler::runParsingRobotsSendingStatus(const char* urlLink)
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


void WINAPI Crawler::twoSecondPrint()
{
	while (WaitForSingleObject(this->statusEvent, 2000) == true)
	{
		printf("[%3d] %4d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n",
			(double)(clock() - this->startTimer) / CLOCKS_PER_SEC,
			this->numberSizePendingQueue,
			this->parserHelper->numberExtractedURL,
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

		// pass the status on to the next person
		ResetEvent(statusEvent);
	}
}
void Crawler::finalPrint()
{

	double totalTimeElapsed = (double)((double)clock() - this->startTimer) / (double)CLOCKS_PER_SEC;

	// Output final stats
	printf("Extracted %d URLs @ %d/s\n", this->parserHelper->numberExtractedURL, (int)( this->parserHelper->numberExtractedURL / totalTimeElapsed));
	printf("Looked up %d DNS names @ %d/s\n", this->parserHelper->numberUniqueHost, (int)(this->parserHelper->numberDnsLookup / totalTimeElapsed));
	printf("Attempted %d robots @ %d/s\n", this->parserHelper->numberIpUnique, (int)(this->parserHelper->numberRobotPass / totalTimeElapsed));
	printf("Crawled %d pages @ %d/s (%.2f MB)\n", this->parserHelper->numberSuccessfullyCrawled, (int)(this->parserHelper->numberSuccessfullyCrawled / totalTimeElapsed), (double)(this->totalBytes / (double)1048576));
	printf("Parsed %d links @ %d/s\n", this->parserHelper->numberTotalLinks, (int)(this->parserHelper->numberTotalLinks / totalTimeElapsed));
	printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n", this->parserHelper->http200, this->parserHelper->http300, this->parserHelper->http400, this->parserHelper->http500, this->parserHelper->httpXXX);



}

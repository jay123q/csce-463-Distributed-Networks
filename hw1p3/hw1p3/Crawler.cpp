#include "pch.h"
#include "Crawler.h"




Crawler::Crawler()
{

	//std::string filename("Debug/100url.txt");
	// cout << " in consutruct check \n";
	// this->q = parserHelper->parseTXTFile(this->crawlerFileName);
	// int stall = 0;
	
	int numberThread = 0;

	this->bytesDownloadedInBatch = 0.0;
	this->pagesDownloadedInBatch = 0.0;
	this->startTimer = 0.0;
	this->totalBytes = 0.0;
	this->totalPages = 0.0;
	this->parserHelper = new parsedHtml();
	this->parserHelper->resetParser();
	this->parserHelper->setNumbersForCrawling();
	// cout << " check 1 crawler.cpp 1 \n";

	InitializeCriticalSection(&(this->threadQueueLock));
	InitializeCriticalSection(&(this->editQueueLink));
	// InitializeCriticalSection(&statusCheckLock);

	// cout << " check 2 crawler.cpp 1 \n";


	InitializeCriticalSection(&(this->parserHelper->bitHandlingCheckLock));
	InitializeCriticalSection(&(this->parserHelper->urlCheckLock)); 
	InitializeCriticalSection(&(this->parserHelper->dnsCheckLock));  
	InitializeCriticalSection(&(this->parserHelper->ipCheckLock));   
	InitializeCriticalSection(&(this->parserHelper->statusCheckMux));
	InitializeCriticalSection(&(this->parserHelper->robotCheckLock));
	InitializeCriticalSection(&(this->parserHelper->hostCheckUnique));
	InitializeCriticalSection(&(this->parserHelper->extractUrlLock));
	InitializeCriticalSection(&(this->parserHelper->linkCkeckLock));
	// EnterCriticalSection(&CriticalSection);

	// cout << " check 3 crawler.cpp 1 \n";

	// LeaveCriticalSection(&CriticalSection);
	this->statusEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	// cout << " check 3 crawler.cpp 1 \n";


}

Crawler::~Crawler()
{
	// DeleteCriticalSection(&CriticalSection);

	DeleteCriticalSection(&threadQueueLock);
	DeleteCriticalSection(&editQueueLink);
	// DeleteCriticalSection(&statusCheckLock);

	DeleteCriticalSection(&(this->parserHelper->urlCheckLock));
	DeleteCriticalSection(&(this->parserHelper->extractUrlLock));
	DeleteCriticalSection(&(this->parserHelper->dnsCheckLock));
	DeleteCriticalSection(&(this->parserHelper->ipCheckLock));
	DeleteCriticalSection(&(this->parserHelper->statusCheckMux));
	DeleteCriticalSection(&(this->parserHelper->robotCheckLock));
	DeleteCriticalSection(&(this->parserHelper->hostCheckUnique));
	DeleteCriticalSection(&(this->parserHelper->linkCkeckLock));
	DeleteCriticalSection(&(this->parserHelper->bitHandlingCheckLock));
	
}







DWORD Crawler::runParsingRobotsSendingStatus()
{

	// potentially have a var to set true when fasle 
	// cout << " front is " << this->q.front() << std::endl;
	EnterCriticalSection(&(this->editQueueLink));
	bool isEmpty = this->q.empty();
	if (isEmpty != true)
	{
		//const char* urlLink = this->q.front().c_str();

		// error here

		std::string urlLink(q.front().c_str());
		// const char* urlLink = q.front().c_str();
		q.pop();
		this->numberThread--;



		// cout << " size of  main  parser " << sizeof(parser) << std::endl;
		// parser.resetParser();
		this->parserHelper->webSocket = new Socket();
		// parser->parseString(urlLink);
		LeaveCriticalSection(&(this->editQueueLink));



		// new mutex egage

		bool urlPass = parserHelper->urlCheck(urlLink.c_str(), parserHelper->printPathQueryFragment());


		parserHelper->webSocket->robots = true;
		// parser->webSocket->printDNStiming = true;
		// 
		if (urlPass != true)
		{
			LeaveCriticalSection(&(this->editQueueLink));
			// cout << " URL FAILED moving on to next url move this main.cpp \n";
			parserHelper->resetParser();
			return 0;
		}

		// EnterCriticalSection(&(this->))
		// this->parserHelper->numberExtractedURL++;


		// taking a copy of the server
		parserHelper->transferSetServer(parserHelper->webSocket->getServer());
		// cout << " the socket is " << parser->webSocket->sock << std::endl;

		// lock here??

		bool robotPass = parserHelper->RobotSendRead();
		if (robotPass != true)
		{
			//	cout << "ROBOT FAILED  sending to robots failed in main, moving on to next \n";
			parserHelper->resetParser();
			return 0;
		}

		parserHelper->webSocket = new Socket();
		parserHelper->webSocket->setServer(parserHelper->serverParserTemp);
		bool sendPass = parserHelper->ReconnectHostSend();
		if (sendPass != true)
		{
			//	cout << "RECONNECT HOST FAILED sending the request has failed in main, could not be a issue, moving to next remove me \n";
			parserHelper->resetParser();
			return 0;
		}

		// unlock here
		// 
		// 
		// cout << " finished the  main function contiune running 42 \n";
		// parser->webSocket->~Socket();
		parserHelper->resetParser();



		return 0;


	}
	else
	{
		// queue is empty
		return 0;
	}


}


DWORD Crawler::twoSecondPrint()
{
	while (WaitForSingleObject(this->statusEvent, 2000) == true)
	{
		printf("[%3d] %4d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n",
			(double)(clock() - this->startTimer) / CLOCKS_PER_SEC,
			this->numberThread,
			(int) this->q.size(),
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
		return 0;
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

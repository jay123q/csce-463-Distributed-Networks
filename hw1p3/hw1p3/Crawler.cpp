#include "pch.h"
#include "Crawler.h"





Crawler::Crawler()
{

	//std::string filename("Debug/100url.txt");
	// cout << " in consutruct check \n";
	// this->q = parserHelper->parseTXTFile(this->crawlerFileName);
	// int stall = 0;
	this->startTimer = clock();
	this->numberThread = 0;
	this->bytesDownloadedInBatch = 0.0;
	this->pagesDownloadedInBatch = 0.0;
	this->totalBytes = 0.0;
	this->totalPages = 0.0;
	this->parserStats = new parsedHtml();
	this->parserStats->setNumbersForCrawling();
	// cout << " check 1 crawler.cpp 1 \n";
	InitializeCriticalSection(&(this->threadQueueLock));
	InitializeCriticalSection(&(this->editQueueLink));
	//InitializeCriticalSection(&(this->genericSyntaxLock));
	// InitializeCriticalSection(&statusCheckLock);

	// cout << " check 2 crawler.cpp 1 \n";


	InitializeCriticalSection(&(this->genericSyntaxLock));

	// EnterCriticalSection(&CriticalSection);

	// cout << " check 3 crawler.cpp 1 \n";

	// LeaveCriticalSection(&CriticalSection);
	this->statusEvent = CreateEvent(NULL, true, false, NULL);
	// cout << " check 3 crawler.cpp 1 \n";


}

Crawler::~Crawler()
{
	// DeleteCriticalSection(&CriticalSection);

	DeleteCriticalSection(&threadQueueLock);
	DeleteCriticalSection(&editQueueLink);
	DeleteCriticalSection(&(this->genericSyntaxLock));
	// DeleteCriticalSection(&statusCheckLock);


}







DWORD Crawler::runParsingRobotsSendingStatus()
{
	parsedHtml parserHelper;
	parserHelper.resetParser();
	parserHelper.webSocket->robots = true;

	while (true)
	{
	
		EnterCriticalSection(&(this->editQueueLink));
		// cout << " length " << q.size() << std::endl;
		// potentially have a var to set true when fasle 
		// cout << " front is " << this->q.front() << std::endl;
		//const char* urlLink = this->q.front().c_str();

		// error here
		if(q.size() == 0)
		{
			this->numberThread--;
			LeaveCriticalSection(&(this->editQueueLink));
			break;
		}
		std::string urlLink(q.front().c_str());
		// const char* urlLink = q.front().c_str();
		if (urlLink[0] == '\0')
		{
			LeaveCriticalSection(&(this->editQueueLink));
			// url pop failed
			continue;
		}
		q.pop();
		// cout << " size of  main  parser " << sizeof(parser) << std::endl;
		// parser.resetParser();
		parserStats->numberExtractedURL--;
		// parserHelper.webSocket = new Socket();
		LeaveCriticalSection(&(this->editQueueLink));
		// parser->parseString(urlLink);
		// LeaveCriticalSection(&(this->editQueueLink));




		// new mutex egage

		// parser->webSocket->printDNStiming = true;
		// 
		bool parseCheck = parserHelper.parseString(urlLink);
		parserHelper.urlLink = urlLink.c_str();
		if (parseCheck == false)
		{
			// LeaveCriticalSection(&(this->extractUrlLock));
			// return false;
			continue;
		}




	  //  cout << "\t   Checking host uniqueness... ";

		EnterCriticalSection(&(parserStats->hostCheckUnique));
		auto resultHostCheck = this->seenHosts.insert(parserHelper.host.c_str());
		if (resultHostCheck.second != true)
		{ // duplicate host
			LeaveCriticalSection(&(parserStats->hostCheckUnique));


			//     cout << "failed" << '\n';
			continue;
			// return false;
		}


		parserStats->numberUniqueHost++;
		LeaveCriticalSection(&(parserStats->hostCheckUnique));
		;

		//     bool DNSpass = this->webSocket->DNSCheck(host.c_str());


		if (parserHelper.webSocket->DNSCheck(parserHelper.host) != true)
		{
			continue;
			// return false;
		}
		// this->ip = server.sin_addr;

		EnterCriticalSection(&(parserStats->dnsCheckLock));
		parserStats->numberDnsLookup++; // increment dns checkk
		LeaveCriticalSection(&(parserStats->dnsCheckLock));

		/*
		// cout << " size of seen ips " << seenIPs.size() << std::endl;
			//cout << " ip print is f%" << x << std::endl;
		printf("hex : %x host : %s \n", x, host.c_str());
		if (x == 0xd76fea9)
		{
			int xa = 0;
		}
		*/
		EnterCriticalSection(&(parserStats->ipCheckLock));
		int prevInsert = this->seenIPs.size();
	
		// DWORD x = *(DWORD*)parserHelper.webSocket->getRemote()->h_addr;
		seenIPs.insert(parserHelper.webSocket->getIp());
		if (prevInsert != seenIPs.size())
		{ // duplicate host
	   /// DWORD x = inet_addr(inet_ntoa(server.sin_addr));
			parserStats->numberIpUnique++; // increment IP unique
			LeaveCriticalSection(&(parserStats->ipCheckLock));
		}
		else
		{

			LeaveCriticalSection(&(parserStats->ipCheckLock));
			// return false;
			continue;
		}






		// EnterCriticalSection(&(this->))
		// EnterCriticalSection(&(this->genericSyntaxLock));
		// taking a copy of the server
		parserHelper.transferSetServer(parserHelper.webSocket->getServer());
		// cout << " the socket is " << parser->webSocket->sock << std::endl;
		// cout << " count unique " << inet_addr(inet_ntoa(parserHelper->webSocket->getServer().sin_addr)) << " std " << std::endl;

		// lock here??
		// LeaveCriticalSection(&(this->genericSyntaxLock));



		if (parserHelper.RobotSendRead(parserHelper.port) != true)
		{
			//	cout << "ROBOT FAILED  sending to robots failed in main, moving on to next \n";
			// I really ddont like this here if its shared memroy resetting the parser like this should break soething 
		EnterCriticalSection(&(this->genericSyntaxLock));
			parserHelper.resetParser();
			delete parserHelper.webSocket;
		LeaveCriticalSection(&(this->genericSyntaxLock));
			continue;

			// return 0;
		}

		EnterCriticalSection(&(this->genericSyntaxLock));
		parserHelper.webSocket = new Socket();
		parserHelper.webSocket->setServer(parserHelper.serverParserTemp);
		LeaveCriticalSection(&(this->genericSyntaxLock));


		bool sendPass = parserHelper.ReconnectHostSend(parserHelper.port);
		if (sendPass != true)
		{
			//	cout << "RECONNECT HOST FAILED sending the request has failed in main, could not be a issue, moving to next remove me \n";
		EnterCriticalSection(&(this->genericSyntaxLock));
			parserHelper.resetParser();
		LeaveCriticalSection(&(this->genericSyntaxLock));
			continue;
			//	return 0;
		}

		// unlock here
		// 
		// 
		// cout << " finished the  main function contiune running 42 \n";
		// parser->webSocket->~Socket();
		EnterCriticalSection(&(this->genericSyntaxLock));
		parserHelper.resetParser();
		LeaveCriticalSection(&(this->genericSyntaxLock));



		// return 0;


	}
	// queue is empty
	// cout << " queue \n";
	return 0;



}


DWORD Crawler::twoSecondPrint()
{
	bool printMe = false;
	while ((WaitForSingleObject(this->statusEvent, 2000) == WAIT_TIMEOUT) || !printMe)
	{
		printMe = true;
		printf("[%3d] %4d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n",
			(int)(double)(clock() - this->startTimer) / CLOCKS_PER_SEC,
			this->numberThread,
			(int)this->q.size(),
			this->parserStats->numberExtractedURL,
			this->parserStats->numberUniqueHost,
			this->parserStats->numberDnsLookup,
			this->parserStats->numberIpUnique,
			this->parserStats->numberRobotPass,
			this->parserStats->numberSuccessfullyCrawled, // this is non robbot pages
			this->parserStats->numberTotalLinks / 1000);

		this->bytesDownloadedInBatch = (this->parserStats->newNumberBytesInBatch - this->totalBytes) / (double)((clock() - this->startTimer) / CLOCKS_PER_SEC);
		this->pagesDownloadedInBatch = (this->parserStats->newNumberPagesInBatch - this->totalPages) / (double)((clock() - this->startTimer) / CLOCKS_PER_SEC);

		// Output crawling information
		printf("\t*** crawling %.1f pps @ %.1f Mbps\n", this->pagesDownloadedInBatch, this->bytesDownloadedInBatch / double(125000));
		this->totalBytes = this->parserStats->newNumberBytesInBatch;
		this->totalPages = this->parserStats->newNumberPagesInBatch;
		this->parserStats->newNumberBytesInBatch = 0;
		this->parserStats->newNumberPagesInBatch = 0;

		// pass the status on to the next person
		// ResetEvent(statusEvent);
		return 0;
	}
}
void Crawler::finalPrint()
{

	double totalTimeElapsed = (double)((double)clock() - this->startTimer) / (double)CLOCKS_PER_SEC;

	// Output final stats
	printf("Extracted %d URLs @ %d/s\n", this->parserStats->totalExtractedNoSub, (int)(this->parserStats->totalExtractedNoSub / totalTimeElapsed));
	printf("Looked up %d DNS names @ %d/s\n", this->parserStats->numberUniqueHost, (int)(this->parserStats->numberDnsLookup / totalTimeElapsed));
	printf("Attempted %d robots @ %d/s\n", this->parserStats->numberRobotPass, (int)(this->parserStats->numberRobotPass / totalTimeElapsed));
	printf("Crawled %d pages @ %d/s (%.2f MB)\n", this->parserStats->numberSuccessfullyCrawled, (int)(this->parserStats->numberSuccessfullyCrawled / totalTimeElapsed), (double)(this->totalBytes / (double)1048576));
	printf("Parsed %d links @ %d/s\n", this->parserStats->numberTotalLinks, (int)(this->parserStats->numberTotalLinks / totalTimeElapsed));
	printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n", this->parserStats->http200, this->parserStats->http300, this->parserStats->http400, this->parserStats->http500, this->parserStats->httpXXX);

	printf(" DEBUGGING! number of ip addresses found %d \n", this->parserStats->numberIpUnique);

}
// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include "parsedHTML.h"
using namespace std;

parsedHtml::parsedHtml()
{
    this->resetParser();
    this->setNumbersForCrawling();
}

void parsedHtml::resetParser(void)
{
    this->port = 80;
    this->host = '\0';
    this->fragment = '\0';
    this->query = '\0';
    this->path = '\0';
    this->wholeLink = '\0';
    this->total = '\0';
    this->httpStatus = '\0';
    //this->webSocket->~Socket();
    // this->webSocket=new Socket();
    // this->readFileBuf[0] = '\0';
    this->webSocket->~Socket();
    this->intFileSize = 0;

}

void parsedHtml::setNumbersForCrawling()
{
    this->totalExtractedNoSub = 0;
    this->newNumberBytesInBatch = 0;
    this->newNumberPagesInBatch = 0;
    this->numberExtractedURL = 0; // inside file read
    this->numberUniqueHost = 0; // inside the first check
    this->numberDnsLookup = 0; // inside the first check
    this->numberIpUnique = 0; // insidde the first check 
    this->numberRobotPass = 0; // inside the robots function, passed robots 
    this->numberSuccessfullyCrawled = 0; //  full http inside of the reconnect and resent function
    this->numberTotalLinks = 0; // inside of reconnecct and send parser
    this->http200 = 0;
    this->http300 = 0;
    this->http400 = 0;
    this->http500 = 0;
    this->httpXXX = 0;

}


void parsedHtml::runOnce(const char* urlLink)
{
    // cout << " size of  main  parser " << sizeof(parser) << std::endl;
    // parser.resetParser();
    this->webSocket = new Socket();
    // parser->parseString(urlLink);
    bool urlPass = this->urlCheck(urlLink, this->printPathQueryFragment());
    this->webSocket->robots = true;
    // parser->webSocket->printDNStiming = true;
    // 
    if (urlPass != true)
    {
        // cout << " URL FAILED moving on to next url move this main.cpp \n";
        this->resetParser();
        return;
    }

    // taking a copy of the server
    this->transferSetServer(this->webSocket->getServer());
    // cout << " the socket is " << parser->webSocket->sock << std::endl;

    // lock here??

    bool robotPass = this->RobotSendRead();
    if (robotPass != true)
    {
        //	cout << "ROBOT FAILED  sending to robots failed in main, moving on to next \n";
        this->resetParser();
        return;
    }

    // unlock here ??

    // parser->webSocket->printDNStiming = false;

    // lock here


    this->webSocket = new Socket();
    this->webSocket->setServer(this->serverParserTemp);
    bool sendPass = this->ReconnectHostSend();
    if (sendPass != true)
    {
        //	cout << "RECONNECT HOST FAILED sending the request has failed in main, could not be a issue, moving to next remove me \n";
        this->resetParser();
        return;
    }

    // unlock here
    // 
    // 
    // cout << " finished the  main function contiune running 42 \n";
    // parser->webSocket->~Socket();
    this->resetParser();



}




bool parsedHtml::parseString(string link) {

    // cout << "URL: " << link << std::endl;
    // cout << "\t   Parsing URL... ";

    this->wholeLink = link.c_str();
    if (link.substr(0, 7) != "http://") {
        //    cout << "failed with invalid scheme " << std::endl;
        return false;
    }

    string hostStart = link.substr(7);

    size_t fragmentIndex = hostStart.find_first_of('#');
    if (fragmentIndex != string::npos) {
        this->fragment = hostStart.substr(fragmentIndex + 1);
        hostStart = hostStart.substr(0, fragmentIndex);
    }

    size_t queryIndex = hostStart.find_first_of('?');
    if (queryIndex != string::npos) {
        this->query = hostStart.substr(queryIndex + 1);
        hostStart = hostStart.substr(0, queryIndex);
    }

    size_t pathIndex = hostStart.find_first_of('/');
    if (pathIndex != string::npos) {
        this->path = hostStart.substr(pathIndex);
        hostStart = hostStart.substr(0, pathIndex);
    }
    else {
        this->path = "/";
    }

    size_t portIndex = hostStart.find_first_of(':');
    if (portIndex != string::npos) {
        //  cout << " do i exist here " << portIndex << " host length " << strlen(hostStart.c_str()) - 1 << std::endl;

        if (portIndex == strlen(hostStart.c_str()) - 1) {
            this->port = 80;
            // cout << "failed with invalid port" << std::endl;
           // return false;
        }
        else
        {

            this->port = atoi(hostStart.substr(portIndex + 1).c_str());
            if (port <= 0) {
                //  cout << "failed with invalid port" << std::endl;
                return false;
            }
        }
        this->host = hostStart.substr(0, portIndex);
        //  cout << " host " << this->host << std::endl;
    }
    else {
        this->port = 80;
        this->host = hostStart;
    }

    // cout <<" host " << this->host << " the port is " << this->port << " path is " << this->path << " query is " << this->query << " fragment is  " << this->fragment << std::endl;
     //this->host = hostStart;
    return true;

}

void parsedHtml::generateGETrequestToSend(void)
{

    // something to note for future semesters here, theres something odd with the strings and how they work in the send
    // if this->path + this->query are used it will throw a error saying bad webpage, however if the combined function is called
    // which is literally this->path + this->query, then there is no error
    // this is nonsneical, and took me a day to debug
    // 
    // this function is to make the request to send to connect on the socket
   // std::string getRequest = "GET ";
  //  cout << " get request |" << getRequest << "| \n";
  //  cout << " path " << printPathQueryFragment() << std::endl;
   // this->total =  "GET / HTTP/1.1\r\nUser-agent: JoshTamuCrawler/1.1\r\nHost: tamu.edu\r\nConnection: close\r\n\r\n"; // CORRECT
    this->total = "GET " + printPathQueryFragment() + " HTTP/1.0\r\nUser-agent: JoshTamuCrawler/1.3\r\nHost: " + this->host + "\r\nConnection: close\r\n\r\n";
    //  cout << " total \n" << this->total << std::endl;
     // this->total = "GET /IRL7 HTTP/1.0\r\nUser-agent: JoshTamuCrawler/1.1\r\nHost: s2.irl.cs.tamu.edu\r\nConnection: close\r\n\r\n";
}

void parsedHtml::generateHEADrequestToSend(void)
{
    this->total = "HEAD /robots.txt HTTP/1.0\r\nUser-agent: JoshTamuCrawler/1.3\r\nHost: " + this->host + "\r\nConnection: close\r\n\r\n";
}
queue<string> parsedHtml::parseTXTFile(std::string filename)
{
    // cout << " in parser \n";
    ifstream file(filename, ios::binary | ios::in);
    std::string line;
    std::queue <std::string> queueTotal;
    this->intFileSize = 0;
    while (!file.eof())
    {
        getline(file, line);
        this->intFileSize += strlen(line.c_str());
        // cout << " the line is " << line << std::endl;
       // cout << " push the file " << line << std::endl;
        queueTotal.push(line);
    }

    // EnterCriticalSection(&(this->extractUrlLock));
    this->numberExtractedURL = queueTotal.size();
    this->totalExtractedNoSub = queueTotal.size();
    // LeaveCriticalSection(&(this->extractUrlLock));


    return queueTotal;
}






bool parsedHtml::RobotSendRead(void)
{
    // cout << "\t   Connecting on robots... ";
    EnterCriticalSection(&(this->genericSyntaxLock));
    bool connect = this->webSocket->Connect(this->port);
    if (connect != true)
    {

        LeaveCriticalSection(&(this->genericSyntaxLock));
        return false;
    }
    this->generateHEADrequestToSend();
    //  cout << "request path | Robots " << this->total << "| \n";
    bool socketCheck = this->webSocket->Send(this->total, this->host);
    LeaveCriticalSection(&(this->genericSyntaxLock));

    if (socketCheck)
    {
        // now try to read
        if (this->webSocket->Read()) // after read hit a null terminaotr! to speed up buffer, add one like at curpos?
        {
            this->webSocket->closeSocket(); // maybe move this into read? 
            // so now the html should return the buffer soo
            WSACleanup();

            // bytes must be > 200
            EnterCriticalSection(&(this->bitHandlingCheckLock));


            if (this->webSocket->getCurPos() == 0)
            {
                LeaveCriticalSection(&(this->bitHandlingCheckLock));
                return false;
            }
            this->newNumberBytesInBatch += strlen(this->webSocket->printBuf()); // number of bytes recieved
            this->newNumberPagesInBatch++; // increment the number of pages recieved


            string status(this->webSocket->printBuf());

            const unsigned int statusCode = stoi(status.substr(9.3).c_str());
            LeaveCriticalSection(&(this->bitHandlingCheckLock));

            /*
            try
            {
              const unsigned int statusCode = stoi(status.substr(9.3).c_str());
             LeaveCriticalSection(&(this->bitHandlingCheckLock));

            }
            catch (const std::exception&)
            {
             LeaveCriticalSection(&(this->bitHandlingCheckLock));
             return false;
            }

            */



            //   cout << "\t   Verifying header... ";

            if (statusCode >= 400)
            {
                // robots allowed
                // parse header now

             //   cout << "status code " << statusCode << std::endl;

                // int httpPointer = webSocket->printBuf().find("HTTP/");
               // cout << " parsing HTML ROBOT CHECK PASSED REMOVE ME LATER \n";


                EnterCriticalSection(&(this->robotCheckLock));
                this->numberRobotPass++; // increment robots
                LeaveCriticalSection(&(this->robotCheckLock));



                return true; // go back to the main parsing url

            }
            else if (statusCode < 400 && statusCode >= 200)
            {
                EnterCriticalSection(&(this->genericSyntaxLock));
               // cout << " file url of failed codes " << this->urlLink << std::endl;
                // cout << "status code " << statusCode << std::endl;
                this->webSocket->robots = false;
                LeaveCriticalSection(&(this->genericSyntaxLock));
                return false;
            }
            //std::string resultButString(result);
            //int findHeader = resultButString.find("\r\n\r\n");
            //std::string header = resultButString.substr(0, findHeader);
            //cout << header;


        }
    }

    return false;

}


bool parsedHtml::ReconnectHostSend(void)
{
    // cout << '\t' << " * Connecting on page... ";
    EnterCriticalSection(&(this->genericSyntaxLock)); // could move this to after
    bool connection = this->webSocket->Connect(this->port);
    if (connection != true)
    {
        //   cout << " reconnection failed in reconnectHOSTSEND parsed HTML.CPP \n";
        LeaveCriticalSection(&(this->genericSyntaxLock));
        return false;
    }

    this->generateGETrequestToSend();

    // cout << "request path | Send " << this->total << "| \n";
    bool socketCheck = this->webSocket->Send(this->total, this->host);
    LeaveCriticalSection(&(this->genericSyntaxLock));

    if (socketCheck)
    {
        // now try to read
        if (this->webSocket->Read())
        {
            // HERE && strlen(webSocket->printBuf().c_str()) < MAX_REQUEST_LEN ???
            this->webSocket->closeSocket(); // maybe move this into read? 
            // so now the html should return the buffer soo
           // cout << " web socket in send GET " << this->webSocket->sock << std::endl;
            WSACleanup();
            EnterCriticalSection(&(this->genericSyntaxLock));
            if (this->webSocket->getCurPos() == 0)
            {
                LeaveCriticalSection(&(this->bitHandlingCheckLock));
                return false;
            }
            string status(this->webSocket->printBuf());

            const unsigned int statusCode = stoi(status.substr(9.3).c_str());
            LeaveCriticalSection(&(this->genericSyntaxLock));

            if (statusCode > 199 && statusCode < 300)
            {
                EnterCriticalSection(&(this->statusCheckMux));
                this->http200++; // incrememnt http
                LeaveCriticalSection(&(this->statusCheckMux));


                EnterCriticalSection(&(this->urlCheckLock));
                this->numberSuccessfullyCrawled++; // full htpp obtained
                LeaveCriticalSection(&(this->urlCheckLock));



                clock_t start = clock();
                clock_t finish = clock();
                //   cout << "\t + Parsing page... ";

                EnterCriticalSection(&(this->bitHandlingCheckLock));
                const char* result = this->webSocket->printBuf();
                string stringResult(result);
                const char* pastHeaderPtr = strchr(result, '\r\n\r\n');

                int bytes_recieved = this->webSocket->getCurPos();



                this->newNumberBytesInBatch += bytes_recieved; // number of bytes recieved
                this->newNumberPagesInBatch++; // increment the number of pages recieved
                LeaveCriticalSection(&(this->bitHandlingCheckLock));

                //  EnterCriticalSection(&(this->bitHandlingCheckLock));
                int bytes_header = pastHeaderPtr - result; // header bytes
                int bytes_file = bytes_recieved - bytes_header;
                //  EnterCriticalSection(&(this->bitHandlingCheckLock));


                   // int numberBytesToParse = htmlPointer - webSocket->getCurPos();
                   //asdf
                EnterCriticalSection(&(this->linkCkeckLock));
                int nLinks = 0;
                nLinks = this->webSocket->parser((char*)result, bytes_recieved, (char *) wholeLink.c_str() );




                this->numberTotalLinks += nLinks; // nlinks for the stats page
                LeaveCriticalSection(&(this->linkCkeckLock));



                //   finish = clock();
                //   double timer = (double)(finish - start) / CLOCKS_PER_SEC;
                 //  printf("done in %.1f ms with %d links\n", timer * 1000, nLinks);
                return true;
                // printf("=======================================================\n");

                 // int httpPointer = webSocket->printBuf().find("HTTP/");
            }
            else if (statusCode > 299 && statusCode < 400)
            {
                EnterCriticalSection(&(this->statusCheckMux));
                this->http300++;
                LeaveCriticalSection(&(this->statusCheckMux));
                return false;
            }
            else if (statusCode > 399 && statusCode < 500)
            {
                EnterCriticalSection(&(this->statusCheckMux));
                this->http400++;
                return false;
                LeaveCriticalSection(&(this->statusCheckMux));

            }
            else if (statusCode > 499 && statusCode < 600)
            {
                EnterCriticalSection(&(this->statusCheckMux));
                this->http500++;
                LeaveCriticalSection(&(this->statusCheckMux));
                return false;

            }
            else
            {
                EnterCriticalSection(&(this->statusCheckMux));
                this->httpXXX++;
                LeaveCriticalSection(&(this->statusCheckMux));
                return false;

            }
        }
    }

    return false;
}

bool parsedHtml::urlCheck(std::string link, string pathQueryFragment)
{
    EnterCriticalSection(&(this->extractUrlLock));
    bool parseCheck = parseString(link);
    this->urlLink = link.c_str();
    if (parseCheck == false)
    {
        LeaveCriticalSection(&(this->extractUrlLock));
        return false;
    }

    this->numberExtractedURL--; // remove a url from the link
        LeaveCriticalSection(&(this->extractUrlLock));


    //  cout << "\t   Checking host uniqueness... ";

     EnterCriticalSection(&(this->hostCheckUnique));
    auto resultHostCheck = seenHosts.insert(host.c_str());
    if (resultHostCheck.second != true)
    { // duplicate host
       LeaveCriticalSection(&(this->hostCheckUnique));


        //     cout << "failed" << '\n';
        return false;
    }


    this->numberUniqueHost++;
     LeaveCriticalSection(&(this->hostCheckUnique));
;

 //     bool DNSpass = this->webSocket->DNSCheck(host.c_str());
     EnterCriticalSection(&(this->dnsCheckLock));
    
    if (this->webSocket->DNSCheck(this->host.c_str()) != true)
    {
   //     delete webSocket;
     LeaveCriticalSection(&(this->dnsCheckLock));
        return false;
    }
    this->numberDnsLookup++; // increment dns checkk

     LeaveCriticalSection(&(this->dnsCheckLock));


     
   EnterCriticalSection(&(this->ipCheckLock));
    // below caused a error if INET_ADDR isnt there odd
     int prevInsert = this->seenIPs.size();
     // cout << " size of seen ips " << seenIPs.size() << std::endl;
    DWORD x = inet_addr(inet_ntoa(webSocket->getServer().sin_addr));
    seenIPs.insert(x);
  //  cout << " dword x " << inet_addr(inet_ntoa(this->webSocket->getServer().sin_addr)) << std::endl;
    // if a valid IP, directly drop its binary version into sin_addr
    if (prevInsert != seenIPs.size())
    { // duplicate host
        this->numberIpUnique++; // increment IP unique
        LeaveCriticalSection(&(this->ipCheckLock));

    }
    else
    {

        LeaveCriticalSection(&(this->ipCheckLock));
        return false;
    }

     
     

    //        cout << "passed \n";
            // cout << "passed NO UNIUQE IDS FOUND IN FIRST CHECK  URL CHECK, REMOVE ME LATER \n";




   // LeaveCriticalSection(&(this->extractUrlLock));

    return true;
}
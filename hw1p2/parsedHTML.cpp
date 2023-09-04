#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include "parsedHTML.h"
using namespace std;


void parsedHtml::resetParser(void)
{
    this->port = 80;
    this->host = '\0';
    this->fragment = '\0';
    this->query = '\0';
    this->path = '\0';
    this->wholeLink = '\0';
    this->total = '\0';
    this->httpStatus='\0';
    // this->webSocket->~Socket();
    // this->webSocket=new Socket();
    // this->readFileBuf[0] = '\0';
    this->webSocket->~Socket();
    this->intFileSize=0;
    
}

void parsedHtml::parseString(const char  * link) {

    this->wholeLink = link;
    std::vector<std::string> parsedValue;

    const char* hostStart = link + 7; // Skip "://"
  //  cout << "found path is " << pathStart << endl;

 
    const char* fragmentStart = strchr (link, '#');
    if (fragmentStart != nullptr) {
        std::string fragment(fragmentStart + 1); // Skip the '#'
        this->fragment = fragment;
        //std::string newLink(hostStart, fragmentStart);
        //const char * link = newLink.c_str();
       // cout << " removed fragment " << fragment << std::endl;
    }
    else
    {
        this->fragment = '\0';
    }


    const char* queryStart = strchr(link, '?');
    if (queryStart !=  nullptr ) {
        const char* queryEnd = strchr(queryStart, '#');
        if (!queryEnd) {
            queryEnd = link + strlen(link);
        }
        std::string query(queryStart + 1, queryEnd - queryStart - 1); // Exclude '?' and '#'
        this->query = query;
      //  cout << " removed query " << query << std::endl;
    }
    else
    {
      //  parsedValue.push_back("no query");
        this->query = '\0';
    }

    // const char* host = strchr(link, '/');
    const char* pathStart = strchr(hostStart, '/');
    if (pathStart != nullptr ) 
    {
        // this logic has taken me so long to do
        // if I have to do this ever again, i will be penduluminng myself off the highest bbuilding
        const char* pathEndQuery = strchr(hostStart, '?'); // check query
        const char* pathEndFragment = strchr(hostStart, '#'); // check fragent

        if (pathEndQuery != nullptr)
        {
            std::string path(pathStart + 1, pathEndQuery - pathStart - 1);
            // parsedValue.push_back(path);
            this->path = path;
        }
        else if (pathEndFragment != nullptr) //check fragment
        {
            std::string path(pathStart + 1, pathEndFragment - pathStart - 1); // +1 removes forwrd, -1 removes backward
            // parsedValue.push_back(path);
            this->path = path;
        }
        else if (pathStart + 1 != nullptr)
        {
            this->path = pathStart;
        }
        else
        {
            //  parsedValue.push_back("single /");
            this->path = '/';
        }
    }
    else
    {
      //  parsedValue.push_back("single /");
        this->path = '/';
    }
   // cout << " new path " << parsedValue.at(parsedValue.size()-1) << endl;
    // this is because the link can have a : in the middle of the ppath
    
    const char* portStart = strchr(hostStart, ':');
    if (portStart != nullptr )
    {
        // check for malformed port
        const char* portMalFormed = strstr(hostStart, ":-");
        //std::str
       // int portCheck = hostStart.find_first_of(":");
        
        if (portMalFormed != nullptr)
        {
           // cout << " port is malformed parsedHtml.cpp " << std::endl;
            std::string portString("- negative port");
            parsedValue.push_back("- negative port");
        }

        try
        {

        }
        catch (const std::exception&)
        {

        }
        const char* portEndBackSlash = strchr(hostStart, '/');
        const char* portEndQuery = strchr(hostStart, '?');
        const char* portEndFragment = strchr(hostStart, '#');
        if (portEndBackSlash != nullptr)
        {
           // cout << "1" << std::endl;
            std::string port(portStart + 1, pathStart - portStart - 1); // remove  :, and /
            parsedValue.push_back(port);
        }
        else if (portEndQuery != nullptr)
        {
           // cout << "2" << std::endl;

            std::string port(portStart + 1, portEndQuery - portStart - 1); // remove  :, and ?
            parsedValue.push_back(port);
        }
        else if (portEndFragment != nullptr)
        {
         //   cout << "3" << std::endl;

   //         std::string port(portStart + 1, portEndFragment - portStart - 1); // remove  :, and #
            std::string port(portStart + 1, portEndFragment - portStart - 1); // remove  :, and #
            parsedValue.push_back(port);
        }
        //cout << " new port " << parsedValue.at(parsedValue.size() - 1) << endl;

    }
    else // empty port
    {
        //cout << "4" << std::endl;

        std::string port("80");
        parsedValue.push_back(port);
       // cout << " port is empty if this is spewing gargage " << port << " line 99  to fix " << std::endl;
    }

        
    if (portStart != nullptr)
    {
        //cout << " port check host " << std::endl;
        std::string host(hostStart, portStart - hostStart);
        //cout << " parsed Value " << parsedValue.size();


      //  cout << " port path check " << host << std::endl;
        this->host = host;
    }
    else if (pathStart != nullptr)
    {
        std::string host(hostStart, pathStart - hostStart);

       // parsedValue.push_back(host);
        this->host = host;
       // cout << " host path check " << host << std::endl;
    }
    else if (queryStart != nullptr)
    {
       // cout << " port check query " << std::endl;

        std::string host(hostStart, queryStart - hostStart);
       // parsedValue.push_back(host);
        this->host = host;
     //   cout << " query path check " << host << std::endl;

    }
    else if (fragmentStart != nullptr)
    {
        //cout << " port check fragment " << std::endl;

        std::string host(hostStart, fragmentStart - hostStart);
       // parsedValue.push_back(host);
        this->host = host;
     //   cout << " fragment path check " << host << std::endl;

    }
    else
    {
        std::string host(hostStart);
      //  parsedValue.push_back(host);
        this->host = host;
        // cout << " host print " << host << std::endl;
    }




//    cout << " 174 check " << std::endl;



    try
    {
        // if there is a -, or if the port is nullptr
        if ( strchr(parsedValue.at(0).c_str(), '-') !=  nullptr )
        {
            // if we found a negative port 
          //  cout << "negtaive port spotted parsed html 162 " << std::endl;
            this->port = 65536; // one above port range if its invalid or dne

        }
        else
        {
           // cout << " checking waht parsed value got " << parsedValue.at(0) << std::endl;
            this->port = stoi(parsedValue.at(0));
            if (this->port == 0)
            {
                this->port = 65536;
            }

        }

    }
    catch (...)
    {
        //cout << "setting port to 80, parsedhtml.cpp 173 " << port << std::endl;
        // else there is no port so it becomes 80
        this->port = 80; // one above port range if its invalid or dne
    }



}

void parsedHtml::generateGETrequestToSend( void )
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
    this->total = "GET " + printPathQueryFragment() + " HTTP/1.1\r\nUser-agent: JoshTamuCrawler/1.1\r\nHost: " + this->host + "\r\nConnection: close\r\n\r\n"; 
  //  cout << " total \n" << this->total << std::endl;
   // this->total = "GET /IRL7 HTTP/1.0\r\nUser-agent: JoshTamuCrawler/1.1\r\nHost: s2.irl.cs.tamu.edu\r\nConnection: close\r\n\r\n";
}

void parsedHtml::generateHEADrequestToSend(void)
{
this->total = "HEAD /robots.txt HTTP/1.1\r\nUser-agent: JoshTamuCrawler/1.2\r\nHost: " + this->host + "\r\nConnection: close\r\n\r\n"; 
}
vector<string> parsedHtml::parseTXTFile(std::string filename)
{
    ifstream file(filename, ios::binary | ios::in);
    std::string line;
    std::vector <std::string> vectorTotal;
    while (!file.eof())
    {
        getline(file, line);
        // cout << " the line is " << line << std::endl;
        vectorTotal.push_back(line);
    }
    return vectorTotal;
}

char * parsedHtml::parseTXTFileBROKEN( std::string filename )
{

    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    // process errors
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile failed with %d\n", GetLastError());
        
    }

    // get file size
    LARGE_INTEGER li;
    BOOL bRet = GetFileSizeEx(hFile, &li);
    // process errors
    if (bRet == 0)
    {
        printf("GetFileSizeEx error %d\n", GetLastError());
        
    }

    // read file into a buffer
    int fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
    DWORD bytesRead;
    // allocate buffer
    char* fileBuf = new char[fileSize];
    // read into the buffer
    bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
    // process errors
    if (bRet == 0 || bytesRead != fileSize)
    {
        printf("ReadFile failed with %d\n", GetLastError());
    
    }

    // done with the file
    CloseHandle(hFile);

    int delay = 0;

    // cout << " file  buff " << fileBuf << std::endl;
    std::string urlString(fileBuf);
    int newIndex = urlString.find_first_of('\n');
    // std::string url = urlString.substr(0, newIndex);
   // std::string url = "";
    cout << " the first is " << newIndex << std::endl;
    while (newIndex != std::string::npos)
    {
        // create vector

        std::string url = urlString.substr(0, newIndex + 1);
        cout << " the link is |" << url << "|  blank space " << std::endl;
        //continueRunning(&parser, url.c_str());
        // update string
       // cout << " the return link is " << urlString.substr(0, newIndex) << std::endl;
        //urlString[newIndex] = '\0';
        urlString = urlString.substr(newIndex + 1, strlen(fileBuf));
        // cout << " url string buffer " << urlString << std::endl;
        // find new index
       // cout << " modified string is " << urlString << std::endl;
        int newIndex = urlString.find_first_of("\n");
        if (newIndex == std::string::npos)
        {
            break;
        }

    }

    return fileBuf;

    }



    
 

bool parsedHtml::RobotSendRead(void)
{
    cout << "\t   Connecting on robots... ";

    bool connect = this->webSocket->Connect(this->port);
    if (connect != true)
    {

        return false;
    }
    this->generateHEADrequestToSend();
    bool socketCheck = this->webSocket->Send(this->total, this->host);

    if (socketCheck)
    {
        // now try to read
        if (this->webSocket->Read()) // after read hit a null terminaotr! to speed up buffer, add one like at curpos?
        {
            this->webSocket->closeSocket(); // maybe move this into read? 
            // so now the html should return the buffer soo
            WSACleanup();
            
            string status(this->webSocket->printBuf());
            const unsigned int statusCode = stoi(status.substr(9.3).c_str());
            cout << "\t   Verifying header... ";

            if (statusCode >= 400)
            {
                // robots allowed
                // parse header now

                cout << "status code " << statusCode << std::endl;

                // int httpPointer = webSocket->printBuf().find("HTTP/");
               // cout << " parsing HTML ROBOT CHECK PASSED REMOVE ME LATER \n";
                return true; // go back to the main parsing url

            }
            else if (statusCode < 400 && statusCode >= 200)
            {
                cout << "status code " << statusCode << std::endl;
                this->webSocket->robots = false;
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
    cout << '\t' << " * Connecting on page... ";
    bool connection = this->webSocket->Connect(this->port);
    if (connection != true)
    {
        cout << " reconnection failed in reconnectHOSTSEND parsed HTML.CPP \n";
        return false;
    }
    this->generateGETrequestToSend();


    bool socketCheck = this->webSocket->Send(this->total, this->host);

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

            const char* result = this->webSocket->printBuf();
            // cout << " print the result and junk \n " << webSocket->printBuf() << std::endl;
            // cout << " the result  is " << this->printBuf() << std::endl;
            string status(this->webSocket->printBuf());
            const unsigned int statusCode = stoi(status.substr(9.3).c_str());
            cout << "\t   Verifying header... ";
            string stringResult(result);
            // parse header now
            cout << "status code " << statusCode << std::endl;


            if (statusCode > 199 && statusCode < 300)
            {
                clock_t start = clock();
                clock_t finish = clock();
                cout << "\t + Parsing page... ";

                const char* pastHeaderPtr = strchr(result, '\r\n\r\n');

                int bytes_recieved = this->webSocket->getCurPos();
                int bytes_header = pastHeaderPtr - result; // header bytes
                int bytes_file = bytes_recieved - bytes_header;


                // int numberBytesToParse = htmlPointer - webSocket->getCurPos();
                //asdf
                int nLinks = 0;
                HTMLParserBase htmlLinkRipper;
                char* linkCounter = htmlLinkRipper.Parse((char*)stringResult.c_str(), bytes_file,
                    (char*)wholeLink.c_str(), strlen((char*)wholeLink.c_str()), &nLinks);  // 43


                finish = clock();
                double timer = (double)(finish - start) / CLOCKS_PER_SEC;
                printf("done in %.1f ms with %d links\n", timer * 1000, nLinks);
                return true;
               // printf("=======================================================\n");

                // int httpPointer = webSocket->printBuf().find("HTTP/");
            }
        }
    }
    // robots did not connect
    return false;
}

bool parsedHtml::urlCheck(std::string link, string pathQueryFragment)
            {

                cout << "URL: " << link << std::endl;
                cout << "\t   Parsing URL... ";
                const char* httpLinkCheck = strstr(link.c_str(), "://");

                if (httpLinkCheck == nullptr)
                {
                    //cout << " linkcheck HTTPS " << httpLinkCheck << std::endl;
                    cout << "failed with invalid scheme " << std::endl;

                    return false;
                }
                if (port == 65536)
                {
                    cout << " failed with invalid port " << std::endl;
                    return false;
                }
                cout << "host " << host << ", port " << port << ", request " << pathQueryFragment << std::endl;

                // chhecking host uniqueness



                cout << "\t   Checking host uniqueness... ";
                
                auto resultHostCheck = this->seenHosts.insert(host.c_str());
                if (resultHostCheck.second != true)
                { // duplicate host

                    cout << "failed" << '\n';
                    return false;
                }
                cout << "passed \n";
                // Socket* webSocket = new Socket();
                cout << '\t' << "   Doing DNS... ";
                bool DNSpass = this->webSocket->DNSCheck(host.c_str());

                if (DNSpass != true)
                {
                    // cout << "failed \n";
                     // cout << " parsed HTML DNS FAILED IN  URL CHECK 515 REMOVE LATER \n";
                    return false;
                }

               cout << "\t   Checking IP uniqueness... ";
               //  cout << " ip is " << inet_addr((inet_ntoa(this->webSocket->getServer().sin_addr))) << std::endl;
                 int insertCheckIps = seenIPs.size();

                 // below caused a error if INET_ADDR isnt there odd

                 this->seenIPs.insert(inet_addr(inet_ntoa(this->webSocket->getServer().sin_addr)));

                // if a valid IP, directly drop its binary version into sin_addr
                if (insertCheckIps == seenIPs.size())
                { // duplicate host

                    cout << "failed" << '\n';
                    return false;
                }
                
                cout << "passed \n";
                // cout << "passed NO UNIUQE IDS FOUND IN FIRST CHECK  URL CHECK, REMOVE ME LATER \n";

 
                return true;
            }
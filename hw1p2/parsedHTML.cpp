// Joshua Clapp
// csce 463 500
// Dr Loguinov
// fall 2023
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

bool parsedHtml::parseString(string link) {

    cout << "URL: " << link << std::endl;
    cout << "\t   Parsing URL... ";

    this->wholeLink = link.c_str();
    if (link.substr(0, 7) != "http://")
    {

        cout << "failed with invalid scheme " << std::endl;
        return false;
    }

    string hostStart = link.substr(7, strlen(link.c_str()));
    int fragmentIndex = hostStart.find_first_of('#');
    if (fragmentIndex != -1)
    {
        this->fragment = hostStart.substr(fragmentIndex, strlen(hostStart.c_str()) - 1);
        hostStart = hostStart.substr(0, fragmentIndex);

    }

    hostStart = hostStart.substr(0, hostStart.find_first_of('#'));

    int queryCheck = hostStart.find_first_of('?');
    if (queryCheck != -1)
    {
        this->query = hostStart.substr(queryCheck , strlen(hostStart.c_str() )-1);
       // cout << " query check " << query << std::endl;
        hostStart = hostStart.substr(0, queryCheck );
    }    
    
    int pathCheck = hostStart.find_first_of('/');
    if (pathCheck != -1)
    {
        this->path = hostStart.substr (pathCheck, strlen(hostStart.c_str()) - 1);
       // cout << " path check " << path << std::endl;
        hostStart = hostStart.substr(0, pathCheck );
    } 
    else
    {
        this->path = "/";
    }
    
    // string stringPort = hostStart.substr(0, hostStart.find_first_of(':'));
    int portIndex = hostStart.find_first_of(':');
    // cout << " port check is " << portIndex << " ost start " << strlen(hostStart.c_str()) -1 << std::endl;
    // cout << " the port Index is " << portIndex << std::endl;
    if (portIndex != -1)
    {
        if (portIndex == strlen(hostStart.c_str())-1)
        {
           //  cout << " strlen host " << strlen(hostStart.c_str()) - 1 << std::endl;
            // port doest exist
            cout << " failed with invalid port";
            return false;
            // this->port = 80;
            
        }
        else
        {

        

        this->port = atoi(hostStart.substr(portIndex +1, (int) strlen(hostStart.c_str()) -1).c_str());
        cout << " the port is " << this->port << std::endl;
            if (port <= 0)
            {
                cout << " failed with invalid port";
                return false;
            }
        }
        this->host = hostStart.substr(0, hostStart.find_first_of(':'));

    }
    else
    {
        
        this->port = 80;
        this->host = hostStart;
    }



    return true;
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
    this->total = "GET " + printPathQueryFragment() + " HTTP/1.1\r\nUser-agent: JoshTamuCrawler/1.2\r\nHost: " + this->host + "\r\nConnection: close\r\n\r\n"; 
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
    this->intFileSize = 0;
    while (!file.eof())
    {
        getline(file, line);
        this->intFileSize += strlen(line.c_str());
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
  //  cout << " the first is " << newIndex << std::endl;
    while (newIndex != std::string::npos)
    {
        // create vector

        std::string url = urlString.substr(0, newIndex + 1);
       // cout << " the link is |" << url << "|  blank space " << std::endl;
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
            
            // bytes must be > 200





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
                bool parseCheck = parseString(link);
                if(parseCheck == false )
                {
                    return false;
                }

                cout << "host " << host << ", port " << port << std::endl;

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
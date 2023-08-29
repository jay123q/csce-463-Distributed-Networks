#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include "parsedHTML.h"
using namespace std;
#include <string>
#include <vector>

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
    if (portStart != nullptr)
    {
        // check for malformed port
        const char* portMalFormed = strstr(hostStart, ":-");
        if (portMalFormed != nullptr)
        {
           // cout << " port is malformed parsedHtml.cpp " << std::endl;
            std::string portString("- negative port");
            parsedValue.push_back("- negative port");
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


   // cout << " 206 check " << std::endl;
    /*
    if (  strstr(parsedValue.at(2).c_str(), "single /") != nullptr )
    {
        cout << "parsed value " << strstr(parsedValue.at(2).c_str(), "single /")  << std::endl;
        this->path = '\0';
        // handle this in printer
       // cout << " check if the path is correct as it reads nothing parsedHtml.cpp " << this->path << std::endl;
    }
    else
    {
        this->path = parsedValue.at(2).c_str();
        cout << " path push " << this->path << std::endl;
    }


    cout << " 222 check " << std::endl;

    if (strstr(parsedValue.at(1).c_str(), "no query") != nullptr)
    {
        this->query = '\0';
        cout << " query is " << strstr(parsedValue.at(1).c_str(), "no query") << " pathh is " << this->query << std::endl;
    }
    else
    {

        this->query = parsedValue.at(1).c_str();
    }

    if (strstr(parsedValue.at(0).c_str(), "no fragment") != nullptr)
    {
        this->fragment = '\0';
        cout << " fragment is " << strstr(parsedValue.at(0).c_str(), "no fragment") << " pathh is " << this->fragment << std::endl;

    }
    else
    {
        this->fragment = parsedValue.at(0).c_str();
    }

    */

   // cout << " path final check is " << this->path << std::endl;

   

}

void parsedHtml::generateRequesttoSend( string request)
{

    // something to note for future semesters here, theres something odd with the strings and how they work in the send
    // if this->path + this->query are used it will throw a error saying bad webpage, however if the combined function is called
    // which is literally this->path + this->query, then there is no error
    // this is nonsneical, and took me a day to debug
    // 
    // this function is to make the request to send to connect on the socket
    std::string getRequest = "GET ";
  //  cout << " get request |" << getRequest << "| \n";
  //  cout << " path " << printPathQueryFragment() << std::endl;
    this->total =  "GET / HTTP/1.1\r\nUser-agent: JoshTamuCrawler/1.1\r\nHost: tamu.edu\r\nConnection: close\r\n\r\n"; // CORRECT
    this->total = getRequest + printPathQueryFragment() + " HTTP/1.1\r\nUser-agent: JoshTamuCrawler/1.1\r\nHost: "+this->host+"\r\nConnection: close\r\n\r\n"; // INCORRECT
  //  cout << " total \n" << this->total << std::endl;
   // this->total = "GET /IRL7 HTTP/1.0\r\nUser-agent: JoshTamuCrawler/1.1\r\nHost: s2.irl.cs.tamu.edu\r\nConnection: close\r\n\r\n";
}
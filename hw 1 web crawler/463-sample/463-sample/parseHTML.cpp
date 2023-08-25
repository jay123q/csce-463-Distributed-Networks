#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>

using namespace std;
#include <string>
#include <vector>

std::vector<std::string> parseString(const char  * link) {
    std::vector<std::string> parsedValue;

    const char* hostStart = link + 7; // Skip "://"
  //  cout << "found path is " << pathStart << endl;

 
    const char* fragmentStart = strchr (link, '#');
    if (fragmentStart != nullptr) {
        std::string fragment(fragmentStart + 1); // Skip the '#'
        parsedValue.push_back(fragment);
        //std::string newLink(hostStart, fragmentStart);
        //const char * link = newLink.c_str();
       // cout << " removed fragment " << fragment << std::endl;
    }


    const char* queryStart = strchr(link, '?');
    if (queryStart !=  nullptr ) {
        const char* queryEnd = strchr(queryStart, '#');
        if (!queryEnd) {
            queryEnd = link + strlen(link);
        }
        std::string query(queryStart + 1, queryEnd - queryStart - 1); // Exclude '?' and '#'
        parsedValue.push_back(query);
      //  cout << " removed query " << query << std::endl;
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
            parsedValue.push_back(path);
        }
        else if (pathEndFragment != nullptr) //check fragment
        {
            std::string path(pathStart + 1, pathEndFragment - pathStart - 1); // +1 removes forwrd, -1 removes backward
            parsedValue.push_back(path);
        }
        else
        {
            parsedValue.push_back(" single /");
        }
    }
    else
    {
        parsedValue.push_back("no path");
    }
   // cout << " new path " << parsedValue.at(parsedValue.size()-1) << endl;
    // this is because the link can have a : in the middle of the ppath

    const char* portStart = strchr(hostStart, ':');
    if (portStart != nullptr)
    {
        const char* portEndBackSlash = strchr(hostStart, '/');
        const char* portEndQuery = strchr(hostStart, '?');
        const char* portEndFragment = strchr(hostStart, '#');
        if (portEndBackSlash != nullptr)
        {
            cout << "1" << std::endl;
            std::string port(portStart + 1, pathStart - portStart - 1); // remove  :, and /
            parsedValue.push_back(port);
        }
        else if (portEndQuery != nullptr)
        {
            cout << "2" << std::endl;

            std::string port(portStart + 1, portEndQuery - portStart - 1); // remove  :, and ?
            parsedValue.push_back(port);
        }
        else if (portEndFragment != nullptr)
        {
            cout << "3" << std::endl;

            std::string port(portStart + 1, portEndFragment - portStart - 1); // remove  :, and :
            parsedValue.push_back(port);
        }
        else // empty port
        {
            cout << "4" << std::endl;

            std::string port(portStart + 1);
            parsedValue.push_back(port);
        }

        //cout << " new port " << parsedValue.at(parsedValue.size() - 1) << endl;

    }
        
    if (portStart != nullptr)
    {
        //cout << " port check host " << std::endl;
        std::string host(hostStart, portStart - hostStart);
        parsedValue.push_back(host);
    }
    else if (pathStart != nullptr)
    {
        //cout << " port check path " << std::endl;

        std::string host(hostStart, pathStart - hostStart);
        parsedValue.push_back(host);
    }
    else if (queryStart != nullptr)
    {
       // cout << " port check query " << std::endl;

        std::string host(hostStart, queryStart - hostStart);
        parsedValue.push_back(host);
    }
    else if (fragmentStart != nullptr)
    {
        //cout << " port check fragment " << std::endl;

        std::string host(hostStart, fragmentStart - hostStart);
        parsedValue.push_back(host);
    }
    else
    {
        std::string host(hostStart);
        parsedValue.push_back(host);
    }

    //cout << " new host " << parsedValue.at(parsedValue.size() - 1) << endl;


    

   

    //for (int i = 0; i < parsedValue.size(); i++)
    //{
    //    std::cout << parsedValue[i] << std::endl;
    //}

    return parsedValue;
}

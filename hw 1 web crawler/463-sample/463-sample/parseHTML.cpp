#include "pch.h"
#pragma comment(lib, "ws2_32.lib")
#include <iostream>

using namespace std;
#include <string>
#include <vector>

std::vector<std::string> parseString(const char* link) {
    std::vector<std::string> parsedValue;

    // Find the fragment 
    if (const char* result = strchr(link, '#')) {

       // std::string fragment(result + 1); // Skip the '#'
        std::string fragment(result); // want to have it to recreate, can just skip later
        parsedValue.push_back(fragment);
    }

    // Find the query 
    if (const char* queryStart = strchr(link, '?')) {
        const char* queryEnd = strchr(queryStart, '#'); // this avoids null erminator work since i cant bother
        if (!queryEnd) {
            queryEnd = link + strlen(link); // If no fragment, use end of string
        }
        std::string query(queryStart, queryEnd - queryStart - 1); // keep ?, remove # since frag got it
        parsedValue.push_back(query);
    }

    // Find the port (if present)
    cout << " check port " << strstr(link, "://") << '/n';
    if (const char* schemeEnd = strstr(link, "://")) {
        const char* hostStart = schemeEnd + 3; // Skip "://"

        const char* hostEnd = strchr(hostStart, '/'); // Find end of host part
        if (!hostEnd) {
            hostEnd = link + strlen(link); // If no path, use end of string
        }
        const char* portStart = strchr(hostStart, ':'); // Check for port
        cout << "Checking truth " << * portStart && * portStart < *hostEnd << '\n';
        if (portStart && portStart < hostEnd) {
            std::string port(portStart + 1, hostEnd - portStart - 1); // Exclude ':' and '/'
            parsedValue.push_back(port);
        }
    }

    for (int i = 0; i < parsedValue.size(); i++)
    {
        std::cout << parsedValue[i] << std::endl;
    }

    return parsedValue;
}

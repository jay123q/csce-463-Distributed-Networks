/*
Joshua Clapp
Csce 463-500
*/
#include "pch.h"
#include <string>

#include "HTMLParserBase.h"
#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;
/*

*/

void winsock_test(void);
std::vector<std::string> parseString(const char* link);



int main(void)
{

	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}

// #ifdef DEBUG
	//std::wcout << GetExePath() << '\n';

// #endif // DEBUG
	//char * filename[] = "C:\\Users\\Joshua\\Documents\\github\\PersonalGit\\csce-463-Distributed-Networks\\hw 1 web crawler\\463-sample\\463-sample\\100url.txt";
	/*
	// open html file
	HANDLE hFile = CreateFile((LPCSTR) (&"100url.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	// process errors
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with %d\n", GetLastError());
		return 0;
	}

	// get file size
	LARGE_INTEGER li;
	BOOL bRet = GetFileSizeEx(hFile, &li);
	// process errors
	if (bRet == 0)
	{
		printf("GetFileSizeEx error %d\n", GetLastError());
		return 0;
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
		return 0;
	}

	// done with the file
	CloseHandle(hFile);
	cout << fileBuf << '/n';
	// create new parser object
	HTMLParserBase* parser = new HTMLParserBase;

	char baseUrl[] = "http://www.tamu.edu";		// where this page came from; needed for construction of relative links

	int nLinks;
	char* linkBuffer = parser->Parse(fileBuf, fileSize, baseUrl, (int)strlen(baseUrl), &nLinks);

	// check for errors indicated by negative values
	if (nLinks < 0)
		nLinks = 0;

	printf("Found %d links:\n", nLinks);

	// print each URL; these are NULL-separated C strings
	for (int i = 0; i < nLinks; i++)
	{
		// starting work here
		parseString(linkBuffer);
		linkBuffer += strlen(linkBuffer) + 1;
		// printf("%s\n", linkBuffer);
	}
	




	delete parser;		// this internally deletes linkBuffer
	delete fileBuf;
	*/

	parseString("http://tamu.edu#something");


	// handle socketing
	socket webSocket;


	return 0;
}

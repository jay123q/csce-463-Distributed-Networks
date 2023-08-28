/*
Joshua Clapp
Csce 463-500
*/
#include "pch.h"
#include <string>
#include "socket.h"
#include "parsedHTML.h"
#include <windows.h>
#include <iostream>
#include <vector>
using namespace std;
/*

*/

void winsock_test(void);





int main(void)
{


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
		////return 0;
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
		nLinks = 0;\r\n

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
	
	
	parsedHtml parser;
	parser.parseString("http://tamu.edu");


	// handle socketing
	Socket * webSocket = new Socket();
	// cout << " whole link " << parser.wholeLink << std::endl;
	parser.generateRequesttoSend("GET");
	bool socketCheck = webSocket->Send(parser.total, parser.wholeLink, parser.host, parser.port, parser.printPathQueryFragment());

	if (socketCheck)
	{
		// now try to read
		if (webSocket->Read())
		{
			webSocket->closeSocket(); // maybe move this into read? 
			// so now the html should return the buffer soo
			const char* result = webSocket->printBuf().c_str();
			cout << " the result  is " << webSocket->printBuf() << std::endl;
			cout << "\t Verifying header... ";
			// const char* httpStatus = strstr(webSocket->printBuf(), "HTTP/1.1");
			// llook for \r\n\r\n to parse
			const char * headerPointer = strstr(webSocket->printBuf().c_str(), "\r\n\r\n");
		}
	}


	return 0;
}

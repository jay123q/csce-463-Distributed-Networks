// HTMLParserTest.cpp
// Example program showing how to use the parser
// CSCE 463 sample code
//
#include "stdafx.h"

int main(int argc, char** argv)
{
	char filename [] = "tamu2018.html";

	// open html file
	HANDLE hFile = CreateFile (filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	// process errors
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf ("CreateFile failed with %d\n", GetLastError ());
		return 0;
	}

	// get file size
	LARGE_INTEGER li;
	BOOL bRet = GetFileSizeEx (hFile, &li);
	// process errors
	if (bRet == 0)
	{
		printf ("GetFileSizeEx error %d\n", GetLastError ());
		return 0;
	}

	// read file into a buffer
	int fileSize = (DWORD) li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
	DWORD bytesRead;
	// allocate buffer
	char *fileBuf = new char [fileSize];
	// read into the buffer
	bRet = ReadFile (hFile, fileBuf, fileSize, &bytesRead, NULL);
	// process errors
	if (bRet == 0 || bytesRead != fileSize)
	{
		printf("ReadFile failed with %d\n", GetLastError());
		return 0;
	}

	// done with the file
	CloseHandle(hFile);

	// create new parser object
	HTMLParserBase *parser = new HTMLParserBase;

	char baseUrl [] = "http://www.tamu.edu";		// where this page came from; needed for construction of relative links
	
	int nLinks;
	char *linkBuffer = parser->Parse (fileBuf, fileSize, baseUrl, (int) strlen (baseUrl), &nLinks);

	// check for errors indicated by negative values
	if (nLinks < 0)
		nLinks = 0;

	printf ("Found %d links:\n", nLinks);
	
	// print each URL; these are NULL-separated C strings
	for (int i = 0; i < nLinks; i++)
	{
		printf ("%s\n", linkBuffer);
		linkBuffer += strlen (linkBuffer) + 1;
	}

	delete parser;		// this internally deletes linkBuffer
	delete fileBuf;

	return 0;
}


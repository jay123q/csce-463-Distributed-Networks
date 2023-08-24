/* main.cpp
 * CSCE 463 Sample Code 
 * by Dmitri Loguinov
 */
#include "pch.h"
#include "HTMLParserBase.h"
#include <iostream>
// this class is passed to all threads, acts as shared memory
class Test {
	HANDLE	mutex;
	HANDLE	finished;
	HANDLE	eventQuit;
public:
	Test() {
		// create a mutex for accessing critical sections (including printf); initial state = not locked
		mutex = CreateMutex(NULL, 0, NULL);
		// create a semaphore that counts the number of active threads; initial value = 0, max = 2
		finished = CreateSemaphore(NULL, 0, 2, NULL);
		// create a quit event; manual reset, initial state = not signaled
		eventQuit = CreateEvent(NULL, true, false, NULL);
	}

	void threadA(void);
	void threadB(void);
};

// function inside winsock.cpp
void winsock_test (void);

// this function is where threadA starts
UINT threadA(LPVOID pParam)
{
	Test* p = ((Test*)pParam);
	p->threadA();
	return 0;
}

void Test::threadA(void)
{
	// wait for mutex, then print and sleep inside the critical section
	WaitForSingleObject (mutex, INFINITE);					// lock mutex
	printf ("threadA %d started\n", GetCurrentThreadId ());		// print inside critical section to avoid screen garbage
	Sleep (1000);												// sleep 1 second
	ReleaseMutex (mutex);									// release mutex

	// signal that this thread has finished its job
	ReleaseSemaphore (finished, 1, NULL);

	// wait for threadB to allow us to quit
	WaitForSingleObject (eventQuit, INFINITE);

	// print we're about to exit
	WaitForSingleObject (mutex, INFINITE);					
	printf ("threadA %d quitting on event\n", GetCurrentThreadId ());		
	ReleaseMutex (mutex);										
}

// this function is where threadB starts
UINT threadB(LPVOID pParam)
{
	Test* p = ((Test*)pParam);
	p->threadB();
	return 0;
}

void Test::threadB(void)
{
	// wait for both threadA threads to quit
	WaitForSingleObject (finished, INFINITE);
	WaitForSingleObject (finished, INFINITE);

	printf ("threadB running!\n");				// no need to sync as only threadB can print at this time
	Sleep (3000);

	printf ("threadB setting eventQuit\n"); 	// no need to sync as only threadB can print at this time

	// force other threads to quit
	SetEvent (eventQuit);
}
#include <string>
#include <vector>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

std::wstring GetExePath() {
	// Retrieve fully qualified module pathname
	std::vector<wchar_t> buffer(MAX_PATH);
	DWORD cbSize = ::GetModuleFileNameW(nullptr, buffer.data(),
		static_cast<DWORD>(buffer.size()));
	while (cbSize == buffer.size()) {
		buffer.resize(buffer.size() + MAX_PATH);
		cbSize = ::GetModuleFileNameW(nullptr, buffer.data(),
			static_cast<DWORD>(buffer.size()));
	}
	if (cbSize == 0) {
		throw ::GetLastError();
	}

	// Remove filename from fully qualified pathname
	if (::PathRemoveFileSpecW(buffer.data())) {
		::PathAddBackslashW(buffer.data());
	}

	// Construct string object from character buffer
	std::wstring str(&buffer[0]);
	return str;
}


#define _DEBUG
char * HTMLPrinter(void)
{

	#ifdef _DEBUG
		std::wcout << GetExePath() << std::endl;
		char filename[] = "C:\\Users\\Joshua\\Documents\\github\\PersonalGit\\csce-463-Distributed-Networks\\hw 1 web crawler\\463-sample\\463-sample\\Debug\\463-sample.tlog\\100url.txt";

	#endif // _DEBUG
	#ifndef _DEBUG
		char filename[] = "C:\\Users\\Joshua\\Documents\\github\\PersonalGit\\csce-463-Distributed-Networks\\hw 1 web crawler\\463-sample\\463-sample\\100url.txt";
	#endif
	// open html file
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
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
		printf("%s\n", linkBuffer);
		linkBuffer += strlen(linkBuffer) + 1;
	}
	// printf(linkBufffer);

	delete parser;		// this internally deletes linkBuffer
	delete fileBuf;

	return linkBuffer;


}
int main(void)
{
//	HTMLParserBase* HTMLParser = new HTMLParserBase;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;
	std::cout << "a aaaaaaaaa " << std::endl;

	char * UrlList = HTMLPrinter(); // THIS RUNS THE SCRIPT TO TAKE EACH TEST AND BREAK APART

	std::wcout << UrlList << std::endl;

	// connect to a server; test basic winsock functionality
	winsock_test ();

	printf ("-----------------\n");

	// print our primary/secondary DNS IPs
	DNS dns;
	dns.printDNSServer ();

	printf ("-----------------\n");

	CPU cpu;
	// run a loop printing CPU usage 10 times
	for (int i = 0; i < 10; i++)
	{
		// average CPU utilization over 200 ms; must sleep at least a few milliseconds *after* the constructor 
		// of class CPU and between calls to GetCpuUtilization
		Sleep (200);
		// now print
		double util = cpu.GetCpuUtilization (NULL);
		// -2 means the kernel counters did not accumulate enough to produce a result
		if (util != -2)
			printf ("current CPU utilization %f%%\n", util);
	}

	printf ("-----------------\n");

	// thread handles are stored here; they can be used to check status of threads, or kill them
	HANDLE *handles = new HANDLE [3];
	Test p;
	
	// get current time; link with winmm.lib
	clock_t t = clock();

	// structure p is the shared space between the threads
	handles [0] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)threadA, &p, 0, NULL);		// start threadA (instance #1) 
	handles [1] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)threadA, &p, 0, NULL);		// start threadA (instance #2)
	handles [2] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)threadB, &p, 0, NULL);		// start threadB 

	// make sure this thread hangs here until the other three quit; otherwise, the program will terminate prematurely
	for (int i = 0; i < 3; i++)
	{
		WaitForSingleObject (handles [i], INFINITE);
		CloseHandle (handles [i]);
	}
	
	printf ("terminating main(), completion time %.2f sec\n", (double)(clock() - t)/CLOCKS_PER_SEC);

	return 0; 
}

/* main.cpp
 * CSCE 463 Sample Code 
 * by Dmitri Loguinov
 */
#include "pch.h"

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

int main(void)
{
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

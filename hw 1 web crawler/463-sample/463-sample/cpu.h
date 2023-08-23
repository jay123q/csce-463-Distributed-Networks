/* 
 * CPU.h
 * CPSC 313 Sample Code 
 * by Dmitri Loguinov
 *
 */

#pragma once
#include <Winternl.h>
#include <psapi.h>
#include <math.h>

#define MAX_CPU	1024
#define MEGABYTE 1048576

class CPU{
public:
	int cpus;			// # of CPUs
	HMODULE hDll;		// handle to the NT dll
	HANDLE	hProcess;	// handle to the current process

	NTSTATUS  (__stdcall *NtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS SystemInformationClass,
			PVOID SystemInformation,
			ULONG SystemInformationLength,
			PULONG ReturnLength);

	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION info[MAX_CPU];
	ULONG len;
	__int64 idle[MAX_CPU], kernel [MAX_CPU], user [MAX_CPU];

			CPU (void);
			~CPU(); 
	double	GetCpuUtilization (double*);
	int		GetProcessRAMUsage(bool physical);
	int		GetSystemRAM (void);
	int		GetSystemRAMUsage (void);
};
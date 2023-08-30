/* 
 * CPU.cpp
 * CSCE 313 Sample Code 
 * by Dmitri Loguinov
 *
 * Obtains the current CPU utilization
 */
#include "pch.h"

// NOTE #1: link with Psapi.lib; this class uses the same functions as Task Manager
// NOTE #2: do not use unicode chars in the project; this will not compile! 
// project->properties->general->character set = Not Set

CPU::CPU (void)
{
	if ((hDll = GetModuleHandle("ntdll.dll")) == NULL)
	{
		printf("%s: cannot open handle to ntdll.dll with %d\n", __FUNCTION__, GetLastError());
		exit(-1);
	}

	// obtain a pointer to function NtQuerySystemInformation
	NtQuerySystemInformation = (NTSTATUS(__stdcall*)(SYSTEM_INFORMATION_CLASS
		SystemInformationClass,
		PVOID SystemInformation,
		ULONG SystemInformationLength,
		PULONG ReturnLength)) GetProcAddress(hDll, "NtQuerySystemInformation");

	if (NtQuerySystemInformation == NULL)
	{
		printf("%s: failed to get address of NtQuerySystemInformation with %d\n", __FUNCTION__, GetLastError());
		exit(-1);
	}

	SYSTEM_INFORMATION_CLASS query = SystemProcessorPerformanceInformation;
	NTSTATUS code = (NtQuerySystemInformation)(query, info,
		sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * MAX_CPU, &len);

	// how many CPUs
	this->cpus = len / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

	//printf ("found %d CPUs, total RAM %d MB\n", cpus, GetSystemRAM ());

	if (cpus > MAX_CPU)
	{
		printf("%s: found %d logical cores, maximum allowed is %d\n", __FUNCTION__, cpus, MAX_CPU);
		exit(-1);
	}

	for (int i = 0; i < cpus; i++)
	{
		this->idle[i] = info[i].IdleTime.QuadPart;
		this->kernel[i] = info[i].KernelTime.QuadPart;
		this->user[i] = info[i].UserTime.QuadPart;
	}

	if ((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, GetCurrentProcessId())) == NULL)
	{
		printf("%s: cannot open process with %d\n", __FUNCTION__, GetLastError());
		exit(-1);
	}
}

CPU::~CPU()
{
	CloseHandle (hProcess);
}

// returns utilization * 100.0, averaged over all CPUs; if the array is not NULL, fills in individual per-CPU utilization figures
double CPU::GetCpuUtilization (double *CPUarr)
{
	SYSTEM_INFORMATION_CLASS query = SystemProcessorPerformanceInformation;

	NTSTATUS code = (NtQuerySystemInformation)(query, info, 
		sizeof (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)*MAX_CPU, &len);

	double average = 0;
	for (int i = 0; i < cpus; i++)
	{
		__int64 idle_dur = info[i].IdleTime.QuadPart - idle[i];
		__int64 kernel_dur = info[i].KernelTime.QuadPart - kernel[i];
		__int64 user_dur = info[i].UserTime.QuadPart - user[i];
		
		__int64 sys_time = kernel_dur + user_dur;

		if (sys_time == 0)		// called too often; need to wait between calls at least 50 ms
			return -2;

		double val = 100.0 * (sys_time - idle_dur) / sys_time;
		
		// store individual CPU utilization if desired by caller
		if (CPUarr != NULL)	
			CPUarr [i] = val;

		average += val;

		idle[i] = info[i].IdleTime.QuadPart;
		kernel[i] = info[i].KernelTime.QuadPart;
		user[i] = info[i].UserTime.QuadPart;
	}

	return average/cpus;
}

// return in megabytes
int CPU::GetProcessRAMUsage(bool physical)
{
	PROCESS_MEMORY_COUNTERS pp;
	if (GetProcessMemoryInfo(hProcess, &pp, sizeof(PROCESS_MEMORY_COUNTERS)))
	{
		if (physical)
			return (int)floor(pp.WorkingSetSize / MEGABYTE + 0.5);			// physical memory usage
		else
			return (int)floor(pp.PagefileUsage / MEGABYTE + 0.5);			// virtual memory usage
	}
	else
	{
		printf("%s: failed to get process memory info with %d\n", __FUNCTION__, GetLastError());
		exit(-1);
	}
}

int CPU::GetSystemRAM (void)
{
	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof (statex);
	if (GlobalMemoryStatusEx(&statex) == 0)
	{
		printf("%s: failed to get memory status with %d\n", __FUNCTION__, GetLastError());
		exit(-1);
	}

	// total virtual memory
	//return (int) (statex.ullTotalPageFile/MEGABYTE + 0.5); //(statex.ullTotalPhys/1e6);
	// total physical memory
	return (int) (statex.ullTotalPhys/MEGABYTE + 0.5); 
}

int CPU::GetSystemRAMUsage (void)
{
	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof (statex);
	if (GlobalMemoryStatusEx(&statex) == 0)
	{
		printf("%s: failed to get memory status with %d\n", __FUNCTION__, GetLastError());
		exit(-1);
	}

	// physical memory
	//return ((statex.ullTotalPhys-statex.ullAvailPhys)/1e6);
	// virtual memory
	return (int) ((statex.ullTotalPageFile -
		statex.ullAvailPageFile)/MEGABYTE + 0.5); 
}
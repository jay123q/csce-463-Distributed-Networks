#pragma once
#include <iostream>
#include <string>
#include "pch.h"
using namespace std;

#pragma pack(push,1) // sets struct padding/alignment to 1 byte
class QueryHeader;
class FixedDNSheader;
#pragma pack(pop) 





class DnsHelper
{
public:
	bool amI1or12;
	DWORD ip;
	int attemptCount;
	string modifiedQuery;
	string query;
	void makeDNSquestion(char* buffer, string query );
};

// jump return a string to then contaction
// take a substring and then at each point, I can then send this into the buffer 
// so take all my shit and then return + jump() etc
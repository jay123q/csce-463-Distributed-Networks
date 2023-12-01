#pragma once
#pragma once

#include "pch.h"
#include <ctype.h> 
#include <stdio.h> 
#include <windows.h>
#include <string>
#include <iostream>

class checksum
{
public:
	DWORD crc_table[256];

	checksum();
	DWORD CRC32(unsigned char* buf, UINT64 len);
	u_short ip_checksum(u_short* buffer, int size);

};
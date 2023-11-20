#include <ctype.h> 
#include <stdio.h> 
#include <windows.h>
#include <string>
#include <iostream>
#include "pch.h"
#include "Checksum.h"

Checksum::Checksum()
{
	// set up a lookup table for later use
	for (DWORD i = 0; i < 256; i++)
	{
		DWORD c = i;
		for (int j = 0; j < 8; j++) {
			c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
		}
		crc_table[i] = c;
	}
}
DWORD Checksum::CRC32(unsigned char* buf, UINT64 len)
{

	DWORD c = 0xFFFFFFFF;
	for (size_t i = 0; i < len; i++)
		c = crc_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);

	if ((c ^ 0xFFFFFFFF )== 0)
	{

	}
	else
	{
		// return 
	}
	return c ^ 0xFFFFFFFF;
}


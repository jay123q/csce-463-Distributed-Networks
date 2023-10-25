#pragma once

#include "pch.h"
#include <ctype.h> 
#include <stdio.h> 
#include <windows.h>
#include <string>
#include <iostream>

class Checksum
{
public:
	DWORD crc_table[256];

	Checksum();
	DWORD CRC32(unsigned char* buf, size_t len);
};
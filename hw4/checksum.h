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

	u_short ip_checksum(u_short* buffer, int size);

};
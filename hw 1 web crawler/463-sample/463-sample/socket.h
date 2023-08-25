#pragma once
#include "pch.h"

class Socket {
	private:
		Socket sock; // socket handle
		char* buf; // current buffer
		int allocatedSize; // bytes allocated for buf
		int curPos; // current position in buffer
		
public:
		// extra stuff as needed
		bool Read(void);
		Socket();
};
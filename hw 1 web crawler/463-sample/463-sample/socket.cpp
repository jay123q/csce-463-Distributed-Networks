


#include "pch.h"
#include "socket.h"

int INITIAL_BUF_SIZE = 8512;

Socket::Socket()
{
	// create this buffer once, then possibly reuse for multiple connections in Part 3
	buf = new char[INITIAL_BUF_SIZE]; // either new char [INITIAL_BUF_SIZE] or malloc (INITIAL_BUF_SIZE)
	allocatedSize = INITIAL_BUF_SIZE;
}
bool Socket::Read(void)
{
	// set timeout to 10 seconds
	while (true)
	{
		// wait to see if socket has any data (see MSDN)
		if ((ret = select(0, &fd, ..., timeout)) > 0)
		{
			// new data available; now read the next segment
			int bytes = recv(sock, buf + curPos, allocatedSize – curPos, ...);
			if (errors)
				// print WSAGetLastError()
				break;
			if (connection closed)
				// NULL-terminate buffer
				return true; // normal completion
			curPos += bytes; // adjust where the next recv goes
			if (allocatedSize – curPos < THRESHOLD)
				// resize buffer; you can use realloc(), HeapReAlloc(), or
			   // memcpy the buffer into a bigger array
		}
		else if (timeout)
			// report timeout
			break;
		else
			// print WSAGetLastError()
			break;
	}
	return false;
}

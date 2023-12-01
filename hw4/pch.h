// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here

#define _WINSOCK_DEPRECATED_NO_WARNINGS

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;
#undef UNICODE
#define UNICODE
#undef _WINSOCKAPI_
#define _WINSOCKAPI_
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <string.h>
#include <string>
#include <ws2tcpip.h>
#include <ctime>
#include <vector>

#include "checksum.h"


#endif //PCH_H

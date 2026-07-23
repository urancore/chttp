#ifndef CHTTP_PLATFORM_H
#define CHTTP_PLATFORM_H

#ifdef _WIN32
 	#include <winsock2.h>
	#include <windows.h>
	#include <stdio.h>

	#include "io_win.h"
#else
	#include <sys/socket.h>
	#include <stdlib.h>

	#include "io_unix.h"
#endif

#endif /* CHTTP_PLATFORM_H */

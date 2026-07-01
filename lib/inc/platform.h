#ifndef CHTTP_PLATFORM_H
#define CHTTP_PLATFORM_H

#ifdef _WIN32
	#include "io_win.h"
#else
	#include "io_unix.h"
#endif

#endif

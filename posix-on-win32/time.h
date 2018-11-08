// time.h

#pragma once
#ifndef _TIME_H
#define _TIME_H

// POSIX stuff

// Macros that emulate include_next for our specific use case. These strings belong
// in a config file that goes next to the generated project.
#define MICROSOFT_UCRT "c:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt/time.h"
#define _MICROSOFT_UCRT_INCLUDE_NEXT_UCRT(HEADER) <c:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt/HEADER>

// Include the Microsoft <time.h>
// TBD - just replace all the Microsoft code with local code? It would make things simpler
//#include <c:/Program Files (x86)/Windows Kits/10/Include/10.0.10240.0/ucrt/time.h>
#include _MICROSOFT_UCRT_INCLUDE_NEXT_UCRT(time.h)

#endif // _TIME_H

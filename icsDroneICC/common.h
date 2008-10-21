/*
icsDrone, Copyright (c) 1996-2001, Henrik Oesterlund Gram
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are 
met:

Redistributions of source code must retain the above copyright 
notice, this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in 
the documentation and/or other materials provided with the 
distribution. 

The names of its contributors may not be used to endorse or 
promote products derived from this software without specific 
prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _COMMON_H
#define _COMMON_H

#define BUF_SIZE 8192

#define NEWLINE "\r\n"

#define PID_T pid_t

#if defined(__STDC__)
# define P(arg) arg
#else
# define P(arg) ()
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(sun)
# include <sys/filio.h>
# include <sys/uio.h> 
#endif
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>       
#include <netdb.h>
#include <sys/ioctl.h>
#include <arpa/telnet.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <ctype.h>
#include <zlib.h>

#if defined(linux) || defined(hpux) 
# define BSDgettimeofday
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#define OK 0
#define ERROR -1

#undef FALSE
#undef TRUE
typedef enum { FALSE = 0, TRUE = 1 } Bool;

/* If the above code makes your compiler puke, exchange it with the following:
#ifndef FALSE 
#define FALSE 0
#endif
#ifndef TRUE 
#define TRUE 1
#endif
typedef int Bool;
*/

#endif

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

#ifndef _UTILS_H
#define _UTILS_H

void ConvShortToLong(char *short_not, char *long_not);
extern void bytezero P((void *dest, int n));
extern void bytecopy P((void *dest, const void *src, int n));
extern void stringcat P((void *dest, const void *src, int n));
extern void stringcopy P((void *dest, const void *src, int n));
extern void ConvIcsLanToComp P((char *));
extern void ConvIcsSanToComp P((char *));
extern void ConvIcsSpecialToComp P((char *));
extern void ConvCompToIcs P((char *));
void ConvIcsPromToComp(char *move, char prom_piece);
typedef enum { LOG_ERROR = 1, LOG_WARNING, LOG_INFO, LOG_CHAT, LOG_DEBUG } LogType;
extern void logme P((LogType type, const char* format, ...));

extern void StartLogging P(());
extern void StopLogging P(());

#endif

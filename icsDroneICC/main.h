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

#ifndef _MAIN_H
#define _MAIN_H

/* 
 *  These are returned when program quits and is stored in the 'status' 
 *  shell variable for inspection.
 */
#define EXIT_RESTART  1
#define EXIT_SOFTQUIT 2
#define EXIT_HARDQUIT 3
#define ICC_SERVER 1 /* Playing at the ICC server (chessclub.com)*/

typedef struct {
    Bool  computerActive;
    Bool  computerIsWhite;
    int   computerReadFd;
    int   computerWriteFd;
    pid_t computerPid;
    int   icsFd;
    char* handle;
    char* passwd;
    Bool  quitPending;
    Bool  waitingForFirstBoard;
    int   exitValue;
    int   gameID;
    int   moveNum;
    int   waitingForPingID;
} RunData;

typedef struct {
    char* icsHost;
    int   icsPort;
    int   searchDepth;
    int   secPerMove;
    int   logLevel;
    char* logFile;
    char* owner;
    int   console;
    int   easyMode;
    int   book;
    Bool  craftyMode;
    char* program;
    Bool  longAlgMoves;
    char* sendGameEnd;
    Bool  haveCmdNew;
    Bool  haveCmdType;
    Bool  haveCmdResult;
    int   sendTimeoutCmds;
    Bool  killEngine;
    int   limitRematches;
    Bool  autoConfigure;
    Bool  haveCmdPing;
    Bool  haveUsermove;
} AppData;

extern RunData runData;
extern AppData appData;

extern void ExitOn(const char *errmsg);

#endif

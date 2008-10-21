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

#include "common.h"
#include "net.h"
#include "main.h"
#include "utils.h"
#include "argparser.h"
#include "computer.h"
#include "fics.h"

RunData runData = {
    FALSE, 
    FALSE, 
    0, 
    0, 
    (pid_t) 0, 
    0,
    NULL, 
    NULL,
    FALSE, 
    FALSE, 
    0,
    -1,
    -1,
    0
};

AppData appData = {
    "freechess.org", 
    5000, 
    0, 
    0, 
    255,
    NULL, 
    NULL, 
    0,
    0,
    1,
    FALSE,
    "gnuchessx",
    FALSE,
    NULL,
    FALSE,
    FALSE,
    FALSE,
    0,
    TRUE,
    0,
    FALSE,
    FALSE
};

int  ProcessRawInput P((int, char *, int, void (*)(char *)));
void ProcessConsoleLine P((char *));

void MainLoop P(());
void TerminateProg P((int));
void BrokenPipe P((int));
void Usage P(());

void ProcessConsoleLine(char *line) 
{
    if (!strncmp(line, "computer: ", 10)) {
	SendToComputer("%s\n", line+10);
    } else if (!strncmp(line, "ics: ", 5)) {
	SendToIcs("%s\n", line+5);
    } else {
	printf("Unknown target\n");
    }
}

void MainLoop()
{
    fd_set readfds;
    struct timeval timeout;
    u_char cBuf[2*BUF_SIZE], sBuf[2*BUF_SIZE], conBuf[2*BUF_SIZE];

    sBuf[0] = '\0';
    cBuf[0] = '\0';
    while (1) {
	FD_ZERO(&readfds);
	if (!runData.waitingForPingID)
	    FD_SET(runData.icsFd, &readfds);
	if (appData.console)
	    FD_SET(0, &readfds);
	if (runData.computerActive) 
	    FD_SET(runData.computerReadFd, &readfds);
	//timeout.tv_sec = 1000000;
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	// ICC/minimardi hack :
	// If the Engine hangs, icsDrone will block for 277h  :)
	// I had problems when the engine didn't respond a ping, lets fix
	// that first. Best would be to handle all engine output, but then
	// we would need to determine the time control and add a upper limit...
	if (runData.waitingForPingID) 
	{
	    timeout.tv_sec=35; 
	}
	/*
	if (select(MAX(runData.icsFd, runData.computerReadFd) + 1,
		   &readfds, NULL, NULL, &timeout) < 0 && errno == EINTR)
	    continue;  // Alarm caught 
	*/
	int ret = select(MAX(runData.icsFd, runData.computerReadFd) + 1,  &readfds, NULL, NULL, &timeout);
	if (!ret && runData.waitingForPingID) {
	     ExitOn("Computer didn't respond to our ping.\n");
	}
	if (ret  < 0 && errno == EINTR)
	     continue;  /* Alarm caught */

	if (appData.console && 
	    FD_ISSET(0, &readfds))
	    
	    ProcessRawInput(0, conBuf, sizeof(conBuf), ProcessConsoleLine);
	if (runData.computerActive && 
	    FD_ISSET(runData.computerReadFd, &readfds)) 

	    if (ProcessRawInput(runData.computerReadFd, cBuf, sizeof(cBuf), ProcessComputerLine) == ERROR)
		ExitOn("Lost contact with computer.\n");
	if (!runData.waitingForPingID && 
	    FD_ISSET(runData.icsFd, &readfds)) {
	    if (ProcessRawInput(runData.icsFd, sBuf, sizeof(sBuf), ProcessIcsLine) == ERROR)
		ExitOn("Disconnected from ics.\n");
	}

    }
}

void ExitOn(const char *errmsg)
{
    if (!errmsg) {
#ifndef NO_STRERROR
	logme(LOG_ERROR, "%s", strerror(errno));
#else
	logme(LOG_ERROR, "Fatal error (errno %d)", errno);
#endif
    } else {
	logme(LOG_ERROR, "%s", errmsg);
    }

    if (runData.computerActive) {
	KillComputer();
    }
    close(runData.icsFd);

    StopLogging();

    exit(runData.exitValue);
}  

void TerminateProc(int sig)
{
    ExitOn("Got SIGINT/SIGTERM signal");
}

void BrokenPipe(int sig)
{
    ExitOn("Got SIGPIPE signal");
}  

void Usage()
{
    printf("%s %s, (c) 1996-2001 by Henrik Oesterlund Gram.  All rights reserved.\n\n"
	   "See README file on how to use it.\n", PRODUCTNAME, PRODUCTVERSION);
    exit(0);
}

int main(int argc, char *argv[]) 
{
    FILE *fp;
    char buf[256];

    if (argc == 1 || ParseArgs(argc, argv) == ERROR)
	Usage();

    StartLogging();

    /* auto-config */
    if (appData.autoConfigure) {
	if (strstr(appData.program, "crafty")) {
	    printf("Auto-configuring for Crafty\n");
	    appData.killEngine = FALSE;
	    appData.haveCmdNew = TRUE;
	    appData.haveCmdResult = TRUE;
	    appData.haveCmdType = FALSE;
	    appData.craftyMode = TRUE;
	    appData.longAlgMoves = FALSE;
	    appData.haveCmdPing = TRUE;
	} else if (strstr(appData.program, "gnuchessx")) {
	    printf("Auto-configuring for GNUChess 4.x\n");
	    appData.killEngine = TRUE;
	    appData.haveCmdNew = FALSE;
	    appData.haveCmdResult = FALSE;
	    appData.haveCmdType = FALSE;
	    appData.craftyMode = FALSE;
	    appData.longAlgMoves = FALSE;
	    appData.haveCmdPing = FALSE;
	} else {
	    printf("Cannot auto-configure as I don't recognize \"%s\"\n", appData.program);
	}
    }

    fp = fopen(".icsdronerc", "r");

    if (!(runData.handle = getenv("FICSHANDLE"))) {
	if (!fp || !fgets(buf, sizeof(buf), fp)) {
	    ExitOn("You must specify the handle.\nSee README file for help.\n");
	} else {
	    buf[ strlen(buf)-1 ] = '\0';
	    runData.handle = strdup(buf);
	}
    }

    if (!(runData.passwd = getenv("FICSPASSWD"))) {
	if (!fp || !fgets(buf, sizeof(buf), fp)) {
	    ExitOn("You must specify the password.\nSee README file for help.\n");
	} else {
	    buf[ strlen(buf)-1 ] = '\0';
	    runData.passwd = strdup(buf);
	}
    }

    signal(SIGINT, TerminateProc);
    signal(SIGTERM, TerminateProc);
    signal(SIGPIPE, BrokenPipe);

    if ((runData.icsFd = OpenTCP(appData.icsHost, appData.icsPort)) <= 0) 
	ExitOn("Failed to connect to server.\n");

    SendToIcs("%s\n%s\n", runData.handle, runData.passwd);

    /*
     *  If we are in craftymode and don't kill engine after each game, 
     *  we might aswell start the engine now.  This way the engine can
     *  init TBs or whatever it needs before it's supposed to start
     *  playing                                                       
     */
    if (appData.haveCmdNew && !appData.killEngine)
	StartComputer();
  
    SendToIcs("set style 12\n"
	      "set shout 0\n"
	      "set open 1\n"
	      "set highlight 0\n"
	      "set bell 0\n" 
	      "set width 240\n"
	      "set pin 0\n"
	      "set gin 0\n"
	      "set interface %s %s, (c) 1996-2001 by Henrik Gram.\n",
	      PRODUCTNAME, PRODUCTVERSION);

    /* Send the rest of the lines in .icsdronerc as commands to the server */
    if (fp) {
	while(fgets(buf, sizeof(buf), fp))
	    SendToIcs("%s\n", buf);
	fclose(fp);
    }  
    
    MainLoop();

    logme(LOG_INFO, "Program terminated normally.");

    StopLogging();

    return 0;
}

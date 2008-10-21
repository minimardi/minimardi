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
#include "main.h"
#include "utils.h"
#include "net.h"
#include "computer.h"

#ifdef __EMX__  /* port of gcc for oddball platforms */

/* 
 *  The following replacement of StartComputer() was provided
 *  by William J. Little <nightowl@gis.net>.  It's supposed 
 *  to make RoboFICS work correctly on OS/2 - but please note
 *  that I have not tested it myself so I dont know if it
 *  even compiles :)
 */

void StartComputer()
{
    int fdin[2], fdout[2], tempin, tempout, temp;
    char argv[200];

    pipe(fdin);
    pipe(fdout);
    tempin=dup(fileno(stdin));
    tempout=dup(fileno(stdout));
    close(fileno(stdin));
    dup(fdin[0]);
    close(fdin[0]);
    close(fileno(stdout));
    dup(fdout[1]);
    close(fdout[1]);

    if (!appData.craftyMode) {
	if (appData.secPerMove) {
	    sprintf(secs, "%d", appData.secPerMove);
	    temp=spawnlp(P_NOWAIT,appData.program, appData.program, secs, NULL);
	} else {
	    temp=spawnlp(P_NOWAIT,appData.program, appData.program, NULL);
	}    
    } else {
	temp=spawnlp(P_NOWAIT,appData.program, appData.program, "xboard", NULL);
    }
  
    if(temp == -1)
	ExitOn("Unable to execute chess program!\n");

    close(fileno(stdin));
    dup(tempin);
    close(tempin);
    close(fileno(stdout));
    dup(tempout);
    close(tempout);
    runData.computerReadFd = fdout[0];
    runData.computerWriteFd = fdin[1];
    runData.computerActive = TRUE;

    SendToComputer("xboard\n");   /* zippy v1 */
    SendToComputer("protover 2\n");  /* zippy v2 */
    SendToComputer("ics %s\n", appData.icsHost);  /* zippy v2 */

    if (appData.haveCmdPing)
	PingComputer();
}

#else   /* A Real OS */

void StartComputer()
{
    int fdin[2], fdout[2], argc = 0;
    char buf[1024];
    char *argv[80], *p;

    pipe(fdin);
    pipe(fdout);
    switch(runData.computerPid = fork()) {
    case -1:
	ExitOn("Unable to create new process!\n");
    case 0:
	close(fileno(stdin));
	dup(fdin[0]);
	close(fileno(stdout));
	dup(fdout[1]);
	close(fdin[0]);
	close(fdin[1]);
	close(fdout[0]);
	close(fdout[1]);

	strcpy(buf, appData.program);
	if (appData.craftyMode) {
	    strcat(buf, " xboard");
	} else if (!appData.craftyMode && appData.secPerMove) {
	    sprintf(buf+strlen(buf), " %d", appData.secPerMove);
	}

	p = strtok(buf, " ");
	do {
	    argv[argc++] = strdup(p);
	} while ((p = strtok(NULL, " ")));
	argv[argc] = NULL;

	execvp(argv[0], argv);

	ExitOn("Unable to execute chess program!  (have you supplied the correct path/filename?)\n");

	break;
    default:
	close(fdin[0]);
	close(fdout[1]);
	runData.computerReadFd = fdout[0];
	runData.computerWriteFd = fdin[1];
	break;
    } 
    runData.computerActive = TRUE;

    SendToComputer("xboard\n");   /* zippy v1 */
    SendToComputer("protover 2\n");  /* zippy v2 */
    SendToComputer("ics %s\n", appData.icsHost);  /* zippy v2 */

    if (appData.haveCmdPing)
	PingComputer();
}

#endif

void KillComputer()
{
    logme(LOG_INFO, "Killing computer");

    kill(runData.computerPid, SIGKILL);

    /*  
     *  The behaviour of this seems to differ from system to system.
     *  If it fails, comment it out, but watch out what happens to those
     *  child processes - I tried running it on Solaris 2.5.1 without the
     *  wait() which resulted in over 2000 gnuchess zombies :)
     */
    if (wait(NULL) == -1 && errno != ECHILD) 
	ExitOn("wait() failed!");

    runData.waitingForPingID = 0;
    runData.computerActive = FALSE;
    close(runData.computerReadFd);
    close(runData.computerWriteFd);
}

void InterruptComputer()
{
    logme(LOG_DEBUG, "Interrupting computer");

    kill(runData.computerPid, SIGINT);
}

void PingComputer()
{
    static int pingID = 1;

    runData.waitingForPingID = pingID++;
    SendToComputer("ping %d\n", runData.waitingForPingID);
}

void ProcessComputerLine(char *line) 
{
    char move[256];

    logme(LOG_DEBUG, "<- engine: %s", line);

    if (sscanf(line, "%*s ... %s", move) == 1 ||
	sscanf(line, "move %s", move) == 1) {
	if (runData.gameID != -1) {
	    ConvCompToIcs(move);
	    SendToIcs("%s\n", move);
	} else {
	    logme(LOG_WARNING, "Move received from engine but we're not playing anymore!"); 
	}
    } else if (!strncmp(line, "pong ", 5)) {
	if (runData.waitingForPingID == atoi(line + 5)) {
	    runData.waitingForPingID = 0;
	    logme(LOG_DEBUG, "Got ping reply from computer matching our ping request.");
	} else {
	    logme(LOG_WARNING, "Got unexpected ping reply from computer.");
	}
    } else if (!strncmp(line, "feature ", 8)) {
	/* TODO: feature switches (Zippy v2) */

	/* If reuse=1, we acknowledge this and override whatever the user told us */
	if (strstr(line, " reuse=1")) {
	    appData.killEngine = FALSE;
	    appData.haveCmdNew = TRUE;
	}

	/* If sigint=0, we assume craftyMode */
	if (strstr(line, " sigint=0")) {
	    appData.craftyMode = TRUE;
	}

	/* ping command */
	if (strstr(line, " ping=1")) {
	    appData.haveCmdPing = TRUE;
	}

    } else if (strstr(line, "Draw") || strstr(line, "draw")) {
	SendToIcs("draw\n");
    } else if (!strncmp(line, "tellics ", 8)) {
	SendToIcs("%s\n", line + 8);
    } else if (!strncmp(line, "tellicsnoalias ", 15)) {
	SendToIcs("$%s\n", line + 15);
    } else if (!strncmp(line, "whisp", 5) ||
	       !strncmp(line, "kibit", 5) ||
	       !strncmp(line, "say", 3) ||
	       !strncmp(line, "tell", 4)) {
	SendToIcs("%s\n", line);
    }
}

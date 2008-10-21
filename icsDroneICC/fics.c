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

#define NORMAL               0
#define PARSING_MOVELIST     1
#define PARSING_MESSAGELIST  2
#define PARSING_LIST         3
#define PARSING_PENDING      4

#define WHITE 0
#define BLACK 1

/* For sending timeout commands */
void IdleTimeout(int dummy)
{
    FILE *fp;
    char buf[1024];

    /* Reset alarm */
    alarm(appData.sendTimeoutCmds * 60);
    signal(SIGALRM, IdleTimeout);

    /* Send cmds to server */
    if ((fp = fopen(".icsdronerc-idlecmds", "r"))) {
	while (fgets(buf, sizeof(buf)-1, fp)) {
	    SendToIcs("\n%s\n", buf);
	}
	fclose(fp);
    }
}

/* For limitRematches code */
void FindChallenge(int dummy)
{
    /* Reset alarm */
    alarm(0);
    signal(SIGALRM, SIG_IGN);
    
    /* Get a list of pending challenges; much better than maintaining it ourselves */
    SendToIcs("\npending\n");

    /* In case there are no pendings left, we'll want to have the timeout handler setup */
    if (appData.sendTimeoutCmds) {
	alarm(appData.sendTimeoutCmds * 60);
	signal(SIGALRM, IdleTimeout);
    }
}

/* For auto-flagging */
void Flag(int dummy)
{
    /* Reset alarm */
    alarm(0);
    signal(SIGALRM, SIG_IGN);

    /* Flag */
    SendToIcs("\nflag\n");
}

/* For courtesyadjourning games after x minutes of waiting */
void CourtesyAdjourn(int dummy)
{
    alarm(0);
    signal(SIGALRM, SIG_IGN);

    /* Adjourn */
    SendToIcs("\nadjourn\n");
}

void ProcessIcsLine(char *line)
{
    char move[256], move2[256], tmp[8192*10], san[80], lan[80];
    int basetime, inctime, whitetime, blacktime, moveNum;
    char dummy, c;
    char *p, *oppname;
    int i, j, x, prom_move,  color;
    
    static char   lastPlayer[20] = "";
    static int    numGamesInSeries = 0;
    static int    backupoffer = -1;
    static time_t timeOfLastGame = 0;
    int offer;

    static char onMove;
    static int state = NORMAL;
    static char moveList[8192];
    char tmpmoveList[8192];
    static int messageindex;
    static char message[8192];
    static char rating[256] = "", rating2[256] = "", name[256], name2[256];
    static char last_talked_to[80] = { 0 };
    static char listname[256];
    static char listmembers[32*1024];  /* enough for over 1800 names */
    static int listsize, listindex;
    
    while (!strncmp(line, "aics% ", 6)) 
      line += 6;

    logme(LOG_DEBUG, "<- ics: %s", line);
    switch(state) {
    case NORMAL:
    
	/*
	 *  An attempt to catch non-authentic msgs (fuck sscanf() !) - the 
	 *  assumption beeing that no real messages starts with a space. 
	 */

	if (line[0] == ' ')
	    break;
	
	/*
	 *  Detect our nickname - this is needed when 1) it was given in the 
	 *  wrong case and 2) we login as guest where we are assigned a random
	 *  guest nickname. 
	 */

	if (sscanf(line, "**** Starting FICS session as %[^\( ]s", name) == 1 &&
	    !strncasecmp(runData.handle, name, strlen(runData.handle))) {
	    if (strcmp(runData.handle, name)) {
		logme(LOG_WARNING, "Nickname was corrected to: %s", name);
		strcpy(runData.handle, name);
	    }
	    break;
	}

	/*
	 *  Pending list; find the first offer made not by the last player
	 *  (limitRematches code)
	 */
	
	if (appData.limitRematches &&
	    !strncmp(line, "Offers from other players", 25)) {
	    backupoffer = -1;
	    state = PARSING_PENDING;
	    logme(LOG_DEBUG, "Detected start of pending offers list");
	    break;
	}

	/*
	 *  Respond to offers
	 */

	if (sscanf(line, "%s offers you a draw%c", name, &dummy) == 2) {
	    if (appData.craftyMode)
		SendToComputer("draw\n");
	    else
		SendToIcs("tell %s Sorry, this computer doesn't accept draws.\n", 
			  name);
	    break;
	} else if (sscanf(line, "%s would like to abor%c", name, &dummy) == 2) {
	    SendToIcs("tell %s Sorry, this computer doesn't accept aborts.\n", name);
	    break;
	}
    
	/*
	 *  Respond to player tells
	 */
    
	if ((sscanf(line, "%s tells you: %[^\n\r]s", name, tmp) == 2 || 
	     sscanf(line, "%s says: %[^\n\r]s", name, tmp) == 2) &&
	    strcmp(name, runData.handle)) {
	    /*  Cut off possible () and [] from the name */
	    if (strchr(name, '(')) {
		name[strchr(name, '(') - name] = '\0';
	    } else if (strchr(name, '[')) {
		name[strchr(name, '[') - name] = '\0';
	    }
      
	    if (appData.owner && !strcmp(appData.owner, name)) {
		if (!strcmp(tmp, "restart")) {
		    runData.exitValue = EXIT_RESTART;
		    if (runData.gameID != -1) {
			SendToIcs("tell %s Ok, I will restart as soon as this game is over.\n",
				  appData.owner);
			runData.quitPending = TRUE;
		    } else {
			SendToIcs("tell %s Ok, I will restart now...\n", appData.owner);
			ExitOn("Restart command executed.");
		    }
		} else if (!strcmp(tmp, "softquit")) {
		    runData.exitValue = EXIT_SOFTQUIT;
		    if (runData.gameID != -1) {
			SendToIcs("tell %s Ok, I will quit as soon as this game is over.\n",
				  appData.owner);
			runData.quitPending = TRUE;
		    } else {
			SendToIcs("tell %s Ok, I will quit now...\n", appData.owner);
			ExitOn("SoftQuit command executed.");
		    }
		} else if (!strcmp(tmp, "hardquit")) {
		    SendToIcs("tell %s Ok, I will quit now!\n", appData.owner);
		    runData.exitValue = EXIT_HARDQUIT;
		    ExitOn("HardQuit command executed.");
		} else if (tmp[0] == '*') {
		    SendToComputer("%s\n", tmp+1);
		    SendToIcs("tell %s Forwarded \"%s\" to the engine.\n", appData.owner, tmp+1);
		} else {
		    SendToIcs("%s\n", tmp);
		}
	    } else {
		if (strcmp(last_talked_to, name)) {
		    SendToIcs("tell %s (auto-response) Hi, I'm an automated computer.  If you need to contact my owner, just leave a message for me.\n", name);
		}
		strcpy(last_talked_to, name);
	    }
	    break;
	}
    
	/*
	 *  Remind a player of an unfinished game...
	 */

	if (sscanf(line, "Notification: %s who has an adjourned game%c", name, &dummy) == 2) {
	    name[strlen(name)-1] = '\0';
	    SendToIcs("match %s\ntell %s Hi %s.  Let's finish our adjourned game, please.\n",
		      name, name, name);
	    break;
	}

	/*
	 *  Start of message?
	 */

	if (!strncmp(line, "Messages:", 9) || 
	    !strncmp(line, "Messages from", 13)) {

	    logme(LOG_DEBUG, "Detected start of message list.\n");
	    state = PARSING_MESSAGELIST;
	    /* following is a hack to ensure we detect end of the messagelist */
	    SendToIcs("tell %s dummy\n", runData.handle); 
	    messageindex = 1;
	    message[0] = '\0';
	    break;
	}

	/*
	 *  Start of list output?
	 */

	if (sscanf(line, "-- %s list: %d nam%c", listname, &listsize, &dummy) == 3) {
	    if (listsize == 0) {
		SendToIcs("tell %s The %s list is empty.\n", appData.owner, listname);
	    } else {
		state = PARSING_LIST;
		listmembers[0] = '\0';
		listindex = 0;
	    }
	    break;
	}

	/*
	 *  Accept incoming matches
	 */

	if (sscanf(line, "Challenge: %s ", name) == 1) {
	    if (appData.limitRematches &&
		!strcmp(lastPlayer, name) &&
		numGamesInSeries > appData.limitRematches &&
		time(0) - timeOfLastGame < 60) {
		
		SendToIcs("tell %s You have played me %d times in a row;  I'll wait a minute for other players to get a chance to challenge me before I accept your challenge.\n", name, numGamesInSeries);
		alarm(60);
		signal(SIGALRM, FindChallenge);
	    } else {
		SendToIcs("accept %s\n", name);
	    }
	    break;
	}

	/*
	 *  If we flagged opponent and he is not out of time, setup alarm for
	 *  courtesyadjourning after x minutes
	 */
	if (!strncasecmp(line, "Opponent is not out of time.", 28)) {
	    signal(SIGALRM, CourtesyAdjourn);
	    alarm(3*60);  /* lets give him 3 minutes */
	    break;
	}
	
	/*
	 *  Detect start of game, find gametype and rating of opponent
	 */

	if (sscanf(line, "Creating: %s %s %s %s %*s %s %c", 
		   name, rating, name2, rating2, tmp, &dummy) == 6) {

	    logme(LOG_DEBUG, "Detected start of game: %s (%s) vs %s (%s)", name, rating, name2, rating2);

	    if (runData.computerActive && !appData.killEngine && !appData.haveCmdNew) {
		/* wait for it to exit */
		if (wait(NULL) == -1 && errno != ECHILD)
		    ExitOn("wait() failed!");
		runData.computerActive = FALSE;
		close(runData.computerReadFd);
		close(runData.computerWriteFd);
	    }

	    if (!runData.computerActive)
		StartComputer();

	    /* reset movelist and ask for the moves */
	    moveList[0] = '\0';
	    if (appData.longAlgMoves)
		SendToIcs("moves l\n");
	    else
		SendToIcs("moves\n");

	    /* update our state */
	    state = PARSING_MOVELIST;
	    runData.waitingForFirstBoard = TRUE;

	    if (appData.haveCmdType)
		SendToComputer("type %s\n", tmp);
	    
	    /* zippy 2 */
	    if (strcmp(tmp, "blitz") && strcmp(tmp, "standard") && strcmp(tmp, "lightning"))
		SendToComputer("variant %s\n", tmp);
	    
	    break;
	}
      
	/*
	 *  Parse the board
	 */
    
	if (sscanf(line, "<12> %*72c%c%*d%*d%*d%*d%*d%*d%*d%*s%*s%*d%*d%*d%*d%*d%d%d%d%s%*s%s", &onMove, &whitetime, &blacktime, &moveNum, lan, san) == 6) {
	    if (moveNum != runData.moveNum &&
		((onMove == 'W' && runData.computerIsWhite) || 
		 (onMove == 'B' && !runData.computerIsWhite))) {

		runData.moveNum = moveNum;

		logme(LOG_INFO, "Move from ICS: %s %s", lan, san);
		if ((runData.computerIsWhite && blacktime <= 0) || 
		    (!runData.computerIsWhite && whitetime <= 0)) {
		    Flag(0);
		}
	       
		/* Reset alarm */
		alarm(0);
		signal(SIGALRM, SIG_IGN);

		if (moveNum > 1 && !appData.craftyMode) 
		    InterruptComputer();
		if (appData.longAlgMoves) {
		    ConvIcsSpecialToComp(lan);
		    strcpy(move, lan);
		} else {
		    ConvIcsSanToComp(san); 
		    strcpy(move, san);
		}
		/* HERE <--------------------------------------------------------> */
		/* always use long-alg */
		prom_move = 0;
		for (x=0; san[x] != '\0'; x++) {
		  if (san[x]=='=') {
		    prom_move = 1;
		    break;
		  }
		}
		if (prom_move) ConvIcsPromToComp(lan, san[x+1]);
		else ConvIcsSpecialToComp(lan);
		strcpy(move, lan);
		/* end */
		whitetime *= 100; blacktime *= 100;
		SendToComputer("time %d\notim %d\n%s\n", 
			       ((runData.computerIsWhite) ? whitetime : blacktime),
			       ((runData.computerIsWhite) ? blacktime : whitetime),
			       move);
		
	    } else {

		/* It's the opponent to move -> start 'flag' timer */
		i = runData.computerIsWhite ? blacktime : whitetime;
		if (i <= 0) {
		    Flag(0);
		} else {
		    signal(SIGALRM, Flag);
		    alarm(i+1);
		}
	    }

	    break;
	}
	break;

    case PARSING_MOVELIST:
	/*
	 *  Parse the first board;  determine colour, opponent, time controls
	 */
    
	if (runData.waitingForFirstBoard && !strncmp(line, "<12> ", 5)) {
	    if (sscanf(line, "<12> %72c%c%*d%*d%*d%*d%*d%*d%*d%s%s%*d%d%d%*d%*d%d%d%*d%s", tmp, &onMove, name, name2, &basetime, &inctime, &whitetime, &blacktime, move) == 9){
		/* 
		 *  Is this a non standard board?  If so, setup the board of the
		 *  chess program.
		 */
		if (!strcmp(move, "none") &&
		    strcmp(tmp, "rnbqkbnr pppppppp -------- -------- -------- -------- PPPPPPPP RNBQKBNR ")) {
		    /* Start edit mode and clear the board */
		    SendToComputer("edit\n#\n");
		    color = WHITE;
		    for (i = 0; i < 8; i++) {
			for (j = 0; j < 8; j++) {
			    c = tmp[i*8 + i%8 + j];
			    if (islower(c)) {
				if (color == WHITE) {
				    SendToComputer("c\n");
				    color = BLACK;
				}
			    } else if (isupper(c)) {
				if (color == BLACK) {
				    SendToComputer("c\n");
				    color = WHITE;
				}
			    }
			    if (c != '-' && c != ' ')
				SendToComputer("%c%c%c\n", tolower(c), 'a' + j, '8' - i);
			}
		    }
		    if (appData.craftyMode) {
			SendToComputer(".\n");
		    } else {
			SendToComputer(".\nbd\n");
		    }
		}

		/* Reset alarm */
		alarm(0);
		signal(SIGALRM, SIG_IGN);

		if (!strcmp(name, runData.handle)) {
		    logme(LOG_INFO, "I'm playing white.");
		    runData.computerIsWhite = TRUE;
		    oppname = name2;
		} else {
		    logme(LOG_INFO, "I'm playing black.");
		    runData.computerIsWhite = FALSE;
		    oppname = name;
		}

		/* Setup flag handler if nessesary */
		if ((onMove == 'W' && !runData.computerIsWhite) || 
		    (onMove == 'B' && runData.computerIsWhite)) {
		
		    i = runData.computerIsWhite ? blacktime : whitetime;
		    if (i <= 0) {
			Flag(0);
		    } else {
			signal(SIGALRM, Flag);
			alarm(i+1);
		    }
		}
		
		/* 
		 *  Setup chessprogram with the right options.
		 *  I think I finally got this right
		 */
		if (!appData.secPerMove && !appData.searchDepth) {
		    SendToComputer("level 0 %d %d\n", basetime, inctime);
		} else {
		    if (appData.searchDepth) {
			SendToComputer("depth\n%d\n", appData.searchDepth);
		    }
		}
		if (!appData.craftyMode) {
		    if (!appData.easyMode) 
			SendToComputer("hard\n");
		    else
			SendToComputer("hard\neasy\n");
		    if (!appData.book) 
			SendToComputer("book\n");
		    SendToComputer("random\n");
		}
		whitetime *= 100; blacktime *= 100;
		SendToComputer("time %d\notim %d\n", 
			       ((runData.computerIsWhite) ? whitetime : blacktime),
			       ((runData.computerIsWhite) ? blacktime : whitetime));
		if (appData.craftyMode && rating[0] != '\0') {
		    /* Cut off "(" and ")" */
		    stringcopy(rating, rating+1, strlen(rating)-2);
		    stringcopy(rating2, rating2+1, strlen(rating2)-2);
		    /* Are these really ratings? */
		    if (atoi(rating) && atoi(rating2)) {
			SendToComputer("name %s\n", oppname);
			SendToComputer("rating %s %s\n", 
				       (runData.computerIsWhite) ? rating : rating2,
				       (runData.computerIsWhite) ? rating2 : rating);
		    }
		    rating[0] = '\0';
		}
		runData.waitingForFirstBoard = FALSE;

		if (!strcmp(lastPlayer, oppname)) {
		    numGamesInSeries++;
		} else {
		    numGamesInSeries = 1;
		    strcpy(lastPlayer, oppname);
		}

		if (appData.limitRematches && 
		    numGamesInSeries > appData.limitRematches) {
		    SendToIcs("kibitz Last game of the series!\n");
		}
	    } else {
		ExitOn("Failed to parse style12");
	    }
	    break;
	}

	/*
	 *  Retrieve the movelist
	 */

	if (sscanf(line, "%*3d. %s (%*s %s (%*s", move, move2) == 2) {
	    onMove = 'W';
	    if (appData.longAlgMoves) {
		ConvIcsLanToComp(move);
		ConvIcsLanToComp(move2);
	    } else {
		ConvIcsSanToComp(move);
		ConvIcsSanToComp(move2);
	    }
	    sprintf(moveList+strlen(moveList), "%s\n%s\n", move, move2);
	    break;
	} else if (sscanf(line, "%*3d. %s (%*s %s (%*s", move, move2) == 1) {
	    onMove = 'B';
	    if (appData.longAlgMoves)
		ConvIcsLanToComp(move);
	    else
		ConvIcsSanToComp(move);
	    sprintf(moveList+strlen(moveList), "%s\n", move);
	    break;
	} 

	/*
	 *  Write movelist to program and prepare it to play
	 */
	if (!strncmp(line, "       {Game in progress} *", 27)) {
	  logme(LOG_INFO, "Found end of movelist; %c on move", onMove);

	  /* Convert to long notation */
	  ConvShortToLong(moveList, tmpmoveList);

	  SendToComputer("force\n%s", tmpmoveList);
	  if (onMove == 'W' && runData.computerIsWhite) {
	    SendToComputer("white\ngo\n");
	  } else if (onMove == 'W' && !runData.computerIsWhite) {
	    SendToComputer("white\n");
	    SendToComputer("playother\n");
	  } else if (onMove == 'B' && runData.computerIsWhite) {
	    SendToComputer("black\n");
	    SendToComputer("playother\n");
	  } else {	    
	    SendToComputer("black\ngo\n");
	  }

	  state = NORMAL;
	  break;
	}
	break;

    case PARSING_MESSAGELIST:

	sprintf(tmp, "%2d. ", messageindex);
	if (line[0] == '\\') {
	    strcat(message, line + 3);
	} else if (!strncmp(line, tmp, strlen(tmp))) {
	    strcpy(message, line);
	} else {
	    logme(LOG_INFO, "Forwarding message to owner: %s", message);
	    SendToIcs("tell %s %s\n", appData.owner, message);
	    /* Was this the last one? */
	    sprintf(tmp, "%2d. ", ++messageindex);
	    if (!strncmp(line, tmp, strlen(tmp))) {
		message[0] = '\0';
		strcpy(message, line);
	    } else {
		logme(LOG_DEBUG, "Finished parsing personal messages.");
		state = NORMAL;
	    }
	}
	break;

    case PARSING_LIST:

	p = strtok(line, " \r\n");
	while (listindex < listsize && p) {
	    strcat(listmembers, p);
	    strcat(listmembers, ",");
	    p = strtok(NULL, " \r\n");
	    listindex++;
	}
    
	if (listindex == listsize) {
	    /* We are done, send it to the owner */
	    listmembers[strlen(listmembers)-1] = '\0';
	    while (strlen(listmembers)+25 >= 1024) {
		/* Too big for one tell; we must split it up */
		stringcopy(tmp, listmembers, 1024-25);
		p = strrchr(tmp, ',');
		*p = '\0';
		SendToIcs("tell %s %s\n", appData.owner, tmp);
		bytecopy(listmembers, listmembers + (p-tmp) + 1, strlen(listmembers) - (p-tmp));
	    }
	    SendToIcs("tell %s %s\n", appData.owner, listmembers);

	    state = NORMAL;
	}

	break;

    case PARSING_PENDING:

	if (sscanf(line, " %d: %s is offering a challeng%c", &offer, name, &dummy) == 3) {
	    
	    if (strcmp(name, lastPlayer)) {
		SendToIcs("accept %d\n", offer);
	    } else {
		backupoffer = offer;
	    }
	    break;
	}

	if (!strncmp(line, "If you wish to accept any of these offers type", 46)) {
	    
	    if (backupoffer != -1)
		SendToIcs("accept %d\n", backupoffer);

	    logme(LOG_DEBUG, "Detected end of pending offers list.");
	    state = NORMAL;
	    break;
	}

	break;
    }

    /*
     *  Fetch the game ID when a game is started.
     */
    
    if (!strncmp(line, "{Game ", 6) &&
	strstr(line, runData.handle) &&
	(strstr(line, ") Creating ") || strstr(line, ") Continuing "))) {
	
	sscanf(line, "{Game %d", &runData.gameID);
	logme(LOG_INFO, "Current game has ID: %d", runData.gameID);
    } 

    /* 
     *  Check for game-end messages no matter what state we are in.
     *  We dont care how the game ended, just that it ended.
     */

    
    if (sscanf(line, "{Game %*s %*s %*s %*s Creating %*s %*s %c}", &dummy)==1)  
      ; 
    else if (sscanf(line, "{Game %*s %*s %*s %*s Continuing %*s %*s %c}", &dummy)==1) ; 	 
    else
    if (!strncmp(line, "{Game ", 6) && 
	strstr(line, runData.handle) &&
	(strstr(line, "1-0") || strstr(line, "0-1") || strstr(line, "1/2-1/2") || strstr(line, "*"))) {

	logme(LOG_INFO, "Detected end of game: %s", line);

	runData.gameID = -1;
	runData.moveNum = -1;

	timeOfLastGame = time(0);

	if (!appData.craftyMode) {
	    InterruptComputer();
	}

	if (appData.haveCmdPing) {
	    PingComputer();
	}

	if (appData.haveCmdResult) {
	    SendToComputer("result %s\n", strstr(line, "1-0") ? "1-0" : 
			   (strstr(line, "0-1") ? "0-1" :
			    (strstr(line, "1/2-1/2") ? "1/2-1/2" : "*")));
	}

	if (!appData.haveCmdNew) {
	    if (appData.killEngine) {
		KillComputer();
	    } else {
		SendToComputer("quit\nexit\n");
	    }
	} else {
	    SendToComputer("\nforce\nnew\n");
	}
	state = NORMAL;
    
	if (runData.quitPending) {
	    if (runData.exitValue == EXIT_RESTART)
		ExitOn("Restart command executed.");
	    else		
		ExitOn("SoftQuit command executed.");
	}

	if (appData.sendGameEnd)
	    SendToIcs("%s\n", appData.sendGameEnd);

	if (appData.sendTimeoutCmds) {
	    alarm(appData.sendTimeoutCmds * 60);
	    signal(SIGALRM, IdleTimeout);
	} else {
	    alarm(0);
	    signal(SIGALRM, SIG_IGN);
	}
    }
}

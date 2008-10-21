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
#include "argparser.h"

typedef enum { ArgNull = 0, ArgString, ArgInt, ArgBool } ArgType;

typedef struct {
    char    *argName;
    ArgType  argType;
    void    *argValue;
} ArgTuple;

typedef ArgTuple ArgList[]; 

ArgList argList = { 
    {"-icsHost",         ArgString, &appData.icsHost         }, 
    {"-icsPort",         ArgInt,    &appData.icsPort         },
    {"-searchDepth",     ArgInt,    &appData.searchDepth     },
    {"-secPerMove",      ArgInt,    &appData.secPerMove      },
    {"-logLevel",        ArgInt,    &appData.logLevel        },
    {"-logFile",         ArgString, &appData.logFile         },
    {"-owner",           ArgString, &appData.owner           },
    {"-console",         ArgBool,   &appData.console         },
    {"-easyMode",        ArgBool,   &appData.easyMode        },
    {"-book",            ArgBool,   &appData.book            },
    {"-craftyMode",      ArgBool,   &appData.craftyMode      },
    {"-program",         ArgString, &appData.program         },
    {"-longAlgMoves",    ArgBool,   &appData.longAlgMoves    }, 
    {"-sendGameEnd",     ArgString, &appData.sendGameEnd     },
    {"-haveCmdNew",      ArgBool,   &appData.haveCmdNew      },
    {"-haveCmdType",     ArgBool,   &appData.haveCmdType     },
    {"-haveCmdResult",   ArgBool,   &appData.haveCmdResult   },
    {"-sendTimeoutCmds", ArgInt,    &appData.sendTimeoutCmds },
    {"-killEngine",      ArgBool,   &appData.killEngine      },
    {"-limitRematches",  ArgInt,    &appData.limitRematches  },
    {"-autoConfigure",   ArgBool,   &appData.autoConfigure   },
    {"-haveCmdPing",     ArgBool,   &appData.haveCmdPing     },
    {NULL, ArgNull, NULL} 
};

int ParseArgs(int argc, char* argv[])
{
    int i, j, flag, k;

    for (j = 1; (j < argc); j++) {
	flag = 0;
	for (i = 0; (argList[i].argType != ArgNull && !flag); i++) {
	    if (!strcmp(argv[j], argList[i].argName)) {
		flag = 1;
		switch(argList[i].argType) {
		case ArgString:
		    j++;
		    if (!argv[j] || argv[j][0] == '-') {
			fprintf(stderr, "Missing value for \"%s\"\n", argv[j-1]);
			return ERROR;
		    } else {
#if 0 
			fprintf(stderr, "%s %s\n", argv[j-1], argv[j]);
#endif
			/* Turn '\' + 'n' into ' ' + '\n' */
			for (k = 0; k < strlen(argv[j]) - 1; k++) {
			    if (argv[j][k] == '\\' && argv[j][k+1] == 'n') {
				argv[j][k] = ' ';
				argv[j][k+1] = '\n';
			    }
			}

			*((char **) argList[i].argValue) = strdup(argv[j]);
		    }
		    break;
		case ArgInt:
		    j++;
		    if (!argv[j] || argv[j][0] == '-') {
			fprintf(stderr, "Missing value for \"%s\"\n", argv[j-1]);
			return ERROR;
		    } else {
			int a;
			for (a = 0; (argv[j][a] && isdigit(argv[j][a])); a++)
			    ;
			if (argv[j][a] != '\0') {
			    fprintf(stderr, "Bad value given for \"%s\"\n", argv[j-1]);
			    return ERROR;
			} else {
#if 0
			    fprintf(stderr, "%s %s\n", argv[j-1], argv[j]);
#endif
			    *((int *) argList[i].argValue) = atoi(argv[j]);
			}
		    }
		    break;
		case ArgBool:
		    j++;
		    if (!argv[j] || argv[j][0] == '-') {
			fprintf(stderr, "Missing value for \"%s\"\n", argv[j-1]);
			return ERROR;
		    } else {
			if (!strcasecmp(argv[j], "true") || 
			    !strcasecmp(argv[j], "on"))
			    *((int *) argList[i].argValue) = TRUE;
			else if (!strcasecmp(argv[j], "false") || 
				 !strcasecmp(argv[j], "off"))
			    *((int *) argList[i].argValue) = FALSE;
			else {
			    fprintf(stderr, "Bad value given for \"%s\"\n", argv[j-1]);
			    return ERROR;
			}
		    }
		    break;
		case ArgNull:
		default:
		    fprintf(stderr, "Internal error: Wrong argparser configuration!");
		    return ERROR;
		}
	    }
	}
	if (!flag) {
	    fprintf(stderr, "Unknown option \"%s\"\n", argv[j]);
	    return ERROR;
	}
    }
    return OK;
}

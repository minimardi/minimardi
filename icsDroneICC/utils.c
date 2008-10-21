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
#include "utils.h"
#include "main.h"
#define MAX_MOVE_BUFFER_SIZE 4000

/* returns 1: normal long alg. move, 2: promotion move*/
int isMove(char *input){
  if (!input) return 0;
  if (input[0] == '\0' || input[1] == '\0' || input[2] == '\0' || input[3] == '\0')
    return 0;
  if (input[0] >= 97 && input[0] <= 104 && input[1] >=49 && input[1] <= 56 &&
      input[2] >= 97 && input[2] <= 104 && input[3] >=49 && input[3] <= 56 )
    {
      if ((tolower(input[4]) == 'q') ||(tolower(input[4]) == 'r') ||
	  (tolower(input[4]) == 'b') || (tolower(input[4]) == 'n'))
	return 2;
      return 1;
    }
  return 0;
  
}

/* Convert from short to long notation, using the program pgn-extract, 
   pgn-extract should  be in the same directory as icsdrone. 
   short_not is a series of moves, long_not is a buffer to output the
   long alg. moves.
*/
void ConvShortToLong(char *short_not, char *long_not){
  char buff[MAX_MOVE_BUFFER_SIZE], buff2[MAX_MOVE_BUFFER_SIZE];
  int i=0, j=5, c, ln=0;
  char *command = "| pgn-extract -Wlalg -C -otmp.pgn -s";
  FILE *long_file;
  buff[0]='e';
  buff[1]='c';
  buff[2]='h';
  buff[3]='o';
  buff[4]=' ';
  while(short_not[i] != '\0') {
   if (short_not[i] != '\n') {
     buff[j] = short_not[i]; 
     i++;
     j++;
   }
   else {
     buff[j] = ' ';
     i++;
     j++;
   }
  }
  buff[j] = ' ';
  j++;
  buff[j] = '1';
  buff[j+1] = '-';
  buff[j+2] = '0';
  j = j + 3;
  i=0;
  while(command[i] != '\0'){
    buff[j] = command[i];
    i++;
    j++;
  }
  system(buff);
  long_file=fopen("tmp.pgn", "r");
  if (!long_file) {
    printf("Probably didn't find Extract\n");
    exit(1);
  }
  fseek(long_file, 0, SEEK_SET);
  for (j=0; j < 2000; j++){
    if ((c=fgetc(long_file)) == EOF)
      break;
    buff2[j] = (char) c;
  }
  fclose(long_file);
  j=0;
  while (buff2[j]=='[') {
    while (buff2[j] != ']') j++;
    j++;
    if (buff2[j]=='\n') j++;
  }
  if (buff2[j]=='\n') j++;
  while (buff2[j] != '\0') {
    if (isMove(buff2+j)==1) {
      long_not[ln]=buff2[j];
      ln++;
      long_not[ln]=buff2[j+1];
      ln++;
      long_not[ln]=buff2[j+2];
      ln++;
      long_not[ln]=buff2[j+3];
      ln++;
      long_not[ln]='\n';
      ln++;
    } 
    else if (isMove(buff2+j)==2) { /* promotion */
      long_not[ln]=buff2[j];
      ln++;
      long_not[ln]=buff2[j+1];
      ln++;
      long_not[ln]=buff2[j+2];
      ln++;
      long_not[ln]=buff2[j+3];
      ln++;
      long_not[ln]=buff2[j+4];
      ln++; 
      long_not[ln]='\n';
      ln++;
    }
    j++;
  }
  ln++;
  long_not[ln]='\0';
}

void bytezero(void *dest, int n)
{
//    int i;
//		
//    for (i = 0; (i < n); i++)
//	*(((u_char *) dest)++) = '\0';
// 
	memset((u_char *)dest,'\0',n);

}

void bytecopy(void *dest, const void *src, int n)
{

//    int i;
//
//    for (i = 0; (i < n); i++)
//	*(((u_char *) dest)++) = *(((const u_char *) src)++);
	memcpy((u_char *)dest,(const u_char *)src,n);
}

void stringcat(void *dest, const void *src, int n)
{
/*
    for ( ; (*((u_char *) dest) != '\0'); ((u_char *) dest)++)
	;

    stringcopy(dest, src, n);
*/

	strncat((u_char *)dest, (u_char *)src, n);
}

void stringcopy(void *dest, const void *src, int n)
{
/*
    int i;
  
    for (i = 0; (i < n); i++)
	*(((u_char *) dest)++) = *(((u_char *) src)++);

    *((u_char *) dest) = '\0';
*/
 	//strncpy((u_char *)dest,(const u_char *)src,n);
	//*(((u_char *) dest) + n) = '\0';
	int i;
	for (i=0; i<n; i++) {
		*(((u_char *)dest)+i) =  *(((u_char *)src)+i);
	}
	*(((u_char *) dest)+n) = '\0';
	
}  

void ConvCompToIcs(char *move)
{
    if (strlen(move) == 5 && (move[4] == 'q' || move[4] == 'r' || 
			      move[4] == 'b' || move[4] == 'n' ||
			      move[4] == 'Q' || move[4] == 'R' ||
			      move[4] == 'B' || move[4] == 'N' ||
			      move[4] == 'k' || move[4] == 'K')) {
	move[6] = '\0';
	move[5] = move[4];
	move[4] = '=';
    }
}

/*  Converts move from ICS LAN to COMPUTER LAN  */
void ConvIcsLanToComp(char *move)
{
    /*  frfr=P --> frfr=p  */
  if (move[4] == '=') {
    move[4] = tolower(move[5]);
    move[5] = '\0';
  }
}

/*  Converts move from ICS SAN to COMPUTER SAN  */
void ConvIcsSanToComp(char *move)
{
    /*  Only castling seems to be wrong here  */
    if (!strncmp("O-O-O", move, 5)) 
	move[0] = move[2] = move[4] = 'o';
    if (!strncmp("O-O", move, 3)) 
	move[0] = move[2] = 'o';
}


void ConvIcsPromToComp(char *move, char prom_piece)
{
    /*  P/fr-fr=P --> frfr=p  */
  if (!strncmp("o-o-o", move, 5)) return;
  if (!strncmp("o-o", move, 3)) return;
  if (!strncmp("O-O-O", move, 5)) return;
  if (!strncmp("O-O", move, 3)) return;
    move[0] = move[2];
    move[1] = move[3];
    move[2] = move[5];
    move[3] = move[6];
    move[4] = tolower(prom_piece);
    move[5] = '\0';
      
}

/*  Converts move from ICS SPECIAL to COMPUTER LAN  */
void ConvIcsSpecialToComp(char *move)
{
    /*  P/fr-fr=P --> frfr=p  */
  if (!strncmp("o-o-o", move, 5)) return;
  if (!strncmp("o-o", move, 3)) return;
  if (!strncmp("O-O-O", move, 5)) return;
  if (!strncmp("O-O", move, 3)) return;
    move[0] = move[2];
    move[1] = move[3];
    move[2] = move[5];
    move[3] = move[6];
    if (move[7] == '=') {
      move[4] = tolower(move[8]);
      move[5] = '\0';
    } else {
      move[4] = '\0';
    }
}

static FILE* logfile = NULL;
#if defined(ZLIB_VERSION)
static gzFile gzlogfile = NULL;
#endif

void StartLogging()
{
    if (appData.logFile) {
	if (!strcmp(&appData.logFile[strlen(appData.logFile) - 3], ".gz")) {
#if defined(ZLIB_VERSION)
	    gzlogfile = gzopen(appData.logFile, "wb9");
#else
	    ExitOn("Can't log to .gz as icsDrone wasn't linked with zlib.");
#endif
	} else {
	    logfile = fopen(appData.logFile, "w");
	}
    }
}

void StopLogging()
{
    if (logfile) {
	fclose(logfile);
#if defined(ZLIB_VERSION)
    } else if (gzlogfile) {
	gzclose(gzlogfile);
#endif
    }
}

void logme(LogType type, const char *format, ... )
{
    static const char* logtypestr[] = { "", "ERROR", "WARNING", "INFO", "CHAT", "DEBUG" };
    char buf[1024] = "";
    char out[2048] = "";
    char *p;
    time_t t = time(0);
    va_list ap;
    char *timestring = ctime(&t);
    int i;

    /* get rid of the '\n' - ctime() must be on the top ten of worst 
     * standard functions */
    timestring[strlen(timestring)-1] = '\0';

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    /* make sure te buf ends with a \n */
    if (buf[strlen(buf) - 1] != '\n')
	strcat(buf, "\n");

    /* write them out line by line */
    for (p = buf, i = 0; i < strlen(p); ) {
	if (p[i] == '\n') {
	    p[i] = '\0';
	    sprintf(out, "%s:%s:%s\n", timestring, logtypestr[type], p);
	    p = &p[i];
	    i = 0;

	    if (logfile)
		fprintf(logfile, out);
#if defined(ZLIB_VERSION)
	    else if (gzlogfile)
		gzputs(gzlogfile, out);
#endif
	    else 
		fprintf(stderr, out);

	} else {
	    i++;
	}
    }
}

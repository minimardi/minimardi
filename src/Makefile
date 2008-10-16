CC	= gcc 
CFLAGS 	= -Wall -O3 -fno-stack-protector
PROFILE = -pg
OBJECTS =  board.o position.o search.o move.o piece.o eval.o move_list.o book.o score.o main.o hist.o trans.o utils.o
EXE 	= minimardi
MAKE 	= make

$(EXE) : $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXE)

main.o : main.c main.h
board.o : board.c piece.h
search.o : search.c position.h eval.h book.h trans.h move.h
position.o : position.c position.h board.h
eval.o : eval.c position.h piece.h
move.o : move.c piece.h board.h move.h
move_list.o : move_list.c move.h
piece.o : piece.c piece.h
book.o : book.c position.h move_list.h move.h
score.o : score.c score.h
hist.o: hist.c move.h
trans.o: position.h
utils.o:
clean : 
	rm *.o 
	rm -f *~	
	rm $(EXE)	
profile :
	$(MAKE) CFLAGS="$(CFLAGS) $(PROFILE)"	
debug :
	$(MAKE) CFLAGS="$(CFLAGS) -g"	

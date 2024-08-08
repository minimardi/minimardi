 miniMardi 1.3 - Messy Chess Program
 -----------------------------------

This is a "oh no not another simple chess program" chess program.
It's written in C and works under Unix and Windows.

Features:
 - Opening book (small, limited by the simple search).
 - AlphaBeta search. 
 - Time management. 
 - Search heuristics (history moves).
 - Draw aware (3-fold repetition).
 - FEN support.
 - xboard compatible.
 - Zippy compatible.

Implementation Details:
 - 0x88 board representation.
 - zobrist keys for repetition detection.
 - moves encoded into an INT.
 - move sorting (mvv/lva), history moves.
 - AlphaBeta search with Quiescent.

New for this release:
 - Null move.
 - Transposition table.
 - Option for search: MTD, PV.
 - Almost no global variables.

Thanks to:
 - Bruce Moreland for his excellent site on computer chess.
 - Marcel van Kervinck, MSCP.
 - Computer Chess Club.
 - Opening book, Unknown author.

Copying:
 See file COPYING. 
   Juan Pablo Fernandez 

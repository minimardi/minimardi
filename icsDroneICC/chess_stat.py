import sys
import sqlite
import optparse
import re


def main(argv):
    p = optparse.OptionParser()
    p.add_option('--clean', '-c', action='store_true', help="Start with a new database", dest="clean", default=False)
    p.add_option('--game', '-g', dest='game', help="A pgn game result, eg {Game 1367 (minimardi vs. GreekPhilosophy) GreekPhilosophy resigns} 1-0}")
    p.add_option('--engine', '-e', dest='engine', help="Engine name")

    options, arguments = p.parse_args()
    con = sqlite.connect('chess_stat.db')
    cur = con.cursor()
    if (options.clean):
        print "Creating new db ..."
        cur.execute('CREATE TABLE stat (name VARCHCHAR(20), wins INTEGER, draws INTEGER, losses INTEGER)') 
        con.commit()
    elif (options.game and options.engine):
        #print options.game 
        p = re.compile('\{Game (\S+) \((\S+) vs. (\S+)\) ([^\}]+)\} ([^\}]+)\}')
        m = p.search(options.game)
        if (m):
            print m.group(3)
            result = m.group(5)
            p1 = m.group(2) #White
            p2 = m.group(3) #Black
            isWhite = False
            if (p1 == options.engine):
                isWhite = True
            #cur.execute('SELECT * FROM stat where NAME=');
            #print cur.fetchall()
            wins = 0
            draws = 0
            losses = 0
            score = 0
            if (result=="1-0"):
                if (isWhite):
                    losses+=1
                else: 
                    wins+=1
            elif (result=="0-1"):
                if (isWhite):
                    wins+=1
                else:
                    losses+=1
            elif (result=="1/2-1/2"):
                draws+=1
            
            score = wins - losses
                 
if __name__ == "__main__":
        main(sys.argv[1:])

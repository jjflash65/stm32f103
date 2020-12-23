/* -------------------------------------------------------------
                          reversi_ki.c

     Die KI zum Spiel Reversi / Othello

     Ausgangsprojekt war ein Quellcode fuer Linux-Konsole
     mit (leider) unbekanntem Author.

     30.10.2019  R. Seelig
   ------------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>

#include "reversi_ki.h"

  /* -------------------------------------------------------------
                             Prototypen
     ------------------------------------------------------------- */
  int human (void);
  int maxdiffstrategy3(int, int *);
  int maxweighteddiffstrategy1(int, int *);
  int maxweighteddiffstrategy3(int, int *);


/* -------------------------------------------------------------
          globale Variable und Konstante der original KI
   ------------------------------------------------------------- */

  const int ALLDIRECTIONS[8]={-11, -10, -9, -1, 1, 9, 10, 11};
  const int BOARDSIZE=100;

  const int EMPTY = 0;
  const int WHITE = 1;
  const int BLACK = 2;
  const int OUTER = 3;             // the value of a square on the perimeter

  const int WIN   = 2000;          // a WIN and LOSS for player are outside
  const int LOSS  = -2000;         // the range of any board evaluation function */

  long int BOARDS;


  void * STRATEGIES[5][4] =
  {
    {"human", "human", human},
    {"diff3", "maxdiff, 3-move minmax lookahead", maxdiffstrategy3},
    {"wdiff1", "maxweigteddiff, 1-move minmax lookahead", maxweighteddiffstrategy1},
    {"wdiff3", "maxweigteddiff, 3-move minmax lookahead", maxweighteddiffstrategy3},
    {NULL, NULL, NULL}
  };

  /* ---------------------------------------------------------------------
                          Originale Funktionen der KI
     ---------------------------------------------------------------------

       the possible square values are integers (0-3), but we will
       actually want to print the board as a grid of symbols,
       . instead of 0, b instead of 1, w instead of 2, and ? instead
       of 3 (though we might only print out this latter case for
       purposes of debugging).
     --------------------------------------------------------------------- */
  char nameof (int piece)
  {
    static char piecenames[5] = ".OX?";

    return(piecenames[piece]);
  }

  /* --------------------------------------------------------------
     if the current player is WHITE (1), then the opponent is BLACK (2),
     and vice versa
     -------------------------------------------------------------- */
  int opponent (int player)
  {
    switch (player)
    {
      case 1  : return 2;
      case 2  : return 1;
      default : return 0;
    }
  }

  /* --------------------------------------------------------------
     The copyboard function mallocs space for a board, then copies
     the values of a given board to the newly malloced board.
     -------------------------------------------------------------- */
  int *copyboard (int * board)
  {
    int i, * newboard;

    newboard = (int *)malloc(BOARDSIZE * sizeof(int));
    for (i= 0; i< BOARDSIZE; i++) newboard[i] = board[i];
    return newboard;
  }


  /* --------------------------------------------------------------
     the initial board has values of 3 (OUTER) on the perimeter,
     and EMPTY (0) everywhere else, except the center locations
     which will have two WHITE (1) and two BLACK (2) values.
     -------------------------------------------------------------- */
  int *initialboard (void)
  {
    int i, * board;

    board = (int *)malloc(BOARDSIZE * sizeof(int));
    for (i = 0; i<= 9; i++) board[i]=OUTER;
    for (i = 10; i<= 89; i++)
    {
      if (i%10 >= 1 && i%10 <= 8) board[i]= EMPTY;
                             else board[i]= OUTER;
    }
    for (i = 90; i<= 99; i++) board[i]=OUTER;
    board[44]= BLACK; board[45]= WHITE; board[54]= WHITE; board[55]= BLACK;
    return board;
  }


  /* --------------------------------------------------------------
     count the number of squares occupied by a given player (1 or 2,
     or alternatively WHITE or BLACK)
     -------------------------------------------------------------- */
  int count (int player, int * board)
  {
    int i, cnt;

    cnt= 0;
    for (i= 1; i<= 88; i++)
      if (board[i] == player) cnt++;
    return cnt;
  }



  /* ---------------------------------------------------------------------
                     Routines for insuring legal play
     ---------------------------------------------------------------------
       The code that follows enforces the rules of legal play for Othello.
     --------------------------------------------------------------------- */

  int validp (int move)
  // a "valid" move must be a non-perimeter square
  {
    if ((move >= 11) && (move <= 88) && (move%10 >= 1) && (move%10 <= 8))
       return 1;
    else return 0;
  }


  int findbracketingpiece(int square, int player, int * board, int dir)
  {
    while (board[square] == opponent(player)) square = square + dir;
    if (board[square] == player) return square;
                            else return 0;
  }

  int wouldflip (int move, int player, int * board, int dir)
  {
    int c;

    c = move + dir;
    if (board[c] == opponent(player))
       return findbracketingpiece(c+dir, player, board, dir);
    else return 0;
  }

  int legalp (int move, int player, int * board)
  {
    int i;

    if (!validp(move)) return 0;
    if (board[move]==EMPTY)
    {
      i= 0;
      while (i<=7 && !wouldflip(move, player, board, ALLDIRECTIONS[i])) i++;
      if (i==8) return 0;
           else return 1;
    }
    else return 0;
  }


  void makeflips (int move, int player, int * board, int dir)
  {
    int bracketer, c;

    bracketer = wouldflip(move, player, board, dir);
    if (bracketer)
    {
      c = move + dir;
      do
      {
        board[c] = player;
        c = c + dir;
      } while (c != bracketer);
    }
  }

  void makemove (int move, int player, int * board)
  {
    int i;

    board[move] = player;
    for (i= 0; i<= 7; i++) makeflips(move, player, board, ALLDIRECTIONS[i]);
  }


  int anylegalmove (int player, int * board)
  {
    int move;

    move = 11;
    while (move <= 88 && !legalp(move, player, board)) move++;
    if (move <= 88) return 1
             ; else return 0;
  }

  int nexttoplay (int * board, int previousplayer)
  {
    int opp;
    opp = opponent(previousplayer);
    if (anylegalmove(opp, board)) return opp;
    if (anylegalmove(previousplayer, board))
    {
      status_meld(2);
      return previousplayer;
    }
    return 0;
  }


  int *legalmoves (int player, int * board)
  {
    int move, i, * moves;

    moves = (int *)malloc(65 * sizeof(int));
    moves[0] = 0;
    i = 0;
    for (move=11; move<=88; move++)
      if (legalp(move, player, board))
      {
        i++;
        moves[i]=move;
      }
    moves[0]= i;
    return moves;
  }

  /* ---------------------------------------------------------------------
                          Strategies for playing
     --------------------------------------------------------------------- */

  int human (void)
  // if a human player, then get the next move from standard input
  {
    int move;

    move= stone_gettoset();
    return move;
  }


  /* --------------------------------------------------------------
     diffeval and weighteddiffeval are alternate utility functions
     for evaluation the quality (from the perspective of player)
     of terminal boards in a minmax search.
     -------------------------------------------------------------- */
  int diffeval (int player, int * board)
  {                                        // utility is measured
    int i, ocnt, pcnt, opp;                // by the difference in
    pcnt=0; ocnt = 0;                      // number of pieces
    opp = opponent(player);
    for (i=1; i<=88; i++)
    {
      if (board[i]==player) pcnt++;
      if (board[i]==opp) ocnt++;
    }
    return (pcnt-ocnt);
  }

  int weighteddiffeval (int player, int * board)
  {
    int i, ocnt, pcnt, opp;

    const int weights[100]={ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                             0,120,-20, 20,  5,  5, 20,-20,120,  0,
                             0,-20,-40, -5, -5, -5, -5,-40,-20,  0,
                             0, 20, -5, 15,  3,  3, 15, -5, 20,  0,
                             0,  5, -5,  3,  3,  3,  3, -5,  5,  0,
                             0,  5, -5,  3,  3,  3,  3, -5,  5,  0,
                             0, 20, -5, 15,  3,  3, 15, -5, 20,  0,
                             0,-20,-40, -5, -5, -5, -5,-40,-20,  0,
                             0,120,-20, 20,  5,  5, 20,-20,120,  0,
                             0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

    pcnt=0; ocnt=0;
    opp = opponent(player);
    for (i=1; i<=88; i++)
    {
      if (board[i]==player) pcnt=pcnt+weights[i];
      if (board[i]==opp) ocnt=ocnt+weights[i];
    }
    return (pcnt - ocnt);
  }


  int minmax (int player, int * board, int ply, int (* evalfn) (int, int *))
  {
    int i, max, ntm, newscore, bestmove, * moves, * newboard;
    int maxchoice (int, int *, int, int (*) (int, int *));
    int minchoice (int, int *, int, int (*) (int, int *));

    moves = legalmoves(player, board);                     // get all legal moves for player
    max = LOSS - 1;                                        // any legal move will exceed this score
    for (i=1; i <= moves[0]; i++)
    {
      newboard = copyboard(board); BOARDS = BOARDS + 1;
      makemove(moves[i], player, newboard);
      ntm = nexttoplay(newboard, player);
      if (ntm == 0)
      {
        // game over, so determine winner
        newscore = diffeval(player, newboard);
        if (newscore > 0) newscore = WIN;                  // Spieler gewinnt
        if (newscore < 0) newscore = LOSS;                 // Computer gewinnt
      }
      if (ntm == player)                                   // Computer kann nicht ziehen
         newscore = maxchoice(player, newboard, ply-1, evalfn);
      if (ntm == opponent(player))
         newscore = minchoice(player, newboard, ply-1, evalfn);
      if (newscore > max)
      {
        max = newscore;
        bestmove = moves[i];                               // besseren Spielzug gefunden
      }
      free(newboard);
    }
    free(moves);
    return(bestmove);
  }

  int maxchoice (int player, int * board, int ply,
                 int (* evalfn) (int, int *))
  {

    int i, max, ntm, newscore, * moves, * newboard;
    int minchoice (int, int *, int, int (*) (int, int *));

    if (ply == 0) return((* evalfn) (player, board));
    moves = legalmoves(player, board);
    max = LOSS - 1;
    for (i=1; i <= moves[0]; i++)
    {
      newboard = copyboard(board); BOARDS = BOARDS + 1;
      makemove(moves[i], player, newboard);
      ntm = nexttoplay(newboard, player);
      if (ntm == 0)
      {
        newscore = diffeval(player, newboard);
        if (newscore > 0) newscore = WIN;
        if (newscore < 0) newscore = LOSS;
      }
      if (ntm == player)
         newscore = maxchoice(player, newboard, ply-1, evalfn);
      if (ntm == opponent(player))
         newscore = minchoice(player, newboard, ply-1, evalfn);
      if (newscore > max) max = newscore;
      free(newboard);
    }
    free(moves);
    return(max);
  }

  int minchoice (int player, int * board, int ply,
                 int (* evalfn) (int, int *))
  {
    int i, min, ntm, newscore, * moves, * newboard;

    if (ply == 0) return((* evalfn) (player, board));
    moves = legalmoves(opponent(player), board);
    min = WIN+1;
    for (i=1; i <= moves[0]; i++)
    {
      newboard = copyboard(board); BOARDS = BOARDS + 1;
      makemove(moves[i], opponent(player), newboard);
      ntm = nexttoplay(newboard, opponent(player));
      if (ntm == 0)
      {
        newscore = diffeval(player, newboard);
        if (newscore > 0) newscore = WIN;
        if (newscore < 0) newscore = LOSS;
      }
      if (ntm == player)
         newscore = maxchoice(player, newboard, ply-1, evalfn);
      if (ntm == opponent(player))
         newscore = minchoice(player, newboard, ply-1, evalfn);
      if (newscore < min) min = newscore;
      free(newboard);
    }
    free(moves);
    return(min);
  }




  /* --------------------------------------------------------------
     the following strategies use minmax search
     -------------------------------------------------------------- */

  int maxdiffstrategy3(int player, int * board)
  {
    return(minmax(player, board, 3, diffeval));
  }

  /* --------------------------------------------------------------
      use weigteddiffstrategy as utility function
     -------------------------------------------------------------- */
  int maxweighteddiffstrategy1(int player, int * board)
  {
     return(minmax(player, board, 1, weighteddiffeval));
  }

  int maxweighteddiffstrategy3(int player, int * board)
  {
     return(minmax(player, board, 3, weighteddiffeval));
  }


  /* -----------------------------------------------------------------
                       Coordinating matches
     ----------------------------------------------------------------- */

  /* --------------------------------------------------------------
     get the next move for player using strategy
     -------------------------------------------------------------- */
  void getmove (int (* strategy) (int, int *), int player, int * board)
  // Achtung, rekursive !
  {
    int move;

    printboard(board);
    move = (* strategy)(player, board);
    if (legalp(move, player, board))
    {
      makemove(move, player, board);
    }
    else
    {
      status_meld(1);                             // unerlaubter Zug
      getmove(strategy, player, board);
    }
  }


  uint8_t othello (int (* blstrategy) (int, int *),
                   int (* whstrategy) (int, int *))
  {
    int *board;
    int player;

    int wanz, banz;                                       // nimmt Anzahl der Steine auf, die zum
                                                          // Schluss auf dem Spielfeld liegen
    int result;

    board = initialboard();
    player = WHITE;                                       // hier ist angegeben, wer das Spiel beginnt
    do
    {
      if (player == WHITE) getmove(blstrategy, WHITE, board);
      else getmove(whstrategy, BLACK, board);
      player = nexttoplay(board, player);
    }
    while (player != 0);

    // das Spiel ist beendet
    printboard(board);
    wanz= count(WHITE, board);
    banz= count(BLACK, board);
    if (wanz == banz) result = 5;                         // unentschieden
    if (wanz > banz) result = 4;                          // Spieler gewinnt
    if (wanz < banz) result = 3;                          // CPU gewinnt

    return result;
  }

  uint8_t playgame (int difficult)
  {
    uint8_t result;

    int p1, p2;
    int (* strfn1)(int, int *);
    int (* strfn2)(int, int *);

    p1= 0;
    p2= difficult;

    strfn1 = STRATEGIES[p1][2]; strfn2 = STRATEGIES[p2][2];
    result= othello(strfn1, strfn2);
    return result;
  }


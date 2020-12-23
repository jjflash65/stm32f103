/***************************************************************************
**                                                                        **
**                          Connect-4 Algorithm                           **
**                                                                        **
**                              Version 3.11                              **
**                                                                        **
**                            By Keith Pomakis                            **
**                          (pomakis@pobox.com)                           **
**                                                                        **
**                             November, 2009                             **
**                                                                        **
****************************************************************************
**                                                                        **
**  This file provides the functions necessary to implement a front-end-  **
**  independent Connect-4 game.  Multiple board sizes are supported.      **
**  It is also possible to specify the number of pieces necessary to      **
**  connect in a row in order to win.  Therefore one can play Connect-3,  **
**  Connect-5, etc.  An efficient tree-searching algorithm (making use    **
**  of alpha-beta cutoff decisions) has been implemented to insure that   **
**  the computer plays as quickly as possible.                            **
**                                                                        **
**  The declaration of the public functions necessary to use this file    **
**  are contained in "c4.h".                                              **
**                                                                        **
**  In all of the public functions (all of which have the "c4_" prefix),  **
**  the value of player can be any integer, where an even integer refers  **
**  to player 0 and an odd integer refers to player 1.                    **
**                                                                        **
****************************************************************************
**  $Id: c4.c,v 3.11 2009/11/03 14:42:01 pomakis Exp pomakis $
***************************************************************************/

// #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "c4.h"


/* Some macros for convenience. */

#define other(x)        ((x) ^ 1)
#define real_player(x)  ((x) & 1)

#define pop_state()     (current_state = &state_stack[--depth])

#define goodness_of(player)  (current_state->score[player] - current_state->score[other(player)])

typedef struct {
  char  **board;
  int   *(score_array[2]);
  int   score[2];
  short int winner;
  int num_of_pieces;
} Game_state;

/* Static global variables. */

static int size_x, size_y, total_size;
static int num_to_connect;
static int win_places;

static int ***map;
static int magic_win_number;
static bool game_in_progress = false, move_in_progress = false;
static bool seed_chosen = false;
static void (*poll_function)(void) = NULL;
static clock_t poll_interval, next_poll;
static Game_state state_stack[C4_MAX_LEVEL+1];
static Game_state *current_state;
static int depth;
static int states_allocated = 0;
static int *drop_order;

/* ------------------------------------------------------
                  Lokale Funktionen
   ------------------------------------------------------ */

static int num_of_win_places(int x, int y, int n);
static void update_score(int player, int x, int y);
static int drop_piece(int player, int column);
static void push_state(void);
static int evaluate(int player, int level, int alpha, int beta);
static void *emalloc(size_t size);


void c4_poll(void (*poll_func)(void), clock_t interval)
{
  poll_function = poll_func;
  poll_interval = interval;
}

void c4_new_game(int width, int height, int num)
{
  register int i, j, k, x;
  int win_index, column;
  int *win_indices;

  assert(!game_in_progress);
  assert(width >= 1 && height >= 1 && num >= 1);

  size_x = width;
  size_y = height;
  total_size = width * height;
  num_to_connect = num;
  magic_win_number = 1 << num_to_connect;
  win_places = num_of_win_places(size_x, size_y, num_to_connect);

  // zufaellige Wahl, wenn fuer 2 Zuege eine gleiche Bewertung
  // erteilt wurde

  if (!seed_chosen)
  {
    //srand((unsigned int) time((time_t *) 0));
    srand(95);
    seed_chosen = true;
  }


  depth = 0;
  current_state = &state_stack[0];

  current_state->board = (char **) emalloc(size_x * sizeof(char *));
  for (i=0; i<size_x; i++)
  {
    current_state->board[i] = (char *) emalloc(size_y);
    for (j=0; j<size_y; j++)
      current_state->board[i][j] = C4_NONE;
  }

  current_state->score_array[0] = (int *) emalloc(win_places * sizeof(int));
  current_state->score_array[1] = (int *) emalloc(win_places * sizeof(int));
  for (i=0; i<win_places; i++)
  {
    current_state->score_array[0][i] = 1;
    current_state->score_array[1][i] = 1;
  }

  current_state->score[0] = current_state->score[1] = win_places;
  current_state->winner = C4_NONE;
  current_state->num_of_pieces = 0;

  states_allocated = 1;

  map = (int ***) emalloc(size_x * sizeof(int **));
  for (i=0; i<size_x; i++)
  {
    map[i] = (int **) emalloc(size_y * sizeof(int *));
    for (j=0; j<size_y; j++)
    {
      map[i][j] = (int *) emalloc((num_to_connect*4 + 1) * sizeof(int));
      map[i][j][0] = -1;
    }
  }

  win_index = 0;

  for (i=0; i<size_y; i++)
    for (j=0; j<size_x-num_to_connect+1; j++)
    {
      for (k=0; k<num_to_connect; k++)
      {
        win_indices = map[j+k][i];
        for (x=0; win_indices[x] != -1; x++);
        win_indices[x++] = win_index;
        win_indices[x] = -1;
      }
      win_index++;
    }

  for (i=0; i<size_x; i++)
  {
    for (j=0; j<size_y-num_to_connect+1; j++)
    {
      for (k=0; k<num_to_connect; k++)
      {
        win_indices = map[i][j+k];
        for (x=0; win_indices[x] != -1; x++);
        win_indices[x++] = win_index;
        win_indices[x] = -1;
      }
      win_index++;
    }
  }

    for (i=0; i<size_y-num_to_connect+1; i++)
      for (j=0; j<size_x-num_to_connect+1; j++)
      {
        for (k=0; k<num_to_connect; k++)
        {
          win_indices = map[j+k][i+k];
          for (x=0; win_indices[x] != -1; x++);
          win_indices[x++] = win_index;
          win_indices[x] = -1;
        }
        win_index++;
      }

    for (i=0; i<size_y-num_to_connect+1; i++)
      for (j=size_x-1; j>=num_to_connect-1; j--)
      {
        for (k=0; k<num_to_connect; k++)
        {
          win_indices = map[j-k][i+k];
          for (x=0; win_indices[x] != -1; x++);
          win_indices[x++] = win_index;
          win_indices[x] = -1;
        }
        win_index++;
      }

    drop_order = (int *) emalloc(size_x * sizeof(int));
    column = (size_x-1) / 2;
    for (i=1; i<=size_x; i++)
    {
      drop_order[i-1] = column;
      column += ((i%2)? i : -i);
    }

    game_in_progress = true;
}



bool c4_make_move(int player, int column, int *row)
{
  int result;

  assert(game_in_progress);
  assert(!move_in_progress);

  if (column >= size_x || column < 0) return false;

  result = drop_piece(real_player(player), column);
  if (row != NULL && result >= 0) *row = result;
  return (result >= 0);
}

bool c4_auto_move(int player, int level, int *column, int *row)
{
  int i;

  int best_column = -1, goodness = 0, best_worst = -(INT_MAX);
  int num_of_equal = 0, real_player, current_column, result;

  assert(game_in_progress);
  assert(!move_in_progress);
  assert(level >= 1 && level <= C4_MAX_LEVEL);

  real_player = real_player(player);

  if (current_state->num_of_pieces < 2 &&
      size_x == 7 && size_y == 6 && num_to_connect == 4 &&
      (current_state->num_of_pieces == 0 ||
      current_state->board[3][0] != C4_NONE))
  {
    if (column != NULL) *column = 3;
    if (row != NULL) *row = current_state->num_of_pieces;
    drop_piece(real_player, 3);
    return true;
  }

  move_in_progress = true;

  for (i=0; i<size_x; i++)
  {
    push_state();
    current_column = drop_order[i];

    result = drop_piece(real_player, current_column);

    if (result < 0)
    {
      pop_state();
      continue;
    }
    else
    if (current_state->winner == real_player)
    {
      best_column = current_column;
      pop_state();
      break;
    }
    else
    {
      next_poll = clock() + poll_interval;
      goodness = evaluate(real_player, level, -(INT_MAX), -best_worst);
    }

    if (goodness > best_worst)
    {
      best_worst = goodness;
      best_column = current_column;
      num_of_equal = 1;
    }

    // wenn 2 Zuege gleich gut sind, eine zufaellige Wahl treffen
    else if (goodness == best_worst)
    {
      num_of_equal++;
      if ((rand()>>4) % num_of_equal == 0) best_column = current_column;
    }

    pop_state();
    }

    move_in_progress = false;

    if (best_column >= 0)
    {
      result = drop_piece(real_player, best_column);
      if (column != NULL) *column = best_column;
      if (row != NULL) *row = result;
      return true;
    }
    else
    return false;
}


char **c4_board(void)
{
  assert(game_in_progress);
  return current_state->board;
}


/****************************************************************************/
/**                                                                        **/
/**  This function returns the "score" of the specified player.  This      **/
/**  score is a function of how many winning positions are still available **/
/**  to the player and how close he/she is to achieving each of these      **/
/**  positions.  The scores of both players can be compared to observe how **/
/**  well they are doing relative to each other.                           **/
/**                                                                        **/
/****************************************************************************/

int
c4_score_of_player(int player)
{
  assert(game_in_progress);
  return current_state->score[real_player(player)];
}

bool c4_is_winner(int player)
{
  assert(game_in_progress);
  return (current_state->winner == real_player(player));
}

bool c4_is_tie(void)
{
  assert(game_in_progress);
  return (current_state->num_of_pieces == total_size && current_state->winner == C4_NONE);
}

void c4_win_coords(int *x1, int *y1, int *x2, int *y2)
{
  register int i, j, k;
  int          winner, win_pos = 0;
  bool         found;

  assert(game_in_progress);

  winner = current_state->winner;
  assert(winner != C4_NONE);

  while (current_state->score_array[winner][win_pos] != magic_win_number)
    win_pos++;

  found = false;
  for (j=0; j<size_y && !found; j++)
    for (i=0; i<size_x && !found; i++)
      for (k=0; map[i][j][k] != -1; k++)
        if (map[i][j][k] == win_pos)
        {
          *x1 = i;
          *y1 = j;
          found = true;
          break;
        }

  found = false;
  for (j=size_y-1; j>=0 && !found; j--)
    for (i=size_x-1; i>=0 && !found; i--)
      for (k=0; map[i][j][k] != -1; k++)
        if (map[i][j][k] == win_pos)
        {
          *x2 = i;
          *y2 = j;
          found = true;
          break;
        }
}

void c4_end_game(void)
{
  int i, j;

  assert(game_in_progress);
  assert(!move_in_progress);

  for (i=0; i<size_x; i++)
  {
    for (j=0; j<size_y; j++) free(map[i][j]);
    free(map[i]);
  }
  free(map);

  for (i=0; i<states_allocated; i++)
  {
    for (j=0; j<size_x; j++) free(state_stack[i].board[j]);
    free(state_stack[i].board);
    free(state_stack[i].score_array[0]);
    free(state_stack[i].score_array[1]);
  }
  states_allocated = 0;

  free(drop_order);

  game_in_progress = false;
}

void c4_reset(void)
{
  assert(!move_in_progress);
  if (game_in_progress) c4_end_game();
  poll_function = NULL;
}


const char * c4_get_version(void)
{
    return "$Id: c4.c,v 3.11 2009/11/03 14:42:01 pomakis Exp pomakis $";
}

/* --------------------------------------------------------------------------
                        lokale Funktionen
   -------------------------------------------------------------------------- */

static int num_of_win_places(int x, int y, int n)
{
  if (x < n && y < n) return 0;
  else
  if (x < n) return x * ((y-n)+1);
  else
  if (y < n) return y * ((x-n)+1);
  else  return 4*x*y - 3*x*n - 3*y*n + 3*x + 3*y - 4*n + 2*n*n + 2;
}


static void update_score(int player, int x, int y)
{
    register int i;
    int win_index;
    int this_difference = 0, other_difference = 0;
    int **current_score_array = current_state->score_array;
    int other_player = other(player);

    for (i=0; map[x][y][i] != -1; i++) {
        win_index = map[x][y][i];
        this_difference += current_score_array[player][win_index];
        other_difference += current_score_array[other_player][win_index];

        current_score_array[player][win_index] <<= 1;
        current_score_array[other_player][win_index] = 0;

        if (current_score_array[player][win_index] == magic_win_number)
            if (current_state->winner == C4_NONE)
                current_state->winner = player;
    }

    current_state->score[player] += this_difference;
    current_state->score[other_player] -= other_difference;
}


/****************************************************************************/
/**                                                                        **/
/**  This function drops a piece of the specified player into the          **/
/**  specified column.  The row where the piece ended up is returned, or   **/
/**  -1 if the drop was unsuccessful (i.e., the specified column is full). **/
/**                                                                        **/
/****************************************************************************/

static int
drop_piece(int player, int column)
{
    int y = 0;

    while (current_state->board[column][y] != C4_NONE && ++y < size_y)
        ;

    if (y == size_y)
        return -1;

    current_state->board[column][y] = player;
    current_state->num_of_pieces++;
    update_score(player, column, y);

    return y;
}


/****************************************************************************/
/**                                                                        **/
/**  This function pushes the current state onto a stack.  pop_state()     **/
/**  is used to pop from this stack.                                       **/
/**                                                                        **/
/**  Technically what it does, since the current state is considered to    **/
/**  be the top of the stack, is push a copy of the current state onto     **/
/**  the stack right above it.  The stack pointer (depth) is then          **/
/**  incremented so that the new copy is considered to be the current      **/
/**  state.  That way, all pop_state() has to do is decrement the stack    **/
/**  pointer.                                                              **/
/**                                                                        **/
/**  For efficiency, memory for each stack state used is only allocated    **/
/**  once per game, and reused for the remainder of the game.              **/
/**                                                                        **/
/****************************************************************************/

static void
push_state(void)
{
    register int i, win_places_array_size;
    Game_state *old_state, *new_state;

    win_places_array_size = win_places * sizeof(int);
    old_state = &state_stack[depth++];
    new_state = &state_stack[depth];

    if (depth == states_allocated) {

        /* Allocate space for the board */

        new_state->board = (char **) emalloc(size_x * sizeof(char *));
        for (i=0; i<size_x; i++)
            new_state->board[i] = (char *) emalloc(size_y);

        /* Allocate space for the score array */

        new_state->score_array[0] = (int *) emalloc(win_places_array_size);
        new_state->score_array[1] = (int *) emalloc(win_places_array_size);

        states_allocated++;
    }

    /* Copy the board */

    for (i=0; i<size_x; i++)
        memcpy(new_state->board[i], old_state->board[i], size_y);

    /* Copy the score array */

    memcpy(new_state->score_array[0], old_state->score_array[0],
           win_places_array_size);
    memcpy(new_state->score_array[1], old_state->score_array[1],
           win_places_array_size);

    new_state->score[0] = old_state->score[0];
    new_state->score[1] = old_state->score[1];
    new_state->winner = old_state->winner;
    new_state->num_of_pieces = old_state->num_of_pieces;

    current_state = new_state;
}


/****************************************************************************/
/**                                                                        **/
/**  This recursive function determines how good the current state may     **/
/**  turn out to be for the specified player.  It does this by looking     **/
/**  ahead level moves.  It is assumed that both the specified player and  **/
/**  the opponent may make the best move possible.  alpha and beta are     **/
/**  used for alpha-beta cutoff so that the game tree can be pruned to     **/
/**  avoid searching unneccessary paths.                                   **/
/**                                                                        **/
/**  The specified poll function (if any) is called at the appropriate     **/
/**  intervals.                                                            **/
/**                                                                        **/
/**  The worst goodness that the current state can produce in the number   **/
/**  of moves (levels) searched is returned.  This is the best the         **/
/**  specified player can hope to achieve with this state (since it is     **/
/**  assumed that the opponent will make the best moves possible).         **/
/**                                                                        **/
/****************************************************************************/

static int
evaluate(int player, int level, int alpha, int beta)
{
  int i;

    if (poll_function != NULL && next_poll <= clock()) {
        next_poll += poll_interval;
        (*poll_function)();
    }

    if (current_state->winner == player)
        return INT_MAX - depth;
    else if (current_state->winner == other(player))
        return -(INT_MAX - depth);
    else if (current_state->num_of_pieces == total_size)
        return 0; /* a tie */
    else if (level == depth)
        return goodness_of(player);
    else {
        /* Assume it is the other player's turn. */
        int best = -(INT_MAX);
        int maxab = alpha;
        for(i=0; i<size_x; i++) {
            if (current_state->board[drop_order[i]][size_y-1] != C4_NONE)
                continue; /* The column is full. */
            push_state();
            drop_piece(other(player), drop_order[i]);
            int goodness = evaluate(other(player), level, -beta, -maxab);
            if (goodness > best) {
                best = goodness;
                if (best > maxab)
                    maxab = best;
            }
            pop_state();
            if (best > beta)
                break;
        }

        /* What's good for the other player is bad for this one. */
        return -best;
    }
}


/****************************************************************************/
/**                                                                        **/
/**  A safer version of malloc().                                          **/
/**                                                                        **/
/****************************************************************************/

static void *
emalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL) {
//        printf("c4: emalloc() - Can't allocate %ld bytes.\n", (long) size);
        while(1);
    }
    return ptr;
}

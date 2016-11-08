/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Adriano Grieb
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_LENGTH 255
#define FINISHED -10
#define UNAVAILABLE 10
#define AVAILABLE -1
#define EMPTY 0


// gameplay
int search_depth = 10;
int tile_victory_worth = 5;
int middle_worth = 2;
struct
{
  int timebank;
  int time_per_move;
  int botid;
} game_settings;

typedef struct table
{
  int macro[3 * 3];
  int field[9 * 9];
  //int finished[9];
} table;

table board;

typedef struct move
{
  int x;
  int y;
  int value;
} move;

void settings(char *, char *);
void update(char *, char *, char *);
void action(char *, char *);
move recursive_search(table curr_board, int remaining_depth, int turn, int myNum);

int main(int argc, char const *argv[])
{
  char line[MAX_LINE_LENGTH];
  char part[3][MAX_LINE_LENGTH];

  #ifdef DEBUG
  freopen("test.in", "r", stdin);
  #endif
  srand(time(NULL));
  while(fgets(line, MAX_LINE_LENGTH, stdin) != NULL)
  {
    if (!strncmp(line, "action ", 7))
    {
      sscanf(&line[7], "%s %s", part[0], part[1]);
      action(part[0], part[1]);
      fflush(stdout);
      continue;
    }
    if (!strncmp(line, "update ", 7))
    {
      sscanf(&line[7], "%s %s %s", part[0], part[1], part[2]);
      update(part[0], part[1], part[2]);
      continue;
    }
    if (!strncmp(line, "settings ", 9))
    {
      sscanf(&line[9], "%s %s", part[0], part[1]);
      settings(part[0], part[1]);
      continue;
    }
  }

  #ifdef DEBUG
  fprintf(stderr, "Game over!\n");
  #endif

  exit(EXIT_SUCCESS);
}

int macro_coords_to_field(int macro_x, int macro_y){
  return macro_y * 27 + macro_x * 3; 
}

int testWon(int field[], int macro_x, int macro_y, int num){
  // just test all possibilities, there are only 8
  int x0y0 = field[macro_y * 3 * 9 + macro_x * 3];
  int x0y1 = field[macro_y * 4 * 9 + macro_x * 3];
  int x0y2 = field[macro_y * 5 * 9 + macro_x * 3];
  int x1y0 = field[macro_y * 3 * 9 + macro_x * 3 + 1];
  int x1y1 = field[macro_y * 4 * 9 + macro_x * 3 + 1];
  int x1y2 = field[macro_y * 5 * 9 + macro_x * 3 + 1];
  int x2y0 = field[macro_y * 3 * 9 + macro_x * 3 + 2];
  int x2y1 = field[macro_y * 4 * 9 + macro_x * 3 + 2];
  int x2y2 = field[macro_y * 5 * 9 + macro_x * 3 + 2];

  if (x0y0 == num && x0y1 == num && x0y2 == num){
    return 1;
  }
  if (x1y0 == num && x1y1 == num && x1y2 == num){
    return 1;
  }
  if (x2y0 == num && x2y1 == num && x2y2 == num){
    return 1;
  }

  if (x0y0 == num && x1y0 == num && x2y0 == num){
    return 1;
  }
  if (x0y1 == num && x1y1 == num && x2y1 == num){
    return 1;
  }
  if (x0y2 == num && x1y2 == num && x2y2 == num){
    return 1;
  }
  if (x0y0 == num && x1y1 == num && x2y2 == num){
    return 1;
  }
  if (x2y0 == num && x1y1 == num && x0y2 == num){
    return 1;
  }

  return 0;
}

int macro_coords_to_int(int x, int y){
  return x * 3 + y;
}

int all_macro_coords_to_int(int m_x, int m_y, int x, int y){
  return m_x * 3 * 9 + m_y * 3 + x * 9 + y;
}


int estimate_value_single(int field[], int x, int y, int myNum){
  int i_won = testWon(field, x, y, myNum);
  if (i_won)
    return tile_victory_worth;
  int he_won = testWon(field, x, y, myNum == 3 - myNum);
  if (he_won)
    return -tile_victory_worth;

  if (field[macro_coords_to_field(x,y) + 10] == myNum){
    return middle_worth;
  }
  else if (field[macro_coords_to_field(x,y) + 10] == 3 - myNum){
    return -middle_worth;
  }
  return 0;
}



int estimate_value_all(table curr_board, int myNum){
  int i = 0;
  int j = 0;

  int total = 0;
  for (i = 0; i < 3; i++){
    for (j = 0; j < 3; j++){
      total += estimate_value_single(curr_board.field, i, j, myNum);
    }
  }
  return total;
}


void update_table(table *curr_board, int m_x, int m_y, int x, int y, int player){
  curr_board->field[all_macro_coords_to_int(m_x, m_y, x, y)] = player;

  int i = 0;
  int j = 0;

  if (testWon(curr_board->field, m_x, m_y, 1) || testWon(curr_board->field, m_x, m_y, 2)){
    curr_board->macro[macro_coords_to_int(m_x, m_y)] = FINISHED;
    for (i = 0; i < 3; i++){
      for (j = 0; j < 3; j++){
        if (curr_board->macro[macro_coords_to_int(i,j)] != FINISHED){
         curr_board->macro[macro_coords_to_int(i,j)] = AVAILABLE;
       }
     }
   }
 } else{
  for (i = 0; i < 3; i++){
    for (j = 0; j < 3; j++){
      if (curr_board->macro[macro_coords_to_int(i, j)] != FINISHED){
        curr_board->macro[macro_coords_to_int(i,j)] = UNAVAILABLE;
      }      
    }
  }
  curr_board->macro[macro_coords_to_int(m_x, m_y)] = AVAILABLE;
}



}

move recursive_search_min_max(table curr_board, int remaining_depth, int turn, int myNum){
  int x = 0;
  int y = 0;
  move chosen_move = {0,0, turn == myNum ? -100 : 100};
  int m_x = 0;
  int m_y = 0;
  for (m_x = 0; m_x < 3; m_x++){
    for (m_y = 0; m_y < 3; m_y++){
      if (curr_board.macro[macro_coords_to_int(m_x, m_y)] == AVAILABLE){
        for (x = 0; x < 3; x++){
          for (y = 0; y < 3; y++){
            // check not taken already
            if (curr_board.field[all_macro_coords_to_int(m_x, m_y, x, y)] == EMPTY){
              table new_board = curr_board;
              update_table(&new_board, m_x, m_y, x, y, turn);
              move found = recursive_search(new_board, remaining_depth-1, 3 - turn, myNum);
              if (turn == myNum){
                if (found.value > chosen_move.value){
                  chosen_move.value = found.value;
                  chosen_move.x = m_x * 3 + x;
                  chosen_move.y = m_y * 3 + y;
                }
              } else{
                if (found.value < chosen_move.value){
                  chosen_move.value = found.value;
                  chosen_move.x = m_x * 3 + x;
                  chosen_move.y = m_y * 3 + y;
                }
              }
            }
          }
        }
      }
    }
  }
  return chosen_move;
}

move recursive_search(table curr_board, int remaining_depth, int turn, int myNum){
  if (remaining_depth == 0){
    move a = {0, 0, estimate_value_all(curr_board, myNum)};
    return a;
  }
  move chosen_move = recursive_search_min_max(curr_board, remaining_depth, turn, myNum);
  return chosen_move;
}

void random_action(char *action, char *value){
  if (!strcmp(action, "move"))
  {
    int count = 0;
    int x = 0, y = 0;
    int i = 0;
    for (i = 0; i < 9; i++)
    {
      if (board.macro[i] == -1)
      {
        int j;
        for (j = 0; j < 9; j++)
        {
          if (board.field[i * 9 + j] == 0)
          {
            count++;
          }
        }
      }
    }

    int pick = rand() % count;
    #ifdef DEBUG
    fprintf(stderr, "available moves: %d pick: %d\n", count, pick);
    #endif

    count = -1;
    for (i = 0; i < 9; i++)
    {
      if (board.macro[i] == -1)
      {
        int j;
        for (j = 0; j < 9; j++)
        {
          if (board.field[i * 9 + j] == 0)
          {
            if (++count == pick)
            {
              x = ((i % 3) * 3) + (j % 3);
              y = ((i / 3) * 3) + (j / 3);
            }
          }
        }
      }
    }

    // place_move 1 1 // places an O (for player 1) in the middle small square of
    //                   the top-left big square
    fprintf(stdout, "place_move %d %d\n", x, y);
    #ifdef DEBUG
    fprintf(stderr, "place_move %d %d\n", x, y);
    #endif
  }
  #ifdef DEBUG
  else
  {
    fprintf(stderr, "unknown action: [%s: %s]\n", action, value);
  }
  #endif
}


void action(char *action, char *value)
{
  assert(action != NULL);
  assert(value != NULL);

  move m = recursive_search(board, search_depth, game_settings.botid, game_settings.botid);

  fprintf(stdout, "place_move %d %d\n", m.y, m.x);
  return;

}

void update(char *game, char *update, char *value)
{
  assert(game != NULL);
  assert(update != NULL);
  assert(value != NULL);

  if (strcmp(game, "game"))
  {
    #ifdef DEBUG
    fprintf(stderr, "unknown update: [%s - %s: %s]\n", game, update, value);
    #endif
    return;
  }

  // update game round 1
  if (!strcmp(update, "round"))
  {
    return;
  }
  // update game move 1
  if (!strcmp(update, "move"))
  {
    return;
  }
  // update game macroboard -1,-1,-1,-1,-1,-1,-1,-1,-1
  if (!strcmp(update, "macroboard"))
  {
    sscanf(value, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &board.macro[0],
      &board.macro[1], &board.macro[2], &board.macro[3], &board.macro[4],
      &board.macro[5], &board.macro[6], &board.macro[7], &board.macro[8]);
    return;
  }
  // update game field  0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0,
  //                    0,0,0,0,0,0,0,0,0
  // no new lines and no spaces after the commas
  if (!strcmp(update, "field"))
  {
    sscanf(value,                   
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d," 
      "%d,%d,%d,%d,%d,%d,%d,%d,%d", 
      &board.field[0], &board.field[1], &board.field[2], &board.field[3], &board.field[4], &board.field[5], &board.field[6], &board.field[7], &board.field[8], &board.field[9],
      &board.field[10], &board.field[11], &board.field[12], &board.field[13], &board.field[14], &board.field[15],
      &board.field[16], &board.field[17], &board.field[18], &board.field[19], &board.field[20], &board.field[21],
      &board.field[22], &board.field[23], &board.field[24], &board.field[25], &board.field[26], &board.field[27],
      &board.field[28], &board.field[29], &board.field[30], &board.field[31], &board.field[32], &board.field[33],
      &board.field[34], &board.field[35], &board.field[36], &board.field[37], &board.field[38],
      &board.field[39], &board.field[40], &board.field[41], &board.field[42], &board.field[43],
      &board.field[44], &board.field[45], &board.field[46], &board.field[47], &board.field[48],
      &board.field[49], &board.field[50], &board.field[51], &board.field[52], &board.field[53],
      &board.field[54], &board.field[55], &board.field[56], &board.field[57], &board.field[58],
      &board.field[59], &board.field[60], &board.field[61], &board.field[62], &board.field[63],
      &board.field[64], &board.field[65], &board.field[66], &board.field[67], &board.field[68], &board.field[69],
      &board.field[70], &board.field[71], &board.field[72], &board.field[73], &board.field[74], &board.field[75],
      &board.field[76], &board.field[77], &board.field[78], &board.field[79], &board.field[80]);
    return;
  }
}

void settings(char *setting, char *value)
{
  assert(setting != NULL);
  assert(value != NULL);

  // settings timebank 10000
  if (!strcmp(setting, "timebank"))
  {
    game_settings.timebank = atoi(value);
    #ifdef DEBUG
    fprintf(stderr, "settings timebank: %d\n", game_settings.timebank);
    #endif
    return;
  }
  // settings time_per_move 500
  if (!strcmp(setting, "time_per_move"))
  {
    game_settings.time_per_move = atoi(value);
    #ifdef DEBUG
    fprintf(stderr, "time_per_move: %d\n", game_settings.time_per_move);
    #endif
    return;
  }
  // settings player_names player1,player2
  if (!strcmp(setting, "player_names"))
  {
    return;
  }
  // settings your_bot player1
  if (!strcmp(setting, "your_bot"))
  {
    return;
  } 
  // settings your_botid 1
  if (!strcmp(setting, "your_botid"))
  {
    game_settings.botid = atoi(value);
    #ifdef DEBUG
    fprintf(stderr, "botid: %d\n", game_settings.botid);
    #endif
    return;
  }

  #ifdef DEBUG
  fprintf(stderr, "unknown setting: [%s: %s]\n", setting, value);
  #endif
}

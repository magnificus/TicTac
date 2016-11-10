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
#include <limits.h>

#define MAX_LINE_LENGTH 255
#define FINISHED -10
#define UNAVAILABLE 10
#define AVAILABLE -1
#define EMPTY 0

#define WON_1 1
#define WON_2 2
#define TIE 3

#define MAX(a,b) ((a) > (b) ? a : b);
#define MIN(a,b) ((a) < (b) ? a : b);
// gameplay
int search_depth = 9;
int tile_victory_worth = 20;

int worths[] = {2,1,2,1,3,1,2,1,2};

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
//move recursive_search(table curr_board, int remaining_depth, int turn, int myNum, int alpha, int beta);

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




int getState(int field[], int macro_x, int macro_y){
  // just test all possibilities, there are only 8
  int x0y0 = field[macro_coords_to_field(macro_x, macro_y)];
  int x0y1 = field[macro_coords_to_field(macro_x, macro_y) + 1*9];
  int x0y2 = field[macro_coords_to_field(macro_x, macro_y) + 2*9];
  int x1y0 = field[macro_coords_to_field(macro_x, macro_y) + 1];
  int x1y1 = field[macro_coords_to_field(macro_x, macro_y) + 1*9 + 1];
  int x1y2 = field[macro_coords_to_field(macro_x, macro_y) + 2*9 + 1];
  int x2y0 = field[macro_coords_to_field(macro_x, macro_y) + 2];
  int x2y1 = field[macro_coords_to_field(macro_x, macro_y) + 1*9 + 2];
  int x2y2 = field[macro_coords_to_field(macro_x, macro_y) + 2*9 + 2];

  if (x0y0 != EMPTY && x0y0 == x0y1 && x0y1 == x0y2){
    return x0y0;
  }
  if (x1y0 != EMPTY && x1y1 == x1y0 && x1y2 == x1y0){
    return x1y0;
  }
  if (x2y0 != EMPTY && x2y1 == x2y0 && x2y2 == x2y0){
    return x2y0;
  }
  if (x0y0 != EMPTY && x1y0 == x0y0 && x2y0 == x0y0){
    return x0y0;
  }
  if (x0y1 != EMPTY && x1y1 == x0y1 && x2y1 == x0y1){
    return x0y1;
  }
  if (x0y2 != EMPTY && x1y2 == x0y2 && x2y2 == x0y2){
    return x0y2;
  }
  if (x0y0 != EMPTY && x1y1 == x0y0 && x2y2 == x0y0){
    return x0y0;
  }
  if (x2y0 != EMPTY && x1y1 == x2y0 && x0y2 == x2y0){
    return x2y0;
  }

  if (x0y0 != EMPTY && x0y1 != EMPTY && x0y2 != EMPTY && x1y0 != EMPTY && x1y1 != EMPTY && x1y2 != EMPTY && x2y0 != EMPTY && x2y1 != EMPTY && x2y2 != EMPTY){
    return TIE;
  }
  return EMPTY;
}

int macro_coords_to_field(int macro_x, int macro_y){
  return macro_y * 27 + macro_x * 3; 
}

int macro_coords_to_int(int x, int y){
  return y * 3 + x;
}

int all_coords_to_int(int m_x, int m_y, int x, int y){
  return m_x * 3 + m_y * 27 + x + y * 9;
}


int estimate_value_single(int field[], int x, int y){
  int state = getState(field, x, y);

  if (state == game_settings.botid)
    return tile_victory_worth;
  if (state == 3 - game_settings.botid){
    return - tile_victory_worth;
  }
  if (state == TIE)
    return 0;

  // no clear winner, time for heuristics
  int totalVal = 0;
  int i = 0;
  int j = 0;

  // worths for individual tiles
  for (i = 0; i < 3; i++){
    for (j = 0; j < 3; j++){
      int state = field[all_coords_to_int(x,y,i,j)];
      if (state != EMPTY){
          int factor = (game_settings.botid == state) ? 1 : -1;
          totalVal += worths[i + j*3] * factor;
      }

    }
  }

  // worths for 
  return totalVal;
}



int estimate_value_all(table *curr_board){
  int i = 0;
  int j = 0;

  int res[9];

  for (i = 0; i < 3; i++){
    for (j = 0; j < 3; j++){
      res[i+j*3] = getState(curr_board->field, i, j);
    }
  }

  //check victory
  if ((res[0] == WON_1 || res[0] == WON_2) && res[1] == res[0] && res[2] == res[0]){
    return (res[0] == game_settings.botid) ? 100000 : -100000;
  }
  if ((res[3] == WON_1 || res[3] == WON_2) && res[4] == res[3] && res[5] == res[3]){
    return (res[3] == game_settings.botid) ? 100000 : -100000;
  }
  if ((res[6] == WON_1 || res[6] == WON_2) && res[7] == res[6] && res[8] == res[6]){
    return (res[6] == game_settings.botid) ? 100000 : -100000;
  }
  if ((res[0] == WON_1 || res[0] == WON_2) && res[3] == res[0] && res[6] == res[0]){
    return (res[0] == game_settings.botid) ? 100000 : -100000;
  }
  if ((res[1] == WON_1 || res[1] == WON_2) && res[4] == res[1] && res[7] == res[1]){
    return (res[1] == game_settings.botid) ? 100000 : -100000;
  }
  if ((res[2] == WON_1 || res[2] == WON_2) && res[5] == res[2] && res[8] == res[2]){
    return (res[2] == game_settings.botid) ? 100000 : -100000;
  }
  //diagonals
  if ((res[0] == WON_1 || res[0] == WON_2) && res[4] == res[0] && res[8] == res[0]){
    return (res[0] == game_settings.botid) ? 100000 : -100000;
  }
  if ((res[2] == WON_1 || res[2] == WON_2) && res[4] == res[2] && res[6] == res[2]){
    return (res[2] == game_settings.botid) ? 100000 : -100000;
  }


  // regular heuristic
  int total = 0;
  for (i = 0; i < 3; i++){
    for (j = 0; j < 3; j++){
      total += estimate_value_single(curr_board->field, i, j) * worths[i + j*3];
    }
  }
  return total;
}


void update_table(table *curr_board, int m_x, int m_y, int x, int y, int player){

  curr_board->field[all_coords_to_int(m_x, m_y, x, y)] = player;

  int i = 0;
  int j = 0;

  if (getState(curr_board->field, x, y) != EMPTY){
    curr_board->macro[macro_coords_to_int(x, y)] = FINISHED;
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
    curr_board->macro[macro_coords_to_int(x, y)] = AVAILABLE;
  }
}
}

move recursive_search(table *curr_board, int remaining_depth, int turn, int alpha, int beta){
  if (remaining_depth == 0){
    int res = estimate_value_all(curr_board);
    move a = {0,0,res};
    return a;
  }

  //if (remaining_depth % 2 == 0){
   //potentially abandon
   //int res = estimate_value_all(curr_board);
   //if (res < 0){
     //move a = {0, 0, res};
     //return a;
   //}
 //}

  int x = 0;
  int y = 0;
  
  int m_x = 0;
  int m_y = 0;
  int oldMacro[9];
  if (turn == game_settings.botid){
    move chosen_move;
    int v = INT_MIN;
    //printf("level %d\n", remaining_depth);
    for (m_x = 0; m_x < 3; m_x++){
      for (m_y = 0; m_y < 3; m_y++){
        if (curr_board->macro[macro_coords_to_int(m_x, m_y)] == AVAILABLE){
          for (x = 0; x < 3; x++){
            for (y = 0; y < 3; y++){
            // check not taken already
              if (curr_board->field[all_coords_to_int(m_x, m_y, x, y)] == EMPTY){
                memcpy(oldMacro, curr_board->macro, sizeof(curr_board->macro));
                update_table(curr_board, m_x, m_y, x, y, turn);
                move found = recursive_search(curr_board, remaining_depth-1, 3 - turn, alpha, beta);
                int recieved = found.value;
                curr_board->field[all_coords_to_int(m_x, m_y, x, y)] = EMPTY;
                memcpy(curr_board->macro, oldMacro, sizeof(oldMacro));

                //printf("ME %d %d %d\n", x, y, recieved);

                if (recieved > v){
                  chosen_move = found;
                  chosen_move.x = m_x * 3 + x;
                  chosen_move.y = m_y * 3 + y;
                  v = recieved;
                  alpha = MAX(alpha, v);
                  if (beta <= alpha)
                    return chosen_move;
                  } 
                }
              }
            }
          }
        }
      }
    return chosen_move;
  }
  else{
    move chosen_move;
    int v = INT_MAX;  
    for (m_x = 0; m_x < 3; m_x++){
      for (m_y = 0; m_y < 3; m_y++){
        if (curr_board->macro[macro_coords_to_int(m_x, m_y)] == AVAILABLE){
          for (x = 0; x < 3; x++){
            for (y = 0; y < 3; y++){
            // check not taken already
              if (curr_board->field[all_coords_to_int(m_x, m_y, x, y)] == EMPTY){
                memcpy(oldMacro, curr_board->macro, sizeof(curr_board->macro));
                update_table(curr_board, m_x, m_y, x, y, turn);
                move found = recursive_search(curr_board, remaining_depth-1, 3 - turn, alpha, beta);
                int recieved = found.value;
                curr_board->field[all_coords_to_int(m_x, m_y, x, y)] = EMPTY;
                memcpy(curr_board->macro, oldMacro, sizeof(oldMacro));
                //printf("move value HIM: %d\n", recieved);
                if (recieved < v){
                  v = recieved;
                  //chosen_move = found;
                  chosen_move.value = recieved;
                  chosen_move.x = m_x * 3 + x;
                  chosen_move.y = m_y * 3 + y;
                  beta = MIN(v, beta);
                  if (beta <= alpha)
                    return chosen_move;
                } 
              }
            }
          }
        }
      }
    }
    return chosen_move;
}
}


void action(char *action, char *value)
{
  assert(action != NULL);
  assert(value != NULL);

  move m = recursive_search(&board, search_depth, game_settings.botid, INT_MIN, INT_MAX);

  int x = m.x;
  int y = m.y;
  fprintf(stdout, "place_move %d %d\n", x, y);
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

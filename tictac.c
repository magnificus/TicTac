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
#include <time.h>

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


#define PRECOMPUTED_LENGTH 729;

#define GENERATE_DATA_ONLY 0
// gameplay
int search_depth = 9;
int tile_victory_worth = 40;
int prev_timebank = 0;


int worths[] = {3,2,3,2,4,2,3,2,3};

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


typedef struct precomputed_solution
{
  int field[9*9];
  int x;
  int y;
} precomputed_solution;


precomputed_solution* precomputed_solutions;

void settings(char *, char *);
void update(char *, char *, char *);
void action(char *, char *);

void load_precomputed();

int main(int argc, char const *argv[])
{
  char line[MAX_LINE_LENGTH];
  char part[4][MAX_LINE_LENGTH];

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


void get_tile(int m_x, int m_y, int *field, int* tiles){
  tiles[0] = field[macro_coords_to_field(m_x, m_y)];
  tiles[1] = field[macro_coords_to_field(m_x, m_y) + 1];
  tiles[2] = field[macro_coords_to_field(m_x, m_y) + 2];
  tiles[3] = field[macro_coords_to_field(m_x, m_y) + 9];
  tiles[4] = field[macro_coords_to_field(m_x, m_y) + 9 + 1];
  tiles[5] = field[macro_coords_to_field(m_x, m_y) + 9 + 2];
  tiles[6] = field[macro_coords_to_field(m_x, m_y) + 18];
  tiles[7] = field[macro_coords_to_field(m_x, m_y) + 18 + 1];
  tiles[8] = field[macro_coords_to_field(m_x, m_y) + 18 + 2];
}


int getState(int* field, int macro_x, int macro_y){

  // just test all possibilities, there are only 8
  int m_field = macro_coords_to_field(macro_x, macro_y);
  int res[9];
  res[0] = field[m_field];
  res[3] = field[m_field + 1*9];
  res[6] = field[m_field + 2*9];
  res[1] = field[m_field+ 1];
  res[4] = field[m_field + 1*9 + 1];
  res[7] = field[m_field + 2*9 + 1];
  res[2] = field[m_field + 2];
  res[5] = field[m_field + 1*9 + 2];
  res[8] = field[m_field + 2*9 + 2];

  if (res[0] != EMPTY && res[0] == res[3] && res[3] == res[6]){
    return res[0];
  }
  if (res[1] != EMPTY && res[4] == res[1] && res[7] == res[1]){
    return res[1];
  }
  if (res[2] != EMPTY && res[5] == res[2] && res[8] == res[2]){
    return res[2];
  }
  if (res[0] != EMPTY && res[1] == res[0] && res[2] == res[0]){
    return res[0];
  }
  if (res[3] != EMPTY && res[4] == res[3] && res[5] == res[3]){
    return res[3];
  }
  if (res[6] != EMPTY && res[7] == res[6] && res[8] == res[6]){
    return res[6];
  }
  if (res[0] != EMPTY && res[4] == res[0] && res[8] == res[0]){
    return res[0];
  }
  if (res[2] != EMPTY && res[4] == res[2] && res[6] == res[2]){
    return res[2];
  }

  for (int i = 0; i < 9; i++){
    if (res[i] == EMPTY){
      return EMPTY;

    }
  }
  return TIE;
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

int get_state(int field[]){
  if (field[0] != EMPTY && field[0] == field[3] && field[0] == field[6]){
    return field[0];
  }
  if (field[1] != EMPTY && field[4] == field[1] && field[7] == field[1]){
    return field[1];
  }
  if (field[2] != EMPTY && field[5] == field[2] && field[8] == field[2]){
    return field[2];
  }
  if (field[0] != EMPTY && field[1] == field[0] && field[2] == field[0]){
    return field[0];
  }
  if (field[3] != EMPTY && field[4] == field[3] && field[5] == field[3]){
    return field[3];
  }
  if (field[6] != EMPTY && field[7] == field[6] && field[8] == field[6]){
    return field[6];
  }
  if (field[0] != EMPTY && field[4] == field[0] && field[8] == field[0]){
    return field[0];
  }
  if (field[2] != EMPTY && field[4] == field[2] && field[6] == field[2]){
    return field[2];
  }

  for (int i = 0; i < 9; i++){
    if (i == EMPTY){
      return EMPTY;
    }
  }
  return TIE;
}

int estimate_value_single_heuristic(int *field, int x, int y, int state){

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
return totalVal;
}



int estimate_value_all(table *curr_board){
  int i = 0;
  int j = 0;
  int res[9];


  int* tileArray = calloc(9, sizeof(int));
  for (i = 0; i < 3; i++){
    for (j = 0; j < 3; j++){
      get_tile(i, j, curr_board->field, tileArray);
      res[i+j*3] = get_state(tileArray);
      //res[i+j*3] = getState(curr_board->field, i, j);
    }
  }
  free(tileArray);

  //check victory
  int win_state = get_state(curr_board->macro);
  if (win_state == game_settings.botid){
    return 1000000;
  }
  else if (win_state == (3 - game_settings.botid)){
    return -100000;
  }

  // ordinary heuristic
  else{
    int total = 0;
    for (i = 0; i < 3; i++){
      for (j = 0; j < 3; j++){
        int state = res[macro_coords_to_int(i, j)];
        if (state == game_settings.botid)
          total += tile_victory_worth;
        else if (state == 3 - game_settings.botid)
          total -= tile_victory_worth;
        else if (state != TIE)
          total += estimate_value_single_heuristic(curr_board->field, i, j, state) * worths[macro_coords_to_int(i, j)];     
      }
    }
    return total;
  }



  // regular heuristic

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


clock_t begin;
move recursive_search(table *curr_board, int remaining_depth, int turn, int alpha, int beta){
  int win_state = get_state(curr_board->macro);
  if (remaining_depth == 0 || (((double)(clock() - begin) / CLOCKS_PER_SEC) > (game_settings.timebank) / 2000) || win_state == WON_1 || win_state == WON_2){
    int res = estimate_value_all(curr_board);
    move a = {0,0,res};
    return a;
  }

  int x = 0;
  int y = 0;
  
  int m_x = 0;
  int m_y = 0;
  int oldMacro[9];
  move chosen_move;
  if (turn == game_settings.botid){
    int v = INT_MIN;
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
                  chosen_move.value = recieved;
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
    if (v == INT_MIN){
      // no child nodes
      int res = estimate_value_all(curr_board);
      move a = {0,0,res};
      return a;
    } else{
      return chosen_move;
    }
  }
  else{
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
    if (v == INT_MAX){
      // no child nodes
      int res = estimate_value_all(curr_board);
      move a = {0,0,res};
      return a;
    } else{
      return chosen_move;
    }
    return chosen_move;
  }
}


void print_field(int *field, int len){
  int i = 0;
  for (i = 0; i < len; i++){
    printf("%d,", field[i]);
  }
}


int curr_num;
void action(char *action, char *value)
{
  assert(action != NULL);
  assert(value != NULL);

  game_settings.timebank = atoi(value);
  if (game_settings.timebank < prev_timebank){
    search_depth--;
  } else if (game_settings.timebank == 10000 && prev_timebank == 10000){
    search_depth = MIN(search_depth+1, 10);
  }

  prev_timebank = game_settings.timebank;

  begin = clock();
  move m = recursive_search(&board, search_depth, game_settings.botid, INT_MIN, INT_MAX);
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  //printf("time spent: %lf\n", time_spent);

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









/// experimental
void flip_field(int* field){
  int i;
  for (i = 0; i < 81; i++){
    if (field[i] == 1 || field[i] == 2){
      field[i] = 3 - field[i];
    }
  }
}

void load_precomputed(){
  precomputed_solutions = calloc(729, sizeof(precomputed_solution));
  //precomputed_solutions[0]
}

precomputed_solution check_precomputed(int *field){
  int i = 0; 
  for (i = 0; i < 729; i++){
    if (memcmp(precomputed_solutions[i].field, field, 81) == 0){
      return precomputed_solutions[i];
    }
  }
}
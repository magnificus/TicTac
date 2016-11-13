#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <time.h>

void print_field(int *field, int len){
	int i = 0;
	for (i = 0; i < len; i++){
		printf("%d,", field[i]);
	}
}


void reset(int *field){
	int i = 0;
	for (i = 0; i < 81; i++){
		field[i] = 0;
	}
}

void set_macro(int active, int *macro) {
	int i = 0;
	for (i = 0; i < 9; i++){
		macro[i] = 0;
	}
	macro[active] = -1;
}

int main(int argc, char const *argv[]){
	printf("settings timebank 10000\n\n");
	printf("settings time_per_move 500\n\n");
	printf("settings player_names player1,player2\n\n");
	printf("settings your_bot player1\n\n");
	printf("settings your_botid 1\n\n");
	printf("update game round 1\n\n");
	printf("update game move 1\n\n");
	printf("update game field 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n\n");
	printf("update game macroboard -1,-1,-1,-1,-1,-1,-1,-1,-1\n\n");
	printf("action move 10000\n\n");

	int field[81];
	int macro[9];
	int i = 0;
	for (i = 0; i < 9; i++){
		int j = 0;
		for (j = 0; j < 9; j++){
			reset(field);
			field[i*9 + j] = 2;
			printf("\n\nupdate game field ");
			print_field(field, 81);
			printf("\n\nupdate game macroboard ");
			set_macro(j, macro);
			print_field(macro, 9);
			printf("\n\naction move 10000");
		}

	}

	//for ()


}
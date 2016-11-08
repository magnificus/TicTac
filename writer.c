#include <stdio.h>

int main(){
	int i = 0;
	for (i = 0; i < 9*9; i++){
		printf("&board.field[%d], ", i);
	}
}
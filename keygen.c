#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[]) {

  if (argc != 2){
		printf("Usage: ./keygen [number of characters]\n");
    printf("Example: ./keygen 10\n");
		exit (0);
	}

  int num_char, i;
  srand(time(NULL));

  num_char = atoi(argv[1]);

  for(i = 0;i < num_char;i++)
		printf("%c", (rand()%(90-65))+65); //65 is ASCII for capital A, 90 is ASCII for capital Z

  return 0;
}

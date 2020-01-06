build: battleship.c
	gcc -o battleship battleship.c -g -Wall -lm -lcurses
clean: battleship
	rm battleship

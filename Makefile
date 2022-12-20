all: main.c
	gcc -o zelda -Wall -g main.c -lglfw -lGLEW -lGL -lm

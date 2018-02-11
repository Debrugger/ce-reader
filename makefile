FLAGS := -g -Wall
INCLUDES :=  -I/usr/include

all:
	gcc $(FLAGS) $(INCLUDES) compress.c -o bin/compress

FLAGS := -g -Wall
INCLUDES :=  -I/usr/include


all:
	gcc $(FLAGS) $(INCLUDES) compress.c -o compress
	gcc $(FLAGS) $(INCLUDES) decompress.c -o decompress

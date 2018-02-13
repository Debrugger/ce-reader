FLAGS := -g -Wall
INCLUDES :=  -I/usr/include

.PHONY: all compress decompress

compress:
	gcc $(FLAGS) $(INCLUDES) compress.c -o compress

decompress:
	gcc $(FLAGS) $(INCLUDES) decompress.c -o decompress

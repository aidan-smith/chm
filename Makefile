.PHONY: all clean

CC = g++
CFLAGS = -g -Wall -Wextra -Werror

all:

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

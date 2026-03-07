CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -lncursesw -ljson-c

SRC = src/main.c src/pr.c src/json_parser.c src/ui.c
OBJ = $(SRC:.c=.o)
TARGET = pr-selector

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

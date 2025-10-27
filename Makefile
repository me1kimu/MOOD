CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c17 -pthread

SRC = main.c config.c sim.c hero.c monster.c combat.c sync.c
OBJ = $(SRC:.c=.o)

BIN = doom_sim

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

run: all
	./$(BIN) assets/ejemplo.cfg

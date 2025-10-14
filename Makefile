CC = gcc
CFLAGS = -Wall -Wextra -pthread -lm -std=c99
TARGET = doom_simulation
SOURCE = doom_simulation.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(SOURCE) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) config_example.txt

.PHONY: all clean run

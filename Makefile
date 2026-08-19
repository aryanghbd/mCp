CC = cc
CFLAGS = -Wall -Wextra -Werror -std=c11
TARGET = hello
SRC = $(wildcard src/*.c)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

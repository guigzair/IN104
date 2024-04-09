CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g 
LDFLAGS = -lSDL2 -lm -lSDL2_image test.c
SRC = SDL_example.c
EXECUTABLE = output

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

debug: $(EXECUTABLE)
	gdb ./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

SRC = main.c
EXE = main
INC = -Iinclude
LIBS = -lm -lpthread
CC = gcc
CFLAGS = -Wall -Werror
VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all --trace-children=yes -s

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(INC) $(SRC) -o $(EXE) $(LIBS)

test1: $(EXE) 
	chmod +x ./$(EXE) 
	./$(EXE) test/test1 1 

test2: $(EXE) 
	chmod +x ./$(EXE)
	./$(EXE) test/test2 5 

test3: $(EXE) 
	chmod +x ./$(EXE) 
	valgrind $(VFLAGS) ./$(EXE) test/test3 5 

.PHONY: all test1 test2 test3
.SILENT: test1 test2 test3

clean:
	rm -f $(EXE)

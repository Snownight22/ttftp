CC = gcc
DEBUG = -g
INC = -I./include/ 
LIBS = 
CFLAGS =

SRC_DIR = .        \
          session  \
		  command 

SRC = $(foreach dir,$(SRC_DIR), $(wildcard $(dir)/*.c))

BIN = ftpclient

all:$(SRC)
	$(CC) $(SRC) $(CFLAGS) $(DEBUG) $(INC) $(LIBS) -o $(BIN)

.PHONY:

clean:
	rm -rf $(BIN)

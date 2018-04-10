CC = gcc
DEBUG = -g
INC = -I./include/ 
LIBS = -lpthread 
CFLAGS =

SRC_DIR = .        \
          session  \
          ctrl 

SRC = $(foreach dir,$(SRC_DIR), $(wildcard $(dir)/*.c))

BIN = ftpclient

all:$(SRC)
	$(CC) $(SRC) $(CFLAGS) $(DEBUG) $(INC) $(LIBS) -o $(BIN)

.PHONY:

clean:
	rm -rf $(BIN)

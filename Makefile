CC = gcc
DEBUG =
INC = -I./include/ 
LIBS = 

SRC = ftp_main.c

OBJ = ftpclient

all:$(SRC)
	$(CC) $(SRC) $(DEBUG) $(INC) $(LIBS) -o $(OBJ)

.PHONY:

clean:
	rm -rf $(OBJ)

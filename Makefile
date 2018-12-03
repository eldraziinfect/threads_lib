

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: cthread clibthread
	

cthread: #dependências para a regra1
	$(CC) -c $(SRC_DIR)/cthread.c -Wall -o $(BIN_DIR)/cthread.o

clibthread:
	$(CC) -c $(SRC_DIR)/cthread.c -Wall -o $(BIN_DIR)/cthread.o
	ar rcs $(LIB_DIR)/libcthread.a $(BIN_DIR)/cthread.o $(BIN_DIR)/support.o

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/cthread.o 

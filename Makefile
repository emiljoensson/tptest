CC = gcc
LD = gcc
INC_DIR = include
SRC_DIR = src
OBJ_DIR = objects
CFLAGS = -g -Wall -I$(INC_DIR)
LDFLAGS = 
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/error.c $(SRC_DIR)/send_data.c
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/error.o $(OBJ_DIR)/send_data.o
PROG = tptest
RM = /bin/rm

# all: tptest

all: $(PROG)

# tptest: main.o error.o send_data.o
# 	gcc -o tptest main.o error.o send_data.o

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

#main.o: main.c
#	gcc -g -c main.c

#error.o: error.c
#	gcc -g -c error.c

#send_data.o: send_data.c
#	gcc -g -c send_data.c

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#clean:
#	rm main.o error.o send_data.o

clean:
	$(RM) $(PROG) $(OBJS)
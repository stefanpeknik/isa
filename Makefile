CC = g++
CFLAGS = -g -Wall -std=c++17

CLIENT_SRC_DIR = ./src/tftp-client
SERVER_SRC_DIR = ./src/tftp-server
COMMON_SRC_DIR = ./src/common

CLIENT_SRCS = $(wildcard $(CLIENT_SRC_DIR)/*.cpp) $(shell find $(COMMON_SRC_DIR) -type f -name '*.cpp')
SERVER_SRCS = $(wildcard $(SERVER_SRC_DIR)/*.cpp) $(shell find $(COMMON_SRC_DIR) -type f -name '*.cpp')

CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)

CLIENT_EXEC = tftp-client
SERVER_EXEC = tftp-server

all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC) $(CLIENT_OBJS) $(SERVER_OBJS)

pack:
	tar -cvf xpekni01.tar 

.PHONY: all clean pack

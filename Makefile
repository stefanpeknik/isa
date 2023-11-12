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

HOSTNAME = "127.0.0.1"
PORT = 2023
SERVER_DIR = "./server-files"
CLIENT_DIR = "./client-files"

# FILE_ON_SERVER = "doge-on-server.jpg"
# FILE_ON_CLIENT = "doge-on-client.jpg"
# FILE_FROM_SERVER = "doge-from-server.jpg"
# FILE_FROM_CLIENT = "doge-from-client.jpg"
# ---
FILE_ON_SERVER = "page-on-server.html"
FILE_ON_CLIENT = "page-on-client.html"
FILE_FROM_SERVER = "page-from-server.html"
FILE_FROM_CLIENT = "page-from-client.html"

all: $(CLIENT_EXEC) $(SERVER_EXEC)


clientr: $(CLIENT_EXEC)
	mkdir -p $(CLIENT_DIR)
	rm -f $(CLIENT_DIR)/$(FILE_FROM_SERVER)
	./$(CLIENT_EXEC) -h $(HOSTNAME) -p $(PORT) -f $(FILE_ON_SERVER) -t $(CLIENT_DIR)/$(FILE_FROM_SERVER) 

clientw: $(CLIENT_EXEC)
	mkdir -p $(CLIENT_DIR)
	rm -f $(SERVER_DIR)/$(FILE_FROM_CLIENT)
	./$(CLIENT_EXEC) -h $(HOSTNAME) -p $(PORT) -t $(FILE_FROM_CLIENT) < $(CLIENT_DIR)/$(FILE_ON_CLIENT)

server: $(SERVER_EXEC)
	mkdir -p $(SERVER_DIR)
	./$(SERVER_EXEC) -p $(PORT) $(SERVER_DIR) 

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC) $(CLIENT_OBJS) $(SERVER_OBJS)

.PHONY: all clean

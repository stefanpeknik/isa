# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

# Directories
SRC_DIR = src
COMMON_DIR = $(SRC_DIR)/common
CLIENT_DIR = $(SRC_DIR)/tftp-client
SERVER_DIR = $(SRC_DIR)/tftp-server
OBJ_DIR = obj

# Source files
COMMON_SRCS = $(wildcard $(COMMON_DIR)/*.cpp)
CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.cpp) $(COMMON_SRCS)
SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.cpp) $(COMMON_SRCS)

# Object files
COMMON_OBJS = $(patsubst $(COMMON_DIR)/%.cpp,$(OBJ_DIR)/common/%.o,$(COMMON_SRCS))
CLIENT_OBJS = $(patsubst $(CLIENT_DIR)/%.cpp,$(OBJ_DIR)/tftp-client/%.o,$(CLIENT_SRCS))
SERVER_OBJS = $(patsubst $(SERVER_DIR)/%.cpp,$(OBJ_DIR)/tftp-server/%.o,$(SERVER_SRCS))

# Executables
CLIENT_EXEC = tftp-client
SERVER_EXEC = tftp-server

# Targets
all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
$(OBJ_DIR)/common/%.o: $(COMMON_DIR)/%.cpp | $(OBJ_DIR)/common
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/tftp-client/%.o: $(CLIENT_DIR)/%.cpp | $(OBJ_DIR)/tftp-client
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/tftp-server/%.o: $(SERVER_DIR)/%.cpp | $(OBJ_DIR)/tftp-server
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/common:
	mkdir -p $@

$(OBJ_DIR)/tftp-client:
	mkdir -p $@

$(OBJ_DIR)/tftp-server:
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(CLIENT_EXEC) $(SERVER_EXEC)

.PHONY: all clean

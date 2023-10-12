# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Directories
SRC_DIR = src
COMMON_DIR = $(SRC_DIR)/common
TFTP_CLIENT_DIR = $(SRC_DIR)/tftp-client
TFTP_SERVER_DIR = $(SRC_DIR)/tftp-server

# Source files
COMMON_SRC = $(wildcard $(COMMON_DIR)/*.cpp)
TFTP_CLIENT_SRC = $(wildcard $(TFTP_CLIENT_DIR)/*.cpp)
TFTP_SERVER_SRC = $(wildcard $(TFTP_SERVER_DIR)/*.cpp)

# Object files
COMMON_OBJ = $(COMMON_SRC:.cpp=.o)
TFTP_CLIENT_OBJ = $(TFTP_CLIENT_SRC:.cpp=.o)
TFTP_SERVER_OBJ = $(TFTP_SERVER_SRC:.cpp=.o)

# Executables
TFTP_CLIENT_EXEC = tftp-client
TFTP_SERVER_EXEC = tftp-server

# Targets
all: $(TFTP_CLIENT_EXEC) $(TFTP_SERVER_EXEC)

$(TFTP_CLIENT_EXEC): $(COMMON_OBJ) $(TFTP_CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TFTP_SERVER_EXEC): $(COMMON_OBJ) $(TFTP_SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(COMMON_OBJ) $(TFTP_CLIENT_OBJ) $(TFTP_SERVER_OBJ) $(TFTP_CLIENT_EXEC) $(TFTP_SERVER_EXEC)

.PHONY: all clean

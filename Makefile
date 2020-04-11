PROJ_DIR = $(shell pwd)
BUILD_DIR = .build
CXX = clang++
CC = clang

DEBUG_FLAGS = -g
CXXFLAGS = -Wall -I $(PROJ_DIR) $(DEBUG_FLAGS)
CCFLAGS = $(CXXFLAGS)
LDLIBS = -lpthread


SOURCES = engine/commands.cpp  engine/engine.cpp network/server.c network/simple_socket_server.cpp
OBJECTS=$(SOURCES:%=$(BUILD_DIR)/%.o)

SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp

SERVER_EXEC=$(BUILD_DIR)/server.out
CLIENT_EXEC=$(BUILD_DIR)/client.out

all: $(BUILD_DIR) $(OBJECTS) $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(OBJECTS) $(SERVER_SRC:%=$(BUILD_DIR)/%.o)
$(CLIENT_EXEC): $(OBJECTS) $(CLIENT_SRC:%=$(BUILD_DIR)/%.o)
$(SERVER_EXEC) $(CLIENT_EXEC):
	$(CXX) $(CXXFLAGS) \
	       $(LDLIBS) \
		   $^ -o $@
# $^ means "prerequisites for this target", which are objects

$(BUILD_DIR)/%.c.o : %.c
	$(CC) $(CCFLAGS) $< -c -o $@

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

run serve: $(SERVER_EXEC)
	$(SERVER_EXEC)

client cli: $(CLIENT_EXEC)
	$(CLIENT_EXEC) localhost


$(BUILD_DIR):
	mkdir $(BUILD_DIR) -p
	mkdir $(BUILD_DIR)/network -p
	mkdir $(BUILD_DIR)/engine -p

format:
	clang-format -i --verbose --sort-includes -style=WebKit \
	    $(shell find . -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h")
	# clang-format -i --verbose --sort-includes -style=WebKit $(shell find . -name "*.hpp")
	# clang-format -i --verbose --sort-includes -style=WebKit $(shefind . -name "*.c")

clean:
	rm -rf *.out *.o
	rm -rf $(BUILD_DIR)


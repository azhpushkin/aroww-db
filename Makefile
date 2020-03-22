PROJ_DIR = $(shell pwd)
BUILD_DIR = .build
CXX = clang++
CC = clang

DEBUG_FLAGS = -g
CXXFLAGS = -Wall -I $(PROJ_DIR)
CCFLAGS = $(CXXFLAGS)
LDLIBS = -lpthread

CCSOURCES = network/server.c network/simple_socket_server.cpp
CXXSOURCES = engine/commands.cpp  engine/engine.cpp  main.cpp
CCOBJECTS=$(CCSOURCES:%.c=$(BUILD_DIR)/%.o)
CXXOBJECTS=$(CXXSOURCES:%.cpp=$(BUILD_DIR)/%.o)

EXECUTABLE = $(BUILD_DIR)/main.out

all: $(BUILD_DIR) $(CCOBJECTS) $(CXXOBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(BUILD_DIR) $(CCOBJECTS) $(CXXOBJECTS)
	$(CXX) $(CXXFLAGS) \
	       $(LDLIBS) \
		   $(CCOBJECTS) \
		   $(CXXOBJECTS) \
		   -o $@

$(BUILD_DIR)/%.o : %.c
	$(CC) $(CCFLAGS) $< -c -o $@

$(BUILD_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

run: $(EXECUTABLE)
	$(EXECUTABLE)

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


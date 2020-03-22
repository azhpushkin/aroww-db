BUILD_DIR = .build
CXX = clang++
CC = clang

DEBUG_FLAGS = -g
CXXFLAGS = -Wall
CCFLAGS = $(CXXFLAGS)
LDLIBS = -lpthread

CCSOURCES = server.c
CXXSOURCES = commands.cpp  engine.cpp  main.cpp  ui.cpp
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

format:
	clang-format -i --verbose --sort-includes -style=WebKit *.cpp *.c *.hpp

clean:
	rm -rf *.out *.o
	rm -rf $(BUILD_DIR)


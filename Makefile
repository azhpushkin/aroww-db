PROJ_DIR = $(shell pwd)
BUILD_DIR = .build
CC = clang++

DEBUG_FLAGS = -g
CCFLAGS = -std=c++17 -Wall -I $(PROJ_DIR) $(DEBUG_FLAGS)
LDLIBS = -pthread -lprotobuf -lpthread -lspdlog -lfmt


PROTO_OUT_DIR = proto_dist

SOURCES = engine/engine.cpp \
		  network/socket_server.cpp \
		  proto_dist/commands.pb.cc

OBJECTS=$(SOURCES:%=$(BUILD_DIR)/%.o)


SERVER_SRC = server.cpp
CLIENT_SRC = lib/aroww.cpp client.cpp 
TESTS_SRC = tests/tests_main.cpp tests/tests_engine.cpp

SERVER_EXEC=$(BUILD_DIR)/server.out
CLIENT_EXEC=$(BUILD_DIR)/client.out
TESTS_EXEC=$(BUILD_DIR)/tests.out

all: $(BUILD_DIR) proto $(OBJECTS) $(SERVER_EXEC) $(CLIENT_EXEC)

$(SERVER_EXEC): $(OBJECTS) $(SERVER_SRC:%=$(BUILD_DIR)/%.o)
$(CLIENT_EXEC): $(OBJECTS) $(CLIENT_SRC:%=$(BUILD_DIR)/%.o)
$(TESTS_EXEC): $(OBJECTS) $(TESTS_SRC:%=$(BUILD_DIR)/%.o)


$(CLIENT_EXEC): $(OBJECTS) $(CLIENT_SRC:%=$(BUILD_DIR)/%.o)
$(SERVER_EXEC) $(CLIENT_EXEC) $(TESTS_EXEC):
	$(CC) $(CCFLAGS) \
	       $(LDLIBS) \
		   $^ -o $@

lib: $(BUILD_DIR) $(OBJECTS)
	ar sr $(BUILD_DIR)/lib.a $(OBJECTS) .build/lib/aroww.cpp.o
	ar -t $(BUILD_DIR)/lib.a
	cp $(BUILD_DIR)/lib.a  bench/
# $^ means "prerequisites for this target", which are objects

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(CC) $(CCFLAGS) $< -c -o $@

$(BUILD_DIR)/%.cc.o : %.cc
	$(CC) $(CCFLAGS) $< -c -o $@

run serve: $(SERVER_EXEC)
	$(SERVER_EXEC)

client cli: $(CLIENT_EXEC)
	$(CLIENT_EXEC) localhost

test tests: $(BUILD_DIR) $(TESTS_EXEC)
	$(TESTS_EXEC)

$(BUILD_DIR):
	mkdir $(BUILD_DIR) -p
	mkdir $(BUILD_DIR)/network -p
	mkdir $(BUILD_DIR)/engine -p
	mkdir $(BUILD_DIR)/proto_dist -p
	mkdir $(BUILD_DIR)/lib -p
	mkdir $(BUILD_DIR)/tests -p

proto:
	mkdir $(PROTO_OUT_DIR) -p
	protoc -I $(PROJ_DIR) --cpp_out=$(PROTO_OUT_DIR) commands.proto

format:
	clang-format -i --verbose --sort-includes -style=WebKit \
	    $(shell find . -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h")
	# clang-format -i --verbose --sort-includes -style=WebKit $(shell find . -name "*.hpp")
	# clang-format -i --verbose --sort-includes -style=WebKit $(shefind . -name "*.c")

clean:
	rm -rf *.out *.o
	rm -rf $(BUILD_DIR)


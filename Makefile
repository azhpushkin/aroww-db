.PHONY: *

all:
	clang++ -x c server.c -x c++ main.cpp engine.cpp ui.cpp commands.cpp -lpthread -o main.out

run:
	./main.out

format:
	clang-format -i --verbose --sort-includes -style=WebKit *.cpp
	clang-format -i --verbose --sort-includes -style=WebKit *.hpp

clean:
	rm -f *.out

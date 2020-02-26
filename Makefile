.PHONY: *

all:
	clang++ main.cpp engine.cpp ui.cpp commands.cpp -o main.out

run:
	./main.out

clean:
	rm -f *.out

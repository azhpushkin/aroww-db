.PHONY: *

all:
	g++ echo.cpp -o main.out && ./main.out

clean:
	rm -f *.out

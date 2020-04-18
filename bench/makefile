psql:
	clang++ -g postgers_connection.cpp run.cpp base_connection.cpp -std=c++17 -lpqxx -lpq -O3 -o bench.out

ar:
	clang++ run.cpp base_connection.cpp ar.cpp lib.a -lprotobuf -std=c++17 -o bench.out

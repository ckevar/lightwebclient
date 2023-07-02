CPP = g++

main: main.cpp WebClientSSL.cpp
	$(CPP) -o main main.cpp WebClientSSL.cpp -lssl -lcrypto -Wall

mdebug: main.cpp WebClientSSL.cpp
	$(CPP) -o mdebug main.cpp WebClientSSL.cpp -lssl -lcrypto -Wall -DDEBUG_CHUNK_SIZES

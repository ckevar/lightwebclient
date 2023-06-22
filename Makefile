CPP = g++

main: main.cpp WebClientSSL.cpp
	$(CPP) -o main main.cpp WebClientSSL.cpp -lssl -lcrypto -Wall
CPP = g++

main: main.cpp PapayitaWC.cpp
	$(CPP) -o main main.cpp PapayitaWC.cpp -lssl -lcrypto -Wall

mdebug: main.cpp PapayitaWC.cpp
	$(CPP) -o mdebug main.cpp PapayitaWC.cpp -lssl -lcrypto -Wall -DDEBUG_CHUNK_SIZES

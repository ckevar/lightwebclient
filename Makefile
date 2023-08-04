CPP = g++
CFLAGS = -lssl -lcrypto -Wall -O2

main: main.cpp
	$(CPP) -o $@ $^ $(CFLAGS) -lpapayitawc

PapayitaWC.o: PapayitaWC.cpp PapayitaWC.h
	$(CPP) -c PapayitaWC.cpp $(CFLAGS)

main_o: main.cpp PapayitaWC.o
	$(CPP) -o $@ $^ $(CFLAGS)

debug: main.cpp PapayitaWC.o
	$(CPP) -o mdebug $^ $(CFLAGS) -DDEBUG_CHUNK_SIZES

install: libpapayitawc.so
	@cp $^ /usr/lib/
	@cp PapayitaWC.h /usr/include/
	@ldconfig

libpapayitawc.so: PapayitaWC.cpp PapayitaWC.h
	$(CPP) -fPIC -shared -o $@ PapayitaWC.cpp $(CFLAGS)

test: test/test.cpp
	$(CPP) -o $@ $^ $(CFLAGS) -lpapayitawc

clear:
	rm main main_o *.o mdebug *.so

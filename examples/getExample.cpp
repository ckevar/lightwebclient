#include <iostream>
#include <stdio.h>
#include <cstring>

#include "PapayitaWC.h"
#include "apikeys.h"
#include "BufferSizes.h"

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << "you need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	PapayitaWC webClient(argv[1]);
	if(webClient.get_error() < 0)
		return -1;

	int buff_size = BUFFSIZE_WWW_GOOGLE_COM;
	char buff[BUFFSIZE_WWW_GOOGLE_COM];

	webClient.get("/", buff, buff_size);

	return 0;
}
#include <iostream>
#include <stdio.h>
#include <cstring>

#include "WebClientSSL.h"
#include "apikeys.h"
#include "BufferSizes.h"

/* If you know that  the transmision enconding is by chuncks, like www.google.com
   Modify the following line on WebClientSSL.cpp, it's close to the end
	poll(&m_pfd, 1, 0);
	change as:
	poll(&m_pfd, 1, 10);
	I'm working on this
*/

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << "you need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	WebClientSSL webClient(argv[1]);
	if(webClient.get_error() < 0)
		return -1;

	int buff_size = BUFFSIZE_WWW_GOOGLE_COM;
	char buff[BUFFSIZE_WWW_GOOGLE_COM];

	webClient.get("/", 1, buff, buff_size);

	return 0;
}
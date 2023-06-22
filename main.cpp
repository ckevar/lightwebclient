#include "WebClientSSL.h"
#include <iostream>

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << "you need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	WebClientSSL webClient(argv[1]);
	int buff_size = 6400; // for hostelworld
	char buff[buff_size];

	if(webClient.get_error() < 0)
		return -1;
	// webClient.set_header("Authorization: bxase\r\n", 22);
	// webClient.set_header("Authorization-2: bxase2\r\n", 25);

	webClient.get("/", 1, buff, buff_size);
	// webClient.post(resource, content, content_length);
	
	//webClient.free() /* No needed the WebClientSSL destructor */
	return 0;
}
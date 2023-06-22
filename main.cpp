#include "WebClientSSL.h"
#include <iostream>

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << "you need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	WebClientSSL webClient(argv[1]);
	int buff_size = 700; // for hostelworld
	char buff[buff_size];

	if(webClient.get_error() < 0)
		return -1;
	// webClient.set_header("Authorization: bxase\r\n", 22);
	// webClient.set_header("Authorization-2: bxase2\r\n", 25);

	webClient.get("/bot6234049897:AAGWLi4-5vCQwQhraFUBgH-me3970XO1byg/getMe", 56, buff, buff_size);
	// webClient.post(resource, content, content_length);
	
	//webClient.free() /* No needed the WebClientSSL destructor */
	return 0;
}
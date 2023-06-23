#include <iostream>
#include <stdio.h>

#include "WebClientSSL.h"
/* In case you have you have to perform some api-keying
   create this file apikeys.h and include it here */
// #include "apikeys.h"

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << "you need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	WebClientSSL webClient(argv[1]);
	char resource[57];
	int buff_size = BUFFSIZE_API_TELEGRAM_ORG_GETME;
	char buff[BUFFSIZE_API_TELEGRAM_ORG_GETME];

	if(webClient.get_error() < 0)
		return -1;
	// webClient.set_header("Authorization: bxase\r\n", 22);
	// webClient.set_header("Authorization-2: bxase2\r\n", 25);


	sprintf(resource, "/bot%s/getMe", TG_TOKEN);

	webClient.get(resource, TG_TOKEN_LEN + 10, buff, buff_size);
	// webClient.post(resource, content, content_length);
	
	//webClient.free() /* No needed the WebClientSSL destructor */
	return 0;
}
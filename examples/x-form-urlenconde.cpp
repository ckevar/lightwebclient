#include <iostream>
#include <stdio.h>
#include <cstring>

#include "PapayitaWC.h"
/* In case you have you have to perform some api-keying
   create this file apikeys.h and include it here */
#include "apikeys.h"
#include "BufferSizes.h"

/* Include this only if you're gonna handler x-www-forms or javascript enconding*/
#include <map> 

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << "you need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	PapayitaWC webClient(argv[1]);
	if(webClient.get_error() < 0)
		return -1;

	char resource[100];
	int buff_size = 2*BUFFSIZE_API_TELEGRAM_ORG_GET_ME;
	char buff[2*BUFFSIZE_API_TELEGRAM_ORG_GET_ME];

	std::map<const char *, const char*> content;
	content["parse_mode"] = "HTML";
	content["chat_id"] = TELEGRAM_CHAT_ID;
	content["text"] = "test test?, <b>test</b>\ntest";


	sprintf(resource, "/bot%s/sendMessage", TG_TOKEN);
	webClient.set_header("Content-Type: application/x-www-form-urlencoded");
	int buff_in = WebClient_urlencode(buff, content);
	webClient.post(resource, buff, buff_in, buff_size);
	
	return 0;
}
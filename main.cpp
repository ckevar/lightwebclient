#include <iostream>
#include <stdio.h>
#include <cstring>

#include "WebClientSSL.h"
/* In case you have you have to perform some api-keying
   create this file apikeys.h and include it here */
#include "apikeys.h"
#include "BufferSizes.h"

/* Include this only if you're gonna handler x-www-forms or javascript enconding*/
#include <map> 

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

	char resource[100];
	int buff_size = 2*BUFFSIZE_API_TELEGRAM_ORG_GET_ME;
	char buff[2*BUFFSIZE_API_TELEGRAM_ORG_GET_ME];

	std::map<const char *, const char*> content;
	content["parse_mode"] = "HTML";
	content["chat_id"] = TELEGRAM_CHAT_ID;
	content["text"] = "test test?, <b>test</b>\ntest";


	sprintf(resource, "/bot%s/sendMessage", TG_TOKEN);
	webClient.set_header("Content-Type: application/x-www-form-urlencoded\r\n", 49);
	int buff_in = WebClient_urlencode(buff, content);
	webClient.post(resource, buff, buff_in, buff_size);

	/* EXAMPLES */
	// sprintf(resource, "/bot%s/sendMessage", TG_TOKEN);
	// webClient.set_header("Content-Type: application/x-www-form-urlencoded\r\n", 49);
	// memcpy(buff, "parse_mode=HTML&text=test+test%3F%2C+%3Cb%3Etest%3C%2Fb%3E&chat_id=597192342", 76);
	// webClient.post(resource, TG_TOKEN_LEN + 16, buff, 76, buff_size);

	// sprintf(resource, "/bot%s/sendMessage", TG_TOKEN);
	// memcpy(resource + TG_TOKEN_LEN + 16, "?parse_mode=HTML&text=test+test%3F%2C+%3Cb%3Etest%3C%2Fb%3E&chat_id=TELEGRAM_CHAT_ID", 77);
	// webClient.post(resource, TG_TOKEN_LEN + 16 + 77, buff, 0, buff_size);
	
	// sprintf(resource, "/bot%s/sendMessage", TG_TOKEN);
	// memcpy(resource + TG_TOKEN_LEN + 16, "?parse_mode=HTML&text=test+test%3F%2C+%3Cb%3Etest%3C%2Fb%3E&chat_id=TELEGRAM_CHAT_ID", 77);
	// webClient.get(resource, TG_TOKEN_LEN + 16 + 77, buff, buff_size);
	
	// sprintf(resource, "/", TG_TOKEN);
	// webClient.get("/", 1, buff, buff_size);

	return 0;
}
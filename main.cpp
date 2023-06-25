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
		std::cerr << "You need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	WebClientSSL webClient(argv[1]);
	if(webClient.get_error() < 0)
		return -1;

	char collection[] = "contacts";
	char itemTag[] = "tomorrowcontacts";
	unsigned short buff_size = 20000;
	char buff[buff_size];
	char Api_Key[MDB_TOKEN_LEN + 9];
	sprintf(Api_Key, "Api-Key: %s",MDB_TOKEN);

	webClient.set_header("Content-Type: application/json");
	webClient.set_header(Api_Key);

	unsigned short buff_in = sprintf(buff, "{\
\"dataSource\":\"hpt\",\
\"collection\":\"%s\",\
\"database\":\"warehouse\",\
\"filter\":{\
\"tag\":\"%s\"\
}}", collection, itemTag);

	webClient.post("/app/data-esgfv/endpoint/data/v1/action/findOne", buff, buff_in, buff_size);
	// webClient.show_request_headers();
	return 0;
}
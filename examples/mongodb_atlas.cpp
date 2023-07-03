#include <iostream>
#include <stdio.h>
#include <cstring>

#include "PapayitaWC.h"
#include "BufferSizes.h"

/* In case you have you have to perform some api-keying
   create this file apikeys.h and include it here */
#include "apikeys.h"

int main(int argc, char const *argv[])
{
	if (argc < 2) {
		std::cerr << "You need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	PapayitaWC webClient(argv[1]);
	if(webClient.get_error() < 0)
		return -1;

	char collection[] = "contacts";
	char itemTag[] = "tomorrowcontacts";
	unsigned short buff_size = 20000;
	char buff[buff_size];
	char Api_Key[MDB_TOKEN_LEN + 9];

	sprintf(Api_Key, "Api-Key: %s", MDB_TOKEN); /* Here it goes your API-KEY */

	webClient.set_header("Content-Type: application/json");
	webClient.set_header(Api_Key);

	unsigned short buff_io = sprintf(buff, "{\
\"dataSource\":\"<YOUR_DATA_SOURCE>\",\
\"collection\":\"%s\",\
\"database\":\"<YOUR_DATABASE>\",\
\"filter\":{\
\"tag\":\"%s\"\
}}", collection, itemTag);

	buff_io = webClient.post("/app/data-esgfv/endpoint/data/v1/action/findOne", buff, buff_io, buff_size);
	if (buff_io <= 0) {
		std::cerr << "[ERROR:] nothing was return from server" << std::endl;
		return buff_io;
	}

	char *buff_it = buff + webClient.responseHeader_size() + 1;

	/* check if replied  document is null or not */
	while(buff_it < (buff + buff_io)) {
		if (*buff_it == '\"') {
			if (memcmp(buff_it, "\"document\":", 11) == 0) {
				buff_it += 11;
				if (memcmp(buff_it, "null", 4) == 0) {
					std::cerr << "[ERROR:MDB:] document null" << std::endl;
					return 1;
				}
				else
					break;
			} 
		}
		buff_it++;
	}
	if(buff_it >= (buff + buff_io)) {
		std::cerr << "[ERROR:] no document field was found" << std::endl;
	}

	/* Get Phones numbers reservations from HostelWORLD */
	std::cerr << buff_it << std::endl;

	return 0;
}
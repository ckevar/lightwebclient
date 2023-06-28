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

	unsigned short buff_io = sprintf(buff, "{\
\"dataSource\":\"hpt\",\
\"collection\":\"%s\",\
\"database\":\"warehouse\",\
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
	// std::cerr << buff_it << std::endl;
	char *PhoneNumber[35]; 
	char *resourceName[35];
	char *reservationCode[35]; 
	char **PhoneNumber_it = PhoneNumber; 
	char **resourceName_it = resourceName; 
	char **reservationCode_it = reservationCode; 
	char *PhoneNumber_tmp;
	char isHostelworldGuest = 0;

	while(buff_it < (buff + buff_io)) {
		if(*buff_it == '\"') {
			buff_it++;
			if (*buff_it == 'p') {
				if (memcmp(buff_it, "phoneNumbers\":[{\"value\":", 24) == 0) {
					buff_it += 24;
					PhoneNumber_tmp = buff_it;
					while(*buff_it != ',') buff_it++;
					*buff_it = 0;
				} 
			}
			else if (*buff_it == 'B') {
				if (memcmp(buff_it, "BookingMethodId\",\"value\":\"2\"", 28) == 0) {
					buff_it += 28;
					while(*buff_it != ',') buff_it++;
					*buff_it = 0;
					*PhoneNumber_it = PhoneNumber_tmp;
					PhoneNumber_it++;
					isHostelworldGuest = 1;
				}
			}
			else if (*buff_it == 'R') {
				if (memcmp(buff_it, "ReservationCode\",\"value\":", 25) == 0 && isHostelworldGuest) {
					buff_it += 25;
					*reservationCode_it = buff_it;
					while(*buff_it != ',' && *buff_it != '}') buff_it++;
					*buff_it = 0;
					std::cerr << *reservationCode_it << std::endl;
				}
			}
			else if (*buff_it == 'r') {
				// std::cerr << buff_it << std::endl;
				if (memcmp(buff_it, "resourceName\":", 14) == 0 && isHostelworldGuest) {
					buff_it += 14;
					*resourceName_it = buff_it;
					while(*buff_it != ',' && *buff_it != '}') buff_it++;
					*buff_it = 0;
					std::cerr << *resourceName_it << std::endl;
					isHostelworldGuest = 0;
				}
			}
		}
		buff_it++;
	}

	/* Mongo DB ends here */
	webClient.terminate_session();

	/* Hostelworld starts here */
	webClient.new_session("inbox.hostelworld.com");
	if(webClient.get_error() < 0)
		return -1;
	buff_io = webClient.get("/", buff, 6000);
	std::cerr << buff << std::endl;

	return 0;
}
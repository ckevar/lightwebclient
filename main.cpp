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
	unsigned short buff_size = 22000;
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

	/* MONGODB APPLICATION STUFF */
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
	/*
		TODO:
		- Copy the resourceNames and Reservation Codes to another char array
	*/
	char resourceName[35][29];
	char reservationCode[35][17]; 
	char *PhoneNumber_tmp;
	char *buff_it_tmp = 0;
	char isHostelworldGuest = 0;
	int contact_it = 0; 

	while(buff_it < (buff + buff_io)) {
		if(*buff_it == '\"') {
			buff_it++;
			/* Get Phone Numbers temporarily*/
			if (*buff_it == 'p') {
				if (memcmp(buff_it, "phoneNumbers\":[{\"value\":", 24) == 0) {
					buff_it += 24;
					PhoneNumber_tmp = buff_it;
					while(*buff_it != ',') buff_it++;
				} 
			}
			/* if Booking Method is HostelWorld (2), verify if the number is not
			saved yet */
			else if (*buff_it == 'B') {
				if (memcmp(buff_it, "BookingMethodId\",\"value\":\"2\"", 28) == 0) {
					buff_it += 28;
					while(*buff_it != ',' && *buff_it != '}') buff_it++;
					if (*PhoneNumber_tmp == '\"' && *(PhoneNumber_tmp + 1) == '\"')
						isHostelworldGuest = 1;
				}
			}

			/* Get the reservation Code from those who are from HostelWorld and do 
			not seem to have a phone number */
			else if (*buff_it == 'R') {
				if (memcmp(buff_it, "ReservationCode\",\"value\":\"", 26) == 0 && isHostelworldGuest) {
					buff_it += 26;
					buff_it_tmp = buff_it;
					while(*buff_it != '\"') buff_it++;
					memcpy(reservationCode[contact_it], buff_it_tmp, buff_it - buff_it_tmp);
				}
			}
			/* Get the resourceName, to update People API (Google Contacts) */
			else if (*buff_it == 'r') {
				if (memcmp(buff_it, "resourceName\":", 14) == 0 && isHostelworldGuest) {
					buff_it += 14;
					buff_it_tmp = buff_it;
					while(*buff_it != ',' && *buff_it != '}') buff_it++;
					memcpy(resourceName[contact_it], buff_it_tmp, buff_it - buff_it_tmp);
					contact_it++;
					isHostelworldGuest = 0;
				}
			}
		}
		buff_it++;
	}

	/* Mongo DB ends here */
	webClient.terminate_session();

	/* 
		Hostelworld starts here, get cookie 
	*/
	std::cout << "^^^ GETTING TOKEN AND COOKIE" << std::endl;

	webClient.new_session("inbox.hostelworld.com");
	if(webClient.get_error() < 0)
		return -1;

	/* Get Method */
	buff_io = webClient.get("/", buff, 6000);
	char *Cookie = NULL;
	int CookieLen = webClient.Cookie(&Cookie);

	/* HOSTELWORLD APPLICATION STUFF, going straight to the content */
	buff_it = buff + webClient.responseHeader_size() + 1;
	
	/* 
		search for the formToken 
	 	i.e.: formToken" value="35be80b1fb04f845bac6b58726affc1a2b7d247f"
	*/
	char *formToken = NULL;
	while(buff_it < (buff + buff_io)) {
		if (*buff_it == 'f') {
			if (*(buff_it + 9) == '\"') {
				if (memcmp(buff_it, "formToken\" value=\"", 18) == 0) {
					buff_it += 18;
					formToken = buff_it;
					while(*buff_it != '\"') buff_it++;
					*buff_it = 0;
					break;
				}
			}
		}
		buff_it++;
	}
	if(buff_it >= (buff + buff_io))
		std::cerr << "[ERROR:] no formToken was found" << std::endl;

	/* Cookie and Token ends here */
	webClient.terminate_session();

	/* 
		LOGIN 
		TODO: test this sections, get tokens
	*/
	std::cout << "^^^ LOGIN " << std::endl;

	webClient.new_session("inbox.hostelworld.com");
	if(webClient.get_error() < 0)
		return -1;
	char CookieHeader[150];
	memcpy(CookieHeader, "Cookie: ", 8);
	memcpy(CookieHeader + 8, Cookie, CookieLen);

	std::map<const char *, const char*> content;
	content["formToken"] = formToken;
	content["SessionLanguage"] = HOSTELWORLD_SESSION_LANGUAGE;
	content["HostelNumber"] = HOSTELWORLD_HOSTEL_NUMBER;
	content["Username"] = HOSTELWORLD_USERNAME;
	content["Password"] = HOSTELWORLD_PASSWORD;

	webClient.set_header(CookieHeader);
	webClient.set_header("Content-Type: application/x-www-form-urlencoded");
	buff_io = WebClient_urlencode(buff, content);
	buff_io = webClient.post("/inbox/trylogin.php", buff, buff_io, 6000);	

	CookieLen = webClient.Cookie(&Cookie);

	/* Login ends here */
	webClient.terminate_session();

	/* 
		BOOKING VIEW -> iterate this part on reservation Codes
		TODO: test this section
	*/

	std::cout << "^^^ BOOKING VIEW " << std::endl;

	char reservationCodeTest[] = "554622933";
	char resource[50];

	webClient.new_session("inbox.hostelworld.com");
	if (webClient.get_error() < 0)
		return -1;

	char *c = Cookie;
	while(*c != ';') c++; // just selecting the first cookie, the InboxCookie
	memcpy(CookieHeader, "Cookie: ", 8);
	memcpy(CookieHeader + 8, Cookie, c - Cookie);

	sprintf(resource, "/booking/view/%s", reservationCodeTest);
	webClient.set_header(CookieHeader);
	memset(buff, 0, buff_size);

	buff_io = webClient.get(resource, buff, buff_size);
	buff_it = buff + webClient.responseHeader_size() + 1;
	// std::cerr << "reading bytes " << buff_io << " BUFF " << buff << std::endl;
	//------------------------------------------------------------//

	// std::cout << "BOOKING VIEW RESPONSE -------------" << buff_io << std::endl;
	// std::cerr << buff << std::endl;
	// std::cout << "-----------------------------------" << std::endl << std::endl;
 
	// char bookingReferenceFound = 0;
	// while(buff_it < (buff + buff_io)) {
	// 	if (*buff_it == 'b') {
	// 		if (*(buff_it + 7) == 'R') {
	// 			if(memcmp (buff_it, "bookingReference", 16) == 0) {
	// 				bookingReferenceFound = 1;
	// 			}
	// 		}
	// 	} 
	// 	if (bookingReferenceFound) {
	// 		if (*buff_it == 'p') {
	// 			if (*(buff_it + 6) == ':') {

	// 				while(*buff_it != ',') buff_it++;
	// 				buff_it = 0;
	// 				std::cerr << buff << std::endl;
	// 				break;
	// 			}
	// 		}
	// 	}
	// 	buff_it++;
	// }
	// if (buff_it >= buff + buff_io) 
	// 	std::cerr << "[ERROR:] phone number not found" << std::endl; 

	/* Search for phone Number Ends Here */
	// webClient.terminate_session();

	/* New session on MongoDB, to update the phone number */
	/* New session on google, to update the phone number on People API */

	/* New session on google, to generate the links */

	return 0;
}
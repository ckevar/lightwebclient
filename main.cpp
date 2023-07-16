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
		std::cerr << "You need to a domain name, such as www.google.com" << std::endl; 
		return -1;
	}

	PapayitaWC papayita(argv[1]);
	if(papayita.get_error() < 0)
		return -1;

	char collection[] = "contacts";
	char itemTag[] = "tomorrowcontacts";
	unsigned short buff_size = 22000;
	char buff[buff_size];
	char Api_Key[MDB_TOKEN_LEN + 9];

	sprintf(Api_Key, "Api-Key: %s",MDB_TOKEN);

	papayita.set_header("Content-Type: application/json");
	papayita.set_header(Api_Key);

	unsigned short buff_io = sprintf(buff, "{\
\"dataSource\":\"hpt\",\
\"collection\":\"%s\",\
\"database\":\"warehouse\",\
\"filter\":{\
\"tag\":\"%s\"\
}}", collection, itemTag);

	buff_io = papayita.post("/app/data-esgfv/endpoint/data/v1/action/findOne", buff, buff_io, buff_size);
	if (buff_io <= 0) {
		std::cerr << "[ERROR:] nothing was return from server" << std::endl;
		return buff_io;
	}

	char *buff_it = buff + papayita.responseHeader_size() + 1;

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
	char resourceName[35][30];
	char reservationCode[35][20]; 
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
	papayita.terminate_session();

	/* 
		Hostelworld starts here, get cookie 
	*/
	std::cout << "^^^ GETTING TOKEN AND COOKIE" << std::endl;

	papayita.new_session("inbox.hostelworld.com");
	if(papayita.get_error() < 0)
		return -1;

	/* Get Method */
	buff_io = papayita.get("/", buff, 6000);
	papayita.show_requestHeaders();
	char *Cookie = NULL;
	papayita.Cookie(&Cookie);

	/* HOSTELWORLD APPLICATION STUFF, going straight to the content */
	buff_it = buff + papayita.responseHeader_size() + 1;
	
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
	if(buff_it >= (buff + buff_io)) {
		std::cerr << "[ERROR:] no formToken was found" << std::endl;
		return -1;
	}

	/* Cookie and Token ends here */
	papayita.terminate_session();

	/* 
		LOGIN 
		TODO: test this sections, get tokens
	*/
	std::cout << "^^^ LOGIN " << std::endl;

	papayita.new_session("inbox.hostelworld.com");
	if(papayita.get_error() < 0)
		return -1;

	char CookieHeader[250];
	char *c = Cookie;
	while(*c != ';') c++; // just selecting the first cookie, the InboxCookie
	memcpy(CookieHeader, "Cookie: ", 8);
	memcpy(CookieHeader + 8, Cookie, c - Cookie);
	*(CookieHeader + 8 + (c - Cookie)) = 0;

	std::map<const char *, const char*> content;
	content["formToken"] = formToken;
	content["SessionLanguage"] = HOSTELWORLD_SESSION_LANGUAGE;
	content["HostelNumber"] = HOSTELWORLD_HOSTEL_NUMBER;
	content["Username"] = HOSTELWORLD_USERNAME;
	content["Password"] = HOSTELWORLD_PASSWORD;

	papayita.set_header(CookieHeader);
	papayita.set_header("Content-Type: application/x-www-form-urlencoded");
	buff_io = WebClient_urlencode(buff, content);
	buff_io = papayita.post("/inbox/trylogin.php", buff, buff_io, 6000);	

	papayita.Cookie(&Cookie);

	/* Login ends here */
	papayita.terminate_session();

	/* 
		BOOKING VIEW -> iterate this part on reservation Codes
		TODO: test this section
	*/

	std::cout << "^^^ BOOKING VIEW " << std::endl;

	char phoneNumbers[35][20];
	char resource[50];
	int i = 0;

	c = Cookie;
	while(*c != ';') c++; // just selecting the first cookie, the InboxCookie
	memcpy(CookieHeader, "Cookie: ", 8);
	memcpy(CookieHeader + 8, Cookie, c - Cookie);
	*(CookieHeader + 8 + (c - Cookie)) = 0;

	while(i < contact_it) {
		papayita.new_session("inbox.hostelworld.com");
		if (papayita.get_error() < 0)
			return -1;

		sprintf(resource, "/booking/view/%s", reservationCode[i] + 6); /* + 6 skips the hostel Number */
		papayita.set_header(CookieHeader);
		memset(buff + buff_size/2, 0, buff_size/2); /* IMPORTANT, since pages are all similar, 
												there might the possibility to grab previous
												data or at least the bottom half*/
		std::cout << resource << std::endl;
		buff_io = papayita.get(resource, buff, buff_size);
		buff_it = buff + papayita.responseHeader_size() + 1;

		char bookingReferenceFound = 0;
		char *tmp;
		memcpy(phoneNumbers[i], "null\0", 5);

		while(buff_it < (buff + buff_io)) {
			if (*buff_it == 'b' && bookingReferenceFound == 0) {
				if (*(buff_it + 7) == 'R') {
					if(memcmp (buff_it, "bookingReference\":\"", 19) == 0) {
						buff_it += 25; /* it skips "52458-" too */
						tmp = buff_it;
						while(*buff_it != '\"') buff_it++;
						if (memcmp(tmp, reservationCode[i] + 6, buff_it - tmp) == 0)
							bookingReferenceFound = 1;
					}
				}
			} 
			if (bookingReferenceFound) {
				if (*buff_it == 'p') {
					if (*(buff_it + 7) == '\"') {
						buff_it += 8; /* Skips '"' */
						tmp = buff_it;
						
						if (*buff_it == '\"') break;
						while(*buff_it != ',') buff_it++;
						
						if (buff_it - tmp < 4) break;

						if (*tmp != '+') {
							tmp--;
							*tmp = '+';
						}

						memcpy(phoneNumbers[i], tmp, buff_it - tmp);
						break;
					}
				}
			}
			buff_it++;
		}
		if (buff_it >= buff + buff_io) {
			if (bookingReferenceFound)
				std::cerr << "[ERROR:] phone number field was not found" << std::endl;
			else 
				std::cerr << "[ERROR:] bookingReference field was not found" << std::endl; 
		}
		else if (*phoneNumbers[i] == 'n' && bookingReferenceFound) {
			std::cerr << "[ERROR:] No phone number on this contact" << std::endl;
		}
		std::cout << reservationCode[i] << ' ' << phoneNumbers[i] << std::endl;
		i++;

		/* Search for phone Number Ends Here */
		papayita.terminate_session();
	}

	/* New session on MongoDB, to update the phone number */
	/* New session on emulating messaging, to update the phone number on People API */
	/* New session on emulating messaging, to generate the links */

	return 0;
}
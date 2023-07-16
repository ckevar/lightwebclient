#include "PapayitaWC.h"

#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <poll.h>

/* 
	This client:
 	- supports only HTTP/1.1
 	- Supports only deflate encoding, futures versions will accept br
 	- Does not support Upgrade-Insecure-Requests
 	- Does not support Cache-Control
*/

PapayitaWC::PapayitaWC(const char *host) {

	/* Init SSL */
	preConstructor();

	if (m_ctx != nullptr) 
		new_session(host);
	else {
		std::cerr << "[ERROR:] SST_CTX_new returned nullptr." << std::endl;
		m_error = -1;
	}
}

PapayitaWC::PapayitaWC() {

	/* Init SSL */
	preConstructor();
	if (m_ctx == nullptr) {
		std::cerr << "[ERROR:] SST_CTX_new returned nullptr." << std::endl;
		m_error = -1;
	}
}

void PapayitaWC::preConstructor() {
	OpenSSL_add_all_algorithms(); // load & register all cryptos
	SSL_load_error_strings();
	const SSL_METHOD *method = SSLv23_client_method(); /* Create new client-method instance */	
	m_ctx = SSL_CTX_new(method);
}

void PapayitaWC::new_session(const char *host) {
	xtraheaders_i = 0;
	m_hostname = (char *) host;
	isShowRequest = 0;
	isSessionTerminated = 0;

	m_ssl = SSL_new(m_ctx);
	if (m_ssl != nullptr) {

		SSL_set_tlsext_host_name(m_ssl, m_hostname);
		m_error = OpenConnection(); /* defines m_fd */
		if (m_error == 0) {
			SSL_set_fd(m_ssl, m_fd);
			m_error = SSL_connect(m_ssl);
		}
	}
	else {
		std::cerr << "[ERROR:] SSL_new returned nullptr." << std::endl;
		m_error = -2;
	}
}

int PapayitaWC::OpenConnection() {
	struct hostent *host;
	
	if((host = gethostbyname(m_hostname)) == nullptr) {
		std::cerr << "[ERROR:] gethostbyname returned nullptr." << std::endl; 
		return -3;
	}
	
	struct addrinfo hints = {0}, *addrs;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if(getaddrinfo(m_hostname, HTTPS_PORT, &hints, &addrs) != 0) {
		std::cerr << "[ERROR:] on getaddrinfo." << std::endl;
		return -4;
	}

	for (struct addrinfo *addr = addrs; addr != nullptr; addr = addr->ai_next) {
        m_fd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (m_fd == -1)
            continue;

        if (connect(m_fd, addr->ai_addr, addr->ai_addrlen) == 0)
            break;

        m_fd = -1;
        close(m_fd);
    }

    freeaddrinfo(addrs);

	if(m_fd == -1) {
		std::cerr << "[ERROR:] No file description could be defined." << std::endl;
		return -5;
	}

	return 0;
}

	
void PapayitaWC::show_certificate() {
	X509 *cert = SSL_get_peer_certificate(m_ssl); /* get the server's certificate */
    if (cert != nullptr) {
    	std::cerr << "Server certificates:" << std::endl;

        char *line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        std::cerr << "Subject: " << line << std::endl;
        delete line;

        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        std::cerr << "Issuer: " << line << std::endl;
        delete line;
        
        X509_free(cert);
    }
    else
    	std::cerr << "[WARNING]: No client certificates configured." << std::endl;
}

int PapayitaWC::get_error() {
	return m_error;
}

void PapayitaWC::BuildHeader(const char *resource, char *content, int content_length, char method) {
	http_it = http;

	/* setting the get method */
	if (method == GET_METHOD) {
		memcpy(http_it, "GET ", 4);
		http_it += 4;
	} else if (method == POST_METHOD) {
		memcpy(http_it, "POST ", 5);
		http_it += 5;
	}

	/* setting the resource */
	http_it += sprintf(http_it, "%s HTTP/1.1\r\n", resource);

	/* setting header field Host: */
	http_it += sprintf(http_it, "Host: %s\r\n", m_hostname);

	/* Setting user-agent */
	memcpy(http_it, USER_AGENT_HEADER, USER_AGENT_HEADER_SIZE);
	http_it += USER_AGENT_HEADER_SIZE;

	/* Setting accept */
	memcpy(http_it, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n", 84);
	http_it += 84;

	/* setting Accept-language */
	memcpy(http_it, "Accept-Language: en-US,en;q=0.5\r\n", 33);
	http_it += 33;

	/* setting enconding, IMPORTATNT by default
	 it only accepts deflate ... yet*/
	memcpy(http_it, "Accept-Encoding: deflate\r\n", 26);
	http_it += 26;

	/* Setting Upgrade-Insecure-Requests NOT SUPPORTED */
	//memcpy(http_it, "Upgrade-Insecure-Requests: 1\r\n", 30);
	//http_it += 26;

	/* Setting Sec-Fetch-X*/
	memcpy(http_it, "Sec-Fetch-Dest: document\r\n\
Sec-Fetch-Mode: navigate\r\n\
Sec-Fetch-Site: none\r\n\
Sec-Fetch-User: ?1\r\n", 94);
	http_it += 94;

	/* the xtra header is add */
	for (int i = 0; i < xtraheaders_i; ++i)
		http_it += sprintf(http_it, "%s\r\n", m_xtraheaders[i]);

	/* Set Content-Length */
	if (method == POST_METHOD)
		http_it += sprintf(http_it, "Content-Length: %d\r\n", content_length);

	/* Setting Cache-Control: no-cache*/
	memcpy(http_it, "Cache-Control: no-cache\r\n\r\n", 27);
	http_it += 27;

	/* 
		On post, here you can add the content, some say that 
		GET_METHOD cannot carry a content, and some say it means
		GET should not and that does not meant it cannot. You 
		can modify this as you wish. 
	*/
	if (method == GET_METHOD){
		*http_it = 0;
		return;
	}

	memcpy(http_it, content, content_length);
	http_it += content_length;
	*http_it = 0;

}

void PapayitaWC::Write() {
	/* SSL_WRITE */
	m_error = SSL_write(m_ssl, http, http_it - http);
	if (isShowRequest) {
		std::cout << "REQUESTING ON SERVER----" << std::endl;
		std::cout << http << std::endl;
		std::cout << "END REQUEST ------------" << std::endl << std::endl;
	}

    if (m_error < 1) {
        std::cerr << "[ERROR:] on SSL_write" << std::endl;

        /* On SSL error, it needs to follow a certain protocol */
        if (SSL_get_error(m_ssl, m_error) == 6)
            SSL_shutdown(m_ssl);

        SSL_free(m_ssl);
        close(m_fd);
        SSL_CTX_free(m_ctx);
        m_error = -6;
    }
}

char PapayitaWC::HeaderParser(char *response_i) {
	if (response_i[5] == '2') {
		m_error = -8;
		std::cerr << "[ERROR:] HTTP/2 No supported" << std::endl;
		return -1;
	}
	if (responseHeader.status == 0)
		sscanf(response_i + 9, "%hu", &responseHeader.status);

	for (int i = 0; i < m_error; i++) {
		/* Parse for "Transfer-Encoding:" */
		if (response_i[i] == 'T' || response_i[i] == 't') {
			if (response_i[i + 15] == 'n' &&
				response_i[i + 16] == 'g' && 
				response_i[i + 17] == ':') 
			{
				if (memcmp(response_i + i + 19, "chunked", 7) == 0)
					responseHeader.TransferEnconding = TRANSFER_ENCODING_CHUNCKED;
			}
		}	 
		/* Parse for "Content-Length:" */
		else if (response_i[i] == 'C' || response_i[i] == 'c') {
			if (response_i[i + 12] == 't' &&
				response_i[i + 13] == 'h' && 
				response_i[i + 14] == ':'){
					sscanf(response_i + i + 16, "%hd", &responseHeader.ContentLength);
			}
		}
		/* Get Set-cookie: */
		else if (response_i[i] == 'S' || response_i[i] == 's') {
			if (response_i[i + 8] == 'i' &&
				response_i[i + 9] == 'e' &&
				response_i[i + 10] == ':') {
				i += 12;
				responseHeader.Cookie = response_i + i;
				while (response_i[i] != '\r') i++;
				responseHeader.Cookie_size = response_i + i - responseHeader.Cookie;
			}
		}
		/* Headers ends here */
		else if (response_i[i] == 10 && 
			response_i[i + 2] == 10) {
				responseHeader.size += i + 2;
				return 0;
		}
	}
	responseHeader.size += m_error;
	return 1;
}

int PapayitaWC::Read(char *response, int response_length){
	char *response_i = response;
	struct pollfd m_pfd;
	char isHeader = 1;
	int timeout = -1;

	responseHeader.TransferEnconding = 0; 	/* No Transfer Enconde Field*/
	responseHeader.ContentLength = -1; 		/* No Content Length */
	responseHeader.status = 0;
	responseHeader.size = 0;
	responseHeader.chunkSize = 0;

	m_pfd.fd = m_fd;
	m_pfd.events = POLLIN;

	poll(&m_pfd, 1, timeout);

	while(m_pfd.revents == POLLIN) {	
		m_error = SSL_read(m_ssl, response_i, response_length);
		// response_i += m_error;
		// *response_i = 0;
		// response_i -= m_error;
		// std::cerr << response_i << std::endl;
		
		if(m_error < 1) {
			std::cerr << "[ERROR:] SSL_read " << m_error << ", SSL_get_error type: ";
			m_error = SSL_get_error(m_ssl, m_error);

			if (m_error == SSL_ERROR_NONE) {
				std::cerr << "No errors in fact." << std::endl;
				break;
			}

			else if (m_error == SSL_ERROR_ZERO_RETURN) 
				std::cerr << "Zero return." << std::endl;

			else if (m_error == SSL_ERROR_WANT_READ)
				std::cerr << "Want to read." << std::endl;

			else if (m_error == SSL_ERROR_WANT_WRITE)
				std::cerr << "Want to write." << std::endl;
			
			else if (m_error == SSL_ERROR_WANT_CONNECT)
				std::cerr << "Want to connect." << std::endl;

			else if (m_error == SSL_ERROR_WANT_ACCEPT)
				std::cerr << "Want to accept." << std::endl;

			else if (m_error == SSL_ERROR_WANT_X509_LOOKUP)
				std::cerr << "X509 Lookup." << std::endl;

			else if (m_error == SSL_ERROR_SYSCALL) {
				std::cerr << "SYSCALL ";
				if (ERR_get_error() == 0) {
					std::cerr << " EOF, no error." << std::endl;
					break;
				}
				std::cerr << "Something went wrong." << std::endl;
			}

			else if (m_error == SSL_ERROR_SSL)
				std::cerr << "SSL" << std::endl;
			return -7;
		}
		if (isHeader) {
			isHeader = HeaderParser(response_i);
			if (isHeader < 0) return -8;
		}

		response_i += m_error;
		response_length -= m_error;

		if (response_length == 0) 
			std::cerr << "[WARNING:] Buffer size might be too small." << std::endl;

		else if (response_length < 0) {
			std::cerr << "[WARNING:] Buffer size is too small." << std::endl;
			return -9;
		}
		/* 	
			TODO: Find a way to compute the timeout for the chunks
		*/
		if (responseHeader.TransferEnconding == TRANSFER_ENCODING_CHUNCKED) {
			timeout = 1000;
			if (isHeader == 0) {
				int tmpchunkSize; 
				do {				
					/* This where the chunk size is located */
					char *tmp = response + responseHeader.size + 1 + responseHeader.chunkSize;
					short chunkSize_size = 0;
					tmpchunkSize = -1; /* No chunk size was read */ 

					if (sscanf(tmp, "%X", &tmpchunkSize) != 1) std::cerr << "[WARNING:] Reading chunk size, chunkSize " <<  tmpchunkSize << std::endl;

					#ifdef DEBUG_CHUNK_SIZES
					std::cout << std::endl << "||>>>> CHUNK STARTS --------------- prevChunk: " << responseHeader.chunkSize;
					std::cout << ", currentChunk: " << tmpchunkSize << " -- " << tmp << std::endl;
					#endif

					if (tmpchunkSize == 0) break;
					/*
						TODO: store where the chunk starts, and the length of such chunk.
					*/
					/* This is where the chunk starts */
					while(*tmp != '\n') {
						tmp++;
						chunkSize_size++;
					}

					/* This is where the chunk should end */
					tmp += tmpchunkSize + 1;

					#ifdef DEBUG_CHUNK_SIZES
					std::cout << "||>>> previous chunk " << responseHeader.chunkSize << " -- " << std::endl;
					#endif 

					/* if the whole chunk is already decoded then 
					 the following condition should be correct */
					if (*tmp == '\r' && *(tmp + 1) == '\n') {
						responseHeader.chunkSize += tmpchunkSize + 2 + chunkSize_size;

						#ifdef DEBUG_CHUNK_SIZES
						std::cout << "||>>> next CHUNK SIZE " << responseHeader.chunkSize << std::endl;
						std::cout << "||>>> CHUNK ENDS -----------------" << std::endl;
						#endif
					} 
					
					/* Otherwise the chunk ism't fully decoded yet */
					else {
						#ifdef DEBUG_CHUNK_SIZES
						std::cout << "||>>> BREAKING INNER LOOP " << std::endl;
						#endif

						break;
					}

				} while (1);

				/* If the previous loop was broken by chunk size */
				if (tmpchunkSize == 0)
					break;
			}
		} else {
			timeout = 0;
		}
		poll(&m_pfd, 1, timeout); /* POLLIN is set until there's no data to be read on the socket */
	}
	#ifdef DEBUG_CHUNK_SIZES
	std::cout << "m_pfd.revents: " << m_pfd.revents << std::endl;
	#endif

	*response_i = 0;

	return response_i - response;
}

int PapayitaWC::get(const char *resource, char *response, int response_length) {	
	BuildHeader(resource, nullptr, 0, GET_METHOD);

	Write();
	if (m_error < 0) return m_error;

	m_error = Read(response, response_length);
	return m_error;
}

int PapayitaWC::post(const char *resource, 
	char *response_content, int content_length, int response_length)
{	

	BuildHeader(resource, response_content, content_length, POST_METHOD);
	
	Write();
	if (m_error < 0) return m_error;

	m_error = Read(response_content, response_length);
	return m_error;
}

void PapayitaWC::showRequest(){
	isShowRequest = 1;
}

void PapayitaWC::set_header(const char *headerfield) {
	/* TODO: 
	- block adding headers such as Sec-Fetch-Mode, Sec-Fetch-Site, Sec-Fetch-User
	*/
	m_xtraheaders[xtraheaders_i] = (char *)headerfield;
	xtraheaders_i++;
}
int PapayitaWC::Cookie(char **C) {
	*C = responseHeader.Cookie;
	return responseHeader.Cookie_size;
}


void PapayitaWC::show_requestHeaders() {
	std::cerr << "BEGIN BREQUEST ->>" << std::endl;
	std::cerr << http << std::endl;
	std::cerr << "END BREQUEST -<<" << std::endl;
}

void PapayitaWC::terminate_session() {
	m_error = SSL_shutdown(m_ssl);
	if (m_error < 0) 
		std::cerr << "[ERROR:] on SSL_shutdown" << std::endl;
	SSL_free(m_ssl);
	close(m_fd);
	isSessionTerminated = 1;
}

PapayitaWC::~PapayitaWC() {
	/*m_error = SSL_shutdown(m_ssl);
	if (m_error < 0) 
		std::cerr << "[ERROR:] on SSL_shutdown" << std::endl;
	
	SSL_free(m_ssl);
	close(m_fd); */
	if (isSessionTerminated == 0)
		terminate_session();
	SSL_CTX_free(m_ctx);
}



/* Utility functions */
int WebClient_urlencode(char *buff, std::map<const char *, const char*> cntnt) {
	std::map<const char *, const char *>::iterator it = cntnt.begin();
	char *buff_it = buff;

	while(it != cntnt.end()){
		
		// copying key
		buff_it += sprintf(buff_it, "%s=", it->first);

		// encoding value
		while(*it->second != 0) {
			// std::cout << it->second << std::endl;
			if (*it->second == ' ') { /* space is enconde as '+'' in URI (after the?)*/
				*buff_it = '+';

			} else if (*it->second == 0x0A) { /* backslash */
				buff_it += sprintf(buff_it, "%%0%X", 10) - 1; 

			} else if ((*it->second > 0x20 && *it->second < 0x30) ||
				(*it->second > 0x39 && *it->second < 0x41) || 
				(*it->second > 0x5A && *it->second < 0x61) ||
				(*it->second > 0x7A)) {				
				buff_it += sprintf(buff_it, "%%%X", *it->second) - 1; 

			} else {
				*buff_it = *it->second;

			}

			it->second++;
			buff_it++;
		}

		*buff_it = '&';
		buff_it++;
		it++;
	}
	*(buff_it - 1) = '\0';

	return buff_it - buff - 1;
}

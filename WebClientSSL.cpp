#include "WebClientSSL.h"

#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <poll.h>

/* This client:
 - supports only HTTP/1.1
 - Supports only deflate encoding, futures versions will accept br
 - Does not support Upgrade-Insecure-Requests
 - Does not support Cache-Control
*/

WebClientSSL::WebClientSSL(const char *host) {

	/* Init SSL */
	xtraheaders_i = 0;

	m_hostname = (char *) host;

	OpenSSL_add_all_algorithms(); // load & register all cryptos
	SSL_load_error_strings();
	const SSL_METHOD *method = SSLv23_client_method(); /* Create new client-method instance */
	m_ctx = SSL_CTX_new(method);
    
	if (m_ctx != nullptr) {
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
    else {
    	std::cerr << "[ERROR:] SST_CTX_new returned nullptr." << std::endl;
		m_error = -1;
    }
}


int WebClientSSL::OpenConnection() {
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

	
void WebClientSSL::show_certificate() {
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

int WebClientSSL::get_error() {
	return m_error;
}

void WebClientSSL::BuildHeader(const char *resource, int resource_len, char *content, int content_length, char method) {
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
	memcpy(http_it, resource, resource_len);
	http_it += resource_len;

	/* setting HTTP/1.1\r\nHost: */
	memcpy(http_it, " HTTP/1.1\r\nHost: ", 17);
	http_it += 17;

	/* setting header field Host: */
	memcpy(http_it, m_hostname, strlen(m_hostname));
	http_it += strlen(m_hostname);

	/* Setting user-agent */
	memcpy(http_it, "\r\nUser-Agent: lightWebClient 1.1.0\r\n", 36);
	http_it += 36;

	/* Setting accept */
	memcpy(http_it, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n", 84);
	http_it += 84;

	/* setting Accept-language */
	memcpy(http_it, "Accept-Language: en-US,en;q=0.5\r\n", 33);
	http_it += 33;

	/* setting enconding, IMPORTATNT by default
	 it only accepts deflate */
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
	for (int i = 0; i < xtraheaders_i; ++i) {
		memcpy(http_it, m_xtraheaders[i].header, m_xtraheaders[i].length);
		http_it += m_xtraheaders[i].length;
	}

	/* Set Content-Length */
	if (method == POST_METHOD) {
		char cl[30];
		sprintf(cl, "Content-Length: %d\r\n", content_length);
		memcpy(http_it, cl, strlen(cl));
		http_it += strlen(cl);
	}

	/* Setting Cache-Control: no-cache*/
	memcpy(http_it, "Cache-Control: no-cache\r\n\r\n", 27);
	http_it += 27;

	/* On post, here you can add the content */
	if (method == GET_METHOD) return;

	memcpy(http_it, content, content_length);
	http_it += content_length;
}

void WebClientSSL::Write() {
	/* SSL_WRITE */
	m_error = SSL_write(m_ssl, http, http_it - http);

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

void WebClientSSL::Read(char *response, int response_length){
	char *response_i = response;
	struct pollfd m_pfd;
	m_pfd.fd = m_fd;
	m_pfd.events = POLLIN;

	poll(&m_pfd, 1, -1);

	while(m_pfd.revents == POLLIN) {	
		m_error = SSL_read(m_ssl, response_i, response_length);
		
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
    		m_error = -7;
	    	return;
	    }
	    response_i += m_error;
	    response_length -= m_error;

	    if (response_length == 0) 
	    	std::cerr << "[WARNING:] Buffer size might be too small" << std::endl;

	    else if (response_length < 0) {
	    	std::cerr << "[WARNING:] Buffer size is too small" << std::endl;
	    	m_error = -8;
	    	return;
	    }
	    /* TODO: For chunk Enconding Transfer, the timeout on poll needs
	       to be change to a value that the network can handle. This requires 
	       to parse the header to undestand the arrival content */
    	poll(&m_pfd, 1, 10); /* POLLIN is set until there's no data to be read on the socket */
    }
	std::cerr <<  response << std::endl;	
}

int WebClientSSL::get(const char *resource, int resource_len, char *response, int response_length) {	
	BuildHeader(resource, resource_len, nullptr, 0, GET_METHOD);

	std::cerr << http << std::endl;

	Write();
	if (m_error < 0) return m_error;

	Read(response, response_length);
	if (m_error < 0) return m_error;
	
	return 1;
}

int WebClientSSL::post(const char *resource, int resource_len, char *response_content, int content_length, int response_length) {	
	BuildHeader(resource, resource_len, response_content, content_length, POST_METHOD);
	std::cout << http << std::endl;
	
	Write();
	if (m_error < 0) return m_error;

	Read(response_content, response_length);
	if (m_error < 0) return m_error;
	
	return 1;
}

void WebClientSSL::set_header(const char *headerfield, int length) {
	/* TODO: 
	- block adding headers such as Sec-Fetch-Mode, Sec-Fetch-Site, Sec-Fetch-User
	*/
	std::cout << "set header" << std::endl;
	m_xtraheaders[xtraheaders_i].header = (char *)headerfield;
	m_xtraheaders[xtraheaders_i].length = length;
	std::cout << m_xtraheaders[xtraheaders_i].header << std::endl;
	xtraheaders_i++;
}

WebClientSSL::~WebClientSSL() {
	m_error = SSL_shutdown(m_ssl);
    if (m_error < 0) std::cerr << "[ERROR:] on SSL_shutdown" << std::endl;
	SSL_free(m_ssl);
	close(m_fd);
	SSL_CTX_free(m_ctx);
}


/* Utility functions */
int WebClient_urlencode(char *buff, std::map<const char *, const char*> cntnt) {
	std::map<const char *, const char *>::iterator it = cntnt.begin();

	while(it != cntnt.end()){
		std::cout << "Key: " << it->first << ", value: " << it->second << std::endl;
		++it;
	}
	return 1;
}

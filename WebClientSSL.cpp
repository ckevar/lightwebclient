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
		else
			m_error = -2;

    } 
    else
		m_error = -1;
}


int WebClientSSL::OpenConnection() {
	struct hostent *host;
	
	if((host = gethostbyname(m_hostname)) == nullptr)
		return -3;
	
	struct addrinfo hints = {0}, *addrs;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if(getaddrinfo(m_hostname, HTTPS_PORT, &hints, &addrs) != 0)
		return -4;

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

	if(m_fd == -1)
		return -5;

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
    	std::cerr << "Info: No client certificates configured." << std::endl;
}

int WebClientSSL::get_error() {
	return m_error;
}

void WebClientSSL::build_header(const char *resource, int resource_len, char *content, int content_length) {
	http_it = http;

	/* setting the get method */
	memcpy(http_it, "GET ", 4);
	http_it += 4;

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

	/* Setting Cache-Control: no-cache*/
	memcpy(http_it, "Cache-Control: no-cache\r\n\r\n", 27);
	http_it += 27;

	/* On post, here you can add the content */
	if (content_length == 0) return;
}

int WebClientSSL::get(const char *resource, int resource_len, char *response, int response_length) {	
	build_header(resource, resource_len, nullptr, 0);

	/* SSL_WRITE */
	m_error = SSL_write(m_ssl, http, http_it - http);
    if (m_error < 1) {
        std::cerr << "[ERROR:] on SSL_write" << std::endl;
        if (SSL_get_error(m_ssl, m_error) == 6)
            SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
        close(m_fd);
        SSL_CTX_free(m_ctx);
        return -6;
    }


    /* SSL_READ */
    char *response_i = response;
    struct pollfd m_pfd;
    m_pfd.fd = m_fd;
    m_pfd.events = POLLIN;
    poll(&m_pfd, 1, -1);

    while(m_pfd.revents == POLLIN) {	
	    m_error = SSL_read(m_ssl, response_i, response_length);
		
		if(m_error < 1) {
	    	std::cerr << "[ERROR:] on SSL_read " << m_error << std::endl;
	    	m_error = SSL_get_error(m_ssl, m_error);
    		std::cerr << "[ERROR:] " << m_error << std::endl;

    		if (m_error == SSL_ERROR_NONE) {
    			std::cerr << "No errors " << std::endl;
    			break;
    		}

    		else if (m_error == SSL_ERROR_ZERO_RETURN) 
    			std::cerr << "Zero return" << std::endl;

    		else if (m_error == SSL_ERROR_WANT_READ)
    			std::cerr << "Want to read " << std::endl;

    		else if (m_error == SSL_ERROR_WANT_WRITE)
    			std::cerr << "Want to write " << std::endl;
    		
    		else if (m_error == SSL_ERROR_WANT_CONNECT)
    			std::cerr << "want to connect" << std::endl;

    		else if (m_error == SSL_ERROR_WANT_ACCEPT)
    			std::cerr << "Want to accept" << std::endl;

    		else if (m_error == SSL_ERROR_WANT_X509_LOOKUP)
    			std::cerr << "X509 ERROR" << std::endl;

    		else if (m_error == SSL_ERROR_SYSCALL) {
    			std::cerr << "SYSCALL ERROR ";
    			if (ERR_get_error() == 0) break;
    			std::cerr << "something else happened " << std::endl;
    		}

    		else if (m_error == SSL_ERROR_SSL)
    			std::cerr << "SSL ERROR" << std::endl;
	    	return -7;
	    }
	    response_i += m_error;
	    response_length -= m_error;
		std::cerr << " left space " <<  response_length << std::endl;

	    if (response_length == 0) 
	    	std::cerr << "[WARNING:] buffer size might be too small" << std::endl;

	    else if (response_length < 0) {
	    	std::cerr << "[WARNING:] buffer size too small" << std::endl;
	    	return -8;
	    }
    	poll(&m_pfd, 1, 0); /* POLLIN is set until there's no data to be read on the socket */
    }
	std::cerr <<  response << std::endl;
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
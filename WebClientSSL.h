#ifndef WEB_CLIENT_SSL
#define WEB_CLIENT_SSL 

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <map>

#define HTTP_LENGTH 4096 /* min 400, just for header */
#define MAX_ADDITIONAL_HEADERS 10
#define HTTPS_PORT 	"443"

#define GET_METHOD 	1
#define POST_METHOD 2

typedef struct xtraheaders_t
{
	char *header;
	int length;
} xtraheaders_t;


class WebClientSSL
{
public:
	WebClientSSL(const char *host);

	/* Set a Header field, i.e.: "Cookie: <somecookie>"*/
	void set_header(const char *header_field, int length);

	/* std::couts the server certificate */
	void show_certificate();

	/* Executes requests get */
	int get(const char *resource, int length, char *buff, int buff_size); 
	int post(const char *resource, int length, char *buff, int buff_in, int buff_out); 

	/* gets error */
	int get_error();
	~WebClientSSL();
private:
	char http[HTTP_LENGTH];
	char *http_it;
	xtraheaders_t m_xtraheaders[MAX_ADDITIONAL_HEADERS];
	int xtraheaders_i;
	// char *xtraheaders[MAX_ADDITIONAL_HEADERS];
	// int xtraheaders_lengths[MAX_ADDITIONAL_HEADERS]
	int m_error;
	int m_fd;
	SSL_CTX *m_ctx;
	SSL *m_ssl;
	char *m_hostname;

	int OpenConnection();
	void BuildHeader(const char *resource, int length, char *buff, int buff_size, char method);
	void Read(char *response, int response_length);
	void Write();

};

/* Utility Functions */
int WebClient_urlencode(char *buff, std::map<const char*, const char *> cntnt);
#endif
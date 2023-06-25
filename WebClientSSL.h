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
#define TRANSFER_ENCODING_CHUNCKED 1

#define USER_AGENT_HEADER "User-Agent: lightWebClient 1.1.0\r\n"

typedef struct ResponseHeader
{
	char TransferEnconding;
	short ContentLength;
	unsigned short status;
} ResponseHeader;

class WebClientSSL
{
public:
	WebClientSSL(const char *host);

	/* Set a Header field, i.e.: "Cookie: <somecookie>"*/
	void set_header(const char *header_field);

	/* Get request header*/
	void show_request_headers();

	/* std::couts the server certificate */
	void show_certificate();

	/* Executes requests get */
	int get(const char *resource, char *buff, int buff_size); 
	int post(const char *resource, char *buff, int buff_in, int buff_out); 

	/* gets error */
	int get_error();
	~WebClientSSL();
private:
	int m_error;
	int m_fd;

	char http[HTTP_LENGTH];
	char *http_it;

	char *m_hostname;
	char *m_xtraheaders[MAX_ADDITIONAL_HEADERS];
	int xtraheaders_i;
	ResponseHeader responseHeader;

	SSL_CTX *m_ctx;
	SSL *m_ssl;

	int OpenConnection();
	void BuildHeader(const char *resource, char *buff, int buff_size, char method);
	char HeaderParser(char *response_i);
	void Read(char *response, int response_length);
	void Write();

};

/* Utility Functions */
int WebClient_urlencode(char *buff, std::map<const char*, const char *> cntnt);
#endif
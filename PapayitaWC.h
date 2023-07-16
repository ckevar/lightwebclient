#ifndef WEB_LIGHT_CLIENT_SSL
#define WEB_LIGHT_CLIENT_SSL 

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <map>

#define HTTP_LENGTH 4096 /* min 400, just for header */
#define MAX_ADDITIONAL_HEADERS 10
#define HTTPS_PORT 	"443"

#define GET_METHOD 	1
#define POST_METHOD 2
#define TRANSFER_ENCODING_CHUNCKED 1

#define USER_AGENT_HEADER "User-Agent: Papayita 1.1.0\r\n"

/***** BEGIN CUSTOM TYPES *****/
struct PA2YITA_BUFF_struct
{
	char *head; 		/* Head of the buffer */
	char *tail;			/* Current reading of the buffer a.k.a. iterator */
	int capacity;		/* Buffer capacity */
	int size;			/* Current size of the buffer */
};

typedef PA2YITA_BUFF_struct papayita_buffer;

typedef struct ResponseHeader
{
	char TransferEnconding;
	int chunkSize;
	short ContentLength;
	char *Cookie;
	int Cookie_size;
	unsigned short status;
	int size;
} ResponseHeader;
/***** END CUSTOM TYPES *****/


class PapayitaWC
{
public:
	PapayitaWC(const char *host);
	PapayitaWC();

	/* Set a Header field, i.e.: "Cookie: <somecookie>"*/
	void set_header(const char *header_field);
	int Cookie(char **Cookie);
	/* Get request header*/
	void show_requestHeaders();

	/* std::couts the server certificate */
	void show_certificate();

	/* Executes requests get */
	int get(const char *resource, char *buff, int buff_size);
	int post(const char *resource, char *buff, int buff_in, int buff_out);
	void showRequest();

	int responseHeader_size() {
		return responseHeader.size;
	}

	/* gets error */
	int get_error();
	void new_session(const char *host);
	void terminate_session();
	~PapayitaWC();
private:
	/* Class error */
	int m_error;

	/* filde descripto of the socket */
	int m_fd;

	/* Content to be transmited to the server */
	char http[HTTP_LENGTH];
	char *http_it;

	/* to show what's being transmitted 
	to the server */
	char isShowRequest;

	char *m_hostname;

	/* Users headers for request, maximun of
	 10 headers */
	char *m_xtraheaders[MAX_ADDITIONAL_HEADERS];
	int xtraheaders_i;

	/* Response header */
	ResponseHeader responseHeader;

	/* SSL Handlers */
	SSL_CTX *m_ctx;
	SSL *m_ssl;
	char isSessionTerminated;

	/* */
	int OpenConnection();
	void BuildHeader(const char *resource, char *buff, int buff_size, char method);
	char HeaderParser(char *response_i);
	int Read(char *response, int response_length);
	void Write();

	void preConstructor();

};

/* Utility Functions */
int WebClient_urlencode(char *buff, std::map<const char*, const char *> cntnt);



#endif
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
#define USER_AGENT_HEADER_SIZE 28

#define MAX_NUMBER_OF_SET_COOKIE 5

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
	char *Cookie[MAX_NUMBER_OF_SET_COOKIE];
	int Cookie_size[MAX_NUMBER_OF_SET_COOKIE];
	int Cookie_i;
	unsigned short status;
	int size;
} ResponseHeader;
/***** END CUSTOM TYPES *****/


class PapayitaWC
{
public:
	/** Binds the host over TCP and SSL ***/
	PapayitaWC(const char *host);
	PapayitaWC();
	// PapayitaWC(papayita_buffer *buff); // This is a future
	// implementation where the object can be created using a buffer

	void new_session(const char *host);
	void terminate_session();
	/* gets general errors error */
	int get_error();
	/**************************************/
	
	/*** SSL Oriented ***/
	/* std::couts the server certificate */
	void show_certificate();
	/********************/

	/*** HTTP1.1 operations ***/
	/* Set a Header field, i.e.: "Cookie: <somecookie>"*/
	void set_header(const char *header_field);
	int Cookie(char **Cookie, int c_i);
	/* Get request header*/
	void show_requestHeaders();
	int responseHeader_size() {
		return responseHeader.size;
	}
	/*************************/

	/*** RESTful requests ***/
	int get(const char *resource, char *buff, int buff_size);
	int post(const char *resource, char *buff, int buff_in, int buff_out);
	void showRequest();
	/************************/

	~PapayitaWC();
private:

	/*** Class General Variables ***/
	/* Class error */
	int m_error;
	/* file descripto of the socket */
	int m_fd;
	/* to show what's being transmitted 
	to the server */
	char isShowRequest;
	char *m_hostname;
	/*******************************/

	/*** SSL Handlers ***/
	SSL_CTX *m_ctx;
	SSL *m_ssl;
	char isSessionTerminated;
	/********************/

	/*** HTTP variablese ***/
	/* Content to be transmited to the server */
	char http[HTTP_LENGTH];
	char *http_it;

	/* Users headers for request, maximun of
	 10 headers */
	char *m_xtraheaders[MAX_ADDITIONAL_HEADERS];
	int xtraheaders_i;
	/* Response header */
	ResponseHeader responseHeader;
	/***********************/


	/*** TCP and SSL Ops ***/
	int OpenConnection();
	/* Loads every SSL needs */
	void preConstructor();
	/* Read and write over TCP */
	int Read(char *response, int response_length);
	void Write();
	/***************/

	/*** HTTP1.1 ops ***/
	void BuildHeader(const char *resource, char *buff, int buff_size, char method);
	char HeaderParser(char *response_i);

};

/* Utility Functions */
int WebClient_urlencode(char *buff, std::map<const char*, const char *> cntnt);



#endif
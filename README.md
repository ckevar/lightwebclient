# lightwebclient
It's a small web client impleted in C/C++ that supports basic websites, meaning:
- Supports SSL, through OpenSSL, it automaticly browses on port 443. 
- Supports HTTP/1.1, doesnt't support HTTP/2 or HTTP/3.
- Supports deflated enconding, doesn't support gzip neither br.
- Supports get and post method only, check under folder examples.
- Supports x-www-form-urlenconded encoding by a function WebClient_urlencode through c++, see example/x-form-urlencode.cpp:
```c++
std::map<const char *, const char*>
```
- Supports partially "Transfer-Encoding: chunked"
- Supports getting Cooking from header and the coockie size (amount of bytes the cookie has): COOCKIE_LEN = webClientSSL.Cookie(&A_CHAR_POINTER_VARIABLE).
- Supports cookie setting by set_header("Cookie: COOKIE_CONTENT").
- Supports session, meaning, after the object webclient is created, it's linked to the given host, the same object can be linked later to another host by terminating the previous session with WebClientSSL.terminate_session(). This doesn't not free the CTX created by OpenSSL at object creation, it only frees the SSL and closes the TCP connection. The new session where the previous CTX is linked to another host can be started by WebClientSSL.new_session("ANOTHER.HOST.COM").
- Doesn't support 301, forwarding
- Doesn't no support "Content-Length:" on response headers, so, the buff size has to be determined by trial-and-error.
- Doesn't use malloc either.
- Doesn't support header response handling.
- Doesn't support application/json

The idea of the project is to be as light as possible.

## Getting Started
#### Compile
```bash
make main
```
#### Run

```bash
./main <domain>
```
## Examples
- *getExample.cpp* after compiling it, you can test it with www.google.com as you may have read, certain web sites such as google have a chuck type transfer encoding, something this lightWebClient isn't yet capable of handle, so that's why if you know (it comes in the response header of the site you're browsing) you can modify the poll line

- *post.cpp* this performs a post on api.telegram.org with the query on the URL

- *post_x-form.cpp* Performs a post on api.telegram.org with the query on the content, that's why the header field "Content-Type: application/x-www-form-urlencoded\r\n" is addded.

- *x-form-urlenconde.cpp* Performs a post using a WebClient_urlencode to enconde query-like requestes. The enconder was not fully tested. The enconders is based on std::map <const char*, const char *>

- *tgbotsendmessage.cpp* Sends a message to your telegram API. PAY ATTENTION, this DOES NOT LISTEN arriving messages, if you have the chat_id you can send messages.

- *mongodb_atlas.cpp* An example how to findOne value on MongoDB cloud through ATLAS API. You can execute this with the "data.mongodb-api.com" domain.


## Buffer Size recommended
|Web|Buffer Size (char)|REST Method|
|-|-|-|
|www.google.com|20000|get|
|api.telegram.org/bot<BOTTOKEN>/getMe|700|get

## Reqs
- openssl (libssl-dev) libssl1.1
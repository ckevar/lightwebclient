# lightwebclient
It's a small web client impleted in C/C++ that supports basic websites, meaning:
- Supports SSL, through OpenSSL, it automaticly browses on port 443. 
- Supports HTTP/1.1, doesnt't support HTTP/2 or HTTP/3.
- Supports deflated enconding, doesn't support gzip neither br.
- Supports get and post method only
- Doesn't support 301, forwarding
- Doesn't support "Transfer-Encoding: chunked"
- Doesn't no support "Content-Length" on response headers, so, the buff size has to be determined by trial-and-error.
- Doesn't use malloc either.
- Doesn't support header response handling.
- Doesn't support automatic x-www-form-urlenconded yet, WORKING ON IT

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

## Buffer Size recommended
|Web|Buffer Size (char)|REST Method|
|-|-|-|
|www.google.com|20000|get|
|api.telegram.org/bot<BOTTOKEN>/getMe|700|get

## Reqs
- openssl (libssl-dev) libssl1.1
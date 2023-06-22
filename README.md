# lightwebclient
It's a small web client impleted in C/C++ that supports basic websites, meaning: 
- Supports HTTP/1.1, doesnt't support HTTP/2 or HTTP/3.
- Supports deflated enconding, doesn't support gzip neither br.
- Doesn't support 301, forwarding
- Doesn't support "Transfer-Encoding: chunked"
- Headers might be twested
- Doesn't no support "Content-Length", so, the buff size has to be determined by trial-and-error.
- It doesn't not use malloc either.
- Only supports *get* method, as follows: get(resource, resource_len, arriving_buffer, arriving_buffer_length)
- *post* method - WORKING ON IT

## Getting Started
#### Compile
```bash
make main
```
#### Run

```bash
./main www.google.com
```


## Reqs
- openssl (libssl-dev) libssl1.1
# lightwebclient
It's a small web client impleted in C/C++ that supports basic websites, meaning: 
- Supports HTTP/1.1, doesnt't support HTTP/2 or HTTP/3.
- Supports deflated enconding, doesn't support gzip neither br.
- Doesn't support 301, forwarding
- Doesn't support "Transfer-Encoding: chunked"
- Headers might be twested
- Doesn't no support "Content-Length", so, the buff size has to be determined by trial-and-error.

## Getting Started
#### Compile
```bash
make main
```
#### Run

```bash
./main
```


## Reqs
- openssl (libssl-dev) libssl1.1
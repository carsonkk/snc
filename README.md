# snc
> A (simplified) implementation of netcat

## About

This program is an attempt at a faithful, albeit briefer, implementation of the networking utility `netcat`. It provides a subset of the flags and features available in the real netcat, outlined further in `doc/`, and relies on 0 dependencies outside of "standard" libraries (where the word standard is used pretty loosely since it tends to mean different things to different people in the C community...). To learn more on netcat in general, check out the [man page](https://man.openbsd.org/nc), the [Wikipedia page](https://en.wikipedia.org/wiki/Netcat), or pretty much [any article online](https://www.google.com/search?q=netcat).

## Getting Started

#### Linux (Ubuntu)

1. Clone the repo and build the project
```bash
git clone git@github.com:carsonkk/snc.git
cd snc/
make
```
2. To test by example, try out the `hello world` of netcat by getting a server and client communicating with each other locally:
```bash
# Start a TCP server listening on port 8080
./snc -l 8080

# In a different Terminal tab, start a TCP client and have it connect to port 8080 on localhost
./snc localhost 8080

# Try typing something into one instance and make sure it "cat's" it to the other
```
3. Checkout `doc/` for exactly which options are supported or `example/` for various useful netcat command combinations that are supported

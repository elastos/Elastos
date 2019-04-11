# Overview

DNS is a address dispatch service of the ELA peer-to-peer network.

# Build

```shell
$ make dns
```

# Run DNS service

There are 3 optional parameters for the DNS service
1. Port, use `-p <port>` to specify a port for the DNS service, it will overwrite the port
 value in network parameters even the network has been specified by using `-net` parameter.
2. Net, use `-net <network>` to specify the network parameters for the DNS service,
 it can be `mainnet` `testnet` or `regnet`, the DNS service use `mainnet` parameters by default.
3. Debug, use `-debug` to enable debug mode, in debug mode, DNS service will print out debug logs.

Run DNS service with default network parameters.
```shell
$ ./ela-dns
```

Specify the serving port.
```shell
$ ./ela-dns -p 20866
```

Specify the network.
```shell
$ ./ela-dns -net testnet
```

Enable debug mode.
```shell
$ ./ela-dns -debug
```

Use all of them.
```shell
$ ./ela-dns -p 20866 -net regnet -debug
```
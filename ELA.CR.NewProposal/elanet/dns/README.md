# Overview

DNS is a address dispatch service of the ELA peer-to-peer network.

# Build

```shell
$ make dns
```

# Run DNS service

There are 3 optional parameters for the DNS service
1. Net, use `-net <network>` to specify the network parameters for the DNS service,
 it can be `mainnet` `testnet` or `regnet`, the DNS service use `mainnet` parameters by default.
2. Magic, use `-magic` to specify the magic number of the DNS service, it will overwrite the magic
 number in network parameters even the network has been specified by using `-net` parameter.
3. Port, use `-port` to specify a port for the DNS service, it will overwrite the port
 number in network parameters even the network has been specified by using `-net` parameter.
4. Debug, use `-debug` to enable debug mode, in debug mode, DNS service will print out debug logs.

Run DNS service with default `mainnet` network parameters.
```shell
$ ./ela-dns
```

Specify the network.
```shell
$ ./ela-dns -net testnet
```

Specify the magic number.
```shell
$ ./ela-dns -magic 123123
```

Specify the serving port.
```shell
$ ./ela-dns -port 12345
```

Enable debug mode.
```shell
$ ./ela-dns -debug
```

Use all of them.
```shell
$ ./ela-dns -net regnet -magic 123123 -port 12345  -debug
```
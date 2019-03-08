# HOW-TO-USE
## Usages

The **elapfd** must be run with configuration file of which you can specifiy it with option **-c**:

```shell
$./elapfd -c YOUR-CONFIG-PATH/elapfd.conf
```

 or run without options:

```shell
$ ./elapfd
```

 In this case, the upper command would be internally configured with the following paths in order of search priority:

```markdown
./elapfd.conf
../etc/carrier/elapfd.conf
/usr/local/etc/carrier/elapfd.conf
/etc/carrier/elapfd.conf
```

Beware that the last two candidate paths would be neglected on Windows.

Notice that the environment value **DYLD_LIBRARY_PATH** should be explicitly set to the path of dynamic libraries when running on MacOS:

```shell
$DYLD_LIBRARY_PATH=../lib ./elapfd [-c YOUR-CONFIG-FILE]
```

On windows, run the following command:

```shell
$elapfd.exe [-c YOUR-CONFIG-FILE]
```

To use port forwarding, two instances of elapfd must be started: One acts as the server while the other acts as the client.

Though the 'services' section in the server's configuration file should list some services which may be available to the client, the server can also choose to provide which service to the client using the 'users' section.

In the client's configuration file, a server ID and a server address must be supplied. What's more, a local port which will associate with the service in the server side must be given in the 'services' section.

# DID Wallet Service

This is the wallet service for the DID Sidechain, it's very similar to the mainchain wallet service.

?> You can find it at [https://github.com/elastos/Elastos.ORG.DID.Service](https://github.com/elastos/Elastos.ORG.DID.Service)

### Configuration

You can find the configuration file at `did.api/src/main/resources/application.properties`

##### Some Notes:

- You must use a normal address for the `didAddress` starting in 'E' or '8', not an 'X' address like the wallet service.

### Setup

1. Update `application.properties`
    - Set the `node.prefix` to your DID node's `HttpRestPort`
    - Set the `did.address` to a new wallet address, but you need ELA in it


2. Pick an open port for the service, by default it uses 8091 which may conflict with other programs.
You can check what's listening on your port via `sudo lsof -i tcp:8090` (OSX/Ubuntu), then changing it till it don't conflict with anything.
In my case I am using port `8092` because I moved my mainchain wallet service to 8091.

3. Build project `mvn install -Dmaven.test.skip -Dgpg.skip`.

4. Start it `java -jar base.api/target/base.api-0.0.6.jar`

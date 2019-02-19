
# Wallet Service

This is the basic interface to access wallet features

?> You can find it at [https://github.com/elastos/Elastos.ORG.Wallet.Service](https://github.com/elastos/Elastos.ORG.Wallet.Service)

##### Some Notes:

- ignore the Python files, those are used to serve the docs which you can find at [https://walletservice.readthedocs.io/en/latest/api_guide.html#create-a-ela-wallet](https://walletservice.readthedocs.io/en/latest/api_guide.html#create-a-ela-wallet).
However be warned the version online may be different.


### Requirements

1. Java - this is a Spring Framework Application
2. MySQL - minimum version is 5.7
3. Maven - you can install this with Brew (OSX)


### Configuration

You can find the configuration file at `base.api/src/main/resources/application.properties`

### Setup

1. Update `application.properties`
    - Set the node.prefix to your node's `HttpRestPort`
    - Set the MySQL server credentials and info
    - the `did.mainChainAddress` is the genesis recharge address retrieved from passing the genesis hash from calling
    `/api/v1/block/hash/0` on the sidechain `HttpRestPort` into `ela-cli wallet -g <hash>`
    - Set the `server.port`, *see next step*

!> NOTE: if you change this anytime you need to rebuild it! (Step 4)

2. Pick an open port for the service, this is set to the `server.port` setting in `application.properties`.
By default it uses 8090 which may conflict with programs such as Webstorm Daemon.
You can check what's listening on your port via `sudo lsof -i tcp:8090` (OSX/Ubuntu), then changing it till it doesn't conflict with anything.
In my case I am using port `8091`.

3. Create a blank MySQL database specified by `application.properties` in the `spring.datasource.url` field, by default this is named `ws`

4. Build project `mvn install -Dmaven.test.skip -Dgpg.skip`.

5. Start it `java -jar base.api/target/base.api-0.0.6.jar`


### Make Sure It Worked!

If it's running you should see something like `org.elastos.Application: Started Application in 8.198 seconds (JVM running for 8.709)`

Now query your, genesis block wallet address, you should see 33 million ELA if you haven't touched it: `http://localhost:8091/api/1/balance/8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta`

### Errors

##### Tomcat fails to start, something is running on your port

You can check what's listening on your port via `sudo lsof -i tcp:8090` (OSX/Ubuntu), then changing it till it doesn't conflict with anything.

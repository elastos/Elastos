# Demo deployment

## Requirements

1. nodejs >= 10.4.0 (10.13.0 LTS recommended)

2. web3.js >= 1.0.0 beta 

   ```shell
   ~$ npm install web3
   ```

3. Express.js >= 4.16.0 

   ```shell
   ~$ npm install express
   ```

4. pm2 >= 3.0.0

   ```shell
   ~$ npm install -g pm2
   ```

5. pkill command

## Instructions

1. deploy the three tar archive to three servers

   ```shell
   ~$ tar xzvf elastos1.tar.gz
   ```

   ```shell
   ~$ tar xzvf elastos2.tar.gz
   ```

   ```shell
   ~$ tar xzvf elastos3.tar.gz
   ```

2. Modify the "SeedList" settings in the **config.json** files under the ~/elastos*[n]*/ELA/ directories according to the IP addresses of the three servers

   ```shell
    "SeedList": [
      "127.0.0.1:7893",
      "127.0.0.1:8893"
    ],
   ```

3. Modify the --bootnodes parameter in the **start.sh** files under the ~/elastos*[n]*/Geth/ directories according to the IP address of the server hosting the elastos1 deployment

   ```shell
   --bootnodes "enode://0e96be781e9e2bc7a960e60bf597847f6aec476c2dcf4b14d7cc5f1722c5e70184b71c955c2fb4b263f3c02b7ebb1395346b828e3c0a25fcafc18e47a505f033@127.0.0.1:30301"
   ```

4. Start the ELA nodes 

   ```shell
   ~/elastos[n]/ELA$ ./start.sh
   ```

5. Start the bootnode on the server hosting the elastos1 deployment

   ```shell
   ~/elastos1/Geth$ ./bootstart.sh
   ```

6. Start the geth nodes and the oracle services

   ```shell
   ~/elastos[n]/Geth$ ./start.sh
   ```

7. Check the logs of the geth nodes and the oracle services under the ~/elastos*[n]*/Geth/Logs directories

8. Deploy the cross-chain payload processor smart contract to the geth sidechain with script on the server hosting the elastos1 deployment (**NOTE:** This should only be done once and before trying to access the oracle services for the first time. Repeated deployments of the contract won't do much harm but are useless)

   ```shell
   ~/elastos1/Geth$ ./deployctrt.sh
   ```

9. The oracle services can be accessed at their respective IP addresses and ports

   ```shell
   http://<elastos1 server ip>:16666/json_rpc/
   
   http://<elastos2 server ip>:17777/json_rpc/
   
   http://<elastos3 server ip>:18888/json_rpc/
   ```

10. When shutting down, first stop the geth nodes and oracle services, then the ELA nodes

    ```shell
    ~/elastos[n]/Geth$ ./stop.sh
    
    ~/elastos[n]/ELA$ ./stop.sh
    ```


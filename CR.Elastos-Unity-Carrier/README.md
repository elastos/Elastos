# React Native Elastos Carrier Plugin

Elastos Carrier is a decentralized peer-to-peer platform that takes over all network traffic between virtual machines and conveys information on applications’ behalf.

This repo is the react native bridge between native carrier lib and javascript api. so developer could easy to create a elastos dapp with carrier module by javascript.

The release version for plugin is [here](https://www.npmjs.com/package/react-native-elastos-carrier)

## Getting started
```
npm install --save react-native-elastos-carrier

react-native link react-native-elastos-carrier
```


## Next step
### iOS
* open xcode
* add ElastosCarrier.framework to **Linked Frameworks and Libraries** from **/node_modules/react-native-elastos-carrier/ios/Carrier** folder
* run project


### Android
add following lines to project build.gradle 
```
flatDir{
    dirs "$rootDir/../node_modules/react-native-elastos-carrier/android/libs"
}
```
under allprojects repositories block.

### Unit test with Jest
For plugin JS side, integrate with Jest to mock and test. 
```
npm test
```
!('./doc/jest.png')


## Usage
```
const carrier = new Carrier('app_name', {
    // carrier callback
});
await carrier.start();
```

## API List

```
  /*
   * @brief: get current carrier version
   * @return: (string) current verison
   */
  static getVersion(){
    return exec('getVersion');
  }

  /*
   * @brief: check given address is a valid carrier address or not
   * @return: (boolean) true or false.
   */
  static isValidAddress(address){
    return exec('isValidAddress', address);
  }

  /*
   * @brief: check given nodeId is a valid carrier node id or not
   * @return: (boolean) true or false.
   */
  static isValidId(nodeId){
    return exec('isValidId', nodeId);
  }

  /*
   * @brief: connect the local node to carrier network.
   * @return: ok
   * @error: if connect failure, throw an error here.
   */
  start(){
    return exec('createObject', this.config);
  }

  /*
   * @brief: get current node address
   * @return: (string) node address
   */
  getAddress(){
    return exec('getAddress', this.id);
  }

  /*
   * @brief: get current node id.
   * @return: (string) node id.
   */
  getNodeId(){
    return exec('getNodeId', this.id);
  }

  /*
   * @brief: get current node profile
   * @return: (json) node profile
   * @formatter: userId, gender, region, phone, email, description, name
   */
  getSelfInfo(){
    return exec('getSelfInfo', this.id);
  }

  /*
   * @brief: set value to node profile
   * @param: (json)info
   * @formatter: userId, gender, region, phone, email, description, name
   * @return: ok
   * @error: throw an error if failure
   */
  setSelfInfo(info){
    const user_info = _.extend({
      name : '',
      description : '',
      email : '',
      phone : '',
      gender : '',
      region : ''
    }, info);
    return exec('setSelfInfo', this.id, user_info);
  }

  /*
   * @brief: set current node presence status
   * @param: (number)presence, 0:online, 1:away, 2:busy
   * @return: ok
   * @error: throw an error if failure
   */
  setSelfPresence(presence){
    return exec('setSelfPresence', this.id, presence);
  }

  /*
   * @brief: compare with a node address as friend
   * @param: (string) address, node address
   * @param: (string) msg, hello message
   * @return: ok
   */
  addFriend(address, msg){
    return exec('addFriend', this.id, address, msg);
  }

  /*
   * @brief: accept the friend compare request
   * @param: (string) friend userId
   * @return: ok
   */
  acceptFriend(userId){
    return exec('acceptFriend', this.id, userId);
  }

  /*
   * @brief: get friend info
   * @param: friend user id
   * @return: json (friendInfo)
   */
  getFriendInfo(friendId){
    return exec('getFriendInfo', this.id, friendId);
  }

  /*
   * @brief: send message to friend node
   * @param: friend userId
   * @param: friend message
   * @return: ok
   */
  sendMessage(friendId, msg){
    return exec('sendFriendMessageTo', this.id, friendId, msg);
  }

  /*
   * @brief: remove friend from friend list
   * @param: friend userId
   * @return: ok
   */
  removeFriend(friendId){
    return exec('removeFriend', this.id, friendId);
  }

  /*
   * @brief: set label for a friend
   * @param: friend userId
   * @param: friend label
   * @return: ok
   */
  setLabel(friendId, label){
    return exec('setLabel', this.id, friendId, label);
  }

  /*
   * @brief: get friends info list
   * @return: (json array) friends info
   */
  getFriendList(){
    return exec('getFriendList', this.id);
  }
  
  /*
   * @brief: close carrier
   * @return: ok
   */
  close(){
    return exec('close', this.id);
  }

  /*
   * @brief: clean carrier
   * @return: ok
   */
  clean(){
    return exec('clean', this.id);
  }

  /*
   * @brief: create carrier session
   * @param: friend userId
   * @param: (enum) STREAM_STATE
   * @param: (enum) STREAM_MODE
   * @return: ok
   */
  createSession(friendId, streamType, streamMode){
    return exec('createSession', this.id, friendId, streamType, streamMode);
  }

  /*
   * @brief: request to connect as session
   * @param: friend userId
   * @return: ok
   */
  sessionRequest(friendId){
    return exec('sessionRequest', this.id, friendId);
  }

  /*
   * @brief: reply the session request
   * @param: friend userId
   * @param: accept or refuse
   * @param: if refuse, here is the reason
   * @return: ok
   */
  sessionReplyRequest(friendId, status, reason){
    return exec('sessionReplyRequest', this.id, friendId, status, reason);
  }

  /*
   * @brief: send data to the session stream
   * @param: stream id or friend userId
   * @param: (string) data string.
   * @return: ok
   */
  writeStream(streamIdOrFriendId, data){
    return exec('writeStream', this.id, streamIdOrFriendId, data);
  }

  /*
   * @brief: remove the session stream for friend node
   * @param: friend userId
   * @return: ok
   */
  removeStream(friendId){
    return exec('removeStream', this.id, friendId);
  }

  /*
   * @brief: close the session for friend node
   * @param: friend userId
   * @return: ok
   */
  closeSession(friendId){
    return exec('closeSession', this.id, friendId);
  }

  /*
   * @brief: add session service to friend node
   * @param: friend userId
   * @param: service name
   * @param: target host name
   * @param: host port
   * @return: ok
   */
  addService(friendId, serviceName, host, port){
    return exec('addService', this.id, friendId, serviceName, host, port);
  }

  /*
   * @brief: remove session service
   * @param: friend userId
   * @param: service name 
   * @return: ok
   */
  removeService(friendId, serviceName){
    return exec('removeService', this.id, friendId, serviceName);
  }

  /*
   * @brief: connect the server with carrier port forwarding
   * @param: friend userId
   * @param: service name
   * @param: server host or ip address
   * @param: server port
   * @return: (Number) port forwarding id
   */
  openPortFowarding(friendId, serviceName, host, port){
    return exec('openPortFowarding', this.id, friendId, serviceName, host, port);
  }

  /*
   * @brief: close the server connection with carrier port forwaring
   * @param: friend userId
   * @param: port forwarding id
   * @return: ok
   */
  closePortForwarding(friendId, portForwardingId){
    return exec('closePortForwarding', this.id, friendId, portForwardingId);
  }

  /*
   * @brief: open the channel for friend node.
   * @param: friend userId
   * @param: (string)cookie string
   * @return: (Number)channel id
   */
  openChannel(friendId, cookie){
    return exec('openChannel', this.id, friendId, cookie);
  }

  /*
   * @brief: close the channel with channel id
   * @param: friend userId
   * @param: channel id
   * @return: ok
   */
  closeChannel(friendId, channelId){
    return exec('closeChannel', this.id, friendId, channelId);
  }

  /*
   * @brief: send data to the friend channel
   * @param: friend userId
   * @param: channel id
   * @param: data string
   * @return: (Number) data size
   */
  writeChannel(friendId, channelId, data){
    return exec('writeChannel', this.id, friendId, channelId, data)
  }

  /*
   * @brief: pend the channel with channel id
   * @param: friend userId
   * @param: channel id
   * @return: ok
   */
  pendChannel(friendId, channelId){
    return exec('pendChannel', this.id, friendId, channelId);
  }

  /*
   * @brief: resume the channel with channel id
   * @param: friend userId
   * @param: channel id
   * @return: ok
   */
  resumeChannel(friendId, channelId){
    return exec('resumeChannel', this.id, friendId, channelId);
  }
```

## Plugin configuration
```
    CONNECTION_STATUS : {
      'CONNECTED' : 0,
      'DISCONNECTED' : 1
    },
    PRESENCE_STATUS : {
      'ONLINE' : 0,
      'AWAY' : 1,
      'BUSY' : 2
    },
    STREAM_TYPE : {
      'AUDIO' : 0,
      'VIDEO' : 1,
      'TEXT' : 2,
      'APPLICATION' : 3,
      'MESSAGE' : 4
    },
    STREAM_STATE : {

      Initialized: 1,
      TransportReady: 2,
      Connecting: 3,
      Connected: 4,
      Deactivated: 5,
      Closed: 6,
      Error: 7
    },
    STREAM_MODE : {
      COMPRESS: 1,
      PLAIN: 2,
      RELIABLE: 4,
      MULTIPLEXING: 8,
      PORT_FORWARDING: 16
    }
```

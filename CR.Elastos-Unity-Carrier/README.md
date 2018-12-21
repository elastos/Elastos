# React Native Elastos Carrier Plugin

The purpose of this repository is to provide an npm package for [Elastos React Native Framework](https://github.com/cyber-republic/elastos-ReactNative-framework) Alpha Release.

Elastos Carrier is a decentralized peer-to-peer platform that takes over all network traffic between virtual machines and conveys information on applications’ behalf.

The release version for plugin is [here](https://www.npmjs.com/package/react-native-elastos-carrier)

## Primary Code Reviewer
[Jacky Li](https://github.com/liyangwood)

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

## Usage
```
const carrier = new Carrier('app_name', {
    // carrier callback
});
await carrier.start();
```

## API List

### static getVersion
* return current Carrier version
```
Carrier.getVersion();
```

### static isValidAddress(address)
* check address is valid for carrier node or not.
* return boolean.
```
Carrier.isValidAddress('Xm4WXfsSbAHhwiAe6tdo3oxhH93p8jb6Kata9ywyohkjssBMAH8n') // true
Carrier.isValidAddress('abcde') //false
```

### getAddress
* return current instance address
```
const address = await carrier.getAddress();
```

### getSelfInfo
* return current node profile info.
```
const selfInfo = await carrier.getSelfInfo();
/*
 userId: 'EzhQQz9X3nR6NYb2makCpmZjnuiFsD11CGuak9FnE8hf',
 gender: 'male',
 region: 'China',
 phone: '123456789',
 email: 'carrier@elastos.org',
 description: 'I am a node for Elastos carrier',
 name: 'new world',
 */
```

### setSelfInfo(info)
* set self profile info for current node.
```
await carrier.setSelfInfo({
    gender: 'male',
    region: 'China',
    phone: '123456789',
    email: 'carrier@elastos.org',
    description: 'I am a node for Elastos carrier',
    name: 'new world',
});
```

### setSelfPresence(presence)
* set self presence status
```
// presence status
{
    'ONLINE' : 0,
    'AWAY' : 1,
    'BUSY' : 2
}

await carrier.setSelfPresence(Carrier.config.PRESENCE_STATUS.AWAY);
```

### addFriend(address, msg)
* pair with new carrier node as friend.
* return boolean.
```
await carrier.addFriend('Dg3h2TecXGzBU5NruvdYaMJoCdxGc3etPmJ6GVynKpLUm1whnQyE', 'hello');
```

### acceptFriend(userId)
* accpet pair request from another carrier node.
* userId is friend node userId, not address.
* return boolean
```
this.carrier.acceptFriend('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV');
```

### getFriendInfo(userId)
* return friend node info
```
const friendInfo = await carrier.getFriendInfo('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV');

/*
 gender: 'male',
 region: 'China',
 phone: '123456789',
 email: 'carrier@elastos.org',
 description: 'I am a node for Elastos carrier',
 name: 'new world',
 label: 'friend',
 status: 0,
 presence: 0
 */
```

### removeFriend(userId)
* remove friend.
```
await carrier.removeFriend('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV');
```

### getFriendList
* return current node friends list<FriendInfo>
* return data structure is friend info.
```
const friendList = await carrier.getFriendList();
```

### setLabel(friendId, label)
* set friend label
```
await carrier.setLabel('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV', 'label')
```

### sendMessage(friendId, message)
* send a message to friend.
* return boolean
```
await carrier.sendMessage('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV', 'hello world');
```

### close
* close carrier node

### clean
* remove carrier node file from device. that means the node will be gone.

### createSession(friendId, streamType, streamMode)
```
await this.carrier.createSession(
    '4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV',
    Carrier.config.STREAM_TYPE.TEXT,
    Carrier.config.STREAM_MODE.RELIABLE
);
```

### sessionRequest(friendId)
* send session request to friendId
```
await this.carrier.sessionRequest('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV')
```

### sessionReplyRequest(friendId, status, reason)
* accept or refuse session request
* status=0 accept, status=1 refuse.
* if refuse. reason is avaiable.
```
await this.carrier.sessionReplyRequest('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV', 0, null);
```

### writeStream(streamIdOrFriendId, data)
* write data to stream
* streamId or friendId both available.
```
await this.carrier.writeStream('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV', 'stream data ...');
```

### removeStream(friendId)
* remove stream from session
```
await this.carrier.removeStream('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV');
```

### closeSession(friendId)
* close session
```
await this.carrier.closeSession('4ni3UKYY9xHDcodNaP1edAWDGuF5cmWTU8QWH4JnNfwV');
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

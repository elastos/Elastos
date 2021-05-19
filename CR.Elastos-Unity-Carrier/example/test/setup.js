jest.mock('react-native-elastos-carrier', ()=>{

  const _ = require('lodash');

  const C = {
    nodeId : '6XwWqntxZFwa6XmAtSmJLNZbrL9VwbsMr8GDMxKAUPmy',
    address : 'DAQb3hTPLiaeLjhLyHvHK4ebJ8PcAvJUewwtxCQmbgZLVaQdLkjc'
  };
  C.info = {
    name : 'jacky',
    usreId : C.nodeId,
    gender : 'male',
    region : 'China',
    phone : '123',
    email : 'jacky@elastos.org',
    description : ''
  };
  C.friendInfo = {
    name : 'Adem',
    usreId : C.nodeId,
    gender : 'male',
    region : 'US',
    phone : '123',
    email : 'adem@elastos.org',
    label : 'TL'
  };

  C.friendList = [
    C.friendInfo
  ];

  const Carrier = class {
    static isValidAddress(address){
      return address === C.address;
    }

    static getVersion(){
      return '5.1'
    }

    static isValidId(nodeId){
      return nodeId === C.nodeId;
    }

    start(){
      return 'ok'
    }

    getAddress(){
      return C.address;
    }

    getNodeId(){
      return C.nodeId;
    }

    getSelfInfo(){
      return C.info;
    }

    setSelfInfo(info){
      C.info = _.extend(C.info, info);
      return 'ok';
    }

    setSelfPresence(presence){
      return 'ok';
    }

    addFriend(address, msg){
      return 'ok';
    }

    acceptFriend(userId){
      return 'ok'
    }

    getFriendInfo(friendId){
      return C.friendInfo;
    }

    sendMessage(friendId, msg){
      return 'ok';
    }

    removeFriend(friendId){
      return 'ok';
    }

    setLabel(friendId, label){
      C.friendInfo.label = label;
      return 'ok';
    }

    getFriendList(){
      return C.friendList;
    }

    close(){
      return 'ok';
    }

    clean(){
      return 'ok'
    }

    createSession(friendId, streamType, streamMode){
      return 'ok'
    }

    sessionRequest(friendId){
      return 'ok'
    }

    sessionReplyRequest(friendId, status, reason){
      return 'ok'
    }

    writeStream(streamIdOrFriendId, data){
      return 'ok'
    }

    removeStream(friendId){
      return 'ok'
    }

    closeSession(friendId){
      return 'ok'
    }

    addService(friendId, serviceName, host, port){
      return 'ok'
    }

    removeService(friendId, serviceName){
      return 'ok'
    }

    openPortFowarding(friendId, serviceName, host, port){
      return 1
    }

    closePortForwarding(friendId, portForwardingId){
      if(portForwardingId === 1){
        return 'ok'
      }
      return false;
    }

    openChannel(friendId, cookie){
      return 2;
    }

    closeChannel(friendId, channelId){
      if(channelId === 2){
        return 'ok';
      }
      return false;
    }

    writeChannel(friendId, channelId, data){
      return data.length;
    }

    pendChannel(friendId, channelId){
      if(channelId === 2){
        return 'ok';
      }
      return false;
    }

    resumeChannel(friendId, channelId){
      if(channelId === 2){
        return 'ok';
      }
      return false;
    }


  };

  return {
    Carrier
  };
});
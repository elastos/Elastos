import {NativeModules, NativeEventEmitter} from 'react-native';
import _ from 'lodash';
import config from './config';

const NativeCarrier = NativeModules.CarrierPlugin;
const Listener = new NativeEventEmitter(NativeCarrier);

/*
 * This is Elastos Carrier plugin
 * 
 */



const exec = async (fnName, ...args)=>{
  return new Promise((resolve, reject)=>{
    NativeCarrier[fnName](...args, (err, rs)=>{
      console.log('exec ['+fnName+'] ===>', err, rs);
      if(err){
        reject(err);
      }
      else{
        resolve(rs);
      }
    });
  });
};

const Carrier = class {
  static config = config;
  static getVersion(){
    return exec('getVersion');
  }

  static isValidAddress(address){
    return exec('isValidAddress', address);
  }

  constructor(id, callbacks){
    this.id = id;

    this.config = {
      name : this.id,
      udp_enabled : true,
      bootstraps : config.bootstraps
    };

    this.buildCallbacks(callbacks);
  }

  buildCallbacks(callbacks){
    const def_fn = (name)=>{
      return (...args)=>{
        console.log(`callback [${name}] fired : `, args);
      }
    };
    _.each(_.concat(config.CARRIER_CB_NAMES, config.STREAM_CB_NAMES), (name)=>{
      const fn = callbacks[name] || def_fn(name);
      Listener.addListener(name, (data)=>{
        fn(...data);
      });
    });
  }

  start(){
    return exec('createObject', this.config);
  }

  getAddress(){
    return exec('getAddress', this.id);
  }

  getSelfInfo(){
    return exec('getSelfInfo', this.id);
  }
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
  setSelfPresence(presence){
    return exec('setSelfPresence', this.id, presence);
  }

  addFriend(address, msg){
    return exec('addFriend', this.id, address, msg);
  }
  acceptFriend(userId){
    return exec('acceptFriend', this.id, userId);
  }
  getFriendInfo(friendId){
    return exec('getFriendInfo', this.id, friendId);
  }
  sendMessage(friendId, msg){
    return exec('sendFriendMessageTo', this.id, friendId, msg);
  }
  removeFriend(friendId){
    return exec('removeFriend', this.id, friendId);
  }
  setLabel(friendId, label){
    return exec('setLabel', this.id, friendId, label);
  }
  getFriendList(){
    return exec('getFriendList', this.id);
  }
  
  close(){
    return exec('close', this.id);
  }
  clean(){
    return exec('clean', this.id);
  }

  createSession(friendId, streamType, streamMode){
    return exec('createSession', this.id, friendId, streamType, streamMode);
  }

  sessionRequest(friendId){
    return exec('sessionRequest', this.id, friendId);
  }

  sessionReplyRequest(friendId, status, reason){
    return exec('sessionReplyRequest', this.id, friendId, status, reason);
  }

  // addStreamWithType()

  writeStream(streamIdOrFriendId, data){
    return exec('writeStream', this.id, streamIdOrFriendId, data);
  }

  removeStream(friendId){
    return exec('removeStream', this.id, friendId);
  }

  closeSession(friendId){
    return exec('closeSession', this.id, friendId);
  }

  // TODO
  addService(){}
  removeService(){}
  openPortFowarding(){}
  closePortForwarding(){}
  

  

  

  test(){
    NativeCarrier.test();
  }
};


export default Carrier;
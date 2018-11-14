import {NativeModules, NativeEventEmitter} from 'react-native';
import _ from 'lodash';
import config from './config';

const NativeCarrier = NativeModules.CarrierPlugin;
const Listener = new NativeEventEmitter(NativeCarrier);

/*
 * This is Elastos Carrier plugin
 * 
 */

const STREAM_CB_NAMES = [
  "onStateChanged",
  "onStreamData",
  "onChannelOpen",
  "onChannelOpened",
  "onChannelClose",
  "onChannelData",
  "onChannelPending",
  "onChannelResume",
];

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
      udp_enabled : false,
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
    _.each(config.CARRIER_CB_NAMES, (name)=>{
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

  createSession(friendId){
    return exec('createSession', this.id, friendId);
  }

  

  

  

  test(){
    NativeCarrier.test();
  }
};


export default Carrier;
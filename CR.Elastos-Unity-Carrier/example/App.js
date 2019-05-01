import React, {Component} from 'react';

import {AppRegistry, StyleSheet, View, Image, ActionSheetIOS, NativeModules, AlertIOS, Platform} from 'react-native';
import {Root, Toast, Container, Header, Content, Footer, FooterTab, Button, Text, Row, Col } from 'native-base';

import {Carrier} from 'react-native-elastos-carrier';


let targetAddress = '';
let target = '';
if(Platform.OS === 'ios'){
  targetAddress = '7qMfNokEqAubtDiQ59i5iDyMfnnZ7V7AMGeVhwrC6cUHrWZYy2gd';
  target = '47LBjMwsybaJK551bvSW3eRLLJuBVM53k6TJdL3LAwAM';
}
if(Platform.OS === 'android'){
  targetAddress ='TNSXbdWyHg6dDDbABH2xuqNH81TEwCWwSaBA9gFXcnLMX9cmaj6K';
  target = 'CzrPpmBSutLGVH6VjtoqE6rhoy8jwn7tfADp2nnrArv8';
}

const testID = (name)=>{
  const rs = {};
  if(Platform.OS === 'android'){
    rs.accessibilityLabel = name;
  }
  else{
    rs.testID = name;
  }
  return rs;
}

class App extends Component{
  constructor(){
    super();
    this.state = {
      log : [],
      error : ''
    };

    this.carrier = null;


  }
  render() {
    return (
      <Root>
      <Container style={styles.container}>
        <Text style={styles.log} {...testID('log')}>{this.state.log.join('\n')}</Text>
        <Button style={styles.btn} {...testID('log:clear')} danger block onPress={this.testFn.bind(this, 'clearLog')}>
          <Text>clear log</Text>
        </Button>
        {/* <Text style={styles.error}>{this.state.error}</Text> */}
        

        <Content>
          <Row>
            <Col>
              <Button style={styles.btn} {...testID('btn:getVersion')} primary onPress={this.testFn.bind(this, 'getVersion')}>
                <Text>getVersion</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:isValidAddress')} success onPress={this.testFn.bind(this, 'isValidAddress')}>
                <Text>isValidAddress</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:isValidId')} primary onPress={this.testFn.bind(this, 'isValidId')}>
                <Text>isValidId</Text>
              </Button>
            </Col>
          </Row>

          <Row>
            <Col>
              <Button style={styles.btn} {...testID('btn:getAddress')} success onPress={this.testFn.bind(this, 'getAddress')}>
                <Text>getAddress</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:getNodeId')} primary onPress={this.testFn.bind(this, 'getNodeId')}>
                <Text>getNodeId</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:setSelfInfo')} success onPress={this.testFn.bind(this, 'setSelfInfo')}>
                <Text>setSelfInfo</Text>
              </Button>
            </Col>
          </Row>

          <Row>
            <Col>
              <Button style={styles.btn} {...testID('btn:getSelfInfo')} primary onPress={this.testFn.bind(this, 'getSelfInfo')}>
                <Text>getSelfInfo</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:addFriend')} success onPress={this.testFn.bind(this, 'addFriend')}>
                <Text>addFriend</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:acceptFriend')} primary onPress={this.testFn.bind(this, 'acceptFriend')}>
                <Text>acceptFriend</Text>
              </Button>
            </Col>
          </Row>

          <Row>
            <Col>
              <Button style={styles.btn} {...testID('btn:getFriendInfo')} success onPress={this.testFn.bind(this, 'getFriendInfo')}>
                <Text>getFriendInfo</Text>
              </Button>
            </Col>
              
            <Col>
              <Button style={styles.btn} {...testID('btn:sendMessage')} primary onPress={this.testFn.bind(this, 'sendMessage')}>
                <Text>sendMessage</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:createSession')} success onPress={this.testFn.bind(this, 'createSession')}>
                <Text>createSession</Text>
              </Button>
            </Col>
          </Row>

          <Row>
            {/* <Col>
              <Button style={styles.btn} {...testID('btn:sessionRequest')} primary onPress={this.testFn.bind(this, 'sessionRequest')}>
                <Text>sessionRequest</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:sessionReplyRequest')} success onPress={this.testFn.bind(this, 'sessionReplyRequest')}>
                <Text>sessionReplyRequest</Text>
              </Button>
            </Col> */}
            <Col>
              <Button style={styles.btn} {...testID('btn:writeStream')} primary onPress={this.testFn.bind(this, 'writeStream')}>
                <Text>writeStream</Text>
              </Button>
            </Col>
            <Col>
              <Button style={styles.btn} {...testID('btn:closeSession')} success block onPress={this.testFn.bind(this, 'closeSession')}>
                <Text>closeSession</Text>
              </Button>
            </Col>
          </Row>
          
          <Row>
            <Col>
              
            </Col>
            <Col></Col>
            <Col></Col>
          </Row>

        </Content>

      </Container>
      </Root>
    );
  }

  async openPrompt(title, message=''){
    return new Promise((resolve, reject)=>{
      AlertIOS.prompt(title, message, (value)=>{
        resolve(value);
      });
    });
  }

  async testFn(name){
    let rs = null;
    let tmp = '';
    switch(name){
      case 'getVersion':
        rs = await Carrier.getVersion();
        break;
      case 'isValidAddress':
        // tmp = await this.openPrompt('Enter an address');
        rs = await Carrier.isValidAddress(targetAddress);
        rs = targetAddress + ' is a valid address => '+rs.toString();
        break;
      case 'isValidId':
        rs = await Carrier.isValidId(target);
        rs = target + ' is a valid nodeId => '+rs.toString();
        break;
      case 'getAddress':
        rs = await this.carrier.getAddress();
        break;
      case 'getNodeId':
        rs = await this.carrier.getNodeId();
        break;
        
      case 'setSelfInfo':
        const info = {
          name : 'bbb',
          email : 'aaa@bbb.com',
          phone : '123456',
          description : 'bbbbb',
          region : 'cccc',
          gender : 'male'
        };
        rs = await this.carrier.setSelfInfo(info);
        break;
      case 'getSelfInfo':
        tmp = await this.carrier.getSelfInfo();
        rs = JSON.stringify(tmp);
        break;
      case 'addFriend':
        try{
          rs = await this.carrier.addFriend(target, 'hello');
          console.log(rs);
        }catch(e){
          this.setError(e);
        }
        break;
      case 'acceptFriend':
        try{
          rs = await this.carrier.acceptFriend(target);
        }catch(e){
          this.setError(e);
        }
        break;
      case 'getFriendInfo':
        try{
          tmp = await this.carrier.getFriendInfo(target);
          rs = JSON.stringify(tmp);
        }catch(e){
          this.setError(e);
        }
        break;
      case 'sendMessage':
        try{
          rs = await this.carrier.sendMessage(target, 'adsfsfdsf');
        }catch(e){
          this.setError(e);
        }
        
        break;
      case 'close':
        rs = await this.carrier.close();
        break;
      case 'createSession':
        try{
          rs = await this.carrier.createSession(
            target,
            Carrier.config.STREAM_TYPE.TEXT,
            Carrier.config.STREAM_MODE.RELIABLE
          );
        }catch(e){
          this.setError(e);
        }
        
        break;
      case 'sessionRequest':
        try{
          rs = await this.carrier.sessionRequest(target);
        }catch(e){
          this.setError(e);
        }
        break;
      case 'sessionReplyRequest':
        try{
          rs = await this.carrier.sessionReplyRequest(target, 0, null);
        }catch(e){
          this.setError(e);
        }
        break;
      case 'writeStream':
        try{
          // const buf = new Buffer('hello word')
          rs = await this.carrier.writeStream(target, 'sljfdlsjflsj');
        }catch(e){
          this.setError(e);
        }

        break;
      case 'closeSession':
        try{
          rs = await this.carrier.closeSession(target);
        }catch(e){
          this.setError(e);
        }
        break;
      case 'clearLog':
        rs = null;
        this.setState({log : ['']});
        break;
    }
    if(rs || _.isString(rs)){
      this.setLog(rs.toString());
    }
    
  }

  setLog(log){
    const mlog = this.state.log;
    mlog.unshift(log)
    this.setState({log : mlog});
  }
  setError(error){
    const mlog = this.state.log;
    mlog.unshift('[error]=>'+error);
    this.setState({log : mlog});
    Toast.show({
      text : error,
      type : 'danger'
    })
  }

  async componentDidMount(){
    this.setLog('Address : '+targetAddress);
    this.setLog('userid : '+target);

    this.carrier = new Carrier('carrier_demo', {
      onReady : ()=>{
        this.setLog('carrier is ready');
      },
      onConnection : (status)=>{
        this.setLog('carrier connection status : '+status);
      },
      onFriends : (list)=>{
        this.setLog('carrier connection status : '+JSON.stringify(list));
      },
      onFriendMessage : (data)=>{
        this.setLog('receive message from '+data.userId+' with ['+data.message+']');
      },
      onSessionRequest : (data)=>{
        this.setLog('carrier onSessionRequest '+JSON.stringify(data));
      },
      onStateChanged : (data)=>{
        this.setLog('carrier onStateChanged : '+JSON.stringify(data));
      },
      onStreamData : (data)=>{
        this.setLog('carrier onStreamData : '+JSON.stringify(data));

        
      }
    });
    const k = await this.carrier.start();
    this.setLog('start carrier => '+k);

    this.setLog('carrier init success');
  }
    
      
}

const styles = StyleSheet.create({
    container: {
      flex: 1,
      backgroundColor: '#f9f9f9',
      paddingLeft: 20,
      paddingRight: 20,
      paddingTop: 50
    },
    btn : {
      marginTop: 12
    },
    log : {
      backgroundColor: '#000',
      color: '#ff0',
      fontSize:14, 
      width:"100%",
      height : 300,
      overflow: 'scroll'
    },
    error : {
      marginTop: 10,
      backgroundColor: '#000',
      color: 'red',
      fontSize:14, 
      width:"100%"
    }
  });

export default App;
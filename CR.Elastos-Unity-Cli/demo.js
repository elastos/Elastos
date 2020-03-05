/* eslint-disable prettier/prettier */
/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 *
 * @format
 * @flow
 */

import React from 'react';
import {
  StyleSheet,
  ScrollView,
  View,
  Text,
  Button,
  Image,
} from 'react-native';

import RNElastosMainchain from 'react-native-elastos-unity-wallet';
import {Carrier} from 'react-native-elastos-unity-carrier';

let targetAddress = '';
let target = '';
if(Platform.OS === 'ios'){
  targetAddress = '7qMfNokEqAubtDiQ59i5iDyMfnnZ7V7AMGeVhwrC6cUHrWZYy2gd';
  target = '47LBjMwsybaJK551bvSW3eRLLJuBVM53k6TJdL3LAwAM';
}
if(Platform.OS === 'android'){
  targetAddress = 'TNSXbdWyHg6dDDbABH2xuqNH81TEwCWwSaBA9gFXcnLMX9cmaj6K';
  target = 'CzrPpmBSutLGVH6VjtoqE6rhoy8jwn7tfADp2nnrArv8';
}

class App extends React.Component {

  constructor(props) {
    super(props);
    this.state = {
      result: '',
      log : [],
      error : ''
    }
    this.carrier = null;
  }

  async componentDidMount() {
    console.log('Address : '+targetAddress);
    console.log('userid : '+target);

    this.carrier = new Carrier('carrier_demo', {
      onReady : ()=>{
        console.log('carrier is ready');
      },
      onConnection : (status)=>{
        console.log('carrier connection status : '+status);
      },
      onFriends : (list)=>{
        console.log('carrier connection status : '+JSON.stringify(list));
      },
      onFriendMessage : (data)=>{
        console.log('receive message from '+data.userId+' with ['+data.message+']');
      },
      onSessionRequest : (data)=>{
        console.log('carrier onSessionRequest '+JSON.stringify(data));
      },
      onStateChanged : (data)=>{
        console.log('carrier onStateChanged : '+JSON.stringify(data));
      },
      onStreamData : (data)=>{
        console.log('carrier onStreamData : '+JSON.stringify(data));

        
      }
    });
    const k = await this.carrier.start();
    console.log('start carrier => '+k);
    console.log('carrier init success');
  }

  GetBalance = () => {
    RNElastosMainchain.GetBalance(0, (err, res) => {
      this.setState({result: parseFloat(res / 100000000).toFixed(2)})
    });
  }

  CreateAddress = () => {
    RNElastosMainchain.CreateAddress( (err, res) => {
      this.setState({result: res})
    });
  }

  GetAllTransaction = () => {
    RNElastosMainchain.GetAllTransaction( (err, res) => {
      this.setState({result: res})
    });
  }

  sendClicked = () => {
    RNElastosMainchain.Send(this.state.sendingamount, this.state.toaddress, true, (err, res) => {
      this.setState({result: res})
    });
  }

  exportClicked = () => {
    RNElastosMainchain.ExportWalletWithMnemonic( "elastos2018", (err, res) => {
      this.setState({result: res})
    });
  }

  GetMultiSignPubKeyWithMnemonic = () => {
    RNElastosMainchain.GetMultiSignPubKeyWithMnemonic("cry mechanic bean they discover vendor couple adapt walk room edit dinner", (err, res) => {
      this.setState({result: res})
    });
  }

  GetMultiSignPubKeyWithPrivKey = () => {
    RNElastosMainchain.GetMultiSignPubKeyWithPrivKey("", (err, res) => {
      this.setState({result: res})
    });
  }

  GetPublicKey = () => {
    RNElastosMainchain.GetPublicKey((err, res) => {
      this.setState({result: res})
    });
  }

  IsAddressValid = () => {
    RNElastosMainchain.IsAddressValid("EWzDfqRmfYKHhg2V4gPGz3jQJkZU97grow", (err, res) => {
      if (res == true) {
        this.setState({result: "True"})
      } else {
        this.setState({result: "False"})
      }
    });
  }

  GetSupportedChains = () => {
    RNElastosMainchain.GetSupportedChains((err, res) => {
      this.setState({result: JSON.stringify(res)})
    });
  }

  ChangePassword = () => {
    RNElastosMainchain.ChangePassword("elastos2018","elastos2018", (err, res) => {
      this.setState({result: res})
    });
  }

  GetBalanceInfo = () => {
    RNElastosMainchain.GetBalanceInfo((err, res) => {
      this.setState({result: res})
    });
  }

  GetBalanceWithAddress = () => {
    RNElastosMainchain.GetBalanceWithAddress(this.state.toaddress, (err, res) => {
      this.setState({result: res})
    });
  }

  GetAllAddress = () => {
    RNElastosMainchain.GetAllAddress((err, res) => {
      this.setState({result: res})
    });
  }

  GenerateMnemonic = () => {
    RNElastosMainchain.GenerateMnemonic( (err, res) => {
      // this.setState({seedText: res})
      this.setState({result: res})
    });
  }

  CreateWallet = () => {
    RNElastosMainchain.CreateWallet("cry mechanic bean they discover vendor couple adapt walk room edit dinner" , (err, res) => {
      this.setState({result: res})
    });
  }

  ImportWallet = () => {
    RNElastosMainchain.ImportWalletWithMnemonic("cry mechanic bean they discover vendor couple adapt walk room edit dinner", (err, res) => {
      this.setState({result: res})
    })
  }

  async CarrierGetVersion() {
    result = await Carrier.getVersion();
    this.setState({result})
  }

  async CarrierIsValidAddress() {
    result = await Carrier.isValidAddress(targetAddress);
    this.setState({result: result.toString()})
  }

  async CarrierIsValidId() {
    result = await Carrier.isValidId(target);
    this.setState({result: result.toString()})
  }

  async CarrierGetAddress() {
    result = await this.carrier.getAddress();
    this.setState({result: result.toString()})
  }

  async CarrierGetNodeId() {
    result = await this.carrier.getNodeId();
    this.setState({result: result.toString()})
  }

  async CarrierSetSelfInfo() {
    const info = {
      name : 'bbb',
      email : 'aaa@bbb.com',
      phone : '123456',
      description : 'bbbbb',
      region : 'cccc',
      gender : 'male'
    };
    result = await this.carrier.setSelfInfo(info);
    this.setState({result})
  }

  async CarrierGetSelfInfo() {
    result = await this.carrier.getSelfInfo();
    this.setState({result: JSON.stringify(result)})
  }

  async CarrierAddFriend() {
    try{
      result = await this.carrier.addFriend(target, 'hello');
      this.setState({result})
    }catch(e){
      console.log(e);
    }
  }

  async CarrierAcceptFriend() {
    try{
      result = await this.carrier.acceptFriend(target);
      this.setState({result})
    }catch(e){
      console.log(e);
    }
  }

  async CarrierGetFriendInfo() {
    try{
      tmp = await this.carrier.getFriendInfo(target);
      result = JSON.stringify(tmp);
      this.setState({result})
    }catch(e){
      console.log(e);
    }
  }

  async CarrierSendMessage() {
    try{
      result = await this.carrier.sendMessage(target, 'adsfsfdsf');
      this.setState({result})
    }catch(e){
      console.log(e);
    }
  }

  async CarrierCépse() {
    result = await this.carrier.close();
    this.setState({result})
  }

  async CarrierCreateSession() {
    try{
      result = await this.carrier.createSession(
        target,
        Carrier.config.STREAM_TYPE.TEXT,
        Carrier.config.STREAM_MODE.RELIABLE
      );
      this.setState({result})
    }catch(e){
      console.log(e);
    }
  }

  async CarrierWriteStream() {
    try{
      result = await this.carrier.writeStream(target, 'sljfdlsjflsj');
      this.setState({result})
    }catch(e){
      console.log(e);
    }
  }

  async CarrierCloseSession() {
    try{
      result = await this.carrier.closeSession(target);
      this.setState({result})
    }catch(e){
      console.log(e);
    }
  }


  render() {

    return (
      <ScrollView style={styles.container}>
        <View style={{flex:1, paddingBottom: 100}}>

          <Image style={{alignSelf:'center', height:40, width: 250, resizeMode: 'stretch'}} source={require('./resources/UnityLogoGradient.png')} />
          <Text testID={'txt:Result'} style={styles.result}>{this.state.result}</Text>

            <Text style={styles.title}>Wallet</Text>

            <Button onPress={this.GenerateMnemonic} title="GenerateMnemonic"/>
            <Button onPress={this.CreateWallet} title="CreateWallet"/>
            <Button onPress={this.ImportWallet} title="ImportWallet"/>
            <Button onPress={this.CreateAddress} title="CreateAddress"/>
            <Button onPress={this.GetBalanceInfo} title="GetBalanceInfo"/>
            <Button onPress={this.exportClicked} title="ExportWallet"/>
            <Button onPress={this.ChangePassword} title="ChangePassword"/>
            <Button onPress={this.GetMultiSignPubKeyWithMnemonic} title="GetMultiSignPubKeyWithMnemonic (Android only)"/>
            <Button onPress={this.GetPublicKey} title="GetPublicKey"/>
            <Button onPress={this.GetSupportedChains} title="GetSupportedChains"/>
            <Button onPress={this.IsAddressValid} title="IsAddressValid"/>

            <Text style={styles.title}>Carrier</Text>

            <Button onPress={this.CarrierGetVersion.bind(this)} title="getVersion"/>
            <Button onPress={this.CarrierIsValidAddress.bind(this)} title="isValidAddress"/>
            <Button onPress={this.CarrierIsValidId.bind(this)} title="isValidId"/>
            <Button onPress={this.CarrierGetAddress.bind(this)} title="getAddress"/>
            <Button onPress={this.CarrierGetNodeId.bind(this)} title="getNodeId"/>
            <Button onPress={this.CarrierSetSelfInfo.bind(this)} title="setSelfInfo"/>
            <Button onPress={this.CarrierGetSelfInfo.bind(this)} title="getSelfInfo"/>
            <Button onPress={this.CarrierAddFriend.bind(this)} title="addFriend"/>
            <Button onPress={this.CarrierAcceptFriend.bind(this)} title="acceptFriend"/>
            <Button onPress={this.CarrierGetFriendInfo.bind(this)} title="getFriendInfo"/>
            <Button onPress={this.CarrierSendMessage.bind(this)} title="sendMessage"/>
            <Button onPress={this.CarrierCreateSession.bind(this)} title="createSession"/>
            <Button onPress={this.CarrierWriteStream.bind(this)} title="writeStream"/>
            <Button onPress={this.CarrierCloseSession.bind(this)} title="closeSession"/>
          </View>
      </ScrollView>
    );
  }
}

const styles = StyleSheet.create({
  title: {
    flex:1,
    height: 50,
    fontSize:30,
    backgroundColor:'red',
    color: 'white',
    textAlign:'center'
  },
  buttonView: {
    marginBottom:10,
    marginLeft:100,
    marginRight:100
  },
  container: {
    paddingBottom: -300,
    paddingTop: 50,
    flex: 1,
  },
  elaAmout : {
    marginBottom : 10,
    textAlign: 'center'
  },
  elaNewAddress : {
    marginBottom : 10,
    textAlign: 'center',
  },
  item : {
    flex : 1,
    flexDirection: 'column',
    marginBottom : 10
  },
  list: {
    flex: 1,
    marginLeft: 20,
    marginRight: 20,
    marginTop : 10
  },
  result : {
    marginBottom : 10,
    textAlign: 'center'
  },
  textbox: {
    borderBottomWidth: 1,
    borderColor: 'black',
    borderRadius: 5,
    height: 50,
    marginBottom: 5,
    marginLeft: 20,
    marginRight: 20,
    marginTop: 10,
    textAlign: 'left'
  },
});

export default App;

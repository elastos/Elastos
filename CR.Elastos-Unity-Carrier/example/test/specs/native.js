import 'babel-polyfill';
import _ from 'lodash';


let log;

const getBtnElement = async (name)=>{
  return await $(`~btn:${name}`);
}
const getLogElement = async ()=>{
  return await $('~log');
}
const print = (msg)=>{
  console.log(`[LOG] => ${msg}`);
};
const printLog = async ()=>{
  log = await getLogElement();
  const msg = await log.getText();
  print(msg);
}

describe('Elastos Carrier auto test with WebdriverIO and Appium', ()=>{
  beforeEach(async () => {
    
    const clear = await $('~log:clear');
    await clear.click();
  });

  

  it('getVersion', async () => {
    let btn = await getBtnElement('getVersion');
    await btn.click();
    await printLog();
  });

  it('isValidAddress', async()=>{
    let btn = await getBtnElement('isValidAddress');
    await btn.click();
    await printLog();
  });

  it('isValidId', async()=>{
    let btn = await getBtnElement('isValidId');
    await btn.click();
    await printLog();
  });

  it('getAddress', async()=>{
    let btn = await getBtnElement('getAddress');
    await btn.click();
    await printLog();
  });

  it('getNodeId', async()=>{
    let btn = await getBtnElement('getNodeId');
    await btn.click();
    await printLog();
  });

  it('setSelfInfo', async()=>{
    let btn = await getBtnElement('setSelfInfo');
    await btn.click();
    await printLog();
  });

  it('getSelfInfo', async()=>{
    let btn = await getBtnElement('getSelfInfo');
    await btn.click();
    await printLog();
  });

  it('addFriend', async()=>{
    const btn = await getBtnElement('addFriend');
    await btn.click();
    await printLog();
  });
  it('acceptFriend', async()=>{
    const btn = await getBtnElement('acceptFriend');
    await btn.click();
    await printLog();
  });
  it('getFriendInfo', async()=>{
    const btn = await getBtnElement('getFriendInfo');
    await btn.click();
    await printLog();
  });
  it('sendMessage', async()=>{
    const btn = await getBtnElement('sendMessage');
    await btn.click();
    await printLog();
  });

  it('createSession', async()=>{
    const btn = await getBtnElement('createSession');
    await btn.click();
    await printLog();
  });
  // it('sessionRequest', async()=>{
  //   const btn = await getBtnElement('sessionRequest');
  //   await btn.click();
  //   await printLog();
  // });
  // it('sessionReplyRequest', async()=>{
  //   const btn = await getBtnElement('sessionReplyRequest');
  //   await btn.click();
  //   await printLog();
  // });
  // it('writeStream', async()=>{
  //   const btn = await getBtnElement('writeStream');
  //   await btn.click();
  //   await printLog();
  // });
  it('closeSession', async()=>{
    const btn = await getBtnElement('closeSession');
    await btn.click();
    await printLog();
  });

 
});
import 'react-native';



import {Carrier} from 'react-native-elastos-carrier';

describe('carrier -> static', ()=>{
  test('isValidAddress', ()=>{
    const rs = Carrier.isValidAddress('DAQb3hTPLiaeLjhLyHvHK4ebJ8PcAvJUewwtxCQmbgZLVaQdLkjc');
    expect(rs).toBe(true);
  })
  
  test('getVersion', ()=>{
    const rs = Carrier.getVersion();
    expect(rs).toBe('5.1')
  })

  test('isValidId', ()=>{
    const rs = Carrier.isValidId('6XwWqntxZFwa6XmAtSmJLNZbrL9VwbsMr8GDMxKAUPmy');
    expect(rs).toBe(true);
  })
})

describe('carrier -> basic', ()=>{
  const carrier = new Carrier();
  test('start', ()=>{
    const rs = carrier.start();
    expect(rs).toBe('ok');
  })

  test('getAddress', ()=>{
    const rs = carrier.getAddress();
    expect(rs).toBe('DAQb3hTPLiaeLjhLyHvHK4ebJ8PcAvJUewwtxCQmbgZLVaQdLkjc')
  })

  test('getNodeId', ()=>{
    const rs = carrier.getNodeId();
    expect(rs).toBe('6XwWqntxZFwa6XmAtSmJLNZbrL9VwbsMr8GDMxKAUPmy')
  })

  test('setSelfInfo & getSelfInfo', ()=>{
    const rs1 = carrier.setSelfInfo({
      name : 'jacky1'
    });
    const rs2 = carrier.getSelfInfo();

    expect(rs1).toBe('ok');
    expect(rs2.name).toBe('jacky1');
  })

  test('setSelfPresence', ()=>{
    const rs = carrier.setSelfPresence(1);
    expect(rs).toBe('ok')
  })

  test('addFriend', ()=>{
    const rs = carrier.addFriend('DAQb3hTPLiaeLjhLyHvHK4ebJ8PcAvJUewwtxCQmbgZLVaQdLkjc', 'hello');
    expect(rs).toBe('ok');
  })

  test('acceptFriend', ()=>{
    const rs = carrier.acceptFriend('6XwWqntxZFwa6XmAtSmJLNZbrL9VwbsMr8GDMxKAUPmy');
    expect(rs).toBe('ok');
  })

  test('getFriendInfo', ()=>{
    const rs = carrier.getFriendInfo();
    expect(rs.name).toBe('Adem');
  })

  test('sendMessage', ()=>{
    const rs = carrier.sendMessage('6XwWqntxZFwa6XmAtSmJLNZbrL9VwbsMr8GDMxKAUPmy', 'hello');
    expect(rs).toBe('ok');
  })

  test('removeFriend', ()=>{
    const rs = carrier.removeFriend('6XwWqntxZFwa6XmAtSmJLNZbrL9VwbsMr8GDMxKAUPmy');
    expect(rs).toBe('ok');
  })

  test('setLabel', ()=>{
    const rs = carrier.setLabel('TL');
    expect(rs).toBe('ok');
  })

  test('getFriendList', ()=>{
    const rs = carrier.getFriendList();
    expect(rs[0].name).toBe('Adem');
  })

  test('close', ()=>{
    const rs = carrier.close();
    expect(rs).toBe('ok');
  })

  test('clean', ()=>{
    const rs = carrier.clean();
    expect(rs).toBe('ok');
  })
})

describe('carrier -> session', ()=>{
  const carrier = new Carrier();
  const fid = '6XwWqntxZFwa6XmAtSmJLNZbrL9VwbsMr8GDMxKAUPmy';

  test('createSession', ()=>{
    const rs = carrier.createSession(fid, 2, 1);
    expect(rs).toBe('ok');
  })

  test('sessionRequest', ()=>{
    const rs = carrier.sessionRequest(fid);
    expect(rs).toBe('ok');
  })

  test('sessionReplyRequest', ()=>{
    const rs = carrier.sessionReplyRequest(fid);
    expect(rs).toBe('ok');
  })

  test('writeStream', ()=>{
    const rs = carrier.writeStream(fid);
    expect(rs).toBe('ok');
  })

  test('removeStream', ()=>{
    const rs = carrier.removeStream(fid);
    expect(rs).toBe('ok');
  })

  test('closeSession', ()=>{
    const rs = carrier.closeSession(fid);
    expect(rs).toBe('ok');
  })

  test('addService', ()=>{
    const rs = carrier.addService(fid, 'service', '127.0.0.1', 80);
    expect(rs).toBe('ok');
  })

  test('removeService', ()=>{
    const rs = carrier.removeService(fid, 'service');
    expect(rs).toBe('ok');
  })

  test('openPortFowarding', ()=>{
    const rs = carrier.openPortFowarding(fid, 'opf', '127.0.0.1', 80);
    expect(rs).toBe(1);
  })

  test('closePortForwarding', ()=>{
    const rs = carrier.closePortForwarding(fid, 1);
    expect(rs).toBe('ok');
  })

  test('openChannel', ()=>{
    const rs = carrier.openChannel(fid, 'channel_cookie');
    expect(rs).toBe(2);
  })

  test('closeChannel', ()=>{
    const rs = carrier.closeChannel(fid, 2);
    expect(rs).toBe('ok');
  })

  test('writeChannel', ()=>{
    const rs = carrier.writeChannel(fid, 2, 'helloworld');
    expect(rs).toBe(10);
  })

  test('pendChannel', ()=>{
    const rs = carrier.pendChannel(fid, 2);
    expect(rs).toBe('ok');
  })

  test('resumeChannel', ()=>{
    const rs = carrier.resumeChannel(fid, 2);
    expect(rs).toBe('ok');
  })
})


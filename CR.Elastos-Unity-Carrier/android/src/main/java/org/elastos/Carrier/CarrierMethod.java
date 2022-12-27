package org.elastos.Carrier;


import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.Callback;
import com.facebook.react.bridge.ReadableMap;

import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

import android.telecom.Call;
import android.widget.Toast;
import java.io.File;
import java.util.Iterator;

import org.elastos.carrier.*;
import org.elastos.carrier.session.*;
import org.elastos.carrier.exceptions.ElastosException;
import org.json.JSONObject;

import com.facebook.react.bridge.LifecycleEventListener;
import com.facebook.react.bridge.WritableArray;


public class CarrierMethod extends ReactContextBaseJavaModule
        implements LifecycleEventListener {

    public static final String ok = "ok";

    private HashMap<String, RN_CARRIER> ALL_MAP = new HashMap<String, RN_CARRIER>();
    private Util util;

    private Session _session;
    private Stream _stream;


    public CarrierMethod(ReactApplicationContext reactContext) {
        super(reactContext);

        RN_CARRIER._reactContext = reactContext;

        util = Util.singleton(reactContext);

        reactContext.addLifecycleEventListener(this);
    }

    @ReactMethod
    public void test() {
        Toast.makeText(getReactApplicationContext(), "test toast", Toast.LENGTH_LONG).show();
    }

    @Override
    public String getName() {
        return "CarrierPlugin";
    }

    @Override
    public Map<String, Object> getConstants() {
        final Map<String, Object> constants = new HashMap<>();

        return constants;
    }


    @ReactMethod
    public static void getVersion(Callback callback){
        String version = Carrier.getVersion();
        callback.invoke(null, version);
    }

    @ReactMethod
    public static void isValidId(String nodeId, Callback cb){
        Boolean f = Carrier.isValidId(nodeId);
        cb.invoke(null, f);
    }

    @ReactMethod
    public static void isValidAddress(String address, Callback cb){
        Boolean f = Carrier.isValidAddress(address);
        cb.invoke(null, f);
    }

    @ReactMethod
    public void createObject(ReadableMap config, Callback cb){
        String name = config.getString("name");

        RN_CARRIER _rn = new RN_CARRIER(config);
        ALL_MAP.put(name, _rn);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void getAddress(String name, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            cb.invoke(null, _carrier.getAddress());
        }catch(ElastosException e){
            util.error("[getAddress] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }
    @ReactMethod
    public void getNodeId(String name, Callback cb){
        Carrier _carrier = getInstanceByName(name);
        try{
            cb.invoke(null, _carrier.getNodeId());
        }catch(ElastosException e){
            util.error("[getNodeId] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void getSelfInfo(String name, Callback cb){
        Carrier _carrier = getInstanceByName(name);
        RN_CARRIER _rn = getByName(name);

        try{
            RN_UserInfo info = new RN_UserInfo(_carrier.getSelfInfo());
            cb.invoke(null, info.toJS());
        }catch(ElastosException e){
            util.error("[getSelfInfo] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void setSelfInfo(String name, ReadableMap info, Callback cb){
        HashMap map = info.toHashMap();
        Carrier _carrier = getInstanceByName(name);

        try{
            RN_UserInfo new_info = new RN_UserInfo(_carrier.getSelfInfo());
            new_info.extendWithHashMap(map);
            _carrier.setSelfInfo(new_info);
            cb.invoke(null, ok);
        }catch(ElastosException e){
            util.error("[setSelfInfo] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void addFriend(String name, String address, String msg, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            _carrier.addFriend(address, msg);
            cb.invoke(null, ok);
        }catch(ElastosException e){
            util.error("[addFriend] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void acceptFriend(String name, String userId, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            // TODO acceptFriend
            _carrier.AcceptFriend(userId);
            cb.invoke(null, ok);
        }catch(ElastosException e){
            util.error("[acceptFriend] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void getFriendInfo(String name, String friendId, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            FriendInfo f_info = _carrier.getFriend(friendId);
            RN_FriendInfo ff = new RN_FriendInfo(f_info);
            cb.invoke(null, ff.toJS());
        }catch(ElastosException e){
            util.error("[getFriendInfo] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void setLabel(String name, String friendId, String label, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            // TODO labelFriend
            _carrier.LabelFriend(friendId, label);
            cb.invoke(null, ok);
        }catch(ElastosException e){
            util.error("[setLabel] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void getFriendList(String name, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            List<FriendInfo> list = _carrier.getFriends();
            WritableArray fl = Arguments.createArray();

            Iterator<FriendInfo> iterator = list.iterator();
            while(iterator.hasNext()){
                FriendInfo tmp = iterator.next();
                fl.pushMap((new RN_FriendInfo(tmp)).toJS());
            }

            cb.invoke(null, fl);
        }catch(ElastosException e){
            util.error("[getFriendList] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void sendFriendMessageTo(String name, String userId, String msg, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            _carrier.sendFriendMessage(userId, msg);
            cb.invoke(null, ok);
        }catch(ElastosException e){
            util.error("[sendFriendMessageTo] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void removeFriend(String name, String userId, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        try{
            _carrier.removeFriend(userId);
            cb.invoke(null, ok);
        }catch(ElastosException e){
            util.error("[removeFriend] "+e.toString());
            cb.invoke(e.toString(), null);
        }
    }

    @ReactMethod
    public void close(String name){
        Carrier _carrier = getInstanceByName(name);

        _carrier.kill();
    }

    @ReactMethod
    public void clean(String name){
        Carrier _carrier = getInstanceByName(name);


    }

    @ReactMethod
    public void createSession(String name, String friendId, int streamType, int streamMode, Callback cb){
        Carrier _carrier = getInstanceByName(name);

        RN_FriendInfo info = null;
        try{
            FriendInfo friendInfo = _carrier.getFriend(friendId);
            info = new RN_FriendInfo(friendInfo);
        }catch(ElastosException e){
            util.error("get friend info " + e.getErrorCode());
            cb.invoke(e.toString(), null);
            return;
        }

        if (info.getConnection().value() != 0) {
            cb.invoke(String.format("target %s is offline", friendId));
            return;
        }

        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        int streamId = _rs.start(friendId, streamType, streamMode);

        cb.invoke(null, streamId);
    }


    @ReactMethod
    public void writeStream(String name, int streamId, String data, Callback cb){

        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.writeToStream(streamId, data);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void writeStream(String name, String friendId, String data, Callback cb){

        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        FriendSessionStream fss = _rs.getFriendSessionByFriendId(friendId);
        _rs.writeToStream(fss.getStreamId(), data);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void sessionRequest(String name, String friendId, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();

        _rs.sessionRequest(friendId);
        cb.invoke(null, ok);
    }

    @ReactMethod
    public void sessionReplyRequest(String name, String friendId, int status, String reason, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();

        _rs.sessionReplyRequest(friendId, status, reason);
        cb.invoke(null, ok);
    }
    @ReactMethod
    public void removeStream(String name, String friendId, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.removeStream(friendId);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void closeSession(String name, String friendId, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.close(friendId);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void addService(String name, String friendId, String serviceName, String host, String port, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.addService(friendId, serviceName, host, port);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void removeService(String name, String friendId, String serviceName, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.removeService(friendId, serviceName);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void openPortFowarding(String name, String friendId, String serviceName, String host, String port, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        int rs = _rs.openPortFowarding(friendId, serviceName, host, port);
        cb.invoke(null, rs);
    }

    @ReactMethod
    public void closePortForwarding(String name, String friendId, int portForwardingId, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.closePortForwarding(friendId, portForwardingId);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void openChannel(String name, String friendId, String cookie, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        int rs = _rs.openChannel(friendId, cookie);

        cb.invoke(null, rs);
    }

    @ReactMethod
    public void closeChannel(String name, String friendId, int channelId, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.closeChannel(friendId, channelId);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void writeChannel(String name, String friendId, int channelId, String data, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        int rs = _rs.writeChannel(friendId, channelId, data);

        cb.invoke(null, rs);
    }

    @ReactMethod
    public void pendChannel(String name, String friendId, int channelId, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.pendChannel(friendId, channelId);

        cb.invoke(null, ok);
    }

    @ReactMethod
    public void resumeChannel(String name, String friendId, int channelId, Callback cb){
        RN_SESSION _rs = getByName(name).getRNSessionInstance();
        _rs.resumeChannel(friendId, channelId);

        cb.invoke(null, ok);
    }





    public RN_CARRIER getByName(String name){
        RN_CARRIER _rn = ALL_MAP.get(name);
        if(_rn == null){
            util.error(name+" instance not exist");
        }
        return _rn;
    }
    public Carrier getInstanceByName(String name){
        RN_CARRIER _rn = getByName(name);
        return _rn.getCarrierInstance();
    }

    @Override
    public void onHostResume() {
        util.log("[ onHostResume ]");
    }

    @Override
    public void onHostPause() {
        util.log("[ onHostPause ]");
    }

    @Override
    public void onHostDestroy() {
        util.log("[ onHostDestroy ]");
        for(String key : ALL_MAP.keySet()){
            RN_CARRIER rn = ALL_MAP.get(key);
            Carrier c = rn.getCarrierInstance();
            c.kill();
        }
        util.log("destory, killed all carrier instance.");
    }
}

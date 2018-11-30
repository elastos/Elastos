package org.elastos.Carrier;

import com.facebook.react.bridge.Callback;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReadableArray;
import com.facebook.react.bridge.ReadableMap;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.WritableArray;
import com.facebook.react.bridge.WritableMap;
import android.support.annotation.Nullable;
import com.facebook.react.bridge.Arguments;
import com.facebook.react.modules.core.DeviceEventManagerModule;

import org.elastos.carrier.*;
import org.elastos.carrier.Carrier;
import org.elastos.carrier.exceptions.*;
import org.elastos.carrier.session.*;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class RN_CARRIER extends AbstractCarrierHandler {

    public static ReactContext _reactContext;

    private Util util;
    private Carrier _carrier;
    private RN_SESSION _rs;

    public RN_CARRIER(ReadableMap config) {

        util = Util.singleton(null);

        start(config);
    }

    public void start(ReadableMap config){
        String name = config.getString("name");
        Boolean udp_enabled = config.getBoolean("udp_enabled");
        ArrayList bootstraps = config.getArray("bootstraps").toArrayList();

        String elaCarrierPath = util.getCarrierFilePath(name);
        util.log(elaCarrierPath);

        File elaCarrierDir = new File(elaCarrierPath);
        if (!elaCarrierDir.exists()) {
            elaCarrierDir.mkdirs();
        }

        List<Carrier.Options.BootstrapNode> param_bootstraps = new ArrayList<>();
        try{
            for(int i=0, m=bootstraps.size(); i<m; i++){
                Map<String, String> tmp = (Map)bootstraps.get(i);
                Carrier.Options.BootstrapNode bootstrap = new Carrier.Options.BootstrapNode();
                String ipv4 = tmp.get("ipv4");
                if (ipv4 != null) {
                    bootstrap.setIpv4(ipv4);
                }
                String ipv6 = tmp.get("ipv6");
                if (ipv6 != null) {
                    bootstrap.setIpv6(ipv6);
                }

                bootstrap.setPort(tmp.get("port"));
                bootstrap.setPublicKey(tmp.get("publicKey"));
                param_bootstraps.add(bootstrap);
            }

        }catch(Exception e){
            util.error(e.toString());
        }

        Carrier.Options options = new Carrier.Options();
        options.setPersistentLocation(elaCarrierPath).
                setUdpEnabled(udp_enabled).
                setBootstrapNodes(param_bootstraps);

        try{
            Carrier.initializeInstance(options, this);
            _carrier = Carrier.getInstance();

            _rs = new RN_SESSION(_carrier);

            util.log(String.format("Address => %s", _carrier.getAddress()));
            util.log(String.format("UserID => %s", _carrier.getUserId()));

            util.log("carrier start success");
        }catch(ElastosException e){
            util.error("[carrier init] "+e.toString());
        }

        _carrier.start(50);

    }

    @Override
    public void onConnection(Carrier carrier, ConnectionStatus status){
        util.log("[ onConnection ] : " + status);

        sendEvent("onConnection", status.value());
    }

    @Override
    public void onReady(Carrier carrier){
        util.log("Elastos carrier instance is ready.");

        sendEvent("onReady", "");
    }

    @Override
    public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {
        util.log(String.format("[ onFriendConnection ] : %s, %s", friendId, status));

        WritableMap param = Arguments.createMap();
        param.putString("friendId", friendId);
        param.putInt("status", status.value());

        sendEvent("onFriendConnection", param);

//        if(status.value() == 0){
//            getRNSessionInstance().start(friendId);
//        }

    }

    @Override
    public void onFriendAdded(Carrier carrier, FriendInfo friendInfo) {
        util.log(String.format("[ onFriendAdded ] : %s", friendInfo.toString()));

        WritableMap param = Arguments.createMap();
        param.putMap("friendInfo", (new RN_FriendInfo(friendInfo)).toJS());

        sendEvent("onFriendAdded", param);
    }

    @Override
    public void onFriendRemoved(Carrier carrier, String friendId) {
        util.log("friend " + friendId + "removed");

        WritableMap param = Arguments.createMap();
        param.putString("friendId", friendId);

        sendEvent("onFriendRemoved", friendId);
    }

    @Override
    public void onFriendPresence(Carrier carrier, String friendId, PresenceStatus presence) {
        util.log("Friend " + friendId + " presence changed to " + presence);

        WritableMap param = Arguments.createMap();
        param.putString("friendId", friendId);
        param.putInt("presence", presence.value());

        sendEvent("onFriendPresence", param);
    }

    @Override
    public void onFriendInfoChanged(Carrier carrier, String friendId, FriendInfo friendInfo) {
        RN_FriendInfo f_info = new RN_FriendInfo(friendInfo);
        util.log("[ onFriendInfoChanged ] : " + friendInfo.toString());

        sendEvent("onFriendInfoChanged", f_info.toJS());
    }

    @Override
    public void onFriends(Carrier carrier, List<FriendInfo> friends) {
        util.log("[ onFriends ] : " + friends);

        WritableArray param = Arguments.createArray();
        for(FriendInfo info: friends){
            param.pushMap((new RN_FriendInfo(info)).toJS());
        }

        sendEvent("onFriends", param);
    }

    @Override
    public void onFriendRequest(Carrier carrier, String userId, UserInfo userInfo, String hello){
        util.log(String.format("[ onFriendRequest ] : %s , %s, %s", userId, userInfo.toString(), hello));

        WritableMap param = Arguments.createMap();
        param.putString("userId", userId);
        param.putMap("userInfo", (new RN_UserInfo(userInfo)).toJS());
        param.putString("msg", hello);

        sendEvent("onFriendRequest", param);
    }

    @Override
    public void onFriendMessage(Carrier carrier, String from, String msg){
        util.log(String.format("[ onFriendMessage ] : %s, %s", from, msg));

        WritableMap param = Arguments.createMap();
        param.putString("userId", from);
        param.putString("message", msg);

        sendEvent("onFriendMessage", param);
    }

    @Override
    public void onSelfInfoChanged(Carrier carrier, UserInfo info){
        util.log(String.format("[ onSelfInfoChanged ] : %s", info.toString()));

        sendEvent("onSelfInfoChanged", (new RN_UserInfo(info)).toJS());
    }




    public Carrier getCarrierInstance(){
        return _carrier;
    }


    public static void sendEvent(String eventName, @Nullable Integer params){
        WritableArray array = Arguments.createArray();
        array.pushInt(params);
        _reactContext.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter.class).emit(eventName, array);
    }
    public static void sendEvent(String eventName, @Nullable String params){
        WritableArray array = Arguments.createArray();
        array.pushString(params);
        _reactContext.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter.class).emit(eventName, array);
    }
    public static void sendEvent(String eventName, @Nullable WritableMap params){
        WritableArray array = Arguments.createArray();
        array.pushMap(params);
        _reactContext.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter.class).emit(eventName, array);
    }
    public static void sendEvent(String eventName, @Nullable WritableArray params){
        WritableArray array = Arguments.createArray();
        array.pushArray(params);
        _reactContext.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter.class).emit(eventName, array);
    }


    public RN_SESSION getRNSessionInstance(){
        return _rs;
    }


}

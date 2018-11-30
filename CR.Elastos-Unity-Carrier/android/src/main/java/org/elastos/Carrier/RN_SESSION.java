package org.elastos.Carrier;

import android.icu.util.Freezable;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.WritableMap;

import org.elastos.carrier.Carrier;
import org.elastos.carrier.FriendInfo;
import org.elastos.carrier.session.*;
import org.elastos.carrier.exceptions.ElastosException;

import java.util.HashMap;


public class RN_SESSION extends AbstractStreamHandler implements SessionRequestCompleteHandler {

    private Util util;


    private Carrier _carrier;

    private Manager _manager;

    String sessionRequestSdp;

    public RN_SESSION(Carrier carrier){
        _carrier = carrier;
        try{
            _manager = Manager.getInstance(_carrier, new ManagerHandler(){
                @Override
                public void onSessionRequest(Carrier carrier, String friendId, String sdp) {
                    util.log(String.format("[ onSessionRequest ] => %s, %s", friendId, sdp));
                    sessionRequestSdp = sdp;

                    FriendSessionStream fss = getFriendSessionByFriendId(friendId);
                    fss.setSdp(sdp);

                    WritableMap param = Arguments.createMap();
                    param.putString("sdp", sdp);
                    param.putString("friendId", friendId);
                    RN_CARRIER.sendEvent("onSessionRequest", param);
                }
            });
        }catch(ElastosException e){
            util.error(String.format("[ Session Manager ] => %s", e.getErrorCode()));
        }

        util = Util.singleton(null);
    }

    public void close(String friendId) {

        FriendSessionStream fss = FriendSessionStream.map.get(friendId);
        util.log("[ close ]");
        if(fss != null){
            fss.close();

        }

    }


    /*
    * start session
    * */
    public int start(String friendId, int streamType, int streamMode) {
        util.log(String.format("[ RN_SESSION.create ] => %s", friendId));

//        int sopt = Stream.PROPERTY_RELIABLE;
        int rs = addStreamWithType(friendId, StreamType.valueOf(streamType), streamMode);

        return rs;
    }

    public int addStreamWithType(String friendId, StreamType type, int mode){
        FriendSessionStream fss = getFriendSessionByFriendId(friendId);
        Session session = fss.getSession();
        Stream stream = null;
        try{

            stream = session.addStream(type, mode, this);
            fss.setStream(stream);
        }catch(ElastosException e){
            util.error(String.format("add stream error (0x%x)", e.getErrorCode()));
            e.getStackTrace();
        }



        return stream.getStreamId();
    }

    public void removeStream(String friendId){
        FriendSessionStream fss = getFriendSessionByFriendId(friendId);
        try{
            Session session = fss.getSession();
            Stream stream = fss.getStream();
            session.removeStream(stream);
            fss.close();
        }catch(ElastosException e){
            util.error(String.format("remove stream error (0x%x)", e.getErrorCode()));
            e.getStackTrace();
        }
    }

    public FriendSessionStream getFriendSessionByFriendId(String friendId){
        FriendSessionStream fss = FriendSessionStream.map.get(friendId);
        if(fss == null){
            fss = new FriendSessionStream(friendId);
            try{
                Session session = _manager.newSession(friendId);
                fss.setSession(session);
            }catch(ElastosException e){
                e.printStackTrace();

                util.error(String.format("new session error (0x%x)", e.getErrorCode()));
            }
            FriendSessionStream.map.put(friendId, fss);
        }

        return fss;
    }

    public void writeToStream(int streamId, String data){
        util.log(String.format("[ writeToStream ] => %s", data));
        FriendSessionStream fss = FriendSessionStream.getInstanceByStreamId(streamId);

        Stream stream = fss.getStream();
        byte[] bindary = data.getBytes();

        try{
            stream.writeData(bindary);
        }catch(ElastosException e){
            e.getStackTrace();
            util.error(String.format("writeToStream error => %s", e.getErrorCode()));
        }

    }


    public void sessionRequest(String friendId){
        // TODO check session state
        util.log(String.format("[ sessionRequest ] => %s", friendId));
        FriendSessionStream fss = getFriendSessionByFriendId(friendId);
        try{
            Session session = fss.getSession();
            session.request(this);
            util.log("Session request sent.");
        }catch(ElastosException e){
            util.error("[ sessionRequest ]");
        }
    }

    public void sessionReplyRequest(String friendId, int status, String reason){
        // TODO check session state
        util.log(String.format("[ sessionReplyRequest ] => %s, %s, %s", friendId, status, reason));
        FriendSessionStream fss = getFriendSessionByFriendId(friendId);
        try{
            Session session = fss.getSession();
            session.replyRequest(status, reason);
            util.log("Session reply request.");

            String sdp = fss.getSdp();
            session.start(sdp);
            util.log("Session started success.");
        }catch(ElastosException e){
            util.error("[ sessionReplyRequest ]");
        }
    }

    @Override
    public void onCompletion(Session session, int status, String reason, String sdp) {
        util.log(String.format("[ onCompletion ] => status=%s, reason=%s, sdp=%s", status, reason, sdp));
        if (status != 0) {
            util.log(String.format("Session request completion with error (%d:%s", status, reason));

            return;
        }

        try {
//            sessionRequestSdp = sdp;
            session.start(sdp);
            util.log("Session started success.");
        } catch (ElastosException e) {
            util.error("Session start error " + e.getErrorCode());
        }
    }

    @Override
    public void onStreamData(Stream stream, byte[] receivedData) {
        util.log(String.format("[ onStreamData ] => data=%s", new String(receivedData)));

        FriendSessionStream fss = FriendSessionStream.getInstanceByStreamId(stream.getStreamId());

        WritableMap param = Arguments.createMap();

        param.putString("text", new String(receivedData));
        param.putInt("streamId", stream.getStreamId());
        param.putString("friendId", fss.getId());

        RN_CARRIER.sendEvent("onStreamData", param);
    }

    @Override
    public void onStateChanged(Stream stream, StreamState state) {
        util.log("onStateChanged : " + stream.getStreamId() + "  :  " + state.toString());

        FriendSessionStream fss = FriendSessionStream.getInstanceByStreamId(stream.getStreamId());
        if(fss == null){
//            util.error(String.format("[FriendSessionStream.getInstanceByStreamId] => %d" + stream.getStreamId()));
            return;
        }


        fss.setState(state);

        WritableMap param = Arguments.createMap();
        param.putInt("streamId", stream.getStreamId());
        param.putString("friendId", fss.getId());
        param.putInt("state", state.value());

        RN_CARRIER.sendEvent("onStateChanged", param);
        switch (state) {
            case Initialized:
                break;
            case TransportReady:
                util.log("Stream to transport ready");
                break;

            case Connected:
                util.log("Stream to connected.");


                break;

            case Deactivated:
                util.log("Stream deactived");
//                fss.close();
                break;
            case Closed:
                util.log("Stream closed");
//                fss.close();
                break;
            case Error:
                util.log("Stream error");
//                fss.close();

                // TODO restart carrier

                break;
        }
    }
}

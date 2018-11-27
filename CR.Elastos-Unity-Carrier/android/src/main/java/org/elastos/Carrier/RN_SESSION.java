package org.elastos.Carrier;

import org.elastos.carrier.Carrier;
import org.elastos.carrier.FriendInfo;
import org.elastos.carrier.session.*;
import org.elastos.carrier.exceptions.ElastosException;

import java.util.HashMap;


public class RN_SESSION extends AbstractStreamHandler implements SessionRequestCompleteHandler {

    private Util util;
    private StreamState mState = StreamState.Closed;


    private Carrier _carrier;

    private Manager _manager;

    String sessionRequestSdp;

    public RN_SESSION(Carrier carrier){
        _carrier = carrier;
        try{
            _manager = Manager.getInstance(_carrier, new ManagerHandler(){
                @Override
                public void onSessionRequest(Carrier carrier, String s, String s1) {
                    util.log(String.format("[ onSessionRequest ] => %s, %s", s, s1));
                    sessionRequestSdp = s1;

                }
            });
        }catch(ElastosException e){
            util.error(String.format("[ Session Manager ] => %s", e.getErrorCode()));
        }

        util = Util.singleton(null);
    }

    public void close(String friendId) {

        FriendSessionStream fss = getFriendSessionByFriendId(friendId);
        util.log("[ close ]");
        if(fss != null){
            fss.close();

            mState  = StreamState.Closed;
        }

    }

    /*
    * start session
    * */
    public int start(String friendId) {
        util.log(String.format("[ RN_SESSION.create ] => %s", friendId));

//        if (mState == StreamState.Initialized || mState == StreamState.TransportReady
//                || mState == StreamState.Connecting) {
//            return;
//        }
//        else if (mState == StreamState.Connected) {
//
//            return;
//        }
//        else {
//
//
//        }

        mState = StreamState.Closed;

//            int sopt = Stream.PROPERTY_PLAIN | Stream.PROPERTY_RELIABLE;
        int sopt = Stream.PROPERTY_RELIABLE;
        int rs = addStreamWithType(friendId, StreamType.Text, sopt);

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


    public FriendSessionStream getFriendSessionByFriendId(String frinedId){
        FriendSessionStream fss = FriendSessionStream.map.get(frinedId);
        if(fss == null){
            fss = new FriendSessionStream(frinedId);
            try{
                Session session = _manager.newSession(frinedId);
                fss.setSession(session);
            }catch(ElastosException e){
                e.printStackTrace();

                util.error(String.format("new session error (0x%x)", e.getErrorCode()));
            }
            FriendSessionStream.map.put(frinedId, fss);
        }

        return fss;
    }

    @Override
    public void onCompletion(Session session, int status, String reason, String sdp) {
        util.log(String.format("[ onCompletion ] => status=%s, reason=%s, sdp=%s", status, reason, sdp));
        if (status != 0) {
            util.log(String.format("Session request completion with error (%d:%s", status, reason));

            return;
        }

        try {
            sessionRequestSdp = sdp;
            session.start(sdp);
            util.log("Session started success.");
        } catch (ElastosException e) {
            util.error("Session start error " + e.getErrorCode());
        }
    }

    @Override
    public void onStreamData(Stream stream, byte[] receivedData) {
        util.log(String.format("[ onStreamData ] => data=%s", new String(receivedData)));
    }

    @Override
    public void onStateChanged(Stream stream, StreamState state) {
        util.log("onStateChanged : " + stream.getStreamId() + "  :  " + state);

        FriendSessionStream fss = FriendSessionStream.getInstanceByStreamId(stream.getStreamId());
        if(fss == null){
            util.error(String.format("[FriendSessionStream.getInstanceByStreamId] => %s" + stream.getStreamId()));
            return;
        }

        Session _session = fss.getSession();

        mState = state;
        switch (state) {
            case Initialized:
                try{
                    _session.request(this);
                    util.log("Session request sent.");
                }catch(ElastosException e){
                    util.error("[ Session.request ]");
                }

                break;

            case TransportReady:
                util.log("Stream to transport ready");

                break;

            case Connected:
                util.log("Stream to connected.");


                break;

            case Deactivated:
                util.log("Stream deactived");
                fss.close();
                break;
            case Closed:
                util.log("Stream closed");
                fss.close();
                break;
            case Error:
                util.log("Stream error");
                fss.close();

                // TODO restart carrier

                break;
        }
    }
}

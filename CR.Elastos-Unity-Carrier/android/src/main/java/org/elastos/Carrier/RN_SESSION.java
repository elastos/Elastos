package org.elastos.Carrier;

import org.elastos.carrier.Carrier;
import org.elastos.carrier.FriendInfo;
import org.elastos.carrier.session.*;
import org.elastos.carrier.exceptions.ElastosException;


public class RN_SESSION extends AbstractStreamHandler implements SessionRequestCompleteHandler {

    private Util util;
    private Session _session;
    private Stream _stream;
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

    public void close() {
        util.log("[ close ]");
        if (_session != null) {
            _session.close();
            _session = null;
            _stream = null;
            mState  = StreamState.Closed;
        }
    }

    /*
    * start session
    * */
    public void start(String friendId) {
        util.log(String.format("[ RN_SESSION.create ] => %s", friendId));

//        RN_FriendInfo info = null;
//        try{
//            FriendInfo friendInfo = _carrier.getFriend(friendId);
//            info = new RN_FriendInfo(friendInfo);
//        }catch(ElastosException e){
//            util.error("get friend info " + e.getErrorCode());
//        }
//
//        if (info == null || info.getConnection().value() != 0) {
//            return;
//        }

        if (mState == StreamState.Initialized || mState == StreamState.TransportReady
                || mState == StreamState.Connecting) {
            return;
        }
        else if (mState == StreamState.Connected) {

            return;
        }
        else {
            mState = StreamState.Closed;

            int sopt = Stream.PROPERTY_MULTIPLEXING
                    | Stream.PROPERTY_PORT_FORWARDING
                    | Stream.PROPERTY_RELIABLE;

            try {
                _session = _manager.newSession(friendId);
                _session.addStream(StreamType.Application, sopt, this);
            }
            catch (ElastosException e) {
                e.printStackTrace();

                if (_session == null) {
                    util.error(String.format("New session error (0x%x)", e.getErrorCode()));
                }
                else {
                    util.error(String.format("Add stream error (0x%x)", e.getErrorCode()));
                    _session.close();
                    _session = null;
                }
            }
        }
    }


    @Override
    public void onCompletion(Session session, int status, String reason, String sdp) {
        util.log(String.format("[ onCompletion ] => status=%s, reason=%s, sdp=%s", status, reason, sdp));
        if (status != 0) {
            util.log(String.format("Session request completion with error (%d:%s", status, reason));
            close();
            return;
        }

        try {
            session.start(sdp);
            util.log("Session started success.");
        } catch (ElastosException e) {
            util.error("Session start error " + e.getErrorCode());
        }
    }

    @Override
    public void onStateChanged(Stream stream, StreamState state) {
        util.log("onStateChanged : " + stream.getStreamId() + "  :  " + state);
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
                _stream = stream;

                break;

            case Deactivated:
                util.log("Stream deactived");
                close();
                break;
            case Closed:
                util.log("Stream closed");
                close();
                break;
            case Error:
                util.log("Stream error");
                close();

                // TODO restart carrier

                break;
        }
    }
}

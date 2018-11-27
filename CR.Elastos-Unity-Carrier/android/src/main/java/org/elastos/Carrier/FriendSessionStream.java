package org.elastos.Carrier;

import org.elastos.carrier.session.*;

import java.util.HashMap;

public class FriendSessionStream {

    public static HashMap<String, FriendSessionStream> map = new HashMap<>();

    private String id;

    private Session session;
    private Stream stream;

    public FriendSessionStream(String friendId){
        this.id = friendId;
    }

    public String getId(){
        return this.id;
    }

    public void setSession(Session session){
        this.session = session;
    }

    public Session getSession(){
        return this.session;
    }

    public void setStream(Stream stream){
        this.stream = stream;
    }

    public Stream getStream(){
        return this.stream;
    }

    public int getStreamId(){
        return this.stream.getStreamId();
    }

    public void close(){
        // TODO

        this.session.close();
        FriendSessionStream.map.remove(this.id);
    }


    // static method
    public static FriendSessionStream getInstanceByStreamId(int streamId){
        FriendSessionStream rs = null;

        for(FriendSessionStream val : FriendSessionStream.map.values()){
            Stream stream = val.getStream();
            if(stream != null && val.getStreamId() == streamId){
                rs = val;
                break;
            }
        }

        return rs;
    }
}

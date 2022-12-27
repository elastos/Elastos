package org.elastos.Carrier;

import org.elastos.carrier.UserInfo;
import com.facebook.react.bridge.WritableNativeMap;

import java.util.HashMap;


public class RN_UserInfo extends UserInfo {
    private Util util = Util.singleton(null);

    private String userId;

    @Override
    public String getUserId() {
        return this.userId;
    }

    public RN_UserInfo(UserInfo info){
        super();

        this.setEmail(info.getEmail());
        this.setDescription(info.getDescription());
        this.setGender(info.getGender());
        this.setHasAvatar(info.hasAvatar());
        this.setPhone(info.getPhone());
        this.setRegion(info.getRegion());
        this.setName(info.getName());

        this.userId = info.getUserId();
    }

    public void extendWithHashMap(HashMap<String, String> map){
        this.setDescription(map.get("description"));
        this.setName(map.get("name"));
        this.setEmail(map.get("email"));
        this.setGender(map.get("gender"));
        this.setPhone(map.get("phone"));
        this.setRegion(map.get("region"));
    }



    public WritableNativeMap toJS(){
        WritableNativeMap rs = new WritableNativeMap();

        rs.putString("name", getName());
        rs.putString("email", getEmail());
        rs.putString("description", getDescription());
        rs.putString("userId", getUserId());
        rs.putString("region", getRegion());
        rs.putString("gender", getGender());
        rs.putString("phone", getPhone());

        return rs;
    }


}

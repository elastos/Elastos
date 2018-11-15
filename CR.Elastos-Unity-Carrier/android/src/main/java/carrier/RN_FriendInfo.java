package carrier;

import com.facebook.react.bridge.WritableNativeMap;

import org.elastos.carrier.ConnectionStatus;
import org.elastos.carrier.PresenceStatus;
import org.elastos.carrier.UserInfo;
import org.elastos.carrier.FriendInfo;

public class RN_FriendInfo extends RN_UserInfo {

    private PresenceStatus presence;
    private ConnectionStatus connection;
    private String label;

    public RN_FriendInfo(FriendInfo friendInfo){
        super(friendInfo);

        this.presence = friendInfo.getPresence();
        this.connection = friendInfo.getConnectionStatus();
        this.label = friendInfo.getLabel();
    }

    @Override
    public WritableNativeMap toJS(){
        WritableNativeMap rs = super.toJS();

        rs.putInt("presence", this.presence.value());
        rs.putInt("status", this.connection.value());
        rs.putString("label", this.label);

        return rs;
    }
}

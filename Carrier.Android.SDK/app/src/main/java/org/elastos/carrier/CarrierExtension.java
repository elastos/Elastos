package org.elastos.carrier;

import org.elastos.carrier.exceptions.CarrierException;

public abstract class CarrierExtension {
    private static final String TAG = "CarrierExtension";
    private Carrier carrier;
    private long nativeCookie = 0;

    private native boolean native_init(Carrier carrier);
    private native boolean invite_friend(Carrier carrier, String to, String data,
                                         FriendInviteResponseHandler handler);
    private native boolean reply_friend(Carrier carrier, String to, int status, String reason, String data);
    private native TurnServerInfo get_turn_server(Carrier carrier);
    private native void native_cleanup(Carrier carrier);
    private static native int get_error_code();

    protected static class TurnServerInfo {
        private String server;
        private String username;
        private String password;
        private String realm;
        private int port;

        TurnServerInfo(String server, String username, String password, String realm,
                       int port) {
            this.server = server;
            this.username = username;
            this.password = password;
            this.realm = realm;
            this.port = port;
        }

        public String getServer() {
            return server;
        }

        public int getPort() {
            return port;
        }

        public String getUsername() {
            return username;
        }

        public String getPassword() {
            return password;
        }

        public String getRealm() {
            return realm;
        }
    }

    abstract protected void onFriendInvite(Carrier carrier, String from, String data);

    protected CarrierExtension(Carrier carrier) {
        if (carrier == null)
            throw new IllegalArgumentException();

        this.carrier = carrier;

        Log.i(TAG, "CarrierExtension instance created");
    }

    protected TurnServerInfo getTurnServerInfo() throws CarrierException {
        TurnServerInfo sinfo;

        if (nativeCookie == 0)
            throw new IllegalStateException();

        sinfo = get_turn_server(carrier);
        if (sinfo == null)
            throw CarrierException.fromErrorCode(get_error_code());

        Log.d(TAG, String.format("Get TURN server info {\n  server: %s\n  username: %s\n  password: %s\n" +
                        "  realm: %s\n  port: %d\n}",
              sinfo.getServer(), sinfo.getUsername(), sinfo.getPassword(), sinfo.getRealm(), sinfo.getPort()));

        return sinfo;
    }

    protected void inviteFriend(String to, String data, FriendInviteResponseHandler handler)
            throws CarrierException {
        if (nativeCookie == 0)
            throw new IllegalStateException();

        if (to == null || to.length() == 0 ||
                data == null || data.length() == 0 || handler == null)
            throw new IllegalArgumentException();

        Log.d(TAG, "Inviting friend " + to + "with greet data " + data);

        if (!invite_friend(carrier, to, data, handler))
            throw CarrierException.fromErrorCode(get_error_code());

        Log.d(TAG, "Send friend invite request to " + to);
    }

    protected void replyFriendInvite(String to, int status, String reason, String data)
            throws CarrierException {
        if (nativeCookie == 0)
            throw new IllegalStateException();

        if (to == null || to.length() == 0 || (status != 0 && reason == null))
            throw new IllegalArgumentException();

        if (status == 0)
            Log.d(TAG, String.format("Attempt to confirm friend invite to %s with data [%s]",
                    to, data));
        else
            Log.d(TAG, String.format("Attempt to refuse friend invite to %s with status %d," +
                    "and reason %s", to, status, reason));

        if (!reply_friend(carrier, to, status, reason, data))
            throw CarrierException.fromErrorCode(get_error_code());

        if (status == 0)
            Log.d(TAG, String.format("Confirmed friend invite to %s with data [%s]", to, data));
        else
            Log.d(TAG, String.format("Refused friend invite to %s with status %d and " +
                    "reason %s", to, status, reason));
    }

    protected void registerExtension() throws CarrierException {
        if (nativeCookie != 0)
            return;

        if (!native_init(carrier))
            throw CarrierException.fromErrorCode(get_error_code());
    }

    @Override
    protected void finalize() throws Throwable {
        if (nativeCookie != 0)
            native_cleanup(carrier);
        super.finalize();
    }
}

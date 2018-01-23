package org.elastos.carrier.session;

import org.elastos.carrier.Carrier;
import org.elastos.carrier.Log;
import org.elastos.carrier.exceptions.ElastosException;

/**
 * The class representing Carrier session manager.
 */
public class Manager {
    private static final String TAG = "SessionMgr";

    private static Manager sessionMgr;

    private Carrier carrier;
    private boolean didCleanup;

    // jni native methods.
    private static native boolean native_init(Carrier carrier, ManagerHandler handler);
    private static native void native_cleanup(Carrier carrier);
    private native Session create_session(Carrier carrier, String to);
    private static native int get_error_code();

    /**
     * Get a carrier session manager instance.
     *
     * This function is convinience way to get instance without interest to session request
     * from friends.
     *
     * @param
     * 		carrier		Carrier node instance
     *
     * @return
     * 		A carrier session manager
     *
     * @throws
     * 		ElastosException
     */
    public static Manager getInstance(Carrier carrier) throws ElastosException {

        return getInstance(carrier, null);
    }

    /**
     * Get a carrier session manager instance.
     *
     * @param
     * 		carrier		Carrier node instance
     * @param
     *      handler     The interface handler for carrier session manager to comply with
     *
     * @return
     * 		A carrier session manager
     *
     * @throws
     * 		ElastosException
     */
    public static Manager getInstance(Carrier carrier, ManagerHandler handler)
			throws ElastosException {

        if (sessionMgr != null && sessionMgr.carrier != carrier) {
            sessionMgr.cleanup();
        }

        if (sessionMgr == null) {
            if (carrier == null)
                throw new IllegalArgumentException();

            Log.d(TAG, "Attempt to create carrier session manager instance ...");

            if (!native_init(carrier, handler))
                throw new ElastosException(get_error_code());

            sessionMgr = new Manager(carrier);

            Log.d(TAG, "Carrier session manager instance created");
        }

        return sessionMgr;
    }

    /**
     * Get a carrier session manager instance.
     *
     * @return
     * 		A carrier session manager or null
     */
    public static Manager getInstance() {
        return sessionMgr;
    }

    private Manager(Carrier carrier) {
        this.carrier = carrier;
        this.didCleanup = false;
    }

    @Override
    protected void finalize() throws Throwable {
        cleanup();
        super.finalize();
    }

    /**
     * Clean up carrier session manager.
     */
    public synchronized void cleanup() {
        if (!didCleanup) {
            native_cleanup(carrier);
			carrier = null;
            Manager.sessionMgr = null;
            didCleanup = true;
        }
    }

    /**
     * Create a new session to a friend.
     *
     * The session object represent a conversation handle to a friend.
     *
     * @param
     *      to          The target id(userid or userid@nodeid).
     *
     * @return
     *      The new Session object
     *
     * @throws
     *      IllegalArgumentException
     * 		ElastosException
     */
    public Session newSession(String to) throws ElastosException {

        if (to == null)
            throw new IllegalArgumentException();

        Log.d(TAG, "Attempt to create a new session to:" + to);

        Session session = create_session(carrier, to);
        if (session == null) {
            throw new ElastosException(get_error_code());
        }

        Log.d(TAG, "Session to " + to +  " created");

        return session;
    }
}

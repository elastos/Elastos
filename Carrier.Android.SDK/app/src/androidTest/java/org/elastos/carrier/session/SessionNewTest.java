package org.elastos.carrier.session;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.util.Log;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.*;
import org.elastos.carrier.robot.RobotProxy;
import org.elastos.carrier.exceptions.CarrierException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

public class SessionNewTest {
    private static final String TAG = "SessionNewTest";
    private static Carrier carrierInst;
    private static TestHandler handler;
    private static Manager sessionMgr;
    private static RobotProxy robotProxy;
    private static RobotProxy.RobotIdReceiver receiver;
    private static String robotId;
    private static String robotAddress;

    private static Context getAppContext() {
        return InstrumentationRegistry.getTargetContext();
    }

    private static String getAppPath() {
        return getAppContext().getFilesDir().getAbsolutePath();
    }

    static class TestHandler extends AbstractCarrierHandler {
        Synchronizer synch = new Synchronizer();

        public void onReady(Carrier carrier) {
            synch.wakeup();
        }

        public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {
            if (status == ConnectionStatus.Connected)
                synch.wakeup();
        }
    }

    static class TestReceiver implements RobotProxy.RobotIdReceiver {
        private Synchronizer synch = new Synchronizer();

        @Override
        public void onReceived(String address, String userId) {
            robotAddress = address;
            robotId = userId;
            synch.wakeup();
        }
    }

    static class TestStreamHandler extends AbstractStreamHandler {
        Synchronizer synch = new Synchronizer();
        Stream stream;
        StreamState state;

        byte[] receivedData;

        @Override
        public void onStateChanged(Stream stream, StreamState state) {
			Log.i(TAG, "Stream " + stream.getStreamId() + "'s state changed to be " + state.name());
            this.stream = stream;
            this.state = state;
            synch.wakeup();
        }

        @Override
        public void onStreamData(Stream stream, byte[] data) {
            this.stream = stream;
            this.receivedData = data;
            synch.wakeup();
        }
    }

    static class TestSessionRequestCompleteHandler implements SessionRequestCompleteHandler {
        Synchronizer synch = new Synchronizer();

        Session session;
        int status;
        String reason;
        String sdp;

        public void onCompletion(Session session, int status, String reason, String sdp) {
            this.session = session;
            this.status = status;
            this.reason = reason;
            this.sdp = sdp;

            synch.wakeup();
        }
    }

    @BeforeClass
    public static void setUp() {
        try {
            TestReceiver receiver = new TestReceiver();
            robotProxy = RobotProxy.getRobot(getAppContext());
            robotProxy.bindRobot(receiver);
            receiver.synch.await(); // for acquiring robot id.

            handler = new TestHandler();
            Carrier.initializeInstance(new TestOptions(getAppPath()), handler);
            carrierInst = Carrier.getInstance();
            carrierInst.start(1000);
            handler.synch.await();

            sessionMgr = Manager.getInstance(carrierInst);
            robotProxy.tellRobotInitSessionManager();
			robotProxy.waitForSesionManagerInitialzed();

        } catch (CarrierException e) {
            Log.e(TAG, "error: " + e.getErrorCode());
            e.printStackTrace();
        }
    }

    @AfterClass
    public static void tearDown() {
        try {
            robotProxy.tellRobotCleanupSessionManager();
            sessionMgr.cleanup();
            carrierInst.kill();
            robotProxy.unbindRobot();
        }catch(Exception e) {
            e.printStackTrace();
        }
    }

    private void makeFriendAnyWay() {
        try {
            if (!carrierInst.isFriend(robotId))
                carrierInst.addFriend(robotAddress, "auto-accepted");

            handler.synch.await(); // for friend added.
        } catch (CarrierException e) {
            e.printStackTrace();
            assertTrue(false);
        }
    }

    @Test
    public void testSession() {
        makeFriendAnyWay();

        try {
            Session session = sessionMgr.newSession(robotId);

            TestStreamHandler streamHandler = new TestStreamHandler();
            Stream  stream = session.addStream(StreamType.Text, 0, streamHandler);
            streamHandler.synch.await();
            assertEquals(streamHandler.stream, stream);
            assertEquals(streamHandler.state, StreamState.Initialized);

            TestSessionRequestCompleteHandler reqCompleteHandler = new TestSessionRequestCompleteHandler();
            session.request(reqCompleteHandler);

			streamHandler.synch.await();
			assertEquals(streamHandler.stream, stream);
			assertEquals(streamHandler.state, StreamState.TransportReady);

			robotProxy.waitForSessionRequestArrival();
			robotProxy.tellRobotReplySessionRequestAndStart();

            reqCompleteHandler.synch.await();
            assertEquals(reqCompleteHandler.session, session);
            assertEquals(reqCompleteHandler.status, 0);
            assertNull(reqCompleteHandler.reason);
            assertNotNull(reqCompleteHandler.sdp);

            session.start(reqCompleteHandler.sdp);
			streamHandler.synch.await();
            assertEquals(streamHandler.state, StreamState.Connecting);
			streamHandler.synch.await();
            assertEquals(streamHandler.state, StreamState.Connected);

			robotProxy.waitForSessionConnected();

			byte[] data = new byte[10];
			for ( int i = 0; i < 10; i++) {
				data[i] = (byte)i;
			}

            stream.writeData(data);
            streamHandler.synch.await();

			assertEquals(data.length, streamHandler.receivedData.length);
			assertEquals(streamHandler.receivedData.length, 10);
			for (int i = 0; i < 10; i++) {
				assertEquals(data[i], streamHandler.receivedData[i]);
			}

            session.removeStream(stream);
            session.close();

        } catch (CarrierException e) {
			Log.e(TAG, "error: " + e.getErrorCode());
            e.printStackTrace();
			assertTrue(false);
        }
    }
}

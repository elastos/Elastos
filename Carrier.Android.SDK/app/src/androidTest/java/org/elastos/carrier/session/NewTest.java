package org.elastos.carrier.session;

import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.elastos.carrier.AbstractCarrierHandler;
import org.elastos.carrier.Carrier;
import org.elastos.carrier.ConnectionStatus;
import org.elastos.carrier.FriendInfo;
import org.elastos.carrier.common.RobotConnector;
import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestContext;
import org.elastos.carrier.common.TestHelper;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
public class NewTest {
	private static final String TAG = "SessionNewTest";
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer commonSyncher = new Synchronizer();
	private static TestContext context = new TestContext();
	private static TestHandler handler = new TestHandler(context);
	private static SessionManagerHandler sessionHandler = new SessionManagerHandler();
	private static RobotConnector robot;
	private static Carrier carrier;
	private static Manager sessionManager;

	static class TestHandler extends AbstractCarrierHandler {
		private TestContext mContext;

		TestHandler(TestContext context) {
			mContext = context;
		}

		@Override
		public void onReady(Carrier carrier) {
			synchronized (carrier) {
				carrier.notify();
			}
		}

		@Override
		public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {
			TestContext.Bundle bundle = mContext.getExtra();
			bundle.setRobotOnline(status == ConnectionStatus.Connected);
			bundle.setRobotConnectionStatus(status);
			bundle.setFrom(friendId);

			Log.d(TAG, "Robot connection status changed -> " + status.toString());
			friendConnSyncher.wakeup();
		}

		@Override
		public void onFriendAdded(Carrier carrier, FriendInfo info) {
			Log.d(TAG, String.format("Friend %s added", info.getUserId()));
			commonSyncher.wakeup();
		}

		@Override
		public void onFriendRemoved(Carrier carrier, String friendId) {
			Log.d(TAG, String.format("Friend %s removed", friendId));
			commonSyncher.wakeup();
		}
	}

	static class SessionManagerHandler implements ManagerHandler {
		@Override
		public void onSessionRequest(Carrier carrier, String from, String sdp) {
			Log.d(TAG, String.format("Session Request from %s", from));
		}
	}

	@Test
	public void testNewSession() {
		try {
			friendConnSyncher.reset();
			commonSyncher.reset();

			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));
			Session session = sessionManager.newSession(robot.getNodeid());
			assertNotNull(session);
			session.close();
		}
		catch (CarrierException e) {
			e.printStackTrace();
		}
	}

	@Test
	public void testNewSessionWithStrager() {
		try {
			friendConnSyncher.reset();
			commonSyncher.reset();

			assertTrue(TestHelper.removeFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertFalse(carrier.isFriend(robot.getNodeid()));
			Session session = sessionManager.newSession(robot.getNodeid());
			assertNull(session);
		}
		catch (CarrierException e) {
			assertEquals(e.getErrorCode(), 0x8100000A);
		}
	}

	private static boolean isConnectToRobot = false;
	@BeforeClass
	public static void setUp() {
		robot = RobotConnector.getInstance();
		if (!robot.isConnected()) {
			isConnectToRobot = true;
			if (!robot.connectToRobot()) {
				android.util.Log.e(TAG, "Connection to robot failed, abort this test");
				fail();
			}
		}

		TestOptions options = new TestOptions(context.getAppPath());
		try {
			carrier = Carrier.createInstance(options, handler);
			carrier.start(0);
			synchronized (carrier) {
				carrier.wait();
			}
			Log.i(TAG, "Carrier node is ready now");

			sessionManager = Manager.createInstance(carrier, sessionHandler);
			assertNotNull(sessionManager);
		}
		catch (CarrierException | InterruptedException e) {
			e.printStackTrace();
			Log.e(TAG, "Carrier node start failed, abort this test.");
		}
	}

	@AfterClass
	public static void tearDown() {
		sessionManager.cleanup();
		carrier.kill();

		if (isConnectToRobot) {
			robot.disconnect();
		}
	}
}

package org.elastos.carrier;

import android.util.Log;

import org.elastos.carrier.common.RobotConnector;
import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestContext;
import org.elastos.carrier.common.TestHelper;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.Timeout;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.fail;

@RunWith(JUnit4.class)
public class FriendMessageTest {
	private static final String TAG = "FriendMessageTest";
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer commonSyncher = new Synchronizer();
	private static TestContext context = new TestContext();
	private static final TestHandler handler = new TestHandler(context);
	private static RobotConnector robot;
	private static Carrier carrier;

	@Rule
	public Timeout globalTimeout = Timeout.seconds(300);

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

		@Override
		public void onFriendMessage(Carrier carrier, String from, byte[] message) {
			TestContext.Bundle bundle = mContext.getExtra();
			bundle.setFrom(from);
			bundle.setExtraData(getActualValue(message));

			Log.d(TAG, String.format("Friend message %s ", from));
			commonSyncher.wakeup();
		}
	}

	private static String getActualValue(byte[] data) {
		//The string from robot has '\n', delete it.
		byte[] newArray = new byte[data.length - 1];
		System.arraycopy(data, 0, newArray, 0, data.length - 1);
		return new String(newArray);
	}

	@Test
	public void testSendMessageToFriend() {
		friendConnSyncher.reset();
		commonSyncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));
			String out = "message-test";

			carrier.sendFriendMessage(robot.getNodeid(), out);

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 1);
			assertEquals(out, getActualValue(args[0].getBytes()));
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testReceiveMessageFromFriend() {
		try {
			friendConnSyncher.reset();
			commonSyncher.reset();

			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));

			String msg = "message-test";
			assertTrue(robot.writeCmd(String.format("fmsg %s %s", carrier.getUserId(), msg)));

			// wait for message from robot.
			commonSyncher.await();

			TestContext.Bundle bundle = context.getExtra();
			assertEquals(robot.getNodeid(), bundle.getFrom());
			assertEquals(msg, bundle.getExtraData().toString());
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testSendMessageToStranger() {
		try {
			TestHelper.removeFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context);
			assertFalse(carrier.isFriend(robot.getNodeid()));
			String msg = "test-message";
			carrier.sendFriendMessage(robot.getNodeid(), msg);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			assertEquals(0x8100000A, e.getErrorCode());
		}
	}

	@Test
	public void testSendMessageToSelf() {
		try {
			String msg = "test-message";
			carrier.sendFriendMessage(carrier.getUserId(), msg);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			assertEquals(0x81000001, e.getErrorCode());
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
			Carrier.initializeInstance(options, handler);
			carrier = Carrier.getInstance();
			carrier.start(0);
			synchronized (carrier) {
				carrier.wait();
			}
			Log.i(TAG, "Carrier node is ready now");
		}
		catch (CarrierException | InterruptedException e) {
			e.printStackTrace();
			Log.e(TAG, "Carrier node start failed, abort this test.");
		}
	}

	@AfterClass
	public static void tearDown() {
		carrier.kill();
		if (isConnectToRobot) {
			robot.disconnect();
		}
	}

	@Before
	public void setUpCase() {
		robot.clearSocketBuffer();
	}
}

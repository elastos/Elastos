package org.elastos.carrier;

import android.util.Log;
import java.util.Date;

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
public class FriendOfflineMessageTest {
	private static final String TAG = "FriendOfflineMessageTest";
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer commonSyncher = new Synchronizer();
	private static TestContext context = new TestContext();
	private static final TestHandler handler = new TestHandler(context);
	private static RobotConnector robot;
	private static Carrier carrier;

	@Rule
	public Timeout globalTimeout = Timeout.seconds(600);

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
		public void onFriendMessage(Carrier carrier, String from, byte[] message, Date date, boolean isOffline) {
			TestContext.Bundle bundle = mContext.getExtra();
			bundle.setFrom(from);
			bundle.setExtraData(new String(message));

			Log.d(TAG, String.format("Friend message %s ", from));
			commonSyncher.wakeup();
		}
	}

	public void sendOfflineMsgToFriend(int count, int timeout) {
		friendConnSyncher.reset();
		commonSyncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));

			assertTrue(robot.writeCmd("killnode"));
			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("killnode", args[0]);
			assertEquals("success", args[1]);

			friendConnSyncher.await();

			TestContext.Bundle bundle = context.getExtra();
			ConnectionStatus robotConnectionStatus = bundle.getRobotConnectionStatus();
			assertTrue(robotConnectionStatus == ConnectionStatus.Disconnected);

			String timestamp = String.valueOf(System.currentTimeMillis() / 1000);
			assertTrue(robot.writeCmd(String.format("setmsgheader %s:", timestamp)));

			args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("setmsgheader", args[0]);
			assertEquals("success", args[1]);

			int i;
			String out = null;
			for (i = 0; i < count; i++) {
				out = timestamp + ":" + String.valueOf((count > 1) ? (i + 1) : i);
				boolean isOnline = carrier.sendFriendMessage(robot.getNodeid(), out);
				assertFalse(isOnline);
			}

			if (count > 1)
				assertTrue(robot.writeCmd(String.format("restartnode %d %d", timeout, count)));
			else
				assertTrue(robot.writeCmd(String.format("restartnode %d", timeout)));

			friendConnSyncher.await();

			robotConnectionStatus = bundle.getRobotConnectionStatus();
			assertTrue(robotConnectionStatus == ConnectionStatus.Connected);

			args = robot.readAck();
			assertTrue(args != null && args.length == 3);
			assertEquals("ready", args[0]);
			assertEquals(robot.getNodeid(), args[1]);
			assertEquals(robot.getAddress(), args[2]);

			if (count > 1) {
				int recvCount = 0;

				args = robot.readAck();
				assertTrue(args != null && args.length == 1);
				recvCount = Integer.parseInt(args[0]);
				assertEquals(count, recvCount);
			} else {
				args = robot.readAck();
				assertTrue(args != null && args.length == 1);
				assertEquals(out, args[0]);
			}
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testSendOfflineMsgToFriend()
	{
		sendOfflineMsgToFriend(1, 900);
	}

	@Test
	public void testSendOfflineMsgsToFriend()
	{
		sendOfflineMsgToFriend(10, 900);
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

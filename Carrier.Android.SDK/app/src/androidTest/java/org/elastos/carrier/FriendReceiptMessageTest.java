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

import java.nio.ByteBuffer;
import java.util.Date;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.fail;

@RunWith(JUnit4.class)
public class FriendReceiptMessageTest {
	private class Extra {
		Synchronizer syncher = new Synchronizer();
		long msgid;
		ReceiptState state;
	};

	private static final String TAG = "FriendReceiptMessageTest";
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer commonSyncher = new Synchronizer();
	private static TestContext context = new TestContext();
	private static final TestHandler handler = new TestHandler(context);
	private static RobotConnector robot;
	private static Carrier carrier;
	private Extra extra = new Extra();

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

	@Test
	public void testRequestFriendByExpress() {
		Log.i("elastos", "Testing RequestFriendByExpress");
		commonSyncher.reset();
		friendConnSyncher.reset();
		extra.syncher.reset();

		try {
			TestHelper.removeFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context);
			assertFalse(carrier.isFriend(robot.getNodeid()));

			Thread.sleep(2000);
			killNode();
			Thread.sleep(2000);

			carrier.addFriend(robot.getAddress(), "auto-reply");

			startNode();

			String[] args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("fadd", args[0]);
			assertEquals("succeeded", args[1]);

			TestContext.Bundle bundle = context.getExtra();
			if (!bundle.getRobotOnline()) {
				friendConnSyncher.await();
			}
		}
		catch (Exception e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testSendMessageWithReceipt() {
		Log.i("elastos", "Testing SendMessageWithReceipt");
		friendConnSyncher.reset();
		commonSyncher.reset();
		extra.syncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));
			String msg = "message-test";

			long msgid = carrier.sendFriendMessage(robot.getNodeid(), msg,
					new FriendMessageReceiptHandler() {
						@Override
						public void onReceipt(long messageid, ReceiptState state) {
							extra.msgid = messageid;
							extra.state = state;
							extra.syncher.wakeup();
						}
					});
			assert(msgid > 0);

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 1);
			assertEquals(msg, args[0]);

			extra.syncher.await();
			assertEquals(extra.msgid, msgid);
			assertEquals(extra.state, ReceiptState.ReceiptByFriend);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testSendBulkMsgWithReceipt() {
		Log.i("elastos", "Testing SendBulkMsgWithReceipt");
		friendConnSyncher.reset();
		commonSyncher.reset();
		extra.syncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));
			int datalen = 2048;
			StringBuffer sb = new StringBuffer();
			for(int idx = 0; idx < datalen - 5; idx++) {
				sb.append('0' + (idx % 8));
			}
			sb.append("end");
			String data = sb.toString();

			long msgid = carrier.sendFriendMessage(robot.getNodeid(), data,
					new FriendMessageReceiptHandler() {
						@Override
						public void onReceipt(long messageid, ReceiptState state) {
							extra.msgid = messageid;
							extra.state = state;
							extra.syncher.wakeup();
						}
					});
			assert(msgid > 0);

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 1);
			assertEquals(data, args[0]);

			extra.syncher.await();
			assertEquals(extra.msgid, msgid);
			assertEquals(extra.state, ReceiptState.ReceiptByFriend);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testSendOffineMsgWithReceipt() {
	    Log.i("elastos", "Testing SendOffineMsgWithReceipt");
		friendConnSyncher.reset();
		commonSyncher.reset();
		extra.syncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));

			TestContext.Bundle bundle = context.getExtra();
			killNode();
			if (bundle.getRobotOnline()) {
				friendConnSyncher.await();
			}

			String msg = "offline-message-test";
			long msgid = carrier.sendFriendMessage(robot.getNodeid(), msg,
					new FriendMessageReceiptHandler() {
						@Override
						public void onReceipt(long messageid, ReceiptState state) {
							Log.d(TAG, "FriendMessageReceiptHandler.onReceipt() msgid=" + messageid + ", state=" + state);
							extra.msgid = messageid;
							extra.state = state;
							extra.syncher.wakeup();
						}
					});
			assert(msgid > 0);

			extra.syncher.await();
			assertEquals(extra.msgid, msgid);
			assertEquals(extra.state, ReceiptState.DeliveredAsOffline);

			startNode();
			if (!bundle.getRobotOnline()) {
				friendConnSyncher.await();
			}

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 1);
			assertEquals(msg, args[0]);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testSendOfflineBMsgWithReceipt() {
		Log.i("elastos", "Testing SendOfflineBMsgWithReceipt");
		friendConnSyncher.reset();
		commonSyncher.reset();
		extra.syncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));

			TestContext.Bundle bundle = context.getExtra();
			killNode();
			if (bundle.getRobotOnline()) {
				friendConnSyncher.await();
			}

			int datalen = 2048;
			StringBuffer sb = new StringBuffer();
			for(int idx = 0; idx < datalen - 5; idx++) {
				sb.append('0' + (idx % 8));
			}
			sb.append("end");
			String data = sb.toString();

			long msgid = carrier.sendFriendMessage(robot.getNodeid(), data,
					new FriendMessageReceiptHandler() {
						@Override
						public void onReceipt(long messageid, ReceiptState state) {
							extra.msgid = messageid;
							extra.state = state;
							extra.syncher.wakeup();
						}
					});
			assert(msgid > 0);

			startNode();
			if (!bundle.getRobotOnline()) {
				friendConnSyncher.await();
			}

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 1);
			assertEquals(data, args[0]);

			extra.syncher.await();
			assertEquals(extra.msgid, msgid);
			assertEquals(extra.state, ReceiptState.DeliveredAsOffline);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testSendEdgeMsgWithReceipt() {
		Log.i("elastos", "Testing SendEdgeMsgWithReceipt");
		friendConnSyncher.reset();
		commonSyncher.reset();
		extra.syncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));

			TestContext.Bundle bundle = context.getExtra();
			killNode();

			String msg = "edge-message-test";
			long msgid = carrier.sendFriendMessage(robot.getNodeid(), msg,
					new FriendMessageReceiptHandler() {
						@Override
						public void onReceipt(long messageid, ReceiptState state) {
							Log.d(TAG, "FriendMessageReceiptHandler.onReceipt() msgid=" + messageid + ", state=" + state);
							extra.msgid = messageid;
							extra.state = state;
							extra.syncher.wakeup();
						}
					});
			assert(msgid > 0);

			extra.syncher.await();
			assertEquals(extra.msgid, msgid);
			assertEquals(extra.state, ReceiptState.DeliveredAsOffline);

			startNode();
			if (!bundle.getRobotOnline()) {
				friendConnSyncher.await();
			}

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 1);
			assertEquals(msg, args[0]);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
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

	private void startNode() {
		assertTrue(robot.writeCmd(String.format("restartnode %d", 10000)));

		String[] args = robot.readAck();
		assertTrue(args != null && args.length > 1);
		assertEquals("ready", args[0]);
	}

	private void killNode() {
		assertTrue(robot.writeCmd("killnode"));

		String[] args = robot.readAck();
		assertTrue(args != null && args.length == 2);
		assertEquals("killnode", args[0]);
		assertEquals("success", args[1]);
	}
}

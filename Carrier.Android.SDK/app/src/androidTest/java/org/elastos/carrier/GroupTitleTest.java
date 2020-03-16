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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

public class GroupTitleTest {
	private static final String TAG = "GroupTitleTest";
	private static Synchronizer commonSyncher = new Synchronizer();
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer groupSyncher = new Synchronizer();
	private static TestContext context = new TestContext();
	private static TestHandler handler = new TestHandler(context);
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
			mContext.setExtra(bundle);

			Log.d(TAG, "Robot connection status changed -> " + status.toString());
			friendConnSyncher.wakeup();
		}

		@Override
		public void onFriendRequest(Carrier carrier, String userId, UserInfo info, String hello) {
			Log.d(TAG, "Received friend request from " + userId + " with hello: " + hello);
			commonSyncher.wakeup();
		}

		@Override
		public void onFriendAdded(Carrier carrier, FriendInfo info) {
			commonSyncher.wakeup();
			Log.d(TAG, String.format("Friend %s added", info.getUserId()));
		}

		@Override
		public void onFriendRemoved(Carrier carrier, String friendId) {
			commonSyncher.wakeup();
			Log.d(TAG, String.format("Friend %s removed", friendId));
		}

		@Override
		public void onGroupInvite(Carrier carrier, String from, byte[] cookie) {
			commonSyncher.wakeup();
			Log.d(TAG, String.format("Group invite from: %s", from));
		}

		@Override
		public void onGroupConnected(Group group) {
			Log.d(TAG, "onGroupConnected");
		}

		@Override
		public void onGroupMessage(Group group, String from, byte[] message) {
			Log.d(TAG, String.format("Group Message from: %s, message:%s", from, (new String(message))));
		}

		@Override
		public void onGroupTitle(Group group, String from, String title) {
			Log.d(TAG, String.format("Group Title from: %s, title:%s", from, title));
		}

		@Override
		public void onPeerName(Group group, String peerId, String peerName) {
			Log.d(TAG, String.format("Peer Name peerId:%s, title:%s", peerId, peerName));
		}

		@Override
		public void onPeerListChanged(Group group) {
			groupSyncher.wakeup();
			Log.d(TAG, "onPeerListChanged");
		}
	}

	@Test
	public void testGroupSetTitle() {
		try {
			commonSyncher.reset();
			friendConnSyncher.reset();
			groupSyncher.reset();

			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue((carrier.isFriend(robot.getNodeid())));

			Group group = carrier.newGroup();
			assertNotNull(group);

			group.invite(robot.getNodeid());

			// wait until robot having received the invitation
			String[] args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("ginvite", args[0]);
			assertEquals("received", args[1]);

			assertTrue(robot.writeCmd("gjoin"));

			// wait until robot having joined in the group
			args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("gjoin", args[0]);
			assertEquals("succeeded", args[1]);

			// wait until onPeerListChanged callback invoked
			groupSyncher.await();

			//Short title
			String shortTitle = "S";
			group.setTitle(shortTitle);
			String title = group.getTitle();
			assertEquals(shortTitle, title);

			args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("gtitle", args[0]);
			assertEquals(shortTitle, args[1]);

			assertTrue(robot.writeCmd("gleave"));
			// wait until robot having left the group
			args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("gleave", args[0]);
			assertEquals("succeeded", args[1]);

			// wait until onPeerListChanged callback invoked
			groupSyncher.await();

			carrier.groupLeave(group);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testGroupSetMaxLengthTitle() {
		try {
			commonSyncher.reset();
			friendConnSyncher.reset();
			groupSyncher.reset();

			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue((carrier.isFriend(robot.getNodeid())));

			Group group = carrier.newGroup();
			assertNotNull(group);

			group.invite(robot.getNodeid());

			// wait until robot having received the invitation
			String[] args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("ginvite", args[0]);
			assertEquals("received", args[1]);

			assertTrue(robot.writeCmd("gjoin"));

			// wait until robot having joined in the group
			args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("gjoin", args[0]);
			assertEquals("succeeded", args[1]);

			// wait until onPeerListChanged callback invoked
			groupSyncher.await();

			//Short title
			final int ELA_MAX_GROUP_TITLE_LEN = 127;
			//Long title
			StringBuilder longTitle = new StringBuilder();
			for (int i = 0; i < ELA_MAX_GROUP_TITLE_LEN; i++) {
				longTitle.append('L');
			}
			group.setTitle(longTitle.toString());
			String title = group.getTitle();
			assertEquals(longTitle.toString(), title);

			args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("gtitle", args[0]);
			assertEquals(longTitle.toString(), args[1]);

			assertTrue(robot.writeCmd("gleave"));
			// wait until robot having left the group
			args = robot.readAck();
			assertEquals(2, args.length);
			assertEquals("gleave", args[0]);
			assertEquals("succeeded", args[1]);

			// wait until onPeerListChanged callback invoked
			groupSyncher.await();

			carrier.groupLeave(group);
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
}

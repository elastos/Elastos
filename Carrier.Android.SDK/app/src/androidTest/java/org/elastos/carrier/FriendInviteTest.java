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
public class FriendInviteTest {
	private static final String TAG = "FriendInviteTest";

	private static TestContext context = new TestContext();
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer commonSyncher = new Synchronizer();
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
		public void onFriendInviteRequest(Carrier carrier, String from, String data) {
			TestContext.Bundle bundle = mContext.getExtra();
			bundle.setFrom(from);
			LocalData localData = (LocalData)bundle.getExtraData();
			if (localData == null) {
				localData = new LocalData();
			}
			localData.data = data;
			bundle.setExtraData(localData);
			Log.d(TAG, String.format("Friend [%s] invite info [%s]", from, data));

			commonSyncher.wakeup();
		}
	}

	static class LocalData {
		private int status = 0;
		private String reason = null;
		private String data = null;
	}

	class InviteResposeHandler implements FriendInviteResponseHandler {
		TestContext mContext;
		InviteResposeHandler(TestContext context) {
			mContext = context;
		}

		public void onReceived(String from, int status, String reason, String data) {
			TestContext.Bundle bundle = mContext.getExtra();
			LocalData localData = (LocalData)bundle.getExtraData();
			if (localData == null) {
				localData = new LocalData();
			}
			localData.status = status;
			localData.reason = reason;
			localData.data = data;
			bundle.setExtraData(localData);
			Log.d(TAG, String.format("onReceived from [%s], data is [%s]", from, data));

			commonSyncher.wakeup();
		}
	}

	@Test
	public void testFriendInviteConfirm() {
		try {
			friendConnSyncher.reset();
			commonSyncher.reset();

			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue((carrier.isFriend(robot.getNodeid())));
			String hello = "hello";
			InviteResposeHandler h = new InviteResposeHandler(context);
			carrier.inviteFriend(robot.getNodeid(), hello, h);
			String[] args = robot.readAck();
			if (args == null || args.length != 2) {
				fail();
			}
			assertEquals("data", args[0]);
			assertEquals(hello, args[1]);

			String invite_rsp_data = "invitation-confirmed";
			assertTrue(robot.writeCmd(String.format("freplyinvite %s confirm %s", carrier.getUserId(),
				invite_rsp_data)));

			// wait for invite response callback invoked.
			commonSyncher.await();

			TestContext.Bundle bundle = context.getExtra();
			LocalData localData = (LocalData) bundle.getExtraData();
			assertEquals(robot.getNodeid(), bundle.getFrom());
			assertEquals(0, localData.status);
			assertTrue(localData.reason == null || localData.reason.isEmpty());
			assertEquals(invite_rsp_data, localData.data);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testFriendInviteReject() {
		try {
			friendConnSyncher.reset();
			commonSyncher.reset();

			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue((carrier.isFriend(robot.getNodeid())));

			String hello = "hello";
			InviteResposeHandler h = new InviteResposeHandler(context);
			carrier.inviteFriend(robot.getNodeid(), hello, h);
			String[] args = robot.readAck();
			if (args == null || args.length != 2) {
				fail();
			}
			assertEquals("data", args[0]);
			assertEquals(hello, args[1]);

			String reason = "unknown-error";
			assertTrue(robot.writeCmd(String.format("freplyinvite %s refuse %s", carrier.getUserId(),
				reason)));

			// wait for invite response callback invoked.
			commonSyncher.await();

			TestContext.Bundle bundle = context.getExtra();
			LocalData localData = (LocalData) bundle.getExtraData();
			assertEquals(robot.getNodeid(), bundle.getFrom());
			assertTrue(localData.status != 0);
			assertEquals(reason, localData.reason);
			assertTrue(localData.data == null || localData.data.isEmpty());
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testFriendInviteStranger() {
		try {
			friendConnSyncher.reset();
			commonSyncher.reset();

			assertTrue(TestHelper.removeFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertFalse((carrier.isFriend(robot.getNodeid())));

			String hello = "hello";
			InviteResposeHandler h = new InviteResposeHandler(context);
			carrier.inviteFriend(robot.getNodeid(), hello, h);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			assertEquals(0x8100000A, e.getErrorCode());
		}
		catch (Exception e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testFriendInviteSelf() {
		try {
			friendConnSyncher.reset();
			commonSyncher.reset();

			String hello = "hello";
			InviteResposeHandler h = new InviteResposeHandler(context);
			carrier.inviteFriend(carrier.getUserId(), hello, h);
		}
		catch (CarrierException e) {
			e.printStackTrace();
			assertEquals(0x8100000A, e.getErrorCode());
		}
		catch (Exception e) {
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

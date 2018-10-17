package org.elastos.carrier;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.elastos.carrier.robot.RobotProxy;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.exceptions.CarrierException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class FriendInviteTest {
	private static final String TAG = "FriendInviteTest";
	private static Carrier carrierInst;
	private static TestOptions options;
	private static TestHandler handler;
	private static RobotProxy robotProxy;
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

		String from;
		String msgBody;
		ConnectionStatus friendStatus;

		@Override
		public void onReady(Carrier carrier) {
			synch.wakeup();
		}

		@Override
		public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {
			from = friendId;
			friendStatus = status;
			if (status == ConnectionStatus.Connected)
				synch.wakeup();
		}
	}

	static class InviteResponseHandler implements FriendInviteResponseHandler {
		Synchronizer synch = new Synchronizer();

		String from;
		int status = -1;
		String data;

		public void onReceived(String from, int status, String reason, String data) {
			this.from = from;
			this.status = status;
			this.data = data;
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

	@BeforeClass
	public static void setUp() {
		options = new TestOptions(getAppPath());
		handler = new TestHandler();

		try {
			TestReceiver receiver = new TestReceiver();
			robotProxy = RobotProxy.getRobot(getAppContext());
			robotProxy.bindRobot(receiver);
			receiver.synch.await();

			Carrier.initializeInstance(options, handler);
			carrierInst = Carrier.getInstance();
			carrierInst.start(1000);
			handler.synch.await();
		} catch (CarrierException e) {
			e.printStackTrace();
		}
	}

	@AfterClass
	public static void tearDown() {
		try {
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
	public void testInviteFriend() {
		makeFriendAnyWay();

		try {
			InviteResponseHandler handler = new InviteResponseHandler();
			String reqData = "Invite Friend";

			carrierInst.inviteFriend(robotId, reqData, handler);
			handler.synch.await();

			assertEquals(handler.from, robotId);
			assertEquals(handler.status, 0);
			assertEquals(handler.data, reqData);
		} catch (CarrierException e) {
			e.printStackTrace();
			String error = String.format("error: 0x%x", e.getErrorCode());
			Log.e(TAG, "error: " + error);
			assertTrue(false);
		}
	}
}

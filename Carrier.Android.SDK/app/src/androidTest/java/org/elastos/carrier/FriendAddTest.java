package org.elastos.carrier;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import android.content.Context;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.elastos.carrier.robot.RobotProxy;
import org.elastos.carrier.robot.RobotService;
import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.ElastosException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class FriendAddTest {
	private static final String TAG = "FriendAddTest";
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
		return getAppContext().getFilesDir().getAbsolutePath() + "-1";
	}

	static class TestHandler extends AbstractCarrierHandler {
		Synchronizer synch = new Synchronizer();
		String from;
		ConnectionStatus friendStatus;

		public void onReady(Carrier carrier) {
			synch.wakeup();
		}

		public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {
			Log.i(TAG, "friendid:" + friendId + "connection changed to: " + status);
			from = friendId;
			friendStatus = status;
			if (friendStatus == ConnectionStatus.Connected)
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

			carrierInst = Carrier.getInstance(options, handler);
			carrierInst.start(1000);
			handler.synch.await();

			Log.i(TAG, "carrier client is ready now");

		} catch (ElastosException e) {
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@AfterClass
	public static void tearDown() {
		try {
			robotProxy.unbindRobot();
			carrierInst.kill();
		}catch(Exception e) {
			e.printStackTrace();
		}
	}

	private void removeFriendAnyWay() {
		try {
			if (carrierInst.isFriend(robotId))
				carrierInst.removeFriend(robotId);

			robotProxy.tellRobotRemoveFriend(carrierInst.getUserId());
		} catch (ElastosException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}

	private void makeFriendAnyWay() {
		try {
			if (!carrierInst.isFriend(robotId))
				carrierInst.addFriend(robotAddress, "auto-accepted");

			handler.synch.await(); // for friend connected
		} catch (ElastosException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}

	@Test
	public void testAddedFriendAndAccepted() {
		removeFriendAnyWay();

		try {
			Thread.sleep(100);
		} catch (Exception e) {
		}

		try {
			assertEquals(carrierInst.isFriend(robotId), false);

			carrierInst.addFriend(robotAddress, "hello");
			robotProxy.waitForRequestArrival();
			robotProxy.tellRobotAcceptFriend(carrierInst.getUserId());

			handler.synch.await(); // for friend connection.;
			Log.i(TAG, "Robot connected");

			assertEquals(handler.from, robotId);
			assertEquals(handler.friendStatus, ConnectionStatus.Connected);
			assertEquals(carrierInst.isFriend(handler.from), true);
		} catch (ElastosException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}

	@Test
	public void testAlreadyBeFriend() {
		makeFriendAnyWay();

		try {
			carrierInst.addFriend(robotAddress, "hello");
		} catch (ElastosException e) {
			assertEquals(e.getErrorCode(), 0x8100000c);
			assertTrue(true);
		}
	}
}

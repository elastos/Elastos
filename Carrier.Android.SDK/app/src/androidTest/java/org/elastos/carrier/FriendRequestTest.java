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
import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.ElastosException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class FriendRequestTest {
	private static final String TAG = "FriendRequestTest";
	private static Carrier carrierInst;
	private static TestOptions options;
	private static TestHandler handler;
	private static RobotProxy robotProxy;
	private static String robotId;

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

		public void onFriendRemoved(Carrier carrier, String friendId) {
			synch.wakeup();
		}

		public void onFriendAdded(Carrier carrier, FriendInfo info) {
			synch.wakeup();
		}
	}

	static class TestReceiver implements RobotProxy.RobotIdReceiver {
		private Synchronizer synch = new Synchronizer();

		@Override
		public void onReceived(String userId) {
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

	private void removeFriendAnyWay(String userId) {
		try {
			if (carrierInst.isFriend(userId)) {
				carrierInst.removeFriend(userId);
				handler.synch.await();
			}
		} catch (ElastosException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}

	private void makeFriendAnyWay(String userId) {
		try {
			if (!carrierInst.isFriend(userId)) {
				carrierInst.addFriend(robotId, "auto confirmed");
				handler.synch.await(); // for friend request reply.
				handler.synch.await(); // for friend added.
			}
		} catch (ElastosException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}

	/*
	@Test
	public void testConfirmFriendRequest() {
		removeFriendAnyWay(robotId);

		try {
			carrierInst.friendRequest(robotId, "hello");
			robotProxy.waitForRequestArrival();
			robotProxy.tellRobotConfirmFriendRequest(carrierInst.getUserId(), "2017-12-12,12:12");

			handler.synch.await(); // for response;
			assertEquals(handler.from, robotId);
			assertEquals(handler.status, 0);
			assertNull(handler.reason);
			assertEquals(handler.entrusted, false);

			handler.synch.await(); // for friend added.
			assertTrue(whisperInst.isFriend(robotId));
		} catch (WhisperException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}

	@Test
	public void testRejectFriendRequest() {
		removeFriendAnyWay(robotId);

		try {
			whisperInst.friendRequest(robotId, "hello");
			robotProxy.waitForRequestArrival();
			robotProxy.tellRobotRejectFriendRequest(whisperInst.getUserId(), "test error");
			handler.synch.await();

			assertEquals(handler.from, robotId);
			assertEquals(handler.status, -1);
			assertNotNull(handler.reason);
			assertNull(handler.expire);
			assertEquals(handler.entrusted, false);
		} catch (WhisperException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}
	*/

	@Test
	public void testAlreadyBeFriend() {
		makeFriendAnyWay(robotId);

		try {
			carrierInst.addFriend(robotId, "hello");
		} catch (ElastosException e) {
			assertEquals(e.getErrorCode(), 0x8100000c);
			assertTrue(true);
		}
	}
}

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
import org.elastos.carrier.exceptions.ElastosException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class FriendMessageTest {
	private static final String TAG = "FriendRequestTest";
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

		public void onReady(Carrier carrier) {
			synch.wakeup();
		}

		public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {
			from = friendId;
			friendStatus = status;
			if (status == ConnectionStatus.Connected)
				synch.wakeup();
		}

		public void onFriendMessage(Carrier whisper, String friendId, String message) {
			msgBody = message;
			from = friendId;
			Log.i(TAG, "Get a message (" + message + ") from (" + friendId + ")");
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
		} catch (ElastosException e) {
			e.printStackTrace();
		} catch (Exception e) {
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

	@Test
	public void testSendMeMessage() {
		try {
			String hello = "test send me message";
			carrierInst.sendFriendMessage(carrierInst.getUserId(), hello);
			handler.synch.await();

			assertEquals(handler.from, carrierInst.getUserId());
			assertEquals(handler.msgBody, hello);

		} catch (Exception e) {
			e.printStackTrace();
			assertTrue(true);
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

	@Test
	public void testSendStrangeAMessage() {
		removeFriendAnyWay();

		try {
			String hello = "test send friend message";
			carrierInst.sendFriendMessage(robotId, hello);
		} catch (ElastosException e) {
			assertEquals(e.getErrorCode(), 0x8100000b);
			Log.i(TAG, "errcode: " +  e.getErrorCode());
			assertTrue(true);
		}
	}

	private void makeFriendAnyWay() {
		try {
			if (!carrierInst.isFriend(robotId))
				carrierInst.addFriend(robotAddress, "auto-accepted");

			handler.synch.await(); // for friend added.
		} catch (ElastosException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}

	@Test
	public void testSendFriendMessage() {
		makeFriendAnyWay();

		try {
			String hello = "test send friend message";
			carrierInst.sendFriendMessage(robotId, hello);
			robotProxy.waitForMessageArrival();

			robotProxy.tellRobotSendMessage(carrierInst.getUserId(), hello);
			handler.synch.await();

			Log.i(TAG, " hello : " + hello);
			Log.i(TAG, " msgBody: " + handler.msgBody);

			assertEquals(handler.from, robotId);
			//assertEquals(handler.msgBody, hello); //TODO
		} catch (ElastosException e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}
}

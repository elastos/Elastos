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
import org.elastos.carrier.common.TestHelper.ITestChannelExecutor;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.Timeout;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
public class RequestReplyTest {
	private static final String TAG = "RequestReplyTest";
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer commonSyncher = new Synchronizer();
	private static TestContext context = new TestContext();
	private static TestHandler handler = new TestHandler(context);
	private static RobotConnector robot;
	private static Carrier carrier;
	private static Manager sessionManager;
	private static final SessionManagerHandler sessionHandler = new SessionManagerHandler();
	private static final TestStreamHandler streamHandler = new TestStreamHandler();
	private static Session session;
	private static Stream stream;

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
	}

	static class SessionManagerHandler implements ManagerHandler {
		@Override
		public void onSessionRequest(Carrier carrier, String from, String sdp) {
			LocalData data = (LocalData)context.getExtra().getExtraData();
			if (data == null) {
				data = new LocalData();
				context.getExtra().setExtraData(data);
			}
			data.mRequestReceived = true;
			data.mSdp = sdp;

			Log.d(TAG, String.format("Session Request from %s", from));
			synchronized (sessionHandler) {
				sessionHandler.notify();
			}
		}
	}

	private static class LocalData {
		private String mSdp = null;
		private boolean mRequestReceived = false;
		private StreamState mState;
		private int mCompleteStatus = 0;
	}

	static class TestStreamHandler implements StreamHandler {
		@Override
		public void onStateChanged(Stream stream, StreamState state) {
			LocalData data = (LocalData)context.getExtra().getExtraData();
			if (data == null) {
				data = new LocalData();
				context.getExtra().setExtraData(data);
			}
			data.mState = state;

			Log.d(TAG, "onStateChanged state="+state);
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public void onStreamData(Stream stream, byte[] data) {
			Log.d(TAG, "onStreamData data="+(new String(data)));
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public boolean onChannelOpen(Stream stream, int channel, String cookie) {
			Log.d(TAG, "onChannelOpen cookie="+cookie);
			synchronized (this) {
				this.notify();
			}
			return true;
		}

		@Override
		public void onChannelOpened(Stream stream, int channel) {
			Log.d(TAG, "onChannelOpened channel="+channel);
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public void onChannelClose(Stream stream, int channel, CloseReason reason) {
			Log.d(TAG, "onChannelClose channel="+channel);
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public boolean onChannelData(Stream stream, int channel, byte[] data) {
			Log.d(TAG, "onChannelData channel="+channel+", data="+(new String(data)));
			synchronized (this) {
				this.notify();
			}
			return true;
		}

		@Override
		public void onChannelPending(Stream stream, int channel) {
			Log.d(TAG, "onChannelPending channel="+channel);
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public void onChannelResume(Stream stream, int channel) {
			Log.d(TAG, "onChannelResume channel="+channel);
			synchronized (this) {
				this.notify();
			}
		}
	}

	static class TestSessionRequestCompleteHandler implements SessionRequestCompleteHandler {
		@Override
		public void onCompletion(Session session, int status, String reason, String sdp) {
			Log.d(TAG, String.format("Session complete, status: %d, reason: %s", status, reason));
			LocalData data = (LocalData)context.getExtra().getExtraData();
			data.mCompleteStatus = status;

			if (status == 0) {
				try {
					session.start(sdp);
				}
				catch (CarrierException e) {
					e.printStackTrace();
					fail();
				}
			}

			synchronized (this) {
				notify();
			}
		}
	}

	@Test
	public void testSessionRequest() {
		testStreamScheme(StreamType.Text, 0, null);
	}

	@Test
	public void testSessionReply() {
		testStreamReplyScheme(StreamType.Text, 0, null);
	}

	private static void testStreamReplyScheme(StreamType stream_type, int stream_options
				, ITestChannelExecutor channelExecutor)
	{
		friendConnSyncher.reset();
		commonSyncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));

			assertTrue(robot.writeCmd("sinit"));

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("sinit", args[0]);
			assertEquals("success", args[1]);

			assertTrue(robot.writeCmd(String.format("srequest %s %d", carrier.getUserId(), stream_options)));

			args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("srequest", args[0]);
			assertEquals("success", args[1]);

			//For session request callback
			synchronized (sessionHandler) {
				sessionHandler.wait(5000);
			}

			LocalData data = (LocalData)context.getExtra().getExtraData();
			assertEquals(0, data.mCompleteStatus);
			assertTrue(data.mRequestReceived);
			assertTrue(data.mSdp.length() > 0);

			session = sessionManager.newSession(robot.getNodeid());
			assertNotNull(session);

			stream = session.addStream(stream_type, stream_options, streamHandler);
			assertNotNull(stream);

			//Stream initialized
			synchronized (streamHandler) {
				streamHandler.wait(1000);
			}

			data = (LocalData)context.getExtra().getExtraData();
			assertEquals(StreamState.Initialized, data.mState);

			session.replyRequest(0, null);

			//Stream initialized
			synchronized (streamHandler) {
				streamHandler.wait(1000);
			}
			data = (LocalData)context.getExtra().getExtraData();
			assertEquals(StreamState.TransportReady, data.mState);

			session.start(data.mSdp);

			//Stream connecting
			synchronized (streamHandler) {
				streamHandler.wait(1000);
			}

			if ((!data.mState.equals(StreamState.Connecting)) && (!data.mState.equals(StreamState.Connected))) {
				// if error, consume ctrl acknowlege from robot.
				args = robot.readAck();
			}

			assertTrue(data.mState == StreamState.Connecting || data.mState == StreamState.Connected);
			args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("sconnect", args[0]);
			assertEquals("success", args[1]);

			//Stream connected
			synchronized (streamHandler) {
				streamHandler.wait(1000);
			}

			assertEquals(StreamState.Connected, data.mState);

			if (channelExecutor != null) {
				channelExecutor.executor();
			}

			session.removeStream(stream);
			session.close();

			data = (LocalData)context.getExtra().getExtraData();
			assertEquals(StreamState.Closed, data.mState);

			robot.writeCmd("sfree");
		}
		catch (CarrierException | InterruptedException e) {
			e.printStackTrace();
		}
	}

	private void testStreamScheme(StreamType stream_type, int stream_options, ITestChannelExecutor channelExecutor)
	{
		friendConnSyncher.reset();
		commonSyncher.reset();

		try {
			assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
			assertTrue(carrier.isFriend(robot.getNodeid()));
			assertTrue(robot.writeCmd("sinit"));

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("sinit", args[0]);
			assertEquals("success", args[1]);

			session = sessionManager.newSession(robot.getNodeid());
			assertNotNull(session);

			stream = session.addStream(stream_type, stream_options, streamHandler);
			assertNotNull(stream);

			//Stream initialized
			synchronized (streamHandler) {
				streamHandler.wait();
			}

			LocalData data = (LocalData)context.getExtra().getExtraData();
			assertEquals(StreamState.Initialized, data.mState);

			TestSessionRequestCompleteHandler completeHandler = new TestSessionRequestCompleteHandler();
			session.request(completeHandler);

			//Stream initialized
			synchronized (streamHandler) {
				streamHandler.wait(1000);
			}
			data = (LocalData)context.getExtra().getExtraData();
			assertEquals(StreamState.TransportReady, data.mState);

			args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("srequest", args[0]);
			assertEquals("received", args[1]);

			assertTrue(robot.writeCmd(String.format("sreply confirm %d %d", stream_type.value(), stream_options)));

			//Stream initialized
			synchronized (completeHandler) {
				completeHandler.wait();
			}

			data = (LocalData)context.getExtra().getExtraData();
			assertEquals(0, data.mCompleteStatus);

			args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("sreply", args[0]);
			assertEquals("success", args[1]);

			//Stream connecting
			synchronized (streamHandler) {
				streamHandler.wait(1000);
			}

			data = (LocalData)context.getExtra().getExtraData();
			if ((!data.mState.equals(StreamState.Connecting)) && (!data.mState.equals(StreamState.Connected))) {
				// if error, consume ctrl acknowlege from robot.
				args = robot.readAck();
			}

			// Stream 'connecting' state is a transient state.
			assertTrue(data.mState == StreamState.Connecting || data.mState == StreamState.Connected);
			args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("sconnect", args[0]);
			assertEquals("success", args[1]);

			//Stream connected
			synchronized (streamHandler) {
				streamHandler.wait(1000);
			}

			assertEquals(StreamState.Connected, data.mState);

			if (channelExecutor != null) {
				channelExecutor.executor();
			}

			session.removeStream(stream);
			session.close();

			data = (LocalData)context.getExtra().getExtraData();
			assertEquals(StreamState.Closed, data.mState);

			robot.writeCmd("sfree");
		}
		catch (CarrierException | InterruptedException e) {
			e.printStackTrace();
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

    @Before
    public void setUpCase() {
        robot.clearSocketBuffer();
    }
}

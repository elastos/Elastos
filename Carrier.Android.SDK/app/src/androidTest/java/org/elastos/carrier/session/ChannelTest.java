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

import java.util.Random;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
public class ChannelTest {
	private static final String TAG = "ChannelTest";
	private static Synchronizer friendConnSyncher = new Synchronizer();
	private static Synchronizer commonSyncher = new Synchronizer();
	private static TestContext context = new TestContext();
	private static TestHandler handler = new TestHandler(context);
	private static RobotConnector robot;
	private static Carrier carrier;
	private static Manager sessionManager;
	private static final SessionManagerHandler sessionHandler = new SessionManagerHandler();
	private static final TestStreamHandler streamHandler = new TestStreamHandler();
	private static Stream stream;
	private static final TestSessionRequestCompleteHandler completeHandler
				= new TestSessionRequestCompleteHandler();

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

			Log.d(TAG, String.format("Session Request from %s", from));
			synchronized (sessionHandler) {
				sessionHandler.notify();
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

	private static class LocalData {
		private StreamState mState;
		private int mCompleteStatus = 0;
		private int[] mChannelIDs = new int[128];
		private int mChannelIDCount = 0;
		private int[] mChannelErrorStates = new int[128];
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

			Log.d(TAG, "Stream state changed to:"+state);
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public void onStreamData(Stream stream, byte[] data) {
			Log.d(TAG, "Stream received data="+(new String(data)));
		}

		@Override
		public boolean onChannelOpen(Stream stream, int channel, String cookie) {
			Log.d(TAG, "Stream request open new channel: " + channel);
			synchronized (this) {
				this.notify();
			}
			return true;
		}

		@Override
		public void onChannelOpened(Stream stream, int channel) {
			Log.d(TAG, "Opened Channel :"+channel);
			LocalData data = (LocalData)context.getExtra().getExtraData();
			data.mChannelErrorStates[channel - 1] = 0;
			data.mChannelIDs[channel - 1] = channel;
			data.mChannelIDCount ++;
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public void onChannelClose(Stream stream, int channel, CloseReason reason) {
			Log.d(TAG, String.format("Channel %d closeing with %s.", channel, reason.toString()));
			LocalData data = (LocalData)context.getExtra().getExtraData();
			if (reason == CloseReason.Error || reason == CloseReason.Timeout) {
				data.mChannelErrorStates[channel - 1] = 1;
			}

			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public boolean onChannelData(Stream stream, int channel, byte[] data) {
			Log.d(TAG, String.format("Channel [%d] received data [%s]",
				channel, (new String(data))));

			synchronized (this) {
				this.notify();
			}
			return true;
		}

		@Override
		public void onChannelPending(Stream stream, int channel) {
			Log.d(TAG, String.format("Stream channel [%d] pend data.", channel));
			synchronized (this) {
				this.notify();
			}
		}

		@Override
		public void onChannelResume(Stream stream, int channel) {
			Log.d(TAG, String.format("Stream channel [%d] resume data.", channel));
			synchronized (this) {
				this.notify();
			}
		}
	}

	private static int writeDataToChannel(Stream stream, int channel, byte[] data, int len)
	{
		int left;
		int pos = 0;
		int rc;

		left = len;
		while(left > 0) {
			try {
				rc = stream.writeData(channel, data, pos, left);
				if (rc == 0) {
					fail();
				}
				else if (rc < 0) {
					fail();
				}
				else {
					pos += rc;
					left -= rc;
				}
			}
			catch (CarrierException e) {
				int errorCode = e.getErrorCode();
				if (errorCode == 0x81000010) {
					try {
						Thread.sleep(100);
					}
					catch (InterruptedException ie) {
						ie.printStackTrace();
					}
				}
				else {
					Log.d(TAG, String.format("Write channel data failed (0x%x)", errorCode));
					return -1;
				}
			}
		}

		return 0;
	}

	private static void doBulkChannelWrite()
	{
		final int max_data_sz = 1024*1024*10;
		final int min_data_sz = 1024*1024;
		final int max_pkt_sz  = 1900;
		final int min_pkt_sz  = 1024;

		Thread thread = new Thread(new Runnable() {
			@Override
			public void run() {
				int data_sz;
				int pkt_sz;
				long pkt_count;
				int i;
				long duration;
				float speed;

				Random random = new Random();
				data_sz = random.nextInt(max_data_sz - min_data_sz) + min_data_sz;
				pkt_sz  = /*random.nextInt(max_pkt_sz - min_pkt_sz) + */min_pkt_sz;
				pkt_count = data_sz / pkt_sz + 1;

				char[] chars = new char[pkt_sz];
				for (int j = 0; j < pkt_sz; j++) {
					chars[j] = 'D';
				}

				Log.d(TAG, "Begin sending data...");
				LocalData data = (LocalData)context.getExtra().getExtraData();
				Log.d(TAG, String.format("Stream channel %d send %d packets in total and %d bytes per packet.",
						data.mChannelIDs[0], pkt_count, pkt_sz));

				long start = System.currentTimeMillis();
				byte[] charBytes = TestHelper.getBytes(chars);
				for (i = 0; i < pkt_count; i++) {
					int rc = writeDataToChannel(stream, data.mChannelIDs[0], charBytes, charBytes.length);
					if (rc < 0) {
						Log.d(TAG, "stream channel write failed.");
						return;
					}
				}
				long end = System.currentTimeMillis();
				duration = end - start;
				speed = (((float)(pkt_sz * pkt_count) / duration) * 1000) / 1024;

				Log.d(TAG, String.format("Finish! Total %d bytes in %d.%d seconds. %.2f KB/s",
						(pkt_sz * pkt_count), (int)(duration / 1000), (int)(duration % 1000), speed));
			}
		});
		thread.start();

		try {
			thread.join();
		}
		catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	class TestSingleChannelExecutor implements ITestChannelExecutor {
		@Override
		public void executor() {
			/*send command 'copen'*/
			assertTrue(robot.writeCmd("cready2open confirm"));

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("cready2open", args[0]);
			assertEquals("success", args[1]);

			/*open channel*/
			LocalData data = (LocalData)context.getExtra().getExtraData();

			try {
				data.mChannelIDs[0] = stream.openChannel("cookie");
				assertTrue(data.mChannelIDs[0] > 0);

				synchronized (streamHandler) {
					streamHandler.wait(500);
				}
				assertEquals(0, data.mChannelErrorStates[0]);

				/*pend*/
				assertTrue(robot.writeCmd("cpend"));

				args = robot.readAck();
				assertTrue(args != null && args.length == 2);
				assertEquals("cpend", args[0]);
				assertEquals("success", args[1]);

				assertTrue(robot.writeCmd("cresume"));

				args = robot.readAck();
				assertTrue(args != null && args.length == 2);
				assertEquals("cresume", args[0]);
				assertEquals("success", args[1]);

				doBulkChannelWrite();

				stream.closeChannel(data.mChannelIDs[0]);

				data.mChannelIDs[0] = -1;
				synchronized (streamHandler) {
					streamHandler.wait(500);
				}
				assertEquals(0, data.mChannelErrorStates[0]);
			}
			catch (CarrierException | InterruptedException e) {
				e.printStackTrace();
			}
		}
	}

	private void doBulkMultipleChannelsWrite() {
		final int max_data_sz = 1024*1024*50;
		final int min_data_sz = 1024*128;
		final int max_pkt_sz  = 1900;
		final int min_pkt_sz  = 1024;

		Thread thread = new Thread(new Runnable() {
			@Override
			public void run() {
				int data_sz;
				int pkt_sz;
				int pkt_count;
				char[] packet;
				int i;
				int j;
				long duration;
				final int MAX_CHANNEL_COUNT = 128;

				try {
					LocalData data = (LocalData)context.getExtra().getExtraData();
					for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
						data.mChannelIDCount = i + 1;
						data.mChannelIDs[i] = stream.openChannel("cookie");

						if (data.mChannelIDs[i] < 0) {
							Log.d(TAG, String.format("Open new channel %d failed.", i + 1));
							//reclaim opened channels
							for (i = 0; i < data.mChannelIDCount; i++) {
								stream.closeChannel(data.mChannelIDs[i]);
							}
							data.mChannelIDCount = 0;
							return;
						} else {
							// Log.d(TAG, String.format("Open channel [%d] succeeded (id:%d)", i, data.mChannelIDs[i]));
							synchronized (streamHandler) {
								streamHandler.wait(1000);
							}
						}
					}

					Random random = new Random();
					data_sz = /*random.nextInt(max_data_sz - min_data_sz)*/ + min_data_sz;
					pkt_sz  = /*random.nextInt(max_pkt_sz - min_pkt_sz)*/ + min_pkt_sz;
					pkt_count = data_sz / pkt_sz + 1;

					packet = new char[pkt_sz];
					for (int p = 0; p < pkt_sz; p++) {
						packet[p] = 'D';
					}
					byte[] packetBytes = TestHelper.getBytes(packet);

					Log.d(TAG,"Open new 128 channels successfully.");
					Log.d(TAG,"Begin to write data.....");

					long start = System.currentTimeMillis();
					for (i = 0; i < MAX_CHANNEL_COUNT / 2; i++) {
						int rc;
						for (j = 0; j < pkt_count; j++) {
							rc = writeDataToChannel(stream, data.mChannelIDs[i], packetBytes, packetBytes.length);
							if (rc < 0) {
								for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
									stream.closeChannel(data.mChannelIDs[i]);
								}
								return;
							}
						}
					}

					for (j = 0; j < pkt_count; j++) {
						for (i = MAX_CHANNEL_COUNT / 2; i < MAX_CHANNEL_COUNT; i++) {
							int rc = writeDataToChannel(stream, data.mChannelIDs[i], packetBytes, packetBytes.length);
							if (rc < 0) {
								for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
									stream.closeChannel(data.mChannelIDs[i]);
								}
								return;
							}
						}
					}

					long end = System.currentTimeMillis();
					duration = end - start;

					Log.d(TAG, String.format("Finish! Total 128 channel write data bytes in %d.%d seconds.",
							(int)(duration / 1000), (int)(duration % 1000)));

					for (i = 0; i < MAX_CHANNEL_COUNT; i++) {
						stream.closeChannel(data.mChannelIDs[i]);
					}

					data.mChannelIDCount = 0;
				}
				catch (CarrierException | InterruptedException e) {
					e.printStackTrace();
				}
			}
		});
		thread.start();

		try {
			thread.join();
		}
		catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	class TestMultipleChannelExecutor implements ITestChannelExecutor {
		@Override
		public void executor() {
			/*send command 'copen'*/
			assertTrue(robot.writeCmd("cready2open confirm"));

			String[] args = robot.readAck();
			assertTrue(args != null && args.length == 2);
			assertEquals("cready2open", args[0]);
			assertEquals("success", args[1]);

			doBulkMultipleChannelsWrite();
		}
	}

	private void channelBulkWrite(int stream_options)
	{
		TestSingleChannelExecutor executor = new TestSingleChannelExecutor();
		testStreamScheme(StreamType.Text, stream_options, executor);
	}

	private void multipleChannelsBulkWrite(int stream_options)
	{
		TestMultipleChannelExecutor executor = new TestMultipleChannelExecutor();
		testStreamScheme(StreamType.Text, stream_options, executor);
	}

	@Test
	public void testSessionChannel() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;

		channelBulkWrite(stream_options);
	}

	@Test
	public void testSessionChannelPlain() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;
		stream_options |= Stream.PROPERTY_PLAIN;

		channelBulkWrite(stream_options);
	}

	@Test
	public void testSessionChannelReliable() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;
		stream_options |= Stream.PROPERTY_RELIABLE;

		channelBulkWrite(stream_options);
	}

	@Test
	public void testSessionChannelReliablePlain() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;
		stream_options |= Stream.PROPERTY_PLAIN;
		stream_options |= Stream.PROPERTY_RELIABLE;

		channelBulkWrite(stream_options);
	}

	@Test
	public void testSessionMultipleChannels() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;

		multipleChannelsBulkWrite(stream_options);
	}

	@Test
	public void testSessionMultipleChannelsPlain() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;
		stream_options |= Stream.PROPERTY_PLAIN;

		multipleChannelsBulkWrite(stream_options);
	}

	@Test
	public void testSessionMultipleChannelsReliable() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;
		stream_options |= Stream.PROPERTY_RELIABLE;

		multipleChannelsBulkWrite(stream_options);
	}

	@Test
	public void testSessionMultipleChannelsReliablePlain() {
		int stream_options = 0;
		stream_options |= Stream.PROPERTY_MULTIPLEXING;
		stream_options |= Stream.PROPERTY_PLAIN;
		stream_options |= Stream.PROPERTY_RELIABLE;

		multipleChannelsBulkWrite(stream_options);
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

			Session session = sessionManager.newSession(robot.getNodeid());
			assertNotNull(session);

			stream = session.addStream(stream_type, stream_options, streamHandler);
			assertNotNull(stream);

			//Stream initialized
			synchronized (streamHandler) {
				streamHandler.wait();
			}

			LocalData data = (LocalData)context.getExtra().getExtraData();
			assertEquals(StreamState.Initialized, data.mState);

			session.request(completeHandler);

			//Stream TransportReady
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

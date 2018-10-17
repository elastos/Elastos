package org.elastos.carrier.robot;

import android.os.Looper;
import android.util.Log;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Messenger;
import android.os.Message;
import android.os.Handler;
import android.os.Bundle;

import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.common.Synchronizer;

import org.elastos.carrier.AbstractCarrierHandler;
import org.elastos.carrier.UserInfo;
import org.elastos.carrier.Carrier;
import org.elastos.carrier.exceptions.CarrierException;
import org.elastos.carrier.session.AbstractStreamHandler;
import org.elastos.carrier.session.Manager;
import org.elastos.carrier.session.Session;
import org.elastos.carrier.session.ManagerHandler;
import org.elastos.carrier.session.Stream;
import org.elastos.carrier.session.StreamState;
import org.elastos.carrier.session.StreamType;

public class RobotService extends Service {
	private static final String TAG = "RobotService";

	static final int MSG_TEST = 1;
	static final int MSG_REQ_ROBOT_ID = 2;
	static final int MSG_RSP_ROBOT_ID = 3;
	static final int MSG_REQ_FRIEND_ARRIVAL = 4;
	static final int MSG_MESSAGE_ARRIVAL = 5;
	static final int MSG_REMOVE_FRIEND = 6;
	static final int MSG_ACCEPT_FRIEND = 7;
	static final int MSG_SEND_MESSAGE = 8;
	static final int MSG_SESSION_MANAGER_INIT = 9;
	static final int MSG_SESSION_MANAGER_CLEANUP = 10;
	static final int MSG_SESSION_MANAGER_INITIALIZED = 11;
	static final int MSG_SESSION_REQUEST_ARRIVAL = 1;
	static final int MSG_REPLY_SESSION_REQUST_AND_START = 13;
	static final int MSG_SESSION_CONNECTED = 14;

	private Messenger messenger;
	private TestOptions options;
	private Carrier carrierInst;
	private Messenger proxyMsger;

	private Manager sessionMgr;
	private String sessionRequestFrom;
	private String sessionRequestSdp;
	private Session activeSession;
	private Stream  activeStream;

	private String getAppPath() {
		return getFilesDir().getAbsolutePath() + "-robot";
	}

	class TestHandler extends AbstractCarrierHandler {
		private Synchronizer synch = new Synchronizer();

		@Override
		public void onReady(Carrier carrier) {
			synch.wakeup();
		}

		@Override
		public void onFriendRequest(Carrier carrier, String userId, UserInfo info, String hello) {
			try {

				if (hello.equals("auto-accepted")) {
					carrierInst.acceptFriend(userId);
				} else {
					Message msg = Message.obtain(null, RobotService.MSG_REQ_FRIEND_ARRIVAL);
					Bundle data = new Bundle();
					data.putString("from", userId);
					data.putString("hello", hello);
					msg.setData(data);

					proxyMsger.send(msg);
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		@Override
		public void onFriendMessage(Carrier carrier, String from, byte[] message) {
			try {
				Message msg = Message.obtain(null, RobotService.MSG_MESSAGE_ARRIVAL);
				Bundle data = new Bundle();
				data.putString("from", from);
				data.putString("message", new String(message));
				msg.setData(data);

				proxyMsger.send(msg);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		@Override
		public void onFriendInviteRequest(Carrier carrier, String from, String data) {
			try {
				carrierInst.replyFriendInvite(from, 0, null, data);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	class IncomingHandler extends Handler {
		IncomingHandler () {
			super(Looper.getMainLooper());
		}

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case MSG_TEST:
					Log.i(TAG, "Received test message:");
					Log.i(TAG, "hello: " + msg.getData().getString("hello"));
					break;
				case MSG_REQ_ROBOT_ID:
					proxyMsger = msg.replyTo;
					replyRobotId(msg);
					break;
				case MSG_REMOVE_FRIEND:
					removeFriend(msg);
					break;
				case MSG_ACCEPT_FRIEND:
					acceptFriend(msg);
					break;
				case MSG_SEND_MESSAGE:
					sendTestMessage(msg);
					break;
				case MSG_SESSION_MANAGER_INIT:
					initSessionManager(msg);
					break;
				case MSG_SESSION_MANAGER_CLEANUP:
					cleanupSessionManager(msg);
					break;
				case MSG_REPLY_SESSION_REQUST_AND_START:
					replySessionRequestAndStart(msg);
					break;
				default:
					super.handleMessage(msg);
			}
		}
	}

	@Override
	public void onCreate() {
		messenger = new Messenger(new IncomingHandler());
		options = new TestOptions(getAppPath());

		TestHandler handler = new TestHandler();
		try {
			Carrier.initializeInstance(options, handler);
			carrierInst = Carrier.getInstance();
			carrierInst.start(10000);
			handler.synch.await();

			Log.i(TAG, "Carrier instance of robot created");
		} catch (CarrierException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onDestroy() {
		if (carrierInst != null) {
			carrierInst.kill();
			carrierInst = null;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return messenger.getBinder();
	}

	void replyRobotId(Message msg) {
		try {
			Message rsp = Message.obtain(null, RobotService.MSG_RSP_ROBOT_ID);
			Bundle data = new Bundle();
			data.putString("address", carrierInst.getAddress());
			data.putString("robotId", carrierInst.getUserId());
			rsp.setData(data);

			msg.replyTo.send(rsp);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void removeFriend(Message msg) {
		try {
			String friendId = msg.getData().getString("friendId");

			if (carrierInst.isFriend(friendId))
				carrierInst.removeFriend(friendId);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void acceptFriend(Message msg) {
		try {
			String userId = msg.getData().getString("userId");

			if (!carrierInst.isFriend(userId))
				carrierInst.acceptFriend(userId);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void sendTestMessage(Message msg) {
		try {
			String to = msg.getData().getString("to");
			String msgBody = msg.getData().getString("message");

			carrierInst.sendFriendMessage(to, msgBody);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void initSessionManager(Message msg) {
		try {
			sessionMgr = Manager.getInstance(carrierInst, new ManagerHandler() {
				@Override
				public void onSessionRequest(Carrier carrier, String from, String sdp) {
					sessionRequestFrom = from;
					sessionRequestSdp = sdp;

					try {
						Message msg = Message.obtain(null, RobotService.MSG_SESSION_REQUEST_ARRIVAL);
						proxyMsger.send(msg);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			});

			Message notify = Message.obtain(null, RobotService.MSG_SESSION_MANAGER_INITIALIZED);
			proxyMsger.send(notify);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	void cleanupSessionManager(Message msg) {
		if (sessionMgr != null) {
			sessionMgr.cleanup();
			sessionMgr = null;
		}
	}

	class TestStreamHandler extends AbstractStreamHandler {
		Synchronizer synch = new Synchronizer();
		Stream stream;
		StreamState state;

		byte[] receivedData;

		@Override
		public void onStateChanged(Stream stream, StreamState state) {
			try {
				switch (state) {
					case Initialized:
						activeSession.replyRequest(0, null);
						break;
					case TransportReady:
						activeSession.start(sessionRequestSdp);
						break;
					case Connected:
						Message msg = Message.obtain(null, RobotService.MSG_SESSION_CONNECTED);
						proxyMsger.send(msg);
						break;
					default:
						break;
				}
			} catch (CarrierException e) {
				e.printStackTrace();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		@Override
		public void onStreamData(Stream stream, byte[] data) {
			try {
				stream.writeData(data);
			} catch (CarrierException e) {
				e.printStackTrace();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	void replySessionRequestAndStart(Message msg) {
		try {
			activeSession = sessionMgr.newSession(sessionRequestFrom);

			TestStreamHandler streamHandler = new TestStreamHandler();
			activeStream = activeSession.addStream(StreamType.Text, 0, streamHandler);
		} catch (CarrierException e) {
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
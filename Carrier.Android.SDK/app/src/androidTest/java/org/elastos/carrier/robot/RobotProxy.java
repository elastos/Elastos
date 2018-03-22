package org.elastos.carrier.robot;

import android.os.Looper;
import android.os.RemoteException;

import android.util.Log;

import android.content.ComponentName;
import android.content.Intent;
import android.content.Context;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Handler;
import android.os.Messenger;
import android.os.Message;
import android.os.Bundle;

import org.elastos.carrier.common.Synchronizer;

public class RobotProxy {
	private static final String TAG = "RobotProxy";

	private static RobotProxy robotProxy;

	private Context   context;
	private Messenger robotMsger;
	private Messenger proxyMsger;
	private boolean   isBound;
	private RobotIdReceiver robotIdReceiver;
	private Synchronizer waitObj;

	public interface RobotIdReceiver {
		public void onReceived(String robotAddress, String robotId);
	}

	private ServiceConnection connection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			robotMsger = new Messenger(service);
			try {
				Message msg = Message.obtain(null, RobotService.MSG_REQ_ROBOT_ID);
				msg.replyTo = proxyMsger;
				robotMsger.send(msg);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			robotMsger = null;
		}
	};

	class IncomingHandler extends Handler {
		IncomingHandler() {
			super(Looper.getMainLooper());
		}

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
				case RobotService.MSG_RSP_ROBOT_ID:
					robotIdReceiver.onReceived(msg.getData().getString("address"),
												msg.getData().getString("robotId"));
					break;

				case RobotService.MSG_REQ_FRIEND_ARRIVAL:
					waitObj.wakeup();
					break;

				case RobotService.MSG_MESSAGE_ARRIVAL:
					waitObj.wakeup();
					break;

				case RobotService.MSG_SESSION_MANAGER_INITIALIZED:
					waitObj.wakeup();
					break;

				case RobotService.MSG_SESSION_REQUEST_ARRIVAL:
					waitObj.wakeup();
					break;

				case RobotService.MSG_SESSION_CONNECTED:
					waitObj.wakeup();
					break;

				default:
					super.handleMessage(msg);
			}
		}
	}

	public static RobotProxy getRobot(Context context) {
		if (robotProxy == null) {
			robotProxy = new RobotProxy(context);
		}
		return robotProxy;
	}

	private RobotProxy(Context context) {
		proxyMsger = new Messenger(new IncomingHandler());
		waitObj = new Synchronizer();
		isBound = false;
		this.context = context;
	}

	public void bindRobot(RobotIdReceiver receiver) {
		Intent intent = new Intent(context, RobotService.class);
		context.bindService(intent, connection, Context.BIND_AUTO_CREATE);
		robotIdReceiver = receiver;
		isBound = true;
	}

	public void unbindRobot() {
		context.unbindService(connection);
		robotIdReceiver = null;
		isBound = false;
	}

	public void testHello() {
		try {
			Message msg = Message.obtain(null, RobotService.MSG_TEST);
			Bundle data = new Bundle();
			data.putString("hello", "test hello");
			msg.setData(data);
			msg.replyTo = proxyMsger;
			proxyMsger.send(msg);
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	public void waitForRequestArrival() {
		waitObj.await();
	}

	public void waitForMessageArrival() {
		waitObj.await();
	}

	public void testRobotAcceptFriend(String me) {
		try {
			Message msg = Message.obtain(null, RobotService.MSG_CONFIRM_FRIEND_REQUEST);
			Bundle req = new Bundle();
			req.putString("to", me);
			msg.setData(req);
			robotMsger.send(msg);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void tellRobotSendMessage(String me, String message) {
		try {
			Message msg = Message.obtain(null, RobotService.MSG_SEND_ME_MESSAGE);
			Bundle req = new Bundle();
			req.putString("to", me);
			req.putString("message", message);
			msg.setData(req);
			robotMsger.send(msg);
		} catch (Exception e)  {
			e.printStackTrace();
		}
	}

	public void tellRobotInitSessionManager(int transports) {
		try {
			Message msg = Message.obtain(null, RobotService.MSG_SESSION_MANAGER_INIT);
			Bundle req = new Bundle();
			req.putInt("transports", transports);
			msg.setData(req);
			robotMsger.send(msg);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void tellRobotCleanupSessionManager() {
		try {
			Message msg = Message.obtain(null, RobotService.MSG_SESSION_MANAGER_CLEANUP);
			robotMsger.send(msg);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void waitForSesionManagerInitialzed() {
		waitObj.await();
	}

	public void waitForSessionRequestArrival() {
		waitObj.await();
	}

	public void tellRobotReplySessionRequestAndStart() {
		try {
			Message msg = Message.obtain(null, RobotService.MSG_REPLY_SESSION_REQUST_AND_START);
			Bundle data = new Bundle();
			msg.setData(data);
			robotMsger.send(msg);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void waitForSessionConnected() {
		waitObj.await();
	}
}

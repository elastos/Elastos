package org.elastos.carrier.common;

import android.support.test.InstrumentationRegistry;
import org.elastos.carrier.ConnectionStatus;
import org.elastos.carrier.UserInfo;

public class TestContext {
	private Bundle mExtraBundle;

	static public class Bundle {
		private boolean robotOnline = false;
		private ConnectionStatus robotConnectionStatus = ConnectionStatus.Disconnected;
		private String from;
		private String hello;
		private UserInfo userInfo;
		private Object extraData;

		public void setRobotOnline(boolean online) {
			robotOnline = online;
		}

		public boolean getRobotOnline() {
			return robotOnline;
		}

		public void setRobotConnectionStatus(ConnectionStatus status) {
			robotConnectionStatus = status;
		}

		public ConnectionStatus getRobotConnectionStatus() {
			return robotConnectionStatus;
		}

		public void setFrom(String from) {
			this.from = from;
		}

		public  String getFrom() {
			return from;
		}

		public  void setHello(String hello) {
			this.hello = hello;
		}

		public String getHello() {
			return hello;
		}

		public void setUserInfo(UserInfo userInfo) {
			this.userInfo = userInfo;
		}

		public UserInfo getUserInfo() {
			return userInfo;
		}

		public void setExtraData(Object extraData) {
			this.extraData = extraData;
		}

		public Object getExtraData() {
			return extraData;
		}
	}

	public TestContext() {
		mExtraBundle = new Bundle();
	}

	public void setExtra(Bundle bundle) {
		mExtraBundle = bundle;
	}

	public Bundle getExtra() {
		return mExtraBundle;
	}

	public void reset() {
		//mExtraBundle = new Bundle();
	}

	public String getAppPath() {
		return InstrumentationRegistry.getTargetContext().getFilesDir().getAbsolutePath();
	}
}

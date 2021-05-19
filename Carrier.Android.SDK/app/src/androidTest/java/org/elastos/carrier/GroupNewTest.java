package org.elastos.carrier;

import org.elastos.carrier.common.TestContext;
import org.elastos.carrier.common.TestErrorCode;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.elastos.carrier.exceptions.GeneralException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

public class GroupNewTest {
	private static final String TAG = "GroupNewTest";
	private static TestContext context = new TestContext();
	private static TestHandler handler = new TestHandler(context);
	private static Carrier carrier;

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
			Log.d(TAG, "Robot connection status changed -> " + status.toString());
		}

		@Override
		public void onFriendRequest(Carrier carrier, String userId, UserInfo info, String hello) {
			Log.d(TAG, "Received friend request from " + userId + " with hello: " + hello);
		}

		@Override
		public void onFriendAdded(Carrier carrier, FriendInfo info) {
			Log.d(TAG, String.format("Friend %s added", info.getUserId()));
		}

		@Override
		public void onFriendRemoved(Carrier carrier, String friendId) {
			Log.d(TAG, String.format("Friend %s removed", friendId));
		}

		@Override
		public void onGroupInvite(Carrier carrier, String from, byte[] cookie) {
			Log.d(TAG, String.format("Group invite from: %s", from));
		}

		@Override
		public void onGroupConnected(Group group) {
			Log.d(TAG, "onGroupConnected");
		}

		@Override
		public void onGroupMessage(Group group, String from, byte[] message) {
			Log.d(TAG, String.format("Group Message from: %s, message:%s", from, (new String(message))));
		}

		@Override
		public void onGroupTitle(Group group, String from, String title) {
			Log.d(TAG, String.format("Group Title from: %s, title:%s", from, title));
		}

		@Override
		public void onPeerName(Group group, String peerId, String peerName) {
			Log.d(TAG, String.format("Peer Name peerId:%s, title:%s", peerId, peerName));
		}

		@Override
		public void onPeerListChanged(Group group) {
			Log.d(TAG, "onPeerListChanged");
		}
	}

	@Test
	public void testNewGroup() {
		try {
			Group group = carrier.newGroup();
			carrier.groupLeave(group);
		}
		catch (CarrierException e) {
			fail();
		}
	}

	@Test
	public void testLeaveGroupTwice() {
		try {
			Group group = carrier.newGroup();
			carrier.groupLeave(group);

			try {
				//carrier: groupLeave
				carrier.groupLeave(group);
			}
			catch (IllegalArgumentException e) {
				e.printStackTrace();
			}

			try {
				//group: leave
				group.leave();
			}
			catch (GeneralException e) {
				final int ELAERR_NOT_EXIST = 0x0A;
				assertEquals(CarrierException.FACILITY_GENERAL, e.getFacility());
				assertEquals(ELAERR_NOT_EXIST, e.getCode());
			}
		}
		catch (CarrierException e) {
			fail();
		}
	}

	@BeforeClass
	public static void setUp() {
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
	}
}

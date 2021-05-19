package org.elastos.carrier;

import org.elastos.carrier.common.RobotConnector;
import org.elastos.carrier.common.TestContext;
import org.elastos.carrier.common.TestErrorCode;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

public class GroupListTest {
	private static final String TAG = "GroupListTest";
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
	public void testGetGroups() {
		try {
			final int LEN = 3;
			List<Group> groups = new ArrayList<>();
			for (int i = 0; i < LEN; i++) {
				Group group = carrier.newGroup();
				assertNotNull(group);
				groups.add(group);
			}

			Collection<Group> groupsCollection = carrier.getGroups();
			assertEquals(groups.size(), groupsCollection.size());

			Iterator it = groupsCollection.iterator();
			int pos = 0;
			while(it.hasNext()) {
				Group group = (Group)it.next();

				boolean has = false;
				for (int i = 0; i < LEN; i++) {
					if (groups.get(i).getId().equals(group.getId())) {
						has = true;
						break;
					}
				}
				assertTrue(has);
				pos ++;
			}

			assertEquals(LEN, pos);

			for (int i = 0; i < LEN; i++) {
				carrier.groupLeave(groups.get(i));
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

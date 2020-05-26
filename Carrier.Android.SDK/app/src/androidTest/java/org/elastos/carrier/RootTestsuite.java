package org.elastos.carrier;

import org.elastos.carrier.common.RobotConnector;
import org.elastos.carrier.session.ChannelTest;
import org.elastos.carrier.session.ManagerTest;
import org.elastos.carrier.session.NewTest;
import org.elastos.carrier.session.PortforwardingTest;
import org.elastos.carrier.session.RequestReplyTest;
import org.elastos.carrier.session.StreamTest;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import static org.junit.Assert.fail;

@RunWith(Suite.class)
@Suite.SuiteClasses({
		//Carrier
		GetInstanceTest.class,
		NodeLoginTest.class,
		CheckIDTest.class,
		FriendAddTest.class,
		FriendInviteTest.class,
		FriendLabelTest.class,
		FriendMessageTest.class,
		// FriendOfflineMessageTest.class,
		FriendReceiptMessageTest.class,
		GetIDTest.class,
		GetInfoTest.class,
		GetVersionTest.class,

		//CarrierExtension
		CarrierExtensionTest.class,

		//Group
		GroupNewTest.class,
		GroupTitleTest.class,
		GroupInviteJoinTest.class,
		GroupListTest.class,
		GroupGetPeerTest.class,
		GroupMessageTest.class,

		//Session
		ManagerTest.class,
		NewTest.class,
		RequestReplyTest.class,
		StreamTest.class,
		PortforwardingTest.class,
		ChannelTest.class,
})

public class RootTestsuite {
	private static String TAG = "RootTestsuite";
	private static RobotConnector robot;
	private static int ConnectRetryTimes = 3;

	@BeforeClass
	public static void setup() {
		Log.d(TAG, "Carrier [setup]");
		robot = RobotConnector.getInstance();
		int retryTimes = 0;
		while (true) {
			if (!robot.connectToRobot()) {
				if (++retryTimes >= ConnectRetryTimes) {
					Log.e(TAG, "Connection to robot failed, abort this test");
					fail();
				}
				Log.d(TAG, "connectToRobot failed, retry " + retryTimes);

                try {
                    Thread.sleep(5000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
			else break;
		}
	}

	@AfterClass
	public static void teardown() {
		Log.d(TAG, "Carrier [teardown]");
		robot.disconnect();
	}
}

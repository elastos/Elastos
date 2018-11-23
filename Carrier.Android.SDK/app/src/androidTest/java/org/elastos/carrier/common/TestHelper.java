package org.elastos.carrier.common;

import android.util.Log;

import org.elastos.carrier.Carrier;
import org.elastos.carrier.exceptions.CarrierException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

public class TestHelper {
	public interface ITestChannelExecutor {
		void executor();
	}

	public static boolean addFriendAnyway(Carrier carrier, RobotConnector robot,
					Synchronizer syncher, Synchronizer friendConnSyncher, TestContext context) {
		try {
			if (!carrier.isFriend(robot.getNodeid())) {
				carrier.addFriend(robot.getAddress(), "auto-reply");

				// wait for friend_added callback invoked.
				syncher.await();
			}
			else {
				String hello = "auto-reply";
				assertTrue(robot.writeCmd(String.format("fadd %s %s %s", carrier.getUserId(),
					carrier.getAddress(), hello)));
			}

			String[] args = robot.readAck();
			assertEquals(args.length, 2);
			assertEquals(args[0], "fadd");
			assertEquals(args[1], "succeeded");

			TestContext.Bundle bundle = context.getExtra();
			if (!bundle.getRobotOnline()) {
				// wait for friend_connection (online) callback invoked.
				friendConnSyncher.await();
			}
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
			return false;
		}

		return true;
	}

	public static boolean removeFriendAnyway(Carrier carrier, RobotConnector robot,
					Synchronizer syncher, Synchronizer friendConnSyncher, TestContext context) {
		try {
			if (carrier.isFriend(robot.getNodeid())) {
				carrier.removeFriend(robot.getNodeid());

				// wait for friend_removed callback invoked.
				syncher.await();

				TestContext.Bundle bundle = context.getExtra();
				if (bundle.getRobotOnline()) {
					// wait for friend_connection (online -> offline) callback invoked.
					friendConnSyncher.await();
				}
			}

			assertTrue(robot.writeCmd(String.format("fremove %s", carrier.getUserId())));

			// wait for completion of robot "fremove" command.
			String[] args = robot.readAck();
			assertEquals(args.length, 2);
			assertEquals(args[0], "fremove");
			assertEquals(args[1], "succeeded");
		}
		catch (CarrierException e) {
			e.printStackTrace();
			return false;
		}

		return true;
	}

	public static byte[] getBytes(char[] chars) {
		String tmp = new String(chars);
		return tmp.getBytes();
	}
}

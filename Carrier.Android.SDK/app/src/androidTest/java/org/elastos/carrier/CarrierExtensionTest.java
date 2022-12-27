package org.elastos.carrier;

import android.util.Log;

import org.elastos.carrier.common.RobotConnector;
import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestContext;
import org.elastos.carrier.common.TestHelper;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.Timeout;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(JUnit4.class)
public class CarrierExtensionTest {
    private static final String TAG = "CarrierExtensionTest";

    private static TestContext context = new TestContext();
    private static Synchronizer friendConnSyncher = new Synchronizer();
    private static Synchronizer commonSyncher = new Synchronizer();
    private static RobotConnector robot;
    private static Carrier carrier;
    private static CarrierExtension extension;
    private static AbstractCarrierHandler handler = new AbstractCarrierHandler() {
        @Override
        public void onReady(Carrier carrier) {
            commonSyncher.wakeup();
        }

        @Override
        public void onFriendConnection(Carrier carrier, String friendId, ConnectionStatus status) {
            TestContext.Bundle bundle = context.getExtra();
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
    };

    @Rule
    public Timeout globalTimeout = Timeout.seconds(600);

    static private class LocalData {
        private int status = 0;
        private String reason = null;
        private String data = null;
    }

    static private class InviteResposeHandler implements FriendInviteResponseHandler {
        public void onReceived(String from, int status, String reason, String data) {
            TestContext.Bundle bundle = context.getExtra();
            LocalData localData = (LocalData)bundle.getExtraData();
            if (localData == null) {
                localData = new LocalData();
            }
            localData.status = status;
            localData.reason = reason;
            localData.data = data;
            bundle.setExtraData(localData);
            Log.d(TAG, String.format("onReceived from [%s], data is [%s]", from, data));

            commonSyncher.wakeup();
        }
    }

    @Test
    public void testFriendInviteConfirm() {
        try {
            friendConnSyncher.reset();
            commonSyncher.reset();

            assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
            assertTrue((carrier.isFriend(robot.getNodeid())));
            String hello = "hello";
            InviteResposeHandler h = new InviteResposeHandler();
            extension.inviteFriend(robot.getNodeid(), hello, h);
            String[] args = robot.readAck();
            if (args == null || args.length != 2) {
                fail();
            }
            assertEquals("ext_data", args[0]);
            assertEquals(hello, args[1]);

            String invite_rsp_data = "invitation-confirmed";
            assertTrue(robot.writeCmd(String.format("extfreplyinvite %s confirm %s", carrier.getUserId(),
                    invite_rsp_data)));

            // wait for invite response callback invoked.
            commonSyncher.await();

            TestContext.Bundle bundle = context.getExtra();
            LocalData localData = (LocalData) bundle.getExtraData();
            assertEquals(robot.getNodeid(), bundle.getFrom());
            assertEquals(0, localData.status);
            assertTrue(localData.reason == null || localData.reason.isEmpty());
            assertEquals(invite_rsp_data, localData.data);
        } catch (CarrierException e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testFriendInviteReject() {
        try {
            friendConnSyncher.reset();
            commonSyncher.reset();

            assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
            assertTrue((carrier.isFriend(robot.getNodeid())));

            String hello = "hello";
            InviteResposeHandler h = new InviteResposeHandler();
            extension.inviteFriend(robot.getNodeid(), hello, h);
            String[] args = robot.readAck();
            if (args == null || args.length != 2) {
                fail();
            }
            assertEquals("ext_data", args[0]);
            assertEquals(hello, args[1]);

            String reason = "unknown-error";
            assertTrue(robot.writeCmd(String.format("extfreplyinvite %s refuse %s", carrier.getUserId(),
                    reason)));

            // wait for invite response callback invoked.
            commonSyncher.await();

            TestContext.Bundle bundle = context.getExtra();
            LocalData localData = (LocalData) bundle.getExtraData();
            assertEquals(robot.getNodeid(), bundle.getFrom());
            assertTrue(localData.status != 0);
            assertEquals(reason, localData.reason);
            assertTrue(localData.data == null || localData.data.isEmpty());
        } catch (CarrierException e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testInvitedByFriendThenAccept() {
        try {
            friendConnSyncher.reset();
            commonSyncher.reset();

            assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
            assertTrue((carrier.isFriend(robot.getNodeid())));

            String hello = "hello";
            assertTrue(robot.writeCmd(String.format("extfinvite %s %s", carrier.getUserId(), hello)));

            // wait for invite callback invoked.
            commonSyncher.await();

            TestContext.Bundle bundle = context.getExtra();
            LocalData localData = (LocalData) bundle.getExtraData();
            assertEquals(robot.getNodeid(), bundle.getFrom());
            assertEquals(localData.status, 0);
            assertNull(localData.reason);
            assertEquals(localData.data, hello);

            extension.replyFriendInvite(robot.getNodeid(), 0, null, hello);

            String[] args = robot.readAck();
            if (args == null || args.length != 3) {
                fail();
            }
            assertEquals("ext_freply", args[0]);
            assertEquals("confirm", args[1]);
            assertEquals(hello, args[2]);
        } catch (CarrierException e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testInvitedByFriendThenReject() {
        try {
            friendConnSyncher.reset();
            commonSyncher.reset();

            assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
            assertTrue((carrier.isFriend(robot.getNodeid())));

            String hello = "hello";
            assertTrue(robot.writeCmd(String.format("extfinvite %s %s", carrier.getUserId(), hello)));

            // wait for invite callback invoked.
            commonSyncher.await();

            TestContext.Bundle bundle = context.getExtra();
            LocalData localData = (LocalData) bundle.getExtraData();
            assertEquals(robot.getNodeid(), bundle.getFrom());
            assertEquals(localData.status, 0);
            assertNull(localData.reason);
            assertEquals(localData.data, hello);

            extension.replyFriendInvite(robot.getNodeid(), -1, hello, null);

            String[] args = robot.readAck();
            if (args == null || args.length != 3) {
                fail();
            }
            assertEquals("ext_freply", args[0]);
            assertEquals("refuse", args[1]);
            assertEquals(hello, args[2]);
        } catch (CarrierException e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testGetTurnServer() {
        try {
            CarrierExtension.TurnServerInfo sinfo = extension.getTurnServerInfo();
            assertNotNull(sinfo);
        } catch (CarrierException e) {
            e.printStackTrace();
            fail();
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
            extension = new CarrierExtension(carrier) {
                protected void onFriendInvite(Carrier carrier, String from, String data) {
                    TestContext.Bundle bundle = context.getExtra();
                    bundle.setFrom(from);
                    LocalData localData = (LocalData)bundle.getExtraData();
                    if (localData == null) {
                        localData = new LocalData();
                    }
                    localData.data = data;
                    localData.reason = null;
                    localData.status = 0;
                    bundle.setExtraData(localData);
                    Log.d(TAG, String.format("Friend [%s] invite info [%s]", from, data));

                    commonSyncher.wakeup();
                }
            };
            extension.registerExtension();
            carrier.start(0);
            commonSyncher.await();
            Log.i(TAG, "Carrier node is ready now");
        } catch (CarrierException e) {
            e.printStackTrace();
            Log.e(TAG, "Carrier node start failed, abort this test.");
        }
    }

    @AfterClass
    public static void tearDown() {
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

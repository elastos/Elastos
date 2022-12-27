package org.elastos.carrier;

import android.util.Log;

import org.elastos.carrier.common.RobotConnector;
import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestContext;
import org.elastos.carrier.common.TestHelper;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.elastos.carrier.filetransfer.FileTransfer;
import org.elastos.carrier.filetransfer.FileTransferHandler;
import org.elastos.carrier.filetransfer.FileTransferState;
import org.elastos.carrier.filetransfer.Manager;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

public class FiletransferNewTest {
    private static final String TAG = "FiletransferFileTest";
    private static Synchronizer commonSyncher = new Synchronizer();
    private static Synchronizer friendConnSyncher = new Synchronizer();
    private static TestContext context = new TestContext();
    private static FiletransferNewTest.TestHandler handler = new FiletransferNewTest.TestHandler(context);
    private static RobotConnector robot;
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
            TestContext.Bundle bundle = mContext.getExtra();
            bundle.setRobotOnline(status == ConnectionStatus.Connected);
            mContext.setExtra(bundle);

            android.util.Log.d(TAG, "Robot connection status changed -> " + status.toString());
            friendConnSyncher.wakeup();
        }

        @Override
        public void onFriendRequest(Carrier carrier, String userId, UserInfo info, String hello) {
            android.util.Log.d(TAG, "Received friend request from " + userId + " with hello: " + hello);
            commonSyncher.wakeup();
        }

        @Override
        public void onFriendAdded(Carrier carrier, FriendInfo info) {
            commonSyncher.wakeup();
            android.util.Log.d(TAG, String.format("Friend %s added", info.getUserId()));
        }

        @Override
        public void onFriendRemoved(Carrier carrier, String friendId) {
            commonSyncher.wakeup();
            android.util.Log.d(TAG, String.format("Friend %s removed", friendId));
        }
    }

    static class TestFiletransferHandler implements FileTransferHandler {
        private TestContext mContext;

        TestFiletransferHandler(TestContext context) {
            mContext = context;
        }

        @Override
        public void onStateChanged(FileTransfer filetransfer, FileTransferState state) {

        }

        @Override
        public void onFileRequest(FileTransfer filetransfer, String fileId, String filename, long size) {

        }

        @Override
        public void onPullRequest(FileTransfer filetransfer, String fileId, long offset) {

        }

        @Override
        public boolean onData(FileTransfer filetransfer, String fileId, byte[] data) {
            return false;
        }

        @Override
        public void onDataFinished(FileTransfer filetransfer, String fileId) {

        }

        @Override
        public void onPending(FileTransfer filetransfer, String fileId) {

        }

        @Override
        public void onResume(FileTransfer filetransfer, String fileId) {

        }

        @Override
        public void onCancel(FileTransfer filetransfer, String fileId, int status, String reason) {

        }
    }

    @Test
    public void testFiletransferNew() {
        try {
            String args[];
            FileTransfer ft = null;
            Manager mgr = null;
            TestFiletransferHandler filetransferHandler = null;

            commonSyncher.reset();
            friendConnSyncher.reset();

            assertTrue(TestHelper.addFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
            assertTrue((carrier.isFriend(robot.getNodeid())));

            mgr = Manager.createInstance(carrier);
            assertTrue(mgr != null);

            assertTrue(robot.writeCmd("ft_init"));

            args = robot.readAck();
            if (args == null || args.length != 2) {
                fail();
            }
            assertEquals("ft_init", args[0]);
            assertEquals("succeeded", args[1]);

            filetransferHandler = new TestFiletransferHandler(context);
            assertTrue(filetransferHandler != null);
            ft = mgr.newFileTransfer(robot.getNodeid(), null, filetransferHandler);
            assertTrue(ft != null);

            assertTrue(robot.writeCmd("ft_cleanup"));

            args = robot.readAck();
            if (args == null || args.length != 2) {
                fail();
            }
            assertEquals("ft_cleanup", args[0]);
            assertEquals("succeeded", args[1]);

            mgr.cleanup();
        } catch (CarrierException e) {
            e.printStackTrace();
            fail();
        }
    }

    @Test
    public void testFiletransferNewWithStranger() {
        try {
            FileTransfer ft = null;
            Manager mgr = null;
            TestFiletransferHandler filetransferHandler = null;

            commonSyncher.reset();
            friendConnSyncher.reset();

            assertTrue(TestHelper.removeFriendAnyway(carrier, robot, commonSyncher, friendConnSyncher, context));
            assertFalse((carrier.isFriend(robot.getNodeid())));

            mgr = Manager.createInstance(carrier);
            assertTrue(mgr != null);

            filetransferHandler = new TestFiletransferHandler(context);
            assertTrue(filetransferHandler != null);
            ft = mgr.newFileTransfer(robot.getNodeid(), null, filetransferHandler);

            mgr.cleanup();
        } catch (CarrierException e) {
            assertEquals(0x8100000A, e.getErrorCode());
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
        }
        catch (CarrierException | InterruptedException e) {
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
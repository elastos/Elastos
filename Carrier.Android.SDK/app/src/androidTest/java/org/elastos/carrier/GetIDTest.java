package org.elastos.carrier;

import android.util.Log;

import org.elastos.carrier.common.TestContext;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(JUnit4.class)
public class GetIDTest {
	private static final String TAG = "GetIDTest";
	private static TestContext context = new TestContext();
	private static TestHandler handler = new TestHandler();
	private static Carrier carrier;

	static class TestHandler extends AbstractCarrierHandler {
		@Override
		public void onReady(Carrier carrier) {
			synchronized (carrier) {
				carrier.notify();
			}
		}
	}

	@Test
	public void testGetAddress() {
		try {
			String address = carrier.getAddress();
			assertTrue(address != null && !address.isEmpty());
			assertTrue(Carrier.isValidAddress(address));
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testGetNodeId() {
		try {
			String nodeId = carrier.getNodeId();
			assertTrue(nodeId != null && !nodeId.isEmpty());
			assertTrue(Carrier.isValidId(nodeId));
		}
		catch (CarrierException e) {
			e.printStackTrace();
			fail();
		}
	}

	@Test
	public void testGetUserId() {
		try {
			String userid = carrier.getUserId();
			assertTrue(userid != null && !userid.isEmpty());
			assertTrue(Carrier.isValidId(userid));
		}
		catch (CarrierException e) {
			e.printStackTrace();
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

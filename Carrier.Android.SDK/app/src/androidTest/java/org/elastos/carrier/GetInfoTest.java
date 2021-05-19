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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(JUnit4.class)
public class GetInfoTest {
	private static final String TAG = "GetInfoTest";
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
	public void testGetSelfInfo() {
		try {
			UserInfo me = new UserInfo();
			me.setName("zhangsan");
			me.setDescription("We all want a code to live by.");
			me.setGender("male");
			me.setPhone("01012345");
			me.setEmail("zhangsan@163.com");
			me.setRegion("Beijing");
			me.setHasAvatar(false);
			me.setUserId(carrier.getUserId());

			carrier.setSelfInfo(me);

			UserInfo info = carrier.getSelfInfo();
			assertEquals(me.getUserId(), info.getUserId());
			assertEquals(me.getRegion(), info.getRegion());
			assertEquals(me.getPhone(), info.getPhone());
			assertEquals(me.getName(), info.getName());
			assertEquals(me.getGender(), info.getGender());
			assertEquals(me.getEmail(), info.getEmail());
			assertEquals(me.getDescription(), info.getDescription());
			assertEquals(me.hasAvatar(), info.hasAvatar());
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

package org.elastos.carrier;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;


import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.ElastosException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class UserLoginTest {
	private static final String TAG = "UserLoginTest";
	private static Carrier carrierInst;
	private static TestOptions options;

	private static String getAppPath() {
		return InstrumentationRegistry.getTargetContext().getFilesDir().getAbsolutePath();
	}

	static class TestHandler extends AbstractCarrierHandler {
		Synchronizer synch = new Synchronizer();

		public void onReady(Carrier carrier) {
			synch.wakeup();
		}
	}

	@BeforeClass
	public static void setUp() {
		options = new TestOptions(getAppPath());
		TestHandler handler = new TestHandler();

		try {

			Carrier.initializeInstance(options, handler);
			carrierInst = Carrier.getInstance();
			carrierInst.start(1000);
			handler.synch.await();
		} catch (ElastosException e) {
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@AfterClass
	public static void tearDown() {
		try {
			carrierInst.kill();
		}catch(Exception e) {
			e.printStackTrace();
		}
	}

	@Test
	public void getNodeId() {
		try {
			assertEquals(carrierInst.getNodeId(), carrierInst.getUserId());
		} catch (ElastosException e){
			Log.e(TAG, "getNodeId error:" + e.getErrorCode());
			assertTrue(false);
		}
	}

	@Test
	public void getUserId() {
		try {
			assertEquals(carrierInst.getUserId(), carrierInst.getUserId());
		} catch (ElastosException e){
			Log.e(TAG, "getUserId error:" + e.getErrorCode());
			assertTrue(false);
		}
	}

	@Test
	public void testSelfInfo() {
		try {
			UserInfo info1 = carrierInst.getSelfInfo();
			UserInfo info2 = carrierInst.getSelfInfo();

			info2.setDescription("test description");
			info2.setEmail("test.selfinfo@fake.com");
			info2.setGender("female");
			info2.setName("test name");
			info2.setPhone("test phone");
			info2.setRegion("test region");
			info2.setHasAvatar(false);

			carrierInst.setSelfInfo(info2);

			UserInfo info3 = carrierInst.getSelfInfo();
			assertEquals(info3.getUserId(), info2.getUserId());
			assertEquals(info3.getDescription(), info2.getDescription());
			assertEquals(info3.getEmail(), info2.getEmail());
			assertEquals(info3.getGender(), info2.getGender());
			assertEquals(info3.getPhone(), info2.getPhone());
			assertEquals(info3.getRegion(), info2.getRegion());

			info1.setHasAvatar(false);
			carrierInst.setSelfInfo(info1);

			UserInfo info4 = carrierInst.getSelfInfo();
			assertEquals(info4.getUserId(), info1.getUserId());
			assertEquals(info4.getDescription(), info1.getDescription());
			assertEquals(info4.getEmail(), info1.getEmail());
			assertEquals(info4.getGender(), info1.getGender());
			assertEquals(info4.getPhone(), info1.getPhone());
			assertEquals(info4.getRegion(), info1.getRegion());
		} catch (ElastosException e) {
			Log.e(TAG, "testSelfInfo error: " + e.getErrorCode());
			assertTrue(false);
		} catch (Exception e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}
}

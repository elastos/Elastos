package org.elastos.carrier;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.junit.Test;
import org.junit.runner.RunWith;

import org.elastos.carrier.common.Synchronizer;
import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.ElastosException;
import org.elastos.carrier.Carrier;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

@RunWith(AndroidJUnit4.class)
public class GetInstanceTest extends AbstractCarrierHandler {
	private static final String TAG = "InstanceTest";

	private String getAppPath() {
		return InstrumentationRegistry.getTargetContext().getFilesDir().getAbsolutePath();
	}

	class TestHandler extends AbstractCarrierHandler {
		Synchronizer synch = new Synchronizer();

		public void onReady(Carrier carrier) {
			synch.wakeup();
		}
	}

	@Test
	public void testCarrier() {
		TestOptions options = new TestOptions(getAppPath());
		TestHandler handler = new TestHandler();

		try {
			Carrier carrier = Carrier.getInstance(options, handler);
			assertNotEquals(null, carrier);

			carrier.start(1000);
			handler.synch.await();

			assertEquals(carrier, Carrier.getInstance());
			assertEquals(carrier.getNodeId(), carrier.getUserId());

			carrier.kill();
			assertEquals(null, Carrier.getInstance());

		} catch (ElastosException e) {
			Log.e(TAG, "test error:" + e.getErrorCode());
			assertTrue(false);
		} catch (Exception e) {
			e.printStackTrace();
			assertTrue(false);
		}
	}
}

package org.elastos.carrier;

import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import org.elastos.carrier.common.TestOptions;
import org.elastos.carrier.exceptions.CarrierException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

@RunWith(AndroidJUnit4.class)
public class GetInstanceTest extends AbstractCarrierHandler {
	private static final String TAG = "GetInstanceTest";

	private String getAppPath() {
		return InstrumentationRegistry.getTargetContext().getFilesDir().getAbsolutePath();
	}

	class TestHandler extends AbstractCarrierHandler {
		@Override
		public void onReady(Carrier carrier) {
			synchronized(carrier) {
				carrier.notify();
			}
		}
	}

	@Test
	public void testCarrier() {
		TestOptions options = new TestOptions(getAppPath());
		TestHandler handler = new TestHandler();

		try {
			Carrier carrier = Carrier.createInstance(options, handler);
			assertNotNull(carrier);

			carrier.start(0);
			synchronized(carrier) {
				carrier.wait();
			}
			assertEquals(carrier.getNodeId(), carrier.getUserId());

			carrier.kill();
//			assertNull(Carrier.getInstance());
		} catch (CarrierException | InterruptedException e) {
			e.printStackTrace();
			fail();
		}
	}
}

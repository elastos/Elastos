package org.elastos.carrier;

import android.util.Log;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import static org.junit.Assert.assertNotNull;

@RunWith(JUnit4.class)
public class GetVersionTest {
    private static final String TAG = "GetVersionTest ";

    @Test
    public void testGetVersion() {
        String ver = Carrier.getVersion();
        assertNotNull(ver);
        Log.i(TAG, String.format("Current version: %s", ver));
    }
}

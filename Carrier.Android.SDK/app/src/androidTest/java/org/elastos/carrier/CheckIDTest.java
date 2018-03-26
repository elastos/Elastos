package org.elastos.carrier;

import android.support.test.runner.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.elastos.carrier.Carrier;

import static org.junit.Assert.assertEquals;

@RunWith(AndroidJUnit4.class)
public class CheckIDTest {
	@Test
	public void checkValidId() {
		String userId  = "7sRQjDsniyuHdZ9zsQU9DZbMLtQGLBWZ78yHWgjPpTKm";
		assertEquals(Carrier.isValidId(userId), true);
	}

	@Test
	public void checkValidAddress() {
		String address = "VyhDgjkjd5MkPcuwCjGEUCp5jV6HArxSVmBnpXnk7d9h7cQtboMN";
		assertEquals(Carrier.isValidAddress(address), true);
	}

	@Test
	public void checkInvalidId() {
		String userId  = "aaaaaasniyuHdZ9zsQU9DZbMLtQGLBWZ78yHWgjPpTKm";
		assertEquals(Carrier.isValidId(userId), false);
	}

	@Test
	public void checkInvalidAddress() {
		String address = "aaaaaaakjd5MkPcuwCjGEUCp5jV6HArxSVmBnpXnk7d9h7cQtboMN";
		assertEquals(Carrier.isValidAddress(address), false);
	}
}

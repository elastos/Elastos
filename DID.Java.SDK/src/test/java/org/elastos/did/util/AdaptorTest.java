package org.elastos.did.util;

import org.elastos.did.adaptor.SPVAdaptor;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class AdaptorTest {
	private static SPVAdaptor adaptor;

	@Test
	public void test0() {
		adaptor = new SPVAdaptor("directory", "wallet", "network");
	}

	@Test
	public void test1() {
		adaptor.createIdTransaction("payload", "memo", "password");
	}

	@Test
	public void test2() {
		String s = adaptor.resolve("testdid");
		System.out.println(s);
	}

	@Test
	public void test3() {
		adaptor.destroy();
	}

}

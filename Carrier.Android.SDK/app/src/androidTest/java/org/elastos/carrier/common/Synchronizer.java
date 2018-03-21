package org.elastos.carrier.common;

import java.util.concurrent.Semaphore;

public class Synchronizer {
	Semaphore sema;

	public Synchronizer() {
		sema = new Semaphore(0);
	}

	public void wakeup() {
		sema.release();
	}

	public void await() {
		try {
			sema.acquire();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}

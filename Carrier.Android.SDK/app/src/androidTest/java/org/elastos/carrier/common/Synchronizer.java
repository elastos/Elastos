package org.elastos.carrier.common;

import android.util.Log;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Synchronizer {
	private Lock mLock;
	private Condition mCondition;
	private int signal_counts;

	public Synchronizer() {
		mLock = new ReentrantLock();
		mCondition = mLock.newCondition();
		signal_counts = 0;
	}

	public void await() {
		mLock.lock();
		try {
			if (--signal_counts < 0)
				mCondition.await();
		} catch (Exception e) {
			e.printStackTrace();
		}
		mLock.unlock();
	}

	public void wakeup() {
		mLock.lock();
		try {
			if (signal_counts++ < 0)
				mCondition.signal();
		} catch (Exception e) {
			e.printStackTrace();
		}
		mLock.unlock();
	}

	public void reset() {
		mLock.lock();
		try {
			mCondition.awaitNanos(1000);
			signal_counts = 0;
		} catch (Exception e) {
			e.printStackTrace();
		}
		mLock.unlock();
	}
}

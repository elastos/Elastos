package org.elastos.carrier.session;

import java.net.InetSocketAddress;

public class AddressInfo {
	private CandidateType type;
	private InetSocketAddress addr;
	private InetSocketAddress relatedAddr;

	public CandidateType getCandidateType() {
		return type;
	}

	void setCandidateType(int type) {
		this.type = CandidateType.valueOf(type);
	}

	public InetSocketAddress getAddress() {
		return addr;
	}

	void setAddress(InetSocketAddress addr) {
		this.addr = addr;
	}

	public InetSocketAddress getRelatedAddress() {
		return relatedAddr;
	}

	void setRelatedAddress(InetSocketAddress addr) {
		this.relatedAddr = addr;
	}
}

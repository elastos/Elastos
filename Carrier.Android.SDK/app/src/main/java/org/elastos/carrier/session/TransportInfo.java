package org.elastos.carrier.session;

import java.net.InetSocketAddress;

public class TransportInfo {
	private NetworkTopology topology;
	private AddressInfo localAddr;
	private AddressInfo remoteAddr;

	public NetworkTopology getTopology() {
		return topology;
	}

	void setTopology(int topology) {
		this.topology = NetworkTopology.valueOf(topology);
	}

	public AddressInfo getLocalAddressInfo() {
		return localAddr;
	}

	void setLocalAddressInfo(AddressInfo addrInfo) {
		this.localAddr = addrInfo;
	}

	public AddressInfo getRemoteAddressInfo() {
		return remoteAddr;
	}

	void setRemoteAddressInfo(AddressInfo addrInfo) {
		this.remoteAddr = addrInfo;
	}
}

// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRPeer.h"

#include "Peer.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		const uint32_t DEFAULT_MAGICNUMBER = uint32_t(0);

		std::string Peer::toString() const {
			//todo complete me
			return "";
		}

		BRPeer *Peer::getRaw() const {
			return _peer;
		}

		Peer::Peer() {
			_peer = BRPeerNew(DEFAULT_MAGICNUMBER);
		}

		Peer::Peer(const BRPeer &peer) {
			_peer = BRPeerNew(DEFAULT_MAGICNUMBER);
			*_peer = peer;
		}

		Peer::Peer(const UInt128 &addr, uint16_t port, uint64_t timestamp) {
			_peer = BRPeerNew(DEFAULT_MAGICNUMBER);
			_peer->address = addr;
			_peer->port = port;
			_peer->timestamp = timestamp;
			_peer->services = SERVICES_NODE_NETWORK;
			_peer->flags = 0;
		}

		Peer::Peer(uint32_t magicNumber) {
			_peer = BRPeerNew(magicNumber);
		}

		Peer::~Peer() {
			if (_peer != nullptr) {
				BRPeerFree(_peer);
			}
		}

		Peer::Peer(const Peer &peer) {
			_peer = BRPeerCopy(peer.getRaw());
		}

		Peer &Peer::operator=(const Peer &peer) {
			_peer = BRPeerCopy(peer.getRaw());
			return *this;
		}

		UInt128 Peer::getAddress() const {
			BRPeer *peer = getRaw();
			return peer->address;
		}

		uint16_t Peer::getPort() const {
			BRPeer *peer = getRaw();
			return peer->port;
		}

		uint64_t Peer::getTimestamp() const {
			BRPeer *peer = getRaw();
			return peer->timestamp;
		}

		void Peer::setEarliestKeyTime(uint32_t earliestKeyTime) {
			BRPeer *peer = getRaw();
			BRPeerSetEarliestKeyTime(peer, earliestKeyTime);
		}

		void Peer::setCurrentBlockHeight(uint32_t currentBlockHeight) {
			BRPeer *peer = getRaw();
			BRPeerSetCurrentBlockHeight(peer, currentBlockHeight);
		}

		Peer::ConnectStatus Peer::getConnectStatusValue() const {
			BRPeer *peer = getRaw();
			return ConnectStatus(BRPeerConnectStatus(peer));
		}

		void Peer::connect() const {
			BRPeer *peer = getRaw();
			BRPeerConnect(peer);
		}

		void Peer::disconnect() const {
			BRPeer *peer = getRaw();
			BRPeerDisconnect(peer);
		}

		void Peer::scheduleDisconnect(double time) {
			BRPeer *peer = getRaw();
			BRPeerScheduleDisconnect(peer, time);
		}

		void Peer::setNeedsFilterUpdate(bool needsFilterUpdate) {
			BRPeer *peer = getRaw();
			BRPeerSetNeedsFilterUpdate (peer, needsFilterUpdate);
		}

		std::string Peer::getHost() const {
			BRPeer *peer = getRaw();
			return std::string(BRPeerHost(peer));
		}

		uint32_t Peer::getVersion() const {
			BRPeer *peer = getRaw();
			return BRPeerVersion(peer);
		}

		std::string Peer::getUserAgent() const {
			BRPeer *peer = getRaw();
			return std::string(BRPeerUserAgent(peer));
		}

		uint32_t Peer::getLastBlock() const {
			BRPeer *peer = getRaw();
			return BRPeerLastBlock(peer);
		}

		uint64_t Peer::getFeePerKb() const {
			BRPeer *peer = getRaw();
			return BRPeerFeePerKb(peer);
		}

		double Peer::getPingTime() const {
			BRPeer *peer = getRaw();
			return BRPeerPingTime(peer);
		}

	}
}

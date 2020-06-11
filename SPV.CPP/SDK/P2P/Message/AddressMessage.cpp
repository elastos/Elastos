// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "AddressMessage.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <P2P/Peer.h>
#include <P2P/PeerManager.h>

#include <arpa/inet.h>

namespace Elastos {
	namespace ElaWallet {
		AddressMessage::AddressMessage(const MessagePeerPtr &peer) :
				Message(peer) {

		}

		bool AddressMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);
			uint64_t count = 0;

			if (!stream.ReadUint64(count)) {
				_peer->error("addr message read count fail");
				return false;
			}

			if (count > 1000) {
				_peer->error("dropping addr message, {} is too many addresses, max is 1000", count);
				return false;
			}

			if (_peer->SentGetaddr()) { // simple anti-tarpitting tactic, don't accept unsolicited addresses
				std::vector<PeerInfo> peers;
				peers.reserve(count);

				time_t now = time(NULL);

				_peer->info("got addr with {} address(es)", count);

				for (size_t i = 0; i < count; i++) {
					uint64_t timestamp;
					if (!stream.ReadUint64(timestamp)) {
						_peer->error("addr msg read timestamp fail");
						return false;
					}

					uint64_t services;
					if (!stream.ReadUint64(services)) {
						_peer->error("addr msg read services fail");
						return false;
					}

					uint128 address;
					if (!stream.ReadBytes(address)) {
						_peer->error("addr msg read addr fail");
						return false;
					}

					uint16_t port;
					if (!stream.ReadUint16(port)) {
						_peer->error("addr msg read port fail");
						return false;
					}
					PeerInfo p(address, port, timestamp, services);

					if ((p.Services & SERVICES_NODE_NETWORK) != SERVICES_NODE_NETWORK) {
						_peer->warn("p[{}]: t: {} s: {} {}:{} dropped: don't carry full blocks",
									i, p.Timestamp, p.Services, p.GetHost(), p.Port);
						continue; // skip peers that don't carry full blocks
					}

					if ((p.Services & SERVICES_NODE_BLOOM) != SERVICES_NODE_BLOOM) {
						_peer->warn("p[{}]: t: {} s: {} {}:{} dropped: don't support bloom filter",
									i, p.Timestamp, p.Services, p.GetHost(), p.Port);
						continue;
					}

					if (!p.IsIPv4() || p.Port == 0 || (p.IsIPv4() &&
						p.Address.begin()[12] == 127 && p.Address.begin()[13] == 0 &&
						p.Address.begin()[14] == 0 && p.Address.begin()[15] == 1)) {
						_peer->warn("p[{}]: t: {} s: {} {}:{} dropped",
									i, p.Timestamp, p.Services, p.GetHost(), p.Port);
						continue;
					}

					PEER_INFO(_peer, "p[{}]: t: {} s: {} {}:{}", i, p.Timestamp, p.Services, p.GetHost(), p.Port);


					// if address time is more than 10 min in the future or unknown, set to 5 days old
					if (p.Timestamp > now + 10 * 60 || p.Timestamp == 0) p.Timestamp = uint64_t(now - 5 * 24 * 60 * 60);
					p.Timestamp = p.Timestamp - 2 * 60 * 60; // subtract two hours

					peers.push_back(p);
				}

				if (peers.size() > 0) FireRelayedPeers(peers);
			}

			return true;
		}

		void AddressMessage::Send(const SendMessageParameter &param) {
			ByteStream stream;
			stream.WriteVarUint(0);

			//TODO: send peer addresses we know about
//			SendMessage(stream.GetBytes(), Type());
		}

		std::string AddressMessage::Type() const {
			return MSG_ADDR;
		}

	}
}

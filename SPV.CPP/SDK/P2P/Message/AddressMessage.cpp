// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arpa/inet.h>
#include <SDK/Common/Log.h>

#include "BRArray.h"
#include "SDK/P2P/Peer.h"
#include "AddressMessage.h"

namespace Elastos {
	namespace ElaWallet {
		AddressMessage::AddressMessage(const MessagePeerPtr &peer) :
				Message(peer) {

		}

		bool AddressMessage::Accept(const CMBlock &msg) {
			size_t off = 0;
			size_t count = UInt64GetLE(&msg[off]);
			off += sizeof(uint64_t);

			bool r = true;
			if (off == 0 || off + count * 30 > msg.GetSize()) {
				_peer->Perror("malformed addr message, length is {}, should be {} for {} address(es)", msg.GetSize(),
						   BRVarIntSize(count) + 30*count, count);
				r = false;
			} else if (count > 1000) {
				_peer->Perror("dropping addr message, {} is too many addresses, max is 1000", count);
			} else if (_peer->SentGetaddr()) { // simple anti-tarpitting tactic, don't accept unsolicited addresses
				std::vector<PeerInfo> peers;
				peers.reserve(count);

				time_t now = time(NULL);

				_peer->Pinfo("got addr with {} address(es)", count);

				for (size_t i = 0; i < count; i++) {
					uint64_t timestamp = UInt64GetLE(&msg[off]);
					off += sizeof(uint64_t);
					uint64_t services = UInt64GetLE(&msg[off]);
					off += sizeof(uint64_t);
					UInt128 address;
					UInt128Get(&address, &msg[off]);
					off += sizeof(UInt128);
					uint16_t port = UInt16GetLE(&msg[off]);
					off += sizeof(uint16_t);
					uint64_t id = UInt64GetLE(&msg[off]);
					off += sizeof(uint64_t);
					PeerInfo p(address, port, timestamp, services);

					char host[INET6_ADDRSTRLEN] = {0};
					if (p->isIPv4())
						inet_ntop(AF_INET, &p->getAddress().u32[3], host, sizeof(host));
					else
						inet_ntop(AF_INET6, &p->getAddress().u8[0], host, sizeof(host));
					_peer->Pdebug("peers[{}] = {}, timestamp = {}, port = {}, services = {}",
							 i, host, p->getTimestamp(), p->getPort(), p->getServices());

					if (!(p->getServices() & SERVICES_NODE_NETWORK)) continue; // skip peers that don't carry full blocks
					if (!p->isIPv4())
						continue; // ignore IPv6 for now
					if (p->isIPv4() &&
						p->getAddress().u8[12] == 127 && p->getAddress().u8[13] == 0 &&
						p->getAddress().u8[14] == 0 && p->getAddress().u8[15] == 1) {
						_peer->Pwarn("drop peers[{}]", i);
						continue;
					}

					// if address time is more than 10 min in the future or unknown, set to 5 days old
					if (p.Timestamp > now + 10 * 60 || p.Timestamp == 0) p.Timestamp = uint64_t(now - 5 * 24 * 60 * 60);
					p.Timestamp = p.Timestamp - 2 * 60 * 60; // subtract two hours

					peers.push_back(p);
				}

				if (peers.size() > 0) FireRelayedPeers(peers, peers.size());
			}

			return r;
		}

		void AddressMessage::Send(const SendMessageParameter &param) {
			CMBlock msg;
			msg.Resize(BRVarIntSize(0));
			BRVarIntSet(msg, msg.GetSize(), 0);

			//TODO: send peer addresses we know about
//			SendMessage(msg, Type());
		}

		std::string AddressMessage::Type() const {
			return MSG_ADDR;
		}

	}
}
